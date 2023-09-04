#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#define R 6371
#define graus 3.1415926536/180
#define MAXARST 20

// Structure to make decisions easily
typedef char bool;
enum
{
    false,
    true
};

// Structure that contains info about the edges
typedef struct{
    char carrer[12];
    unsigned numnode;
    double llargada;
}infoaresta;

// Structure that contains node info not relevant to A*
typedef struct{
    long long int id;
    double latitud, longitud;
    int narst;
    infoaresta arestes[MAXARST];
}node;

// Structure that contains relevant node info to A*
typedef struct{
    double dist_origen;
    double pes;
    unsigned anterior;
    bool IsOpen;
}EstatAe;

// Structure for queue elements
typedef struct Element{
    unsigned node;
    struct Element * seg;
}ElementCua;

// Queue struct
typedef struct{
    ElementCua * inici, * final;
}UnaCua;

void imprimirtaula(node *nodes, unsigned numnod);
void posar_amb_prioritat(UnaCua *, unsigned, EstatAe *);
void treureNelement(UnaCua *, unsigned);
unsigned buscapunt(long long int ident, node l[], unsigned nnodes);
double distancia(node, node);
void mostracami(unsigned final, node* nodes, EstatAe* infnodes, unsigned origen_idx);

int main( int argc, char *argv[]){
    FILE *nodeF;
    node* nodes;
    EstatAe* infnodes;

    unsigned numnod=0, ll;

    if((nodeF=fopen("nodes.csv", "r"))==NULL){
        printf("No es pot obrir el fitxer\n");
        return 1;
    }

    while((ll=fgetc(nodeF)) != (unsigned)EOF){
        if (ll=='\n'){numnod++;}
    }

    printf("# Dades de %d nodes\n", numnod);
    rewind(nodeF);

    if((nodes = (node *) malloc(numnod *  sizeof(node))) == NULL){
        printf ("\nNo es possible assignar la memoria necessaria...\n\n");
        return 1;
    }
    if((infnodes = (EstatAe *) malloc(numnod *  sizeof(EstatAe))) == NULL){
        printf ("\nNo es possible assignar la memoria necessaria...\n\n");
        return 1;
    }
    
    // Llegim l'informació dels nodes i els guardem al vector de nodes principal 
    for(unsigned i=0; i<numnod; i++){
        fscanf(nodeF, "%lld;%lf;%lf\n", &(nodes[i].id), &(nodes[i].latitud), &(nodes[i].longitud));
    }
    fclose(nodeF);

    FILE *carrerF;
    unsigned posant;
    unsigned pos;
    unsigned c;

    char idcarrer[12];
    long long int idnode;

    if((carrerF=fopen("Carrers.csv", "r"))==NULL){
        printf("No es pot obrir el fitxer\n");
        return 1;
    }

    // We proceed to read information about the roads
    // making sure they exist
    while((c=fgetc(carrerF))!=(unsigned)EOF){
        fscanf(carrerF,"d=%[0-9]", idcarrer);
        fscanf(carrerF, ";%lld", &idnode);
        // Keep calling method to check if the node exists
        posant=buscapunt(idnode, nodes, numnod);
        while((posant==numnod)&&(fgetc(carrerF)!='\n')){
            printf("# %lld no existeix1\n", idnode);
            fscanf(carrerF, "%lld", &idnode);
            posant=buscapunt(idnode, nodes, numnod);
        }
        while(fgetc(carrerF)!='\n'){
            fscanf(carrerF, "%lld", &idnode);
            pos=buscapunt(idnode, nodes, numnod);
            while((pos==numnod)&&(fgetc(carrerF)!='\n')){
                printf("# %lld no existeix2\n", idnode);
                fscanf(carrerF, "%lld", &idnode);
                pos=buscapunt(idnode, nodes, numnod);
            }
            if(pos<numnod){
                // If an adjacent pair is found, we add the relative information to both nodes
                double dist = distancia(nodes[pos], nodes[posant]);
                strcpy(nodes[pos].arestes[nodes[pos].narst].carrer, idcarrer);
                nodes[pos].arestes[nodes[pos].narst].numnode=posant;
                nodes[pos].arestes[nodes[pos].narst].llargada=dist;
                nodes[pos].narst++;
                strcpy(nodes[posant].arestes[nodes[posant].narst].carrer, idcarrer);
                nodes[posant].arestes[nodes[posant].narst].numnode=pos;
                nodes[posant].arestes[nodes[posant].narst].llargada=dist;
                nodes[posant].narst++;

            }
            posant=pos;
        }
    }
    fclose(carrerF);
    printf("# Carrers pujats\n");

    // Un cop tenim tots els nodes carregats, anem a actualitzar la distància respecte
    // el final amb la funció distància

    // Obtenim com a arguments passats per consola els nodes d'inici i final
    long long int origen_id;
    sscanf(argv[1], "%lld", &origen_id);

    long long int desti_id;
    sscanf(argv[2], "%lld", &desti_id);

    unsigned origen_idx = buscapunt(origen_id, nodes, numnod);
    unsigned desti_idx = buscapunt(desti_id, nodes, numnod);


    if (origen_idx != numnod && desti_idx != numnod){
        printf("# Correcte, nodes trobats.\n");
    }else{
        printf("No s'han trobat els nodes.\n");
        return 1;
    }
    
    // Marquem tots els nodes com a tancats i amb distància màxima a l'origen
    for(unsigned i=0; i<numnod; i++){
        infnodes[i].dist_origen = FLT_MAX;
        infnodes[i].IsOpen = false;
        
    }
    
    // Per al node inicial, definim les constants pertinents de l'A*
    infnodes[origen_idx].dist_origen = 0;
    infnodes[origen_idx].anterior = ULONG_MAX;
    infnodes[origen_idx].pes = distancia(nodes[origen_idx], nodes[desti_idx]);
    unsigned node_actual_idx;
    unsigned node_inici_idx = origen_idx;
    float d;

    // Inicialitzem la cua amb el primer node
    UnaCua cua = {NULL, NULL};
    posar_amb_prioritat(&cua, origen_idx, infnodes);
    

    while (cua.inici!=NULL){
        node_inici_idx = cua.inici->node;

        // Condició que indica que hem trobat el node destí pel camí òptim
        if (nodes[node_inici_idx].id == desti_id){
            break;
        }
        // Iterating over child nodes from start
        for(int i = 0; i < nodes[node_inici_idx].narst; i++){
            //nova distància  a considerar
            d = infnodes[node_inici_idx].dist_origen + nodes[node_inici_idx].arestes[i].llargada;
            node_actual_idx = nodes[node_inici_idx].arestes[i].numnode;
            if (d >= infnodes[node_actual_idx].dist_origen){ //comparem amb dist_origen, és el PES que usem només al encuar
                continue;
            }else{

                if (infnodes[node_actual_idx].IsOpen == true){ 
                        // mirem que el node estigui actualment obert. Si així és, el traiem
                        // per a posar-lo en prioritat a continuació
                    treureNelement(&cua, node_actual_idx);
                }
                infnodes[node_actual_idx].anterior = node_inici_idx;
                // Per a maximitzar l'eficiència de l'algorisme s'ha usat l'optimització presentada a la diapo
                // 120 de l'últim pdf. On a l'estructura de supost no guardem la distància al destí,
                // la recuperem a través del pes. A part, només la calculem quan expandim
                // un node per primer cop
                infnodes[node_actual_idx].pes = d + ((infnodes[node_actual_idx].dist_origen == FLT_MAX) ? 
                distancia(nodes[node_actual_idx], nodes[desti_idx]) :  infnodes[node_actual_idx].pes - infnodes[node_actual_idx].dist_origen);
                
                infnodes[node_actual_idx].dist_origen = d;
                infnodes[node_actual_idx].IsOpen = true; //estigui ja dins o no, marquem que ho estarà d'ara en endavant.

                posar_amb_prioritat(&cua, node_actual_idx, infnodes);

            }
        }
        infnodes[node_inici_idx].IsOpen = false; //marquem que ja no està dins de la cua, tot i que pot tornar a entrar-hi
        // traiem el primer element
        treureNelement(&cua, node_inici_idx);
    }
    if (node_inici_idx == desti_idx){
        printf("# S'ha trobat el cami\n");
        mostracami(node_inici_idx, nodes, infnodes, origen_idx);
    }else{
        printf("No s'ha trobat el cami\n");
    }
}


