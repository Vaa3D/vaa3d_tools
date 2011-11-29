/* swc_editor_plugin.h
 * This is plugin is used to editor swc file
 * 2011-07-08 : by Hang Xiao
 */
 
#ifndef __SWC_EDITOR_PLUGIN_H__
#define __SWC_EDITOR_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class SWCEditorPlugin : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const {return QStringList();}
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent)
	{return false;}
};

#endif

