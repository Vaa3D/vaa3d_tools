#include <dirent.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

#include <qstringlist.h>
#include <qfile.h>
#include <qvector.h>
#include <qfileinfo.h>

#include "Deep_Learning_Handler_plugin.h"

using namespace std;

void DL_Handler::cropStack(unsigned char InputImagePtr[], unsigned char OutputImagePtr[],
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

QStringList DL_Handler::importSeriesFileList_addnumbersort(const QString & curFilePath)
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
