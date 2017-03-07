#ifndef EDGE_H
#define EDGE_H

#include <vector>

using namespace std;

class Vertex; 
class pixPoint;

class Edge{
public:
	Vertex* v1;
	Vertex* v2;
	vector<pixPoint*> slab;

	Edge();
	Vertex* getOppositeVertex(Vertex* startVertex);
};

#endif
