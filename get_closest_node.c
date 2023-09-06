#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#define R 6371
#define DEG 3.1415926536/180


typedef struct{
    long long int id;
    double lat, longt;
}node;

long long int get_closest_and_filter(double latitude, double longitude, node* nodes, unsigned numnods);
double distance(double latitude, double longitude, node b);

int main( int argc, char *argv[]){
    FILE *nodeF;
    node* nodes;

    unsigned numNod=0, ll;

    // Open and read the nodes file
    if ((nodeF=fopen("nodes.csv", "r"))==NULL){
        printf("Can't open nodes file\n");
        return 1;
    }

    while ((ll=fgetc(nodeF)) != (unsigned)EOF){
        if (ll=='\n'){numNod++;}
    }
    rewind(nodeF);

    if((nodes = (node *) malloc(numNod * sizeof(node))) == NULL){
        printf("\nCan't allocate memory...\n");
        return 1;
    }

    for(unsigned i=0; i<numNod; i++){
        fscanf(nodeF, "%lld;%lf;%lf\n", &(nodes[i].id), &(nodes[i].lat), &(nodes[i].longt));
    }
    fclose(nodeF);

    // recieve latitude and longitude
    double latitude;
    double longitude;
    sscanf(argv[1], "%lf", &latitude);
    sscanf(argv[2], "%lf", &longitude);


    long long int closest_id = get_closest_and_filter(latitude, longitude, nodes, numNod);
        // just write to the file, in the new format, if the filter is passed
    printf("%lld\n", closest_id);
}


long long int get_closest_and_filter(double latitude, double longitude, node* nodes, unsigned numnods){
    float min_dist = FLT_MAX;
    long long int min_id = 0;
    for (unsigned i = 0; i < numnods; i++){
        float dist = distance(latitude, longitude, nodes[i]); 
        if (dist < min_dist){
            min_dist = dist;
            min_id = nodes[i].id;
        }
    }

    return min_id;
}

double distance(double latitude, double longitude, node b){
    double d, x1, y1, z1, x2, y2, z2;
    x1=R*cos(longitude*DEG)*cos(latitude*DEG);
    y1=R*sin(longitude*DEG)*cos(latitude*DEG);
    z1=R*sin(latitude*DEG);
    x2=R*cos(b.longt*DEG)*cos(b.lat*DEG);
    y2=R*sin(b.longt*DEG)*cos(b.lat*DEG);
    z2=R*sin(b.lat*DEG);
    d=(x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) + (z1-z2)*(z1-z2);
    return sqrt(d)*(double)1000.;
}