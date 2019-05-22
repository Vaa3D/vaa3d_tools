/* [response orientation nms filterBank] = steerableDetector3D(image, filterOrder, sigma);
 *
 * (c) Francois Aguet, 30/08/2012 (last modified 09/02/2012).
 *
 * Compilation:
 * Mac/Linux: mex -I/usr/local/include -I../../mex/include /usr/local/lib/libgsl.a /usr/local/lib/libgslcblas.a steerableDetector3D.cpp
 * Windows: mex COMPFLAGS="$COMPFLAGS /TP /MT" -I"..\..\..\extern\mex\include\gsl-1.14" -I"..\..\mex\include" "..\..\..\extern\mex\lib\gsl.lib" "..\..\..\extern\mex\lib\cblas.lib" -output steerableDetector3D steerableDetector3D.cpp
 */


#include <cstring>
#include <gsl_include/gsl_poly.h>
#include <gsl_include/gsl_math.h>
#include <gsl_include/gsl_eigen.h>
#include <gsl_include/gsl_cblas.h>
#include <algorithm>
#include <vector>
#include <fstream>
#include <string>
#include <math.h>
#include "stackutil.h"
#include "../../../v3d_main/jba/newmat11/newmatap.h"
#include "../../../v3d_main/jba/newmat11/newmatio.h"

#include "convolver3D.h"


double zhi_abs(double num);
int swapthree(double& dummya, double& dummyb, double& dummyc);

class Filter {
    
public:
    Filter(const double voxels[], const int nx, const int ny, const int nz, const int M, const double sigma);
    Filter(const double voxels[], const int nx, const int ny, const int nz, const int M, const double sigma, const double zxRatio);
    ~Filter();
    void init();
    
    double* getResponse();
    double** getOrientation();
    double* getNMS();
    
private:
    const double* voxels_;
    double* response_;
    double** orientation_;
    double* nms_;
    int nx_, ny_, nz_;
    int M_;
    double sigma_, sigmaZ_;
    int N_;
    double alpha_, sign_, c_;
    
    double *gxx_, *gxy_, *gxz_, *gyy_, *gyz_, *gzz_;
    
    void calculateTemplates();
    double interpResponse(const double x, const double y, const double z);
    static int mirror(const int x, const int nx);
    void normalize(double v[], const int k);
    void cross(const double v1[], const double v2[], double r[]);
    
    void computeCurveNMS();
    void computeSurfaceNMS();
    void run();
};
