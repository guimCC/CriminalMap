#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#define R 6371
#define DEG 3.1415926536/180
#define MAXEDGE 20

// Structure to make decisions easily
typedef char bool;
enum
{
    false,
    true
};

// Structure that contains info about the edges
typedef struct{
    char road[12];
    unsigned numnode;
    double length;
}infoedge;

// Structure that contains node info not relevant to A*
typedef struct{
    long long int id;
    double lat, longt;
    int nedge;
    infoedge edges[MAXEDGE];
}node;

// Structure that contains relevant node info to A*
typedef struct{
    double dist_origin;
    double weight;
    unsigned previous;
    bool IsOpen;

    double criminality;
}StateAe;

// Structure for queue elements
typedef struct Element{
    unsigned node;
    struct Element * next;
}ElementQueue;

// Queue struct
typedef struct{
    ElementQueue * start, * finish;
}Queue;

void print_table(node *nodes, unsigned numnod);
void insert_with_priority(Queue *, unsigned, StateAe *);
void remove_element(Queue *, unsigned);
unsigned search_node(long long int ident, node l[], unsigned nnodes);
double distance(node, node);
void show_path(unsigned finish, node* nodes, StateAe* infnodes, unsigned origin_idx);
double crime_penalisation(int type, int weight);

int main( int argc, char *argv[]){
    FILE *nodeF;
    node* nodes;
    StateAe* infnodes;

    unsigned numnod=0, ll;

    if((nodeF=fopen("nodes.csv", "r"))==NULL){
        printf("Can't open nodes file\n");
        return 1;
    }

    while((ll=fgetc(nodeF)) != (unsigned)EOF){
        if (ll=='\n'){numnod++;}
    }

    printf("# Data from %d nodes\n", numnod);
    rewind(nodeF);

    if((nodes = (node *) malloc(numnod *  sizeof(node))) == NULL){
        printf ("\nCan't allocate memory...\n\n");
        return 1;
    }
    if((infnodes = (StateAe *) malloc(numnod *  sizeof(StateAe))) == NULL){
        printf ("\nCan't allocate memory...\n\n");
        return 1;
    }
    
    // Reading node info and storing it to an array 
    for(unsigned i=0; i<numnod; i++){
        fscanf(nodeF, "%lld;%lf;%lf\n", &(nodes[i].id), &(nodes[i].lat), &(nodes[i].longt));
    }
    fclose(nodeF);

    FILE *roadF;
    unsigned insertfirst;
    unsigned insertsecond;
    unsigned c;

    char idroad[12];
    long long int idnode;

    if((roadF=fopen("roads.csv", "r"))==NULL){
        printf("Can't open roads file\n");
        return 1;
    }

    // We proceed to read information about the roads
    // making sure they exist
    while((c=fgetc(roadF))!=(unsigned)EOF){
        fscanf(roadF,"d=%[0-9]", idroad);
        fscanf(roadF, ";%lld", &idnode);
        // Keep calling method to check if the node exists
        insertfirst=search_node(idnode, nodes, numnod);
        while((insertfirst==numnod)&&(fgetc(roadF)!='\n')){
            printf("# %lld doesn't exist\n", idnode);
            fscanf(roadF, "%lld", &idnode);
            insertfirst=search_node(idnode, nodes, numnod);
        }
        while(fgetc(roadF)!='\n'){
            fscanf(roadF, "%lld", &idnode);
            insertsecond=search_node(idnode, nodes, numnod);
            while((insertsecond==numnod)&&(fgetc(roadF)!='\n')){
                printf("# %lld doesn't exist\n", idnode);
                fscanf(roadF, "%lld", &idnode);
                insertsecond=search_node(idnode, nodes, numnod);
            }
            if(insertsecond<numnod){
                // If an adjacent pair is found, we add the relative information to both nodes
                double dist = distance(nodes[insertsecond], nodes[insertfirst]);
                strcpy(nodes[insertsecond].edges[nodes[insertsecond].nedge].road, idroad);
                nodes[insertsecond].edges[nodes[insertsecond].nedge].numnode=insertfirst;
                nodes[insertsecond].edges[nodes[insertsecond].nedge].length=dist;
                nodes[insertsecond].nedge++;
                strcpy(nodes[insertfirst].edges[nodes[insertfirst].nedge].road, idroad);
                nodes[insertfirst].edges[nodes[insertfirst].nedge].numnode=insertsecond;
                nodes[insertfirst].edges[nodes[insertfirst].nedge].length=dist;
                nodes[insertfirst].nedge++;

            }
            insertfirst=insertsecond;
        }
    }
    fclose(roadF);
    printf("# Roads uploaded\n");

    // Read crime info and insert to state vector and recieve weight
    FILE* crimeF;
    int numcrime = 0;

    int weight;
    sscanf(argv[3], "%d", &weight);

    if((crimeF=fopen("crimes.csv", "r"))==NULL){
        printf("Can't open crimes file\n");
        return 1;
    }

    while((ll=fgetc(crimeF)) != (unsigned)EOF){
        if (ll=='\n'){numcrime++;}
    }

    rewind(crimeF);

    long long int crime_id;
    unsigned crime_idx;
    int crime_type;
    float penalty;

    for(unsigned i=0; i<numcrime; i++){
        fscanf(nodeF, "%lld;%d;%*s\n", &crime_id, &crime_type);
        crime_idx = search_node(crime_id, nodes, numnod);
        penalty = crime_penalisation(crime_type, weight);
        infnodes[crime_idx].criminality = penalty;
    }
    fclose(nodeF);


    // We get start and finish nodes from the command line
    long long int origin_id;
    sscanf(argv[1], "%lld", &origin_id);

    long long int destiny_id;
    sscanf(argv[2], "%lld", &destiny_id);

    unsigned origin_idx = search_node(origin_id, nodes, numnod);
    unsigned destiny_idx = search_node(destiny_id, nodes, numnod);

    if (origin_idx != numnod && destiny_idx != numnod){
        printf("# Origin and destiny found.\n");
    }else{
        printf("Origin and destiny not found.\n");
        return 1;
    }
    
    // Mark all nodes with the desidered preconditions
    for(unsigned i=0; i<numnod; i++){
        infnodes[i].dist_origin = FLT_MAX;
        infnodes[i].IsOpen = false;
        
    }
    
    // Defines initial state for origin
    infnodes[origin_idx].dist_origin = 0;
    infnodes[origin_idx].previous = ULONG_MAX;
    infnodes[origin_idx].weight = distance(nodes[origin_idx], nodes[destiny_idx]);
    unsigned node_actual_idx;
    unsigned node_start_idx = origin_idx;
    float d;

    // Inserts the first node to an empty queue
    Queue queue = {NULL, NULL};
    insert_with_priority(&queue, origin_idx, infnodes);
    

    while (queue.start!=NULL){
        node_start_idx = queue.start->node;

        // If condition met, the optimal path has been found
        if (nodes[node_start_idx].id == destiny_id){
            break;
        }
        // Iterating over child nodes from start
        for(int i = 0; i < nodes[node_start_idx].nedge; i++){
            // new distance to consider
            d = infnodes[node_start_idx].dist_origin + nodes[node_start_idx].edges[i].length;
            node_actual_idx = nodes[node_start_idx].edges[i].numnode;
            if (d >= infnodes[node_actual_idx].dist_origin){ // distance when enqueuing
                continue;
            }else{

                if (infnodes[node_actual_idx].IsOpen == true){ 
                        // if node is actualle enqueued, must remove it to add it with priority
                    remove_element(&queue, node_actual_idx);
                }
                infnodes[node_actual_idx].previous = node_start_idx;
                // To optimize, we just store nodes' weignt, not distance to destiny
                // And we compute this just once
                // We also add criminal penalty to weight, so it's taken into consideration when inserting 
                infnodes[node_actual_idx].weight = d + infnodes[node_actual_idx].criminality + ((infnodes[node_actual_idx].dist_origin == FLT_MAX) ? 
                    distance(nodes[node_actual_idx], nodes[destiny_idx]) :  
                    infnodes[node_actual_idx].weight - infnodes[node_actual_idx].dist_origin - infnodes[node_actual_idx].criminality);
                
                infnodes[node_actual_idx].dist_origin = d;
                infnodes[node_actual_idx].IsOpen = true; // The enqueued node will now be markt as

                insert_with_priority(&queue, node_actual_idx, infnodes);

            }
        }
        infnodes[node_start_idx].IsOpen = false; // Marks the node as dequeued
        // Once finished, remove the expanded element
        remove_element(&queue, node_start_idx);
    }
    if (node_start_idx == destiny_idx){
        printf("# Path found\n");
        show_path(node_start_idx, nodes, infnodes, origin_idx);
    }else{
        printf("Path not found\n");
    }
}


