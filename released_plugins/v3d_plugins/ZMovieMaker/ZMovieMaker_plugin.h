/* ZMovieMaker_plugin.h
 * This plugin can be used to generate a smooth movie by several points
 * 2013-11-21 : by Zhi Zhou
 */
 
#ifndef __ZMOVIEMAKER_PLUGIN_H__
#define __ZMOVIEMAKER_PLUGIN_H__

#include <QtGui>

#include "v3d_interface.h"

class ZMovieMaker : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
    float getPluginVersion() const {return 0.95f;}

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

class lookPanel: public QDialog
{
    Q_OBJECT

public:
    lookPanel(V3DPluginCallback2 &v3d, QWidget *parent);

public:
    v3dhandle curwin;
    V3dR_MainWindow *surface_win;
    View3DControl *view;

    QList <V3dR_MainWindow *> list_3dviewer;
    v3dhandleList list_triview;

    V3DPluginCallback2 & m_v3d;

    QGridLayout *gridLayout;
    QListWidget *list_anchors;
    QSpinBox* box_SampleRate;

    MyComboBox* combo_surface;
    QLabel* label_surface;

    bool saveAnchorFile(QString filename);

private slots:
    void _slot_record();
    void _slot_preview();
    void _slot_show_item(QListWidgetItem *item);
    void _slot_show();
    void _slot_delete();
    void _slot_upload();
    void _slot_save();
    void _slot_load();
};

bool _saveAnchorFile(QString filename, QStringList ParaLists, bool b_append);


#endif

