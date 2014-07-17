/* eliminate_swc_plugin.h
 * This plugin is used to eliminate small segments in swc file
 * 2013-12-16 : by Zhi Zhou
 */
 
#ifndef __ELIMINATE_SWC_PLUGIN_H__
#define __ELIMINATE_SWC_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class eliminate_swc : public QObject, public V3DPluginInterface2_1
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

