#ifndef SWC2MASK_H
#define SWC2MASK_H

#include <cmath>
#include <set>
#include <vector>

#include "my_surf_objs.h"

using namespace std;

vector<MyMarker*> getLeaf_marker(vector<MyMarker*> & inmarkers);

bool getMarkerBetween(vector<MyMarker> &allmarkers, MyMarker m1, MyMarker m2);
bool swcTomask(unsigned char * &outimg1d, vector<MyMarker * > & inswc, long sz0, long sz1, long sz2);

#endif // SWC2MASK_H
