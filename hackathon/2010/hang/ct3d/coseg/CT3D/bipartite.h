#ifndef BIPARTITE_H
#define BIPARTITE_H
#include <vector>
using namespace std;

float bipartite_matching(vector<float>& weights, int nrows, int ncols, vector<int>& ids1, vector<int>& ids2);

float bipartite_matching(vector<float>& weights, int nrows, int ncols, vector<int>& ids);
#endif
