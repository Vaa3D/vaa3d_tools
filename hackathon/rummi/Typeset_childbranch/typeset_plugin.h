/* typeset_plugin.h
 * Typeset child brances based on node marker
 * 2014-06-26 : by Surobhi Ganguly
 */
 
#ifndef __TYPESET_PLUGIN_H__
#define __TYPESET_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
//#include <QObject>

class TypesetPlugin : public QObject, public V3DPluginInterface2_1
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
    //static controlPanel *panel;


    QList <V3dR_MainWindow *> list_3dviewer;
    v3dhandleList list_triview;

    V3DPluginCallback2 & m_v3d;

    QGridLayout *gridLayout;
    QListWidget *list_markers;
    QSpinBox* box_Typeset;

    MyComboBox* combo_surface;
    QLabel* label_surface;

    //bool saveAnchorFile(QString filename);


private slots:
    void _slot_sort();//V3DPluginCallback2 &_v3d, QWidget *parent);
    void _slot_typeset(V3DPluginCallback2 &v3d, QWidget *parent);
    //void _slot_typeset();
    void _slot_refresh();
    void _slot_delete();
    void _slot_show();
    void _slot_save();





//    void _slot_preview();
//    void _slot_show_item(QListWidgetItem *item);
//    void _slot_upload();
//    void _slot_load();
//    void _slot_up();
//    void _slot_down();

};

//bool _saveAnchorFile(QString filename, QStringList ParaLists, bool b_append);


#endif

