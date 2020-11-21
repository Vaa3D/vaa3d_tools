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

void ReconOperator::denAxonCombine()
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
	inputDenFileFolder.setFilter(QDir::Files | QDir::NoDotAndDotDot);
	QStringList denFileList = inputDenFileFolder.entryList();

	for (auto& denFile : denFileList)
	{
		QString denFullName = this->rootPath + "\\dendrite\\" + denFile;
		NeuronTree outputTree = readSWC_file(denFullName);
		QString axonFullName = this->rootPath + "\\axon\\" + denFile;
		outputTree.listNeuron.append(readSWC_file(axonFullName).listNeuron);
		QString outputSWCfullName = outputFolderName + denFile;
		writeSWC_file(outputSWCfullName, outputTree);
	}
	
	v3d_msg(QString("Dendrite and axon conbination done."));
}