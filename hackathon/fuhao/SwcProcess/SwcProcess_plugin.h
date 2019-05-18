/* SwcProcess_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2019-1-3 : by fuhao
 */
 
#ifndef __SWCPROCESS_PLUGIN_H__
#define __SWCPROCESS_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
#include "scoreinput.h"

class TestPlugin : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
};

//class MyComboBox : public QComboBox
//{
//    Q_OBJECT

//public:
//    V3DPluginCallback2 * m_v3d;
//    MyComboBox(V3DPluginCallback2 * ini_v3d) {m_v3d = ini_v3d;}

//    void enterEvent(QEvent * event);
//    void addFileName(QString swcfilename);
//private:
//    QStringList inputSWcName;
//public slots:
//    void updateList();
//};

//class separateDialog: public QDialog
//{
//    Q_OBJECT

//public:
//    separateDialog(V3DPluginCallback2 &v3d, QWidget *parent);
//    ~separateDialog();

//public:
//    v3dhandle curwin;
//    V3dR_MainWindow *surface_win;
//    View3DControl *view;


//    QList <V3dR_MainWindow *> list_3dviewer;

//    V3DPluginCallback2 & m_v3d;

//    QGridLayout *gridLayout;

//    MyComboBox* combobox_win;

////    QSpinBox *spinbox_steplength;
//    QCheckBox *ReadFromFile;


////private slots:
////    void _slot_run();


//};


#endif

