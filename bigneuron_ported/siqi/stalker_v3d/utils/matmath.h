#include <vector>
#include "stackutil.h"

typedef std::vector<float> vectype;

// Generate a values in linear space between a and b
vectype linspace(const double a, const double b, const int n);

// Repeat a 1d-vector to a 2D matrix
vectype repmat1d(vectype, const int, const int);

// 2D matrix transpose
vectype transpose(vectype, const int, const int);

// Spherical coordinates to Cartisian
void sph2cart(vectype thvec, vectype phivec, vectype r, vectype * xvec, vectype * yvec, vectype * zvec );

// Cartisian coordinates to spherical
void cart2sph(vectype xvec, vectype yvec, vectype zvec, vectype* thvec, vectype* phivec, vectype* r );

void savepts2csv(vectype a, vectype b, vectype c, const char* filename); // Save a list of points to a text file

vectype eucdistance2center(float x, float y, float z, vectype lx, vectype ly, vectype lz);
// find the radius of the seed
int appradius(unsigned char * inimg1d, V3DLONG * sz,  double thresh, int location_x, int location_y, int location_z);
