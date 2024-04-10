/* Text to image encrytion decryption


*/

// standard libraries
#include <stdlib.h> //standard library
#include <stdio.h>  //standard input/output library
#include <ctype.h>  //character classification functions
#include <string.h> //strings
#include <stdbool.h> //booloean macros
#include <time.h>

// imported libraries
#include "FreeImage.h" //image processing

// Definitions

// folder and file addresses relative to root folder
#define word_dictionary "data\\words.txt"
#define data_folder "data\\"
#define data_file "data\\userdat.bin"
#define contact_file "data\\contacts.bin"
#define message_folder "messages\\"
#define decrypt_folder "decrypt\\"
#define encrypt_folder "encrypt\\"

// color definitions
#define KRED  "\033[31m"   // Error end colors
#define KGRN  "\033[32m"   // success end colors
#define KYEL  "\033[33m"   // Warning cause colors
#define KBLU  "\033[34m"   // message and image contents colors
#define KMAG  "\033[35m"   // login colors
#define KCYN  "\033[36m"   // questions colors
#define KWHT  "\033[37m"   // starting instructions colors

// string limits
#define string_size 30      // normal 30 characters words
#define max_string_size 45  // maximum 45 characters words
#define line_length 512     // maximum 512 characters string

// hashing constants
#define hash_bucket_row 250     // hash all words into 250 row buckets
#define hash_bucket_column 256  // hash all words into 256 column buckets

#define max_hash_val 10000000   // maximum possible hash value < 1Cr

// Public_key = ((Private_key << 2) ^ (K_limit)) >> 2 | Z_limit;
#define K_limit 2000            // while making public key k factor be less than 2000
#define Z_limit 204             // z factor less than 204

// structures
typedef struct node     // node structures for word
{
    char *word;         // hold the word
    struct node *next;  // next node pointer
}node;

typedef struct node_head  // node head for nodes
{
    short unsigned int collision_count;  // counts number of nodes
    node *start;                         // starting node address
}node_head;


typedef struct one_pixel           // node structures for single pixels
{
    short unsigned int red;         // red value of pixel      
    short unsigned int green;       // green value of pixel
    short unsigned int blue;        // blue value of pixel
    struct one_pixel *next_pixel;   // next pixel pointer  
}one_pixel;

// CONTROL CHANNELS
#define control_border_pixel 251       // (251, , ) -> border pixels
#define control_single_encoding 252    // (252, x, x) -> single character encoding
#define control_double_encoding 253    // (253, y, z) -> double character encoding
#define control_nascii_encoding 254    // (254, y, z) -> non-dictionary words and characters
#define control_start_end_pixel 255    // (255, 0, 91) -> start and end of message pixels


typedef struct sender                 // public info of sender
{
    long long int saved_public_key;   // public key of sender
    char saved_name[string_size];     // sender's name
    int saved_name_hash;              // sender's name hashed
}sender;

struct user                                   // user datatype
{
    long long int saved_public_key;           // saved public data of user
    char saved_username[string_size];         // user name
    char saved_password[string_size];         // user password
    int saved_name_hash;                      // user name hashed
    int saved_private_key;                    // private key of user        
    int saved_passkey;                        // user passkey (only login)
} user_data;

// global variables
// user_data as a struct

// DHA algorithm numbers
const int P_DHA = 9973;                         // Chosen prime to operate 
const int G_DHA = 333;                          // chosen primitive root
long long int sender_user_common_key = 0;       // common key established

// Function Prototypes

// Math functions

int babylon_sqrt(int n);                           // find square root and round to greatest integer
int Nearest_prime(int n);                          // finds Nearest prime
long long int a_pow_b_mod_P(int a, int b, int P);  // computes (a^b) % P for DHA algorithm

// basic file functions

void Error_message(int i);
bool file_not_exists(char *filepath);

// User data functions

bool datafile_setup(char *path);
int display_contacts(sender **contact_info);
bool add_contact(one_pixel *contact_pixel);
int add_identity(one_pixel *pdata, node_head hash_table_address[hash_bucket_row][hash_bucket_column]);

// mapping and hashing functions

int hash_word(char *string, int offset);
one_pixel *hash_string(char *string, one_pixel *curr_pixel, short unsigned int val, node_head hash_table_address[hash_bucket_row][hash_bucket_column], short unsigned int name_encode);

// loading functions

void node_adder(node_head hash_table_address[hash_bucket_row][hash_bucket_column], char *string, unsigned short int i, unsigned short int j);
bool dict_Loader(node_head hash_table_address[hash_bucket_row][hash_bucket_column]);
int message_Loader(char *filepath, char ***msg_array); 

// Encryption & decryption functions

bool encode_key_to_start(one_pixel *head_pixel, char c);
bool decode_key_from_start(one_pixel *head_pixel);

// pixel functions

int txt_to_rgb(char **msg_arry, int size, one_pixel *image_data, node_head hash_table_address[hash_bucket_row][hash_bucket_column], char ans_c);
bool png_to_pixel(unsigned short int i, unsigned short int j, unsigned short int collision, one_pixel *currpixel);

// decoding function

void pixel_dehash(one_pixel *image_data, node_head hash_table_address[hash_bucket_row][hash_bucket_column], char *filesavepath);

// free memory functions

void free_contacts(sender *contact_info);
void free_image_data(one_pixel *image_data1);
void message_Unloader(char ***msg_array, int size);
void dict_Unloader(node_head hash_table_address[hash_bucket_row][hash_bucket_column]);

