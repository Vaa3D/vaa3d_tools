#include <math.h>

#include "extractTree.h"
#include "FunctionsForMainCode.h"
#include "basic_surf_objs.h"


int minDistance_path(double dist[], bool sptSet[], int V);
void dijkstra_path(double *graph, int num, int src, int des, int* path, int &index);
int graph_shortest_paths(double *, int, int, int, int*);



void extractTree (double *D_euc, double *DX,double *Xnew,int indori)
{
    double *conn = new double[indori*indori];
    for(V3DLONG i = 0; i<indori*indori; i++)
    {
        if(DX[i] !=0 && DX[i] < INFINITY)
            conn[i] = DX[i];
        else
             conn[i] = 0;
    }

    int path[indori];
    for(int i = 0; i <indori; i++)
        path[0] = -1;
    int length = graph_shortest_paths(conn,indori-1,0,indori,path);


    //NeutronTree structure
    NeuronTree PSF_swc;
    QList <NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();

    NeuronSWC S;

    S.n 	= 1;
    S.type 	= 3;
    S.x 	= Xnew[path[length]*3+0];
    S.y 	= Xnew[path[length]*3+1];
    S.z 	= Xnew[path[length]*3+2];
    S.r 	= 1;
    S.pn = -1;

    listNeuron.append(S);
    hashNeuron.insert(S.n, listNeuron.size()-1);
    int ID = 2;
    for(int i = length-2; i >=0; i--)
    {

       S.n 	= ID;
       S.type 	= 3;
       S.x 	= Xnew[path[i]*3+0];
       S.y 	= Xnew[path[i]*3+1];
       S.z 	= Xnew[path[i]*3+2];
       S.r 	= 1;
       S.pn = ID-1;
       listNeuron.append(S);
       hashNeuron.insert(S.n, listNeuron.size()-1);
       ID++;
    }
    PSF_swc.n = -1;
    PSF_swc.on = true;
    PSF_swc.listNeuron = listNeuron;
    PSF_swc.hashNeuron = hashNeuron;

    writeSWC_file("/home/zhi/Desktop/tmp/first.swc",PSF_swc);


     return;
}


int graph_shortest_paths(double *conn, int src, int des, int n, int *path)
{

    int index;
    dijkstra_path(conn,n,src,des,path,index);
    return index;
}


void dijkstra_path(double *graph, int V, int src, int des, int *path, int &index)
{
     double dist[V];     // The output array.  dist[i] will hold the shortest
                      // distance from src to i
     int P[V];
     bool sptSet[V]; // sptSet[i] will true if vertex i is included in shortest
                     // path tree or shortest distance from src to i is finalized

     // Initialize all distances as INFINITE and stpSet[] as false
     for (int i = 0; i < V; i++)
        dist[i] = INT_MAX, sptSet[i] = false;

     // Distance of source vertex from itself is always 0
      dist[src] = 0;

     // Find shortest path for all vertices
     for (int count = 0; count < V-1; count++)
     {
       // Pick the minimum distance vertex from the set of vertices not
       // yet processed. u is always equal to src in first iteration.
       int u = minDistance_path(dist, sptSet,V);
       if(u == des)
       {
           break;
       }
       // Mark the picked vertex as processed
       sptSet[u] = true;

       // Update dist value of the adjacent vertices of the picked vertex.
       for (int v = 0; v < V; v++)
       {

         // Update dist[v] only if is not in sptSet, there is an edge from
         // u to v, and total weight of path from src to  v through u is
         // smaller than current value of dist[v]
        if (!sptSet[v] && graph[v*V+u] && dist[u] != INT_MAX
                                       && dist[u]+graph[v*V+u] < dist[v])
         {
            dist[v] = dist[u] + graph[v*V+u];
            P[v] = u;
         }
       }
     }

     index = 0;

     while(des!=src)
     {
         path[index] = des;
         des = P[des];
         index++;
     }

    // printf("index is : %d\n",index);
     path[index] = des;


     // print the constructed distance array
}
int minDistance_path(double dist[], bool sptSet[],int V)
{
   // Initialize min value
   double min = INT_MAX;
   int min_index;

   for (int v = 0; v < V; v++)
     if (sptSet[v] == false && dist[v] <= min)
         min = dist[v], min_index = v;

   return min_index;
}
