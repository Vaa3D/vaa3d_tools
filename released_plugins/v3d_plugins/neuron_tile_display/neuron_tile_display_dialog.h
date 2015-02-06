#ifndef NEURON_TILE_DISPLAY_DIALOG_H
#define NEURON_TILE_DISPLAY_DIALOG_H
#include <QtGui>
#include <v3d_interface.h>
#include <vector>

using namespace std;

class NeuronTileDisplayDialog : public QDialog
{
    Q_OBJECT

public:
    NeuronTileDisplayDialog(V3DPluginCallback2 * callback, V3dR_MainWindow* v3dwin);
    void enterEvent(QEvent *e);

public slots:
    void tile();
    void tile(double dis);
    void slot_reset();
    void reject();

protected:
    void reset();
    void checkwindow();

public:
//    QCheckBox *check_boundbox;
//    QcheckBox *check_hide;
    QDoubleSpinBox *spin_x;
    QDoubleSpinBox *spin_y;
    QDoubleSpinBox *spin_z;
    QPushButton *btn_quit;
    QPushButton *btn_tile;
    QPushButton *btn_reset;

private:
    QList<NeuronTree> * ntList;
    double cur_x, cur_y, cur_z;
    int cur_num;
    V3dR_MainWindow * v3dwin;
    V3DPluginCallback2 * callback;
};


class NeuronXYTileDialog : public QDialog
{
    Q_OBJECT

public:
    NeuronXYTileDialog(V3DPluginCallback2 * callback, V3dR_MainWindow* v3dwin);
    void enterEvent(QEvent *e);

public slots:
    void tile();
    void slot_reset();
    void reject();

protected:
    void reset();
    void checkwindow();

public:
//    QCheckBox *check_boundbox;
//    QcheckBox *check_hide;
    QDoubleSpinBox *spin_x;
    QDoubleSpinBox *spin_y;
    QDoubleSpinBox *spin_ratial;
    QPushButton *btn_quit;
    QPushButton *btn_tile;
    QPushButton *btn_reset;

private:
    QList<NeuronTree> * ntList;
    vector<double> cur_x, cur_y;
    int cur_num;
    V3dR_MainWindow * v3dwin;
    V3DPluginCallback2 * callback;
};
#endif // NEURON_TILE_DISPLAY_DIALOG_H