// main function
int main()
{
    // basic variables
    int num_lines, contact_num= 0;
    short unsigned int count = 0, imgflag = 0;
    char passw[string_size], filename[string_size], filepath[max_string_size], ans;
    passw[0] = filename[0] = filepath[0] = '\0';

    // constant
    const int temp_name_length = 10;

    // message array
    char **message_array;

    // contact card of other users
    sender *contacts;

    // initialise the starting pixel
    one_pixel val_image;
    {
        val_image.red = control_start_end_pixel;
        val_image.green = 0;
        val_image.blue = 91;
        val_image.next_pixel = NULL;
    }

    int num_pixel = 0, image_square = 0; // image size parameters for encryption

    // hash table of node heads and collision counts initialised
    node_head hash_table[hash_bucket_row][hash_bucket_column];
    for(int x = 0; x < hash_bucket_row; x++)
    {
        for(int y = 0; y < hash_bucket_column; y++)
        {
            hash_table[x][y].collision_count = 0;
            hash_table[x][y].start = NULL;
        }
    }

    /* Program Starts */
    printf(KWHT"\n!Welcome to Text to image encryption and decryption.!\n");

    // user data initialisation
    if(datafile_setup(data_file))
    {
        printf(KGRN"\nUser data initialized.\nYou can create your contact card and send to others, to receive encrypted messages.\n");
        printf("Your passkey is %d.\nStore it if you forget the password.\nPlease rerun the program.", user_data.saved_passkey);
        getchar();
        return 0;
    }

    // user verification
    printf(KMAG"\tHello %s,\nEnter your password: ", user_data.saved_username);
    //printf(KMAG"\n public key: %lld,\n name hash: %d\n", user_data.saved_public_key, user_data.saved_name_hash);
    do
    {
        if(count)
        {
            printf(KYEL"Incorrect password.\nRetry: ");
        }

        if(count == 6)
        {
            int passkey;
            printf(KMAG"\nEnter passkey: ");
            scanf("%d%*c", &passkey);
            if(passkey != user_data.saved_passkey)
            {
                printf(KYEL"Delete the userdat.bin file in data folder and re-initialise.\n");
                Error_message(2);
            }
            else
            {
                break;
            }
        }

        scanf("%[^\n]%*c", &passw);
        if(passw[0] == '\0' && count == 0) {printf("blank password triggered.\n");}
        count++;

    } while (strcmp(user_data.saved_password, passw) && count <= 6);

    /** Processing sections **/

    printf(KCYN"\nSelect your process from the following:\n"); 
    printf("0 for encrypt:\n\tYou can choose to encrypt for specific person or keep it general.\n"); 
    printf("1 for decrypt:\n\tAutomatic decryption, however words may be gibberish if the message was not intended for you.\n");
    printf("2 for user contact card:\n\tTo generate your user card, you need to share your card for others to send you encrypted message.\n");
    printf("3 for userdata update:\n\tTo update all data, Previous known contacts must be shared the new cards for further communication.\n"); 
    printf("4 for contacts update:\n\tAdd contacts. To add, you need their contact cards.\n");
    printf("Process number: ");
    scanf("%c%*c", &ans);
    //ans = '2';

    printf("\n");

    // invalid options end the program
    if(ans > '4' || ans < '0') { Error_message(2);}

    // process chosen
    if(ans == '0')
    {
        printf(KCYN"\nEnter the name of your message file (max 20 characters long).\nThe file should be present in the messages folder: \n");
        scanf("%[^\n]%*c", &filename);

        // changes to create the filepath
        strcat(filepath, message_folder);
        strcat(filename, ".txt");
        strcat(filepath, filename);

        printf("\n");

        if(file_not_exists(filepath)) 
        {
            printf(KYEL"%s -> No such files found in messages folder.\n", filename); 
            Error_message(2);}

        // number of contacts available and chosen person
        int num_users = 0;
        sender one_person;

        num_users = display_contacts(&contacts);
        if(num_users > 0)
        {
            printf(KCYN"Enter the number of the person you want to send the encrypted file (0 for no security): \n");
            scanf("%d%*c", &contact_num);
        }else if(num_users == 0) { contact_num = 0; }

        // Set common key used for the encryption
        if(contact_num < 0 || contact_num > num_users)
        {
            if(num_users < 0) {printf(KYEL"Error in reading contact file.\n"); Error_message(0); }
            contact_num = -1;
        }
        else if(contact_num == 0)
        {
            sender_user_common_key = 0;
        }
        else
        {
            one_person = contacts[contact_num - 1];
            sender_user_common_key = a_pow_b_mod_P((int) one_person.saved_public_key, user_data.saved_private_key, P_DHA);
        }
        printf(KGRN"Encrypting message for %s, Public key: %lld.\nCommon key established: %lld\n",one_person.saved_name, one_person.saved_public_key,sender_user_common_key);

        free_contacts(contacts);
        if(contact_num < 0) { Error_message(1); }

        // load the dictionary only if the file exists and receivers contact is available.
        if(dict_Loader(hash_table))
        {
            Error_message(1);
        }

        // number of lines present in the message
        num_lines = message_Loader(filepath, &message_array);

        // Error in loading the message
        if(num_lines == 0)
        {
            dict_Unloader(hash_table);
            Error_message(2);
        }
        else if(num_lines < 0)
        {
            dict_Unloader(hash_table);
            message_Unloader(&message_array, (-1 * num_lines - 1));
            Error_message(0);
        }

        printf(KBLU"Contents of %s file for encrypting:\n\n", filename);
        num_pixel = txt_to_rgb(message_array, num_lines, &val_image, hash_table, ans);

        //remove filename's extension
        filename[strlen(filename) - 4] = '\0';

        message_Unloader(&message_array, num_lines);
        dict_Unloader(hash_table);

        if(num_pixel < 0)
        {
            Error_message(0);
        }

        image_square = babylon_sqrt(num_pixel);
        //printf("%d\n", image_square);
    }
    else if(ans == '1')
    {
        printf(KCYN"\nEnter the name of your image file (max 20 characters long).\nThe image should be present in the decrypt folder: \n");
        //strcpy(filename, "output"); 
        scanf("%[^\n]%*c", &filename);

        strcat(filepath, decrypt_folder);
        strcat(filename, ".png");
        strcat(filepath, filename);

        printf("\n");

        if(file_not_exists(filepath)) 
        {
            printf(KYEL"%s -> No such files found in decrypt folder.\n", filename); 
            Error_message(2);
        }
    }
    else if(ans == '2')
    {
        encode_key_to_start(&val_image, ans);

        one_pixel *end_pixel = (one_pixel *)malloc(sizeof(one_pixel));
        {
            end_pixel->red = control_start_end_pixel;
            end_pixel->green = 0;
            end_pixel->blue = 91;
            end_pixel->next_pixel = NULL;
        }
        val_image.next_pixel = end_pixel;

        num_lines = add_identity(end_pixel, hash_table);
        image_square = babylon_sqrt(num_lines);

        if(num_lines < 0)
        {
            free_image_data(&val_image);
            Error_message(0);
        }

        printf("%d %d are the values.\n",num_lines, image_square);
    }
    else if(ans == '3')
    {
        FILE *newfpw;
        int private_key = 0;
        printf(KCYN"Enter your updated name (25 characters): ");
        scanf("%[^\n]%*c", user_data.saved_username);
        printf("Enter your updated password (25 characters): ");
        scanf("%[^\n]%*c", user_data.saved_password);
        printf("Enter a number for new private key generation (0 < Pri < 1000): ");
        scanf("%d%*c", &private_key);

        if(user_data.saved_username[0] == '\0' || user_data.saved_password[0] == '\0' || private_key <= 0)
        {
            Error_message(1);
        }
        else if(private_key >= 1000)
        {
            printf(KYEL"Number should be less than 1000.\n");
            Error_message(1);
        }

        user_data.saved_passkey = hash_word(user_data.saved_password, 0);

        // only for getting the current time
        struct tm* ptr;
        time_t lt;
        lt = time(NULL);
        ptr = localtime(&lt);
        char *user_date = asctime(ptr);

        // using the asctime() function to get the time of initialisation.
        printf(KMAG"\n%s updated account on %s\n", user_data.saved_username, user_date);

        user_data.saved_private_key = ((private_key << 2) ^ (user_data.saved_passkey % K_limit)) >> 2 | (hash_word(user_date, 0) % Z_limit + 1);

        user_data.saved_name_hash = hash_word(user_data.saved_username, Nearest_prime(hash_word(user_date, 0)));
        user_data.saved_public_key =  a_pow_b_mod_P(G_DHA, user_data.saved_private_key, P_DHA);

        if(user_data.saved_public_key < 0) {printf(KYEL"Update unsuccessful.\nPlease change the values of private key.\n"); Error_message(0);}

        // Update the user data file
        newfpw = fopen(data_file, "wb");
        if(fwrite(&user_data, sizeof(struct user), 1, newfpw))
        {
            fclose(newfpw);
        }
        else{printf(KYEL"Update unsuccessful.\n"); Error_message(0);}

        printf(KGRN"\nYour new passkey is %d. Create a new user card and send to all contacts.\n", user_data.saved_passkey);

        fclose(newfpw);
    }
    else if(ans == '4')
    {
        printf(KCYN"\nEnter the name of your image file (max 20 characters long with -card).\nThe image card should be present in the data folder: \n");
        //strcpy(filename, "output"); 
        scanf("%[^\n]%*c", &filename);

        strcat(filepath, data_folder);
        strcat(filename, ".png");
        strcat(filepath, filename);

        printf("\n");

        if(file_not_exists(filepath))
        {
            printf(KYEL"No user card found named %s in data folder.\n", filename);
            Error_message(1);
        }
        else if(file_not_exists(contact_file))
        {
            printf(KWHT"Do not delete the contacts file.\nAll previous contacts lost.\n");
            FILE* fp = fopen(contact_file, "wb");
            fclose(fp);
        }
    }

    /** Image processing section **/

    // Initialize FreeImage library
    FreeImage_Initialise(1);

    if(ans == '0' || ans == '2')
    {
        // Create a new image with dimensions for n x n
        FIBITMAP *image = FreeImage_Allocate(image_square, image_square, 24, 8, 8, 8);

        // Set pixel colors
        RGBQUAD color;
        one_pixel *temp = &val_image;


        for(int r = 0; r < image_square; r++) {
            for(int c = 0; c < image_square; c++) {
                if(temp != NULL)
                {
                    color.rgbRed = temp -> red;
                    color.rgbGreen = temp -> green;
                    color.rgbBlue = temp -> blue;
                    temp = temp -> next_pixel;
                }
                else
                {
                    color.rgbRed = control_border_pixel;   // Red
                    color.rgbGreen = 0;   // Green
                    color.rgbBlue = 255;    // Blue
                }

                FreeImage_SetPixelColor(image, c, r, &color);
            }
        }

        // Set compression type to PNG with zero compression
        // Save the image
        if(ans == '2')
        {
            //filename
            filename[0] = '\0';
            char temp_name[temp_name_length];
            for(int tn = 0; tn < temp_name_length - 1; tn++) { temp_name[tn] = user_data.saved_username[tn];}
            temp_name[temp_name_length] = '\0';
            strcpy(filename, strcat(filename,temp_name));

            // Set compression type to PNG with zero compression
            // Save the image
            FreeImage_Save(FIF_PNG, image, strcat(filename,"-card.png"), PNG_Z_NO_COMPRESSION);
        }
        else
        {
            FreeImage_Save(FIF_PNG, image, strcat(filename,".png"), PNG_Z_NO_COMPRESSION);
        }

        // Unload the image
        FreeImage_Unload(image);
    }
    else if(ans == '1' || ans == '4')
    {
        // Load the PNG image
        FIBITMAP* de_image = FreeImage_Load(FIF_PNG, filepath, 0);

        // Check if the image was loaded successfully
        if (de_image) {
            // Get image dimensions
            unsigned width = FreeImage_GetWidth(de_image);
            unsigned height = FreeImage_GetHeight(de_image);

            RGBQUAD pixelColor;

            one_pixel *temp_pixel = &val_image;

            // Iterate over the image pixels
            for (unsigned y = 0; y < height; y++) {
                for (unsigned x = 0; x < width; x++) {
                    // Get the color of the current pixel
                    if (FreeImage_GetPixelColor(de_image, x, y, &pixelColor)) {
                        // Print the RGB values of the pixel
                        //printf(KBLU"Pixel (%u, %u): [%u %u %u]\n", x, y, pixelColor.rgbRed, pixelColor.rgbGreen, pixelColor.rgbBlue);
                        
                        if(pixelColor.rgbRed != control_border_pixel && png_to_pixel(pixelColor.rgbRed, pixelColor.rgbGreen, pixelColor.rgbBlue, temp_pixel))
                        { temp_pixel = temp_pixel -> next_pixel; }
                        else if(pixelColor.rgbRed != control_border_pixel)
                        {
                            imgflag = 1;
                            printf(KYEL"Image not accesible.\nMessage cannot be decrypted.\n");
                            break;
                        }
                    }
                }
                if(imgflag) { break; }
            }

            // Unload the image when done
            FreeImage_Unload(de_image);

            //remove filename's extension
            filename[strlen(filename) - 4] = '\0';


            // load the dictionary file after decoding the key
            if(ans == '1' && decode_key_from_start(val_image.next_pixel) && dict_Loader(hash_table))
            {
                free_image_data(&val_image);
                Error_message(1);
            }

            // make the text file
            if(!imgflag)
            {
                if(ans == '1')
                { 
                    pixel_dehash(&val_image, hash_table, strcat(filename, ".txt")); 
                }
                else
                {
                    if(!add_contact(val_image.next_pixel))
                    {
                        imgflag = 1;
                    }
                }
            }
        } 
        else { printf(KYEL"Failed to load the image.\n"); imgflag = 1;}
    }

    // Deinitialize FreeImage library
    FreeImage_DeInitialise();

    if(ans == '1') { dict_Unloader(hash_table); }
    if(ans != '3') { free_image_data(&val_image); }

    if(imgflag){ Error_message(0); }

    printf(KGRN"\n!Program terminated successfully!\nPress enter to exit.");
    getchar();

    return 0;
}

