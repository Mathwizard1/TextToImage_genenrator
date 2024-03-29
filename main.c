// libraries
#include <stdlib.h> //standard library
#include <stdio.h>  //standard input/output library
#include <ctype.h>  //character classification functions
#include <string.h> //strings
#include <stdbool.h> //booloean macros

#include "FreeImage.h" //image processing

// definitions
#define word_dictionary "data\\words.txt"
#define data_file "data\\userdat.bin"
#define contact_file "data\\contacts.bin"
#define message_folder "messages\\"
#define decrypt_folder "decryptionfile\\"


#define string_size 30
#define max_string_size 45
#define line_length 512

/*
for random mapping of 2 alphabet letters
it starts from 97 goes to 122
*/
#define hash_bucket_row 250
#define hash_bucket_column 256

#define max_hash_val 10000000

// color definitions
#define KRED  "\x1B[31m"   // Error end
#define KGRN  "\x1B[32m"   // success end
#define KYEL  "\x1B[33m"   // Warning cause
#define KBLU  "\x1B[34m"   // message and image contents
#define KMAG  "\x1B[35m"   // login
#define KCYN  "\x1B[36m"   // questions
#define KWHT  "\x1B[37m"   // starting instructions

// structures
typedef struct node
{
    char *word;
    struct node *next;
}node;

typedef struct node_head
{
    short unsigned int collision_count;
    node *start;
}node_head;


typedef struct one_pixel
{
    short unsigned int red;
    short unsigned int green;
    short unsigned int blue;
    struct one_pixel *next_pixel;    
}one_pixel;

/*
    CONTROL CHANNELS
    (251, , ) -> border pixels
    (252, x, x) -> single character encoding
    (253, y, z) -> double character encoding

    (254, y, z) -> non-dictionary words and characters
    (254, , ) -> non ascii encoding  start and end       --> for future upgrades.

    (255, , ) -> start and end of the pixels
*/

#define control_border_pixel 251
#define control_single_encoding 252
#define control_double_encoding 253
#define control_nascii_encoding 254
#define control_start_end_pixel 255

struct user
{
    char saved_username[string_size];
    char saved_password[string_size];
    int saved_passkey;
} user_data;

// global variables
// user_data as a struct

// basic file functions
void Error_message(int i);
bool file_not_exists(char *filepath);
bool datafile_setup(char *path);
int babylon_sqrt(int n);

// mapping and hashing
int hash_word(char *string, int offset);
one_pixel *hash_string(char *string, one_pixel *curr_pixel, short unsigned int val, node_head hash_table_address[hash_bucket_row][hash_bucket_column]);

// loading functions
void node_adder(node_head hash_table_address[hash_bucket_row][hash_bucket_column], char *string, unsigned short int i, unsigned short int j);
bool dict_Loader(node_head hash_table_address[hash_bucket_row][hash_bucket_column]);
int message_Loader(char *filepath, char ***msg_array); 

// pixel functions
int txt_to_rgb(char **msg_arry, int size, one_pixel *image_data, node_head hash_table_address[hash_bucket_row][hash_bucket_column]);
bool png_to_pixel(unsigned short int i, unsigned short int j, unsigned short int collision, one_pixel *currpixel);

// decoding function
void pixel_dehash(one_pixel *image_data, node_head hash_table_address[hash_bucket_row][hash_bucket_column], char *filesavepath);

// free memory functions
void free_image_data(one_pixel *image_data1);
void message_Unloader(char ***msg_array, int size);
void dict_Unloader(node_head hash_table_address[hash_bucket_row][hash_bucket_column]);


