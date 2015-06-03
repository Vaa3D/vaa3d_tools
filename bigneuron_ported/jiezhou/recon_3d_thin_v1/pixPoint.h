/**
  Written for BigNeuron Hackathon.
  Edward Hottendorf 

  Modification made by Jie Zhou for porting to Vaa3D. 
  June 2015

**/

#ifndef PIXPOINT_H
#define PIXPOINT_H

//using namespace std;

class pixPoint {
public:
	int x;
	int y;
	int z;

	int visitNum;
	int parent;

	pixPoint();
	pixPoint(int , int, int);


};

#endif