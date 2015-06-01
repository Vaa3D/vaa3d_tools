#include <math.h>

#include "extractTree.h"
#include "FunctionsForMainCode.h"
#include "basic_surf_objs.h"
#include <queue>

int minDistance_path(double dist[], bool sptSet[], int V);
void dijkstra_path(double *graph, int num, int src, int des, int* path, int &index);
int graph_shortest_paths(double *, int, int, int, int*);
int graph_conn_comp(double *,int *,int);
void dist2pwlines(double *, double *,double*,int *, int, int);

void extractTree (double *D_euc, double *DX,double *Xnew,int indori, QString output_name)
{
    double *conn = new double[indori*indori];
    for(V3DLONG i = 0; i<indori*indori; i++)
    {
        if(DX[i] !=0 && DX[i] < INFINITY)
            conn[i] = DX[i];
        else
             conn[i] = 0;
    }

    double synapseTHR = 5.0;
    int *cidx = new int[indori];
    int ctrs = graph_conn_comp(conn,cidx,indori);
    printf("total group number is %d\n",ctrs);
    int t = 1;

    double *clu_indCi;
    double *clu_traced;
    double *clu_traced_updated;
    int traced_length;
    int traced_length_all;
    double *clu_current;
    int indCi_length;
    while(1)
    {
        int leafCi = -1;
        double max_dist = 0;
        indCi_length = 0;
        for(V3DLONG i = 0; i < indori; i++)
        {
            if(cidx[i] == 1)
            {
                double dist = D_euc[(indori-1)*indori + i];
                if(dist > max_dist)
                {
                    max_dist = dist;
                    leafCi = i;
                }
                indCi_length++;
            }
        }

        if(leafCi !=-1)
        {
            printf("t is %d, src is %d, des is %d\n", t,indori-1,leafCi);
            int *path = new int[indori];
            for(int i = 0; i <indori; i++)
                 path[0] = -1;
                traced_length = graph_shortest_paths(conn,indori-1,leafCi,indori,path);
            if(t>1)
            {
                clu_current = new double[traced_length*3];
                for(int i = 0; i < traced_length; i++)
                {
                    clu_current[i*3+0] = Xnew[path[traced_length-1-i]*3+0];
                    clu_current[i*3+1] = Xnew[path[traced_length-1-i]*3+1];
                    clu_current[i*3+2] = Xnew[path[traced_length-1-i]*3+2];

                }

                double *Dbr2br = new double[traced_length];
                int *Mbr2br = new int[traced_length];
                dist2pwlines(clu_current,clu_traced,Dbr2br,Mbr2br,traced_length,traced_length_all);
                int biffLocation;
                for(int i = traced_length-1; i>=0; i--)
                {
                    if(Dbr2br[i] < 0.1)
                    {
                        biffLocation = i;
                        break;
                    }
                }
                int lineSeg = Mbr2br[biffLocation];
                double Q1_x,Q1_y,Q1_z,Q2_x,Q2_y,Q2_z,P_x,P_y,P_z;
                Q1_x = clu_traced[lineSeg*3+0];     Q1_y = clu_traced[lineSeg*3+1];     Q1_z = clu_traced[lineSeg*3+2];
                Q2_x = clu_traced[(lineSeg+1)*3+0]; Q2_y = clu_traced[(lineSeg+1)*3+1]; Q2_z = clu_traced[(lineSeg+1)*3+2];
                P_x = clu_current[biffLocation*3+0];     P_y = clu_current[biffLocation*3+1];     P_z = clu_current[biffLocation*3+2];

                double u = ((P_x - Q1_x)*(Q2_x - Q1_x)+(P_y - Q1_y)*(Q2_y - Q1_y)+(P_z - Q1_z)*(Q2_z - Q1_z))/(pow(Q2_x - Q1_x,2.0)+pow(Q2_y - Q1_y,2.0)+pow(Q2_z - Q1_z,2.0));
                double B[3];
                B[0] = u*(Q2_x - Q1_x) + Q1_x;
                B[1] = u*(Q2_y - Q1_y) + Q1_y;
                B[2] = u*(Q2_z - Q1_z) + Q1_z;

                double Bxnew[3];
                Bxnew[0] = 0; Bxnew[1] = 0; Bxnew[2] = 0;
                double dist_min = 0;
                for(int i = 0; i < indori; i++)
                {
                    double x1 = Xnew[i*3+0];
                    double y1 = Xnew[i*3+1];
                    double z1 = Xnew[i*3+2];
                    if(sqrt(pow(x1-B[0],2.0)+pow(y1-B[1],2.0)+pow(z1-B[2],2.0))>dist_min)
                    {
                        dist_min = sqrt(pow(x1-B[0],2.0)+pow(y1-B[1],2.0)+pow(z1-B[2],2.0));
                        Bxnew[0] = x1;
                        Bxnew[1] = y1;
                        Bxnew[2] = z1;
                    }
                }

                double *curr_trace = new double[(traced_length - biffLocation)*3];
                int j = 0;
                for(int i = biffLocation; i <traced_length; i++)
                {
                    curr_trace[j*3+0] = Xnew[path[traced_length-1-i]*3+0];
                    curr_trace[j*3+1] = Xnew[path[traced_length-1-i]*3+1];
                    curr_trace[j*3+2] = Xnew[path[traced_length-1-i]*3+2];
                    j++;
                }

                int flag = 0;
                for(int i = 0; i < traced_length_all; i++)
                {
                    double x1 = clu_traced[i*3+0];
                    double y1 = clu_traced[i*3+1];
                    double z1 = clu_traced[i*3+2];
                    if(sqrt(pow(x1-Bxnew[0],2.0)+pow(y1-Bxnew[1],2.0)+pow(z1-Bxnew[2],2.0)) == 0)
                    {
                        flag = 1;
                        break;
                    }
                }

                if(flag = 0)
                {
                    int traced_length_all_old = traced_length_all;
                    traced_length_all = traced_length_all + 1+ (traced_length - biffLocation) +1;
                    for(int i = 0; i< lineSeg; i++)
                    {
                        clu_traced_updated[i*3+0] = clu_traced[i*3 + 0];
                        clu_traced_updated[i*3+1] = clu_traced[i*3 + 1];
                        clu_traced_updated[i*3+2] = clu_traced[i*3 + 2];
                    }
                    clu_traced_updated[lineSeg*3+0] =  Bxnew[0];
                    clu_traced_updated[lineSeg*3+1] =  Bxnew[1];
                    clu_traced_updated[lineSeg*3+2] =  Bxnew[2];

                    for(int i = lineSeg+1; i< traced_length_all_old+1; i++)
                    {
                        clu_traced_updated[i*3+0] = clu_traced[(i-1)*3 + 0];
                        clu_traced_updated[i*3+1] = clu_traced[(i-1)*3 + 1];
                        clu_traced_updated[i*3+2] = clu_traced[(i-1)*3 + 2];
                    }

                    clu_traced_updated[(traced_length_all_old+1)*3+0] = 0;
                    clu_traced_updated[(traced_length_all_old+1)*3+1] = 0;
                    clu_traced_updated[(traced_length_all_old+1)*3+2] = 0;


                    int j = 0;
                    for(int i = traced_length_all_old+2; i< traced_length_all; i++)
                    {
                        clu_traced_updated[i*3+0] = curr_trace[j*3 + 0];
                        clu_traced_updated[i*3+1] = curr_trace[j*3 + 1];
                        clu_traced_updated[i*3+2] = curr_trace[j*3 + 2];
                        j++;
                    }


                }

                else
                {
                    int traced_length_all_old = traced_length_all;
                    traced_length_all = traced_length_all + (traced_length - biffLocation) +1;
                    clu_traced_updated = new double[traced_length_all*3];
                    for(int i = 0; i< traced_length_all_old; i++)
                    {
                        clu_traced_updated[i*3+0] = clu_traced[i*3 + 0];
                        clu_traced_updated[i*3+1] = clu_traced[i*3 + 1];
                        clu_traced_updated[i*3+2] = clu_traced[i*3 + 2];
                    }
                    clu_traced_updated[traced_length_all_old*3+0] = 0;
                    clu_traced_updated[traced_length_all_old*3+1] = 0;
                    clu_traced_updated[traced_length_all_old*3+2] = 0;

                    int j = 0;
                    for(int i = traced_length_all_old+1; i< traced_length_all; i++)
                    {
                        clu_traced_updated[i*3+0] = curr_trace[j*3 + 0];
                        clu_traced_updated[i*3+1] = curr_trace[j*3 + 1];
                        clu_traced_updated[i*3+2] = curr_trace[j*3 + 2];
                        j++;
                    }
                }

                if(clu_traced) {delete []clu_traced; clu_traced = 0 ;}
                clu_traced = new double[traced_length_all*3];
                for(V3DLONG i = 0; i < traced_length_all*3; i++)
                    clu_traced[i] = clu_traced_updated[i];
                if(clu_traced_updated) {delete []clu_traced_updated; clu_traced_updated = 0 ;}

                if(Dbr2br) {delete []Dbr2br; Dbr2br = 0;}
                if(Mbr2br) {delete []Mbr2br; Mbr2br = 0;}
                if(curr_trace) {delete []curr_trace; curr_trace = 0;}

            }
            else
            {
                traced_length_all = traced_length;
                clu_traced = new double[traced_length_all*3];
                for(int i = 0; i < traced_length_all; i++)
                {
                    clu_traced[i*3+0] = Xnew[path[traced_length_all-1-i]*3+0];
                    clu_traced[i*3+1] = Xnew[path[traced_length_all-1-i]*3+1];
                    clu_traced[i*3+2] = Xnew[path[traced_length_all-1-i]*3+2];

                }


            }
            if(path) {delete []path; path =0;}
            printf("traced length is %d\n",traced_length_all);
            printf("indCi_length is %d\n",indCi_length);

            clu_indCi = new double[indCi_length*3];
            int j = 0;
            for(int i = 0; i < indori; i++)
            {
                if(cidx[i] == 1)
                {
                    clu_indCi[j*3+0] = Xnew[i*3+0];
                    clu_indCi[j*3+1] = Xnew[i*3+1];
                    clu_indCi[j*3+2] = Xnew[i*3+2];
                    j++;
                }

            }

            double *D = new double[indCi_length];
            int *M = new int[indCi_length];
            dist2pwlines(clu_indCi,clu_traced,D,M,indCi_length,traced_length_all);
            if(M) {delete []M; M = 0;}
            t++;
            if(traced_length >2)
            {
                j = 0;
                for(int i = 0; i < indori; i++)
                {
                    if(cidx[i] == 1)
                    {
                        if( D[j] < synapseTHR)
                           cidx[i] = -1;
                        j++;
                    }
                }
                if(D) {delete []D; D = 0;}
             }
            else
            {
              if(D) {delete []D; D = 0;}
              break;
            }
        }
        else
            break;
    }

    if(clu_indCi) {delete []clu_indCi; clu_indCi=0;}
    if(clu_traced_updated) {delete []clu_traced_updated; clu_traced_updated=0;}
    if(clu_current) {delete []clu_current; clu_current = 0;}
      //NeutronTree structure

    NeuronTree PSF_swc;
    QList <NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();

    NeuronSWC S;

    S.n 	= 1;
    S.type 	= 3;
    S.x 	= clu_traced[0];
    S.y 	= clu_traced[1];
    S.z 	= clu_traced[2];
    S.r 	= 1;
    S.pn = -1;

    listNeuron.append(S);
    hashNeuron.insert(S.n, listNeuron.size()-1);
    int ID = 2;
    int flag = 0;
    for(int i = 2; i < traced_length_all; i++)
    {

       if(clu_traced[i*3 + 0] == 0)
       {
           flag = 1;
           continue;
       }
       else
       {
           S.n 	= ID;
           S.type 	= 3;
           S.x 	= clu_traced[i*3 + 0];
           S.y 	= clu_traced[i*3 + 1];
           S.z 	= clu_traced[i*3 + 2];
           S.r 	= 1;
           if(flag ==1) S.pn = -1;
           else S.pn = ID-1;
           listNeuron.append(S);
           hashNeuron.insert(S.n, listNeuron.size()-1);
           flag = 0;
           ID++;
       }
    }
    PSF_swc.n = -1;
    PSF_swc.on = true;
    PSF_swc.listNeuron = listNeuron;
    PSF_swc.hashNeuron = hashNeuron;

    writeSWC_file(output_name.toStdString().c_str(),PSF_swc);

    if(clu_traced) {delete []clu_traced; clu_traced=0;}

     return;
}


