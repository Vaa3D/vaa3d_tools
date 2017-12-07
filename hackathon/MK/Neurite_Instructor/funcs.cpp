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

#include "Neurite_Instructor_plugin.h"
#include "neuriteinstructorui.h"
#include <v3d_interface.h>
#include "classification.h"
#include "mean_shift_fun.h"
#include "ui_Neurite_Instructor.h"

using namespace std;

void cropStack(unsigned char InputImagePtr[], unsigned char OutputImagePtr[],
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

void maxIPStack(unsigned char inputVOIPtr[], unsigned char OutputImage2DPtr[],
    int xlb, int xhb, int ylb, int yhb, int zlb, int zhb)
{
    int xDim = xhb - xlb + 1;
    int yDim = yhb - ylb + 1;
    int zDim = zhb - zlb + 1;
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

/*QStringList importSeriesFileList_addnumbersort(const QString & curFilePath)
{
    QStringList myList;
    myList.clear();

    // get the image files namelist in the directory
    QStringList imgSuffix;
    imgSuffix<<"*.tif"<<"*.raw"<<"*.v3draw"<<"*.lsm"
            <<"*.TIF"<<"*.RAW"<<"*.V3DRAW"<<"*.LSM";

    QDir dir(curFilePath);
    if (!dir.exists())
    {
        qWarning("Cannot find the directory");
        return myList;
    }

    foreach (QString file, dir.entryList(imgSuffix, QDir::Files, QDir::Name))
    {
        myList += QFileInfo(dir, file).absoluteFilePath();
    }

    // print filenames
    //foreach (QString qs, myList)  qDebug() << qs;

    return myList;
}*/

void predictSWCstroke(neuriteInstructorUI* inputFormPtr, V3DPluginCallback2* inputCallBack, v3dhandle curwin)
{

    clock_t startTime, endTime;
    startTime = clock();
    Classifier classifier(inputFormPtr->deployName, inputFormPtr->modelName, inputFormPtr->meanName);

    V3DLONG N = inputFormPtr->imgX;
    V3DLONG M = inputFormPtr->imgY;
    V3DLONG P = inputFormPtr->imgZ;
    int Wx = 30, Wy= 30, Wz=30;
    std::vector<cv::Mat> imgs;
    QList <NeuronTree> * nt_list = inputCallBack->getHandleNeuronTrees_3DGlobalViewer(curwin);
    if(nt_list->size() > 0)
    {
        for(V3DLONG i=0; i<nt_list->size();i++)
        {
            NeuronTree nt = nt_list->at(i);
            for(V3DLONG j=0; j< nt.listNeuron.size(); j++)
            {
                V3DLONG tmpx = nt.listNeuron.at(j).x;
                V3DLONG tmpy = nt.listNeuron.at(j).y;
                V3DLONG tmpz = nt.listNeuron.at(j).z;

                V3DLONG xb = tmpx-Wx; if(xb<0) xb = 0;if(xb>=N-1) xb = N-1;
                V3DLONG xe = tmpx+Wx; if(xe>=N-1) xe = N-1;
                V3DLONG yb = tmpy-Wy; if(yb<0) yb = 0;if(yb>=M-1) yb = M-1;
                V3DLONG ye = tmpy+Wy; if(ye>=M-1) ye = M-1;
                V3DLONG zb = tmpz-Wz; if(zb<0) zb = 0;if(zb>=P-1) zb = P-1;
                V3DLONG ze = tmpz+Wz; if(ze>=P-1) ze = P-1;

                V3DLONG im_cropped_sz[4];
                im_cropped_sz[0] = xe - xb + 1;
                im_cropped_sz[1] = ye - yb + 1;
                im_cropped_sz[2] = 1;
                im_cropped_sz[3] = inputFormPtr->channel;

                unsigned char *im_cropped = 0;

                V3DLONG pagesz = im_cropped_sz[0]* im_cropped_sz[1]* im_cropped_sz[2]*im_cropped_sz[3];
                try {im_cropped = new unsigned char [pagesz];}
                catch(...)  {v3d_msg("cannot allocate memory for im_cropped."); return;}
                memset(im_cropped, 0, sizeof(unsigned char)*pagesz);

                for(V3DLONG iz = zb; iz <= ze; iz++)
                {
                    V3DLONG offsetk = iz*M*N;
                    V3DLONG j = 0;
                    for(V3DLONG iy = yb; iy <= ye; iy++)
                    {
                        V3DLONG offsetj = iy*N;
                        for(V3DLONG ix = xb; ix <= xe; ix++)
                        {
                            if(inputFormPtr->ImgPtr[offsetk + offsetj + ix] >= im_cropped[j])
                                im_cropped[j] = inputFormPtr->ImgPtr[offsetk + offsetj + ix];
                            j++;
                        }
                    }
                }

                cv::Mat img(im_cropped_sz[1], im_cropped_sz[0], CV_8UC1, im_cropped);
                imgs.push_back(img);
            }
            std::vector<std::vector<float> > outputs = classifier.Predict(imgs);
            for(V3DLONG j=0; j< nt.listNeuron.size(); j++)
            {
                std::vector<float> output = outputs[j];

                if (output.at(1) > output.at(0)  &&  output.at(1) > output.at(2))
                {
                    nt.listNeuron[j].type = 2;
                }
                else if (output.at(2) > output.at(0)  &&  output.at(2) > output.at(1))
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
            inputCallBack->setSWC(curwin,nt);
            imgs.clear();
        }
    }
    inputCallBack->update_NeuronBoundingBox(inputCallBack->find3DViewerByName(inputFormPtr->p4DImage->getFileName()));

    endTime = clock();
    double totalTime = double(endTime - startTime) / CLOCKS_PER_SEC;
    cout << "time elapsed: " << totalTime << " secs" << endl;
}
