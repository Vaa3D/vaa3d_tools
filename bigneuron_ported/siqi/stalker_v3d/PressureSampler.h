#pragma once
#ifndef __PRESSSAMPLER__
#define __PRESSSAMPLER__
#include <vector>
#include "lib/ImageOperation.h"
#include "lib/PointOperation.h"

using namespace std;
typedef vector<float> vectype;
typedef map<float*, float> pressuremaptype;


#ifndef MAX
#define MAX(x,y) (x > y ? (x) : (y))
#endif


// For caching the direction neighbours on the sphere
struct neighbours{
       int idx; 
       vector<int> neighbouridx;
    };

    
class PressureSampler
{
public:

    PressureSampler(int ndir, 
    	             int density,
    	             ImagePointer OriginalImg,
    	             GradientImagePointer GVF,
    	             float radius = 10);
    ~PressureSampler();
	void RandRotateSph();
	void RandSample(); // Randomly sample the pressure at specific postion (x,y,z)
	void SetNDir(int ndir);
	vectype GetBaseTh();
	vectype GetBasePhi();
	vectype GetPeakTh();
	vectype GetPeakPhi();
    float radius = 0;
    float x, y, z;// position
    void HalfSphere(float inputth, float inputphi, vectype * binaryth, vectype * binaryphi);
    float GetRadius(); // Calculate the radius at the current position using the raw image
	void UpdatePosition(float x, float y, float z);
    void NextMove(float step); //This function just makes robot move one step
	friend void TestPressureSampler(ImagePointer OriginalImage, GradientImagePointer GVF, LabelImagePointer wallimg,
 PointList3D seeds, vectype * xpfinal, vectype * ypfinal, vectype * zpfinal, vectype * pn, vectype * rfinal, vectype * sn);
    friend void Trace(ImagePointer OriginalImage, GradientImagePointer GVF, LabelImagePointer wallimg,
 PointList3D seeds, vectype * xpfinal, vectype * ypfinal, vectype * zpfinal, vectype * pn, vectype * rfinal, vectype * sn);

private:
	ImagePointer OriginalImg;
	GradientImagePointer GVF;
	int ndir; // Number of directions
    int density; // The density of the sampled points on each sample plane
	vectype baseth, originbaseth; // The theta values of the base vectors
	vectype basephi, originbasephi; // The theta values of the base vectors
	vectype peakth; // The theta values of the peaks
	vectype peakphi; // The theta values of the peaks
    vectype lpressure; // The pressure sampled at each direction
    float Moment(vectype v, vectype x, vectype y, vectype z); // Sample the distortion energy at each direction
    pressuremaptype pressuremap;
    void SampleVoxels(vectype, vectype, vectype); // Sample the distortion energy at each direction
    vector<GradientPixelType> GetGradientAtIndex(vector<int> x, vector<int> y, vector<int> z);
	void GenSph(); // Generate the base spherical directions
    vector<int> FindPeaks(); // Return the indices of the peak directions 
    void FindVoxel2Sample(float th,
                          float phi, vectype * outx, vectype* outy,
                          vectype* outz);
    const int FindDirIdx(float th, float phi);
    vector<int> FindSphNeighbours(int i);
    vector<neighbours> dirneighbours;
};

#endif