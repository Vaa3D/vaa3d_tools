/* UtilityArbor_plugin.h
 * Generation of the utility arbors
 * 2020-9-24 : by Shengdian
 */
 
#ifndef __UTILITYARBOR_PLUGIN_H__
#define __UTILITYARBOR_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
#include "basic_surf_objs.h"
#include "Utility_function.h"

class UtilityArbor : public QObject, public V3DPluginInterface2_1
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
void printHelp();
#endif

