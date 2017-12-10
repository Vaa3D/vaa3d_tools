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
#include <QFileinfo>
#include <QDir>

#include "Operator.h"

using namespace std;
using namespace iim;

void Operator::taskQueuDispatcher()
{
	cout << "task number: " << taskQueu.size() << endl;
	while (!taskQueu.empty())
	{
		operatingTask = taskQueu.front();
		
		if (operatingTask.createPatch == true)
		{
			if (operatingTask.patchOp == stackTo2D)
			{
				emit progressBarReporter("Creating Patches..", 0);
				create2DPatches(stackTo2D);
				emit progressBarReporter("Complete. ", 100);
				cout << "Patch generation complete." << endl;
			}
			else if (operatingTask.patchOp == teraTo2D)
			{
				emit progressBarReporter("Creating Patches..", 0);
				create2DPatches(teraTo2D);
				emit progressBarReporter("Complete. ", 100);
				cout << "Patch generation complete." << endl;
			}
			else if (operatingTask.patchOp == stackTo3D)
			{
				emit progressBarReporter("Creating Patches..", 0);
				create3DPatches(stackTo3D);
				emit progressBarReporter("Complete. ", 100);
				cout << "Patch generation complete." << endl;
			}
			else if (operatingTask.patchOp == teraTo3D)
			{
				emit progressBarReporter("Creating Patches..", 0);
				create3DPatches(teraTo3D);
				emit progressBarReporter("Complete. ", 100);
				cout << "Patch generation complete." << endl;
			}
		}

		if (operatingTask.createList == true)
		{
			if (operatingTask.listOp == subset)
			{
				emit progressBarReporter("Creating List..  ", 0);
				createListFromList(subset);
				emit progressBarReporter("Complete. ", 100);
			}
			else if (operatingTask.listOp == crossVal)
			{
				emit progressBarReporter("Creating Lists..  ", 0);
				createListFromList(crossVal);
				emit progressBarReporter("Complete. ", 100);
			}
			else if (operatingTask.listOp == newList)
			{
				emit progressBarReporter("Creating Lists..  ", 0);
				createListFromPatch(operatingTask.subsetRatio);
				emit progressBarReporter("Complete. ", 100);
			}
		}

		taskQueu.pop();
	}
}

