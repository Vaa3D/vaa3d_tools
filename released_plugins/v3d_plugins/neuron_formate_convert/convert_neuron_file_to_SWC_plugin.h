/* convert_neuron_file_to_SWC_plugin.h
 * The plugin will convert neuron file to swc format. It is based on the code of L-Measure, Sridevi Polavaram. Imported by Hanbo Chen.
 * 2015-4-27 : by Hanbo Chen
 */
 
#ifndef __CONVERT_NEURON_FILE_TO_SWC_PLUGIN_H__
#define __CONVERT_NEURON_FILE_TO_SWC_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class neuronConverter : public QObject, public V3DPluginInterface2_1
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

bool doASC2SWC(QString fname_open, QString fname_save);
void printHelp();
#endif

