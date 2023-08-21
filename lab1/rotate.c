#include <stdio.h> //defines constants and declares functions required for i/o, e.g fprintf, printf and stderr
#include <stdlib.h> // constants and functions for interface C with OS e.g exit()
#include <string.h> // declares string handling functions e.g strlen
#include <ctype.h> // character testing functions e.g islower()

// Compile this program with:
//    cc -std=c11 -Wall -Werror -o rotate rotate.c

// Q1
//changing ROT constant to 3 to create caesar cipher
#define ROT 13 

//  The rotate function returns the character ROT positions further along the
//  alphabetic character sequence from c, or c if c is not lower-case
char rotate(char c, int rotate_by)
{
    // Check if c is lower-case or not
    if(islower(c)) {
        // The ciphered character is ROT positions beyond c,
        // allowing for wrap-around
        return ('a' + (c - 'a' + rotate_by) % 26);
    }
    else if(isupper(c)) {

        return ('A' + (c - 'A' + rotate_by) % 26);
    }
    return c;
}


//  Execution of the whole program begins at the main function

int main(int argcount, char *argvalue[])
{
    // Exit with an error if the number of arguments (including
    // the name of the executable) is not precisely 2

        if(argcount != 3){
            fprintf(stderr, "%s: program expected 2 arguments, received %d\n", argvalue[0], argcount-1);
             exit(EXIT_FAILURE);
        }else{
            int new_rotation = atoi(argvalue[1]);
            for(int i = 1; i < argcount; i++){
            // Calculate the length of the first argument


        int length = strlen(argvalue[i]);

        // Loop for every character in the text
        for(int j = 0; j < length; j++) {
            // Determine and print the ciphered character
            char ch = argvalue[i][j];
            printf("%c %c %d\n",ch, rotate(argvalue[i][j], new_rotation), j); //qu2 printing out each char in one line with /n
        }

        // Print one final new-line character
        printf("\n");

        // Exit indicating success

        }
        exit(EXIT_SUCCESS);
        }

        

        

    return 0;
}


