#ifndef MANUAL_PROOFREAD_DIALOG_H
#define MANUAL_PROOFREAD_DIALOG_H

#include <QtGui>
#include "v3d_interface.h"
#include "spine_detector_dialog.h"
#include "spine_fun.h"

struct init_para{
    int bg_thr;
    int max_dis;
    int channel;
};

class manual_proofread_dialog:public QDialog
{
    Q_OBJECT
public:
    explicit manual_proofread_dialog(V3DPluginCallback2 *cb);
    void create();



private:
    bool readProjFile();
    bool check_btn();
    bool loadImage();
    bool loadSWC();
    bool loadLabel();
    bool prep_data();

public slots:
    bool get_proj_name();
    void run();
    //void dialoguefinish(int);






private:
    V3DPluginCallback2 *callback;
    bool edit_flag,seg_edit_flag;
    unsigned char *image1Dc_in;
    unsigned short *label;
    V3DLONG sz_img[4],label_sz_img[4];
    NeuronTree neuron;

    QString input_proj_name,input_image_name,input_label_name,input_swc_name,input_csv_name;
    QLineEdit *edit_load;
    QComboBox *view_menu;
    QPushButton *btn_run;

    init_para auto_para;

    bool eswc_flag;
};

#endif // MANUAL_PROOFREAD_DIALOG_H
