/* eswc_converter_func.h
 * This is a plugin to convert between swc and eswc.
 * 2012-02-16 : by Yinan Wan
 */
 
#ifndef __ESWC_CONVERTER_FUNC_H__
#define __ESWC_CONVERTER_FUNC_H__

#include <v3d_interface.h>
#include "v3d_message.h"
#include "eswc_core.h"
#include <vector>
#include <iostream>
using namespace std;

int swc2eswc_io(V3DPluginCallback2 &callback, QWidget *parent);
bool swc2eswc_io(const V3DPluginArgList & input, V3DPluginArgList & output);
int eswc2swc_io(V3DPluginCallback2 &callback, QWidget *parent);
bool eswc2swc_io(const V3DPluginArgList & input, V3DPluginArgList & output);
int check_eswc_io(V3DPluginCallback2 &callback, QWidget *parent);
bool check_eswc_io(const V3DPluginArgList & input, V3DPluginArgList & output);

bool export_eswc(NeuronTree & input, vector<V3DLONG> seg_id, vector<V3DLONG> seg_layer, const char* infile_name, const char* outfile_name);
bool export_swc(NeuronTree & neuron, const char* filename);
#endif

