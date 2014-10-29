/* neuron_tile_display_plugin.h
 * This Plugin will tile neuron to display
 * 2014-10-28 : by Hanbo Chen
 */
 
#ifndef __NEURON_TILE_DISPLAY_PLUGIN_H__
#define __NEURON_TILE_DISPLAY_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class neuron_tile_display : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);

    void dotile(V3DPluginCallback2 &callback, QWidget *parent);
};

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

#endif

