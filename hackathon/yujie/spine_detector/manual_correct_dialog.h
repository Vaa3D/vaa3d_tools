#ifndef MANUAL_CORRECT_DIALOG_H
#define MANUAL_CORRECT_DIALOG_H

//#include "detect_fun.h"
#include "v3d_interface.h"
#include "spine_detector_dialog.h"
#include "spine_fun.h"

using namespace std;

class manual_correct_dialog:public QDialog
{
    Q_OBJECT
public:
    explicit manual_correct_dialog(V3DPluginCallback2 *cb);
    void create();
    void reset_image_data();
    void check_window();
    void open_main_triview();
    void adjust_LList_to_imagetrun();
    int save_edit();
    void create_standing_dialog();
    bool auto_spine_detect();
    bool before_proof_dialog();
    void reset_label_group();
    void get_para();
    int save();
    void GetColorRGB(int* rgb, int idx);
    void write_spine_profile(QString filename);

public slots:
    void dialoguefinish(int);
    bool loadImage();
    bool load_swc();
    bool csv_out();
    void check_data();

    void accept_marker();
    void delete_marker();
    void skip_marker();
    void marker_roi();
    void dilate();
    void erode();
    void reset_edit();
    bool finish_proof_dialog();

    //not used
    bool maybe_save();
    void load_marker();
    void loadLabel();
    void check_local_3d_window();

private:
    V3DPluginCallback2 *callback;
    v3dhandle curwin,main_win;
    View3DControl * v3dcontrol;
    QLineEdit *edit_load, *edit_label,*edit_marker, *edit_swc,*edit_csv;
    QPlainTextEdit *edit_status;
    QDialog *mydialog;
    QLabel *marker_info;
    QComboBox *markers,*channel_menu;
    QSpinBox *spin_max_pixel, *spin_min_pixel,*spin_max_dis,*spin_bg_thr,
             *spin_width_thr;
    QPushButton *btn_load,*btn_swc,*btn_csv;

    V3DLONG sz_img[4],sz[4];
    unsigned char *image1Dc_in,*image_trun, *image1Dc_spine;
    int intype;
    NeuronTree neuron;
    LandmarkList LList_in,LList_adj;
    QString fname_image,folder_output,fname_output;
    int x_start,y_start,z_start,sel_channel;
    ROIList pRoiList;
    vector<GOV> label_group,label_group_copy;
    bool edit_flag;
    parameters all_para;
    int rgb[3];

};

#endif // MANUAL_CORRECT_DIALOG_H