int graph_shortest_paths(double *conn, int src, int des, int n, int *path)
{

    int index;
    dijkstra_path(conn,n,src,des,path,index);
    return index+1;
}


void dijkstra_path(double *graph, int V, int src, int des, int *path, int &index)
{
     double *dist = new double[V];     // The output array.  dist[i] will hold the shortest
                      // distance from src to i
     int *P = new int[V];
     bool *sptSet = new bool[V]; // sptSet[i] will true if vertex i is included in shortest
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

    if(dist) {delete []dist; dist = 0;}
    if(P) {delete []P; P = 0;}
    if(sptSet) {delete []sptSet; sptSet = 0;}
    return;
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

int graph_conn_comp(double *conn,int *ctrs,int indori)
{
    std::queue<int> myqueue;
    for(V3DLONG i = 0; i<indori; i++)
          ctrs[i] = -1;
    int group = 1;
    for(V3DLONG i = 0; i<indori; i++)
    {
         if(ctrs[i] == -1)
         {
            ctrs[i] = group;
            myqueue.push(i);
            do
            {
                int current = myqueue.front();
                myqueue.pop();
                for(V3DLONG j = 0; j<indori; j++)
                {
                    if(ctrs[j] == -1 && conn[current*indori+j]!=0)
                    {
                        ctrs[j] = group;
                        myqueue.push(j);
                     }
                }
             }while(!myqueue.empty());
             group++;
         }
         else
            continue;
    }
    return group-1;

}

void dist2pwlines(double *X,double *Y,double *D,int *M, int X_length,int Y_length)
{
    double eps =  2.2204e-16;
    double Q1_x,Q1_y,Q1_z,Q2_x,Q2_y,Q2_z,P_x,P_y,P_z;
    for(int i = 0; i < X_length; i++)
    {
        D[i] = INFINITY;
        M[i] = 0;
    }
    for(int i = 1; i < Y_length -1; i++)
    {
        Q1_x = Y[i*3+0];     Q1_y = Y[i*3+1];     Q1_z = Y[i*3+2];
        Q2_x = Y[(i+1)*3+0]; Q2_y = Y[(i+1)*3+1]; Q2_z = Y[(i+1)*3+2];
        if(Q1_x*Q1_y*Q1_z*Q2_x*Q2_y*Q2_z == 0)
            continue;
        for(int j = 0; j < X_length; j++)
        {
            P_x = X[j*3+0];     P_y = X[j*3+1];     P_z = X[j*3+2];
            double u = ((P_x - Q1_x)*(Q2_x - Q1_x)+(P_y - Q1_y)*(Q2_y - Q1_y)+(P_z - Q1_z)*(Q2_z - Q1_z))/(pow(Q2_x - Q1_x,2.0)+pow(Q2_y - Q1_y,2.0)+pow(Q2_z - Q1_z,2.0));
            if(u > 1 + sqrt(eps) || u < -sqrt(eps))
                continue;
            else
            {
                   double a0 = Q2_x-Q1_x;
                   double a1 = Q2_y-Q1_y;
                   double a2 = Q2_z-Q1_z;
                   double b0 = P_x-Q1_x;
                   double b1 = P_y-Q1_y;
                   double b2 = P_z-Q1_z;
                   double d =  sqrt(pow(a1*b2-a2*b1,2.0)+pow(a2*b0-a0*b2,2.0)+pow(a0*b1-a1*b0,2.0))/sqrt(pow(Q2_x - Q1_x,2.0)+pow(Q2_y - Q1_y,2.0)+pow(Q2_z - Q1_z,2.0));
                   if(d > D[j])
                       continue;
                   else
                   {
                       if(isnan(u))
                       {
                           D[j] = 0;
                           M[j] = i;
                       }
                       else
                       {
                           D[j] = d;
                           M[j] = i;
                       }
                   }
            }
        }
    }

}
