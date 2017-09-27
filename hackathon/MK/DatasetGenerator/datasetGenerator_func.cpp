#include "Dataset_Generator_plugin.h"
#include "dirent.h"
#include <fstream>
#include <stdlib.h>
#include <qstringlist.h>
#include <iostream>
#include <cstdlib>
#include <qfile.h>

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
	string s2 = this->inputImagedir.toStdString();
	imgD = s2.c_str();

	QString imgFolderName;
	DIR* dir;
	struct dirent *ent;
	if ((dir = opendir(imgD)) != NULL) 
	{
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
			imgFolderName = this->inputImagedir + "/" + imgFolderName;
			this->imageFolders.push_back(imgFolderName);
			imgFolderName.clear();
		}
	}
	closedir(dir);
}

void DatasetGenerator::pick_save()
{
	QString imgName;
	for (QVector<QString>::iterator it=this->imageFolders.begin(); it!=this->imageFolders.end(); ++it)
	{
		QStringList folderName = it->split("\\");
		QStringList imgPreList1 = folderName[9].split("/");
		QStringList imgPreList2 = imgPreList1[1].split("_");
		imgName = imgPreList2[0];
		//qDebug() << imgName;

		const char* swcDIR;
		QString inputSWCPath = this->inputSWCdir + "\\";
		string str = inputSWCPath.toStdString();
		swcDIR = str.c_str();
		//cout << swcDIR << endl;
		QString intoSWCFolder = inputSWCPath;
		DIR* dir;
		struct dirent *ent;
		struct dirent* imgEnt;

		if ((dir = opendir(swcDIR)) != NULL) 
		{
			bool flag = false;
			QString swcFolderName;
			while ((ent = readdir(dir)) != NULL) 
			{
				for (size_t i=0; i<30; ++i) 
				{
					if (ent->d_name[i] == NULL) break;
					swcFolderName = swcFolderName + QChar(ent->d_name[i]);
				}

				if (swcFolderName == imgName) 
				{
					intoSWCFolder = intoSWCFolder + swcFolderName;
					swcFolderName.clear();
					break;
				}
				swcFolderName.clear();
			}
			//qDebug() << swcFolderName;
			swcFolderName.clear();
		}
		//qDebug() << intoSWCFolder;
		closedir(dir);

		QString outputfolder = "Z:\\IVSCC\\allDendrite\\" + imgName;
		QDir().mkdir(outputfolder);

		string str2 = intoSWCFolder.toStdString();
		const char* intoSWCdir = str2.c_str();
		if ((dir = opendir(intoSWCdir)) != NULL) 
		{
			int _count = 0;
			bool flag = false;
			QString fileName;
			while ((ent = readdir(dir)) != NULL) 
			{
				for (size_t i=0; i<30; ++i) 
				{
					if (ent->d_name[i] == NULL) break;
					fileName = fileName + QChar(ent->d_name[i]);
				}

				if (fileName == "manual.swc_p.swc")
				{
					QString nodeX, nodeY, nodeZ;
					QString swcFullName = intoSWCFolder + "\\manual.swc_p.swc";
					NeuronTree inputSWCTree = readSWC_file(swcFullName);
					for (QList<NeuronSWC>::iterator neuronIt=inputSWCTree.listNeuron.begin(); neuronIt!=inputSWCTree.listNeuron.end(); ++neuronIt)
					{
						if (neuronIt->type == 3 || neuronIt->type == 4)
						{
							nodeX = QString::number(int(neuronIt->x)); 
							nodeY = QString::number(int(neuronIt->y));
							nodeZ = QString::number(int(neuronIt->z));

							const char* imgDir;
							string str3 = it->toStdString();
							imgDir = str3.c_str();
							DIR* getImgDir;
							//cout << imgDir << endl;
							if ((getImgDir = opendir(imgDir)) != NULL)
							{
								while ((imgEnt = readdir(getImgDir)) != NULL)
								{
									QString imgFileName;
									for (size_t i=0; i<50; ++i) 
									{
										if (imgEnt->d_name[i] == NULL) break;
										imgFileName = imgFileName + QChar(imgEnt->d_name[i]);
									}
									if (imgFileName == "." || imgFileName == "..") 
									{
										imgFileName.clear();
										continue;	
									}
									//qDebug() << imgFileName;
							
									QStringList nameSplit = imgFileName.split("_");
									QStringList xNameSplit = nameSplit[5].split("x");
									QStringList yNameSplit = nameSplit[6].split("y");
									QStringList zSplit = nameSplit[7].split("z");
									QStringList zNameSplit = zSplit[1].split(".");
									QString xName = xNameSplit[1];
									QString yName = yNameSplit[1];
									QString zName = zNameSplit[0];

									if (nodeX == xName && nodeY == yName && nodeZ == zName)
									{
										QString command = "copy " + *it + "\\" + imgFileName;
										command = command + " " + outputfolder + "\\" + imgFileName;
										command.replace("/", "\\");
										string commandStr = command.toStdString();
										const char* systemCmd = commandStr.c_str();
										cout << systemCmd << endl;
										system(systemCmd);
									}
								}
							}
							closedir(getImgDir);
						}
					}
				}
				fileName.clear();
			}
		}
		closedir(dir);
	}
}