#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define	MIN_LAT				-90.0
#define	MAX_LAT				 90.0
#define	MIN_LON				-180.0
#define	MAX_LON				 180.0

double degrees_to_radians(double degrees){
    return (degrees * M_PI)/180;

}

bool valid_location(double latitude, double longitude){
    if(latitude < MIN_LAT && latitude >MAX_LAT){
        return false;
    }if(longitude < MIN_LON && longitude >MAX_LON){
        return false;
    }
    return true;
}
double haversine(double latitude1, double longitude1, double latitude2, double longitude2){


        double r_earth = 6356.752 * 1000.0; 


        double deltaLat =(latitude2 - latitude1)/2.0; 
        double deltaLong = (longitude2 - longitude1)/2.0;


        double sin1 = sin(degrees_to_radians(deltaLat));
        double sin2 = sin(degrees_to_radians(deltaLong));




        double x = sin1* sin1+ sin2 * sin2 *cos(degrees_to_radians(latitude1))* cos(degrees_to_radians(latitude2));



        double ans = 2.0 * r_earth * asin(sqrt(x));

        return ans;

    
}

int main(int argc, char *argv[]){
    double lat1 = atof(argv[1]);
    double long1 = atof(argv[2]);

    double lat2 = atof(argv[3]);
    double long2 = atof(argv[4]);
    int result = EXIT_FAILURE;
    if(argc != 5) {
        printf("Sorry, invalid number of arguments, %d, for %s", argc, argv[0]);
    }else{
        if(valid_location(lat1, long1) == false){
            printf("Sorry, invalid lat: %f and long: %f ", lat1, long1);
            return 0;
        }else if (valid_location(lat2, long2) == false){
            printf("Sorry, invalid lat: %f and long: %f ", lat2, long2);

        }else{

            double dist = haversine(lat1, long1, lat2, long2);
            printf("%i m", (int)dist);
            result = EXIT_SUCCESS;

        }
        

    }
    return result;
   

}