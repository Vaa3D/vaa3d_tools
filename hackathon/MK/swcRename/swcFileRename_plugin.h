/* swcFileRename_plugin.h
 * This is a plugin which intends to fix messy swc files.
 * 2020-9-14 : by MK
 */
 
#ifndef __SWCFILERENAME_PLUGIN_H__
#define __SWCFILERENAME_PLUGIN_H__

#include <QtGui>

#include <v3d_interface.h>

#include "swcRenameDlg.h"

class swcFileRename : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);

private:
	SWC_renameDlg* dlgPtr;
};

#endif

