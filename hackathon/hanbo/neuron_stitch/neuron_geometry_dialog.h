/****************************************************************************
**
** neuron_geometry_dialog.h
** referance: surfaceobj_geometry_dialog.h
** by Hanbo Chen 2014.10.8
**
****************************************************************************/

#ifndef NEURON_GEOMETRY_DIALOG_H
#define NEURON_GEOMETRY_DIALOG_H

#include <QDialog>

#include "ui_neuron_geometry_dial.h"
#include "neuron_match_clique.h"
#include <v3d_interface.h>

class NeuronGeometryDialog: public QDialog, private Ui_NeuronGeometryDialog
{
    Q_OBJECT

public:
    NeuronGeometryDialog(V3DPluginCallback2 * callback, V3dR_MainWindow* v3dwin);
    void enterEvent(QEvent * event);

public:
    //handles
    static NeuronGeometryDialog *dialog;
    V3dR_MainWindow * v3dwin;
    V3DPluginCallback2 * callback;
    View3DControl * v3dcontrol;

protected:
    bool run_status;

    //changes
    double cur_shift_x[2], cur_shift_y[2], cur_shift_z[2],
           cur_scale_x[2], cur_scale_y[2], cur_scale_z[2],
           cur_gscale_x[2], cur_gscale_y[2], cur_gscale_z[2],
           cur_scale_r[2],
           cur_rotate_x[2], cur_rotate_y[2], cur_rotate_z[2];
    bool cur_flip_x[2], cur_flip_y[2], cur_flip_z[2];

    double cur_maxx[2],cur_maxy[2],cur_maxz[2],cur_minx[2],cur_miny[2],cur_minz[2],cur_mmx[2],cur_mmy[2],cur_mmz[2];

    double cur_tmat[2][16]; //storage of transformation matrix

    int cur_marker_num;

    //neurons
    QList<NeuronTree> * ntList;
    QList<NeuronTree *> ntpList;
    QList<NeuronTree> nt_bkList;
    QList<QList<int> > type_bk;
    int ant; // 0 or 1, the id of the neuron tree that is under operation
    int stack_dir; // 0/1/2/3=x/y/z/free

    //Markers
    LandmarkList * mList;

private:
    void resetInternalStates();
    void resetInternalStates(int i);
    void create();
    void updateContent();

public slots:
    void auto_affine();
    void link_marker_neuron_force();
    void link_markers();
    void affine_markers();
    void update_markers();
    void change_ant(int idx);
    void change_stackdir(int dir);
    void update_boundingbox();
    void highlight_matchpoint(double s);//multiple neuron compatible
    void highlight_boundpoint(double s);//multiple neuron compatible
    void highlight_matchpoint_check(int c);//multiple neuron compatible
    void highlight_boundpoint_check(int c);//multiple neuron compatible
    void hide_branch(double s);//only 2 neuron
    void hide_branch_check(int c);//only 2 neuron
    void change_neurontype();
    void reject();
    void accept();
    void reset();
    void save_affine_mat();
    void quickmove();
    void shift_x(double s);
    void shift_y(double s);
    void shift_z(double s);
    void scale_x(double s);
    void scale_y(double s);
    void scale_z(double s);
    void gscale_x(double s);
    void gscale_y(double s);
    void gscale_z(double s);
    void scale_r(double s);
    void rotate_around_x_dial(int v);
    void rotate_around_y_dial(int v);
    void rotate_around_z_dial(int v);
    void rotate_around_x_spin(double s);
    void rotate_around_y_spin(double s);
    void rotate_around_z_spin(double s);
    void refresh_rcent();
    void flip_x(int v);
    void flip_y(int v);
    void flip_z(int v);

signals:
    void xRotationChanged(int);
    void yRotationChanged(int);
    void zRotationChanged(int);
    void xRotationChanged(double);
    void yRotationChanged(double);
    void zRotationChanged(double);

public:
    void highlight_points(bool force);
    void show_branch();//only 2 neuron
    void hide_branch();//only 2 neuron
    void update_markers_delete(); //update markers when there is one deleted
    void link_new_marker_neuron();
    void link_marker_neuron();
};

void highlight_dial(QDial *d);
double NORMALIZE_ROTATION_AngleStep( double  angle );

#endif // NEURON_GEOMETRY_DIALOG_H
