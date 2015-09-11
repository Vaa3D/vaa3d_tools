#ifndef __FUNC_H__
#define __FUNC_H__

#include <v3d_interface.h>
using namespace std;
int simple_func();

bool* shared_lib_func(unsigned char* raw, V3DLONG total_bytes,V3DLONG unit_bytes, V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG t,int paran, double* para, string fileDir);

#endif
