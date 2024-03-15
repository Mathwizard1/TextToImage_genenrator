// libraries
#include <stdlib.h> //standard library
#include <stdio.h>  //standard input/output library
#include <ctype.h>  //character classification functions
#include <string.h> //strings
#include <stdbool.h> //booloean macros

//#include "FreeImage.h" //image processing

// definitions
#define word_dictionary "data\\words.txt"
#define data_file "data\\userdat.bin"
#define contact_file "data\\contacts.bin"

#define string_size 30
#define max_string_size 45
#define line_length 512

#define hash_bucket 2550

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

struct user
{
    char saved_username[string_size];
    char saved_password[string_size];
    int saved_passkey;
} user_data;

// global variables


// functions
void Error_message(int i);
bool file_exists(char *filepath);
int hash_word(char *string, int ltr);

bool datafile_setup(char *path);

// symmetric functions
bool dict_Loader(node_head hash_table_address[hash_bucket]);
int message_Loader(char *filepath, char ***msg_array);

//bool txt_to_rgb(char **msg_array, int size);
//bool rgb_to_png(char **msg_array, int size);

//bool png_to_rgb(char **msg_array, int size);
//bool rgb_to_txt(char **msg_array, int size);

void message_Unloader(char ***msg_array, int size);
bool dict_Unloader(node_head hash_table_address[hash_bucket]);


//main
int main() {

    // basic variables
    int num_lines;
    short unsigned int count = 0, ans;
    char passw[string_size], filename[string_size], filepath[max_string_size] = "messages\\";
    char **message_array;

    // hash table and initialise count
    node_head hash_table[hash_bucket];

    printf("!Welcome to Text to image encryption and decryption.!\n");

    if(datafile_setup(data_file))
    {
        printf("\nUser data initialized. Your passkey is %d.\nStore it if you forget the password.\nPlease rerun the program.", user_data.saved_passkey);
        getchar();
        return 0;
    }

    printf("\tHello %s\nEnter your password: %s %d", user_data.saved_username, user_data.saved_password, user_data.saved_passkey);
    do
    {
        if(count)
        {
            printf("Incorrect password.\nRetry: ");
        }

        if(count == 6)
        {
            int passkey;
            printf("\nEnter passkey: ");
            scanf("%d", &passkey);
            if(passkey != user_data.saved_passkey)
            {
                Error_message(2);
            }
            else
            {
                break;
            }
        }

        scanf("%s", &passw);
        count++;

    } while (!strcmp(user_data.saved_password, passw) && count <= 6);

    printf("\nYou want to encrypt message or decrypt image?\n 1 for encrypt, 0 for decrypt: ");
    //ans = 1; 
    scanf("%hu", &ans);
    if(ans != 0 && ans != 1) {Error_message(2);}
    
    // Initialize FreeImage library
    //FreeImage_Initialise(0);

    /*if(!dict_Loader(hash_table))
    {
        Error_message(1);
    }*/

    if(ans)
    {
        printf("\nEnter the name of your message file (30 characters filename with .txt format).\nThe file should be present in the folder messages: \n");
        //strcpy(filename, "sample.txt"); 
        scanf("%[^\n]%*c", filename);
        printf("\n");

        strcat(filepath, filename);
        if(!file_exists(filepath)) {printf("No such files found.\n"); Error_message(2);}

        num_lines = message_Loader(filepath, &message_array);
        if(num_lines == 0)
        {
            Error_message(2);
        }
        else if(num_lines < 0)
        {
            message_Unloader(&message_array, (-1 * num_lines - 1));
            Error_message(0);
        }

        //FILE *FPS = fopen(strcat(filepath,".txt"), "w");
        printf("Contents of %s file for encrypting:\n", filename);
        for(int i = 0; i < num_lines; i++)
        {
            printf("%s", message_array[i]);
            //fprintf(FPS, "%s", message_array[i]);
        }
        //fclose(FPS);

        //txt_to_png();
        message_Unloader(&message_array, num_lines);
    }
    else
    {

    }

    // Load an image
    /*FIBITMAP *image = FreeImage_Load(FIF_JPEG, "input.jpg", 0);

    // Check if the image was loaded successfully
    if (image) {
        // Convert the image to grayscale
        FIBITMAP *grayImage = FreeImage_ConvertToGreyscale(image);

        // Save the grayscale image
        FreeImage_Save(FIF_JPEG, grayImage, "gray_output.jpg", 0);
        
        // Unload the images
        FreeImage_Unload(grayImage);
        FreeImage_Unload(image);
    } else {
        printf("Failed to load image!\n");
    }*/

    // Deinitialize FreeImage library
    //FreeImage_DeInitialise();

    /*if(!dict_Unloader(hash_table))
    {
        Error_message(0);
    }*/

    printf("\n\n!Program terminated successfully!\nPress enter to exit.");
    getchar();

    return 0;
}

