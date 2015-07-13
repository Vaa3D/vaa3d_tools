/*
Copyright (C) Erasmus MC. Permission to use this software and corresponding documentation for educational, research, and not-for-profit purposes, without a fee and without a signed licensing agreement, is granted, subject to the following terms and conditions.

IT IS NOT ALLOWED TO REDISTRIBUTE, SELL, OR LEASE THIS SOFTWARE, OR DERIVATIVE WORKS THEREOF, WITHOUT PERMISSION IN WRITING FROM THE COPYRIGHT HOLDER. THE COPYRIGHT HOLDER IS FREE TO MAKE VERSIONS OF THE SOFTWARE AVAILABLE FOR A FEE OR COMMERCIALLY ONLY.

IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OF ANY KIND WHATSOEVER, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF ADVISED OF THE POSSIBILITY THEREOF.

THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE EXPRESS OR IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND CORRESPONDING DOCUMENTATION IS PROVIDED "AS IS". THE COPYRIGHT HOLDER HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 * 2015-5-31 : by Miroslav Radojevic
 */

#include "model.h"
#include <math.h>
#include <stdio.h>
#include "v3d_message.h"
#include <v3d_interface.h>
#include "basic_surf_objs.h"

Model::Model(int _D, int _Ndir, bool _is2D, float _zDist)
{
    D = _D;
    Ndir = _Ndir;
    is2D = _is2D;
    zDist = _zDist;

    L2xy    = D/2; // (int) floor((D/2.0)+0.5);
    Lxy     = 2*L2xy+1;

    L2z = (is2D)? 0 : (L2xy/zDist); // L2z     = (_is2D)? 0 : ((D/2)/_zDist); // (int) floor((D/2.0/_zDist)+0.5);
    Lz      = 2*L2z+1;

    nr_sigmas = 0;
    for (float s = sigma_min; s <= 0.4*L2xy; s+=sigma_stp) nr_sigmas++;

    sigmas = new float[nr_sigmas];
    nr_sigmas = 0;
    for (float s = sigma_min; s <= 0.4*L2xy; s+=sigma_stp) sigmas[nr_sigmas++]=s;

    kernels = new float*[Ndir*nr_sigmas];
    for (int i = 0; i < Ndir*nr_sigmas; ++i) kernels[i] = new float[Lxy*Lxy*Lz];

    kernels_avg = new float[Ndir*nr_sigmas];

    kernels_hat = new float*[Ndir*nr_sigmas];
    for (int i = 0; i < Ndir*nr_sigmas; ++i) kernels_hat[i] = new float[Lxy*Lxy*Lz];

    kernels_hat_sum_2 = new float[Ndir*nr_sigmas];

    values = new float[Lxy*Lxy*Lz];
    values_average = 0;

    printf("creating model...\n%d scales\n%d x %d x %d \n", nr_sigmas, Lxy, Lxy, Lz);

    dx = new int[Lxy*Lxy*Lz];
    dy = new int[Lxy*Lxy*Lz];
    dz = new int[Lxy*Lxy*Lz];
    // fill the offsets up
    for (int dxval = -L2xy; dxval <= L2xy; ++dxval) {
        for (int dyval = -L2xy; dyval <= L2xy; ++dyval) {
            for (int dzval = -L2z; dzval <= L2z; ++dzval) {
                int dxidx = dxval+L2xy;
                int dyidx = dyval+L2xy;
                int dzidx = dzval+L2z;
                dx[dzidx*Lxy*Lxy+dyidx*Lxy+dxidx] = dxval;
                dy[dzidx*Lxy*Lxy+dyidx*Lxy+dxidx] = dyval;
                dz[dzidx*Lxy*Lxy+dyidx*Lxy+dxidx] = dzval;
            }
        }
    }

//    printf("created offsets\n");

    vx = new float[Ndir];
    vy = new float[Ndir];
    vz = new float[Ndir];
    calculate_dirs(vx, vy, vz);

    printf("\n--finished calculating dirs--\n");
    for (int ii = 0; ii < Ndir; ++ii) {
        printf("%5.2f , %5.2f , %5.2f ,\n", vx[ii], vy[ii], vz[ii]);
    }

    float ** T;
    T = new float*[3];
    T[0] = new float[3];
    T[1] = new float[3];
    T[2] = new float[3];

    float wx, wy, wz, ux, uy, uz;

    // formation
//    printf("\ncreating templates...\n");
    for (int i = 0; i < Ndir*nr_sigmas; i++) {

//        printf(".");

        int direc_idx = i % Ndir;
        int scale_idx = i / Ndir;

        float sigV = 2*L2xy;              // std dev along the v direction
        float sigW = sigmas[scale_idx];   // std dev along first  orthogonal
        float sigU = sigmas[scale_idx];   // std dev along second orthogonal

        // (0,0,1)->T->(vx,vy,vz)
        rotation_matrix(0,0,1, vx[direc_idx], vy[direc_idx], vz[direc_idx], T);

        // (1,0,0)->(wx,wy,wz)
        rotation_apply(T, 1,0,0, wx,wy,wz);
        // (0,1,0)->(ux,uy,uz)
        rotation_apply(T, 0,1,0, ux,uy,uz);

        kernels_avg[i] = 0; // average

        // filling up the kernel values...
        if (is2D) {

//            printf("\n2D\n");

            int zz = 0;

            for (int xx = -L2xy; xx <= L2xy; ++xx) {
                for (int yy = -L2xy; yy <= L2xy; ++yy) {

                    float projv = xx*vx[direc_idx] + yy*vy[direc_idx] + zz*vz[direc_idx]; // dot prod with (vx, vy, vz)
                    float projw = xx*wx + yy*wy + zz*wz; // with (wx, wy, wz)
                    float proju = xx*ux + yy*uy + zz*uz; // with (ux, uy, uz)

                    int xxidx = xx+L2xy;
                    int yyidx = yy+L2xy;
                    int zzidx = 0;

                    float kernel_val = (float) exp(
                                    -(
                                    ((projv*projv)/(2*sigV*sigV))  +
                                    ((projw*projw)/(2*sigW*sigW))  +
                                    ((proju*proju)/(2*sigU*sigU))
                                    )
                                    );

//                    printf("%d,%d  ", i, xxidx);
                    kernels[i][zzidx*Lxy*Lxy+yyidx*Lxy+xxidx] = kernel_val;
                    kernels_avg[i] += kernel_val;

                }
            }


        }
        else { // 3D

            // central layer - same as in 2D
            int zz = 0;

            for (int xx = -L2xy; xx <= L2xy; ++xx) {
                for (int yy = -L2xy; yy <= L2xy; ++yy) {

                    float projv = xx*vx[direc_idx] + yy*vy[direc_idx] + zz*zDist*vz[direc_idx]; // dot prod with (vx, vy, vz)
                    float projw = xx*wx + yy*wy + zz*zDist*wz; // with (wx, wy, wz)
                    float proju = xx*ux + yy*uy + zz*zDist*uz; // with (ux, uy, uz)

                    int xxidx = xx+L2xy;
                    int yyidx = yy+L2xy;
                    int zzidx = zz+L2z;

                    float kernel_val = (float) exp(
                                    -(
                                    ((projv*projv)/(2*sigV*sigV))  +
                                    ((projw*projw)/(2*sigW*sigW))  +
                                    ((proju*proju)/(2*sigU*sigU))
                                    )
                                    );

                    kernels[i][zzidx*Lxy*Lxy+yyidx*Lxy+xxidx] = kernel_val;
                    kernels_avg[i] += kernel_val;

                }
            }

            // remainder of the layers
            for (int zstep = 1; zstep <= L2z; ++zstep) {
                // do both +/-
                for (int sign = -1; sign <= 1; sign+=2) {

                    zz = zstep*sign;

                    // other layers
                    for (int xx = -L2xy; xx <= L2xy; ++xx) {
                        for (int yy = -L2xy; yy <= L2xy; ++yy) {

                            float projv = xx*vx[direc_idx] + yy*vy[direc_idx] + zz*zDist*vz[direc_idx]; // dot prod with (vx, vy, vz)
                            float projw = xx*wx + yy*wy + zz*zDist*wz; // with (wx, wy, wz)
                            float proju = xx*ux + yy*uy + zz*zDist*uz; // with (ux, uy, uz)

                            int xxidx = xx+L2xy;
                            int yyidx = yy+L2xy;
                            int zzidx = zz+L2z;

                            float kernel_val = (float) exp(
                                            -(
                                            ((projv*projv)/(2*sigV*sigV))  +
                                            ((projw*projw)/(2*sigW*sigW))  +
                                            ((proju*proju)/(2*sigU*sigU))
                                            )
                                            );

                            kernels[i][zzidx*Lxy*Lxy+yyidx*Lxy+xxidx] = kernel_val;
                            kernels_avg[i] += kernel_val;

                        }
                    }

                }
            }

        }

        kernels_avg[i] /= (float)(Lxy*Lxy*Lz);

        kernels_hat_sum_2[i] = 0;

        for (int j = 0; j < Lxy*Lxy*Lz; ++j) {

            kernels_hat[i][j] = kernels[i][j] - kernels_avg[i];
            kernels_hat_sum_2[i] += pow(kernels_hat[i][j], 2);

        }

    }

    for (int i = 0; i < 3; i++) {
        delete [] T[i];
    }
    delete [] T;
    T = 0;

//    printf("\n\ndone creating!\n");
//    for (int ki = 0; ki < Ndir*nr_sigmas; ++ki) {
//        printf("\n%d\n",ki);
//        for (int kii = 0; kii < Lxy*Lxy*Lz; ++kii) {
//            printf("%6.2f ", kernels[ki][kii]);
//        }
//    }
//    printf("\n\ndone creating!\n");

}