void Operator::createListFromList(listOpType listOp)
{
	if (listOp == subset) // Create a subset list out of a given list.
	{
		ifstream inputFile_forSize(operatingTask.source.c_str());
		ifstream inputFile(operatingTask.source.c_str());
		ofstream outputFile(operatingTask.outputFileName.c_str());

		string tempLine;
		double lineCount;
		for (lineCount = 0; getline(inputFile_forSize, tempLine); ++lineCount);
		cout << "total lines: " << lineCount << endl;
		inputFile_forSize.close();

		string line;
		vector<string> lineSplit;
		vector<string> wholeLines;
		int classLabel = 0;
		double count = 0;
		while (getline(inputFile, line))
		{
			++count;
			stringstream lineStream(line);
			string streamedLine;
			while (lineStream >> streamedLine) lineSplit.push_back(streamedLine);
			
			int currLabel;
			string currLabelStr = *(lineSplit.end() - 1);
			istringstream(currLabelStr) >> currLabel;
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

				double processedPortion = count / lineCount;
				int percentageNum = int(processedPortion * 100);
				emit progressBarReporter("Creating List..  ", percentageNum);
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

		inputFile.close();
		outputFile.close();
	}
	else if (listOp == crossVal) // Create cross validation lists set with given fold number.
	{
		ifstream inputFile(operatingTask.source.c_str());

		string line;
		vector<string> lineSplit;
		int classLabel = 0;
		vector<vector<string> > allLinesByClass;
		vector<string> classLines;
		while (getline(inputFile, line))
		{
			stringstream lineStream(line);
			string streamedLine;
			while (lineStream >> streamedLine) lineSplit.push_back(streamedLine);

			int currLabel;
			string currLabelStr = *(lineSplit.end() - 1);
			istringstream(currLabelStr) >> currLabel;
			if (currLabel == classLabel)
			{
				classLines.push_back(line);
				continue;
			}
			else
			{
				allLinesByClass.push_back(classLines);
				classLines.clear();
				classLines.push_back(line);
				classLabel = currLabel;
			}
		}
		allLinesByClass.push_back(classLines);

		for (int i = 0; i < operatingTask.foldNum; ++i)
		{
			string num; 
			int currNum = i + 1;
			ostringstream(num) << currNum;
			string trainFileName = operatingTask.outputDirName + "/train_" + num + ".txt";
			string valFileName = operatingTask.outputDirName + "/val_" + num + ".txt";
			ofstream outTrain(trainFileName.c_str());
			ofstream outVal(valFileName.c_str());

			for (int j = 0; j < allLinesByClass.size(); ++j)
			{
				int currClassLinesNum = allLinesByClass[j].size();
				vector<string>::iterator start = allLinesByClass[j].begin() + (currClassLinesNum / operatingTask.foldNum) * i;
				vector<string>::iterator end = start + (currClassLinesNum / operatingTask.foldNum);
				if ((end - allLinesByClass[j].begin()) >= (allLinesByClass[j].end() - 1 - allLinesByClass[j].begin())) 
					end = allLinesByClass[j].end();

				for (vector<string>::iterator it = allLinesByClass[j].begin(); it != start; ++it) outTrain << *it << endl;
				for (vector<string>::iterator it = start; it != end; ++it) outVal << *it << endl;
				for (vector<string>::iterator it = end; it != allLinesByClass[j].end(); ++it) outTrain << *it << endl;
			}

			double processedPortion = double(i + 1) / double(operatingTask.foldNum);
			int percentageNum = int(processedPortion * 100);
			emit progressBarReporter("Creating Lists..  ", percentageNum);

			outTrain.close();
			outVal.close();
		}
	}
}

void Operator::createListFromPatch(double ratio)
{
	if (ratio == 0)
	{ }
	else
	{
		string patchDirString;
		const char* patchDir;
		DIR* dir;
		struct dirent* ent;

		string outTrainTxt, outValTxt;
		string valPatchFullName, trainPatchFullName;
		QString patchPath;
		if (operatingTask.patchOp == stackTo2D)
		{
			patchDirString = operatingTask.source + "/patches";
			patchDir = patchDirString.c_str();
			outTrainTxt = operatingTask.outputDirName + "/patches/train.txt";
			outValTxt = operatingTask.outputDirName + "/patches/val.txt";

			patchPath = QString::fromStdString(patchDirString);
		}	

		QDir sourceDir(patchPath);
		sourceDir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
		int patchCount = sourceDir.count();
		srand(time(NULL));
		size_t neededNum = operatingTask.subsetRatio * double(patchCount);
		size_t pickedCount = 0;
		vector<size_t> pickedPatchNo;
		while (pickedCount <= neededNum)
		{
			size_t num = rand() % neededNum + 1;
			pickedPatchNo.push_back(num);
			++pickedCount;
		}

		ofstream outTrain(outTrainTxt.c_str());
		ofstream outVal(outValTxt.c_str());
		if ((dir = opendir(patchDir)) != NULL)
		{
			QString patchName;
			size_t patchNum = 1;
			bool val = false;
			while ((ent = readdir(dir)) != NULL)
			{
				for (size_t i = 0; i < 30; ++i)
				{
					if (ent->d_name[i] == NULL) break;
					patchName = patchName + QChar(ent->d_name[i]);
				}
				if (patchName == "." || patchName == ".." || patchName == "..." || patchName == "train.txt" || patchName == "val.txt") 
					continue;

				for (vector<size_t>::iterator it = pickedPatchNo.begin(); it != pickedPatchNo.end(); ++it)
				{
					if (patchNum == *it)
					{
						val = true;
						break;
					}
				}
				if (val == true)
				{
					if (operatingTask.patchOp == stackTo2D)
						valPatchFullName = patchDirString + "/"+ patchName.toStdString();
					outVal << valPatchFullName << endl;
					++patchNum;
					val = false;
				}
				else
				{
					if (operatingTask.patchOp == stackTo2D)
						trainPatchFullName = patchDirString + "/" + patchName.toStdString();
					outTrain << trainPatchFullName << endl;
					++patchNum;
				}
				patchName.clear();

				double processedPortion = double(patchNum) / double(patchCount);
				int percentageNum = int(processedPortion * 100);
				emit progressBarReporter("Creating Lists..  ", percentageNum);
			}
		}
		closedir(dir);
	}
}

void Operator::create2DPatches(patchOpType patchOp)
{
	if (patchOp == stackTo2D)
	{
		QString inputNeuronFileQString = QString::fromStdString(operatingTask.neuronStrucFileName);
		QFileInfo checkFileOrPath(inputNeuronFileQString);
		int xRadius = operatingTask.sideX / 2;
		int yRadius = operatingTask.sideY / 2;
		int zRadius = operatingTask.sideZ / 2;

		if (checkFileOrPath.isFile()) // single stack/structure operation
		{
			unsigned char* ImgPtr = 0;
			V3DLONG in_sz[4];
			int datatype;
			if (!simple_loadimage_wrapper(*OperatorCallback, operatingTask.source.c_str(), ImgPtr, in_sz, datatype))
			{
				cerr << "Error reading image file [" << operatingTask.source << "]. Exit." << endl;
				return;
			}
			int imgX = in_sz[0];
			int imgY = in_sz[1];
			int imgZ = in_sz[2];
			int channel = in_sz[3];

			NeuronStructNavigator nodeProcessor(xRadius, yRadius, zRadius, imgX, imgY, imgZ);
			nodeProcessor.neuronStructFileName = inputNeuronFileQString;
			nodeProcessor.generateNodeQueue();
			double nodeNum = nodeProcessor.nodeQueue.size();
			cout << "Node number: " << nodeNum << endl;
			
			funcSequencer(operatingTask.opSeq); // create function pointer sequence
			
			double nodeCount = 0;
			while (!nodeProcessor.nodeQueue.empty())
			{
				nodeInfo currNode = nodeProcessor.nodeQueue.front(); 

				V3DLONG VOIxyz[4];
				VOIxyz[0] = currNode.xhb - currNode.xlb + 1;
				VOIxyz[1] = currNode.yhb - currNode.ylb + 1;
				VOIxyz[2] = currNode.zhb - currNode.zlb + 1;
				VOIxyz[3] = channel;
				V3DLONG VOIsz = VOIxyz[0] * VOIxyz[1] * VOIxyz[2];
				unsigned char* VOIPtr = new unsigned char[VOIsz];

				V3DLONG ROIxyz[4];
				ROIxyz[0] = currNode.xhb - currNode.xlb + 1;
				ROIxyz[1] = currNode.yhb - currNode.ylb + 1;
				ROIxyz[2] = 1;
				ROIxyz[3] = channel;
				V3DLONG ROIsz = VOIxyz[0] * VOIxyz[1];
				unsigned char* ROIPtr = new unsigned char[ROIsz];

				for (vector<opPtr>::iterator opIt = this->opFuncSeq.begin(); opIt != this->opFuncSeq.end(); ++opIt) 
					(this->**opIt)(ImgPtr, VOIPtr, ROIPtr, currNode.xlb, currNode.xhb, currNode.ylb, currNode.yhb, currNode.zlb, currNode.zhb, imgX, imgY, imgZ);
				
				QString patchPath = QString::fromStdString(operatingTask.outputDirName) + "/patches";
				if (!QDir(patchPath).exists()) QDir().mkpath(patchPath);
				if (operatingTask.dimSelection == xy)
				{
					QString outimg_file = patchPath + QString("/x%1_y%2.tif").arg(currNode.nameX).arg(currNode.nameY);
					string filename = outimg_file.toStdString();
					const char* filenameptr = filename.c_str();
					simple_saveimage_wrapper(*OperatorCallback, filenameptr, ROIPtr, ROIxyz, 1);
				}
				delete[] VOIPtr;
				delete[] ROIPtr;
				
				nodeProcessor.nodeQueue.pop();
				++nodeCount;

				double processedPortion = nodeCount / nodeNum;
				int percentageNum = int(processedPortion * 100);
				emit progressBarReporter("Creating Patches..  ", percentageNum);
			}
			delete[] ImgPtr;
			
			return;
		}
		else 
		{ }
	}
	else if (patchOp == teraTo2D)
	{ 
		QString inputNeuronFileQString = QString::fromStdString(operatingTask.neuronStrucFileName);
		int xRadius = operatingTask.sideX / 2;
		int yRadius = operatingTask.sideY / 2;
		int zRadius = operatingTask.sideZ / 2;

		this->teraStack = VirtualVolume::instance(operatingTask.source.c_str());
		int imgX = this->teraStack->getDIM_H();
		int imgY = this->teraStack->getDIM_V();
		int imgZ = this->teraStack->getDIM_D();
		int channel = this->teraStack->getDIM_C();

		NeuronStructNavigator nodeProcessor(xRadius, yRadius, zRadius, imgX, imgY, imgZ);
		nodeProcessor.neuronStructFileName = inputNeuronFileQString;
		nodeProcessor.generateNodeQueue();
		double nodeNum = nodeProcessor.nodeQueue.size();
		cout << "Node number: " << nodeNum << endl;

		if (operatingTask.opSeq[0] != Crop)
		{
			cerr << "Terafly image needs to be cropped first. Doing nothing." << endl;
			return;
		}
		else operatingTask.opSeq.erase(operatingTask.opSeq.begin());

		funcSequencer(operatingTask.opSeq); // create function pointer sequence
		
		double nodeCount = 0;
		while (!nodeProcessor.nodeQueue.empty())
		{
			nodeInfo currNode = nodeProcessor.nodeQueue.front();

			V3DLONG VOIxyz[4];
			VOIxyz[0] = currNode.xhb - currNode.xlb + 1;
			VOIxyz[1] = currNode.yhb - currNode.ylb + 1;
			VOIxyz[2] = currNode.zhb - currNode.zlb + 1;
			VOIxyz[3] = channel;
			V3DLONG VOIsz = VOIxyz[0] * VOIxyz[1] * VOIxyz[2];
			unsigned char* VOIPtr = new unsigned char[VOIsz];

			V3DLONG ROIxyz[4];
			ROIxyz[0] = currNode.xhb - currNode.xlb + 1;
			ROIxyz[1] = currNode.yhb - currNode.ylb + 1;
			ROIxyz[2] = 1;
			ROIxyz[3] = channel;
			V3DLONG ROIsz = VOIxyz[0] * VOIxyz[1];
			unsigned char* ROIPtr = new unsigned char[ROIsz];

			VOIPtr = this->teraStack->loadSubvolume_to_UINT8(currNode.ylb, currNode.yhb + 1, currNode.xlb, currNode.xhb + 1, currNode.zlb, currNode.zhb + 1);
			for (vector<opPtr>::iterator opIt = this->opFuncSeq.begin(); opIt != this->opFuncSeq.end(); ++opIt)
				(this->**opIt)(VOIPtr, VOIPtr, ROIPtr, currNode.xlb, currNode.xhb, currNode.ylb, currNode.yhb, currNode.zlb, currNode.zhb, imgX, imgY, imgZ);

			QString patchPath = QString::fromStdString(operatingTask.outputDirName) + "/terafly_patches_2D";
			if (!QDir(patchPath).exists()) QDir().mkpath(patchPath);
			QString outimg_file = patchPath + QString("/x%1_y%2.tif").arg(currNode.nameX).arg(currNode.nameY);
			string filename = outimg_file.toStdString();
			const char* filenameptr = filename.c_str();
			simple_saveimage_wrapper(*OperatorCallback, filenameptr, ROIPtr, ROIxyz, 1);
			delete[] VOIPtr;
			delete[] ROIPtr;
			
			nodeProcessor.nodeQueue.pop();
			++nodeCount;

			double processedPortion = nodeCount / nodeNum;
			int percentageNum = int(processedPortion * 100);
			emit progressBarReporter("Creating Patches..  ", percentageNum);
		}
		delete[] this->teraStack;
	}
}

void Operator::create3DPatches(patchOpType patchOp)
{
	if (patchOp == stackTo3D)
	{
		QString inputNeuronFileQString = QString::fromStdString(operatingTask.neuronStrucFileName);
		QFileInfo checkFileOrPath(inputNeuronFileQString);
		int xRadius = operatingTask.sideX / 2;
		int yRadius = operatingTask.sideY / 2;
		int zRadius = operatingTask.sideZ / 2;

		if (checkFileOrPath.isFile()) // single stack/structure operation
		{
			unsigned char* ImgPtr = 0;
			V3DLONG in_sz[4];
			int datatype;
			if (!simple_loadimage_wrapper(*OperatorCallback, operatingTask.source.c_str(), ImgPtr, in_sz, datatype))
			{
				cerr << "Error reading image file [" << operatingTask.source << "]. Exit." << endl;
				return;
			}
			int imgX = in_sz[0];
			int imgY = in_sz[1];
			int imgZ = in_sz[2];
			int channel = in_sz[3];

			NeuronStructNavigator nodeProcessor(xRadius, yRadius, zRadius, imgX, imgY, imgZ);
			nodeProcessor.neuronStructFileName = inputNeuronFileQString;
			nodeProcessor.generateNodeQueue();
			double nodeNum = nodeProcessor.nodeQueue.size();
			cout << "Node number: " << nodeNum << endl;

			funcSequencer(operatingTask.opSeq); // create function pointer sequence

			double nodeCount = 0;
			while (!nodeProcessor.nodeQueue.empty())
			{
				nodeInfo currNode = nodeProcessor.nodeQueue.front();

				V3DLONG VOIxyz[4];
				VOIxyz[0] = currNode.xhb - currNode.xlb + 1;
				VOIxyz[1] = currNode.yhb - currNode.ylb + 1;
				VOIxyz[2] = currNode.zhb - currNode.zlb + 1;
				VOIxyz[3] = channel;
				V3DLONG VOIsz = VOIxyz[0] * VOIxyz[1] * VOIxyz[2];
				unsigned char* VOIPtr = new unsigned char[VOIsz];

				V3DLONG ROIxyz[4];
				ROIxyz[0] = currNode.xhb - currNode.xlb + 1;
				ROIxyz[1] = currNode.yhb - currNode.ylb + 1;
				ROIxyz[2] = currNode.zhb - currNode.zlb + 1;
				ROIxyz[3] = channel;
				V3DLONG ROIsz = VOIxyz[0] * VOIxyz[1];
				unsigned char* ROIPtr = new unsigned char[ROIsz];

				for (vector<opPtr>::iterator opIt = opFuncSeq.begin(); opIt != opFuncSeq.end(); ++opIt)
					(this->**opIt)(ImgPtr, VOIPtr, ROIPtr, currNode.xlb, currNode.xhb, currNode.ylb, currNode.yhb, currNode.zlb, currNode.zhb, imgX, imgY, imgZ);

				QString patchPath = QString::fromStdString(operatingTask.outputDirName) + "/patches_3D";
				if (!QDir(patchPath).exists()) QDir().mkpath(patchPath);
				QString outimg_file = patchPath + QString("/x%1_y%2_z%3.v3draw").arg(currNode.nameX).arg(currNode.nameY).arg(currNode.nameZ);
				string filename = outimg_file.toStdString();
				const char* filenameptr = filename.c_str();
				simple_saveimage_wrapper(*OperatorCallback, filenameptr, VOIPtr, VOIxyz, 1);
				delete[] VOIPtr;
				delete[] ROIPtr;
				
				nodeProcessor.nodeQueue.pop();
				++nodeCount;

				double processedPortion = nodeCount / nodeNum;
				int percentageNum = int(processedPortion * 100);
				emit progressBarReporter("Creating Patches..  ", percentageNum);
			}
			delete[] ImgPtr;

			return;
		}
		else
		{ }
	}
	else if (patchOp == teraTo3D)
	{
		QString inputNeuronFileQString = QString::fromStdString(operatingTask.neuronStrucFileName);
		int xRadius = operatingTask.sideX / 2;
		int yRadius = operatingTask.sideY / 2;
		int zRadius = operatingTask.sideZ / 2;

		this->teraStack = VirtualVolume::instance(operatingTask.source.c_str());
		int imgX = this->teraStack->getDIM_H();
		int imgY = this->teraStack->getDIM_V();
		int imgZ = this->teraStack->getDIM_D();
		int channel = this->teraStack->getDIM_C();

		NeuronStructNavigator nodeProcessor(xRadius, yRadius, zRadius, imgX, imgY, imgZ);
		nodeProcessor.neuronStructFileName = inputNeuronFileQString;
		nodeProcessor.generateNodeQueue();
		double nodeNum = nodeProcessor.nodeQueue.size();
		cout << "Node number: " << nodeNum << endl;

		if (operatingTask.opSeq[0] != Crop)
		{
			cerr << "Terafly image needs to be cropped first. Doing nothing." << endl;
			return;
		}
		else operatingTask.opSeq.erase(operatingTask.opSeq.begin());

		funcSequencer(operatingTask.opSeq); // create function pointer sequence

		double nodeCount = 0;
		while (!nodeProcessor.nodeQueue.empty())
		{
			nodeInfo currNode = nodeProcessor.nodeQueue.front();

			V3DLONG VOIxyz[4];
			VOIxyz[0] = currNode.xhb - currNode.xlb + 1;
			VOIxyz[1] = currNode.yhb - currNode.ylb + 1;
			VOIxyz[2] = currNode.zhb - currNode.zlb + 1;
			VOIxyz[3] = channel;
			V3DLONG VOIsz = VOIxyz[0] * VOIxyz[1] * VOIxyz[2];
			unsigned char* VOIPtr = new unsigned char[VOIsz];

			V3DLONG ROIxyz[4];
			ROIxyz[0] = currNode.xhb - currNode.xlb + 1;
			ROIxyz[1] = currNode.yhb - currNode.ylb + 1;
			ROIxyz[2] = currNode.zhb - currNode.zlb + 1;
			ROIxyz[3] = channel;
			V3DLONG ROIsz = VOIxyz[0] * VOIxyz[1];
			unsigned char* ROIPtr = new unsigned char[ROIsz];

			VOIPtr = this->teraStack->loadSubvolume_to_UINT8(currNode.ylb, currNode.yhb + 1, currNode.xlb, currNode.xhb + 1, currNode.zlb, currNode.zhb + 1);

			QString patchPath = QString::fromStdString(operatingTask.outputDirName) + "/terafly_patches_3D";
			if (!QDir(patchPath).exists()) QDir().mkpath(patchPath);
			QString outimg_file = patchPath + QString("/x%1_y%2_z%3.v3draw").arg(currNode.nameX).arg(currNode.nameY).arg(currNode.nameZ);
			string filename = outimg_file.toStdString();
			const char* filenameptr = filename.c_str();
			simple_saveimage_wrapper(*OperatorCallback, filenameptr, VOIPtr, VOIxyz, 1);
			delete[] VOIPtr;
			delete[] ROIPtr;

			nodeProcessor.nodeQueue.pop();
			++nodeCount;

			double processedPortion = nodeCount / nodeNum;
			int percentageNum = int(processedPortion * 100);
			emit progressBarReporter("Creating Patches..  ", percentageNum);
		}
		delete[] this->teraStack;
	}
}

void Operator::cropStack(unsigned char InputImagePtr[], unsigned char OutputImagePtr[], unsigned char dummiePtr[],
	int xlb, int xhb, int ylb, int yhb, int zlb, int zhb, int imgX, int imgY, int imgZ)
{
	V3DLONG OutputArrayi = 0;
	for (V3DLONG zi = zlb; zi <= zhb; ++zi)
	{
		for (V3DLONG yi = ylb; yi <= yhb; ++yi)
		{
			for (V3DLONG xi = xlb; xi <= xhb; ++xi)
			{
				OutputImagePtr[OutputArrayi] = InputImagePtr[imgX*imgY*(zi - 1) + imgX*(yi - 1) + (xi - 1)];
				++OutputArrayi;
			}
		}
	}
}

void Operator::maxIPStack(unsigned char InputImagePtr[], unsigned char inputVOIPtr[], unsigned char OutputImage2DPtr[],
	int xlb, int xhb, int ylb, int yhb, int zlb, int zhb, int imgX, int imgY, int imgZ)
{
	V3DLONG OutputArrayi = 0;
	int xDim = xhb - xlb + 1;
	int yDim = yhb - ylb + 1;
	int zDim = zhb - zlb + 1;
	if (operatingTask.dimSelection == xy)
	{
		for (int yi = 0; yi < yDim; ++yi)
		{
			for (int xi = 0; xi < xDim; ++xi)
			{
				short int maxVal = 0;
				for (int zi = 0; zi < zDim; ++zi)
				{
					short int curValue = inputVOIPtr[xDim*yDim*zi + xDim*yi + xi];
					if (curValue > maxVal) maxVal = curValue;
				}
				OutputImage2DPtr[xDim*yi + xi] = (unsigned char)(maxVal);
			}
		}
	}
}

void Operator::minIPStack(unsigned char InputImagePtr[], unsigned char inputVOIPtr[], unsigned char OutputImage2DPtr[],
	int xlb, int xhb, int ylb, int yhb, int zlb, int zhb, int imgX, int imgY, int imgZ)
{
	V3DLONG OutputArrayi = 0;
	int xDim = xhb - xlb + 1;
	int yDim = yhb - ylb + 1;
	int zDim = zhb - zlb + 1;
	if (operatingTask.dimSelection == xy)
	{
		for (int yi = 0; yi < yDim; ++yi)
		{
			for (int xi = 0; xi < xDim; ++xi)
			{
				short int minVal = 255;
				for (int zi = 0; zi < zDim; ++zi)
				{
					short int curValue = inputVOIPtr[xDim*yDim*zi + xDim*yi + xi];
					if (curValue < minVal) minVal = curValue;
				}
				OutputImage2DPtr[xDim*yi + xi] = (unsigned char)(minVal);
			}
		}
	}
}

void Operator::funcSequencer(vector<opSequence> uiOpSeq)
{
	opPtr currOpPtr = NULL;
	for (vector<opSequence>::iterator seqIt = uiOpSeq.begin(); seqIt != uiOpSeq.end(); ++seqIt)
	{
		switch (*seqIt)
		{
		case Crop:
			currOpPtr = &Operator::cropStack;
			this->opFuncSeq.push_back(currOpPtr);
			currOpPtr = NULL;
			break;
		case MIP:
			currOpPtr = &Operator::maxIPStack;
			this->opFuncSeq.push_back(currOpPtr);
			currOpPtr = NULL;
			break;
		case mIP:
			currOpPtr = &Operator::minIPStack;
			this->opFuncSeq.push_back(currOpPtr);
			currOpPtr = NULL;
			break;
		}
	}
}


NeuronTree Operator::cropSWCfile3D(NeuronTree nt, int xb, int xe, int yb, int ye, int zb, int ze, int type)
{
	//NeutronTree structure
	NeuronTree nt_cut;
	QList<NeuronSWC> listNeuron;
	QHash<int, int>  hashNeuron;
	listNeuron.clear();
	hashNeuron.clear();

	//set node
	QList<NeuronSWC> list = nt.listNeuron;
	NeuronSWC S;
	for (int i = 0; i<list.size(); i++)
	{
		NeuronSWC curr = list.at(i);
		if (curr.x <= xe && curr.x >= xb && curr.y <= ye && curr.y >= yb && curr.z <= ze && curr.z >= zb && (type == -1 || curr.type == type))
		{
			S.x = curr.x - xb;
			S.y = curr.y - yb;
			S.z = curr.z - zb;
			S.n = curr.n;
			S.type = curr.type;
			S.r = curr.r;
			S.pn = curr.pn;
			listNeuron.append(S);
			hashNeuron.insert(S.n, listNeuron.size() - 1);
		}
	}
	nt_cut.listNeuron = listNeuron;
	nt_cut.hashNeuron = hashNeuron;

	return nt_cut;
}

void Operator::getImageFolders()
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
			++imgFolderCount;

			imgFolderName = this->inputImagedir + "/" + imgFolderName;
			//qDebug() << imgFolderName << "\ntotal folder number: " << imgFolderCount;
			this->imageFolders.push_back(imgFolderName);
			imgFolderName.clear();
		}
	}
	closedir(dir);
}

void Operator::pick_save()
{
	QString imgName;

	for (QVector<QString>::iterator it = this->imageFolders.begin(); it != this->imageFolders.end(); ++it)
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
				for (size_t i = 0; i<30; ++i)
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
				for (size_t i = 0; i<30; ++i)
				{
					if (ent->d_name[i] == NULL) break;
					fileName = fileName + QChar(ent->d_name[i]);
				}

				if (fileName == "manual.swc_p.swc")
				{
					QString nodeX, nodeY, nodeZ;
					QString swcFullName = intoSWCFolder + "\\manual.swc_p.swc";
					NeuronTree inputSWCTree = readSWC_file(swcFullName);
					for (QList<NeuronSWC>::iterator neuronIt = inputSWCTree.listNeuron.begin(); neuronIt != inputSWCTree.listNeuron.end(); ++neuronIt)
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
									for (size_t i = 0; i<50; ++i)
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