void imprimirtaula(node *nodes, unsigned numnod){
    // funció per a imprimir la taula de nodes amb les serves respectives connexions
    // usada com a suport en la fase de desenvolupamentS 
    for(unsigned i=0;i<numnod;i++){
        printf("%u: ", i);
        printf("id: %lli",nodes[i].id);
        printf(" latitud: %lf",nodes[i].latitud);
        printf(" longitud: %lf; adjacents: ", nodes[i].longitud);
        for(int j = 0; j < nodes[i].narst; j++){
            printf(" %u (%s)", nodes[i].arestes[j].numnode, nodes[i].arestes[j].carrer);
        }
        printf("\n");
    }
}

unsigned buscapunt(long long int ident, node l[], unsigned nnodes){
    int lftIdx = 0;
    int rgtIdx = nnodes-1;

    // S'ha implementat una cerca binària usant que els nodes estan ordenats,
    // Així s'aconsegueix més eficiència
    while (lftIdx <= rgtIdx){
        int midIdx = lftIdx + (rgtIdx - lftIdx)/2; //fem d'aquesta manera per a evitar fer la suma que necessitaria més memòria
        if (l[midIdx].id == ident){
            return midIdx;
        } else if (l[midIdx].id < ident){
            lftIdx = midIdx + 1;
        } else {
            rgtIdx = midIdx - 1;
        }
    }
    return nnodes;
}

