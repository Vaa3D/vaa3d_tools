/* Branch_Crop_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2018-12-20 : by YourName
 */
 
#ifndef __BRANCH_CROP_PLUGIN_H__
#define __BRANCH_CROP_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"
struct block{
    QString name;
    XYZ small;
    XYZ large;
};
class branch_crop : public QObject, public V3DPluginInterface2_1
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
vector<int> get_branch_points(NeuronTree nt, bool include_root);
vector< vector<int> > get_close_points(NeuronTree nt,vector<int> a);
XYZ offset_XYZ(XYZ input, XYZ offset);
block offset_block(block input_block, XYZ offset);
void crop_swc(QString input_swc, QString output_swc, block crop_block);
void crop_img(QString image, block crop_block, QString outputdir_img, V3DPluginCallback2 & callback, QString output_format,QString input_swc,int tipnum,XYZ tip);
void get2d_image(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback);
#endif
