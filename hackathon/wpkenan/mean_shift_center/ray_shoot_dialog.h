#ifndef RAY_SHOOT_DIALOG_H
#define RAY_SHOOT_DIALOG_H

#include "mean_shift_fun.h"

class ray_shoot_dialog:public QDialog
{
    Q_OBJECT
public:
    ray_shoot_dialog(V3DPluginCallback2 *cb);
    mean_shift_fun mean_shift_obj;
    void core();

private:

    V3DPluginCallback2 *callback;
    v3dhandle curwin;
    V3DLONG sz_img[4];
    unsigned char *image1Dc_in;
    LandmarkList LList,LList_in,LList_new_center;
    vector<V3DLONG> poss_landmark;
    vector<float> mass_center;
    ImagePixelType pixeltype;

};

#endif // RAY_SHOOT_DIALOG_H
