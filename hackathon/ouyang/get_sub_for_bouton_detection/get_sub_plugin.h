/* get_sub_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2018-10-16 : by guochanghao
 */
 
#ifndef __GET_SUB_PLUGIN_H__
#define __GET_SUB_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

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


int get_sub_auto(V3DPluginCallback2 &callback, QWidget *parent);
int get_sub(V3DPluginCallback2 &callback, QWidget *parent);
int get_sub_by_marker_as_LeftUpCorner(V3DPluginCallback2 &callback, QWidget *parent);
int get_sub_by_input_LeftUpCorner(V3DPluginCallback2 &callback, QWidget *parent);
int get_sub_by_show_boundary(V3DPluginCallback2 &callback, QWidget *parent);
int get_sub_terafly(V3DPluginCallback2 &callback,QWidget *parent);
int get_sub_terafly_and_swc(V3DPluginCallback2 &callback,QWidget *parent);
int test_get_sub_and_swc_auto_by_readfiles(V3DPluginCallback2 &callback, QWidget *parent);
int get_sub_terafly_and_swc_by_marker_in_swcfile_as_center(V3DPluginCallback2 &callback,QWidget *parent);
int test_search_swc(V3DPluginCallback2 &callback,QWidget *parent);

#endif