void print_table(node *nodes, unsigned numnod){
    // Prints nodes and their connections, used in development
    for(unsigned i=0;i<numnod;i++){
        printf("%u: ", i);
        printf("id: %lli",nodes[i].id);
        printf(" lat: %lf",nodes[i].lat);
        printf(" longt: %lf; adjacent: ", nodes[i].longt);
        for(int j = 0; j < nodes[i].nedge; j++){
            printf(" %u (%s)", nodes[i].edges[j].numnode, nodes[i].edges[j].road);
        }
        printf("\n");
    }
}

unsigned search_node(long long int ident, node l[], unsigned nnodes){
    int lftIdx = 0;
    int rgtIdx = nnodes-1;

    // Binary search to locate nodes
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

double distance(node a, node b){
    // Implementation of the basic heuristic function, which computes the dinstace
    // betwhen two points on the surface of the earth
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

double crime_penalisation(int typec, int weight){
    double f = 1/1.5588457 * pow(3, typec/4);
    return (f * 10 + 30 ) * (weight / 10);
}

void insert_with_priority(Queue* queue, unsigned new, StateAe *infnodes){
    register ElementQueue * tmp;
    if((tmp = (ElementQueue *) malloc(sizeof(ElementQueue))) == NULL)
        {
            printf ("\nCan't allocate memory...\n\n");
        }
    tmp->node = new;
    tmp->next=NULL;
    if(queue->start == NULL){
        queue->start=tmp;
    }else{
        if (infnodes[new].weight < infnodes[queue->start->node].weight){
            tmp->next=queue->start;
            queue->start=tmp;
        }
        else{
            ElementQueue* previous = queue->start;
            // To insert with priority, we traverse the queue till the position is found
            while(previous->next!=NULL && infnodes[previous->next->node].weight < infnodes[new].weight){
                previous=previous->next;
            }
            tmp->next=previous->next;
            previous->next=tmp;
        }
    }
}


void remove_element(Queue * queue, unsigned node){
    // Traverse the queue till the element is found
    if(queue->start->node==node){
        ElementQueue *tmp = queue->start;
        queue->start=queue->start->next;
        free(tmp);
    }else{
        ElementQueue *actual=queue->start;
        while((actual->next!=NULL) && (actual->next->node != node)){
            actual=actual->next;
        }
        if(actual->next!=NULL){
            ElementQueue *tmp = actual->next;
            actual->next = tmp->next;
            free(tmp);
        }
    }    
}

void show_path(unsigned finish, node* nodes, StateAe* infnodes, unsigned origin_idx){
    printf("# Distance from %lld to %lld is %f meters.\n", nodes[origin_idx].id, nodes[finish].id, infnodes[finish].dist_origin);
    printf("# Optimal path:\n");

    unsigned* index_buffer = NULL;
    int c = 0;
    unsigned actual = finish;
    // Anem fent de forma dinàmica un vector que ens servirà per a recórrer el camí en el sentit correcte.
    while (actual != origin_idx){
        if((index_buffer = (unsigned * ) realloc(index_buffer, (c + 1) * sizeof(unsigned))) == NULL){
            printf ("\nCan't reallocate memory...\n\n");
        }
        index_buffer[c] = actual;
        actual = infnodes[actual].previous;
        c++;
    }
    if((index_buffer = (unsigned * ) realloc(index_buffer, (c + 1) * sizeof(unsigned))) == NULL){
            printf ("\nCan't reallocate memory...\n\n");
        }
        index_buffer[c] = actual;
        actual = infnodes[actual].previous;
        c++;
    
    for(int idx = c-1; idx >= 0; idx--){
        printf("Id=%lld | %f | %f | Dist=%f\n", nodes[index_buffer[idx]].id, nodes[index_buffer[idx]].lat, nodes[index_buffer[idx]].longt, infnodes[index_buffer[idx]].dist_origin);
    }
    printf("# ---------------------------------\n");
}