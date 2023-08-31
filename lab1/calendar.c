#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int atoi(const char *str);

int is_leap_year(int year){ 
    if ((year % 400 == 0) || ((year % 4 == 0) && (year % 100 != 0))){
            return 1;
    }else{ 
            return 0;
        
    }

}
int first_day_of_month(int month, int year)
{
    struct tm   tm;

//  SET ALL FIELDS OF tm TO ZERO TO MAKE SOME FIELDS 'UNKNOWN'
    memset(&tm, 0, sizeof(tm));

//  INITIALIZE THE FILEDS THAT WE ALREADY KNOW
    tm.tm_mday  = 1;
    tm.tm_mon   = month-1;              // 0=Jan, 1=Feb, ....
    tm.tm_year  = year-1900;

//  ASK THE POSIX FUNCTION mktime TO DETERMINE THE 'UNKNOWN' FIELDS
//  See: http://pubs.opengroup.rg/onlinepubs/9699919799/
    mktime(&tm);


//  RETURN THE INTEGER MONTH VALUE
    return tm.tm_wday;                  // 0=Sun, 1=Mon, .....
}



int number_of_days(int month){ 
    int num_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    return num_days[month];
}

int main(int argc, char *argv[]){
    
    if(argc < 3){
        fprintf(stderr, "Usage: %s argument\n", argv[0]);
        exit(EXIT_FAILURE);
    }else{ 

    int month = atoi(argv[1]);
    int year = atoi(argv[2]);
    int num_days = 0;


    if (month == 2){
        if(is_leap_year(year) == 1){
            num_days = 29; 
        }else{
            num_days = 28;
        }

    }else{ 
        num_days = number_of_days(month);
    }
    int count = 0; 
    
    printf("%c %d \n", month, year);

    printf("Su Mo Tu We Th Fr Sa\n");

    for(int i = 0; i < first_day_of_month(month, year); ++i){
        printf("   ");
        ++count;
    }
    for(int i = 0; i < num_days; ++i){

            printf("%d ", i+1);
            ++count;

            if(i < 8){
                printf(" ");
            }
            if(count > 6){ 
                printf("\n");
                count = 0; 

            }
        
        


    }



    printf("%d \n", first_day_of_month(month, year));
    exit(EXIT_SUCCESS);
        
    }


    return 0;
}