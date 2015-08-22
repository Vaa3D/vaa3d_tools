/*
Copyright (C) Erasmus MC. Permission to use this software and corresponding documentation for educational, research, and not-for-profit purposes, without a fee and without a signed licensing agreement, is granted, subject to the following terms and conditions.

IT IS NOT ALLOWED TO REDISTRIBUTE, SELL, OR LEASE THIS SOFTWARE, OR DERIVATIVE WORKS THEREOF, WITHOUT PERMISSION IN WRITING FROM THE COPYRIGHT HOLDER. THE COPYRIGHT HOLDER IS FREE TO MAKE VERSIONS OF THE SOFTWARE AVAILABLE FOR A FEE OR COMMERCIALLY ONLY.

IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OF ANY KIND WHATSOEVER, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF ADVISED OF THE POSSIBILITY THEREOF.

THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE EXPRESS OR IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND CORRESPONDING DOCUMENTATION IS PROVIDED "AS IS". THE COPYRIGHT HOLDER HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#include "btracer.h"
#include <math.h>
#include <fstream>
#include <iostream>
#include "toolbox.h"
#include <v3d_interface.h>
#include "v3d_message.h"
#include "basic_surf_objs.h"
#include "float.h"
#include <cstdlib>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <iostream>
#include <algorithm>
#include <vector>

using namespace std;

float   BTracer::outerring = 1.0; // tolerance on
float   BTracer::gcsstd2rad = 2.5;
float   BTracer::gcsstd_min = 1.0;
float   BTracer::gcsstd_step = 0.5;
int     BTracer::Nsteps = 4;   // steps to cover scale with samples, 3,4 is reasonable
float   BTracer::ang = 3.14 * (30.0/180.0); // degrees in radians
int     BTracer::Ndirs = 5; // these Ndirs are different from ones used in the main plugin
float   BTracer::K = 4.0;
//float   BTracer::bgratio = 0.20; // fix this and try again
//float   BTracer::ovlpratio = 0.20;

class ComprByVals {
    std::vector<int>* values;
public:
    ComprByVals(std::vector<int>* values1) : values(values1) {}
public:
    bool operator() (const int& a, const int& b) const { return (*values)[a] < (*values)[b];}
};

BTracer::BTracer(int _Niterations, int _Nstates, int _scale, bool _is2D, float _zDist)
{
    Niterations = _Niterations;
    Nstates = _Nstates;

    N1 = (int) pow(Ndirs, Nsteps);
    NN = Nstates * N1;

    node_cnt = 0;

    is2D = _is2D;
    zDist = _zDist;

    step = (float)_scale/(float)Nsteps;

    // if the trace is in 2d then 1 orthogonal, otherwise two orthogonals
    U2 = _scale/2;
    U = 2*U2+1; // 1st orthogonal

    if (is2D) {
        W2 = 0;
        W = 1;
    }
    else {
        W2 = _scale/2;
        W = 2*W2+1;
    }

    V2 = (is2D)?_scale/2:1;//;
    V = 2*V2 + 1;

    // gcsstd define
    gcsstd_max = ((float)1/gcsstd2rad)*U2;
    int cnt = 0;
    for (float sg = gcsstd_min; sg <= gcsstd_max; sg+=gcsstd_step) cnt++;
    gcsstd_nr = cnt;

    gcsstd = new float[gcsstd_nr];
    cnt = 0;
//    cout << "gcsstd:" << flush;
    for (float sg = gcsstd_min; sg <= gcsstd_max; sg+=gcsstd_step) {
        gcsstd[cnt++] = sg;
//        cout << sg << " " << flush;
    }
//    cout << endl;

    img_vals = new float[U*W*V];

    // tt, tta templates define
    tt = new float*[gcsstd_nr]; // cross-section template
    for (int i = 0; i < gcsstd_nr; ++i) {
        tt[i] = new float[U*W*V];
    }

    tta = new float[gcsstd_nr];

    // calculate the templates
    for (int i = 0; i < gcsstd_nr; ++i) {

        float ag = 0;

        // indexing is differnt in 2d and 3d
        if (is2D) {

            for (int vv = -V2; vv <= V2; ++vv) {
                for (int uu = -U2; uu <= U2; ++uu) {
                    //for (int ww = -W2; ww <= W2; ++ww) { // W2=0, W=1
                        float value = exp(-(pow(uu,2)  )/(2*pow(gcsstd[i],2))); // +pow(ww,2)
                        int v1 = vv + V2; // 0-V
                        int u1 = uu + U2; // 0-U
//                        int w1 = ww + W2; // 0-W
                        tt[i][v1*U+u1] = value;
                        ag += value;
                    //}
                }
            }

        }
        else {

            for (int vv = -V2; vv <= V2; ++vv) {
                for (int uu = -U2; uu <= U2; ++uu) {
                    for (int ww = -W2; ww <= W2; ++ww) {
                        float value = exp(-(pow(uu,2)+pow(ww,2))/(2*pow(gcsstd[i],2)));
                        int v1 = vv + V2; // 0-V
                        int u1 = uu + U2; // 0-U
                        int w1 = ww + W2; // 0-W
                        tt[i][v1*U*W+w1*U+u1] = value;
                        ag += value;
                    }
                }
            }

        }

        tta[i] = ag/(U*W*V);

    }

    // transition from 1 source state
    xtt1     = new float*[Nsteps];
    ytt1     = new float*[Nsteps];
    ztt1     = new float*[Nsteps];
    vxtt1    = new float*[Nsteps];
    vytt1    = new float*[Nsteps];
    vztt1    = new float*[Nsteps];
    rtt1     = new float*[Nsteps];
    priortt1 = new float*[Nsteps];
    lhoodtt1 = new float*[Nsteps];
    postrtt1 = new float*[Nsteps];

    for (int i = 0; i < Nsteps; ++i) {

        int alloc = (int)round(pow(Ndirs,i+1));
        xtt1[i] = new float[alloc];
        ytt1[i] = new float[alloc];
        ztt1[i] = new float[alloc];
        vxtt1[i] = new float[alloc];
        vytt1[i] = new float[alloc];
        vztt1[i] = new float[alloc];
        rtt1[i] = new float[alloc];
        priortt1[i] = new float[alloc];
        lhoodtt1[i] = new float[alloc];
        postrtt1[i] = new float[alloc];

    }

    postrtt1_aux = new float[N1];
    postrtt1_idx = new int[N1];

    // transition from Nstates source states
    xttN     = new float*[Nsteps];
    yttN     = new float*[Nsteps];
    zttN     = new float*[Nsteps];
    vxttN    = new float*[Nsteps];
    vyttN    = new float*[Nsteps];
    vzttN    = new float*[Nsteps];
    rttN     = new float*[Nsteps];
    priorttN = new float*[Nsteps];
    lhoodttN = new float*[Nsteps];
    postrttN = new float*[Nsteps];

    for (int i = 0; i < Nsteps; ++i) {

        int alloc = Nstates * (int)round(pow(Ndirs,i+1));
        xttN[i]  = new float[alloc];
        yttN[i]  = new float[alloc];
        zttN[i]  = new float[alloc];
        vxttN[i] = new float[alloc];
        vyttN[i] = new float[alloc];
        vzttN[i] = new float[alloc];
        rttN[i]  = new float[alloc];
        priorttN[i] = new float[alloc];
        lhoodttN[i] = new float[alloc];
        postrttN[i] = new float[alloc];

    }

    // auxiliary arrays for storing posteriors and indexes (sorting will change the array submitted as argument)
    postrttN_aux = new float[NN];
    postrttN_idx = new int[NN];
    // just K highest values is not enough, we need indexes of top K values to backtrack
    // once they are found the realation with the originals is lost and cannot select corresponding locations
    topIdxs = new int[Nstates];

    xt = new float*[Niterations*Nsteps];
    yt = new float*[Niterations*Nsteps];
    zt = new float*[Niterations*Nsteps];
    vxt = new float*[Niterations*Nsteps];
    vyt = new float*[Niterations*Nsteps];
    vzt = new float*[Niterations*Nsteps];
    rt = new float*[Niterations*Nsteps];
    wt = new float*[Niterations*Nsteps];

    for (int i = 0; i < Niterations*Nsteps; ++i) {
        xt[i]  = new float[Nstates];
        yt[i]  = new float[Nstates];
        zt[i]  = new float[Nstates];
        vxt[i] = new float[Nstates];
        vyt[i] = new float[Nstates];
        vzt[i] = new float[Nstates];
        rt[i]  = new float[Nstates];
        wt[i]  = new float[Nstates];
    }

    xc = new float[Niterations*Nsteps];
    yc = new float[Niterations*Nsteps];
    zc = new float[Niterations*Nsteps];
    rc = new float[Niterations*Nsteps];

    vx_template = new float[Ndirs];
    vy_template = new float[Ndirs];
    vz_template = new float[Ndirs];

    calculate_dirs(vx_template, vy_template, vz_template, is2D, Ndirs);

    rot = new float*[3];
    for (int i = 0; i < 3; ++i) {
        rot[i] = new float[3];
    }
    rot[0][0] = rot[0][1] = rot[0][2] = 0;
    rot[1][0] = rot[1][1] = rot[1][2] = 0;
    rot[2][0] = rot[2][1] = rot[2][2] = 0;

    cout << "gen. offsets..." << endl;
    for (int i = 0; i < gcsstd_nr; ++i) { // as many offsets as

        vector<int> ttx;
        vector<int> tty;
        vector<int> ttz;
        vector<int> ttd; // distance towards (0,0,0), will be used to sort the values
        vector<int> tti; // indices for appending them to offx, offy, offz

        int rx, ry, rz; // limits
        // set here to 2 so that the radiuses are 2,3,4,5,6,7...
        // gcsstd2rad can cause check spheres to be too large to check
        rx = ry = round(2 * gcsstd[i]);
        rz =   (is2D)? 0 : round(2 * gcsstd[i] / zDist);

        // loop offsets from the centre outwards so that those closer to the center are the first in the list
        int cnt = 0;
        for (int dx = -rx; dx <= rx; ++dx) {
            for (int dy = -ry; dy <= ry; ++dy) {
                for (int dz = -rz; dz <= rz; ++dz) {
                    bool insph = (dx*dx)/(float)(rx*rx)+(dy*dy)/(float)(ry*ry)+((is2D)?0:(dz*dz)/(float)(rz*rz))<=1;
                    if (insph) {
                        ttx.push_back(dx);
                        tty.push_back(dy);
                        ttz.push_back(dz);
                        ttd.push_back(dx*dx+dy*dy+dz*dz);
                        tti.push_back(cnt++);
                    }
                }
            }
        }

        cout << "\ngcsstd=" << setw(5)<<gcsstd[i] << " Rxy=" << setw(2)<<rx << " Rz=" << setw(2)<<rz << " #=" << setw(5)<<ttx.size() << endl;

        // sort w.r.t distance from the center before adding it - so that those
        // offsetsand hence corresponding node and trace tags that are closest
        // to the center are first in the list
        sort(tti.begin(), tti.end(), ComprByVals(&ttd));

        vector<int> ttxSort;
        vector<int> ttySort;
        vector<int> ttzSort;

        for (int i = 0; i < tti.size(); ++i) {
            ttxSort.push_back(ttx[tti[i]]);
            ttySort.push_back(tty[tti[i]]);
            ttzSort.push_back(ttz[tti[i]]);
        }

//        for (int i = 0; i < ttxSort.size(); ++i) {
//            cout << "[" << ttxSort[i] << "," << ttySort[i] << "," << ttzSort[i] << "] " << flush;
//        }

        // cout << tti[i] << " : " << ttx[tti[i]] << " " << tty[tti[i]] << " " << ttz[tti[i]] << " " << ttd[tti[i]] << endl;
        // cout << "\n----" << endl;

        offx.push_back(ttxSort);
        offy.push_back(ttySort);
        offz.push_back(ttzSort);

    }

}

BTracer::~BTracer() {
    //
    delete [] gcsstd; gcsstd = 0;
    delete [] img_vals; img_vals = 0;

    for (int i = 0; i < gcsstd_nr; ++i) {
        delete [] tt[i];
    }
    delete [] tt; tt = 0;

    delete [] tta;

    for (int i = 0; i < Nsteps; ++i) {

        delete [] xtt1[i]; xtt1[i] = 0;
        delete [] ytt1[i]; ytt1[i] = 0;
        delete [] ztt1[i]; ztt1[i] = 0;
        delete [] vxtt1[i]; vxtt1[i] = 0;
        delete [] vytt1[i]; vytt1[i] = 0;
        delete [] vztt1[i]; vztt1[i] = 0;
        delete [] rtt1[i]; rtt1[i] = 0;
        delete [] priortt1[i]; priortt1[i] = 0;
        delete [] postrtt1[i]; postrtt1[i] = 0;
        delete [] lhoodtt1[i]; lhoodtt1[i] = 0;

        delete [] xttN[i]; xttN[i] = 0;
        delete [] yttN[i]; yttN[i] = 0;
        delete [] zttN[i]; zttN[i] = 0;
        delete [] vxttN[i]; vxttN[i] = 0;
        delete [] vyttN[i]; vyttN[i] = 0;
        delete [] vzttN[i]; vzttN[i] = 0;
        delete [] rttN[i]; rttN[i] = 0;
        delete [] priorttN[i]; priorttN[i] = 0;
        delete [] postrttN[i]; postrttN[i] = 0;
        delete [] lhoodttN[i]; lhoodttN[i] = 0;

    }

    delete [] xtt1; xtt1 = 0;
    delete [] ytt1; ytt1 = 0;
    delete [] ztt1; ztt1 = 0;
    delete [] vxtt1; vxtt1 = 0;
    delete [] vytt1; vytt1 = 0;
    delete [] vztt1; vztt1 = 0;
    delete [] rtt1; rtt1 = 0;
    delete [] priortt1; priortt1 = 0;
    delete [] postrtt1; postrtt1 = 0;
    delete [] lhoodtt1; lhoodtt1 = 0;

    delete [] xttN; xttN = 0;
    delete [] yttN; yttN = 0;
    delete [] zttN; zttN = 0;
    delete [] vxttN; vxttN = 0;
    delete [] vyttN; vyttN = 0;
    delete [] vzttN; vzttN = 0;
    delete [] rttN; rttN = 0;
    delete [] priorttN; priorttN = 0;
    delete [] postrttN; postrttN = 0;
    delete [] lhoodttN; lhoodttN = 0;
    //
    delete [] postrttN_aux; postrttN_aux = 0;
    delete [] postrttN_idx; postrttN_idx = 0;
    //
    delete [] postrtt1_aux; postrtt1_aux = 0;
    delete [] postrtt1_idx; postrtt1_idx = 0;
    //
    delete [] topIdxs; topIdxs = 0;
    //
    for (int i = 0; i < Niterations*Nsteps; ++i) {
        delete [] xt[i]; xt[i] = 0;
        delete [] yt[i]; yt[i] = 0;
        delete [] zt[i]; zt[i] = 0;
        delete [] vxt[i]; vxt[i] = 0;
        delete [] vyt[i]; vyt[i] = 0;
        delete [] vzt[i]; vzt[i] = 0;
        delete [] rt[i]; rt[i] = 0;
        delete [] wt[i]; wt[i] = 0;
    }
    delete [] xt; xt = 0;
    delete [] yt; yt = 0;
    delete [] zt; zt = 0;
    delete [] vxt; vxt = 0;
    delete [] vyt; vyt = 0;
    delete [] vzt; vzt = 0;
    delete [] rt; rt = 0;
    delete [] wt; wt = 0;

    delete [] xc; xc = 0;
    delete [] yc; yc = 0;
    delete [] zc; zc = 0;
    delete [] rc; rc = 0;

    delete [] vx_template; vx_template = 0;
    delete [] vy_template; vy_template = 0;
    delete [] vz_template; vz_template = 0;

    for (int i = 0; i < 3; ++i) {
        delete [] rot[i]; rot[i] = 0;
    }
    delete [] rot; rot = 0;

}

void BTracer::calculate_dirs(float *_vx, float *_vy, float *_vz, bool is2D, int nr_dirs) {

    // different schemes depending on the dimensionality
    if (is2D) {
        for (int di = 0; di < nr_dirs; ++di) {
            float ang1 = ((3.14/2.0)-ang) + di * ((2*ang) / (nr_dirs-1));
            _vx[di] =  cos(ang1);
            _vy[di] =  sin(ang1);
            _vz[di] =  0;
        }
    }
    else {

        // define nr_dirs directions - first done was uniform sampling of predefined number of points on the sphere
        // did not go well in experiments - need sampling to be
        // -- symmetric
        // -- and to fix the number of directions
        // -- problem was that it was spreading out backwards after the recursions

        // theta = -pi
        _vx[0] = 0;
        _vy[0] = 0;
        _vz[0] = -1;

        // theta will define the ring, phi will traverse the ring
        for (int k = 1; k < nr_dirs; ++k) {

            float theta     = -3.14 + ang;
            float phi       = (k-1) * ((2*3.14)/(nr_dirs-1));

            _vx[k] = (float) (sin(theta) * cos(phi));
            _vy[k] = (float) (sin(theta) * sin(phi));
            _vz[k] = (float)  cos(theta);

        }

    }

}

void BTracer::rotation_matrix(float a1, float a2, float a3, float b1, float b2, float b3, float** R) {
    // from http://math.stackexchange.com/questions/180418/calculate-rotation-matrix-to-align-vector-a-to-vector-b-in-3d
    // assume a(a1,a2,a3) and b(b1,b2,b3) are unit vectors

    // v is cross product of (a1, a2, a3) and (b1, b2, b3)
    float v1 = a2*b3 - b2*a3;
    float v2 = -(a1*b3-b1*a3);
    float v3 = a1*b2-b1*a2;

    // cross product is zero - symmetric operations
    float cross_prod_norm_2     = v1*v1+v2*v2+v3*v3;
    float dot_prod              = a1*b1+a2*b2+a3*b3;

    if (cross_prod_norm_2<=0.00001) { // cross product is small

        if (fabs(dot_prod-1)<fabs(dot_prod+1)) {
            // identity mapping (a and b are aligned)
            R[0][0] = 1;
            R[0][1] = 0;
            R[0][2] = 0;

            R[1][0] = 0;
            R[1][1] = 1;
            R[1][2] = 0;

            R[2][0] = 0;
            R[2][1] = 0;
            R[2][2] = 1;
        }
        else {
            // inversion (a and b are opposite)
            R[0][0] = -1;
            R[0][1] = 0;
            R[0][2] = 0;

            R[1][0] = 0;
            R[1][1] = -1;
            R[1][2] = 0;

            R[2][0] = 0;
            R[2][1] = 0;
            R[2][2] = -1;
        }
    }
    else {
        // cross product is not very small - it's safe to calculate
        float tt = (1-(a1*b1+a2*b2+a3*b3))/(v1*v1+v2*v2+v3*v3);
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

}

void BTracer::rotation_apply(float** R, float v1, float v2, float v3, float &out1, float &out2, float &out3) {

    out1 = R[0][0]*v1 + R[0][1]*v2 + R[0][2]*v3;
    out2 = R[1][0]*v1 + R[1][1]*v2 + R[1][2]*v3;
    out3 = R[2][0]*v1 + R[2][1]*v2 + R[2][2]*v3;

}

float BTracer::interp(float atX, float atY, float atZ, unsigned char * img, int width, int height, int length, bool dbg) {

    int x1 = (int) atX;
    int x2 = x1 + 1;
    float x_frac = atX - x1;

    int y1 = (int) atY;
    int y2 = y1 + 1;
    float y_frac = atY - y1;

    if (dbg) {printf("\n>>%d -- %d | %d -- %d\n", x1, x2, y1, y2);}

    if (length==1) { // atZ is not necessary

        bool isIn2D = x1>=0 && x2<width && y1>=0 && y2<height;

        if(!isIn2D) {
            printf("interp() out of boundary [%6.2f, %6.2f, %6.2f],[%d--%d],[%d--%d] M=%d, N=%d, P=%d \n", atX, atY, atZ, x1, x2, y1, y2, width, height, length);
            return 0;
        }

        // take neighbourhood 2d
        float I11_1 = (float) img[y1*width+x1]; // img3d_xyz[ x1 ][ y1 ][ z1 ];  // upper left
        float I12_1 = img[y1*width+x2]; // img3d_xyz[ x2 ][ y1 ][ z1 ];  // upper right
        float I21_1 = img[y2*width+x1]; // img3d_xyz[ x1 ][ y2 ][ z1 ];  // bottom left
        float I22_1 = img[y2*width+x2]; // img3d_xyz[ x2 ][ y2 ][ z1 ];  // bottom right

        if (dbg) {

            printf("checking %d %d %d\n", width, height, length);
            for (int i = 0; i < width*height*length; ++i) {
                if (img[i]>0) {printf("found!\n"); break;}
            }

            printf("took values:\t %d %f %f %f %f \n", (y1*width+x1), I11_1, I12_1, I21_1, I22_1);}

        return (1-y_frac) * ((1-x_frac)*I11_1 + x_frac*I12_1) + (y_frac) * ((1-x_frac)*I21_1 + x_frac*I22_1);

    }
    else {

        int z1 = (int) atZ;
        int z2 = z1 + 1;
        float z_frac = atZ - z1;

        bool isIn3D = y1>=0 && y2<height && x1>=0 && x2<width && z1>=0 && z2<length;

        if(!isIn3D) {
            printf("interp() out of boundary [%6.2f, %6.2f, %6.2f],[%d--%d],[%d--%d],[%d--%d] M=%d, N=%d, P=%d \n", atX, atY, atZ, x1, x2, y1, y2, z1, z2, width, height, length);
            return 0;
        }

        // take neighbourhood 3d
        float I11_1 = img[z1*width*height+y1*width+x1]; // img3d_xyz[ x1 ][ y1 ][ z1 ];  // upper left
        float I12_1 = img[z1*width*height+y1*width+x2]; // img3d_xyz[ x2 ][ y1 ][ z1 ];  // upper right
        float I21_1 = img[z1*width*height+y2*width+x1]; // img3d_xyz[ x1 ][ y2 ][ z1 ];  // bottom left
        float I22_1 = img[z1*width*height+y2*width+x2]; // img3d_xyz[ x2 ][ y2 ][ z1 ];  // bottom right

        float I11_2 = img[z2*width*height+y1*width+x1]; // img3d_xyz[ x1 ][ y1 ][ z2 ]; // upper left
        float I12_2 = img[z2*width*height+y1*width+x2]; // img3d_xyz[ x2 ][ y1 ][ z2 ]; // upper right
        float I21_2 = img[z2*width*height+y2*width+x1]; // img3d_xyz[ x1 ][ y2 ][ z2 ]; // bottom left
        float I22_2 = img[z2*width*height+y2*width+x2]; // img3d_xyz[ x2 ][ y2 ][ z2 ]; // bottom right

        return (1-z_frac)  *
                (  (1-y_frac) * ((1-x_frac)*I11_1 + x_frac*I12_1) + (y_frac) * ((1-x_frac)*I21_1 + x_frac*I22_1) )   +
                        z_frac      *
                (  (1-y_frac) * ((1-x_frac)*I11_2 + x_frac*I12_2) + (y_frac) * ((1-x_frac)*I21_2 + x_frac*I22_2) );

    }

}

int BTracer::get_gcsstd_idx(float gcsstd_val) {
    int offset_idx = 0;
    float mindst = gcsstd[0]-gcsstd_val;
    mindst = (mindst<0)?-mindst:mindst;

    for (int gi = 1; gi < gcsstd_nr; ++gi) {
        float vv = gcsstd[gi]-gcsstd_val;
        vv = (vv<0)?-vv:vv;
        if (vv<mindst) {
            mindst = vv;
            offset_idx = gi;
        }
    }

    return offset_idx;
}

void BTracer::extract(std::vector<int> ttags, std::vector<int> ntags, std::vector<int> & nlist) {
    // used for extracting node tags that correspond to different traces

    nlist.clear();

    // remove duplicates
    std::vector<int> t(ttags);
    sort(t.begin(), t.end());
    t.erase(unique(t.begin(), t.end()), t.end());

    for (int i = 0; i < t.size(); ++i) {
        for (int j = 0; j < ttags.size(); ++j) {
            if (t[i]==ttags[j]) {
                nlist.push_back(ntags[j]);
                break; // stop once it's found
            }
        }
    }

}

bool BTracer::traceoverlap(
        float x,  float y,  float z,
        float             r,
        int               img_width,
        int               img_height,
        int               img_length,
        int *             trace_map,
        int *             node_map,
        vector<Node> &    node_list
        ) {

    int offset_idx = get_gcsstd_idx(r); // scale index closest to current radius
    int  xs, ys, zs, atx, aty, atz;

    atx = round(x);
    aty = round(y);
    atz = round(z);

    vector<int> ttags;
    vector<int> ntags;

    for (int oi = 0; oi < offx[offset_idx].size(); ++oi) {

        xs = atx + offx[offset_idx][oi];
        ys = aty + offy[offset_idx][oi];
        zs = atz + offz[offset_idx][oi];

        if (xs>=0 && xs<img_width && ys>=0 && ys<img_height && zs>=0 && zs<img_length) {

            // read tag_,trace_ maps
            int iaux = zs*(img_width*img_height)+ys*img_width+xs;

            if (trace_map[iaux]>0) { //return true;
//                if (trace_map[iaux]==1) cout << "here!! " << node_map[iaux] << endl;
                ttags.push_back(trace_map[iaux]);
                ntags.push_back(node_map[iaux]);
            }

        }
        else {
            cout << "out of img. boundaries." << endl;
            return true;
        }

    }

    if (ttags.size()>0) {
        // it overlaps, there were trace tags>0

        // tag fill
        int ttagFill = ttags[0]; // take the one closest to the center
        int ntagFill = ntags[0]; // and it's corresponding node

        for (int oi = 0; oi < offx[offset_idx].size(); ++oi) {

            xs = atx + offx[offset_idx][oi];
            ys = aty + offy[offset_idx][oi];
            zs = atz + offz[offset_idx][oi];

            int iaux = zs*(img_width*img_height)+ys*img_width+xs;

            if (trace_map[iaux]<=0) {
                trace_map[iaux] = ttagFill;
                node_map[iaux]  = ntagFill;
            }

        }

        vector<int> nlist;
        extract(ttags, ntags, nlist);

        // link if there are 2+ independent traces captured
        if (nlist.size()>=2) {

            for (int k1 = 0; k1 < nlist.size(); ++k1) {
                for (int k2 = 0; k2 < nlist.size(); ++k2) {
                    if (k1!=k2) {
                        node_list[nlist[k1]].nbr.push_back(nlist[k2]);
                        node_list[nlist[k2]].nbr.push_back(nlist[k1]);
                    }
                }

            }

        }

        return true; // it overlaps, this will block tracing late on
    }
    else
        return false;

}

//bool BTracer::tracetagfound(
//        int               tt,
//        float x,  float y,  float z,
//        float             r,
//        int               img_width,
//        int               img_height,
//        int               img_length,
//        int *             trace_map
//        ) {

//    int offset_idx = get_gcsstd_idx(r); // scale index closest to current radius
//    int  xs, ys, zs, atx, aty, atz;

//    atx = round(x);
//    aty = round(y);
//    atz = round(z);

//    for (int oi = 0; oi < offx[offset_idx].size(); ++oi) {

//        xs = atx + offx[offset_idx][oi];
//        ys = aty + offy[offset_idx][oi];
//        zs = atz + offz[offset_idx][oi];

//        if (xs>=0 && xs<img_width && ys>=0 && ys<img_height && zs>=0 && zs<img_length) {
//            int iaux = zs*(img_width*img_height)+ys*img_width+xs;
//            if (trace_map[iaux]==tt)
//                return true;
//        }
//        else
//            return false;

//    }

//    return false;

//}

void BTracer::traceX(
        bool              first,
        float x,  float y,  float z,
        float vx, float vy, float vz,
        float             r,
        unsigned char *   img,
        int               img_width,
        int               img_height,
        int               img_length,
        float             angstd_deg,
        int *             node_tag_map,
        int *             trace_tag_map,
        vector<Node> &    node_list, // indexes of the node_list will fill tag_map and trace_map
        float             bgratio
        ) {

    node_cnt = 0;

    // auxilliary trace variables
    bool ovlpPrev, ovlpCurr;
    int  ntagPrev, ntagCurr;
//    int  ttagPrev, ttagCurr;

    int  cntbckg = 0;
    int  xs, ys, zs, atx, aty, atz;

    vector<int> ttags;
    vector<int> ntags;
    vector<int> nlist;

    int offset_idx = get_gcsstd_idx(r); // scale index closest to current radius

    cntbckg = 0;  // nr. of those in the background

    atx = round(x);
    aty = round(y);
    atz = round(z);

    int curr_tt;

    if (first) {

        // add the node
        Node tracenode(x, y, z, r, Node::AXON);
        node_list.push_back(tracenode);

        // trace tag
        curr_tt = node_list.size()-1;

        // tag fill
        for (int oi = 0; oi < offx[offset_idx].size(); ++oi) {

            xs = atx + offx[offset_idx][oi];
            ys = aty + offy[offset_idx][oi];
            zs = atz + offz[offset_idx][oi];

            if (xs>=0 && xs<img_width && ys>=0 && ys<img_height && zs>=0 && zs<img_length) {

                int iaux = zs*(img_width*img_height)+ys*img_width+xs;

                if (trace_tag_map[iaux]<=0) {
                    trace_tag_map[iaux]     = curr_tt; // retag at the gpnt start
                    node_tag_map[iaux]      = curr_tt;
                }
            }
            else {
                cout<<"first out of img. boundary"<<endl;
                return;
            }

        }

    }
    else {

        curr_tt = node_tag_map[atz*(img_width*img_height)+aty*img_width+atx]; // no need to add node as it was added when first==T

    }

    ovlpCurr = false;
    ntagCurr = curr_tt;

//    if(ovlpCurr) cout << "(x)" << flush;
//    else cout << "(" << curr_tt << ")" << flush;

    start_px = x;
    start_py = y;
    start_pz = z;
    start_vx = vx;
    start_vy = vy;
    start_vz = vz;

    float vx_pr = vx;
    float vy_pr = vy;
    float vz_pr = vz;

    for (int iter_counter = 0; iter_counter < Niterations; ++iter_counter) {

        if (iter_counter == 0) {

            iter_1(x, y, z,
                   vx, vy, vz,
                   r,
                   img, img_width, img_height, img_length,
                   angstd_deg,
                   vx_pr, vy_pr, vz_pr,
                   false);

            // take Nstates highest ones, sort: auxiliary values and indexes
            for (int ii = 0; ii < N1; ++ii) {
                postrtt1_aux[ii] = postrtt1[Nsteps-1][ii];
                postrtt1_idx[ii] = ii;
            }

            // Nstates highest posteriors, topIdxs in [0, Ndirs^Nsteps)
            getKhighestIdxs(postrtt1_aux, postrtt1_idx, N1, Nstates, topIdxs);

        }
        else {

            int prev = (iter_counter-1)*Nsteps+(Nsteps-1);

            iter_N(xt[prev], yt[prev], zt[prev], vxt[prev], vyt[prev], vzt[prev], rt[prev], wt[prev],
                   img, img_width, img_height, img_length, angstd_deg, //gcsstdstd_pix,
                   vx_pr, vy_pr, vz_pr,
                   false);

            // take Nstates highest ones, sort: auxiliary values and indexes
            for (int ii = 0; ii < NN; ++ii) {
                //float r2 = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/X));
                postrttN_aux[ii] = postrttN[Nsteps-1][ii];
                postrttN_idx[ii] = ii;
            }

            // Nstates highest posteriors, topIdxs in [0, Nstates*Ndirs^Nsteps)
            getKhighestIdxs(postrttN_aux, postrttN_idx, NN, Nstates, topIdxs);

        }

        // extract the track: take each of the Nstates indexes at the last step and loop back xt, yt, zt, vxt, vyt, vzt, rt, wt...
        bool is_first = iter_counter == 0;

        // accumulate directions of the last step necessary for the prior in the next iteration
        vx_pr = 0;
        vy_pr = 0;
        vz_pr = 0;

        for (int i_step = 0; i_step < Nsteps; ++i_step) {

            float wsum = 0; // sum of posteriors at this step

            node_cnt = iter_counter*Nsteps + i_step; // node index in a local variable

            for (int i_sample = 0; i_sample < Nstates; ++i_sample) { // Nstates highest posteriors

                // values in topIdxs will have different range depending on whether it is first iteration
                int pp  = topIdxs[i_sample]/(int)round(pow(Ndirs, (Nsteps-1)-i_step));
                float sample_w = (is_first)? postrtt1[i_step][pp] : postrttN[i_step][pp];
                wsum += sample_w;

                wt[node_cnt][i_sample]    = sample_w;
                xt[node_cnt][i_sample]    = (is_first)? xtt1[i_step][pp] : xttN[i_step][pp];
                yt[node_cnt][i_sample]    = (is_first)? ytt1[i_step][pp] : yttN[i_step][pp];
                zt[node_cnt][i_sample]    = (is_first)? ztt1[i_step][pp] : zttN[i_step][pp];

                vxt[node_cnt][i_sample]   = (is_first)? vxtt1[i_step][pp] : vxttN[i_step][pp];
                vyt[node_cnt][i_sample]   = (is_first)? vytt1[i_step][pp] : vyttN[i_step][pp];
                vzt[node_cnt][i_sample]   = (is_first)? vztt1[i_step][pp] : vzttN[i_step][pp];

                if (i_step==Nsteps-1) { // average samples of the last step
                    vx_pr += vxt[node_cnt][i_sample];
                    vy_pr += vyt[node_cnt][i_sample];
                    vz_pr += vzt[node_cnt][i_sample];
                }

                rt[node_cnt][i_sample]    = (is_first)? rtt1[i_step][pp] : rttN[i_step][pp];

            }

            // wt -> probability distribution, estimate centroids and check stoppage
            xc[node_cnt] = 0;
            yc[node_cnt] = 0;
            zc[node_cnt] = 0;
            rc[node_cnt] = 0;

            for (int i_sample = 0; i_sample < Nstates; ++i_sample) {
                wt[node_cnt][i_sample] /= wsum;
                xc[node_cnt] += wt[node_cnt][i_sample] * xt[node_cnt][i_sample];
                yc[node_cnt] += wt[node_cnt][i_sample] * yt[node_cnt][i_sample];
                zc[node_cnt] += wt[node_cnt][i_sample] * zt[node_cnt][i_sample];
                rc[node_cnt] += wt[node_cnt][i_sample] * rt[node_cnt][i_sample];
            }

            // ------------------------------------
            // ** trace estimates are calculated **
            // ------------------------------------
            offset_idx = get_gcsstd_idx(rc[node_cnt]);
            cntbckg = 0;

            ntags.clear();
            ttags.clear();

            atx = round(xc[node_cnt]);
            aty = round(yc[node_cnt]);
            atz = round(zc[node_cnt]);

            // check the overlap or whether it is necessary to stop
            for (int oi = 0; oi < offx[offset_idx].size(); ++oi) {

                xs = atx + offx[offset_idx][oi];
                ys = aty + offy[offset_idx][oi];
                zs = atz + offz[offset_idx][oi];

                if (xs>=0 && xs<img_width && ys>=0 && ys<img_height && zs>=0 && zs<img_length) {

                    int iaux = zs*(img_width*img_height)+ys*img_width+xs;

                    if (trace_tag_map[iaux]==0) {
                        if (++cntbckg/(float)offx[offset_idx].size()>=bgratio) {
//                            cout << "|" << endl;
                            return;
                        }
                    }
                    else if (trace_tag_map[iaux]>0 && trace_tag_map[iaux]!=curr_tt) {
//                        if (trace_tag_map[iaux]==1) cout << "here!! " << node_tag_map[iaux] << endl;
                        ntags.push_back(node_tag_map[iaux]);
                        ttags.push_back(trace_tag_map[iaux]);
                    }

                }
                else {
                    cout << "out of img. boundaries" << endl;
                    return;
                }
            }

            ovlpPrev = ovlpCurr;
            ntagPrev = ntagCurr;

            ovlpCurr = ttags.size()>0;

//            if(ovlpCurr) cout << "x" << flush;
//            else cout << curr_tt << " " << flush;

            // cases:
            if (!ovlpPrev && !ovlpCurr) {

                // F->F

                // 1. add the node
                Node tracenode(xc[node_cnt], yc[node_cnt], zc[node_cnt], rc[node_cnt], Node::AXON);
                node_list.push_back(tracenode); // nodelist will have r as gcsstd

                // 2. set current node tag
                ntagCurr = node_list.size()-1;

                // 3. set trace tag

                // 4. tagmap fill
                for (int oi = 0; oi < offx[offset_idx].size(); ++oi) {

                    xs = atx + offx[offset_idx][oi];
                    ys = aty + offy[offset_idx][oi];
                    zs = atz + offz[offset_idx][oi];

                    int iaux = zs*(img_width*img_height)+ys*img_width+xs;
                    if (node_tag_map[iaux]<0)   node_tag_map[iaux] = ntagCurr;
                    if (trace_tag_map[iaux]<0)  trace_tag_map[iaux] = curr_tt;
                }

                // 5. linking
                node_list[ntagPrev].nbr.push_back(ntagCurr);
                node_list[ntagCurr].nbr.push_back(ntagPrev);

            }
            else if (!ovlpPrev && ovlpCurr) {

                // F->T

                // 1. add the node

                // 2. set current node tag
                ntagCurr = ntags[0]; //mode(ntags); closest to center

                // 3. set trace tag

                // 4. tagmap fill
                for (int oi = 0; oi < offx[offset_idx].size(); ++oi) {

                    xs = atx + offx[offset_idx][oi];
                    ys = aty + offy[offset_idx][oi];
                    zs = atz + offz[offset_idx][oi];

                    int iaux = zs*(img_width*img_height)+ys*img_width+xs;
                    if (node_tag_map[iaux]<0)   node_tag_map[iaux] = ntagPrev;
                    if (trace_tag_map[iaux]<0)  trace_tag_map[iaux] = curr_tt;
                }

                // 5. linking
                // it overlaps
                if (ttags.size()!=ntags.size()) cout << "error: ttags != ntags" << endl;
                extract(ttags, ntags, nlist);
                // nlist has nodes
                for (int k = 0; k < nlist.size(); ++k) {
                    node_list[ntagPrev].nbr.push_back(nlist[k]);
                    node_list[nlist[k]].nbr.push_back(ntagPrev);
                }

            }
            else if (ovlpPrev && !ovlpCurr) {

                // T->F

                // 1. add the node
                Node tracenode(xc[node_cnt], yc[node_cnt], zc[node_cnt], rc[node_cnt], Node::AXON);
                node_list.push_back(tracenode); // nodelist will have r as gcsstd

                // 2. set current node tag
                ntagCurr = node_list.size()-1;

                // 3. set trace tag
                curr_tt = ntagCurr;

                // 4. tagmap fill
                for (int oi = 0; oi < offx[offset_idx].size(); ++oi) {

                    xs = atx + offx[offset_idx][oi];
                    ys = aty + offy[offset_idx][oi];
                    zs = atz + offz[offset_idx][oi];

                    int iaux = zs*(img_width*img_height)+ys*img_width+xs;
                    if (node_tag_map[iaux]<0)   node_tag_map[iaux] = ntagCurr;
                    if (trace_tag_map[iaux]<0)  trace_tag_map[iaux] = curr_tt;
                }

                // 5. linking
                node_list[ntagPrev].nbr.push_back(ntagCurr);
                node_list[ntagCurr].nbr.push_back(ntagPrev);

            }
            else {

                // T->T

                // 1. add the node

                // 2. set current node tag
                ntagCurr = ntags[0]; //mode(ntags);

                // 3. set trace tag

                // 4. tagmap fill

                // 5. linking

            }


//            ovptrace = (trctags.size()/(float)cnttotal)>ovlpratio && (mode(trctags)!=curr_tt); // TODO::
            // reformulate how overlap is estimated and the tags are extracted
//            int cntlim = 1;//(int)round(ovlpratio*cnttotal);
//            vector<int> reachedtrces;
//            vector<int> reachednodes;
//            selec(trctags, ndetags, cntlim, curr_tt, reachedtrces, reachednodes); // list trctags[] that had more than cntlim but were not curr_tt
//            ovptrace = reachedtrces.size()>0; // if there were such - then overlap=T and the trace stopped, use values from reached to do linking
//            ovlp.push_back(ovptrace);
//            if (ovlpCurr) {
//                ttagCurr = ttagPrev;
//                ntagCurr = ntagPrev;
//                ndetag = mode(ndetags);
//                ntagg.push_back(ndetag);
//                trctag = mode(trctags);
//                ttagg.push_back(trctag);
//            }


        } // loop steps to estimate whether the trace should stop

        // last step accumulated Nstates directions, their average is used to calculate priors for the next iteration
        float norm = sqrt(vx_pr*vx_pr + vy_pr*vy_pr + vz_pr*vz_pr);

        vx_pr /= norm;
        vy_pr /= norm;
        vz_pr /= norm;

//        if (norm<2*FLT_MIN) {
//            cout<<"ERROR: null direction est.!!!" << endl;
//            return;
//        }
//        else {
//            vx_pr /= norm; // to make it unit direction - will be prior in the next iteration
//            vy_pr /= norm;
//            vz_pr /= norm;
//        }

    } // loop iterations

//    cout << ">" << flush;

}

void BTracer::iter_1(float xloc,  float yloc,  float zloc,
                     float vxloc, float vyloc, float vzloc, float rloc,
                     unsigned char *img,
                     int img_width,
                     int img_height,
                     int img_length,
                     float angula_diff_std_deg, //float gcsstd_diff_std_pix,
                     float vx_prior,
                     float vy_prior,
                     float vz_prior,
                     bool dbg) {

    for (int i_step = 0; i_step < Nsteps; ++i_step) { // Nsteps cover the scale 2*neuron_diameter+1

        int nsrcs = round(pow(Ndirs,i_step));

        for (int i_src = 0; i_src < nsrcs; ++i_src) {

            bool init_step = i_step == 0;

            float atx = (init_step)? xloc : xtt1[i_step-1][i_src];
            float aty = (init_step)? yloc : ytt1[i_step-1][i_src];
            float atz = (init_step)? zloc : ztt1[i_step-1][i_src];

            float atvx = (init_step)? vxloc : vxtt1[i_step-1][i_src];
            float atvy = (init_step)? vyloc : vytt1[i_step-1][i_src];
            float atvz = (init_step)? vzloc : vztt1[i_step-1][i_src];

            float atr  = (init_step)? rloc  : rtt1[i_step-1][i_src];

            // prediction + prior
            predict( atx,
                     aty,
                     atz,
                     atvx,
                     atvy,
                     atvz,
                     atr,
                     vx_prior,
                     vy_prior,
                     vz_prior,
                     angula_diff_std_deg, //gcsstd_diff_std_pix,
                     &xtt1[i_step][i_src*Ndirs],
                     &ytt1[i_step][i_src*Ndirs],
                     &ztt1[i_step][i_src*Ndirs], // & == provide with address
                     &vxtt1[i_step][i_src*Ndirs],
                        &vytt1[i_step][i_src*Ndirs],
                        &vztt1[i_step][i_src*Ndirs],
                        &rtt1[i_step][i_src*Ndirs],
                        &priortt1[i_step][i_src*Ndirs]
                        );

        }

        // likelihood
        int npreds = round(pow(Ndirs,i_step+1));

        float prior_sum = 0;

        int gcsstd_idx = -1;

        for (int i_pred = 0; i_pred < npreds; ++i_pred) { // loop current step

            float lh = znccX(
                        xtt1[i_step][i_pred],
                        ytt1[i_step][i_pred],
                        ztt1[i_step][i_pred],
                        vxtt1[i_step][i_pred],
                        vytt1[i_step][i_pred],
                        vztt1[i_step][i_pred],
                                        img,
                                        img_width, img_height, img_length,
                        gcsstd_idx
                        );

            rtt1[i_step][i_pred] = gcsstd[gcsstd_idx];

            lhoodtt1[i_step][i_pred] = exp(K*lh); // lh in [-1, 1]

            prior_sum += priortt1[i_step][i_pred]; // to have priors as probailities

        }

        // posterior (can be put together in the same loop with likelihood if there is no check on zero likelihood)
        float posterior_sum = 0;
        for (int i_pred = 0; i_pred < npreds; ++i_pred) { // loop current step

            postrtt1[i_step][i_pred] = ((i_step == 0)? 1.0 : postrtt1[i_step-1][i_pred/Ndirs]) *
                                       (priortt1[i_step][i_pred]/prior_sum) *
                                        lhoodtt1[i_step][i_pred];

            posterior_sum += postrtt1[i_step][i_pred];

        }

        // normalize posterior to recurse from it in the next iteration
        for (int i_pred = 0; i_pred < npreds; ++i_pred) {
            postrtt1[i_step][i_pred] /= posterior_sum;
        }

    } // Nsteps

//    if (dbg) {
//        printf("\ninitial iter particles:\n");
//    }

}

void BTracer::iter_N(float *                xlocs,
                     float *                ylocs,
                     float *                zlocs,
                     float *                vxlocs,
                     float *                vylocs,
                     float *                vzlocs,
                     float *                rlocs,
                     float *                wlocs,
                     unsigned char *        img,
                     int                    img_width,
                     int                    img_height,
                     int                    img_length,
                     float                  angula_diff_std_deg, //float gcsstd_diff_std_pix,
                     float                  vx_prior,
                     float                  vy_prior,
                     float                  vz_prior,
                     bool                   dbg) {

        for (int i_step = 0; i_step < Nsteps; ++i_step) { // Nsteps cover the scale 2*neuron_diameter+1

            int nsrcs = Nstates*round(pow(Ndirs,i_step));

            for (int i_src = 0; i_src < nsrcs; ++i_src) { // number of sources will depend on the step

                bool init_step = i_step == 0; // if init_step there will be Nstates xlocs, i_src in [0, Nstates)

                float atx = (init_step)? xlocs[i_src] : xttN[i_step-1][i_src];
                float aty = (init_step)? ylocs[i_src] : yttN[i_step-1][i_src];
                float atz = (init_step)? zlocs[i_src] : zttN[i_step-1][i_src];

                float atvx = (init_step)? vxlocs[i_src] : vxttN[i_step-1][i_src];
                float atvy = (init_step)? vylocs[i_src] : vyttN[i_step-1][i_src];
                float atvz = (init_step)? vzlocs[i_src] : vzttN[i_step-1][i_src];

                float atr  = (init_step)? rlocs[i_src]  : rttN[i_step-1][i_src];

                // prediction + prior
                predict( atx, aty, atz,
                         atvx, atvy, atvz, atr,
                         vx_prior, vy_prior, vz_prior,
                         angula_diff_std_deg, //gcsstd_diff_std_pix,
                         &xttN[i_step][i_src*Ndirs],
                         &yttN[i_step][i_src*Ndirs],
                         &zttN[i_step][i_src*Ndirs], // & == provide with address
                         &vxttN[i_step][i_src*Ndirs],
                         &vyttN[i_step][i_src*Ndirs],
                         &vzttN[i_step][i_src*Ndirs],
                         &rttN[i_step][i_src*Ndirs],
                         &priorttN[i_step][i_src*Ndirs]);

            }

            // likelihood
            int npreds = Nstates*round(pow(Ndirs,i_step+1));

            float prior_sum = 0;

            int gcsstd_idx = -1;

            for (int i_pred = 0; i_pred < npreds; ++i_pred) { // loop current step

                float lh = znccX(
                            xttN[i_step][i_pred],
                            yttN[i_step][i_pred],
                            zttN[i_step][i_pred],
                            vxttN[i_step][i_pred],
                            vyttN[i_step][i_pred],
                            vzttN[i_step][i_pred],
                                            img,
                                            img_width, img_height, img_length,
                            gcsstd_idx
                            );

                rttN[i_step][i_pred] = gcsstd[gcsstd_idx];

                lhoodttN[i_step][i_pred] = exp(K*lh); // ln in [-1, 1]

                prior_sum += priorttN[i_step][i_pred]; // to have priors as probailities

            }

            // posterior (can be put together in the same loop with likelihood if there is no check on zero likelihood)
            float posterior_sum = 0;
            for (int i_pred = 0; i_pred < npreds; ++i_pred) {

                postrttN[i_step][i_pred] = ((i_step == 0)? wlocs[i_pred/Ndirs] : postrttN[i_step-1][i_pred/Ndirs]) *
                                           (priorttN[i_step][i_pred]/prior_sum) *
                                            lhoodttN[i_step][i_pred];

                posterior_sum += postrttN[i_step][i_pred];

            }


            // normalize posterior to recurse from it in the next iteration
            for (int i_pred = 0; i_pred < npreds; ++i_pred) {
                postrttN[i_step][i_pred] /= posterior_sum;// (posterior_sum<=2*FLT_MIN)? (1/(float)npreds) : (postrttN[i_step][i_pred]/posterior_sum) ;
            }

//            // debug
//            cout << i_step << "\t : " << endl;
//            for (int i_pred = 0; i_pred < npreds; ++i_pred) {
//                cout << lhoodttN[i_step][i_pred] << "\t";
//            }
//            cout << endl;

//            cout << (i_pred%Ndirs) << "\t"
//                 << postrttN[i_step][i_pred] << " = "
//                 << ((i_step == 0)? wlocs[i_pred/Ndirs] : postrttN[i_step-1][i_pred/Ndirs]) << " x "
//                 << " x "
//                 << lhoodttN[i_step][i_pred] << "\n";


        } // Nsteps

//    if (dbg) {
//        printf("\ninitial iter particles:\n");
//    }

}

void BTracer::predict(float px, float py, float pz,
                      float vx, float vy, float vz,
                      float r_pv,
                      float vx_prior,
                      float vy_prior,
                      float vz_prior,
                      float angula_diff_std_deg, //float gcsstd_diff_std_pix,
                      float * px_out, float * py_out, float * pz_out,
                      float * vx_out, float * vy_out, float * vz_out,
                      float * r_out,
                      float * prior_out
                      ){

    // set of template directions vx_template, vy_template, vz_template
    // is rotated to align with vx, vy, vz as axis
    // they are immediately stored to vx_out, vy_out, vz_out


    // px_out, py_out, pz_out are obtained after translation
    // prior_out are corresponding priors

        if (is2D) {
            // template has tip at (0,1,0)
            rotation_matrix(0,1,0, vx,vy,vz, rot);

            for (int i = 0; i < Ndirs; ++i) {
                // each point will give Ndirs outputs
                rotation_apply(rot,
                               vx_template[i],vy_template[i],vz_template[i],
                               vx_out[i], vy_out[i], vz_out[i]);

                vz_out[i] = 0;

                px_out[i] = px + step * vx_out[i];
                py_out[i] = py + step * vy_out[i];
                pz_out[i] = 0;


                float dot_prod = vx_out[i] * vx_prior + vy_out[i] * vy_prior + vz_out[i] * vz_prior;
                dot_prod = (dot_prod>1)? 1 : (dot_prod<-1)? -1 : dot_prod;
                prior_out[i] = exp(-pow(acos(dot_prod)*(180.0/3.14),2)/(2*pow(angula_diff_std_deg,2)));

                r_out[i] = r_pv;

            }

        }
        else {
            // template has tip at (0,0,-1)
            for (int i = 0; i < Ndirs; ++i) {
                rotation_matrix(0,0,-1, vx,vy,vz, rot);
                rotation_apply(rot,
                               vx_template[i],vy_template[i],vz_template[i],
                               vx_out[i],vy_out[i],vz_out[i]);

                px_out[i] = px + step * vx_out[i];
                py_out[i] = py + step * vy_out[i];
                pz_out[i] = pz + step * vz_out[i] / zDist;

                float dot_prod = vx_out[i] * vx_prior + vy_out[i] * vy_prior + vz_out[i] * vz_prior;
                dot_prod = (dot_prod>1)? 1 : (dot_prod<-1)? -1 : dot_prod;
                prior_out[i] = exp(-pow(acos(dot_prod)*(180.0/3.14),2)/(2*pow(angula_diff_std_deg,2)));

                r_out[i] = r_pv;

            }

        }
}


//float BTracer::zncc( float x, float y, float z,
//                     float vx, float vy, float vz,
//                     unsigned char * img, int img_w, int img_h, int img_l,
//                     float & r, bool dbg
//                    ){

//    // repeat for each template...
//    // loop through U,V,W offsets, sample from locs by interpolating
//    // downscale z value always
//    // loop the same way as when templates were formed to be compliant

//    float nrm = sqrt(pow(vx,2)+pow(vy,2));
//    if (nrm>0.001) {
//        ux = ((vy<0)? -1 : 1)*(vy/nrm);
//        uy = ((vy<0)?  1 :-1)*(vx/nrm);
//        uz = 0;
//    }
//    else {
//        ux = 1; uy = 0; uz = 0;
//    }

//    float ag = 0; // average

//    if (is2D) { // one orthogonal is necessary

//        wx = 0;
//        wy = 0;
//        wz = 0;

//        for (int vv = -V2; vv <= V2; ++vv) { // template length: V*U*1
//                for (int uu = -U2; uu <= U2; ++uu) {
//                    //for (int ww = -W2; ww <= W2; ++ww) { // thos one will

//                        int v1 = vv + V2; // 0- V-1
//                        int u1 = uu + U2; // 0- U-1

//                        float xcoord = vv*(-vx) + uu*ux; // x components of all three orthogonals
//                        float ycoord = vv*(-vy) + uu*uy;

//                        if ( floor(x+xcoord)<0 || ceil(x+xcoord)>=img_w-1) return 0;
//                        if ( floor(y+ycoord)<0 || ceil(y+ycoord)>=img_h-1) return 0;

//                        float value = interp(x+xcoord, y+ycoord, 0, img, img_w, img_h, img_l, false);
//                        img_vals[v1*U+u1] = value;
//                        ag += value;
////if (dbg) printf("\ncalled interp at %f, %f - got -> %f\n", x+xcoord, y+ycoord, value);
//                    //}
//                }
//            }
//        }
//        else { // two orthogonals

//            wx = uy*vz - uz*vy;
//            wy = ux*vz - uz*vx;
//            wz = ux*vy - uy*vx;

//            for (int vv = -V2; vv <= V2; ++vv) { // template length: V*U*W
//                for (int uu = -U2; uu <= U2; ++uu) {
//                    for (int ww = -W2; ww <= W2; ++ww) { // thos one will

//                        int v1 = vv + V2; // 0-V
//                        int u1 = uu + U2; // 0-U
//                        int w1 = ww + W2; // 0-W

//                        float xcoord = vv*(-vx) + uu*ux + ww*wx; // x components of all three orthogonals
//                        float ycoord = vv*(-vy) + uu*uy + ww*wy;
//                        float zcoord = vv*(-vz) + uu*uz + ww*wz;
//                        // z coord is downscaled when sampling the image value for measurement
//                        zcoord = zcoord/zDist;

//                        if ( floor(x+xcoord)<0 || ceil(x+xcoord)>=img_w-1) return 0;
//                        if ( floor(y+ycoord)<0 || ceil(y+ycoord)>=img_h-1) return 0;
//                        if ( floor(z+zcoord)<0 || ceil(z+zcoord)>=img_l-1) return 0;

//                        float value = interp(x+xcoord, y+ycoord, z+zcoord, img, img_w, img_h, img_l, false);
//                        img_vals[v1*U*W+w1*U+u1] = value;
//                        ag += value;
//                    }
//                }

//            }

//    }

//    ag = ag / (U*V*W);

//    // find correlation with corresponding template(s)
//    float out_corr = -FLT_MAX; // ensure that at least one score will be higher
//    float out_r = -FLT_MAX;

//    for (int tidx = 0; tidx < gcsstd_nr; ++tidx) {

//        // check how it correlates with tt[tidx], take max

//        float corra = 0;
//        float corrb = 0;
//        float corrc = 0;

//        for (int k = 0; k <U*W*V; k++) {
//            corra += (img_vals[k]-ag) * (tt[tidx][k]-tta[tidx]);
//            corrb += pow(img_vals[k]-ag, 2);
//            corrc += pow(tt[tidx][k]-tta[tidx], 2);
//        }

//        float corr_val = (corrb*corrc>0.0001)? corra/sqrt(corrb*corrc) : 0;

//        if (corr_val>out_corr) {
//            out_corr = corr_val;
//            out_r = gcsstd[tidx];
//        }

//    }

//    r = out_r;
//    return out_corr;

//}

float BTracer::znccX( float x, float y, float z,
                     float vx, float vy, float vz,
                     unsigned char * img, int img_w, int img_h, int img_l,
                     int & gcsstd_idx
                    ){

    // loop through U,V,W offsets, sample from locs by interpolating
    // downscale z
    // loop the same way as when templates were formed to be compliant

    float nrm = sqrt(pow(vx,2)+pow(vy,2));
    if (nrm>0.001) {
        ux = ((vy<0)? -1 : 1)*(vy/nrm);
        uy = ((vy<0)?  1 :-1)*(vx/nrm);
        uz = 0;
    }
    else {
        ux = 1; uy = 0; uz = 0;
    }

    float ag = 0; // average

    if (is2D) { // one orthogonal is necessary

        wx = 0;
        wy = 0;
        wz = 0;

        for (int vv = -V2; vv <= V2; ++vv) { // template length: V*U*1
                for (int uu = -U2; uu <= U2; ++uu) {
                    //for (int ww = -W2; ww <= W2; ++ww) { // thos one will
                        int v1 = vv + V2; // 0- V-1
                        int u1 = uu + U2; // 0- U-1

                        float xcoord = vv*(-vx) + uu*ux; // x components of all three orthogonals
                        float ycoord = vv*(-vy) + uu*uy;

                        if ( floor(x+xcoord)<0 || ceil(x+xcoord)>=img_w-1) return 0;
                        if ( floor(y+ycoord)<0 || ceil(y+ycoord)>=img_h-1) return 0;

                        float value = interp(x+xcoord, y+ycoord, 0, img, img_w, img_h, img_l, false);
                        img_vals[v1*U+u1] = value;
                        ag += value;
                    //}
                }
            }
        }
        else { // two orthogonals

            wx = uy*vz - uz*vy;
            wy = ux*vz - uz*vx;
            wz = ux*vy - uy*vx;

            for (int vv = -V2; vv <= V2; ++vv) { // template length: V*U*W
                for (int uu = -U2; uu <= U2; ++uu) {
                    for (int ww = -W2; ww <= W2; ++ww) { // thos one will

                        int v1 = vv + V2; // 0-V
                        int u1 = uu + U2; // 0-U
                        int w1 = ww + W2; // 0-W

                        float xcoord = vv*(-vx) + uu*ux + ww*wx; // x components of all three orthogonals
                        float ycoord = vv*(-vy) + uu*uy + ww*wy;
                        float zcoord = vv*(-vz) + uu*uz + ww*wz;
                        // z coord is downscaled when sampling the image value for measurement
                        zcoord = zcoord/zDist;

                        if ( floor(x+xcoord)<0 || ceil(x+xcoord)>=img_w-1) return 0;
                        if ( floor(y+ycoord)<0 || ceil(y+ycoord)>=img_h-1) return 0;
                        if ( floor(z+zcoord)<0 || ceil(z+zcoord)>=img_l-1) return 0;

                        float value = interp(x+xcoord, y+ycoord, z+zcoord, img, img_w, img_h, img_l, false);
                        img_vals[v1*U*W+w1*U+u1] = value;
                        ag += value;
                    }
                }

            }

    }

    ag = ag / (U*V*W);

    // find correlation with corresponding template(s)
    float out_corr = -FLT_MAX; // ensure that at least one score will be higher
    int out_r_idx = -1; // index

    for (int tidx = 0; tidx < gcsstd_nr; ++tidx) {

        // check how it correlates with tt[tidx], take max

        float corra = 0;
        float corrb = 0;
        float corrc = 0;

        for (int k = 0; k <U*W*V; k++) {
            corra += (img_vals[k]-ag) * (tt[tidx][k]-tta[tidx]);
            corrb += pow(img_vals[k]-ag, 2);
            corrc += pow(tt[tidx][k]-tta[tidx], 2);
        }

        float corr_val = (corrb*corrc>0.0001)? corra/sqrt(corrb*corrc) : 0;

        if (corr_val>out_corr) {
            out_corr = corr_val;
            out_r_idx = gcsstd[tidx];
        }

    }

    gcsstd_idx = out_r_idx;
    return out_corr;

}

void BTracer::save_templates(V3DPluginCallback2 &callback, QString inimg_path) {

    unsigned char * tplt;
    tplt = new unsigned char[V*U*W];

    for (int ti = 0; ti < gcsstd_nr; ++ti) {

        QString out_str = inimg_path + QString("_template%1.tif").arg(ti);
//        printf("saving %s\n",out_str.toStdString().c_str());

        V3DLONG in_sz[4];

        if (is2D) { // indexing is differnt in 2d versus 3d, v is y axis in 2d and z axis in 3d
            in_sz[0] = (long) U;
            in_sz[1] = (long) V;
            in_sz[2] = (long) W;
            in_sz[3] = (long) 1;
        }
        else {
            in_sz[0] = (long) U;
            in_sz[1] = (long) W;
            in_sz[2] = (long) V;
            in_sz[3] = (long) 1;
        }

        // save image, convert each kernel to unsigned char * with elements 0-255
        for (int kk = 0; kk < V*U*W; ++kk) {
            tplt[kk] = tt[ti][kk]*255.0;
        }

        simple_saveimage_wrapper(callback, out_str.toStdString().c_str(), tplt, in_sz, V3D_UINT8);

//        Image4DSimple outimg1;
//        outimg1.setData(knl, in_sz[0], in_sz[1], in_sz[2], in_sz[3], V3D_UINT8);
//        callback.saveImage(&outimg1, (char *)out_str.toStdString().c_str());

//        printf("what was it?\n");
//        for (int var = 0; var < Lxy*Lxy*Lz; ++var) {
//            printf("%6.2f (%d,%d,%d)", kernels[ki][var], dx[var], dy[var], dz[var]);
//        }

    }

    delete [] tplt;
    tplt = 0;

}

