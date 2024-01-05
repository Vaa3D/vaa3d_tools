/* morphoQC_post_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2023-10-16 : by SJ
 */
 
#ifndef __MORPHOQC_POST_PLUGIN_H__
#define __MORPHOQC_POST_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
#include <morphoqc_func.h>
#include <soma_confirmation.h>
#include <topo_validation.h>
#include <type_check.h>

class TestPlugin : public QObject, public V3DPluginInterface2_1
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