Model::~Model()
{

    delete [] sigmas;
    sigmas = 0;

    for (int i = 0; i < Ndir*nr_sigmas; i++) {
        delete [] kernels[i];
    }
    delete [] kernels;
    kernels = 0;

    delete [] kernels_avg;
    kernels_avg = 0;

    for (int i = 0; i < Ndir*nr_sigmas; i++) {
        delete [] kernels_hat[i];
    }
    delete [] kernels_hat;
    kernels_hat = 0;

    delete [] kernels_hat_sum_2;
    kernels_hat_sum_2 =0;

    delete [] values;
    values = 0;

    delete [] dx;
    dx = 0;

    delete [] dy;
    dy = 0;

    delete [] dz;
    dz = 0;

    delete [] vx;
    vx = 0;

    delete [] vy;
    vy = 0;

    delete [] vz;
    vz = 0;

}

void Model::calculate_dirs(float * _vx, float * _vy, float * _vz)
{
    // different schemes depending on the dimensionality
    if (is2D) {
        for (int direc_idx = 0; direc_idx < Ndir; ++direc_idx) {
            float ang = direc_idx * (3.14 / Ndir);
            _vx[direc_idx] =  ((float) cos(ang));
            _vy[direc_idx] =  ((float) sin(ang));
            _vz[direc_idx] =  0;
        }
    }
    else {
        //
        double h_k, theta_k, phi_k, phi_k_1 = 0;

        for (int k = 0; k < Ndir; k++) {

            h_k = -1 + 1 * (double)k/(Ndir-1); // -1 : 0
            theta_k = acos(h_k);

                    if(k==0 || k==(Ndir-1)){

                        phi_k   = 0;
                        phi_k_1 = 0;

                    }
                    else{

                        phi_k = phi_k_1 + 3.6 / ( sqrt(Ndir) * sqrt(1-h_k*h_k));
                        phi_k_1 = phi_k;

                    }

                    _vx[k] = (float) (sin(theta_k) * cos(phi_k));
                    _vy[k] = (float) (sin(theta_k) * sin(phi_k));
                    _vz[k] = (float)  cos(theta_k);

        }

    }

}

