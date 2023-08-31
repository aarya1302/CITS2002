#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int my_strlen(char str[]){

    int count = 0; 

    while( str[count] != '\0'){
        ++count;
    }
    return count; 
    
}
int replace(char str[], char chan[], char final[]){
    char final;

}

int main(int argc, char* argv[]){ 
    int num = my_strlen(argv[1]);
    printf("%d", num);


}