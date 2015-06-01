#include "PressureSampler.h"
#include "utils/matmath.h"
#include <vector>
#include <math.h>

using namespace std;
typedef std::vector<double> vectype;

PressureSampler::PressureSampler(int ndir, 
                                 int density,
                                 unsigned char * OriginalImg,
                                 unsigned char * BinaryImg,
                                 float * GVF,
                                 float radius): 
                                                   ndir(ndir),
                                                   density(density), 
                                                   OriginalImg(OriginalImg), 
                                                   BinaryImg(BinaryImg), 
                                                   GVF(GVF), 
                                                   radius(radius)
{
    this->baseth = new float[ndir];
    this->basephi = new float[ndir];
    this->lpressure = new float[ndir*density];
    this->GenSph();
}


PressureSampler::~PressureSampler(){
   if(this->baseth) delete baseth;
   if(this->basephi) delete basephi;
   if(this->lpressure) delete lpressure;
}


void PressureSampler::GenSph(){
// Uniformly pick directions on a unit sphere
// Ref: http://mathworld.wolfram.com/SpherePointPicking.html

    int nphi = (int) pow(((double)this->ndir / 2.0), 0.5);
    int nth = (int) 2 * nphi;
    //itk::Matrix<double, nphi, nth> uM;
    //itk::Matrix<double, nth, nphi> vM;

    vectype u = linspace(0, 1, nth);
    vectype v = linspace(0, 1, nphi);

    vectype repu = repmat1d(u, nphi, 2);

    vectype repv = repmat1d(v, nth, 2);
    vectype transv = transpose(repv, v.size(), nth);// Transpose repv matrix

    for (int i = 0; i < u.size() * nphi; i++)
        this->baseth[i] = 2 * M_PI * repu[i];

    for (int i = 0; i < v.size() * nth; i++)
        this->basephi[i] = acos(2 * transv[i] - 1);
}