void Model::rotation_matrix(float a1, float a2, float a3, float b1, float b2, float b3, float** R)
{

    // v is cross product of (a1, a2, a3) and (b1, b2, b3)
    float v1 = a2*b3 - b2*a3;
    float v2 = -(a1*b3-b1*a3);
    float v3 = a1*b2-b1*a2;

    float tt = (1-(a1*b1+a2*b2+a3*b3))/(v1*v1+v2*v2+v3*v3);

    // from http://math.stackexchange.com/questions/180418/calculate-rotation-matrix-to-align-vector-a-to-vector-b-in-3d
    R[0][0] = 1 + 0     + tt * (-v3*v3-v2*v2);
    R[0][1] = 0 + (-v3) + tt * (v1*v2);
    R[0][2] = 0 + (v2)  + tt * (-v1*v3);

    R[1][0] = 0 + (v3)  + tt * (v1*v2);
    R[1][1] = 1 + 0     + tt * (-v3*v3-v1*v1);
    R[1][2] = 0 + (-v1) + tt * (v2*v3);

    R[2][0] = 0 + (-v2) + tt * (v1*v3);
    R[2][1] = 0 + (v1)  + tt * (v2*v3);
    R[2][2] = 1 + 0     + tt * (-v2*v2-v1*v1);

}

