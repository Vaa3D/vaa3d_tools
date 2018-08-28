/* preprocess_plugin.h
 * Preprocess_neuron_for_analysis
 * 2018-7-27 : by PengXie
 */
 
#ifndef __PREPROCESS_PLUGIN_H__
#define __PREPROCESS_PLUGIN_H__

#include "QtGui"
#include "v3d_interface.h"
#include "pre_processing_main.h"
#include "neurite_analysis_main.h"
class neuron_analysis : public QObject, public V3DPluginInterface2_1
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

