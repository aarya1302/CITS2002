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
    return 0; 

}