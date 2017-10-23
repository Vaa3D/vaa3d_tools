#include "Dataset_Generator_plugin.h"
#include "dirent.h"
#include <fstream>
#include <stdlib.h>
#include <qstringlist.h>
#include <iostream>
#include <cstdlib>
#include <qfile.h>
#include <ctime>

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
	int imgFolderCount = 0;
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
			++imgFolderCount;

			imgFolderName = this->inputImagedir + "/" + imgFolderName;
			//qDebug() << imgFolderName << "\ntotal folder number: " << imgFolderCount;
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

		QString outputfolder = "Z:\\IVSCC\\allAxon\\" + imgName;
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
						if (neuronIt->type == 2)
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

void DatasetGenerator::createList()
{
	this->valProportion = 0.2;
	srand(time(NULL));
	
	ofstream outfileTrain("train.txt");
	ofstream outfileVal("val.txt");
	for (QVector<QString>::iterator it=this->imageFolders.begin(); it!=this->imageFolders.end(); ++it)
	{
		qDebug() << *it;
		DIR* dir;
		struct dirent *ent;
		string imgFolderString = (*it).toStdString();
		const char* imgFolderName = imgFolderString.c_str();
		
		QVector<QString> imgNames;
		if ((dir = opendir(imgFolderName)) != NULL) 
		{
			int fileCount = 0;
			QString name;
			while ((ent = readdir(dir)) != NULL) 
			{
				++fileCount;
				if (fileCount <= 2) continue;
				for (size_t i=0; i<100; ++i) 
				{
					if (ent->d_name[i] == NULL) break;
					else if (ent->d_name[i] == '..'  ||  ent->d_name[i] == '...') continue;
					name = name + QChar(ent->d_name[i]);
				}
				imgNames.push_back(name);
				//qDebug() << name;
				name.clear();
			}

			int imgNum = imgNames.size();
			int valTotalNum = imgNum * this->valProportion;
			cout << "  total patches number: " << imgNum << endl;
			vector<int> valNums; 
			int count = 1;
			while (count <= valTotalNum)
			{
				int num = rand() % valTotalNum + 1;
				valNums.push_back(num);
				++count;
			}
			cout << "  validation patches number: " << valNums.size() << endl << endl;
			
			QString fullPath = *it;
			fullPath.replace("\\/", "/");
			fullPath.replace("\\", "/");
			fullPath.replace("Z:", "MK_Drive");
			for (int i=0; i<imgNum; ++i)
			{
				string patchName = fullPath.toStdString();
				patchName = patchName + "/" + imgNames[i].toStdString();
				//cout << patchName << endl;
				bool valFlag = false;
				for (vector<int>::iterator valIt=valNums.begin(); valIt!=valNums.end(); ++valIt)
				{
					if (*valIt == i)
					{
						outfileVal << patchName << " 2" << endl;
						valFlag = true;
						break;
					}
				}
				if (valFlag == false) outfileTrain << patchName << " 2" << endl;
			}
			valNums.clear();
		}

		imgNames.clear();
		//system("pause");
	}
}

void DatasetGenerator::createList2()
{
	this->proportion1 = 0.5;
	//this->proportion2 = 0.2;		
	srand(time(NULL));
	
	ofstream outfileTrain("train.txt");
	ofstream outfileVal("val.txt");
	for (QVector<QString>::iterator it=this->imageFolders.begin(); it!=this->imageFolders.end(); ++it)
	{
		qDebug() << *it;
		DIR* dir;
		struct dirent *ent;
		string imgFolderString = (*it).toStdString();
		const char* imgFolderName = imgFolderString.c_str();
		
		QVector<QString> imgNames;
		if ((dir = opendir(imgFolderName)) != NULL) 
		{
			int fileCount = 0;
			QString name;
			while ((ent = readdir(dir)) != NULL) 
			{
				++fileCount;
				if (fileCount <= 2) continue;
				for (size_t i=0; i<100; ++i) 
				{
					if (ent->d_name[i] == NULL) break;
					else if (ent->d_name[i] == '..'  ||  ent->d_name[i] == '...') continue;
					name = name + QChar(ent->d_name[i]);
				}
				imgNames.push_back(name);
				//qDebug() << name;
				name.clear();
			}

			int imgNum = imgNames.size();
			int poolTotalNum = imgNum * this->proportion1;
			cout << "  total patches number: " << imgNum << endl;
			vector<int> poolNums; 
			int count = 1;
			while (count <= poolTotalNum)
			{
				int num = rand() % poolTotalNum + 1;
				poolNums.push_back(num);
				++count;
				//cout << num << " ";
			}
			cout << "  pool patches number: " << poolNums.size() << endl << endl;
			
			QString fullPath = *it;
			fullPath.replace("\\/", "/");
			fullPath.replace("\\", "/");
			//fullPath.replace("Z:", "MK_Drive");
			for (int i=0; i<imgNum; ++i)
			{
				for (vector<int>::iterator poolIt=poolNums.begin(); poolIt!=poolNums.end(); ++poolIt)
				{
					if (*poolIt == i)
					{
						string patchName = fullPath.toStdString();
						patchName = patchName + "/" + imgNames[i].toStdString();
						if (size_t(poolIt-poolNums.begin()+1) <= poolNums.size()*0.8) 
						{
							//cout << *poolIt << " ";
							outfileTrain << patchName << " 0" << endl;
						}
						else 
						{
							//cout << "val" << *poolIt << " ";
							outfileVal << patchName << " 0" << endl;
						}
					}
				}				
			}
			poolNums.clear();
			cout << endl;
		}

		imgNames.clear();
		//system("pause");
	}
}