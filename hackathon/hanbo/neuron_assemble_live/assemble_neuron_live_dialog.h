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
    
    QPushButton * btn_link, *btn_loop, *btn_manuallink, *btn_connect,
        *btn_syncmarker, *btn_break, * btn_save, * btn_quit;
    QTabWidget * tab;
    QListWidget * list_marker, * list_link;
    QComboBox * cb_loop;

signals:
    
public slots:

private:
    void creat(QWidget *parent);
    void initNeuron(QList<NeuronTree> &ntList_in);
    void updateDisplay();
    void updateImageWindow();
    void update3DWindow();
    void updateROIWindow();
};

#endif // ASSEMBEL_NEURON_LIVE_DIALOG_H
