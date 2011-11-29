/**
 *   ///////////////////////
 *   // MIN COST MAX FLOW //
 *   ///////////////////////
 *
 *   Authors: Frank Chu, Igor Naverniouk
 **/

/*********************
 * Min cost max flow * (Edmonds-Karp relabelling + Dijkstra)
 *********************
 * Takes a directed graph where each edge has a capacity ('cap') and a 
 * cost per unit of flow ('cost') and returns a maximum flow network
 * of minimal cost ('fcost') from s to t. USE THIS CODE FOR (MODERATELY)
 * DENSE GRAPHS; FOR VERY SPARSE GRAPHS, USE mcmf4.cpp.
 *
 * PARAMETERS:
 *      - cap (global): adjacency matrix where cap[u][v] is the capacity
 *          of the edge u->v. cap[u][v] is 0 for non-existent edges.
 *      - cost (global): a matrix where cost[u][v] is the cost per unit
 *          of flow along the edge u->v. If cap[u][v] == 0, cost[u][v] is
 *          ignored. ALL COSTS MUST BE NON-NEGATIVE!
 *      - n: the number of vertices ([0, n-1] are considered as vertices).
 *      - s: source vertex.
 *      - t: sink.
 * RETURNS:
 *      - the flow
 *      - the total cost through 'fcost'
 *      - fnet contains the flow network. Careful: both fnet[u][v] and
 *          fnet[v][u] could be positive. Take the difference.
 * COMPLEXITY:
 *      - Worst case: O(n^2*flow  <?  n^3*fcost)
 * FIELD TESTING:
 *      - Valladolid 10594: Data Flow
 * REFERENCE:
 *      Edmonds, J., Karp, R.  "Theoretical Improvements in Algorithmic
 *          Efficieincy for Network Flow Problems".
 *      This is a slight improvement of Frank Chu's implementation.
 **/
 
#include <cstring>
#include <climits>
#include <cmath>
#include <cassert>
#include <iostream>
#include <vector>
using namespace std;

// the maximum number of vertices + 1
#ifndef NN
#define NN 200
#endif

#ifndef EPSILON
#define EPSILON 0.00001
#endif

#define ABS(a) ((a)<0 ? -(a) :a)
// adjacency matrix (fill this up)
int cap[NN][NN];

// cost per unit of flow matrix (fill this up)
float cost[NN][NN];

// flow network and adjacency list
int fnet[NN][NN], adj[NN][NN], deg[NN];

// Dijkstra's successor and depth
int par[NN]; // par[source] = source;
float d[NN];

// Labelling function
float pi[NN];

#define CLR(a, x) memset( a, x, sizeof( a ) )
#define Inf (INT_MAX/2.0)

// Dijkstra's using non-negative edge weights (cost + potential)
#define Pot(u,v) (d[u] + pi[u] - pi[v])
bool dijkstra( int n, int s, int t )
{
    for( int i = 0; i < n; i++ ) d[i] = Inf, par[i] = -1;
    d[s] = 0;
    par[s] = -n - 1;

    while( 1 ) 
    {
        // find u with smallest d[u]
        int u = -1;
		bool found = false;
		float bestD = Inf;
        for( int i = 0; i < n; i++ ) {
			if( par[i] < 0 && d[i] < bestD ) {
				bestD = d[u = i];
				found = true;
			}
		}
        if( !found ) break;

        // relax edge (u,i) or (i,u) for all i;
        par[u] = -par[u] - 1;
        for( int i = 0; i < deg[u]; i++ )
        {
            // try undoing edge v->u      
            int v = adj[u][i];
            if( par[v] >= 0 ) continue;
            if( fnet[v][u] && d[v] > Pot(u,v) - cost[v][u] ) 
                d[v] = Pot( u, v ) - cost[v][u], par[v] = -u-1;
        
            // try edge u->v
            if( fnet[u][v] < cap[u][v] && d[v] > Pot(u,v) + cost[u][v] ) 
                d[v] = Pot(u,v) + cost[u][v], par[v] = -u - 1;
        }
    }
  
    for( int i = 0; i < n; i++ ) if( pi[i] < Inf ) pi[i] += d[i];
  
    return par[t] >= 0;
}
#undef Pot

