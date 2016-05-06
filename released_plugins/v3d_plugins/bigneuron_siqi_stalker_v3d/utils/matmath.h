#include <vector>
#include "stackutil.h"

typedef float PRECISION;
typedef std::vector<PRECISION> vectype;

// Generate a values in linear space between a and b
vectype linspace(const PRECISION a, const PRECISION b, const int n);

// Repeat a 1d-vector to a 2D matrix
vectype repmat1d(vectype, const int, const int);

// 2D matrix transpose
vectype transpose(vectype, const int, const int);

// Spherical coordinates to Cartisian
void sph2cart(vectype thvec, vectype phivec, vectype r, vectype * xvec, vectype * yvec, vectype * zvec );

// Cartisian coordinates to spherical
void cart2sph(vectype xvec, vectype yvec, vectype zvec, vectype* thvec, vectype* phivec, vectype* r );

void savepts2csv(vectype a, vectype b, vectype c, const char* filename); // Save a list of points to a text file

void savepts2csvfourva(vectype a, vectype b, vectype c, vectype d, const char* filename); // Save a list of points to a text file


// Calculate the value of the cosine of the angle between two spherical vectors
// Larger cosine between these two angles means closer these two angles are
// Ref: http://math.stackexchange.com/questions/231221/great-arc-distance-between-two-points-on-a-unit-sphere
vectype sphveccos(vectype th1, vectype phi1, vectype th2, vectype phi2);

vectype eucdistance2center(PRECISION x, PRECISION y, PRECISION z, vectype lx, vectype ly, vectype lz);

// find the radius of the seed
int appradius(unsigned char * inimg1d, V3DLONG * sz,  double thresh, int location_x, int location_y, int location_z);

// vector projecttion 
void vecproj(vectype * u, const vectype v);

void vecnorm(vectype * u, const vectype v);

float constrain(float in, float low, float high);

void seedadjust(vectype * seedx, vectype * seedy, vectype * seedz);
