/* screenshots_swc_plugin.h
 * This plugin will make screenshots for a series swc files.
 * 2015-02-06 : by Hanbo Chen
 */
 
#ifndef __SCREENSHOTS_SWC_PLUGIN_H__
#define __SCREENSHOTS_SWC_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class screenshots_swc : public QObject, public V3DPluginInterface2_1
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

class screenshots_swc_dialog : public QDialog
{
    Q_OBJECT

public:
    screenshots_swc_dialog(V3DPluginCallback2 * cb, V3dR_MainWindow* v3dwin);

private:
    V3DPluginCallback2* callback;
    V3dR_MainWindow* v3dwin;
    void creat();
    void checkwindow();

public slots:
    bool load();
    bool output();
    void run();

public:
    QPushButton *btn_load, *btn_output, *btn_run;
    QLineEdit *edit_load, *edit_output;

private:
    QString fname_input;
    QString fname_output;
};

QStringList importSWCFileList_addnumbersort(const QString & curFilePath);

#endif

