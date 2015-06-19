/* multi_channel_swc_plugin.h
 * This plugin will creat swc files based on multiple channel information in the neuron image.
 * 2015-6-18 : by Sumit and Hanbo
 */
 
#ifndef __MULTI_CHANNEL_SWC_PLUGIN_H__
#define __MULTI_CHANNEL_SWC_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class MultiChannelSWC : public QObject, public V3DPluginInterface2_1
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

