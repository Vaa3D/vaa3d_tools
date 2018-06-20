#ifndef _NFMAIN_H
#define _NFMAIN_H
#include "basic_surf_objs.h"
#include "v3d_interface.h"

void test_func(int aaa, double bbbb);
void nf__main(V3DPluginCallback2 &callback, QWidget *parent);
typedef std::vector<int> SWCsegment;
int calSmoothedPN(int pn,NeuronTree & nt_ori,SWCsegment &segm );
#endif