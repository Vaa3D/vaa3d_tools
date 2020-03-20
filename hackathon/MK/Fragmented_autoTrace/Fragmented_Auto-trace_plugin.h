/* Fragmented_Auto-trace_plugin.h
 * This plugin can be called from v3dr_glwidget with hot key Alt+F. Thus it can work like a standard feature in Vaa3D.
 * 2018-12-3 : by MK
 */
 
#ifndef __FRAGMENTED_AUTO_TRACE_PLUGIN_H__
#define __FRAGMENTED_AUTO_TRACE_PLUGIN_H__

#include <QtGui>

#include <v3d_interface.h>
#include "INeuronAssembler.h"

#include "FragTraceControlPanel.h"

class FragmentedAutoTracePlugin : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1)

public:
	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);

private:
	FragTraceControlPanel* UIinstancePtr;
};

#endif

