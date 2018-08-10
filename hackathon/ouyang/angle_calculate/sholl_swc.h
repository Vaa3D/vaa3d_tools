/*
 *  sholl_func.cpp
 *  core functions for sholl neuron swc
 *  2018-06-26: by OuYang Qiang
 */

#ifndef __SHOLL_SWC_H_
#define __SHOLL_SWC_H_

#include <QtGlobal>
//#include <math.h>
//#include <unistd.h> //remove the unnecessary include file. //by PHC 20131228
#include "basic_surf_objs.h"
//#include <string.h>
#include <stdio.h>
//#include <algorithm>
//#include <vector>
#include <iostream>
using namespace std;

#ifndef VOID
#define VOID 1000000000
#endif


bool combine_linker(vector<QList<NeuronSWC> > & linker, QList<NeuronSWC> & combined);
vector<double> ShollSWC(QList<NeuronSWC> & neuron);

#endif
