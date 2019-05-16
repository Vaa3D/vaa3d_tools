#ifndef FUNCTION__H
#define FUNCTION__H

#include <v3d_interface.h>
#include <vector>
using namespace std;

int getNum_file(const QString &dirPath);
vector<NeuronSWC> tip_detection1(NeuronTree &nt);

#endif // FUNCTION__H
