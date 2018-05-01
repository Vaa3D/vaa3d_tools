#ifndef MANUAL_PROOF_IS_H
#define MANUAL_PROOF_IS_H

#include <QtGui>
#include <QDialog>
#include "is_analysis_fun.h"
#include "v3d_interface.h"
#include "vector"

struct init_para_is{
    int spine_bgthr;
    int is_bgthr;
    int spine_channel;
    int is_channel;
    int max_dis;
    int min_voxel;
    int seg_id;
    int marker_id;
    int erodeNB;
    int dilateNB;
    init_para_is(){spine_bgthr=is_bgthr=spine_channel=is_channel=max_dis=min_voxel
                =seg_id=marker_id=erodeNB=dilateNB=-1;}
    init_para_is(int _spine_bgthr,int _is_bgthr,int _spine_channel,
                 int _is_channel,int _max_dis,int _min_voxel,int _seg_id,int _marker_id,
                 int _erodeNB,int _dilateNB)
    {
        spine_bgthr=_spine_bgthr;
        is_bgthr=_is_bgthr;
        spine_channel=_spine_channel;
        is_channel=_is_channel;
        max_dis=_max_dis;
        min_voxel=_min_voxel;
        seg_id=_seg_id;
        marker_id=_marker_id;
        erodeNB=_erodeNB;
        dilateNB=_dilateNB;
    }
};

using namespace std;

QDataStream &operator>>(QDataStream &in, init_para_is &tmp);
QDataStream &operator<<(QDataStream &out, const init_para_is &tmp);
QDataStream &operator<<(QDataStream &out, const GOI &tmp);
QDataStream &operator>>(QDataStream &in, GOI &tmp);
QDataStream &operator<<(QDataStream &out, const LocationSimple &input);
QDataStream &operator>>(QDataStream &in, LocationSimple &output);

class manual_proof_is:public QDialog
{
    Q_OBJECT
public:
    manual_proof_is(V3DPluginCallback2 *cb, bool menu_flag);
    void create_proofread_panel(bool exist_proj);
    void run_interface_with_auto(int ret, NeuronTree auto_neuron, bool eswc_tmp_flag,
        LandmarkList LList, vector<GOI> voxel_group_in, unsigned char *image1Dc, V3DLONG sz_img_in[4],
        int spine_bgthr, int is_bgthr, int max_dis, int min_voxel, int spine_channel, int is_channel,
        QString in_swc_name, QString in_image_name, QString folder_output);
    void create();

private:
    void initDlg();
    bool check_btn();
    void open_trunc_triview(QString trunc_win_name);
    void open_main_triview(QString main_win_name);
    vector<V3DLONG> image_seg_plan(vector<int> seg, NeuronTree *neuron_tmp);
    void set_trunc_image_marker(vector<int> one_seg,int seg_id);
    int csv_generate();
    void backUpProjectData();
    int finish_implement();
    bool loadImage();
    bool loadSWC();
    void check_each_seg();
    int save_edit();

public slots:
    bool get_image_name();
    bool get_swc_name();

    void segment_change();
    void reject_marker();
    void accept_marker();
    void set_on_spine();
    void set_on_dendrite();
    bool save_seg_edit();
    void reset_segment();
    void erode();
    void dilate();
    void reset_marker_clicked();
    void reset_edit(bool visual_flag,int mid);
    void skip_segment_clicked();
    void finish_seg_clicked();
    void reset_segment_clicked();
    void save();
    bool loadProjectData();
    void finish_dialog();
    bool get_proj_name();
    void run_saved_proj();
    void setting_window();
    void marker_in_one_seg();
    void receive_new_para();
    void cancel_new_para();

private:
    V3DPluginCallback2 * callback;
    bool override_img, override_swc;
    v3dhandle curwin,main_win;
    QColor green, red;
    QLineEdit *edit_load,*edit_load_img,*edit_load_swc;
    QPushButton *btn_run;
    QComboBox *segments;
    QDialog *mydialog, *dialog;
    QListWidget *list_markers;
    QPlainTextEdit *edit_seg;
    bool seg_edit_flag,edit_flag,debugF;
    int prev_seg,prev_idx;
    init_para_is auto_para;
    vector<vector<int> > segment_neuronswc;
    vector<bool> finish_flag;
    QVector<GOI> voxel_group,voxel_group_copy;
    LandmarkList LList_in,LList_adj;
    unsigned char *image_trun, *image_is;
    int x_start,y_start,z_start,x_end,y_end,z_end;
    V3DLONG sz_img[4],sz[4];
    NeuronTree neuron;
    NeuronTree *checked_neuron;
    bool eswc_flag;
    QString input_proj_name,input_image_name,input_label_name,input_swc_name,
            input_csv_name,input_marker_name,baseDir,csv_out_name,
            input_image_name2,input_swc_name2;
    QSpinBox *erode_nb, *dilate_nb;

};

#endif // MANUAL_PROOF_IS_H
