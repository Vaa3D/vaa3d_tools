#include <dirent.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <ctime>

#include <qstringlist.h>
#include <qfile.h>
#include <qvector.h>
#include <qfileinfo.h>

#include "Operator.h"

using namespace std;

void Operator::taskQueuDispatcher()
{
	while (!taskQueu.empty())
	{
		operatingTask = taskQueu.front();
		
		if (operatingTask.createPatch == true)
		{
			if (operatingTask.patchOp == stackTo2D)
			{
				emit progressBarReporter("Creating Patches..", 0);
				create2DPatches(stackTo2D);
			}
			else if (operatingTask.patchOp == stackTo3D)
			{
				emit progressBarReporter("Creating Patches..", 0);
				create3DPatches(stackTo3D);
			}
		}

		if (operatingTask.createList == true)
		{
			if (operatingTask.listOp == subset)
			{
				emit progressBarReporter("Creating List..  ", 0);
				createListFromList(subset);
			}
			else if (operatingTask.listOp == crossVal)
			{
				emit progressBarReporter("Creating Lists..  ", 0);
				createListFromList(crossVal);
			}
		}

		if (operatingTask.createPatchNList == true)
		{ }

		taskQueu.pop();
	}
}

void Operator::createListFromList(listOpType listOp)
{
	if (listOp == subset)
	{
		ifstream inputFile_forSize(operatingTask.source);
		ifstream inputFile(operatingTask.source);
		ofstream outputFile(operatingTask.outputFileName);

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

		emit progressBarReporter("Complete. ", 100);

		inputFile.close();
		outputFile.close();
	}
	else if (listOp == crossVal)
	{
		ifstream inputFile(operatingTask.source);

		string line;
		vector<string> lineSplit;
		int classLabel = 0;
		vector<vector<string>> allLinesByClass;
		vector<string> classLines;
		while (getline(inputFile, line))
		{
			stringstream lineStream(line);
			string streamedLine;
			while (lineStream >> streamedLine) lineSplit.push_back(streamedLine);

			int currLabel = stoi(*(lineSplit.end() - 1));
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
			string num = to_string(i + 1);
			string trainFileName = operatingTask.outputDirName + "/train_" + num + ".txt";
			string valFileName = operatingTask.outputDirName + "/val_" + num + ".txt";
			ofstream outTrain(trainFileName);
			ofstream outVal(valFileName);

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
		emit progressBarReporter("Complete. ", 100);
	}
}

void Operator::create2DPatches(patchOpType patchOp)
{
	if (patchOp == stackTo2D)
	{
		QString swcQStringName = QString::fromStdString(operatingTask.neuronStrucFileName);
		QFileInfo checkFileOrPath(swcQStringName);
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

			NeuronTree inputSWC = readSWC_file(swcQStringName);
			int x_coord, y_coord, z_coord;
			int xlb, xhb, ylb, yhb, zlb, zhb;
			double nodeNum = inputSWC.listNeuron.size();
			
			// ------------------------- Create function pointer sequence 
			vector<opPtr> opFuncSeq;
			opPtr currOpPtr = NULL;	
			for (vector<opSequence>::iterator seqIt = operatingTask.opSeq.begin(); seqIt != operatingTask.opSeq.end(); ++seqIt)
			{
				switch (*seqIt)
				{
				case Crop:
					currOpPtr = &Operator::cropStack;
					opFuncSeq.push_back(currOpPtr);
					currOpPtr = NULL;
					break;
				case MIP:
					currOpPtr = &Operator::maxIPStack;
					opFuncSeq.push_back(currOpPtr);
					currOpPtr = NULL;
					break;
				case mIP:
					currOpPtr = &Operator::minIPStack;
					opFuncSeq.push_back(currOpPtr);
					currOpPtr = NULL;
					break;
				}
			}
			// ---------------------------------------------------------
			
			for (QList<NeuronSWC>::iterator it = inputSWC.listNeuron.begin(); it != inputSWC.listNeuron.end(); ++it)
			{
				x_coord = it->x;
				y_coord = it->y;
				z_coord = it->z;
				xlb = x_coord - xRadius;
				xhb = x_coord + xRadius;
				ylb = y_coord - yRadius;
				yhb = y_coord + yRadius;
				zlb = z_coord - zRadius;
				zhb = z_coord + zRadius;
				if (xlb < 1) xlb = 1;
				if (xhb > imgX) xhb = imgX;
				if (ylb < 1) ylb = 1;
				if (yhb > imgY) yhb = imgY;
				if (zlb < 1) zlb = 1;
				if (zhb > imgZ) zhb = imgZ;

				/*NeuronTree patchSWC = cropSWCfile3D(inputSWC, xlb, xhb, ylb, yhb, zlb, zhb, -1);
				QString patchSWCFolder = QString::fromStdString(operatingTask.outputDirName) + "/patchSWCs";
				if (!QDir(patchSWCFolder).exists()) QDir().mkpath(patchSWCFolder);
				QString outimg_fileSWC = patchSWCFolder + QString("/x%1_y%2_z%3.swc").arg(x_coord).arg(y_coord).arg(z_coord);
				writeSWC_file(outimg_fileSWC, patchSWC);*/ 

				V3DLONG VOIxyz[4];
				VOIxyz[0] = xhb - xlb + 1;
				VOIxyz[1] = yhb - ylb + 1;
				VOIxyz[2] = zhb - zlb + 1;
				VOIxyz[3] = channel;
				V3DLONG VOIsz = VOIxyz[0] * VOIxyz[1] * VOIxyz[2];
				unsigned char* VOIPtr = new unsigned char[VOIsz];

				V3DLONG ROIxyz[4];
				ROIxyz[0] = xhb - xlb + 1;
				ROIxyz[1] = yhb - ylb + 1;
				ROIxyz[2] = 1;
				ROIxyz[3] = channel;
				V3DLONG ROIsz = VOIxyz[0] * VOIxyz[1];
				unsigned char* ROIPtr = new unsigned char[ROIsz];

				for (vector<opPtr>::iterator opIt = opFuncSeq.begin(); opIt != opFuncSeq.end(); ++opIt) 
					(this->**opIt)(ImgPtr, VOIPtr, ROIPtr, xlb, xhb, ylb, yhb, zlb, zhb, imgX, imgY, imgZ);
				
				QString patchPath = QString::fromStdString(operatingTask.outputDirName) + "/patches";
				if (!QDir(patchPath).exists()) QDir().mkpath(patchPath);
				if (operatingTask.dimSelection == xy)
				{
					QString outimg_file = patchPath + QString("/x%1_y%2.tif").arg(x_coord).arg(y_coord);
					string filename = outimg_file.toStdString();
					const char* filenameptr = filename.c_str();
					simple_saveimage_wrapper(*OperatorCallback, filenameptr, ROIPtr, ROIxyz, 1);
				}
				delete[] VOIPtr;
				delete[] ROIPtr;

				double processedPortion = size_t(it - inputSWC.listNeuron.begin() + 1) / nodeNum;
				int percentageNum = int(processedPortion * 100);
				emit progressBarReporter("Creating Patches..  ", percentageNum);
			}
			delete[] ImgPtr;
			emit progressBarReporter("Complete. ", 100);
			cout << "Patch generation complete." << endl;
			
			return;
		}
		else 
		{ }
	}
}

