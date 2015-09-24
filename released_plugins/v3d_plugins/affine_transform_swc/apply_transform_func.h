#ifndef _APPLY_TRANSFORM_FUNC_H_
#define _APPLY_TRANSFORM_FUNC_H_

#include <v3d_interface.h>
#include <newmat.h>
#include <basic_surf_objs.h>


struct unit_vector{
    double x;
    double y;
    double z;
};


Matrix affine_matrix(unit_vector u, double angle,double shift_x, double shift_y, double shift_z);
Matrix translate_matrix(double shift_x, double shift_y, double shift_z);
double get_scale_from_trans(Matrix trans);

NeuronTree apply_transform(NeuronTree * nt, Matrix  trans);
bool apply_transform_to_swc(const V3DPluginArgList & input, V3DPluginArgList & output);

#endif

