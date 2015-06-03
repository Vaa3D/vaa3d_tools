#ifndef __SEG_WEIGHT_H__
#define __SEG_WEIGHT_H__

#include <math.h>
#include <vector>
#include "my_surf_objs.h"
using namespace std;
#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif
#define MAX_DOUBLE 1.7976931348623158e+308

double seg_dist(vector<MyMarker*> & seg1, vector<MyMarker*> & seg2);
double seg_dist(vector<MyMarker*> & seg1, vector<MyMarker*> & seg2, vector<pair<int, int> > & matching_res);
double seg_weight(vector<MyMarker*> & seg1, vector<MyMarker*> & seg2);
double seg_weight(vector<MyMarker*> & seg1, vector<MyMarker*> & seg2, vector<pair<int, int> > & matching_res);


double edit_dist(vector<MyMarker*> & seg1, vector<MyMarker*> & seg2, int s1, int e1, int s2, int e2);
double euc_dist(vector<MyMarker*> & seg1, vector<MyMarker*> & seg2, int s1, int e1, int s2, int e2);


#endif
