/* neuron_completeness_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2019-2-25 : by Zhi Zhou
 */
 
#ifndef __NEURON_COMPLETENESS_PLUGIN_H__
#define __NEURON_COMPLETENESS_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
#include "neuron_format_converter.h"
#include <set>

vector<NeuronSWC> loopDetection(V_NeuronSWC_list inputSegList);
void rc_loopPathCheck(size_t inputSegID, vector<size_t> curPathWalk);
vector<V_NeuronSWC_list> showConnectedSegs(const V_NeuronSWC_list& inputSegList);
void rc_findConnectedSegs(V_NeuronSWC_list& inputSegList, set<size_t>& singleTreeSegs, size_t inputSegID, multimap<string, size_t>& segEnd2segIDmap);
set<size_t> segEndRegionCheck(V_NeuronSWC_list& inputSegList, size_t inputSegID);

class TestPlugin : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
    float getPluginVersion() const {return 1.63f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
};

#endif

