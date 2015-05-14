#ifndef NEURON_TIPSPICKER_DIALOG_H
#define NEURON_TIPSPICKER_DIALOG_H

#include <QDialog>
#include <v3d_interface.h>
#include "neuron_stitch_func.h"



class neuron_tipspicker_dialog : public QDialog
{
    Q_OBJECT

public:
    neuron_tipspicker_dialog(V3DPluginCallback2 * callback, V3dR_MainWindow* v3dwin);

private:
    void creat();
    //void checkwindow();
    void link_new_marker_neuron();
    void relink_marker_neuron();
    void updatemarker();
    void checkStatus();
    void output_markers(QString fname);
    void locate_tip();
    void roi_tip();

signals:
    
public slots:
    void search();
    void spineCheck(int);
    void reset_tip();
    void update_tip();
    void accept_tip();
    void reject_tip();
    void skip_tip();
    void change_tip(int);
    void save();
    
private:
    V3DPluginCallback2 * callback;
    V3dR_MainWindow* v3dwin;
    QList<NeuronTree> *ntList;
    LandmarkList * mList;
    View3DControl * v3dcontrol;
    int curIdx;

public:
    QGridLayout *gridLayout;
    QComboBox *cb_dir,*cb_tips, *cb_side;
    QDoubleSpinBox *spin_zscale, *spin_searchspan, *spin_segthr, *spin_gapthr, *spin_angthr;
    QSpinBox *spin_spineLen, *spin_roisize;
    QDoubleSpinBox *spin_spineAng, *spin_spineRadius;
    QCheckBox *check_spine, *check_syncTriView;
    QGroupBox *group_marker, *group_check;

    QPushButton *btn_search, *btn_reset, *btn_update, *btn_accept, *btn_reject, *btn_skip;
    QPushButton *btn_save, *btn_quit;
};

#endif // NEURON_TIPSPICKER_DIALOG_H
