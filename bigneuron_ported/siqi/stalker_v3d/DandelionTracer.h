#ifndef __DANDELIONTRACER__
#define __DANDELIONTRACER__

#include "lib/PointOperation.h"
#include "lib/ImageOperation.h"
#include "lib/SnakeOperation.h"
#include "PressureSampler.h"
#include <vector>

using namespace std;


class DandelionTree
{
public:
	Point3D root;
    PointList3D nodes;
	std::vector<int> parents;
	std::vector<float> radius;
	std::vector<int> id;

private:

};


// The dandelion tracer
class Dandelion
{
public:
	Dandelion(PressureSampler sampler, Point3D position, float step,
		      float fromth, float fromphi, LabelImagePointer wallimg,
		      DandelionTree * tree);
	vector<Dandelion*> walk();
	Point3D GetPosition();
	DandelionTree GetTree();
	void EstimateRadius();

private:
	float step;
	Point3D position;// position
    PressureSampler sampler;
    float fromth = -1;
    float fromphi = -1;
    DandelionTree* tree;
    ImagePointer srcimg;
    LabelImagePointer wallimg;
};


// Serial version for the moment
class DandelionTracer
{
public:    
	DandelionTracer(GradientImagePointer gvf, LabelImagePointer wallimg, PointList3D seeds); 
	void StartTracing();

private:
	void InitialiseDandelions(); // Initialise dandelions from the original seeding positions
	vector<Dandelion*> dandelions;
    ImagePointer votingimg;
    ImagePointer gvf; // The raw voxel image used for tracing
    LabelImagePointer wallimg;
};

#endif