#ifndef MANUAL_PROOFREAD_DIALOG_H
#define MANUAL_PROOFREAD_DIALOG_H

#include <QtGui>
#include "v3d_interface.h"
#include "spine_fun.h"

struct init_para{
    int bg_thr;
    int max_dis;
    int channel;
    int seg_id;
    int marker_id;
};

class manual_proofread_dialog:public QDialog
{
    Q_OBJECT
public:
    explicit manual_proofread_dialog(V3DPluginCallback2 *cb,bool menu_flag);
    void create();
    void create_proofread_panel_by_spine();
    void create_proofread_panel_by_segment(bool exist_proj);
    void run_interface_with_auto(int ret, NeuronTree auto_neuron, bool eswc_tmp_flag,
        LandmarkList LList, vector<GOV> label_group_in, unsigned char *image1Dc, V3DLONG sz_img_in[4],
        int bg_thr, int max_dis, int channel, QString in_swc_name, QString in_image_name, QString folder_output);

private:
    //share by both views
    bool readProjFile();
    bool check_btn();
    bool loadImage();
    bool loadSWC();
    bool loadLabel();
    bool loadMarker(LandmarkList &LList_in);
    void initDlg();
    bool prep_spine_voi();
    void open_trunc_triview(QString trunc_win_name); //both views share
    void open_main_triview(QString main_win_name);
    void check_each_seg();

    //view by spine
    void reset_image_data();
    void reset_label_group();
    void adjust_LList_to_imagetrun();
    int save_edit();
    int finish_and_save(QString window_name);

    //view by segment
    void set_trunc_image_marker(vector<int> one_seg,int seg_id);
    vector<V3DLONG> image_seg_plan(vector<int> seg, NeuronTree *neuron_tmp);
    void reset_segment();
    void reset_edit_for_seg_view(bool visual_flag, int mid);
    int save_edit_for_seg_view();
    bool save_seg_edit_for_seg_view();


public slots:
    bool get_proj_name();
    void run_saved_proj();

    //view by spine
    void accept_marker();
    void delete_marker();
    void skip_marker();
    void dilate();
    void erode();
    void reset_edit();
    void marker_roi();
    bool finish_proof_dialog();
    bool save_current_project();

    //view by segment
    void segment_change();
    void marker_in_one_seg();
    void finish_seg_clicked();
    void reset_segment_clicked();
    void reject_marker_for_seg_view();
    void accept_marker_for_seg_view();
    void skip_segment_clicked();
    void dilate_for_seg_view();
    void erode_for_seg_view();
    void reset_marker_clicked_for_seg_view();
    bool finish_proof_dialog_seg_view();

private:
    V3DPluginCallback2 *callback;
    bool edit_flag;
    unsigned char *image1Dc_in, *image_trun,*image1Dc_spine;
    unsigned short *label;
    V3DLONG sz_img[4],sz[4];
    NeuronTree neuron;
    NeuronTree *checked_neuron;

    QString input_proj_name,input_image_name,input_label_name,input_swc_name,
            input_csv_name,input_marker_name,baseDir;
    QLineEdit *edit_load;
    QComboBox *view_menu,*markers;
    QPushButton *btn_run;
    QDialog *mydialog;
    QPlainTextEdit *edit_status;
    init_para auto_para;
    bool eswc_flag;
    unsigned char view_code; //==1 by spine, ==2 by seg

    LandmarkList LList_in,LList_adj;
    v3dhandle curwin,main_win;
    vector<GOV> label_group,label_group_copy;
    int x_start,y_start,z_start,x_end,y_end,z_end;

    //by segment
    vector<vector<int> > segment_neuronswc;
    QPlainTextEdit *edit_seg;
    QComboBox *segments;
    QListWidget *list_markers;
    bool seg_edit_flag;
    int prev_seg,prev_idx;

    //for existing proj
    vector<bool> finish_flag;
};

#endif // MANUAL_PROOFREAD_DIALOG_H
