/* MorphoHub_DBMS_plugin.h
 * This is data management Plugin for neuron reconstruction management.
 * 2019-9-2 : by Shengdian
 */
 
#ifndef __MORPHOHUB_DBMS_PLUGIN_H__
#define __MORPHOHUB_DBMS_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class MorphoHub_DBMS : public QObject, public V3DPluginInterface2_1
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

