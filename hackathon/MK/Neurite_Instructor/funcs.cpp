#include <dirent.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

#include <qstringlist.h>
#include <qfile.h>
#include <qvector.h>
#include <qfileinfo.h>

#include "Neurite_Instructor_plugin.h"

using namespace std;

void NeuriteInstructor::cropStack(unsigned char InputImagePtr[], unsigned char OutputImagePtr[],
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

void NeuriteInstructor::maxIPStack(unsigned char inputVOIPtr[], unsigned char OutputImage2DPtr[],
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

QStringList NeuriteInstructor::importSeriesFileList_addnumbersort(const QString & curFilePath)
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
}
