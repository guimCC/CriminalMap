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

typedef struct{
    int type;
    double lat, longt;
    char date[10];
}crime;


long long int get_closest_and_filter(crime entry, node* nodes, unsigned numnods, float max_dist);
double distance(crime a, node b);

int main( int argc, char *argv[]){
    FILE *nodeF;
    node* nodes;

    FILE *crimeF;
    crime* crimes;

    FILE *exitF;

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

    printf("# Data from %d nodes\n", numNod);

    if((nodes = (node *) malloc(numNod * sizeof(node))) == NULL){
        printf("\nCan't allocate memory...\n");
        return 1;
    }

    for(unsigned i=0; i<numNod; i++){
        fscanf(nodeF, "%lld;%lf;%lf\n", &(nodes[i].id), &(nodes[i].lat), &(nodes[i].longt));
    }
    fclose(nodeF);
    unsigned numCrim = 0;

    // Open and read the crime file
    if ((crimeF=fopen("tmp.csv", "r"))==NULL){
        printf("Can't open tmp file\n");
        return 1;
    }

    while ((ll=fgetc(crimeF)) != (unsigned)EOF){
        if (ll=='\n'){numCrim++;}
    }
    rewind(crimeF);

    printf("# Data from %d crimes\n", numCrim);

    if((crimes = (crime *) malloc(numCrim * sizeof(crime))) == NULL){
        printf("\nCan't allocate memory...\n");
        return 1;
    }

    for(unsigned i=0; i<numCrim; i++){
        fscanf(crimeF, "%d;%lf;%lf;%s\n", &(crimes[i].type), &(crimes[i].lat), &(crimes[i].longt), crimes[i].date);
    }
    fclose(nodeF);

    if ((exitF=fopen("crimes.csv", "w"))==NULL){
        printf("Can't open tmp file\n");
        return 1;
    }

    double max_dist;
    sscanf(argv[1], "%lf", &max_dist);
    
    for(unsigned i=0; i < numCrim; i++){
        long long int closest_id = get_closest_and_filter(crimes[i], nodes, numNod, max_dist);
        // just write to the file, in the new format, if the filter is passed
        
        if (closest_id != -1){
            fprintf(exitF, "%lld;%d;%s;%lf;%lf\n", closest_id, crimes[i].type, crimes[i].date, crimes[i].lat, crimes[i].longt);
        }
    }
    fclose(exitF);
}


long long int get_closest_and_filter(crime entry, node* nodes, unsigned numnods, float max_dist){
    float min_dist = FLT_MAX;
    long long int min_id = 0;
    for (unsigned i = 0; i < numnods; i++){
        float dist = distance(entry, nodes[i]); 
        if (dist < min_dist){
            min_dist = dist;
            min_id = nodes[i].id;
        }
    }

    // if the minnimum distance is greater that the max_distance, skip it to filter far points
    if (min_dist > max_dist){
        min_id = -1;
    }

    return min_id;
}

double distance(crime a, node b){
    double d, x1, y1, z1, x2, y2, z2;
    x1=R*cos(a.longt*DEG)*cos(a.lat*DEG);
    y1=R*sin(a.longt*DEG)*cos(a.lat*DEG);
    z1=R*sin(a.lat*DEG);
    x2=R*cos(b.longt*DEG)*cos(b.lat*DEG);
    y2=R*sin(b.longt*DEG)*cos(b.lat*DEG);
    z2=R*sin(b.lat*DEG);
    d=(x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) + (z1-z2)*(z1-z2);
    return sqrt(d)*(double)1000.;
}