double distancia(node a, node b){
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

void posar_amb_prioritat(UnaCua* cua, unsigned nou, EstatAe *infnodes){
    register ElementCua * tmp;
    if((tmp = (ElementCua *) malloc(sizeof(ElementCua))) == NULL)
        {
            printf ("\nNo es possible assignar la memoria necessaria...\n\n");
        }
    tmp->node = nou;
    tmp->seg=NULL;
    if(cua->inici == NULL){
        cua->inici=tmp;
    }else{
        if (infnodes[nou].pes < infnodes[cua->inici->node].pes){
            tmp->seg=cua->inici;
            cua->inici=tmp;
        }
        else{
            ElementCua* anterior = cua->inici;
            // Com que estem insertant amb prioritat, anirem recorrent la cua fins
            // a trobar-nos amb la posició a la qual pertany o arribar al final
            while(anterior->seg!=NULL && infnodes[anterior->seg->node].pes < infnodes[nou].pes){
                anterior=anterior->seg;
            }
            tmp->seg=anterior->seg;
            anterior->seg=tmp;
        }
    }
}


void treureNelement(UnaCua * cua, unsigned node){
    // anem recorrent la cua fins a trobar-nos amb el node buscat. Llavors, el desencuem
    // i netegem la memòria corresponent.
    if(cua->inici->node==node){
        ElementCua *tmp = cua->inici;
        cua->inici=cua->inici->seg;
        free(tmp);
    }else{
        ElementCua *actual=cua->inici;
        while((actual->seg!=NULL) && (actual->seg->node != node)){
            actual=actual->seg;
        }
        if(actual->seg!=NULL){
            ElementCua *tmp = actual->seg;
            actual->seg = tmp->seg;
            free(tmp);
        }
    }    
}

void mostracami(unsigned final, node* nodes, EstatAe* infnodes, unsigned origen_idx){
    printf("# La distancia de %lld a %lld es de %f metres.\n", nodes[origen_idx].id, nodes[final].id, infnodes[final].dist_origen);
    printf("# Cami optim:\n");

    unsigned* index_buffer = NULL;
    int c = 0;
    unsigned actual = final;
    // Anem fent de forma dinàmica un vector que ens servirà per a recórrer el camí en el sentit correcte.
    while (actual != origen_idx){
        if((index_buffer = (unsigned * ) realloc(index_buffer, (c + 1) * sizeof(unsigned))) == NULL){
            printf ("\nNo es possible reassignar la memoria necessaria...\n\n");
        }
        index_buffer[c] = actual;
        actual = infnodes[actual].anterior;
        c++;
    }
    if((index_buffer = (unsigned * ) realloc(index_buffer, (c + 1) * sizeof(unsigned))) == NULL){
            printf ("\nNo es possible reassignar la memoria necessaria...\n\n");
        }
        index_buffer[c] = actual;
        actual = infnodes[actual].anterior;
        c++;
    
    for(int idx = c-1; idx >= 0; idx--){
        printf("Id=%lld | %f | %f | Dist=%f\n", nodes[index_buffer[idx]].id, nodes[index_buffer[idx]].latitud, nodes[index_buffer[idx]].longitud, infnodes[index_buffer[idx]].dist_origen);
    }
    printf("# ---------------------------------\n");
}