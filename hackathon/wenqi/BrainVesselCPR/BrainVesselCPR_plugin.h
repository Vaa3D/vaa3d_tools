/* BrainVesselCPR_plugin.h
 * This is a plugin for Brain Vessel CPR in MRA&MRI image
 * 2019-5-14 : by Wenqi Huang
 */

#ifndef __BRAIN_VESSEL_CPR_PLUGIN_H__
#define __BRAIN_VESSEL_CPR_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class BrainVesselCPRPlugin : public QObject, public V3DPluginInterface2_1
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

void startCPR(V3DPluginCallback2 &callback, QWidget *parent);



#endif

