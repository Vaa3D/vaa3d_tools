#ifndef TRACEUTIL_H
#define TRACEUTIL_H
#include "../../ngtypes/basetypes.h"
template<typename T> class Volume;

struct TraceUtil{
    static void GetGradientVectorFlowForTrace(const Volume<double> &sphereRayWet, Volume<double> &sphereRayGradient);
    static void SmoothGradientCurves(const std::vector<double> &ttk1, std::vector<double> &S_diffttk);
    static void ExtractSubRegionOP(const Vec3d&, Volume<NGCHAR>&, VectorVec3d&);//Ã·»°«¯”Ú
    static void ExtractSubRegionAboveThrev(const VectorVec3d&, VectorVec3d&, Volume<NGCHAR>&);//extract area of which value is greater than threshold
    static int sign(double);
    static void GetPartVectorVec3d(const VectorVec3d&, int minId, int maxId, VectorVec3d&);
    static void GetPartVectorVec3d(const VectorVec5d&, int minId, int maxId, VectorVec3d&);
    static bool IsEqualDouble(double a, double b);
};

#endif // TRACEUTIL_H
