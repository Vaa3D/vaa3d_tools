/*****************************************************************************
 * swc2mask_plugin.h,  Jan 20, 2012 : by Hang Xiao
 *
 * ****************************************************************************/
 
#ifndef __V3D_CONVERT_PLUGIN_H__
#define __V3D_CONVERT_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
#include "basic_parser.h"
#include "advanced_parser.h"

extern AdvancedParser parser;

class Cls_swc2maskPlugin : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	Cls_swc2maskPlugin();
	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);

};

#endif

