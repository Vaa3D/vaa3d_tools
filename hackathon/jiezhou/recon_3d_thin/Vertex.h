#ifndef VERTEX_H
#define VERTEX_H

#include <vector>

using namespace std;

class pixPoint;
class Edge;

class Vertex{
public:
	vector <pixPoint*> points;
	vector <Edge*> edges;
	int distance;
	Edge* parent;

	Vertex();
	bool contains(pixPoint*);
};

#endif
