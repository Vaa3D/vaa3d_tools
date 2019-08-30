/* radius_compare_plugin.h
 * This is a plugin to compare radius, input two swc and the node with big difference will be highlighted.
 * 2019-4-12 : by ZhangcanDing
 */
 
#ifndef __RADIUS_COMPARE_PLUGIN_H__
#define __RADIUS_COMPARE_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class radius_compare : public QObject, public V3DPluginInterface2_1
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

NeuronTree swc_radius_compare(NeuronTree nt0, NeuronTree nt1);

#endif

