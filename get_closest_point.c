#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#define R 6371
#define graus 3.1415926536/180


typedef struct{
    long long int id;
    double latitud, longitud;
}node;

typedef struct{
    int type;
    double latitud, longitud;
    char date[10];
}crim;


long long int get_closest_and_filter(crim entry, node* nodes, unsigned numnods, float max_dist);
double distancia(crim a, node b);

int main( int argc, char *argv[]){
    FILE *nodeF;
    node* nodes;

    FILE *crimF;
    crim* crims;

    FILE *exitF;

    unsigned numNod=0, ll;

    // Open and read the nodes file
    if ((nodeF=fopen("nodes.csv", "r"))==NULL){
        printf("No es pot obrir el fitxer\n");
        return 1;
    }

    while ((ll=fgetc(nodeF)) != (unsigned)EOF){
        if (ll=='\n'){numNod++;}
    }
    rewind(nodeF);

    printf("# Dades de %d nodes\n", numNod);

    if((nodes = (node *) malloc(numNod * sizeof(node))) == NULL){
        printf("\nNo es possible assignar la memoria necessaria.\n");
        return 1;
    }

    for(unsigned i=0; i<numNod; i++){
        fscanf(nodeF, "%lld;%lf;%lf\n", &(nodes[i].id), &(nodes[i].latitud), &(nodes[i].longitud));
    }
    fclose(nodeF);
    unsigned numCrim = 0;

    // Open and read the crime file
    if ((crimF=fopen("tmp.csv", "r"))==NULL){
        printf("No es pot obrir el fitxer\n");
        return 1;
    }

    while ((ll=fgetc(crimF)) != (unsigned)EOF){
        if (ll=='\n'){numCrim++;}
    }
    rewind(crimF);

    printf("# Dades de %d nodes\n", numCrim);

    if((crims = (crim *) malloc(numCrim * sizeof(crim))) == NULL){
        printf("\nNo es possible assignar la memoria necessaria.\n");
        return 1;
    }

    for(unsigned i=0; i<numCrim; i++){
        fscanf(crimF, "%d;%lf;%lf;%s\n", &(crims[i].type), &(crims[i].latitud), &(crims[i].longitud), crims[i].date);
    }
    fclose(nodeF);

    exitF=fopen("crims.csv", "w");

    //redefinir com convingui
    double max_dist = 9999;
    for(unsigned i=0; i < numCrim; i++){
        long long int closest_id = get_closest_and_filter(crims[i], nodes, numNod, max_dist);
        // just write to the file, in the new format, if the filter is passed
        
        if (closest_id != 0){
            fprintf(exitF, "%lld;%d;%s\n", closest_id, crims[i].type, crims[i].date);
            //printf("Passed %lf %lf %s\n", crims[i].latitud, crims[i].longitud, crims[i].date);
        }//else{
            //printf("Skipped %lf %lf %s\n", crims[i].latitud, crims[i].longitud, crims[i].date);
        //}
    }
    fclose(exitF);
}


long long int get_closest_and_filter(crim entry, node* nodes, unsigned numnods, float max_dist){
    float min_dist = FLT_MAX;
    long long int min_id = 0;
    for (unsigned i = 0; i < numnods; i++){
        float dist = distancia(entry, nodes[i]); 
        if (dist < min_dist){
            min_dist = dist;
            min_id = nodes[i].id;
        }
    }

    // if the minnimum distance is greater that the max_distance, skip it to filter far points
    if (min_dist > max_dist){
        min_id = 0;
    }

    return min_id;
}

double distancia(crim a, node b){
    // Implementacó de la funció heurística, que calcula la distància entre dos punts geogràficament
    // en metres
    double d, x1, y1, z1, x2, y2, z2;
    x1=R*cos(a.longitud*graus)*cos(a.latitud*graus);
    y1=R*sin(a.longitud*graus)*cos(a.latitud*graus);
    z1=R*sin(a.latitud*graus);
    x2=R*cos(b.longitud*graus)*cos(b.latitud*graus);
    y2=R*sin(b.longitud*graus)*cos(b.latitud*graus);
    z2=R*sin(b.latitud*graus);
    d=(x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) + (z1-z2)*(z1-z2);
    return sqrt(d)*(double)1000.;
}