#ifndef TREEGROWER_H
#define TREEGROWER_H

#include <boost/config.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>
#include <boost/graph/adjacency_list.hpp>

#include "integratedDataTypes.h"
#include "NeuronGeoGrapher.h"
#include "ImgAnalyzer.h"

enum edge_lastvoted_t { edge_lastvoted };
namespace boost
{
	BOOST_INSTALL_PROPERTY(edge, lastvoted);
}

typedef boost::property<boost::edge_weight_t, double> weights;
typedef boost::property<edge_lastvoted_t, int, weights> lastVoted;
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, boost::no_property, lastVoted> undirectedGraph;

class TreeGrower
{
public:
	static boost::container::flat_map<double, NeuronTree> radiusShellNeuronTreeMap(
		const boost::container::flat_map<double, boost::container::flat_set<int>>& inputRadiusMap, const vector<polarNeuronSWC>& inputPolarNodeList);

	static boost::container::flat_map<double, vector<connectedComponent>> shell2radiusConnMap(const boost::container::flat_map<double, NeuronTree> inputRadius2NeuronTreeMap);


	NeuronTree SWC2MSTtree_boost(const NeuronTree& inputTree);




};

#endif