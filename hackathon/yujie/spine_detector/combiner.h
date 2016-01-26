#ifndef COMBINER_H
#define COMBINER_H

#include <QDialog>
#include <QtGui>
#include "v3d_interface.h"
#include <QVector>

struct spineunit
{
    int id;
    int volume;
    int max_dis;
    int min_dis;
    int center_dis;
    float head_width;
    int nearest_node;
};

struct isunit
{
    int id;
    int volume;
    bool on_dendrite;
    int nearest_node;
};

struct node_info
{
    QVector<spineunit> spine_lut;
    QVector<isunit> is_lut;
};

class combiner : public QDialog
{
    Q_OBJECT


public:
    explicit combiner(V3DPluginCallback2 *cb);
    void create();

private:
    bool check_button();
    bool load_swc();
    bool load_spine_csv(QString filename);
    bool load_is_csv(QString filename);
    bool eswc_generate();

private:
    QLineEdit *edit_spine, *edit_swc,*edit_is;
    QPushButton *btn_swc,*btn_spine,*btn_is,*btn_run;
    //QCheckBox *spine_check,*is_check;
    QString spine_csv_name,is_csv_name,input_swc_name;
    QStringList spine_csv_name_list,is_csv_name_list;
    NeuronTree neuron;
    //QList<spineunit> spineGroup;
    //QList<isunit> isGroup;
    QMap<int,node_info> nodegroup_map;
    QDialog *load_spine_dialog,*load_is_dialog;
    QListWidget *load_spine_combo,*load_is_combo;
    QGridLayout *spine_layout,*is_layout;

public slots:
    bool get_spine_csv_name();
    bool get_is_csv_name();
    bool get_swc_name();
    void run();
    void load_spine_panel();
    void load_is_panel();
    void spine_rm_file();
    void is_rm_file();
    bool spine_files_check();
    bool is_files_check();
    void spine_dialog_ok_clicked();
    void is_dialog_ok_clicked();
};

#endif // COMBINER_H
