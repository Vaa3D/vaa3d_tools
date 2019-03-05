/* branch_crop_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2018-12-19 : by YourName
 */
 
#ifndef __BRANCH_CROP_PLUGIN_H__
#define __BRANCH_CROP_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>


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
void get_branches(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback);
QList<int> get_branch_points(NeuronTree nt, bool include_root);
vector< vector<int> > get_close_points(NeuronTree nt,vector<int> a);
XYZ offset_XYZ(XYZ input, XYZ offset);
void crop_swc(QString input_swc, QString output_swc, block crop_block);
void get2d_image(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback);
double computeDist2(const NeuronSWC & s1, const NeuronSWC & s2);
LandmarkList get_missing_branches_menu(V3DPluginCallback2 &callback, QWidget *parent, Image4DSimple * p4DImage);
#endif

