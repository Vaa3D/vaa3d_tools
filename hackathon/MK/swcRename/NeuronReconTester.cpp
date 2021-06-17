#include "NeuronReconTester.h"

#include "NeuronStructNavigatingTester.h"

using namespace std;

NeuronReconTester* NeuronReconTester::testerInstance = nullptr;

NeuronReconTester* NeuronReconTester::instance(const ReconOperator* inputOperatorPtr)
{
	NeuronReconTester::testerInstance = new NeuronReconTester(inputOperatorPtr);
	return NeuronReconTester::testerInstance;
}

NeuronReconTester* NeuronReconTester::getInstance()
{
	if (NeuronReconTester::testerInstance != nullptr) return NeuronReconTester::testerInstance;
	else return nullptr;
}

void NeuronReconTester::uninstance()
{
	if (NeuronReconTester::instance != nullptr) delete NeuronReconTester::testerInstance;
	NeuronReconTester::testerInstance = nullptr;
}

void NeuronReconTester::makeConnectedFolder()
{
	this->connectedTreePathQ = this->rootPathQ + "\\connectedTrees\\";
	QDir connectedTreeFolder(this->connectedTreePathQ);
	if (!connectedTreeFolder.exists()) connectedTreeFolder.mkpath(".");
}

void NeuronReconTester::makeConnectedAssembledFolder()
{
	this->connectedAssembledTreePathQ = this->rootPathQ + "\\connectedAssembledTrees\\";
	QDir connectedAssembledTreeFolder(this->connectedAssembledTreePathQ);
	if (!connectedAssembledTreeFolder.exists()) connectedAssembledTreeFolder.mkpath(".");
}

void NeuronReconTester::saveIntermediateResult(const NeuronTree& inputTree, QString folderNameQ, QString fileNameQ)
{
	if (folderNameQ == "")
	{
		QString fullSaveNameQ = this->rootPathQ + "\\" + fileNameQ + ".swc";
		writeSWC_file(fullSaveNameQ, inputTree);
	}
	else
	{
		QString savePathQ = this->rootPathQ + "\\" + folderNameQ + "\\";
		QDir saveDir(savePathQ);
		if (!saveDir.exists()) saveDir.mkpath(".");
		QString fullSaveNameQ = savePathQ + fileNameQ + ".swc";
		writeSWC_file(fullSaveNameQ, inputTree);
	}
}

void NeuronReconTester::saveIntermediateResult(const boost::container::flat_map<int, profiledTree>& connectedTrees, QString savePathQ, QString baseNameQ)
{
	QString caseSavePathQ = savePathQ + baseNameQ + "\\";
	QDir connectedTreesRootFolder(caseSavePathQ);
	if (!connectedTreesRootFolder.exists()) connectedTreesRootFolder.mkpath(".");

	for (auto& tree : connectedTrees)
	{
		QString saveFullNameQ = caseSavePathQ + baseNameQ + QString::fromStdString(to_string(tree.first)) + ".swc";
		writeSWC_file(saveFullNameQ, tree.second.tree);
	}
}