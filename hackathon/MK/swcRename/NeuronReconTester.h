#ifndef NEURONRECONTESTER_H
#define NEURONRECONTESTER_H

#include "ReconOperator.h"

#include "NeuronStructExplorer.h"
#include "neuronReconErrorTypes.h"
#include "integratedDataTypes.h"

class NeuronReconTester
{
public:
	static NeuronReconTester* testerInstance;
	static NeuronReconTester* instance(const ReconOperator* inputOperatorPtr);
	static NeuronReconTester* getInstance();
	static void uninstance();
	static bool isInstantiated() { return testerInstance != nullptr; }

	ReconOperator const* reconOperatorPtr;

	QString rootPathQ, connectedTreePathQ, connectedAssembledTreePathQ;
	void makeConnectedFolder();
	void makeConnectedAssembledFolder();
	void saveIntermediateResult(const NeuronTree& inputTree, QString folderNameQ, QString fileNameQ);
	void saveIntermediateResult(const boost::container::flat_map<int, profiledTree>& connectedTrees, QString savePathQ, QString baseNameQ);

private:
	NeuronReconTester(const ReconOperator* inputOperatorPtr) : reconOperatorPtr(inputOperatorPtr) {}
};

#endif