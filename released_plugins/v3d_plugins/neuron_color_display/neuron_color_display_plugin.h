/* neuron_color_display_plugin.h
 * This plugin will set the color of the neuron displayed
 * 2015-6-12 : by Hanbo Chen
 */
 
#ifndef __NEURON_COLOR_DISPLAY_PLUGIN_H__
#define __NEURON_COLOR_DISPLAY_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>


class SetNeuronDisplayColor : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);

    void do1clickcolor(V3DPluginCallback2 &callback, QWidget *parent);
};

#endif

