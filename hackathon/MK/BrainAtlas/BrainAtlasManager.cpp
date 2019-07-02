#include "BrainAtlasManager.h"

BrainAtlasManaer::BrainAtlasManaer(QWidget* parent, V3DPluginCallback2* callback)
{
	this->thisCallback = callback;
	
	QString brgFilePath = "../../vaa3d_tools/hackathon/MK/BrainAtlas/brgs";
	QDir inputDir(brgFilePath);
	inputDir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
	QStringList brgFileList = inputDir.entryList();
	cout << brgFileList.size() << endl;
	for (QStringList::iterator it = brgFileList.begin(); it != brgFileList.end(); ++it)
		cout << (*it).toStdString() << endl;
}