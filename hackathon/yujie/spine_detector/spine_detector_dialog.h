#ifndef SPINE_DETECTOR_DIALOG_H
#define SPINE_DETECTOR_DIALOG_H

#include "detect_fun.h"
#include "v3d_interface.h"

using namespace std;

class spine_detector_dialog:public QDialog
{
     Q_OBJECT
public:
    explicit spine_detector_dialog(V3DPluginCallback2 *cb);
    bool load_image();
    void convert2UINT8(unsigned short *pre1d, unsigned char *pPost, V3DLONG imsz);
    void convert2UINT8(float *pre1d, unsigned char *pPost, V3DLONG imsz);
    void core();
    void create();
    void spine_find();
    void count_neighbors(unsigned char *boundary,unsigned char *single);
    void preprosessing();
    void adaptive_thres(int *neuron_id, float *bound_box);
public slots:
    void load_swc();
    void loadImage();
    void dialoguefinish(int);
    void distance_measure();
    void cell_segment();
    void loadMask();
private:
    detect_fun detect_obj;
    V3DPluginCallback2 *callback;
    v3dhandle curwin;
    V3DLONG sz_img[4],mask_sz[4];
    unsigned char *image1Dc_in,*image1Dc_r_channel,*mask_data;
    LandmarkList LList_in;
    NeuronTree neuron;
    QLineEdit *edit_swc, *edit_load;
    int intype,mask_intype;


};

#endif // SPINE_DETECTOR_DIALOG_H
