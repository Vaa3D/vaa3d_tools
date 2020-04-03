#ifndef NEURONSTRUCTNAVIGATINGTESTER_H
#define NEURONSTRUCTNAVIGATINGTESTER_H

#include "NeuronGeoGrapher.h"
#include "NeuronStructExplorer.h"
#include "TreeGrower.h"
#include "TreeTrimmer.h"
#include "NeuronStructUtilities.h"

namespace NeuronStructNavigator
{
	class Tester
	{
	public:
		static Tester* testerInstance;
		static Tester* instance(const NeuronStructExplorer* explorerPtr);
		static Tester* instance(TreeGrower* growerPtr);
		static Tester* instance(TreeTrimmer* trimmerPtr);
		static Tester* reInstance(const NeuronStructExplorer* explorerPtr);
		static Tester* reInstance(TreeGrower* growerPtr);
		static Tester* reInstance(TreeTrimmer* trimmerPtr);
		static Tester* getInstance();
		static void uninstance();
		static bool isInstantiated() { return testerInstance != nullptr; }

		shared_ptr<const NeuronStructExplorer*> sharedExplorerPtr;
		shared_ptr<TreeGrower*> sharedGrowerPtr;
		shared_ptr<TreeTrimmer*> sharedTrimmerPtr;

		map<int, set<vector<float>>> getSegEndClusterNodeMap(const profiledTree& inputProfiledTree) const;

		void checkSegHeadClusters(const profiledTree& inputProfiledTree, const QString& saveNamePrefixQ) const;
		void checkSegTailClusters(const profiledTree& inputProfiledTree, const QString& saveNamePrefixQ) const;
		void checkHeadSegID2cluslter(const profiledTree& inputProfiledTree, const QString& saveNamePrefixQ) const;
		void checkTailSegID2cluslter(const profiledTree& inputProfiledTree, const QString& saveNamePrefixQ) const;
		void assignRGBcolors(map<int, pair<boost::container::flat_set<int>, RGBA8>>& inputMap) const;

	private:
		Tester(const NeuronStructExplorer* explorerPtr);
		Tester(TreeGrower* growerPtr);
		Tester(TreeTrimmer* trimmerPtr);
	};
}

#endif