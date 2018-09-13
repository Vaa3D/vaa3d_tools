#ifndef NEURONSTRUCTEXPLORER_H
#define NEURONSTRUCTEXPLORER_H

#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <string>

#include <boost/config.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>
#include <boost/graph/adjacency_list.hpp>

#include <qlist.h>
#include <qstring.h>
#include <qstringlist.h>

#include "basic_surf_objs.h"
#include "v_neuronswc.h"
#include "NeuronStructUtilities.h"

using namespace std;

enum edge_lastvoted_t { edge_lastvoted };
namespace boost 
{
	BOOST_INSTALL_PROPERTY(edge, lastvoted);
}

typedef boost::property<boost::edge_weight_t, double> weights;
typedef boost::property<edge_lastvoted_t, int, weights> lastVoted;
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, boost::no_property, lastVoted> undirectedGraph;

#ifndef PI
#define PI 3.1415926
#endif

struct topoCharacter
{
	topoCharacter(NeuronSWC centerNode) : topoCenter(centerNode) {};
	NeuronSWC topoCenter;
	NeuronSWC topoCenterPa;
	vector<NeuronSWC> topoCenterImmedChildren;
	double childAngle;
	map<int, double> immedChildrenLengths;
	map<int, double> subsequentChildrenAngles;
};

struct segUnit
{
	QList<NeuronSWC> nodes;
	map<int, size_t> seg_nodeLocMap;
	map<int, vector<size_t> > seg_childLocMap;
	vector<topoCharacter> topoCenters;
};

struct profiledTree
{
	profiledTree(const NeuronTree& inputTree, bool removeRdn = false);

	NeuronTree tree;
	NeuronTree cleanedUpTree;

	QList<NeuronSWC> duRemovedNodeList;
	map<int, size_t> node2LocMap;
	map<int, vector<size_t> > node2childLocMap;

	vector<segUnit> segs;
};

class NeuronStructExplorer
{
public:
	/********* Constructors and basic data members *********/
	NeuronStructExplorer() {};
	NeuronStructExplorer(string neuronFileName);

	NeuronTree* singleTreePtr;
	NeuronTree singleTree;
	NeuronTree processedTree;
	vector<NeuronTree>* treePtrsVector;
	QString neuronFileName;

	map<string, profiledTree> treeDataBase;
	void treeEntry(const NeuronTree& inputTree, string treeName);
	/*******************************************************/

	/***************** Neuron Struct Connecting Functions *****************/
	vector<segUnit> segs;
	static vector<segUnit> findSegs(const QList<NeuronSWC>& inputNodeList, map<int, vector<size_t> >& node2childLocMap);
	NeuronTree SWC2MSTtree(NeuronTree const& inputTreePtr);
	static inline NeuronTree MSTtreeCut(NeuronTree& inputTree, double distThre = 10);
	static NeuronTree MSTbranchBreak(const NeuronTree& inputTree);
	vector<segUnit> MSTtreeTrim(vector<segUnit>& inputSegUnits); 

private:

	/**********************************************************************/

public:
	/********* Pixel-based deep neural network result refining/cleaning *********/
	unordered_map<string, unordered_map<int, float>> zProfileMap;
	void detectedPixelStackZProfile(NeuronTree* inputTreePtr, NeuronTree* outputTreePtr);
	/****************************************************************************/

	/********* Distance-based SWC analysis *********/
	vector<vector<float>> FPsList;
	vector<vector<float>> FNsList;
	void falsePositiveList(NeuronTree* detectedTreePtr, NeuronTree* manualTreePtr, float distThreshold = 20);
	void falseNegativeList(NeuronTree* detectedTreePtr, NeuronTree* manualTreePtr, float distThreshold = 20);
	void detectedDist(NeuronTree* inputTreePtr1, NeuronTree* inputTreePtr2);

	map<int, long int> nodeDistCDF;
	map<int, long int> nodeDistPDF;
	void shortestDistCDF(NeuronTree* inputTreePtr1, NeuronTree* inputTreePtr2, int upperBound, int binNum = 500);
	/*********************************/

	/********* Segment <-> Image assessment functionalities *********/
	V_NeuronSWC_list segmentList;
	void segmentDecompose(NeuronTree* inputTreePtr);
	/**************************************************************/
};

inline NeuronTree NeuronStructExplorer::MSTtreeCut(NeuronTree& inputTree, double distThre)
{
	NeuronTree outputTree;
	for (QList<NeuronSWC>::iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it)
	{
		if (it->parent != -1)
		{
			double x1 = it->x;
			double y1 = it->y;
			double z1 = it->z;
			size_t paID = it->parent;
			double x2 = inputTree.listNeuron.at(paID - 1).x;
			double y2 = inputTree.listNeuron.at(paID - 1).y;
			double z2 = inputTree.listNeuron.at(paID - 1).z;
			double dist = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) + zRATIO * zRATIO * (z1 - z2) * (z1 - z2));
			if (dist > distThre)
			{
				outputTree.listNeuron.push_back(*it);
				(outputTree.listNeuron.end() - 1)->parent = -1;
			}
			else outputTree.listNeuron.push_back(*it);
		}
	}

	return outputTree;
}

#endif