void Model::rotation_apply(float** R, float v1, float v2, float v3, float &out1, float &out2, float &out3)
{

    out1 = R[0][0]*v1 + R[0][1]*v2 + R[0][2]*v3;
    out2 = R[1][0]*v1 + R[1][1]*v2 + R[1][2]*v3;
    out3 = R[2][0]*v1 + R[2][1]*v2 + R[2][2]*v3;

}

void Model::save(V3DPluginCallback2 &callback, QString inimg_path)
{

    unsigned char * knl;
    knl = new unsigned char[Lxy*Lxy*Lz];

    for (int ki = 0; ki < Ndir*nr_sigmas; ++ki) {

        QString out_str = inimg_path + QString("_kernel%1.tif").arg(ki);
        printf("saving %s\n",out_str.toStdString().c_str());

        V3DLONG in_sz[4];
        in_sz[0] = (long) Lxy;
        in_sz[1] = (long) Lxy;
        in_sz[2] = (long) Lz;
        in_sz[3] = (long) 1;

        // save image, convert each kernel to unsigned char * with elements 0-255
        for (int kk = 0; kk < Lxy*Lxy*Lz; ++kk) {
            knl[kk] = kernels[ki][kk]*255.0;
        }

        simple_saveimage_wrapper(callback, out_str.toStdString().c_str(), knl, in_sz, V3D_UINT8);

//        Image4DSimple outimg1;
//        outimg1.setData(knl, in_sz[0], in_sz[1], in_sz[2], in_sz[3], V3D_UINT8);
//        callback.saveImage(&outimg1, (char *)out_str.toStdString().c_str());

//        printf("what was it?\n");
//        for (int var = 0; var < Lxy*Lxy*Lz; ++var) {
//            printf("%6.2f (%d,%d,%d)", kernels[ki][var], dx[var], dy[var], dz[var]);
//        }

    }

    delete [] knl;
    knl = 0;

}

void Model::get_corr(int atX, int atY, int atZ, unsigned char * inimg, int width, int height, int length, float &out_score, float &out_scale, float &out_vx, float &out_vy, float &out_vz)
{

    out_score = -1;
    out_scale = -1;
    out_vx = 0;
    out_vy = 0;
    out_vz = 0;

    if (atX>=L2xy && atX<width-L2xy && atY>=L2xy && atY<height-L2xy && atZ>=L2z && atZ<length-L2z) {
        // location is with the image boundaries so that values can be extracted


        // calculate values and the average
        values_average = 0;

        for (int i = 0; i < Lxy*Lxy*Lz; ++i) {

            int xloc = atX+dx[i];
            int yloc = atY+dy[i];
            int zloc = atZ+dz[i];

            values[i] = inimg[zloc*width*height+yloc*width+xloc];
            values_average += values[i];

        }

        values_average /= (Lxy*Lxy*Lz);

        // see the correlation with different templates
        for (int kidx = 0; kidx < Ndir*nr_sigmas; kidx++) {

            float num = 0;
            float den = 0;

            for (int i = 0; i < Lxy*Lxy*Lz; i++) {
                num += (values[i] - values_average) * kernels_hat[kidx][i];
                den += pow(values[i] - values_average, 2);
            }

            float curr_zncc = num / sqrt(den * kernels_hat_sum_2[kidx]);

            if (curr_zncc>out_score) {

                out_score = curr_zncc;

                int direc_idx = kidx % Ndir;
                out_vx = vx[direc_idx];
                out_vy = vy[direc_idx];
                out_vz = vz[direc_idx];

                int scale_idx = kidx / Ndir;
                out_scale =  sigmas[scale_idx];

            }

        }

    }
}
