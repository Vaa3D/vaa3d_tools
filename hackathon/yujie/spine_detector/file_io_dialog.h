#ifndef FILE_IO_DIALOG_H
#define FILE_IO_DIALOG_H

#include "v3d_interface.h"
#include "spine_fun.h"
#include "spine_detector_dialog.h"

class file_io_dialog:public QDialog
{
    Q_OBJECT
public:
    file_io_dialog(V3DPluginCallback2 *cb);
    void create();
    void initDlg();
    bool auto_spine_detect_invoke();


private:
    void get_para();
    bool check_button();
    bool load_image();
    bool load_swc();
    int calc_nearest_node(float center_x,float center_y,float center_z);
    bool save_auto_detect_result();
public slots:
    bool get_image_name();
    bool get_swc_name();
    bool csv_out();
    void run();

private:
    V3DPluginCallback2 *callback;
    QLineEdit *edit_load, *edit_swc,*edit_csv;
    QPushButton *btn_load,*btn_swc,*btn_csv,*btn_run;
    QComboBox *channel_menu;
    QSpinBox *spin_max_pixel, *spin_min_pixel,*spin_max_dis,*spin_bg_thr,
             *spin_width_thr;
    int sel_channel;

    QString input_image_name,input_swc_name,folder_output;
    bool eswc_flag;
    unsigned char *image1Dc_in;
    int intype;
    V3DLONG sz_img[4];
    NeuronTree neuron;
    parameters all_para;

    vector<GOV> label_group;

};

#endif // FILE_IO_DIALOG_H
