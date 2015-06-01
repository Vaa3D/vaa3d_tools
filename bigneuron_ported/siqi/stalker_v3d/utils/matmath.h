#include <vector>

typedef std::vector<double> vectype;

vectype linspace(const double a, const double b, const int n);

//void sph2cart(float * lth, float * lphi, lx, ly, lz, int sz);

//void cart2sph(float * lx, float * ly, float * lz, int sz);

vectype repmat1d(vectype, const int, const int);

vectype transpose(vectype, const int, const int);