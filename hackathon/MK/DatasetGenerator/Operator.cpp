#include "Operator.h"

#include "dirent.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <ctime>

#include <qstringlist.h>
#include <qfile.h>
#include <qvector.h>



using namespace std;

void Operator::taskQueuDispatcher()
{
	while (!taskQueu.empty())
	{
		operatingTask = taskQueu.front();
		if (taskQueu.front().createPatch == true)
		{ }

		if (taskQueu.front().createList == true)
		{
			if (taskQueu.front().listOp == subset) createListFromList(subset);
		}

		if (taskQueu.front().createPatchNList == true)
		{ }

		taskQueu.pop();
	}
}

void Operator::createListFromList(listOpType listOp)
{
	if (listOp == subset)
	{
		ifstream inputFile(operatingTask.source);
		ofstream outputFile(operatingTask.outputFileName);

		string line;
		vector<string> lineSplit;
		vector<string> wholeLines;
		int classLabel = 0;
		while (getline(inputFile, line))
		{
			stringstream lineStream(line);
			string streamedLine;
			while (lineStream >> streamedLine) lineSplit.push_back(streamedLine);
			
			int currLabel = stoi(*(lineSplit.end() - 1));
			if (currLabel == classLabel)
			{
				wholeLines.push_back(line);
				continue;
			}
			else
			{
				srand(time(NULL));
				size_t neededNum = operatingTask.subsetRatio * wholeLines.size();
				vector<size_t> pickedLineNums;
				size_t pickedCount = 0;
				while (pickedCount <= neededNum)
				{
					size_t num = rand() % neededNum + 1;
					outputFile << wholeLines[num] << endl;
					//cout << wholeLines[num] << endl;
					++pickedCount;
				}

				classLabel = currLabel;
				wholeLines.clear();
				lineSplit.clear();
				pickedLineNums.clear();
				wholeLines.push_back(line);
			}
		}
		
		srand(time(NULL));
		size_t neededNum = operatingTask.subsetRatio * wholeLines.size();
		vector<size_t> pickedLineNums;
		size_t pickedCount = 0;
		while (pickedCount <= neededNum)
		{
			size_t num = rand() % neededNum + 1;
			outputFile << wholeLines[num] << endl;
			//cout << wholeLines[num] << endl;
			++pickedCount;
		}

		wholeLines.clear();
		lineSplit.clear();
		pickedLineNums.clear();
	}
}

void Operator::createList2()
{
	this->proportion1 = 0.5;
	//this->proportion2 = 0.2;		
	srand(time(NULL));

	ofstream outfileTrain("train.txt");
	ofstream outfileVal("val.txt");
	for (QVector<QString>::iterator it = this->imageFolders.begin(); it != this->imageFolders.end(); ++it)
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
				for (size_t i = 0; i<100; ++i)
				{
					if (ent->d_name[i] == NULL) break;
					else if (ent->d_name[i] == '..' || ent->d_name[i] == '...') continue;
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
			for (int i = 0; i<imgNum; ++i)
			{
				for (vector<int>::iterator poolIt = poolNums.begin(); poolIt != poolNums.end(); ++poolIt)
				{
					if (*poolIt == i)
					{
						string patchName = fullPath.toStdString();
						patchName = patchName + "/" + imgNames[i].toStdString();
						if (size_t(poolIt - poolNums.begin() + 1) <= poolNums.size()*0.8)
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