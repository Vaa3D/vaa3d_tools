#ifndef FILE_IO_DIALOG_H
#define FILE_IO_DIALOG_H

#include "v3d_interface.h"
#include "spine_fun.h"
#include "is_analysis_fun.h"


class file_io_dialog:public QDialog
{
    Q_OBJECT
public:
    file_io_dialog(V3DPluginCallback2 *cb, int code);
    void create();
    void initDlg();
    bool auto_spine_detect_invoke();
    bool is_detect_invoke();

private:
    void get_para();
    bool check_button();
    bool load_image();
    bool load_swc();
    bool save_project();
    int maybe_proofread();

    //for is detection
    void create_is();

public slots:
    bool get_image_name();
    bool get_swc_name();
    bool csv_out();
    void run();
    void run_is();

private:
    V3DPluginCallback2 *callback;
    QLineEdit *edit_load, *edit_swc,*edit_csv;
    QPushButton *btn_load,*btn_swc,*btn_csv,*btn_run;
    QComboBox *channel_menu, *channel_is_menu;
    QSpinBox *spin_max_pixel, *spin_min_pixel,*spin_max_dis,*spin_bg_thr,
             *spin_width_thr, *spin_bg_thr2;
    int sel_channel;
    LandmarkList LList_in;
    QString input_image_name,input_swc_name,folder_output;
    bool eswc_flag;
    unsigned char *image1Dc_in;
    int intype;
    V3DLONG sz_img[4];
    NeuronTree neuron,checked_neuron;
    parameters all_para;

    vector<GOV> label_group;

};

#endif // FILE_IO_DIALOG_H
