#ifndef __VN_IMGPREPROCESS__
#define __VN_IMGPREPROCESS__

class PressureSampler
{
public:

    PressureSampler(int ndir, 
    	             int density,
    	             unsigned char * OriginalImg,
    	             unsigned char * BinaryImg,
    	             float * GVF,
    	             float radius = 0);
    ~PressureSampler();
	void RandRotateSph();
	void RandSample(int x, int y, int z);
	float *  GetBaseTh();
	float *  GetBasePhi();
	float *  GetPeakTh();
	float *  GetPeakPhi();

private:
	unsigned char * OriginalImg;
	unsigned char * BinaryImg;
	float * GVF;
    float radius;	
	int ndir; // Number of directions
    float density; // The density of the sampled points on each sample plane
	float * baseth; // The theta values of the base vectors
	float * basephi; // The theta values of the base vectors
    float * lpressure; // The pressure sampled at each direction
    float * SampleVoxels(int * lx, int * ly, int * lz, int sz);
	void GenSph(); // Generate the base spherical directions
	void FindVoxel2Sample(float x, float y, float z, float th, float phi);
};

#endif