#ifndef APP2_H
#define APP2_H

#include <v3d_interface.h>

#include "imgpreprocess.h"

struct paraApp2
{
    Image4DSimple * p4dImage;
    int xc0, xc1, yc0, yc1, zc0, zc1; //the six bounding box boundaries
    LandmarkList landmarks;

    bool is_gsdt;
    bool is_coverage_prune;
    bool is_break_accept;
    int  bkg_thresh;
    double length_thresh;
    int  cnn_type;
    int  channel;
    double SR_ratio;
    int  b_256cube;
    bool b_RadiusFrom2D; //how to estimate radius of each reconstruction node, from 2D plane (for anisotropic case) or 3D (for isotropic case)
    int b_resample;
    int b_intensity;
    int b_brightfiled;
    bool s2Mode;

    bool b_menu;

    NeuronTree result;

    paraApp2() {

        p4dImage = NULL;
        xc0 = xc1 = yc0 = yc1 = zc0 = zc1 = 0;
        landmarks.clear();

        is_gsdt = false; //true; change to false by PHC, 2012-10-11. as there is clear GSDT artifacts
        is_coverage_prune = true;
        is_break_accept = false;
        bkg_thresh = 10; //30; change to 10 by PHC 2012-10-11
        length_thresh = 5; // 1.0; change to 5 by PHC 2012-10-11
        cnn_type = 2; // default connection type 2
        channel = 0;
        SR_ratio = 3.0/9.0;
        b_256cube = 1; //whether or not preprocessing to downsample to a 256xYxZ cube UINT8 for tracing
        b_RadiusFrom2D = true;
        b_resample = 1;
        b_intensity = 0;
        b_brightfiled = 0;
        b_menu = true;
    }
};

bool proc_app2(paraApp2 &p);


#endif // APP2_H
