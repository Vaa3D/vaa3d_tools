/* N3Dfix_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2015-05-04 : by Eduardo Conde-Sousa
 */
 
#ifndef __N3DFIX_PLUGIN_H__
#define __N3DFIX_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class N3DfixPlugin : public QObject, public V3DPluginInterface2_1
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

