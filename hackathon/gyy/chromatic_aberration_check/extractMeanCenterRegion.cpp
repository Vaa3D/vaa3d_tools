#include <iostream>
#include <vector>
#include <v3d_interface.h>
#include "extractCenter_gui.h"

using namespace std;


bool calculateShifts(V3DPluginCallback2 &callback, QWidget *parent)
 {
     ExtractMeanCenterDialog dialog(parent);
     if(dialog.exec()!=QDialog::Accepted)return false;
     dialog.update();

     V3DLONG rangeNum = dialog.rangeBox -> value();

     QDir dir(dialog.openFolderFiles);
     QStringList filters;
     filters << "*.tif";
     QFileInfoList imageList = dir.entryInfoList(filters, QDir::Files);
     qDebug()<<"imageList.size = "<<imageList.size();

     unsigned short * pdata_XZ = 0;
     unsigned short * pdata_YZ = 0;
     ExtractMeanCenterDialog:: extractImage(callback, imageList, pdata_XZ, pdata_YZ, rangeNum, dialog.openFolderFiles, dialog.saveFolderFiles);



      return 0;
 }


