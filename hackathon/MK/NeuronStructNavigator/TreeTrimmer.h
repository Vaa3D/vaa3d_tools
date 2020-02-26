#ifndef TREETRIMMER_H
#define TREETRIMMER_H

#include <memory>

#include "integratedDataTypes.h"
#include "NeuronGeoGrapher.h"
#include "NeuronStructExplorer.h"

class TreeTrimmer
{
public:
	TreeTrimmer() { this->sharedExplorerPtr = nullptr; }
	TreeTrimmer(NeuronStructExplorer* baseExplorerPtr) { this->sharedExplorerPtr = make_shared<NeuronStructExplorer*>(baseExplorerPtr); }

	shared_ptr<NeuronStructExplorer*> sharedExplorerPtr;


	map<int, profiledTree> trimmedTree_segEndClusterBased(const profiledTree& inputProfiledTree, const map<int, ImageMarker>& axonGrowingPoints);


	/************************* Minor Tree Trimming / Refining *************************/
	// Removes short spikes on segments. The skipe length criterion is predefined by users in node count measure.
	static profiledTree spikeRemoval(const profiledTree& inputProfiledTree, int spikeNodeNum = 3);
	static profiledTree itered_spikeRemoval(profiledTree& inputProfiledTree, int spikeNodeNum = 3);

	static profiledTree removeHookingHeadTail(const profiledTree& inputProiledTree, float radAngleThre);
	static profiledTree itered_removeHookingHeadTail(profiledTree& inputProfiledTree, float radAngleThre);

	static profiledTree segSharpAngleSmooth_lengthDistRatio(const profiledTree& inputProfiledTree, const double ratio);
	static profiledTree itered_segSharpAngleSmooth_lengthDistRatio(profiledTree& inputProfiledTree, double ratio);

	static profiledTree segSharpAngleSmooth_distThre_3nodes(const profiledTree& inputProfiledTree, const double distThre = 5);

	// Breaks all branches in [inputProfiledTree].
	// Note, if [spikeRemove] == true, any short branches less than [spikeThre] in length will be removed as undesired spikes in stead of being recognized as branches.
	static NeuronTree branchBreak(const profiledTree& inputProfiledTree, double spikeThre = 10, bool spikeRemove = true);

	// Breaks any node-node length that is greater than [distThre].
	static inline NeuronTree treeCut(NeuronTree& inputTree, double distThre = 10);
	/**********************************************************************************/
};

inline NeuronTree TreeTrimmer::treeCut(NeuronTree& inputTree, double distThre)
{
	NeuronTree outputTree;
	for (QList<NeuronSWC>::iterator it = inputTree.listNeuron.begin(); it != inputTree.listNeuron.end(); ++it)
	{
		if (it->parent != -1)
		{
			double x1 = it->x;
			double y1 = it->y;
			double z1 = it->z;
			int paID = it->parent;
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