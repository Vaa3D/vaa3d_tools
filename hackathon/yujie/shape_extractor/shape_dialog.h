#ifndef SHAPE_DIALOG_H
#define SHAPE_DIALOG_H

#include <QtGui>
#include <QDialog>
#include <QToolBar>
#include <QVBoxLayout>
#include <QToolButton>
#include <v3d_interface.h>
#include "extract_fun.h"

using namespace std;

#define NAME_INWIN "Input_Shape_Extractor"
#define NAME_OUTWIN "Output_Shape_Extractor"


class shape_dialog: public QDialog
{
    Q_OBJECT
public:
    explicit shape_dialog(V3DPluginCallback2 *cb);
    int prev_bgthr;
    int prev_distance;
    int prev_conviter;
    int prev_percent;

private:
    extract_fun shape_ext_obj;
    V3DPluginCallback2 *callback;
    V3DLONG sz_img[4];
    int intype;
    unsigned char *image1Dc_in;
    unsigned char *image1Dc_out;
    LandmarkList LList;
    LandmarkList LList_new_center;
    vector<V3DLONG> poss_landmark;
    vector<V3DLONG> x_all;
    vector<V3DLONG> y_all;
    vector<V3DLONG> z_all;
    vector<V3DLONG> mass_center;

    QPlainTextEdit *edit;
    QSpinBox *spin_distance,*spin_bgthr,*spin_conviter,*spin_percent;
    QDialog *subDialog;
private:

    void create();
    void updateInputWindow();
    void updateOutputWindow();

    void convert2UINT8(unsigned short *pre1d, unsigned char *pPost, V3DLONG imsz);
    void convert2UINT8(float *pre1d, unsigned char *pPost, V3DLONG imsz);

public slots:
    bool load();
    int loadMarkers();
    void extract();
    void display_mass_center();
    void dialoguefinish(int);
    void clear_markers();
};
#endif // SHAPE_DIALOG_H
