#ifndef ASSEMBEL_NEURON_LIVE_DIALOG_H
#define ASSEMBEL_NEURON_LIVE_DIALOG_H

#include <QDialog>
#include <QtGui>
#include "v3d_interface.h"
#include <map>

#define WINNAME_ASSEM "assemble_swc_file"

using namespace std;

typedef struct NOI{
    V3DLONG n; //the id of noi, match with the n variable of node
    V3DLONG cid; //fragment component id
    int type; //type information
    float x, y, z;		// point coordinates
    union{
    float r;			// radius
    float radius;
    };
    QList<float> fea_val; //feature values
    QSet<NOI *> conn; //connections from NOI
}NOI;

typedef QHash<V3DLONG, NOI*>::iterator NodeIter;

class assemble_neuron_live_dialog : public QDialog
{
    Q_OBJECT
public:
    assemble_neuron_live_dialog(V3DPluginCallback2 * callback, QList<NeuronTree> &ntList, Image4DSimple * p_img4d,QWidget *parent = 0);

private:
    V3DPluginCallback2 * callback;
    NeuronTree nt;
    V3DLONG prev_root;
    QHash<V3DLONG, NOI*> nodes;
    int coffset;
    V3DLONG noffset;
    Image4DSimple * p_img4d;
    
    QPushButton * btn_link, *btn_loop, *btn_manuallink, *btn_deletelink, *btn_connect,
        *btn_syncmarker, *btn_break, * btn_save, * btn_quit, * btn_colorloop;
    QTabWidget * tab;
    QListWidget * list_marker, * list_link;
    QComboBox * cb_loop, * cb_color;
    QCheckBox * check_loop;

signals:
    
public slots:
    void setColor(int i);
    void syncMarker();
    void pairMarker();
    void delPair();
    void connectPair();
    void searchPair();
    void sortsaveSWC();
    void highlightPair();
    void highlightMarker();

private:
    void creat(QWidget *parent);
    void initNeuron(QList<NeuronTree> &ntList_in);
    bool connectNode(V3DLONG pid1, V3DLONG pid2);
    QSet<QPair<V3DLONG, V3DLONG> > searchConnection(double xscale, double yscale, double zscale,
                                                    double angThr, double disThr, int matchType,
                                                    bool b_minusradius);
    double getNeuronDiameter();

    LandmarkList * getMarkerList();
    void updateDisplay(); //deep refresh everything
    void update3DView(); //light refresh 3D window
    v3dhandle getImageWindow(); //return the handle if window is open, otherwise 0
    v3dhandle checkImageWindow(); //if window not opened, open it. return the handle
    v3dhandle updateImageWindow(); //deep refresh image window
    V3dR_MainWindow * get3DWindow(); //return the handle if window is open, otherwise 0
    V3dR_MainWindow * check3DWindow(); //if window not opened, open it. return the handle
    V3dR_MainWindow * update3DWindow(); //deep refresh 3D window
    void updateROIWindow(); //to-do
    void updateColor(); //update the neuron type, will sync neuron if changed
};

class pair_marker_dialog : public QDialog
{
    Q_OBJECT
public:
    pair_marker_dialog(LandmarkList * mList,QWidget *parent = 0);
    QListWidget * list1, * list2;
    QPushButton * btn_yes, *btn_no;
};

class connect_param_dialog : public QDialog
{
    Q_OBJECT

public:
    connect_param_dialog();
    ~connect_param_dialog();

private:
    void creat();
    void initDlg();

public slots:
    void myconfig();

public:
    QGridLayout *gridLayout;
    QDoubleSpinBox *spin_zscale, *spin_xscale, *spin_yscale, *spin_ang, *spin_dis;
    QPushButton *btn_quit, *btn_run;
    QComboBox *cb_distanceType, *cb_matchType, *cb_conf;
    QTextEdit* text_info;
};

class sort_neuron_dialog : public QDialog
{
    Q_OBJECT
public:
    sort_neuron_dialog(LandmarkList * mList, V3DLONG prev_root, V3DLONG type1_root, QWidget *parent = 0);
    QComboBox * cb_marker;
    QPushButton * btn_yes, *btn_no;
    QRadioButton *radio_marker, *radio_type, *radio_prev;
};

#define NTDIS(a,b) (((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))
#define NTDOT(a,b) ((a).x*(b).x+(a).y*(b).y+(a).z*(b).z)
#define NTNORM(a) (sqrt((a).x*(a).x+(a).y*(a).y+(a).z*(a).z))

void update_marker_info(const LocationSimple& mk, QList<int>& info); //info[0]=point id, info[0]=matching point id
void update_marker_info(const LocationSimple& mk, QList<int>& info, int* color);
void set_marker_color(const LocationSimple& mk, RGB8 color);
bool get_marker_info(const LocationSimple& mk, QList<int>& info);
QList<NeuronSWC> generate_swc_typesort(QHash<V3DLONG, NOI*>& nodes, V3DLONG n_root);
bool export_list2file(const QList<NeuronSWC>& lN, QString fileSaveName);

#endif // ASSEMBEL_NEURON_LIVE_DIALOG_H
