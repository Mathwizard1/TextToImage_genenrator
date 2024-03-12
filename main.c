#include <stdlib.h> //standard library
#include <stdio.h>  //standard input/output library

#include <ctype.h>  //character classification functions
#include <string.h> //strings

#include <unistd.h> //Os library

#include <stdbool.h> //booloean macros


//functions
void Error_message(int i);
/*
Error_message(0) -> unknown problem
Error_message(1) -> storage problem
Error_message(2) -> input problem
*/


//main
int main()
{
    printf("\nPress enter to exit.\n***Program Executed and Terminated.***");
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
        printf("The insufficient memory to store the current value.\nPlease close other programs and rerun.");
        break;
    case 2:
        printf("Invalid User input.\nPlease enter proper inputs.");
        break;
    default:
        printf("Unknow cause of error in program.");
        break;
    }

    printf("\n\nProgram is terminated. Press enter to exit.\n***Error***");
    getchar();

    //free up
    exit(0);
}