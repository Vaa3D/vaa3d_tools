#ifndef MANUAL_CORRECT_DIALOG_H
#define MANUAL_CORRECT_DIALOG_H

//#include "detect_fun.h"
#include "v3d_interface.h"
#include "spine_detector_dialog.h"
#include "spine_fun.h"
#include "read_raw_file.h"
using namespace std;

//inline bool sortfunc_x_decend(simple_neuronswc *a,simple_neuronswc *b){return (a->x)>(b->x);}
//inline bool sortfunc_y_decend(simple_neuronswc *a,simple_neuronswc *b){return (a->y)>(b->y);}
//inline bool sortfunc_z_decend(simple_neuronswc *a,simple_neuronswc *b){return (a->z)>(b->z);}

class manual_correct_dialog:public QDialog
{
    Q_OBJECT
public:
    explicit manual_correct_dialog(V3DPluginCallback2 *cb,int proofread_code);
    //code=0 view by spine //code=1 view by segment
    manual_correct_dialog(V3DPluginCallback2 *cb);

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
    int save(QString window_name);
    void GetColorRGB(int* rgb, int idx);
    void write_spine_profile(QString filename);
    void write_svm_file(QString filename, vector<int> keep);
private:
    void initDlg();
    bool check_button();
    vector<vector<int> > build_parent_LUT(NeuronTree neuron_tmp);
    int calc_nearest_node(float center_x,float center_y,float center_z);
    vector<vector<int> > neurontree_divide_swc(NeuronTree neuron_tmp);
    void set_visualize_image_marker(vector<int> one_seg, int seg_id);
    float calc_between_dis(int node1_id,int node2_id);

    void standing_segment_dialog();
    void check_window_seg();
    int save_edit_for_seg_view();
    bool save_seg_edit_for_seg_view();
    void reset_edit_for_seg_view(bool visual_flag, int mid);
    void reset_segment();
    void check_neuron_tree();

public slots:
    bool loadImage();
    bool load_swc();
    bool csv_out();
    void run_view_by_spine();
    void run_view_by_seg(); //invoke proofreading by segments

    void accept_marker();
    void delete_marker();
    void skip_marker();
    void marker_roi();
    void dilate();
    void erode();
    void reset_edit();
    bool finish_proof_dialog();

    void segment_change();
    void finish_seg_clicked();
    void reset_segment_clicked();
    void skip_segment_clicked();
    void marker_in_one_seg();
    void reject_marker_for_seg_view();
    void accept_marker_for_seg_view();
    void dilate_for_seg_view();
    void erode_for_seg_view();
    void reset_marker_clicked_for_seg_view();
    bool finish_proof_dialog_seg_view();


    //not used
    bool maybe_save();
    void load_marker();
    void check_local_3d_window();


//for big image handlinng
private:
    vector<vector<int> > neurontree_divide_big_img_eswc();
    vector<V3DLONG> image_seg_plan(vector<int> seg,NeuronTree neuron_tmp);
    void create_big_image();
    NeuronTree prep_seg_neurontree(vector<V3DLONG> coord);
    bool auto_spine_detect_seg_image(unsigned char *data1d, V3DLONG *sz, NeuronTree nt_seg,
                                     int image_id, V3DLONG x_start, V3DLONG y_start, V3DLONG z_start, QSet<V3DLONG> &spine_map);
    bool check_image_size();

public slots:
    bool get_big_image_name();
    void big_image_pipeline_start();
    //void learning_pipeline_starts();
public:


private:
    V3DPluginCallback2 *callback;
    int view_code;
    v3dhandle curwin,main_win;
    QLineEdit *edit_load, *edit_swc,*edit_csv;
    QPlainTextEdit *edit_status;
    QString image_load_path,swc_load_path,csv_load_path;
    QDialog *mydialog;
    QComboBox *markers,*channel_menu;
    QSpinBox *spin_max_pixel, *spin_min_pixel,*spin_max_dis,*spin_bg_thr,
             *spin_width_thr;
    QPushButton *btn_load,*btn_swc,*btn_csv,*btn_run;
    //QCheckBox* small_remover;
    V3DLONG sz_img[4],sz[4],sz_seg[4];
    unsigned short *label;
    unsigned char *image1Dc_in,*image_trun, *image1Dc_spine, *image_seg;
    int intype;
    NeuronTree neuron,checked_neuron;
    LandmarkList LList_in,LList_adj,LList_seg;
    QString folder_output;
    int x_start,y_start,z_start,sel_channel,x_end,y_end,z_end;
    vector<GOV> label_group,label_group_copy;
    bool edit_flag,eswc_flag;
    parameters all_para;
    int rgb[3];

    int x_min,y_min,z_min,x_max,y_max,z_max,prev_idx,prev_seg;
    vector<vector<int> > segment_neuronswc;
    QPlainTextEdit *edit_seg;
    QComboBox *segments;
    QListWidget *list_markers;
    bool seg_edit_flag;
    QString fname;
    QSet<V3DLONG> spine_map;

};

#endif // MANUAL_CORRECT_DIALOG_H
