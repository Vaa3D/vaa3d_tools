/* auto_identify_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2012-01-01 : by YourName
 */
 
#ifndef __AUTO_IDENTIFY_PLUGIN_H__
#define __AUTO_IDENTIFY_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class AutoIdentifyPlugin : public QObject, public V3DPluginInterface2_1
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

