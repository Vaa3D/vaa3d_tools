/* pruning_swc_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2014-05-02 : by Zhi Zhou
 */
 
#ifndef __pruning_SWC_PLUGIN_H__
#define __pruning_SWC_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
class pruning_swc : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);
    Q_PLUGIN_METADATA(IID"com.janelia.v3d.V3DPluginInterface/2.1")


public:
	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
};

#endif

class MyComboBox : public QComboBox
{
    Q_OBJECT

public:
    V3DPluginCallback2 * m_v3d;
    MyComboBox(V3DPluginCallback2 * ini_v3d) {m_v3d = ini_v3d;}

    void enterEvent(QEnterEvent * event);

public slots:
    void updateList();
};



class controlPanel: public QDialog
{
    Q_OBJECT

public:
    controlPanel(V3DPluginCallback2 &v3d, QWidget *parent);
    ~controlPanel();

public:
    v3dhandle curwin;
    V3dR_MainWindow *surface_win;
    View3DControl *view;

    QList <V3dR_MainWindow *> list_3dviewer;
    v3dhandleList list_triview;
    QGridLayout *gridLayout;
    V3DPluginCallback2 & m_v3d;

    MyComboBox* combo_surface;
    QLabel* label_surface;

private slots:
     void _slot_prune();
     void _slot_save();
     void reject();


};
