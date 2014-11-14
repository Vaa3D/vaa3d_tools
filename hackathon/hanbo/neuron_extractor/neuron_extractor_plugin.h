/* neuron_extractor_plugin.h
 * This plugin will extract the neurons selected by markers. The structure connected to each markers will be extracted and saved seperately.
 * 2014-11-2 : by Hanbo Chen
 */
 
#ifndef __NEURON_EXTRACTOR_PLUGIN_H__
#define __NEURON_EXTRACTOR_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class neuron_extractor_by_marker : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);

    void extract(QList<NeuronTree> *ntList, LandmarkList* mList);
    void extract_spine(const NeuronTree& nt, QList<NeuronSWC>& spine, int length);
    void assignComponents(NeuronTree& nt);

};

bool export_list2file(const QList<NeuronSWC>& lN, QString fileSaveName, QString orgSaveName);

#endif

