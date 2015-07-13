#ifndef MODEL_H
#define MODEL_H

#include "v3d_message.h"
#include <v3d_interface.h>

class Model
{
public:

    int             D;                  // max. scale
    int             Ndir;               // to cover semi-circle/sphere
    bool            is2D;
    float           zDist;
    float *         sigmas;             // multiscale
    int             nr_sigmas;          // number of scales

    int             L2xy, L2z, Lxy, Lz; // dimensions in pix

    float **        kernels;            // Ndir*nr_scales x Lxy*Lxy*Lz
    float *         kernels_avg;        // Ndir*nr_scales x 1
    float **        kernels_hat;        // Ndir*nr_scales x Lxy*Lxy*Lz
    float *         kernels_hat_sum_2;  // Ndir*nr_scales x 1

    float *         values;             // storage for the image values
    float           values_average;

    // offsets in x, y, z
    int *           dx;                 // Lxy*Lxy*Lz
    int *           dy;                 // ...
    int *           dz;                 // ...

    // each of the Ndir will correspond to a vector
    float *         vx;                 // ...
    float *         vy;                 // ...
    float *         vz;                 // ...

    const float sigma_min = 1.0;        // multiscale definition
    const float sigma_stp = 0.5;

    Model(int _D, int _Ndir, bool _is2D, float _zDist);
    ~Model();

    void calculate_dirs(float * _vx, float * _vy, float * _vz);
    void rotation_matrix(float a1, float a2, float a3, float b1, float b2, float b3, float** R);
    void rotation_apply(float** R, float v1, float v2, float v3, float &out1, float &out2, float &out3);

    void save(V3DPluginCallback2 &callback, QString inimg_path);
    void get_corr(int atX, int atY, int atZ, unsigned char * inimg, int width, int height, int length, float &out_score, float &out_scale, float &out_vx, float &out_vy, float &out_vz);

};

#endif // MODEL_H
