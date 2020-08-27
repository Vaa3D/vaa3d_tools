/* adaEnhancement_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2020-8-14 : by ZX
 */
 
#ifndef __ADAENHANCEMENT_PLUGIN_H__
#define __ADAENHANCEMENT_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

#define zx_dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))

class adaEnhancementPlugin : public QObject, public V3DPluginInterface2_1
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

#endif

