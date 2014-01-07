/* GVFplugin_plugin.h
 * Implementation of Gradient Vector Flow cell body segmentation
 * 2014.01.06 : by BRL
 */
 
#ifndef __GVFPLUGIN_PLUGIN_H__
#define __GVFPLUGIN_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class GVFplugin : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
    float getPluginVersion() const {return 0.9f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
};

#endif