// Function Definitions

// sqrt round off for sqare base of image
int babylon_sqrt(int n)
{
    float ans = 0.0001;
    float x1 = 0, x2 = 0;

    x1 = (n + 1) / 2.0;
    x2 = (x1 + n / x1) / 2.0;
    while(x1 - x2 >= ans && x1 - x2 > 0)
    {
        x1 = (x1 + n / x1) / 2.0;
        x2 = (x1 + n / x1) / 2.0;
    }

    int val = x2;

    if(val - x2 >= 0)
    {
        return val;
    }
    else
    {
        return (val + 1);
    }
} 

// Find nearest Prime of n
int Nearest_prime(int n)
{
    bool yes_prime = true;
    int num = n;
    int root_n = 1;

    do
    {
        yes_prime = true;
        root_n = babylon_sqrt(num);

        if(num % 2 == 0)
        {
            yes_prime = false;
        }
        else
        {
            for(int i = 3; i < root_n + 1; i += 2)
            {
                if(num % i == 0)
                {
                    yes_prime = false;
                    break;
                }   
            }
        }

        if(yes_prime)
        {
            break;
        }
        else
        {
            num--;
        }
    } while (num);

    return num;
}

// a^b % P computation
long long int a_pow_b_mod_P(int a, int b, int P)
{
    long long int r = 1;
    long long int x = a % P;

    int n_power = b;

    while(n_power > 0)
    {
        if(n_power % 2 == 1)
        {
            r = (r * x) % P;
        }

        x = (x * x) % P;
        n_power = n_power / 2;
    }

    if(r < 0)
    {
        printf(KYEL"Computation not possible.\n");
    }

    return r;
}

