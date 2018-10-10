/* Branch_detection_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2018-5-20 : by Chao Wang
 */
 
#ifndef __BRANCH_DETECTION_PLUGIN_H__
#define __BRANCH_DETECTION_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
using namespace std;

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

int Branch_Point_Detection(V3DPluginCallback2 &callback, QWidget *parent);
int Branch_Point_Detection_single_plane(V3DPluginCallback2 &callback, QWidget *parent);
int Branch_Point_Detection_new(V3DPluginCallback2 &callback, QWidget *parent);
int Thin_branch_points_detection(V3DPluginCallback2&callback,QWidget *parent);
int Thin_branch_points_detection_single(V3DPluginCallback2 &callback, QWidget *parent);
void printHelp();
#endif

