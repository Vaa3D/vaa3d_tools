#include <stdio.h>
#include "kgraph.h"
#include "kgraph.cpp"
#include "maxflow.cpp"

using namespace maxflow;

int main()
{
	printf("Maxflow\n");

	typedef Graph<int,int,int> GraphType;
	GraphType *g = new GraphType(/*estimated # of nodes*/ 2, /*estimated # of edges*/ 1); 

	g -> add_node(); 
	g -> add_node(); 

	g -> add_tweights( 0,   /* capacities */  1, 5 );
	g -> add_tweights( 1,   /* capacities */  2, 6 );
	g -> add_edge( 0, 1,    /* capacities */  3, 4 );

	int flow = g -> maxflow();
	//int flow;

         g->print_graph_size();

	printf("Flow = %d\n", flow);
	printf("Minimum cut:\n");
	if (g->what_segment(0) == GraphType::SOURCE)
		printf("node0 is in the SOURCE set\n");
	else
		printf("node0 is in the SINK set\n");
	if (g->what_segment(1) == GraphType::SOURCE)
		printf("node1 is in the SOURCE set\n");
	else
		printf("node1 is in the SINK set\n");

	delete g;

	return 0;
}

