#ifndef FILTER_DIALOG_H
#define FILTER_DIALOG_H

#include <QtGui>
#include <QDialog>
#include <QToolBar>
#include <QVBoxLayout>
#include <QToolButton>
#include <v3d_interface.h>
#include <vector>

void ComputemaskImage(NeuronTree neurons,unsigned char* pImMask,V3DLONG sx,V3DLONG sy,V3DLONG sz);
void BoundNeuronCoordinates(NeuronTree & neuron, double & output_xmin,double & output_xmax,
                            double & output_ymin,double & output_ymax,double & output_zmin,double & output_zmax);
QHash<V3DLONG, V3DLONG> NeuronNextPn(const NeuronTree &neurons);

class filter_dialog:public QDialog
{
    Q_OBJECT

public:
    filter_dialog(V3DPluginCallback2 *cb);
    V3DPluginCallback2 *callback;


private:
    void create();
    V3DLONG sz_img[4];
    int intype;
    unsigned char *image_data;
    NeuronTree neuron;
    void convert2UINT8(unsigned short *pre1d, unsigned char *pPost, V3DLONG imsz);
    void convert2UINT8(float *pre1d, unsigned char *pPost, V3DLONG imsz);
    void swc_filter_image();

    QGridLayout *mygridLayout;
    QLineEdit *edit_load,*edit_swc;

public slots:
    void load_swc();
    void loadImage();
    void dialoguefinish(int);

};

#endif // FILTER_DIALOG_H
