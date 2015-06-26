#ifndef _APPLY_TRANSFORM_FUNC_H_
#define _APPLY_TRANSFORM_FUNC_H_

#include <v3d_interface.h>
#include <newmat.h>
#include <basic_surf_objs.h>

NeuronTree apply_transform(NeuronTree * nt, Matrix  trans);
bool apply_transform_to_swc(const V3DPluginArgList & input, V3DPluginArgList & output);

#endif

