/* klb_plugin.h
 * To support read and write keller-lab-block-filetype.
 * 2015-6-2 : by Yang Yu (yuy@janelia.hhmi.org)
 */
 
#ifndef __KLB_PLUGIN_H__
#define __KLB_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class KLBPlugin : public QObject, public V3DPluginInterface2_1
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

