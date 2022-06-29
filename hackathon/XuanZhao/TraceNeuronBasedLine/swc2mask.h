#ifndef SWC2MASK_H
#define SWC2MASK_H

#include <cmath>
#include <set>
#include <vector>

#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"

#define zx_dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))

using namespace std;

vector<MyMarker*> getLeaf_marker(vector<MyMarker*> & inmarkers);

bool getMarkerBetween(vector<MyMarker> &allmarkers, MyMarker m1, MyMarker m2);
bool swcTomask(unsigned char * &outimg1d, vector<MyMarker * > & inswc, long sz0, long sz1, long sz2);


#endif // SWC2MASK_H
