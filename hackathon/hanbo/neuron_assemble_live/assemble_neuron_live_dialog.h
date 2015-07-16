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
    QHash<V3DLONG, NOI*> nodes;
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

private:
    void creat(QWidget *parent);
    void initNeuron(QList<NeuronTree> &ntList_in);
    LandmarkList * getMarkerList();
    void updateDisplay();
    void update3DView();
    v3dhandle getImageWindow();
    v3dhandle checkImageWindow(); //if window not opened, open it. return the handle
    v3dhandle updateImageWindow(); //
    V3dR_MainWindow * get3DWindow();
    V3dR_MainWindow * check3DWindow();
    V3dR_MainWindow * update3DWindow();
    void updateROIWindow();
    void updateColor();
};

class pair_marker_dialog : public QDialog
{
    Q_OBJECT
public:
    pair_marker_dialog(LandmarkList * mList,QWidget *parent = 0);
    QListWidget * list1, * list2;
    QPushButton * btn_yes, *btn_no;
};

#define NTDIS(a,b) (((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))
#define NTDOT(a,b) ((a).x*(b).x+(a).y*(b).y+(a).z*(b).z)
#define NTNORM(a) (sqrt((a).x*(a).x+(a).y*(a).y+(a).z*(a).z))

void update_marker_info(const LocationSimple& mk, QList<int>& info); //info[0]=point id, info[0]=matching point id
void update_marker_info(const LocationSimple& mk, QList<int>& info, int* color);
bool get_marker_info(const LocationSimple& mk, QList<int>& info);

#endif // ASSEMBEL_NEURON_LIVE_DIALOG_H
