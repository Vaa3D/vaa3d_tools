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
		static Tester* instance(NeuronStructExplorer* explorerPtr);
		static Tester* instance(TreeGrower* growerPtr);
		static Tester* instance(TreeTrimmer* trimmerPtr);
		static Tester* reInstance(NeuronStructExplorer* explorerPtr);
		static Tester* reInstance(TreeGrower* growerPtr);
		static Tester* reInstance(TreeTrimmer* trimmerPtr);
		static Tester* getInstance();
		static void uninstance();
		static bool isInstantiated() { return testerInstance != nullptr; }

		shared_ptr<NeuronStructExplorer*> sharedExplorerPtr;
		shared_ptr<TreeGrower*> sharedGrowerPtr;
		shared_ptr<TreeTrimmer*> sharedTrimmerPtr;

		map<int, set<vector<float>>> getSegEndClusterNodeMap(const profiledTree& inputProfiledTree);

	private:
		Tester(NeuronStructExplorer* explorerPtr);
		Tester(TreeGrower* growerPtr);
		Tester(TreeTrimmer* trimmerPtr);
	};
}

#endif