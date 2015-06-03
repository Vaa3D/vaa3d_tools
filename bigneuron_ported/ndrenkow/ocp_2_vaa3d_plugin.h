/* ocp_2_vaa3d_plugin.h
 * Plugin to import OCP data to Vaa3D
 * 2015-6-1 : by JHU-APL
 */
 
#ifndef __OCP_2_VAA3D_PLUGIN_H__
#define __OCP_2_VAA3D_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class OCP2Vaa3D : public QObject, public V3DPluginInterface2_1
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
    void import_from_ocp(V3DPluginCallback2 &callback, QWidget *parent);
};

#endif

