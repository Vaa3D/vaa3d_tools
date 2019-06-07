#include <iostream>

#include "NeuronStructUtilities.h"
#include "TreeGrower.h"

using namespace std;

boost::container::flat_map<double, NeuronTree> TreeGrower::radiusShellNeuronTreeMap(const boost::container::flat_map<double, boost::container::flat_set<int>>& inputRadiusMap, const vector<polarNeuronSWC>& inputPolarNodeList)
{
	boost::container::flat_map<double, NeuronTree> outputRadius2NeuronTreeMap;
	for (boost::container::flat_map<double, boost::container::flat_set<int>>::const_iterator it = inputRadiusMap.begin(); it != inputRadiusMap.end(); ++it)
	{
		NeuronTree currShellTree;
		for (boost::container::flat_set<int>::const_iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
		{
			NeuronSWC newNode = NeuronGeoGrapher::polar2CartesianNode(inputPolarNodeList.at(*it2));
			currShellTree.listNeuron.push_back(newNode);
		}

		outputRadius2NeuronTreeMap.insert(pair<double, NeuronTree>(it->first, currShellTree));
	}

	return outputRadius2NeuronTreeMap;
}

boost::container::flat_map<double, vector<connectedComponent>> TreeGrower::shell2radiusConnMap(const boost::container::flat_map<double, NeuronTree> inputRadius2NeuronTreeMap)
{
	NeuronStructUtil myNeuronStructUtil;
	boost::container::flat_map<double, vector<connectedComponent>> outputShell2radiusConnMap;

	for (boost::container::flat_map<double, NeuronTree>::const_iterator it = inputRadius2NeuronTreeMap.begin(); it != inputRadius2NeuronTreeMap.end(); ++it)
	{
		vector<connectedComponent> currConnCompList = myNeuronStructUtil.swc2signal3DBlobs(it->second);
		outputShell2radiusConnMap.insert(pair<double, vector<connectedComponent>>(it->first, currConnCompList));
	}

	return outputShell2radiusConnMap;
}

NeuronTree TreeGrower::SWC2MSTtree_boost(const NeuronTree& inputTree)
{
	NeuronTree MSTtrees;
	undirectedGraph graph(inputTree.listNeuron.size());
	//cout << "processing nodes: \n -- " << endl;
	for (int i = 0; i < inputTree.listNeuron.size(); ++i)
	{

		float x1, y1, z1;
		x1 = inputTree.listNeuron.at(i).x;
		y1 = inputTree.listNeuron.at(i).y;
		z1 = inputTree.listNeuron.at(i).z;
		for (int j = 0; j < inputTree.listNeuron.size(); ++j)
		{
			float x2, y2, z2;
			x2 = inputTree.listNeuron.at(j).x;
			y2 = inputTree.listNeuron.at(j).y;
			z2 = inputTree.listNeuron.at(j).z;

			double Vedge = sqrt(double(x1 - x2) * double(x1 - x2) + double(y1 - y2) * double(y1 - y2) + zRATIO * zRATIO * double(z1 - z2) * double(z1 - z2));
			pair<undirectedGraph::edge_descriptor, bool> edgeQuery = boost::edge(i, j, graph);
			if (!edgeQuery.second && i != j) boost::add_edge(i, j, lastVoted(i, weights(Vedge)), graph);
		}

		//if (i % 1000 == 0) cout << i << " ";
	}
	//cout << endl;

	vector <boost::graph_traits<undirectedGraph>::vertex_descriptor > p(num_vertices(graph));
	boost::prim_minimum_spanning_tree(graph, &p[0]);
	NeuronTree MSTtree;
	QList<NeuronSWC> listNeuron;
	QHash<int, int>  hashNeuron;
	listNeuron.clear();
	hashNeuron.clear();

	for (size_t ii = 0; ii != p.size(); ++ii)
	{
		int pn;
		if (p[ii] == ii) pn = -1;
		else pn = p[ii] + 1;

		NeuronSWC S;
		S.n = ii + 1;
		S.type = 7;
		S.x = inputTree.listNeuron.at(ii).x;
		S.y = inputTree.listNeuron.at(ii).y;
		S.z = inputTree.listNeuron.at(ii).z;
		S.r = 1;
		S.pn = pn;
		listNeuron.append(S);
		hashNeuron.insert(S.n, listNeuron.size() - 1);
	}
	MSTtree.listNeuron = listNeuron;
	MSTtree.hashNeuron = hashNeuron;

	return MSTtree;
}