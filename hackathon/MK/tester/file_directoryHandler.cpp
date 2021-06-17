#include "file_directoryHandler.h"

vector<QString> File_DirectoryHandler::getFiles(QString path, QString filter)
{
	QDir inputDir(path);
	inputDir.setFilter(QDir::Files | QDir::NoDotAndDotDot);

	vector<QString> outputVec;
	QStringList fileList = inputDir.entryList();
	if (filter != "none")
	{
		for (auto& fileName : fileList)
		{
			//qDebug() << fileName;
			if (fileName.contains(filter))
			{
				QString outputFileFullName = path + "\\" + fileName;
				outputVec.push_back(outputFileFullName);
			}
		}
	}

	return outputVec;
}