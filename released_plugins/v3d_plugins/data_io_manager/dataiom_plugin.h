/* neuron_toolbox_plugin.h
 * This is a super plugin that gather all sub-plugins related to neuron structure processing
 * 2012-04-06 : by Yinan Wan
 */
 
#ifndef __DATAIOM_PLUGIN_H__
#define __DATAIOM_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class DataIOMPlugin : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
    float getPluginVersion() const {return 0.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
};

#endif