//main
int main() {

    // basic variables
    int num_lines;
    short unsigned int count = 0, ans, imgflag = 0;
    char passw[string_size], filename[string_size], filepath[max_string_size];
    passw[0] = filename[0] = filepath[0] = '\0';

    // message array
    char **message_array;

    // initialise the starting pixel
    one_pixel val_image;
    {
        val_image.red = control_start_end_pixel;
        val_image.green = 0;
        val_image.blue = control_start_end_pixel / 10;
        val_image.next_pixel = NULL;
    }

    int num_pixel = 0, image_square = 0;

    // hash table and initialise count
    node_head hash_table[hash_bucket_row][hash_bucket_column];

    printf(KWHT"\n!Welcome to Text to image encryption and decryption.!\n");

    if(datafile_setup(data_file))
    {
        printf(KGRN"\nUser data initialized. Your passkey is %d.\nStore it if you forget the password.\nPlease rerun the program.", user_data.saved_passkey);
        getchar();
        return 0;
    }

    printf(KMAG"\tHello %s,\nEnter your password: ", user_data.saved_username);
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
            scanf("%d", &passkey);
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
        count++;

    } while (strcmp(user_data.saved_password, passw) && count <= 6);

    printf(KCYN"\nYou want to encrypt message or decrypt image?\n 1 for encrypt, 0 for decrypt: ");
    scanf("%hu%*c", &ans);
    if(ans != 0 && ans != 1) {Error_message(2);}

    if(dict_Loader(hash_table))
    {
        Error_message(1);
    }

    if(ans)
    {
        printf(KCYN"\nEnter the name of your message file (max 20 characters long).\nThe file should be present in the messages folder: \n");
        scanf("%[^\n]%*c", &filename);

        strcat(filepath, message_folder);
        strcat(filename, ".txt");
        strcat(filepath, filename);

        printf("\n");

        if(file_not_exists(filepath)) 
        {
            printf(KYEL"%s -> No such files found in messages folder.\n", filename); 
            dict_Unloader(hash_table);
            Error_message(2);}

        num_lines = message_Loader(filepath, &message_array);
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

        printf(KBLU"Contents of %s file for encrypting:\n", filename);
        num_pixel = txt_to_rgb(message_array, num_lines, &val_image, hash_table);

        //remove filename's extension
        filename[strlen(filename) - 4] = '\0';

        message_Unloader(&message_array, num_lines);
        dict_Unloader(hash_table);

        if(num_pixel < 0)
        {
            Error_message(0);
        }

        image_square = babylon_sqrt(num_pixel);
        printf("%d\n", image_square);
    }
    else
    {
        printf(KCYN"\nEnter the name of your image file (max 20 characters long).\nThe image should be present in the decryptionfile folder: \n");
        //strcpy(filename, "output"); 
        scanf("%[^\n]%*c", &filename);

        strcat(filepath, decrypt_folder);
        strcat(filename, ".png");
        strcat(filepath, filename);

        printf("\n");

        if(file_not_exists(filepath)) 
        {
            printf(KYEL"%s -> No such files found in decryption folder.\n", filename); 
            dict_Unloader(hash_table);
            Error_message(2);
        }
    }

    /** Image processing section **/

    // Initialize FreeImage library
    FreeImage_Initialise(1);

    if(ans)
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
        FreeImage_Save(FIF_PNG, image, strcat(filename,".png"), PNG_Z_NO_COMPRESSION);

        // Unload the image
        FreeImage_Unload(image);
    }
    else
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

            // make the text file
            if(!imgflag)
            { pixel_dehash(&val_image, hash_table, strcat(filename, ".txt")); }
        } 
        else { printf(KYEL"Failed to load the image.\n"); imgflag = 1;}
    }

    // Deinitialize FreeImage library
    FreeImage_DeInitialise();

    if(!ans) { dict_Unloader(hash_table); }
    free_image_data(&val_image);

    if(imgflag){ Error_message(0); }

    printf(KGRN"\n!Program terminated successfully!\nPress enter to exit.");
    getchar();

    return 0;
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
    char file_exe[5];

    int len = strlen(filepath);
    for(int i = 0; i < 5; i++)
    {
        file_exe[i] = filepath[len + i - 4];
    }

    if(!strcmp(file_exe, ".bin"))
    {
        fp = fopen(filepath, "rb");
    }
    else if(!strcmp(file_exe, ".txt"))
    {
        fp = fopen(filepath, "r");
    }
    else if(!strcmp(file_exe, ".png"))
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

