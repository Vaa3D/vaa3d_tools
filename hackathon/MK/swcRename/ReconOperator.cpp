#include "ReconOperator.h"

void ReconOperator::downSampleReconFile(const QStringList& fileList, float xFactor, float yFactor, float zFactor)
{
	QString newFolderName = this->rootPath + "\\downsampled";
	QDir newDir(newFolderName);
	if (!newDir.exists()) newDir.mkpath(".");

	for (auto& file : fileList)
	{
		QString fullFileName = this->rootPath + "\\" + file;
		if (file.endsWith("swc"))
		{
			QString outputFullName = newFolderName + "\\" + file.left(file.length() - 4) + "_xy" + QString::number(int(1 / xFactor)) + "z" + QString::number(int(1 / zFactor)) + ".swc";
			NeuronTree outputTree = NeuronStructUtil::swcScale(readSWC_file(fullFileName), xFactor, yFactor, zFactor);
			writeSWC_file(outputFullName, outputTree);
		}
		else if (file.endsWith("eswc"))
		{
			QString outputFullName = newFolderName + "\\" + file.left(file.length() - 5) + "_xy" + QString::number(int(1 / xFactor)) + "z" + QString::number(int(1 / zFactor)) + ".eswc";
			NeuronTree outputTree = NeuronStructUtil::swcScale(readSWC_file(fullFileName), xFactor, yFactor, zFactor);
			writeSWC_file(outputFullName, outputTree);
		}
		else if (file.endsWith("apo"))
		{
			QString outputFullName = newFolderName + "\\" + file.left(file.length() - 4) + "_xy" + QString::number(int(1 / xFactor)) + "z" + QString::number(int(1 / zFactor)) + ".apo";
			QList<CellAPO> outputApo = NeuronStructUtil::apoScale(readAPO_file(fullFileName), xFactor, yFactor, zFactor);
			writeAPO_file(outputFullName, outputApo);
		}
		else if (file.endsWith("ano"))
		{
			QString outputFullName = newFolderName + "\\" + file.left(file.length() - 4) + "_xy" + QString::number(int(1 / xFactor)) + "z" + QString::number(int(1 / zFactor)) + ".ano";
			QFile::copy(fullFileName, outputFullName);
		}
	}
}

void ReconOperator::denAxonSeparate(const QStringList& fileList)
{
	QString denFolderName = this->rootPath + "\\dendrite";
	QDir denDir(denFolderName);
	if (!denDir.exists()) denDir.mkpath(".");
	QString axonFolderName = this->rootPath + "\\axon";
	QDir axonDir(axonFolderName);
	if (!axonDir.exists()) axonDir.mkpath(".");

	for (auto& file : fileList)
	{
		if (file.endsWith("swc") || file.endsWith("eswc"))
		{
			QString fullSWCname = this->rootPath + "\\" + file;
			NeuronTree inputTree = readSWC_file(fullSWCname);
			map<int, QList<NeuronSWC>> swcTypeMap = NeuronStructUtil::swcSplitByType(inputTree);

			NeuronTree denTree;
			if (swcTypeMap.find(1) != swcTypeMap.end()) denTree.listNeuron.append(swcTypeMap.at(1));
			denTree.listNeuron.append(swcTypeMap.at(3));
			if (swcTypeMap.find(4) != swcTypeMap.end()) denTree.listNeuron.append(swcTypeMap.at(4));

			NeuronTree axonTree;
			axonTree.listNeuron.append(swcTypeMap.at(2));

			QString denSWCfullName = denFolderName + "\\" + file;
			writeSWC_file(denSWCfullName, denTree);
			QString axonSWCfullName = axonFolderName + "\\" + file;
			writeSWC_file(axonSWCfullName, axonTree);
		}
	}

	v3d_msg(QString("Neurite separation done."));
}

void ReconOperator::denAxonCombine(bool dupRemove)
{
	QDir inputDenFileFolder(this->rootPath + "\\dendrite");
	QDir inputAxonFileFolder(this->rootPath + "\\axon");
	if (!inputDenFileFolder.exists() || !inputAxonFileFolder.exists())
	{
		v3d_msg(QString("Dendrite folder and axon folder not existing. Do nothing and return."));
		return;
	}
	QString outputFolderName = this->rootPath + "\\denAxonCombined\\";
	QDir outputDir(outputFolderName);
	if (!outputDir.exists()) outputDir.mkpath(".");
	inputAxonFileFolder.setFilter(QDir::Files | QDir::NoDotAndDotDot);
	QStringList axonFileList = inputAxonFileFolder.entryList();

	for (auto& axonFile : axonFileList)
	{
		if (axonFile.endsWith(".ano") || axonFile.endsWith(".apo")) continue;

		vector<NeuronTree> trees;
		QString denFullName;
		QString denFullNameEswcQ = this->rootPath + "\\dendrite\\" + axonFile.left(axonFile.length() - 4) + ".ano.eswc";
		QString denFullNameSwcQ = this->rootPath + "\\dendrite\\" + axonFile;

		if (QFile::exists(denFullNameEswcQ)) denFullName = denFullNameEswcQ;
		else denFullName = denFullNameSwcQ;
		trees.push_back(readSWC_file(denFullName));
		QString axonFullName = this->rootPath + "\\axon\\" + axonFile;
		trees.push_back(readSWC_file(axonFullName));
		NeuronTree outputTree = NeuronStructUtil::swcCombine(trees);
		
		if (dupRemove) outputTree = NeuronStructUtil::removeDupNodes(outputTree);
		QString outputSWCfullName = outputFolderName + axonFile;
		writeSWC_file(outputSWCfullName, outputTree);
	}
	
	v3d_msg(QString("Dendrite and axon conbination done."));
}

void ReconOperator::removeDupedNodes()
{
	QDir inputFolder(this->rootPath);
	inputFolder.setFilter(QDir::Files | QDir::NoDotAndDotDot);
	QStringList swcFileList = inputFolder.entryList();

	QString outputFolderQ = this->rootPath + "\\allCleanedUp\\";
	QString remainingFolderQ = this->rootPath + "\\stillMoreThan1Root\\";
	QDir outputDir(outputFolderQ);
	QDir remainingDir(remainingFolderQ);
	if (!outputDir.exists()) outputDir.mkpath(".");

	for (auto& file : swcFileList)
	{
		QString inputSWCFullName = this->rootPath + "\\" + file;
		NeuronTree inputTree = readSWC_file(inputSWCFullName);
		if (NeuronStructUtil::multipleRootCheck(inputTree))
		{
			QString outputSWCfullName;
			NeuronTree dupRemovedTree = NeuronStructUtil::removeDupNodes(inputTree);
			if (!NeuronStructUtil::multipleRootCheck(dupRemovedTree))
			{
				if (file.endsWith("eswc")) outputSWCfullName = outputFolderQ + file.left(file.length() - 4) + "swc";
				else outputSWCfullName = outputFolderQ + file;
				writeSWC_file(outputSWCfullName, dupRemovedTree);
			}
			else
			{
				if (file.endsWith("eswc")) outputSWCfullName = remainingFolderQ + file.left(file.length() - 4) + "swc";
				else outputSWCfullName = remainingFolderQ + file;
				if (!remainingDir.exists()) remainingDir.mkpath(".");
				writeSWC_file(outputSWCfullName, dupRemovedTree);
			}
		}
		else
		{
			QString oldName = this->rootPath + "\\" + file;
			QString newName = outputFolderQ + file;
			QFile::copy(oldName, newName);
		}
	}
}