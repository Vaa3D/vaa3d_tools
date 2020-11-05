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