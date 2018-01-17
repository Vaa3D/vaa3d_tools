#include <dirent.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>

#include <qstringlist.h>
#include <qfile.h>
#include <qvector.h>
#include <qfileinfo.h>

#include <v3d_interface.h>
#include "DeepNeuronUI.h"
#include "classification.h"
#include "mean_shift_fun.h"
#include "ui_DeepNeuronForm.h"

using namespace std;

DLOperator::DLOperator(imgOperator* inputOperator)
{
	this->curImgOperator = inputOperator;
}

void DLOperator::predictSWCstroke(V3DPluginCallback2*& inputCallBack, v3dhandle curwin)
{

	clock_t startTime, endTime;
	startTime = clock();
	Classifier* classifierPtr = new Classifier(this->deployName, this->modelName, this->meanName);

	V3DLONG N = curImgOperator->imgX;
	V3DLONG M = curImgOperator->imgY;
	V3DLONG P = curImgOperator->imgZ;
	int Wx = 30, Wy = 30, Wz = 30;
	std::vector<cv::Mat> imgs;
	QList <NeuronTree> * nt_list = inputCallBack->getHandleNeuronTrees_3DGlobalViewer(curwin);
	if (nt_list->size() > 0)
	{
		for (V3DLONG i = 0; i<nt_list->size(); i++)
		{
			NeuronTree nt = nt_list->at(i);
			for (V3DLONG j = 0; j< nt.listNeuron.size(); j++)
			{
				V3DLONG tmpx = nt.listNeuron.at(j).x;
				V3DLONG tmpy = nt.listNeuron.at(j).y;
				V3DLONG tmpz = nt.listNeuron.at(j).z;

				V3DLONG xb = tmpx - Wx; if (xb<0) xb = 0; if (xb >= N - 1) xb = N - 1;
				V3DLONG xe = tmpx + Wx; if (xe >= N - 1) xe = N - 1;
				V3DLONG yb = tmpy - Wy; if (yb<0) yb = 0; if (yb >= M - 1) yb = M - 1;
				V3DLONG ye = tmpy + Wy; if (ye >= M - 1) ye = M - 1;
				V3DLONG zb = tmpz - Wz; if (zb<0) zb = 0; if (zb >= P - 1) zb = P - 1;
				V3DLONG ze = tmpz + Wz; if (ze >= P - 1) ze = P - 1;

				V3DLONG im_cropped_sz[4];
				im_cropped_sz[0] = xe - xb + 1;
				im_cropped_sz[1] = ye - yb + 1;
				im_cropped_sz[2] = 1;
				im_cropped_sz[3] = curImgOperator->channel;

				unsigned char *im_cropped = 0;

				V3DLONG pagesz = im_cropped_sz[0] * im_cropped_sz[1] * im_cropped_sz[2] * im_cropped_sz[3];
				try { im_cropped = new unsigned char[pagesz]; }
				catch (...)  { v3d_msg("cannot allocate memory for im_cropped."); return; }
				memset(im_cropped, 0, sizeof(unsigned char)*pagesz);

				for (V3DLONG iz = zb; iz <= ze; iz++)
				{
					V3DLONG offsetk = iz*M*N;
					V3DLONG j = 0;
					for (V3DLONG iy = yb; iy <= ye; iy++)
					{
						V3DLONG offsetj = iy*N;
						for (V3DLONG ix = xb; ix <= xe; ix++)
						{
							if (curImgOperator->ImgPtr[offsetk + offsetj + ix] >= im_cropped[j])
								im_cropped[j] = curImgOperator->ImgPtr[offsetk + offsetj + ix];
							j++;
						}
					}
				}

				cv::Mat img(im_cropped_sz[1], im_cropped_sz[0], CV_8UC1, im_cropped);
				imgs.push_back(img);

				double processedPortion = double(j) / double(nt.listNeuron.size());
				int percentageNum = int(processedPortion * 100);
				emit progressBarReporter("Classifying nodes from input stroke.. ", percentageNum);
			}
			std::vector<std::vector<float> > outputs = classifierPtr->Predict(imgs);
			for (V3DLONG j = 0; j< nt.listNeuron.size(); j++)
			{
				std::vector<float> output = outputs[j];

				if (output.at(1) > output.at(0) && output.at(1) > output.at(2))
				{
					nt.listNeuron[j].type = 2;
				}
				else if (output.at(2) > output.at(0) && output.at(2) > output.at(1))
				{
					nt.listNeuron[j].type = 3;
				}
				else
				{
					nt.listNeuron[j].type = 0;
				}
			}

			nt.color.r = 0;
			nt.color.g = 0;
			nt.color.b = 0;
			nt.color.a = 0;
			nt_list->removeAt(i);
			nt_list->push_back(nt);
			inputCallBack->setSWC(curwin, nt);
			imgs.clear();
		}
	}
	inputCallBack->update_NeuronBoundingBox(inputCallBack->find3DViewerByName(inputCallBack->getImageName(curwin)));
	delete classifierPtr;
	emit progressBarReporter("Classification done.", 100);

	endTime = clock();
	double totalTime = double(endTime - startTime) / CLOCKS_PER_SEC;
	cout << "time elapsed: " << totalTime << " secs" << endl;
}