//error message
void Error_message(int i)
{
    printf(KRED"\n***Error***\n");
    //switch case for different messages
    switch (i)
    {
    case 1:
        printf("Files or data failed to load.");
        break;
    case 2:
        printf("Invalid User input.\nPlease use proper inputs.");
        break;
    default:
        printf("Unknow cause of error in program.");
        break;
    }

    printf("\n\nPlease close other programs and rerun.\nProgram is terminated. Press enter to exit.\n***Error***");
    getchar();

    //exit out as program under control
    exit(0);
}

// whether a file exists or not
bool file_not_exists(char *filepath)
{
    bool val;
    FILE *fp;
    char file_exe[5]; // for checking extension

    int len = strlen(filepath);
    for(int i = 0; i < 5; i++)
    {
        file_exe[i] = filepath[len + i - 4];
    }

    if(!strcmp(file_exe, ".bin"))       // binary file check
    {
        fp = fopen(filepath, "rb");
    }
    else if(!strcmp(file_exe, ".txt"))  // txt file check
    {
        fp = fopen(filepath, "r");
    }
    else if(!strcmp(file_exe, ".png"))  // png file check
    {
        fp = fopen(filepath, "r");
    }
    else
    {
        printf(KYEL"\nInvalid file type %s.\nMessage files should be in .txt\nImage files should be in .png.\n", file_exe);
        Error_message(1);
    }

    if(fp == NULL)
    {
        val = true;
    }
    else
    {
        val = false;
    }

    fclose(fp);
    return val;
}

// check if user data file is setup or not
bool datafile_setup(char *path)
{
    int private_key;
    FILE *fpr = fopen(path, "rb");
    FILE *fpw, *fcw;

    if(fpr == NULL)
    {
        fclose(fpr);       

        printf(KMAG"Enter your name (25 characters): ");
        scanf("%[^\n]%*c", user_data.saved_username);
        printf("Enter your password (25 characters): ");
        scanf("%[^\n]%*c", user_data.saved_password);
        printf("Enter a number for private key generation (0 < Pri < 1000): ");
        scanf("%d%*c", &private_key);

        if(user_data.saved_username[0] == '\0' || user_data.saved_password[0] == '\0' || private_key <= 0)
        {
            Error_message(1);
        }
        else if(private_key >= 1000)
        {
            printf("Number should be less than 1000.\n");
            Error_message(1);
        }

        user_data.saved_passkey = hash_word(user_data.saved_password, 0);

        // only for getting the current time
        struct tm* ptr;
        time_t lt;
        lt = time(NULL);
        ptr = localtime(&lt);
        char *user_date = asctime(ptr);

        // using the asctime() function to get the time of initialisation.
        printf("\n%s created account on %s\n", user_data.saved_username, user_date);

        user_data.saved_private_key = ((private_key << 2) ^ (user_data.saved_passkey % K_limit)) >> 2 | (hash_word(user_date, 0) % Z_limit + 1);

        user_data.saved_name_hash = hash_word(user_data.saved_username, Nearest_prime(hash_word(user_date, 0)));
        user_data.saved_public_key =  a_pow_b_mod_P(G_DHA, user_data.saved_private_key, P_DHA);

        if(user_data.saved_public_key < 0) {printf(KYEL"Unable to create profile.\nPlease change the values of private key.\n"); Error_message(0);}

        printf("\nname hash %d private key %d public key %lld\n", user_data.saved_name_hash, user_data.saved_private_key, user_data.saved_public_key);

        fpw = fopen(path, "wb");
        if(fpw == NULL)
        {
            fclose(fpw);
            Error_message(1);
        }

        fcw  = fopen(contact_file, "rb");
        if(fcw == NULL){ fcw = fopen(contact_file, "wb"); }
        fclose(fcw);  

        if(fwrite(&user_data, sizeof(struct user), 1, fpw))
        {
            fclose(fpw);
        }
        else{Error_message(0);}
        return true;
    }

    fread(&user_data, sizeof(struct user), 1, fpr);
    fclose(fpr);

    return false;
}

