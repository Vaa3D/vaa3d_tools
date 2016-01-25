#ifndef COLOR_RENDER_ESWC_DIALOG_H
#define COLOR_RENDER_ESWC_DIALOG_H

#include <QDialog>
#include <QtGui>
#include <v3d_interface.h>
#include <vector>

using namespace std;

class color_render_ESWC_dialog : public QDialog
{
    Q_OBJECT
public:
    explicit color_render_ESWC_dialog(V3DPluginCallback2 * callback, V3dR_MainWindow* v3dwin, int nid);
    void enterEvent(QEvent *e);

signals:

public slots:
    void update();
    void reset();
    void reject();
    void set_black();
    void set_white();
    void set_magenta();
    void set_span();
    void set_colormap();

protected:
    void checkwindow();

public:
    QComboBox *cb_feature, *cb_colormap;
    QDoubleSpinBox *spin_min, *spin_max, *spin_black, *spin_white, *spin_meg;
    QCheckBox *check_black, *check_white, *check_meg;
    QPushButton *btn_update, *btn_quit, *btn_reset;

private:
    NeuronTree * nt;
    int neuronid, feaNum;
    vector<unsigned char> bk_type;
    V3dR_MainWindow * v3dwin;
    V3DPluginCallback2 * callback;
};

#endif // COLOR_RENDER_ESWC_DIALOG_H
