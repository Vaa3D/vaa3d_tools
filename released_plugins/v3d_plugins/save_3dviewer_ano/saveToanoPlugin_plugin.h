/* saveToanoPlugin_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2013-01-13 : by Zhi Zhou
 */
 
#ifndef __SAVETOANOPLUGIN_PLUGIN_H__
#define __SAVETOANOPLUGIN_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class saveToanoPlugin : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
    float getPluginVersion() const {return 2.01f;}

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
    static controlPanel *panel;
    QLineEdit *m_pLineEdit_filename;


    QList <V3dR_MainWindow *> list_3dviewer;

    V3DPluginCallback2 & m_v3d;

    QGridLayout *gridLayout;

    MyComboBox* combo_surface;
    QLabel* label_surface;

    bool saveAnchorFile(QString filename);

private slots:
    void _slot_saveano();
    void _slot_saveano_content();

};


#endif

