/* axon_tracing_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2016-9-29 : by Zhi Zhou
 */
 
#ifndef __AXON_TRACING_PLUGIN_H__
#define __AXON_TRACING_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
#include "boost/graph/prim_minimum_spanning_tree.hpp"
#include "boost/config.hpp"
#include "boost/graph/adjacency_list.hpp"

using namespace boost;

typedef std::pair<int, int> E;

enum edge_lastvoted_t { edge_lastvoted };

namespace boost {
    BOOST_INSTALL_PROPERTY(edge, lastvoted);
}

typedef property < edge_weight_t, double >  WeightNew;
typedef property < edge_lastvoted_t, int, WeightNew> LastVoted;

typedef adjacency_list < vecS, vecS, undirectedS, no_property, LastVoted > UndirectedGraph;
typedef UndirectedGraph::edge_descriptor EdgeNew;
typedef std::pair<EdgeNew, bool> EdgeQuery;



class axon_tracing : public QObject, public V3DPluginInterface2_1
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

