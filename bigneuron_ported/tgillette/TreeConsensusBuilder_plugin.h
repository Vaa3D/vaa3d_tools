/* TreeConsensusBuilder_plugin.h
 * Given multiple reconstructions of the same neuron, produces a consensus given a confidence threshold (some proportion of input reconstructions voting for each branch).
 * 2015-11-19 : by Todd Gillette
 */
 
#ifndef __TREECONSENSUSBUILDER_PLUGIN_H__
#define __TREECONSENSUSBUILDER_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class TreeConsensusBuilderPlugin : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	float getPluginVersion() const {return 1.0f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
};

#endif