// Displays saved user contacts 
int display_contacts(sender **contact_info)
{
    long int pos;
    int number_con = 0;
    FILE *fp = fopen(contact_file, "rb");
    if(fp == NULL)
    {
        printf(KYEL"Contacts file corrupted or not present.\n");
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    pos = ftell(fp);
    number_con = pos / sizeof(sender); 

    *contact_info = (sender *)malloc(number_con * sizeof(sender));
    if(contact_info == NULL){fclose(fp); Error_message(0); }

    if(pos < 1 || number_con < 1) { printf(KWHT"No contacts found.\nSwitching to General encryption.\n"); fclose(fp); return 0; }
    fseek(fp, 0, SEEK_SET);

    printf("%d %d %ld\n", pos, number_con, sizeof(sender));
    for(int i = 0; i < number_con; i++)
    {
        sender person;
        fread(&person, sizeof(sender), 1, fp);
        printf(KGRN"%d)Name: %s\nName_hash: %d\n", i + 1, (person.saved_name), (person.saved_name_hash));
        printf("public key: %lld\n", (person.saved_public_key));
        printf("\n");
        (*contact_info)[i] = person;
    }

    fclose(fp);
    return number_con;
}

// Adds a single contact of user
bool add_contact(one_pixel *contact_pixel)
{
    char c = '\0', name_holder[string_size];
    name_holder[0] = '\0';
    sender new_person;
    int counter = 0, len = 0;
    
    one_pixel *temp = contact_pixel;
    while(temp != NULL)
    {
        if(temp -> next_pixel == NULL)
        {
            new_person.saved_name_hash = (temp->red * control_start_end_pixel * control_start_end_pixel + temp->green * control_start_end_pixel + temp->blue);
        }
        else
        {
            if(temp -> red == control_start_end_pixel)
            {
                if(counter == 0)
                {
                    new_person.saved_public_key = (temp->green * control_start_end_pixel + temp->blue);
                }
                counter++;
            }
            else if(temp->red == control_nascii_encoding && counter == 2)
            {
                c = (char)(temp->green);
                name_holder[len] = c; 
                if(temp->blue > 0)
                {
                    len++;
                    c = (char)(temp->blue);
                    name_holder[len] = c;                
                }
                len++;
            }

            if(counter == 3)
            {
                name_holder[len] = '\0';
            }
        }
        temp = temp -> next_pixel;
    }

    if(len > string_size || new_person.saved_public_key < 0)
    {
        printf(KYEL"Error in reading card.\n");
        return false;
    }

    strcpy(new_person.saved_name, name_holder);

    FILE *fp = fopen(contact_file, "ab");
    if(fp == NULL){ fclose(fp); return false; }

    fseek(fp, 0, SEEK_END);

    if(!fwrite(&new_person, sizeof(sender), 1, fp))
    { fclose(fp); return false; }

    printf(KGRN"\nNew contact added.\n");
    printf("Name: %s\nPublic_key: %lld\nName_hash: %d\n", new_person.saved_name, new_person.saved_public_key, new_person.saved_name_hash);

    fclose(fp);
    return true;
}

// adds identity after a specific pixel segment
int add_identity(one_pixel *pdata,node_head hash_table_address[hash_bucket_row][hash_bucket_column])
{
    int count = 1;
    one_pixel* temp = hash_string(user_data.saved_username, pdata, 0, hash_table_address, 1);

    // save name hash in base 255
    one_pixel* endnum = (one_pixel *)malloc(sizeof(one_pixel));
    if(endnum == NULL){ return -1; }
    {
        endnum->red = ((user_data.saved_name_hash - user_data.saved_name_hash % (control_start_end_pixel * control_start_end_pixel)) / (control_start_end_pixel * control_start_end_pixel));
        endnum->green = ((user_data.saved_name_hash - user_data.saved_name_hash % control_start_end_pixel) / control_start_end_pixel);
        endnum->blue = (user_data.saved_name_hash % control_start_end_pixel);
        endnum->next_pixel = NULL;
    }

    one_pixel *end_pixel = (one_pixel *)malloc(sizeof(one_pixel));
    if(end_pixel == NULL){ return -1; }
    {
        end_pixel->red = control_start_end_pixel;
        end_pixel->green = 0;
        end_pixel->blue = 91;
        end_pixel->next_pixel = endnum;
    }
    temp -> next_pixel = end_pixel;

    temp = pdata;
    while(temp != NULL)
    {
        count++;
        //printf("%d) %p [%d %d %d] -> %p\n", count, temp,temp->red,temp->green,temp->blue,temp->next_pixel);
        temp = temp ->next_pixel;
    }

    return count;
}

/*
for mapping 2 alphabet letters
it starts from 98 goes to 123
*/

// hash function !* Important *! 
int hash_word(char *string, int offset)
{
    int hash_val = 0;
    int l = strlen(string);
    hash_val = 2 * l;
    for(int i = 0; i < l; i++)
    {
        int a = (int) string[i] - l;
        switch (i % 5)
        {        
        case 1:
        case 3:
            hash_val = (((hash_val >> 2) + a) | offset);
            break;
        case 2:
            hash_val = (hash_val | (a * 5)) - offset / 5;
            break;
        case 0:
            hash_val = ((hash_val + 10) | 2);  
            break;  
        default:
            hash_val = hash_val + 250 + 2 * offset;
            break;
        }
         
        hash_val = hash_val % max_hash_val;
    }
    if(hash_val < 0) {hash_val *= -1;}

    return hash_val;
} 

/* for any sentences or character given, return a pixel value*/
one_pixel *hash_string(char *string, one_pixel *curr_pixel, short unsigned int val, node_head hash_table_address[hash_bucket_row][hash_bucket_column], short unsigned int name_encode)
{
    int len = 0, found = 1;
    short unsigned int i = 0, j = 0, offset = 0, looped = 1, collision = 1;

    int jmap_start = hash_word("aa", 0);
    int jmap_end = hash_word("zz", 0);
    int buffer_columns_j = (hash_bucket_column - 1) / (jmap_end - jmap_start);

    char mod_word[max_string_size], min_word[3];
    mod_word[0] = '\0';

    one_pixel *n_pixel = (one_pixel *) malloc(sizeof(one_pixel)), *tail;
    if(n_pixel == NULL) { return NULL; }

    {
        n_pixel -> red = 0;
        n_pixel -> green = 0;
        n_pixel -> blue = 0;
        n_pixel -> next_pixel = NULL;
    }

    strcpy(mod_word, string);
    len = strlen(mod_word);
    mod_word[len] = '\0';

    if(val && !name_encode)
    {
        switch (len)
        {
        case 1:
            // single channel encoding
            {
                n_pixel -> red = control_single_encoding;
                n_pixel -> green = (int)mod_word[0];
                n_pixel -> blue = (int)mod_word[0];
            }
            break;
        case 2:
            // double channel encoding
            {
                n_pixel -> red = control_double_encoding;
                n_pixel -> green = (int)(mod_word[0]);
                n_pixel -> blue = (int)(mod_word[1]);
            }
            break;        
        default:
            // general encoding when word is found
            {
                //using random hash
                i = hash_word(mod_word, (int) sender_user_common_key) % hash_bucket_row;

                {min_word[0] = mod_word[0]; min_word[1] = mod_word[1]; min_word[2] = '\0';}
                j = buffer_columns_j * (hash_word(min_word, 0) - jmap_start); 
                j = j % hash_bucket_column;

                node *temp = hash_table_address[i][j].start;
                while (temp != NULL)
                {
                    if(!strcmp(temp -> word, mod_word))
                    {
                        n_pixel -> red = i;
                        n_pixel -> green = j;
                        n_pixel -> blue = collision;
                        //printf("%s [%d %d %d]\n", mod_word, i, j, collision);
                        found = 0;
                        break;
                    }
                    temp = temp -> next;
                    collision++;
                }
                
                if(j >= (buffer_columns_j + 2) && found)
                {
                    for(offset = 1; offset < buffer_columns_j; offset++)
                    {
                        collision = 1;
                        temp = hash_table_address[i][j - offset].start;
                        while (temp != NULL)
                        {
                            if(!strcmp(temp -> word, mod_word))
                            {
                                n_pixel -> red = i;
                                n_pixel -> green = j - offset;
                                n_pixel -> blue = collision;
                                found = 0;
                                //printf("%s [%d %d %d]\n", mod_word, i, j - offset, collision);
                                break;
                            }
                            temp = temp -> next;
                            collision++;
                        }
                        if(!found) { break; }
                    }
                }

                if(j <= hash_bucket_column - (buffer_columns_j + 2) && found)
                {
                    for(offset = 1; offset < buffer_columns_j; offset++)
                    {
                        collision = 1;
                        temp = hash_table_address[i][j + offset].start;
                        while (temp != NULL)
                        {
                            if(!strcmp(temp -> word, mod_word))
                            {
                                n_pixel -> red = i;
                                n_pixel -> green = j + offset;
                                n_pixel -> blue = collision;
                                //printf("%s [%d %d %d]\n", mod_word, i, j + offset, collision);
                                found = 0;
                                break;
                            }
                            temp = temp -> next;
                            collision++;
                        }
                        if(!found) { break; }
                    }
                }
            }
            break;
        }
    }
    else if(!name_encode)
    {
        // remove 1 left blank space if there
        if(len >= 2 && mod_word[0] == ' ')
        {
            len--;
            for(int b = 0; b < len; b++)
            {
                mod_word[b] = mod_word[b + 1];
            }
        }

        // remove 1 right blank space if there
        if(len >= 2 && mod_word[len - 1] == ' ')
        {
            mod_word[len - 1] = '\0';
            len--;
        }

        // encode the characters in rgb
        if(len == 1)
        {
            // single channel encoding
            {
                n_pixel -> red = control_single_encoding;
                n_pixel -> green = (int)mod_word[0];
                n_pixel -> blue = (int)mod_word[0];
            }

            //printf("%c [%d %d %d]\n", mod_word[strlen(mod_word) - 1], (n_pixel->red), (n_pixel->green), (n_pixel->blue));
        }
        else if (len == 2)
        {
            // double channel encoding
            {
                n_pixel -> red = control_double_encoding;
                n_pixel -> green = (int)(mod_word[0]);
                n_pixel -> blue = (int)(mod_word[1]);
            }            

            //printf("%c%c [%d %d %d]\n", mod_word[strlen(mod_word) - 2],mod_word[strlen(mod_word) - 1], (n_pixel->red), (n_pixel->green), (n_pixel->blue));
        }
    }
        if(((val && found) || !val) && len >= 3 || name_encode)
        {
            // general encoding non ascii control
            while(strlen(mod_word) > 2)
            {
                one_pixel *temp = (one_pixel *)malloc(sizeof(one_pixel));
                if(temp == NULL) { return NULL; }
                {
                    temp -> red = control_nascii_encoding;
                    temp -> green = (int)(mod_word[strlen(mod_word) - 2]);
                    temp -> blue = (int)(mod_word[strlen(mod_word) - 1]);
                }

                //printf("%c%c [%d %d %d]\n", mod_word[strlen(mod_word) - 2],mod_word[strlen(mod_word) - 1], (temp->red), (temp->green), (temp->blue));

                if(looped) { tail = temp; looped = 0; }

                temp -> next_pixel = n_pixel -> next_pixel;
                n_pixel -> next_pixel = temp;
                
                mod_word[strlen(mod_word) - 2] = '\0';
            }

            if(strlen(mod_word) == 2)
            {
                n_pixel -> red = control_nascii_encoding;
                n_pixel -> green = (int)(mod_word[0]);
                n_pixel -> blue = (int)(mod_word[1]);
                //printf("%c%c [%d %d %d]\n", mod_word[strlen(mod_word) - 2],mod_word[strlen(mod_word) - 1], (n_pixel->red), (n_pixel->green), (n_pixel->blue));
            }
            else
            {
                n_pixel -> red = control_nascii_encoding;
                n_pixel -> green = (int)mod_word[0];
                n_pixel -> blue = (int)('\0');
                //printf("%c [%d %d %d]\n", mod_word[strlen(mod_word) - 2], (n_pixel->red), (n_pixel->green), (n_pixel->blue));
            }
        }

    curr_pixel -> next_pixel = n_pixel;
    //printf("%s->%p\n",mod_word, n_pixel);
    
    if(looped)
    {
        return n_pixel;
    }
    else
    {
        return tail;
    }
}

// Add corresponding word nodes in the dictionary table
void node_adder(node_head hash_table_address[hash_bucket_row][hash_bucket_column], char *string, unsigned short int i, unsigned short int j)
{
    node *node_ptr = (node *)malloc(sizeof(node));
    if(node_ptr == NULL){ Error_message(0); }
    (node_ptr -> word) = string;
    (node_ptr ->next) = NULL;

    if(hash_table_address[i][j].start == NULL)
    {
        hash_table_address[i][j].start = node_ptr;
    }
    else
    {
        (node_ptr -> next) = hash_table_address[i][j].start;
        hash_table_address[i][j].start = node_ptr;
    }

    hash_table_address[i][j].collision_count++;
}

// load the dictionary in memory
bool dict_Loader(node_head hash_table_address[hash_bucket_row][hash_bucket_column])
{

    if(file_not_exists(word_dictionary)){ printf(KYEL"\nDictionary not found.\n word.txt should be in data folder."); return 1;}

    int jmap_start = hash_word("aa", 0);
    int jmap_end = hash_word("zz", 0);
    int buffer_columns_j = (hash_bucket_column - 1) / (jmap_end - jmap_start);

    short unsigned int max_len = 0, i = 0, j = 0, offset = 0;
    short unsigned int val = 0;

    int count = 0;

    char c, word[max_string_size], min_word[3];
    min_word[2] = '\0';
    FILE *fp = fopen(word_dictionary, "r");

    while((c = fgetc(fp)) != EOF)
    {
        word[max_len] = c;
        max_len++;

        if(!isascii(c) && c != EOF || max_len > max_string_size)
        {
            printf(KYEL"\nDictionary corrupted.\n");
            fclose(fp);
            return 1;
        }

        if(c == '\n')
        {
            if(max_len >= 4)
            {
                word[max_len - 1] = '\0';
                char *word_holder = (char *) malloc((max_len + 1) * sizeof(char));
                if(word_holder == NULL){ fclose(fp); Error_message(0);}
                strcpy(word_holder, word);

                // assign the first two letter for column mapping
                { min_word[0] = word_holder[0]; min_word[1] = word_holder[1]; }

                //using random hash
                i = hash_word(word_holder, (int) sender_user_common_key) % hash_bucket_row;
                j = (hash_bucket_column - 1) * (hash_word(min_word, 0) - jmap_start) / (jmap_end - jmap_start); 
                j = j % hash_bucket_column;

                if(hash_table_address[i][j].collision_count <= hash_bucket_column - 1)
                {
                    node_adder(hash_table_address, word_holder, i, j);
                    count++;

                }
                else
                {
                    if(j >= (buffer_columns_j + 2) && !val)
                    {
                        for(offset = 1; offset < buffer_columns_j; offset++)
                        {
                            if(hash_table_address[i][j - offset].collision_count <= hash_bucket_column - 1)
                            {
                                count++;
                                node_adder(hash_table_address, word_holder, i, j - offset);
                                val = 1;
                                break;                               
                            }
                        }
                    }
                    if(j <= hash_bucket_column - (buffer_columns_j + 2) && !val)
                    {
                        for(offset = 1; offset < buffer_columns_j; offset++)
                        {
                            if(hash_table_address[i][j + offset].collision_count <= hash_bucket_column - 1)
                            {
                                count++;
                                node_adder(hash_table_address, word_holder, i, j + offset);
                                val = 1;
                                break;                               
                            }
                        }
                    }
                    if(!val)
                    {
                        free(word_holder);
                    }
                }

            }
            val = 0;
            max_len = 0;
        }
    }

    printf(KGRN"\nDictionary ready. %d words loaded.\n", count);
    fclose(fp);
    return 0;
}

// Read the message for encryption and load it in memory
int message_Loader(char *filepath, char ***msg_array)
{
    int count_lines = 1, max_len = 0, lines_i = 0;
    char c, c_line[line_length];

    FILE *fp = fopen(filepath, "r");
    do
    {
        c = fgetc(fp);
        max_len++;

        if(!isascii(c) && c != EOF)
        {
            printf(KYEL"non-ascii Character found.\nEncryption not supported.\n");
            count_lines = 0;
            break;
        }

        if(max_len > line_length)
        {
            printf(KYEL"Lines exceed the input size.\n");
            count_lines = 0;
            break; 
        }

        if(c == '\n')
        {
            count_lines++;
            max_len = 0;
        }
    } while (c != EOF);

    if(max_len == 1 && count_lines == 1){ printf(KYEL"\nFile is empty.\n"); count_lines = 0;}

    if(count_lines == 0){ fclose(fp); return count_lines;}

    if(max_len == 1 && count_lines > 1){ count_lines--;}


    // initialise the 2d array which contains all lines
    *msg_array = (char **) malloc((count_lines) * sizeof(char *));
    if(*msg_array == NULL) { fclose(fp); count_lines = 0; return count_lines;}

    fseek(fp, 0, SEEK_SET);

    while(lines_i < count_lines)
    {
        if(fgets(c_line, line_length ,fp) == NULL) {break;}

        max_len = strlen(c_line);

        for(int l = 0; l < max_len; l++)
        {
            if(isalpha(c_line[l])) {c_line[l] = tolower(c_line[l]);}
        }

        char *line = (char *) malloc((max_len + 2) * sizeof(char));
        if(line == NULL) { fclose(fp); return (-1 * (lines_i + 1));}

        strcpy(line, c_line);
        (*msg_array)[lines_i] = line;
        lines_i++;
    }
    
    fclose(fp);
    return count_lines;    
}

// Add the public key used for encryption
bool encode_key_to_start(one_pixel *val, char c)
{
    if(val == NULL)
    {
        return false;
    }

    if((sender_user_common_key != 0 && c == '0') || c == '2')
    {
        // sent public key
        printf("%lld is the key sent.\n", user_data.saved_public_key);

        // assign the key (key < 9973)
        val->green = (unsigned short int) ((user_data.saved_public_key - user_data.saved_public_key % control_start_end_pixel) / control_start_end_pixel);
        val->blue = (unsigned short int) (user_data.saved_public_key % control_start_end_pixel);
    }
    else
    {
        val->green = 0;
        val->blue = 0;
    }

    return true;
}

// Read the key used for encryption
bool decode_key_from_start(one_pixel *val)
{
    // find the public key of the other person
    sender_user_common_key = (val->green * control_start_end_pixel + val->blue);
    printf("\n\n%lld is the key received.\n\n", sender_user_common_key);

    sender_user_common_key = a_pow_b_mod_P((int) sender_user_common_key, user_data.saved_private_key, P_DHA);

    if(sender_user_common_key < 0)
    {
        printf(KYEL"Error in reading key.\n");
        return false;
    }

    return true;
}

// Convert the text from message loaded in memory to pixels && add user name hash
int txt_to_rgb(char **msg_array, int size, one_pixel *start_pixel, node_head hash_table_address[hash_bucket_row][hash_bucket_column], char ans_c)
{
    char c,char_holder[max_string_size], word_holder[max_string_size];
    int len = 0, w_len = 0, c_len = 0, number_pixels = 0;

    int count = 0;

    // additional basic ceaser encryption
    int ceaser_shift = (sender_user_common_key % control_start_end_pixel);

    one_pixel *temp = start_pixel;

    word_holder[0] = char_holder[0] = '\0';

    for(int i = 0; i < size; i++)
    {
        printf("%s",msg_array[i]);
        len = strlen(msg_array[i]);
        for (int j = 0; j < len; j++)
        {
            c = msg_array[i][j];
            if(isalpha(c))
            {
                c_len = strlen(char_holder);
                if(c_len && !(c_len == 1 && char_holder[0] == ' '))
                {
                    temp = hash_string(char_holder, temp, 0, hash_table_address, 0);
                    if(temp == NULL) return -1;
                }
                char_holder[0] = '\0';

                w_len = strlen(word_holder);
                word_holder[w_len] = c;
                word_holder[w_len + 1] = '\0';                
            }
            else
            {
                w_len = strlen(word_holder);
                if(w_len)
                {
                    temp = hash_string(word_holder, temp, 1, hash_table_address, 0);
                    if(temp == NULL) return -1;
                }
                word_holder[0] = '\0';

                c_len = strlen(char_holder);
                char_holder[c_len] = c;
                char_holder[c_len + 1] = '\0'; 
            }
        }
    }

    // check for unhashed parts.
    w_len = strlen(word_holder);
    c_len = strlen(char_holder);

    if(!c_len && w_len)
    {
        temp = hash_string(word_holder, temp, 1, hash_table_address, 0);
        if(temp == NULL) return -1;
    }
    else if(!w_len && c_len)
    {
        temp = hash_string(char_holder, temp, 0, hash_table_address, 0);
        if(temp == NULL) return -1;
    }

    printf("\n");

    one_pixel *end_pixel = (one_pixel *)malloc(sizeof(one_pixel));
    {
        end_pixel -> red = control_start_end_pixel;
        end_pixel -> green = 0;
        end_pixel -> blue = 91;        
        end_pixel -> next_pixel = NULL;
        temp -> next_pixel = end_pixel;
    }

    // encode key to the start of the message
    if(!encode_key_to_start(start_pixel, ans_c))
    {
        printf(KYEL"Error in encoding public key.\n");
        return -1;
    }

    temp = start_pixel;

    while(temp != NULL)
    {
        if(number_pixels != 0 && temp->next_pixel != NULL)
        {
            if(temp->red < ceaser_shift){temp -> red += control_start_end_pixel; }
            temp -> red = (temp -> red - ceaser_shift) % control_start_end_pixel;
        }
        number_pixels++;
        //printf("%d) %p [%d %d %d]\n", number_pixels, temp, temp->red, temp->green, temp->blue);
        temp = temp -> next_pixel;
    }

    return number_pixels;
}

// convert the image pixels to linked pixel data && decrypt from user name hash
bool png_to_pixel(unsigned short int i, unsigned short int j, unsigned short int collision, one_pixel *currpixel)
{
    one_pixel *n_pixel = (one_pixel *) malloc(sizeof(one_pixel));
    if(n_pixel == NULL) { return false; }

    {
        n_pixel -> red = i;
        n_pixel -> green = j;
        n_pixel -> blue = collision;
    }

    n_pixel -> next_pixel = NULL;
    currpixel -> next_pixel = n_pixel;
    return true;
}

// dehash all pixel data
void pixel_dehash(one_pixel *image_data, node_head hash_table_address[hash_bucket_row][hash_bucket_column], char *filesavepath)
{
    FILE *fp = fopen(filesavepath, "w");
    node *temp_node;
    int collision = 1, count = 0;

    // additional basic ceaser encryption
    int ceaser_deshift = (sender_user_common_key % control_start_end_pixel);

    bool add_space = true;
    char word_holder[max_string_size], next_c = '\0', next_c1 = '\0', curr_c0 = '\0', curr_c1 = '\0';
    word_holder[0] = '\0';

    one_pixel *temp = image_data -> next_pixel;

    while(temp != NULL)
    {
        printf("%d) %p [%d %d %d] -> %p\n",count, temp, temp->red,temp->green,temp->blue,temp->next_pixel);
        if(count < 2)
        {
            // remove ceaser shift
            if(count != 0 && temp->next_pixel != NULL)
            { temp->red = (temp->red + ceaser_deshift) % control_start_end_pixel; }
        
            switch (temp -> red)
            {
            case control_start_end_pixel:
                count++;

                break;
            case control_single_encoding:
                    fprintf(fp, "%c ", (char) (temp -> green));

                break;
            case control_double_encoding:
                    fprintf(fp, "%c%c ", (char) (temp -> green), (char) (temp -> blue));

                break;     
            case control_nascii_encoding:
                    curr_c0 = (char)((temp)->green);
                    curr_c1 = (char)((temp)->blue);

                    if(curr_c1 == '\0')
                    {
                        word_holder[0] = curr_c0;
                        word_holder[1] = '\0';
                    }
                    else
                    {
                        word_holder[0] = curr_c0;
                        word_holder[1] = curr_c1;    
                        word_holder[2] = '\0';              
                    }

                    if((temp->next_pixel)->red == control_nascii_encoding)
                    {
                        next_c = (char)((temp->next_pixel)->green);
                        if(!(isalpha(curr_c1) ^ isalpha(next_c)) || word_holder[1] == '\0')
                        {
                            fprintf(fp, "%s", word_holder);                    
                        }
                        else
                        {
                            fprintf(fp, "%s ", word_holder);
                        }
                    }
                    else
                    {
                        fprintf(fp, "%s ", word_holder);
                    }

                    word_holder[0] = '\0';

                break;
            default:
                    temp_node = hash_table_address[temp ->red][temp ->green].start;
                    while(collision < (temp -> blue) && temp_node != NULL)
                    {
                        temp_node = temp_node -> next;
                        collision++;
                    }
                    if (temp_node == NULL)
                    {
                        printf(KYEL"dictionary not set properly.\n");
                    }
                    else
                    {
                        strcpy(word_holder, temp_node -> word);

                        next_c = (char)((temp->next_pixel)->green);
                        next_c1 = (char)((temp->next_pixel)->blue);            

                        if(next_c == '.' && ((temp->next_pixel)->red == control_single_encoding) || (next_c1 == '\n' && (temp->next_pixel)->red == control_double_encoding))
                        {
                            fprintf(fp, "%s", word_holder);
                        }
                        else
                        {
                            fprintf(fp, "%s ", word_holder);                
                        }

                        word_holder[0] = '\0';
                        collision = 1;
                    }
                break;
            }
        }

        temp = temp -> next_pixel;
    }

    fclose(fp);
}

// free all stored contacts
void free_contacts(sender *contact_info)
{
    free(contact_info);
}

// free pixel values from memory
void free_image_data(one_pixel *image_data)
{
    one_pixel *temp = image_data;
    temp = temp -> next_pixel;
    while(temp != NULL)
    {
        one_pixel *cur_pixel = temp;
        temp = temp -> next_pixel;
        //printf("%p %p\n", cur_pixel, temp);
        free(cur_pixel);
    }
}

// free message from memory
void message_Unloader(char ***msg_array, int size)
{
    for(int i = 0; i < size; i++)
    {
        free((*msg_array)[size - i - 1]);
    }

    free(*msg_array);
}

// free dictionary nodes from memory
void dict_Unloader(node_head hash_table_address[hash_bucket_row][hash_bucket_column])
{
    for(int x = 0; x < hash_bucket_row; x++)
    {
        for(int y = 0; y < hash_bucket_column; y++)
        {
            node *n_ptr = hash_table_address[x][y].start;
            while (n_ptr != NULL)
            {
                node *temp = n_ptr;
                n_ptr = temp -> next;
                free(temp);
            }
        }
    }
}