int mcmf3( int n, int s, int t, float &fcost )
{
    // build the adjacency list
    CLR( deg, 0 );
    for( int i = 0; i < n; i++ )
    for( int j = 0; j < n; j++ ) 
        if( cap[i][j] || cap[j][i] ) adj[i][deg[i]++] = j;
        
    CLR( fnet, 0 );
    CLR( pi, 0 );
    int flow = 0;
	fcost = 0;
    
    // repeatedly, find a cheapest path from s to t
    while( dijkstra( n, s, t ) ) 
    {
        // get the bottleneck capacity
        int bot = INT_MAX;
        for( int v = t, u = par[v]; v != s; u = par[v = u] )
            bot = std::min(bot, fnet[v][u] ? fnet[v][u] : ( cap[u][v] - fnet[u][v] ));


        // update the flow network
        for( int v = t, u = par[v]; v != s; u = par[v = u] )
            if( fnet[v][u] ) { fnet[v][u] -= bot; fcost -= bot * cost[v][u]; }
            else { fnet[u][v] += bot; fcost += bot * cost[u][v]; }
    
        flow += bot;
    }
  
    return flow;
}

float bipartite_matching(vector<float>& weights, int nrows, int ncols, vector<int>& ids1, vector<int>& ids2)
{
	int numV = nrows + ncols + 4;
	if(numV > NN) {
		cerr<<"numV > 200"<<endl;
		return -1.0;
	}
	assert(weights.size() == nrows * ncols);
	float max_w = 0.0;
	vector<float>::iterator it = weights.begin();
	while(it != weights.end())
	{
		max_w = (*it) > max_w ? (*it):max_w;
		it++;
	}
	ids1.clear();
	ids2.clear();
	int numI = nrows;
	int numJ = ncols;
	// cost and cap of (s,XX)
	int a = 0;
	int b = 0;
	for(a = 0; a < numV; a++)
	{
		for(b = 0; b < numV; b++)
		{
			cost[a][b] = 0.0;
			cap[a][b] = 0;
		}
	}
	a = 0;
	for(int b = 1; b <= numI; b++)
	{
		cost[a][b] = 0.0;
		cap[a][b] = 1;
	}
	//b++; //b = numI , e_i -> b
	cost[a][b] = 0.0;
	cap[a][b] =  numJ;
	for(a = 1; a <= numI; a++)
	{
		for(b = numI + 2; b < numI + numJ +2; b++)
		{
			int index = (a - 1) * numJ + (b - numI - 2);
			cost[a][b] = max_w -  weights[index];
			cap[a][b] =  1;
		}
	}
	a = numI + 1; // ei -> a
	for(b = numI + 2; b < numI + numJ + 2; b++)
	{
		cost[a][b]  = max_w;
		cap[a][b] =  1;
	}
	// ei - ej
	b = numI + numJ + 2;
	cost[a][b] = max_w;
	cap[a][b] = ABS(numI - numJ);
	for(a = 1; a <= numI; a++)
	{
		cost[a][b] = max_w;
		cap[a][b]  = 1;
	}
	b = numI + numJ + 3; // t -> b
	for(a = numI + 2; a < numI + numJ +2; a++)
	{
		cost[a][b] = 0.0;
		cap[a][b]  =1;
	}
	a = numI + numJ + 2;
	cost[a][b] =  0.0;
	cap[a][b] =  numI;
	float fcost;
	int flow = mcmf3(numV, 0, numI + numJ + 3, fcost);
	float sum_weight = 0.0;
	for(a = 1; a <= numI; a++)
	{
		int i = a - 1;
		for(b = numI + 2; b < numI + numJ + 2; b++)
		{
			int j = b - numI - 2;
//			if(fnet[a][b] != fnet[b][a])
//			{
//				cout<<"fnet["<<a<<"]["<<b<<"] = "<<fnet[a][b]<<" fnet["<<b<<"]["<<a<<"] = "<<fnet[b][a]<<endl;
//			}
			if((fnet[a][b] - fnet[b][a] == 1 || fnet[a][b] - fnet[b][a] == -1) && weights[i * ncols + j] > EPSILON) 
			{
				ids1.push_back(i);
				ids2.push_back(j);
				sum_weight += weights[i * ncols + j];
			}
		}
	}
	return sum_weight;
}

float bipartite_matching(vector<float>& weights, int nrows, int ncols, vector<int>& ids)
{
	ids.clear();
	vector<int> ids1, ids2;
	float result =bipartite_matching(weights, nrows, ncols, ids1, ids2);
	assert(ids1.size() == ids2.size());
	int result_num = ids1.size();
	for(int i = 0; i < result_num; i++)
	{
		ids.push_back(ids1[i]);
		ids.push_back(ids2[i]);
	}
	return result;
}
