#include "Vertex.h"
#include "pixPoint.h"
#include "Edge.h"


Vertex::Vertex(){

}

bool Vertex::contains(pixPoint* pnt){

	for (int i = 0; i < points.size(); i++){
		if (pnt->x == points[i]->x && pnt->y == points[i]->y && pnt->z == points[i]->z){
			return true;
		}
	}
	return false;
}