void Operator::create3DPatches(patchOpType patchOp)
{
	if (patchOp == stackTo3D)
	{
		QString swcQStringName = QString::fromStdString(operatingTask.neuronStrucFileName);
		QFileInfo checkFileOrPath(swcQStringName);
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

			NeuronTree inputSWC = readSWC_file(swcQStringName);
			int x_coord, y_coord, z_coord;
			int xlb, xhb, ylb, yhb, zlb, zhb;
			double nodeNum = inputSWC.listNeuron.size();

			// ------------------------- Create function pointer sequence 
			vector<opPtr> opFuncSeq;
			opPtr currOpPtr = NULL;
			for (vector<opSequence>::iterator seqIt = operatingTask.opSeq.begin(); seqIt != operatingTask.opSeq.end(); ++seqIt)
			{
				switch (*seqIt)
				{
				case Crop:
					currOpPtr = &Operator::cropStack;
					opFuncSeq.push_back(currOpPtr);
					currOpPtr = NULL;
					break;
				}
			}
			// ---------------------------------------------------------

			for (QList<NeuronSWC>::iterator it = inputSWC.listNeuron.begin(); it != inputSWC.listNeuron.end(); ++it)
			{
				x_coord = it->x;
				y_coord = it->y;
				z_coord = it->z;
				xlb = x_coord - xRadius;
				xhb = x_coord + xRadius;
				ylb = y_coord - yRadius;
				yhb = y_coord + yRadius;
				zlb = z_coord - zRadius;
				zhb = z_coord + zRadius;
				if (xlb < 1) xlb = 1;
				if (xhb > imgX) xhb = imgX;
				if (ylb < 1) ylb = 1;
				if (yhb > imgY) yhb = imgY;
				if (zlb < 1) zlb = 1;
				if (zhb > imgZ) zhb = imgZ;

				/*NeuronTree patchSWC = cropSWCfile3D(inputSWC, xlb, xhb, ylb, yhb, zlb, zhb, -1);
				QString patchSWCFolder = QString::fromStdString(operatingTask.outputDirName) + "/patchSWCs";
				if (!QDir(patchSWCFolder).exists()) QDir().mkpath(patchSWCFolder);
				QString outimg_fileSWC = patchSWCFolder + QString("/x%1_y%2_z%3.swc").arg(x_coord).arg(y_coord).arg(z_coord);
				writeSWC_file(outimg_fileSWC, patchSWC);*/

				V3DLONG VOIxyz[4];
				VOIxyz[0] = xhb - xlb + 1;
				VOIxyz[1] = yhb - ylb + 1;
				VOIxyz[2] = zhb - zlb + 1;
				VOIxyz[3] = channel;
				V3DLONG VOIsz = VOIxyz[0] * VOIxyz[1] * VOIxyz[2];
				unsigned char* VOIPtr = new unsigned char[VOIsz];

				V3DLONG ROIxyz[4];
				ROIxyz[0] = xhb - xlb + 1;
				ROIxyz[1] = yhb - ylb + 1;
				ROIxyz[2] = zhb - zlb + 1;
				ROIxyz[3] = channel;
				V3DLONG ROIsz = ROIxyz[0] * ROIxyz[1] * ROIxyz[2];
				unsigned char* ROIPtr = new unsigned char[ROIsz];

				for (vector<opPtr>::iterator opIt = opFuncSeq.begin(); opIt != opFuncSeq.end(); ++opIt)
					(this->**opIt)(ImgPtr, VOIPtr, ROIPtr, xlb, xhb, ylb, yhb, zlb, zhb, imgX, imgY, imgZ);

				QString patchPath = QString::fromStdString(operatingTask.outputDirName) + "/patches_3D";
				if (!QDir(patchPath).exists()) QDir().mkpath(patchPath);
				QString outimg_file = patchPath + QString("/x%1_y%2_z%3.v3draw").arg(x_coord).arg(y_coord).arg(z_coord);
				string filename = outimg_file.toStdString();
				const char* filenameptr = filename.c_str();
				simple_saveimage_wrapper(*OperatorCallback, filenameptr, VOIPtr, VOIxyz, 1);
				delete[] VOIPtr;
				delete[] ROIPtr;

				double processedPortion = size_t(it - inputSWC.listNeuron.begin() + 1) / nodeNum;
				int percentageNum = int(processedPortion * 100);
				emit progressBarReporter("Creating Patches..  ", percentageNum);
			}
			delete[] ImgPtr;
			emit progressBarReporter("Complete. ", 100);
			cout << "Patch generation complete." << endl;

			return;
		}
		else
		{ }
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
				OutputImage2DPtr[xDim*yi + xi] = unsigned char(maxVal);
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
				OutputImage2DPtr[xDim*yi + xi] = unsigned char(minVal);
			}
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
