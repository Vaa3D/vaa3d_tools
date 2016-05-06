#include "Edge.h"
#include "Vertex.h"
#include "pixPoint.h"
#include <iostream>


Edge::Edge(){

}

Vertex* Edge::getOppositeVertex(Vertex* startVertex){
	if (startVertex == v1){
		return v2;
	}
	else if (startVertex == v2){
		return v1;
	}
	
	cerr << "There's been an error\n";
	return NULL;
}
