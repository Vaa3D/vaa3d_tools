/* branch_crop_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2018-12-19 : by YourName
 */
 
#ifndef __BRANCH_CROP_PLUGIN_H__
#define __BRANCH_CROP_PLUGIN_H__

#include <QtGui>
#include "v3d_interface.h"
//#include <vector>
//#include "v3d_message.h"
//#include "branch_detect.h"
//#include "../../../../vaa3d_tools/hackathon/PengXie/preprocess/sort_swc_redefined.cpp"


using namespace std;

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

//class OpenSWCDialog: public QDialog
//{
//    Q_OBJECT

//public:
//    V3DPluginCallback2 * callback;
//    QListWidget * listWidget;
//    QList<NeuronTree> _3DViewerTreeList;
//    NeuronTree  nt;
//    QString file_name;

//    OpenSWCDialog(QWidget * parent, V3DPluginCallback2 * callback);
//    void getAllNeuronTreeNames(V3DPluginCallback2 * callback);

//public slots:
//    bool run();
//    bool setTree(const QString & file);
//};



#endif

