/* plugin_watershed.h
 * watershed plugin based on Fuhui Long's watershed code
 * by Hanchuan Peng, 2011-02-21
 */


#ifndef __PLUGIN_WATERSHED_VS_H__
#define __PLUGIN_WATERSHED_VS_H__

#include <QtGui>

#include "v3d_interface.h"

#include "label_object_dialog.h"

class WatershedVSPlugin : public QObject, public V3DPluginInterface2_1
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2_1);
	
public:
	float getPluginVersion() const {return 0.9f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);
	
	QStringList funclist() const {return QStringList();}
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent)
	{return false;}
	
	
};


#endif