// sqrt round off for sqare base of image
int babylon_sqrt(int n)
{
    float ans = 0.0001;
    float x1 = 0, x2 = 0;

    x1 = (n + 1) / 2.0;
    x2 = (x1 + n / x1) / 2.0;
    while(x1 - x2 >= ans)
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

/*
for mapping 2 alphabet letters
it starts from 97 goes to 122
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
one_pixel *hash_string(char *string, one_pixel *curr_pixel, short unsigned int val, node_head hash_table_address[hash_bucket_row][hash_bucket_column])
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

    if(val)
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
                i = hash_word(mod_word, 0) % hash_bucket_row;

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
                        printf("%s [%d %d %d]\n", mod_word, i, j, collision);
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
                                printf("%s [%d %d %d]\n", mod_word, i, j - offset, collision);
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
                                printf("%s [%d %d %d]\n", mod_word, i, j + offset, collision);
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
    else
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

            printf("%c [%d %d %d]\n", mod_word[strlen(mod_word) - 1], (n_pixel->red), (n_pixel->green), (n_pixel->blue));
        }
        else if (len == 2)
        {
            // double channel encoding
            {
                n_pixel -> red = control_double_encoding;
                n_pixel -> green = (int)(mod_word[0]);
                n_pixel -> blue = (int)(mod_word[1]);
            }            

            printf("%c%c [%d %d %d]\n", mod_word[strlen(mod_word) - 2],mod_word[strlen(mod_word) - 1], (n_pixel->red), (n_pixel->green), (n_pixel->blue));
        }
    }
        if(((val && found) || !val) && len >= 3)
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

                printf("%c%c [%d %d %d]\n", mod_word[strlen(mod_word) - 2],mod_word[strlen(mod_word) - 1], (temp->red), (temp->green), (temp->blue));

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
                printf("%c%c [%d %d %d]\n", mod_word[strlen(mod_word) - 2],mod_word[strlen(mod_word) - 1], (n_pixel->red), (n_pixel->green), (n_pixel->blue));
            }
            else
            {
                n_pixel -> red = control_nascii_encoding;
                n_pixel -> green = (int)mod_word[0];
                n_pixel -> blue = (int)('\0');
                printf("%c [%d %d %d]\n", mod_word[strlen(mod_word) - 2], (n_pixel->red), (n_pixel->green), (n_pixel->blue));
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

// check if user data file is setup or not
bool datafile_setup(char *path)
{
    FILE *fpr = fopen(path, "rb");
    FILE *fpw;

    if(fpr == NULL)
    {
        fclose(fpr);
        fpw = fopen(path, "wb");
        if(fpw == NULL)
        {
            fclose(fpw);
            Error_message(1);
        }

        printf(KMAG"Enter your name: ");
        scanf("%[^\n]%*c", user_data.saved_username);
        printf("Enter your password: ");
        scanf("%[^\n]%*c", user_data.saved_password);

        if(user_data.saved_username == "" || user_data.saved_password == "")
        {
            Error_message(1);
        }

        user_data.saved_passkey = hash_word(user_data.saved_password, 0);

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

    for(int x = 0; x < hash_bucket_row; x++)
    {
        for(int y = 0; y < hash_bucket_column; y++)
        {
            hash_table_address[x][y].collision_count = 0;
            hash_table_address[x][y].start = NULL;
        }
    }

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
                i = hash_word(word_holder, 0) % hash_bucket_row;
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

    printf(KGRN"\nDictionary ready. %d words loaded.", count);
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

// Convert the text from message loaded in memory to pixels
int txt_to_rgb(char **msg_array, int size, one_pixel *start_pixel, node_head hash_table_address[hash_bucket_row][hash_bucket_column])
{
    char c,char_holder[max_string_size], word_holder[max_string_size];
    int len = 0, w_len = 0, c_len = 0, number_pixels = 0;

    int count = 0;

    one_pixel *temp = start_pixel;

    word_holder[0] = char_holder[0] = '\0';

    for(int i = 0; i < size; i++)
    {
        len = strlen(msg_array[i]);
        for (int j = 0; j < len; j++)
        {
            c = msg_array[i][j];
            if(isalpha(c))
            {
                c_len = strlen(char_holder);
                if(c_len && !(c_len == 1 && char_holder[0] == ' '))
                {
                    temp = hash_string(char_holder, temp, 0, hash_table_address);
                    if(temp == NULL) return -1;
                    count++;
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
                    temp = hash_string(word_holder, temp, 1, hash_table_address);
                    if(temp == NULL) return -1;
                    count++;
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
        temp = hash_string(word_holder, temp, 1, hash_table_address);
        if(temp == NULL) return -1;
        count++;
    }
    else if(!w_len && c_len)
    {
        temp = hash_string(char_holder, temp, 0, hash_table_address);
        if(temp == NULL) return -1;
        count++;
    }

    printf("\n");

    one_pixel *end_pixel = (one_pixel *)malloc(sizeof(one_pixel));
    {
        end_pixel -> red = control_start_end_pixel;
        end_pixel -> green = 0;
        end_pixel -> blue = 25;        
        end_pixel -> next_pixel = NULL;
        temp -> next_pixel = end_pixel;
    }

    temp = start_pixel;

    while(temp != NULL)
    {
        number_pixels++;
        //printf("%d) %p [%d %d %d]\n", number_pixels, temp, temp->red, temp->green, temp->blue);
        temp = temp -> next_pixel;
    }

    return number_pixels;
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

// convert the image pixels to linked pixel data
bool png_to_pixel(unsigned short int i, unsigned short int j, unsigned short int collision, one_pixel *currpixel)
{
    one_pixel *n_pixel = (one_pixel *) malloc(sizeof(one_pixel));
    if(n_pixel == NULL) { return false; }


    if(i == control_single_encoding)
    {
        n_pixel -> red = i;
        if(j == collision)
        {
            n_pixel -> green = j;
            n_pixel -> blue = j;
        }
        else{ return false; }
    }
    else
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

    bool add_space = true;

    char word_holder[max_string_size], next_c = '\0', next_c1 = '\0', curr_c0 = '\0', curr_c1 = '\0';
    word_holder[0] = '\0';

    one_pixel *temp = image_data -> next_pixel;
    while(temp != NULL)
    {
        //printf("%p [%d %d %d] -> %p\n", temp, temp->red,temp->green,temp->blue,temp->next_pixel);
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
            while(collision < (temp -> blue))
            {
                temp_node = temp_node -> next;
                collision++;
            }
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

            break;
        }

        temp = temp -> next_pixel;
    }

    fclose(fp);
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