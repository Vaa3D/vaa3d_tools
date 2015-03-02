#ifndef SHAPE_DIALOG_H
#define SHAPE_DIALOG_H

#include <QtGui>
#include <QDialog>
#include <QToolBar>
#include <QVBoxLayout>
#include <QToolButton>
#include <v3d_interface.h>
#include "extract_fun.h"
#include "basic_surf_objs.h"

using namespace std;

#define NAME_INWIN "Input_Shape_Extractor"
#define NAME_OUTWIN "Output_Shape_Extractor"

NeuronTree readSWC_file(const QString& filename);
void ComputemaskImage(NeuronTree neurons,
                      unsigned char* pImMask, //output mask image
                      unsigned char* ImMark,  //an indicator image to show whether or not a pixel has been visited/processed
                      V3DLONG sx,V3DLONG sy,V3DLONG sz,int method_code);
QHash<V3DLONG, V3DLONG> NeuronNextPn(const NeuronTree &neurons);


class shape_dialog: public QDialog
{
    Q_OBJECT
public:
    explicit shape_dialog(V3DPluginCallback2 *cb);
    int prev_bgthr;
    int prev_conviter;


private:
    extract_fun shape_ext_obj;
    V3DPluginCallback2 *callback;
    V3DLONG sz_img[4];
    int intype;
    unsigned char *image1Dc_in,*image1Dc_out, *label;
    LandmarkList LList,LList_new_center;
    vector<V3DLONG> poss_landmark,volume;
    vector<V3DLONG> x_all,y_all,z_all,mass_center;
    int rgb[3];
    v3dhandle curwin;
    int datasource; //fetched datasource=2; load datasource=1;
    NeuronTree* nt;

    QPlainTextEdit *edit;
    QSpinBox *spin_bgthr,*spin_conviter,*spin_percent;
    QDialog *subDialog;

private:

    void create();
    void updateInputWindow();
    void updateOutputWindow();
    void GetColorRGB(int* rgb, int idx);
    void resetdata();
    void convert2UINT8(unsigned short *pre1d, unsigned char *pPost, V3DLONG imsz);
    void convert2UINT8(float *pre1d, unsigned char *pPost, V3DLONG imsz);
    void swc_to_maskimage(V3DPluginCallback2 &callback, int method_code);


public slots:
    bool load();
    void extract();
    void display_mass_center();
    void dialoguefinish(int);
    //void clear_markers();
    void fetch();
    void display_volume();
    void load_swc_filter();
};
#endif // SHAPE_DIALOG_H
