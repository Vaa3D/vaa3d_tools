/* Cross_detection_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2019-5-15 : by Changhao Guo
 */
 
#ifndef __CROSS_DETECTION_PLUGIN_H__
#define __CROSS_DETECTION_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class Cross_detection_Plugin : public QObject, public V3DPluginInterface2_1
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

int check_One_crossPoint_2D(V3DPluginCallback2 &callback, QWidget *parent);
int detect_crossPoints_2D(V3DPluginCallback2 &callback, QWidget *parent);
int detect_crossPoints_2_5D(V3DPluginCallback2 &callback, QWidget *parent);

bool detect_crossPoints_2D(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent);
bool detect_crossPoints_3D(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent);
int show_find_radius(V3DPluginCallback2 &callback, QWidget *parent);
int test(V3DPluginCallback2 &callback, QWidget *parent);


#endif

