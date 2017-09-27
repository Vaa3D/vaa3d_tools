#include "Dataset_Generator_plugin.h"
#include "dirent.h"
#include <fstream>
#include <stdlib.h>

using namespace std;

DatasetGenerator::DatasetGenerator() 
{

}

void DatasetGenerator::getImageFolders()
{
	const char* swcD;
	const char* imgD;
	string s1 = this->inputSWCdir.toStdString();
	swcD = s1.c_str();
	string s2 = this->inputSWCdir.toStdString();
	imgD = s2.c_str();

	QString imgFolderName;

	DIR* dir;
	struct dirent *ent;
	if ((dir = opendir(imgD)) != NULL) 
	{
		int _count = 0;
		bool flag = false;
		while ((ent = readdir(dir)) != NULL) 
		{
			for (size_t i=0; i<30; ++i) 
			{
				if (ent->d_name[i] == NULL) break;
				imgFolderName = imgFolderName + QChar(ent->d_name[i]);
			}
			if (imgFolderName == "." || imgFolderName == "..") 
			{
				imgFolderName.clear();
				continue;	
			}
			//qDebug() << imgFolderName;

			this->imageFolders.push_back(imgFolderName);

		
			imgFolderName.clear();
		}
	}
	closedir (dir);
}

void DatasetGenerator::pick_save()
{
	//for (QVector<QString>::iterator it=this->imageFolders.begin(); it!=this->imageFolders.end(); ++it)
	//{

	//}

	QString testFolder = this->imageFolders[0];
	QString testFolder = this->inputImagedir + testFolder;
	const char* imgFolder;
	string str = testFolder.toStdString();
	imgFolder = str.c_str();

	DIR* dir;
	struct dirent *ent;
	if ((dir = opendir(imgFolder)) != NULL) 
	{
		int _count = 0;
		bool flag = false;
		QString swcFileName;
		while ((ent = readdir(dir)) != NULL) 
		{
			for (size_t i=0; i<50; ++i) 
			{
				if (ent->d_name[i] == NULL) break;
				swcFileName = swcFileName + QChar(ent->d_name[i]);
			}
			if (swcFileName == "." || swcFileName == "..") 
			{
				swcFileName.clear();
				continue;	
			}
			//qDebug() << swcFileName;
			

		
			swcFileName.clear();
		}
	}
	closedir (dir);
}