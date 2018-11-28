/* flythrough_plugin.h
 * a plugin to create a fly-through virtual reality
 * 11/20/2018 : by Yang Yu
 */

#ifndef __NEURONRECON_PLUGIN_H__
#define __NEURONRECON_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class FlyThroughPlugin : public QObject, public V3DPluginInterface2_1
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2_1);

public:
    float getPluginVersion() const {return 1.01f;}

    QStringList menulist() const;
    void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

    QStringList funclist() const ;
    bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
};

class FlyThroughDialog : public QDialog
{
    Q_OBJECT

public:
    FlyThroughDialog(V3DPluginCallback &callback, QWidget *parentWidget);
    ~FlyThroughDialog();

public slots:
    void update();
    void get_img_name();
    void get_swc_name();

public:
    QString fn_img; // path to finest resolution folder of image in terafly format
    QString fn_swc; // path to swc file

    V3DLONG bsx, bsy, bsz;

    QLabel* label_img;
    QLineEdit *edit_img;
    QPushButton *pb_browse_img;

    QLabel* label_swc;
    QLineEdit *edit_swc;
    QPushButton *pb_browse_swc;

    QLabel *label_size;
    QLabel *label_size_x;
    QLabel *label_size_y;
    QLabel *label_size_z;

    QSpinBox* v_size_x;
    QSpinBox* v_size_y;
    QSpinBox* v_size_z;

    float speed; // fly through
    float vx, vy, vz; // directional vector (0, 0, 1) by default

    QPushButton* ok;
    QPushButton* cancel;
};

#endif

