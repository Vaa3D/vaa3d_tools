#include <dirent.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <ctime>

#include <QStringList>
#include <QFile>
#include <QVector>
#include <QFileInfo>
#include <QDir>

#include "Analyzer.h"

using namespace std;
using namespace iim;

void Analyzer::imgSetHistProfile(string inputPath, map<size_t, vector<float>>) 
{
	const char* pathC_string;
	pathC_string = inputPath.c_str();
	
	DIR* dir;
	struct dirent* ent;
	if ((dir = opendir(pathC_string)) != NULL)
	{
		QString imgFolderName;
		while ((ent = readdir(dir)) != NULL)
		{
			for (size_t i = 0; i<30; ++i)
			{
				if (ent->d_name[i] == NULL) break;
				imgFolderName = imgFolderName + QChar(ent->d_name[i]);
			}
			if (imgFolderName == "." || imgFolderName == "..")
			{
				imgFolderName.clear();
				continue;
			}
			qDebug() << imgFolderName;

			string imgFolder = inputPath + "\\" + imgFolderName.toStdString();
			const char* imgFolderC_str = imgFolder.c_str();
			DIR* dirPatch;
			struct dirent* patchEnt;
			if ((dirPatch = opendir(imgFolderC_str)) != NULL)
			{
				QString QPatchName;
				while ((patchEnt = readdir(dirPatch)) != NULL)
				{
					for (size_t i = 0; i < 50; ++i)
					{
						if (patchEnt->d_name[i] == NULL) break;
						QPatchName = QPatchName + QChar(patchEnt->d_name[i]);
					}
					if (QPatchName == "." || QPatchName == "..")
					{
						QPatchName.clear();
						continue;
					}
					//qDebug() << QPatchName;
					
					string imgFullName = imgFolder + "\\" + QPatchName.toStdString();
					unsigned char* ImgPtr = 0;
					V3DLONG in_sz[4];
					int datatype;
					if (!simple_loadimage_wrapper(*analyzerCallback, imgFullName.c_str(), ImgPtr, in_sz, datatype))
					{
						cerr << "Error reading image file [" << imgFullName << "]. Exit." << endl;
						return;
					}
					int j = 0;
					float sum = 0;
					for (j = 0; j < in_sz[0] * in_sz[1]; ++j) sum = sum + ImgPtr[j];
					this->patchMean = sum / float(j + 1);
					this->patchMeans.push_back(patchMean);
					//cout << this->patchMean << " "; 

					float varSum = 0;
					for (j = 0; j < in_sz[0] * in_sz[1]; ++j) varSum = varSum + ((ImgPtr[j] - this->patchMean)*(ImgPtr[j] - this->patchMean));
					this->patchVar = varSum / float(j + 1);
					this->patchVars.push_back(this->patchVar);
					//cout << this->patchVar << " ";
					
					QPatchName.clear();
				}			
			}


			closedir(dirPatch);

			imgFolderName.clear();
		}
		imgFolderName.clear();
	}
	
	closedir(dir);




}


float Analyzer::stdCompute(vector<float> patchArray)
{
	return 0;
}