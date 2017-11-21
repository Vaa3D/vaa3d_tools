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
			if (taskQueu.front().listOp == subset)
			{
				emit progressBarReporter("Creating List..  ", 0);
				createListFromList(subset);
			}
			else if (taskQueu.front().listOp == crossVal)
			{
				emit progressBarReporter("Creating Lists..  ", 0);
				createListFromList(crossVal);
			}
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

void cropping3D(V3DPluginCallback2 &callback,
	Image4DSimple* p4DImage,
	NeuronTree nt,
	QString outputfolder,
	V3DLONG *in_sz,
	int Wx,
	int Wy,
	int Wz,
	int type,
	int offset)
{
	V3DLONG N = in_sz[0];
	V3DLONG M = in_sz[1];
	V3DLONG P = in_sz[2];
	V3DLONG sc = in_sz[3];

	/*VirtualVolume* data1d;

	for (int i = 0; i<nt.listNeuron.size(); i++)
	{
		if (type == -1 || nt.listNeuron.at(i).type == type)
		{
			V3DLONG tmpx = nt.listNeuron.at(i).x;
			V3DLONG tmpy = nt.listNeuron.at(i).y;
			V3DLONG tmpz = nt.listNeuron.at(i).z;

			if (tmpx >= 0 && tmpx <= N - 1 && tmpy >= 0 && tmpy <= M - 1 && tmpz >= 0 && tmpz <= P - 1)
			{

				V3DLONG xb = tmpx - 1 - Wx; if (xb<0) xb = 0; if (xb >= N - 1) xb = N - 1;
				V3DLONG xe = tmpx - 1 + Wx; if (xe >= N - 1) xe = N - 1;
				V3DLONG yb = tmpy - 1 - Wy; if (yb<0) yb = 0; if (yb >= M - 1) yb = M - 1;
				V3DLONG ye = tmpy - 1 + Wy; if (ye >= M - 1) ye = M - 1;
				V3DLONG zb = tmpz - 1 - Wz; if (zb<0) zb = 0; if (zb >= P - 1) zb = P - 1;
				V3DLONG ze = tmpz - 1 + Wz; if (ze >= P - 1) ze = P - 1;

				QString outimg_file = outputfolder + QString("x%1_y%2_z%3.tif").arg(tmpx).arg(tmpy).arg(tmpz);
				QString outimg_file_swc = outputfolder + QString("x%1_y%2_z%3.swc").arg(tmpx).arg(tmpy).arg(tmpz);
				QString outimg_file_linker = outputfolder + QString("x%1_y%2_z%3.ano").arg(tmpx).arg(tmpy).arg(tmpz);
				int p = 1;

				while (QFile(outimg_file).exists())
				{
					outimg_file = outimg_file + QString("_%1.tif").arg(p);
					outimg_file_swc = outimg_file_swc + QString("_%1.swc").arg(p);
					outimg_file_linker = outimg_file_linker + QString("_%1.ano").arg(p);
					p++;
				}


				NeuronTree nt_cropped;
				if (P>1)
					nt_cropped = cropSWCfile3D(nt, xb, xe, yb, ye, zb, ze, type);
				else
					nt_cropped = cropSWCfile(nt, xb, xe, yb, ye, type);

				NeuronTree nt_sort;
				if (nt_cropped.listNeuron.size()>0)
				{
					nt_sort = SortSWC_pipeline(nt_cropped.listNeuron, nt_cropped.listNeuron.at(0).n, 0);
				}
				else
					nt_sort = nt_cropped;

				writeSWC_file(outimg_file_swc, nt_sort);

				if (offset)
				{
					QString outimg_file_swc_offset = outimg_file_swc + "_offset.swc";
					for (V3DLONG ii = 0; ii < nt_sort.listNeuron.size(); ii++)
					{
						nt_sort.listNeuron[ii].x += xb;
						nt_sort.listNeuron[ii].y += yb;
						nt_sort.listNeuron[ii].z += zb;
					}
					writeSWC_file(outimg_file_swc_offset, nt_sort);
				}


				V3DLONG im_cropped_sz[4];
				im_cropped_sz[0] = xe - xb + 1;
				im_cropped_sz[1] = ye - yb + 1;
				im_cropped_sz[2] = ze - zb + 1;
				im_cropped_sz[3] = sc;


				unsigned char *im_cropped = 0;
				if (p4DImage->getDatatype())
				{
					V3DLONG pagesz = im_cropped_sz[0] * im_cropped_sz[1] * im_cropped_sz[2] * im_cropped_sz[3];
					try { im_cropped = new unsigned char[pagesz]; }
					catch (...)  { v3d_msg("cannot allocate memory for im_cropped."); return; }
					V3DLONG j = 0;
					for (V3DLONG iz = zb; iz <= ze; iz++)
					{
						V3DLONG offsetk = iz*M*N;
						for (V3DLONG iy = yb; iy <= ye; iy++)
						{
							V3DLONG offsetj = iy*N;
							for (V3DLONG ix = xb; ix <= xe; ix++)
							{
								im_cropped[j] = p4DImage->getRawData()[offsetk + offsetj + ix];
								j++;
							}
						}
					}
				}
				else
				{
					data1d = VirtualVolume::instance(p4DImage->getFileName());
					im_cropped = data1d->loadSubvolume_to_UINT8(yb, ye + 1, xb, xe + 1, zb, ze + 1);
				}

				simple_saveimage_wrapper(callback, outimg_file.toStdString().c_str(), (unsigned char *)im_cropped, im_cropped_sz, 1);
				QFile qf_anofile(outimg_file_linker);
				if (!qf_anofile.open(QIODevice::WriteOnly))
				{
					v3d_msg("Cannot open file for writing!", 0);
					return;
				}

				QTextStream out(&qf_anofile);
				out << "RAWIMG= " << outimg_file.toStdString().c_str() << endl;
				out << "SWCFILE= " << outimg_file_swc.toStdString().c_str() << endl;

				if (im_cropped) { delete[]im_cropped; im_cropped = 0; }
				if (data1d) { delete data1d; data1d = 0; }
			}

		}

	}*/
	return;

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