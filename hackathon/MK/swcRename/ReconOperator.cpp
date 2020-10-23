#include "ReconOperator.h"

void ReconOperator::downSampleReconFile(const QStringList& fileList, float xFactor, float yFactor, float zFactor)
{
	QString newFolderName = this->rootPath + "\\downsampled";
	QDir newDir(newFolderName);
	if (!newDir.exists()) newDir.mkpath(".");

	for (auto& file : fileList)
	{
		QString fullFileName = this->rootPath + "\\" + file;
		QString outputFullName = newFolderName + "\\" + file;
		if (file.endsWith("swc") || file.endsWith("eswc"))
		{
			NeuronTree outputTree = NeuronStructUtil::swcScale(readSWC_file(fullFileName), xFactor, yFactor, zFactor);
			writeSWC_file(outputFullName, outputTree);
		}
		else if (file.endsWith("apo"))
		{
			QList<CellAPO> outputApo = NeuronStructUtil::apoScale(readAPO_file(fullFileName), xFactor, yFactor, zFactor);
			writeAPO_file(outputFullName, outputApo);
		}
		else if (file.endsWith("ano")) QFile::copy(fullFileName, outputFullName);
	}
}