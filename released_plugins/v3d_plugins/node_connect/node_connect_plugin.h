/* node_connect_plugin.h
 * This is a plugin that takes two marker point inputs, and connects the two swc nodes corresponding to the same location.
 * 2014-08-08 : by SurobhiGanguly
 */
 
#ifndef __NODE_CONNECT_PLUGIN_H__
#define __NODE_CONNECT_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class TestPlugin : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2_1)

public:
	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
};

class MyComboBox : public QComboBox
{
    Q_OBJECT

public:
    V3DPluginCallback2 * m_v3d;
    MyComboBox(V3DPluginCallback2 * ini_v3d) {m_v3d = ini_v3d;}

    void enterEvent(QEvent * event);

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

    V3DPluginCallback2 & m_v3d;

    QGridLayout *gridLayout;

    MyComboBox* combo_surface;
    QLabel* status_load;
    QLabel* status_connect;


private slots:
    void _slot_load();
    void _slot_connect();
    //void _slot_typeset_marker();
    //void _slot_refresh();


    //void _slot_save();

};

#endif