//error message
void Error_message(int i)
{
    printf("\n***Error***\n");

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
bool file_exists(char *filepath)
{
    bool val;
    FILE *fp;

    if(strstr(filepath, ".bin"))
    {
        fp = fopen(filepath, "rb");
    }
    else if(strstr(filepath, ".txt"))
    {
        fp = fopen(filepath, "r");
    }
    else
    {
        printf("\nInvalid file type.\nMessage files should be in .txt\n");
        Error_message(1);
    }

    if(fp == NULL)
    {
        val = false;
    }
    else
    {
        val = true;
    }

    fclose(fp);
    return val;
}

// !*hash function*!
int hash_word(char *string, int ltr)
{
    int hash_val = 0;
    int l = strlen(string);

    switch(ltr)
    {
        case 1:
        case 2:
        case 3:
            for(int i = 0; i < l; i++)
            {
                int a = (int) string[i] - 97;
                if(i < ltr)
                {
                    hash_val = hash_val * 10 + a;

                }
                else if(i == ltr){break;}   
            }
            break;
        default:
            for(int i = 0; i < l; i++)
            {
                int a = (int) string[i] - 97;
                switch (i % 5)
                {
                case 1:
                case 3:
                    hash_val = (hash_val >> 2) + a;
                    break;
                case 2:
                    hash_val = (hash_val | (a * 5));
                    break;
                default:
                    hash_val = hash_val + 250;
                    break;
                }
            }
            break;
    }
    return hash_val;
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

        printf("Enter your name: ");
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

bool dict_Loader(node_head hash_table_address[hash_bucket])
{

    return 0;
}

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
            printf("non-ascii Character found.\nEncryption not supported.\n");
            count_lines = 0;
            break;
        }

        if(max_len > line_length)
        {
            printf("Lines exceed the input size.\n");
            count_lines = 0;
            break; 
        }

        if(c == '\n')
        {
            count_lines++;
            max_len = 0;
        }
    } while (c != EOF);

    if(max_len == 1){ printf("\nFile is empty.\n"); count_lines = 0;}

    if(count_lines == 0){ fclose(fp); return count_lines;}

    // initialise the 2d array which contains all lines
    *msg_array = (char **) malloc((count_lines) * sizeof(char *));
    if(*msg_array == NULL) { fclose(fp); count_lines = 0; return count_lines;}

    fseek(fp, 0, SEEK_SET);

    while(lines_i < count_lines && count_lines > 0)
    {
        if(fgets(c_line, line_length ,fp) == NULL) {break;}

        max_len = strlen(c_line);
        char *line = (char *) malloc((max_len + 2) * sizeof(char));
        if(line == NULL) { fclose(fp); return (-1 * (lines_i + 1));}

        strcpy(line, c_line);
        (*msg_array)[lines_i] = line;
        lines_i++;
    }
    
    fclose(fp);
    return count_lines;    
}

void message_Unloader(char ***msg_array, int size)
{
    for(int i = 0; i < size; i++)
    {
        free((*msg_array)[size - i - 1]);
    }

    free(*msg_array);
}



bool dict_Unloader(node_head hash_table_address[hash_bucket])
{

    return 0;
}