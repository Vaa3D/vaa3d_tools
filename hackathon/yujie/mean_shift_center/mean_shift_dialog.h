#ifndef MEAN_SHIFT_DIALOG_H
#define MEAN_SHIFT_DIALOG_H

#include <QtGui>
#include <QDialog>
#include <QToolBar>
#include <QVBoxLayout>
#include <QToolButton>
#include <v3d_interface.h>
#include "basic_surf_objs.h"
#include "mean_shift_fun.h"

#define NAME_INWIN "Input_mean_shift_center"
#define NAME_OUTWIN "Output_mean_shift_center"

class mean_shift_dialog:public QDialog
{
    Q_OBJECT
public:
    mean_shift_dialog(V3DPluginCallback2 *cb);
    void create();
    void convert2UINT8(unsigned short *pre1d, unsigned char *pPost, V3DLONG imsz);
    void convert2UINT8(float *pre1d, unsigned char *pPost, V3DLONG imsz);

private:
    void resetdata();
    void updateInputWindow();
    void updateOutputWindow();

private:
    mean_shift_fun mean_shift_obj;
    V3DPluginCallback2 *callback;
    V3DLONG sz_img[4];
    int intype;
    unsigned char *image1Dc_in;
    v3dhandle curwin;
    int datasource; //fetched datasource=2; load datasource=1;
    LandmarkList LList,LList_new_center;
    vector<V3DLONG> poss_landmark;
    vector<V3DLONG> x_all,y_all,z_all,mass_center;

public slots:
    bool load();
    void fetch();
    void mean_shift_center();
    void help();
};




#endif // MEAN_SHIFT_DIALOG_H
