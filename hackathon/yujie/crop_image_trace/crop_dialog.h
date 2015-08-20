#ifndef CROP_DIALOG_H
#define CROP_DIALOG_H

#include <QtGui>
#include "v3d_interface.h"
#include "vector"

using namespace std;

bool load_data(V3DPluginCallback2 *cb, unsigned char * & image1Dc_in, ImagePixelType &pixeltype,
               V3DLONG sz_img[4], NeuronTree &neuron, v3dhandle &curwin);
void backupNeuron(const NeuronTree & source, const NeuronTree & backup);
vector<V3DLONG> pos2xyz(const V3DLONG _pos_input, const V3DLONG _offset_Y, const V3DLONG _offset_Z);
V3DLONG xyz2pos(const V3DLONG _x, const V3DLONG _y, const V3DLONG _z, const V3DLONG _offset_Y, const V3DLONG _offset_Z);

class crop_dialog:public QDialog
{
     Q_OBJECT
public:
    crop_dialog(V3DPluginCallback2 *cb);
    crop_dialog::~crop_dialog();
    void getData();

private:
    bool check_channel();
    bool check_button();
    void initDlg();

public slots:
    void crop();
    bool csv_out();

private:
    V3DPluginCallback2 *callback;
    unsigned char *image1Dc_in;
    V3DLONG sz_img[4];
    v3dhandle curwin;
    NeuronTree neuron;
    QString folder_output;
    QDialog *mydialog2;
    QComboBox *channel_menu;
    QPushButton *ok;
    QLineEdit *edit_csv;
    QSpinBox *spin_x_min,*spin_y_min,*spin_z_min,*spin_x_max,*spin_y_max,*spin_z_max;
};

#endif // CROP_DIALOG_H
