/* neuron_tile_display_plugin.cpp
 * This Plugin will tile neuron to display
 * 2014-10-28 : by Hanbo Chen
 */
 
#include "v3d_message.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <QApplication>
#include <QTime>
#include <cmath>
#include "neuron_tile_display_plugin.h"
#include "basic_surf_objs.h"
#include "../../../v3d_main/neuron_editing/neuron_xforms.h"
#include <QInputDialog>
using namespace std;
#define MYFLOAT double
#define MAXSIZE 100
const double pi = 3.1415926535897;
//Q_EXPORT_PLUGIN2(neuron_tile_display, neuron_tile_display);
void MethodForBigScreenDisplay(V3DPluginCallback2 &callback, QWidget *parent, int displayNum);
void MethodForUpdateSWCDispaly(V3DPluginCallback2 &callback, QWidget *parent);
void printHelpForBigScreenUsage();
void angles_to_quaternions(MYFLOAT q[], MYFLOAT xRot, MYFLOAT yRot,MYFLOAT zRot);
void angles_to_quaternions_3DRotation(MYFLOAT q[], MYFLOAT xRot, MYFLOAT yRot,MYFLOAT zRot);
void slerp_zhi(MYFLOAT q1[], MYFLOAT q2[], MYFLOAT t, MYFLOAT q_sample[]);
void quaternions_to_angles(MYFLOAT Rot_current[], MYFLOAT q_sample[]);
void quaternions_to_angles_3DRotation(MYFLOAT Rot_current[], MYFLOAT q[]);
MYFLOAT dot_multi(MYFLOAT q1[], MYFLOAT q2[]);
MYFLOAT dot_multi_normalized(MYFLOAT q1[], MYFLOAT q2[]);
int ZmovieMaker(V3DPluginCallback2 &callback, QWidget *parent/*,QStringList namelist*/);
void ZmovieMaker(V3DPluginCallback2 &callback, QWidget *parent,const V3DPluginArgList & input);
void ZmovieMaker(V3DPluginCallback2 &callback, QWidget *parent,int samplingRate);
void LoadAnchorFile();
//void LoadAnchorFile(QString fileOpenName);
void LoadAnchorFile(const V3DPluginArgList & input);

QString m_InputfolderName="";
struct ZMovieFileType
{
    QString anchor_file_path;
    QString frame_folder_path;
};
struct AnchorPointPARA
{
    MYFLOAT xRot, yRot,zRot,xShift,yShift,zShift,zoom,xCut0,xCut1,yCut0,yCut1,zCut0,zCut1,frontCut;
    bool channelR,channelG,channelB;
    int showSurf,timePoint;
    int xClip0,xClip1,yClip0,yClip1,zClip0,zClip1;
};
AnchorPointPARA anchorPara[MAXSIZE];
int anchorParaSize=0;

#define GET_PARA \
{ \
    xRot = PARA_temp.xRot;\
    yRot = PARA_temp.yRot;\
    zRot = PARA_temp.zRot;\
    xShift = PARA_temp.xShift;\
    yShift = PARA_temp.yShift;\
    zShift = PARA_temp.zShift;\
    zoom = PARA_temp.zoom;\
    xCut0 = PARA_temp.xCut0 ;\
    xCut1 = PARA_temp.xCut1;\
    yCut0 = PARA_temp.yCut0;\
    yCut1 = PARA_temp.yCut1;\
    zCut0 = PARA_temp.zCut0;\
    zCut1 = PARA_temp.zCut1;\
    channelR = PARA_temp.channelR;\
    channelG = PARA_temp.channelG;\
    channelB = PARA_temp.channelB;\
    showSurf = PARA_temp.showSurf;\
    xClip0 = PARA_temp.xClip0;\
    xClip1 = PARA_temp.xClip1;\
    yClip0 = PARA_temp.yClip0;\
    yClip1 = PARA_temp.yClip1;\
    zClip0 = PARA_temp.zClip0;\
    zClip1 = PARA_temp.zClip1;\
    frontCut = PARA_temp.frontCut;\
    timePoint = PARA_temp.timePoint;\
    }

#define SET_3DVIEW \
{ \
    view->resetRotation();\
    view->doAbsoluteRot((float)xRot,(float)yRot,(float)zRot);\
    view->setXShift((float)xShift);\
    view->setYShift((float)yShift);\
    view->setZShift((float)zShift);\
    view->setZoom((float)zoom);\
    view->setXCut0((float)xCut0);\
    view->setXCut1((float)xCut1);\
    view->setYCut0((float)yCut0);\
    view->setYCut1((float)yCut1);\
    view->setZCut0((float)zCut0);\
    view->setZCut1((float)zCut1);\
    view->setChannelR(channelR);\
    view->setChannelG(channelG);\
    view->setChannelB(channelB);\
    view->setShowSurfObjects(showSurf);\
    view->setXClip0((float)xClip0);\
    view->setXClip1((float)xClip1);\
    view->setYClip0((float)yClip0);\
    view->setYClip1((float)yClip1);\
    view->setZClip0((float)zClip0);\
    view->setZClip1((float)zClip1);\
    view->setFrontCut((float)frontCut);\
    view->setVolumeTimePoint((int)timePoint);\
    \
    if(curwin)\
{\
    callback.updateImageWindow(curwin);\
    }\
    else\
    callback.update_3DViewer(surface_win); \
}

#define INTERPOLATION_PARA \
{ \
    view->resetRotation();\
    angles_to_quaternions(q1,xRot_last,yRot_last,zRot_last);\
    angles_to_quaternions(q2,xRot,yRot,zRot);\
    slerp_zhi(q1, q2, (MYFLOAT)i/N, q_sample);\
    \
    \
    quaternions_to_angles(Rot_current,q_sample);\
    xShift_current = (xShift_last + i*(xShift-xShift_last)/N); \
    yShift_current = (yShift_last + i*(yShift-yShift_last)/N); \
    zShift_current = (zShift_last + i*(zShift-zShift_last)/N); \
    zoom_current = (zoom_last + i*(zoom-zoom_last)/N); \
    channel_current = (MYFLOAT)i/N;\
    xClip0_current = (xClip0_last + i*(xClip0-xClip0_last)/N);\
    xClip1_current = (xClip1_last + i*(xClip1-xClip1_last)/N);\
    yClip0_current = (yClip0_last + i*(yClip0-yClip0_last)/N);\
    yClip1_current = (yClip1_last + i*(yClip1-yClip1_last)/N);\
    zClip0_current = (zClip0_last + i*(zClip0-zClip0_last)/N);\
    zClip1_current = (zClip1_last + i*(zClip1-zClip1_last)/N);\
    xCut0_current = (xCut0_last + i*(xCut0-xCut0_last)/N);\
    xCut1_current = (xCut1_last + i*(xCut1-xCut1_last)/N);\
    yCut0_current = (yCut0_last + i*(yCut0-yCut0_last)/N);\
    yCut1_current = (yCut1_last + i*(yCut1-yCut1_last)/N);\
    zCut0_current = (zCut0_last + i*(zCut0-zCut0_last)/N);\
    zCut1_current = (zCut1_last + i*(zCut1-zCut1_last)/N);\
    frontCut_current = (frontCut_last + i*(frontCut-frontCut_last)/N);\
    timePoint_current = (timePoint_last + i*(timePoint-timePoint_last)/N);\
    \
    \
    \
    view->doAbsoluteRot((float)Rot_current[0], (float)Rot_current[1], (float)Rot_current[2]);\
    view->setXShift((float)(xShift_current));\
    view->setYShift((float)(yShift_current));\
    view->setZShift((float)(zShift_current));\
    view->setZoom((float)(zoom_current));\
    view->setVolumeTimePoint((int)timePoint_current);\
    if(channel_current < 0.5)\
{\
    view->setChannelR(channelR_last);\
    view->setChannelG(channelG_last);\
    view->setChannelB(channelB_last);\
    view->setShowSurfObjects(showSurf_last);\
    }\
    else\
{\
    view->setChannelR(channelR);\
    view->setChannelG(channelG);\
    view->setChannelB(channelB);\
    view->setShowSurfObjects(showSurf);\
    }\
    view->setXClip0((float)(xClip0_current));\
    view->setXClip1((float)(xClip1_current));\
    view->setYClip0((float)(yClip0_current));\
    view->setYClip1((float)(yClip1_current));\
    view->setZClip0((float)(zClip0_current));\
    view->setZClip1((float)(zClip1_current));\
    if(curwin)\
{\
    view->setXCut0((float)(xCut0_current));\
    view->setXCut1((float)(xCut1_current));\
    view->setYCut0((float)(yCut0_current));\
    view->setYCut1((float)(yCut1_current));\
    view->setZCut0((float)(zCut0_current));\
    view->setZCut1((float)(zCut1_current));\
    view->setFrontCut((float)frontCut_current);\
    callback.updateImageWindow(curwin);\
    }\
    else\
    callback.update_3DViewer(surface_win); \
    }
#define UPDATE_PARA \
{ \
    xRot_last = xRot;\
    yRot_last = yRot;\
    zRot_last = zRot;\
    xShift_last = xShift;\
    yShift_last = yShift;\
    zShift_last = zShift;\
    zoom_last = zoom;\
    xCut0_last = xCut0;\
    xCut1_last = xCut1;\
    yCut0_last = yCut0;\
    yCut1_last = yCut1;\
    zCut0_last = zCut0;\
    zCut1_last = zCut1;\
    channelR_last = channelR;\
    channelG_last = channelG;\
    channelB_last = channelB;\
    showSurf_last = showSurf;\
    xClip0_last = xClip0;\
    xClip1_last = xClip1;\
    yClip0_last = yClip0;\
    yClip1_last = yClip1;\
    zClip0_last = zClip0;\
    zClip1_last = zClip1;\
    frontCut_last = frontCut;\
    timePoint_last = timePoint;\
    }

//sleep
//void sleep(unsigned int minute)
//{
//    double stopmsec=minute*1000*60;
//    QTime reachTime=QTime::currentTime().addMSecs(stopmsec);
//    while(QTime::currentTime()<reachTime)
//        QCoreApplication::processEvents(QEventLoop::AllEvents,100);
//}

QStringList neuron_tile_display::menulist() const
{
	return QStringList() 
		<<tr("neuron_tile_display")
        <<tr("neuron_tile_XY")
        <<tr("neuron_tile_groups")
        <<tr("tile_multi_windows (swc and image files selected)")
        <<tr("tile_multi_windows (swc and image files under one folder)")
        <<tr("set configuration (tile_multi_windows options only)")
//       <<tr("BigScreen Display")
//      <<tr("BigScreen Display Update")
        <<tr("about");
}

QStringList neuron_tile_display::funclist() const
{
	return QStringList()
//            <<tr("BigScreenDisplay")
//           <<tr("BigScreenDisplayUpdate")
		<<tr("help");
}

QStringList importFileList_addnumbersort(const QString & curFilePath, int method_code)
{
    QStringList myList;
    myList.clear();

    // get the image files namelist in the directory
    QStringList imgSuffix;
    if (method_code ==1)
        imgSuffix<<"*.swc"<<"*.eswc"<<"*.SWC"<<"*.ESWC";
    else if (method_code ==2)
        imgSuffix<<"*.marker";
    else if (method_code ==3)
        imgSuffix<<"*.raw"<<"*.v3draw"<<"*.v3dpbd"<<"*.tif"<<"*.RAW"<<"*.V3DRAW"<<"*.TIF"<<"*.V3DPBD";

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
    foreach (QString qs, myList)  qDebug() << qs;

    return myList;
}
///used in bigScreen display
QStringList importFileList_addnumbersort(const QString & curFilePath, int method_code,int displayNum)
{

    QStringList myList,displayList/*,pathList*/;
    map<int,double> fileDateMap;
    myList.clear();
    displayList.clear();
    //pathList.clear();
    // get the image files namelist in the directory
    QStringList imgSuffix;
    if (method_code ==1)
        imgSuffix<<"*.swc"<<"*.eswc"<<"*.SWC"<<"*.ESWC";
    else if (method_code ==2)
        imgSuffix<<"*.marker";
    else if (method_code ==3)
        imgSuffix<<"*.raw"<<"*.v3draw"<<"*.v3dpbd"<<"*.tif"<<"*.RAW"<<"*.V3DRAW"<<"*.TIF"<<"*.V3DPBD";

    QDir dir(curFilePath);
    if (!dir.exists())
    {
        qWarning("Cannot find the directory");
        return displayList;
    }

    //exact file name in FinishedNeuron folder
    int fileCount=0;
    foreach (QString file, dir.entryList(imgSuffix, QDir::Files, QDir::Name))
    {
        QString filename=QFileInfo(dir,file).baseName();
        //exact file finished date
        //nameList+=filename;
        QStringList fileInfoList=filename.split("_");
        if(!fileInfoList.size())
            break;
        QString fileDate=fileInfoList[fileInfoList.size()-1];
        //qDebug()<<fileDate;
        QString fileYear=fileDate.right(4);
        QString fileMonthDay=fileDate.left(4);
        int FileYear=fileYear.toInt();
        int FileMonthDay=fileMonthDay.toInt();
        double NeuronFinishedTime=FileYear*365+FileMonthDay;
        fileDateMap[fileCount]=NeuronFinishedTime;
        fileCount++;
    }

    map<int,int> lastIndex;
    for(int ita=0;ita<fileDateMap.size();ita++)
    {
        lastIndex[ita]=0;
        for(int itb=0;itb<fileDateMap.size();itb++)
        {
            if(fileDateMap[ita]<fileDateMap[itb])
            {
                lastIndex[ita]++;
            }
            if(fileDateMap[ita]==fileDateMap[itb]&&ita>itb)
            {
                lastIndex[ita]++;
            }
            if(lastIndex[ita]>displayNum)
            {
                break;
            }
        }
    }
    int count=0;int displayIndex[9];int displayOrderIndex[9];
    if(fileDateMap.size()<=displayNum)
    {
        foreach (QString file, dir.entryList(imgSuffix, QDir::Files, QDir::Name))
        {
            displayList += QFileInfo(dir, file).absoluteFilePath();
        }
    }
    else//pick up displayNumbers new files with lagerest date
    {
        int displayCount=0;
        foreach (QString file, dir.entryList(imgSuffix, QDir::Files, QDir::Name))
        {
            if(lastIndex[count]<displayNum)
            {
                displayIndex[displayCount]=lastIndex[count];
                displayCount++;
                myList += QFileInfo(dir, file).absoluteFilePath();
            }
            count++;
            if(myList.size()==displayNum)
                break;
        }//sort the display
        for(int i=0;i<myList.size();i++)
        {
            displayOrderIndex[displayIndex[i]]=i;
        }
        //std:sort(displayIndex,displayIndex+9,less<int>());

        for(int i=0;i<myList.size();i++)
        {
            displayList+=myList.at(displayOrderIndex[i]);
        }
    }

    foreach (QString qs, displayList)  qDebug() << qs;
    return displayList;
}
void WriteNewFinishedNeuronsFileName(QStringList nameList)
{
    //absolute path
    QString filePath="";QString fileName="/swclist.txt";
    foreach(QString qs,nameList)
    {
        QFileInfo file(qs);
        filePath=file.path()+fileName;
        qDebug() << filePath;
        break;
    }

    QFile finishedFile(filePath);
    if(finishedFile.exists())
    {
        finishedFile.remove();
    }
    if(!finishedFile.open(QIODevice::ReadWrite))
    {
        v3d_msg("Cannot open file for writing!");
        return;
    }
    QTextStream nameOut(&finishedFile);
    foreach(QString qs,nameList)
    {
        nameOut<<qs<<Qt::endl;
        qDebug()<<qs;
    }

}

void neuron_tile_display::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("neuron_tile_display"))
	{
        dotile(callback, parent);
	}
    else if (menu_name == tr("neuron_tile_XY"))
    {
        doxytile(callback, parent);
    }
    else if (menu_name == tr("neuron_tile_groups"))
    {
        dotile_groups(callback, parent);
    }
    else if (menu_name == tr("tile_multi_windows (swc and image files selected)"))
    {
        QStringList fileList;
        fileList = QFileDialog::getOpenFileNames(0, QObject::tr("Open File"),
                "",
                QObject::tr("Supported file (*.swc *.eswc *.raw *.v3draw *.v3dpbd *.tif)"
                    ));
        if(fileList.isEmpty())
            return;

        unsigned int col=1, row=1, xRez=1, yRez=1;

        QSettings settings("HHMI", "Vaa3D");

        col = settings.value("multi_windows_col").toInt(); if(col<1) col = 1;
        row = settings.value("multi_windows_row").toInt(); if(row<1) row = 1;
        xRez = settings.value("multi_windows_xRez").toInt();if(xRez<1) xRez = 1;
        yRez = settings.value("multi_windows_yRez").toInt();if(yRez<1) yRez = 1;

        V3dR_MainWindow * new3DWindow = NULL;
        int offsety = -1;
        for(V3DLONG i = 0; i < fileList.size(); i++)
        {           
            QString curPathFile = fileList.at(i);
            QFileInfo curfile_info(curPathFile);
            QString cur_suffix = curfile_info.suffix().toUpper();
            if ( (cur_suffix=="LSM") ||
                    (cur_suffix=="TIF") ||
                    (cur_suffix=="TIFF") ||
                    (cur_suffix=="RAW") ||
                    (cur_suffix=="V3DRAW") ||
                    (cur_suffix=="VAA3DRAW") ||
                    (cur_suffix=="RAW5") ||
                    (cur_suffix=="V3DRAW5") ||
                    (cur_suffix=="VAA3DRAW5") ||
                    (cur_suffix=="MRC") ||
                    (cur_suffix=="V3DPBD") ||
                    (cur_suffix=="NRRD") ||
                    (cur_suffix=="NHDR") ||
                    (cur_suffix=="VAA3DPBD") ||
                    (cur_suffix=="MP4") ||
                    (cur_suffix=="H5J"))
            {
                 char * inimg_file  = &curPathFile.toStdString()[0];
                 Image4DSimple* p4DImage = callback.loadImage(inimg_file);
                 v3dhandle newwin = callback.newImageWindow();
                 callback.setImage(newwin, p4DImage);
                 callback.setImageName(newwin,curPathFile);
                 callback.updateImageWindow(newwin);
                 callback.open3DWindow(newwin);
            }
            else if (cur_suffix=="SWC" ||
                     cur_suffix=="ESWC")
            {
                new3DWindow = callback.open3DViewerForSingleSurfaceFile(curPathFile);

            }

            QList <V3dR_MainWindow *> cur_list_3dviewer = callback.getListAll3DViewers();
            for (V3DLONG i = 0; i < cur_list_3dviewer.size(); i++)
            {
                if( (i%col)*xRez ==0)
                    offsety++;
                callback.moveWindow(cur_list_3dviewer.at(i),(i%col+row-1)*xRez,offsety*yRez);
                callback.resizeWindow(cur_list_3dviewer.at(i),xRez,yRez);

            }
        }

    }
    else if (menu_name == tr("tile_multi_windows (swc and image files under one folder)"))
    {
        QString m_InputfolderName = QFileDialog::getExistingDirectory(parent, QObject::tr("Choose the directory including all swc files "),
                                                                      QDir::currentPath(),
                                                                      QFileDialog::ShowDirsOnly);


        unsigned int col=1, row=1, xRez=1, yRez=1;
        QSettings settings("HHMI", "Vaa3D");

        col = settings.value("multi_windows_col").toInt(); if(col<1) col = 1;
        row = settings.value("multi_windows_row").toInt(); if(row<1) row = 1;
        xRez = settings.value("multi_windows_xRez").toInt();if(xRez<1) xRez = 1;
        yRez = settings.value("multi_windows_yRez").toInt();if(yRez<1) yRez = 1;

        QRect deskRect = QGuiApplication::screens().first()->availableGeometry();
        qDebug("deskRect height %d and width %d",deskRect.height(),deskRect.width());
        int xRez_o=deskRect.width()/col;
        int yRez_o=deskRect.height()/row;
        col--;
        yRez_o=yRez_o;
        qDebug("every col size is %d",xRez_o);
        qDebug("every row size is %d",yRez_o);

        QStringList swcList = importFileList_addnumbersort(m_InputfolderName, 1);
        QStringList imagelist = importFileList_addnumbersort(m_InputfolderName, 3);

        for(V3DLONG i=0; i < imagelist.size(); i++)
        {
            QString curPathIMAGE = imagelist.at(i);
           // v3d_msg(curPathIMAGE);
           // char * inimg_file  = &curPathIMAGE.toStdString()[0];
          //  v3d_msg(QString("%1").arg(inimg_file));
            Image4DSimple* p4DImage = callback.loadImage((char *)curPathIMAGE.toStdString().c_str());
            v3dhandle newwin = callback.newImageWindow();
            callback.setImage(newwin, p4DImage);
            callback.setImageName(newwin,curPathIMAGE);
            callback.updateImageWindow(newwin);
            callback.open3DWindow(newwin);
        }
        V3dR_MainWindow * new3DWindow = NULL;
        int offsety = -1;
        for(V3DLONG i = 0; i < swcList.size(); i++)
        {
            QString curPathSWC = swcList.at(i);
            new3DWindow = callback.open3DViewerForSingleSurfaceFile(curPathSWC);
        }

        QList <V3dR_MainWindow *> cur_list_3dviewer = callback.getListAll3DViewers();
        for (V3DLONG i = 0; i < cur_list_3dviewer.size(); i++)
        {
            if( (i%col)*xRez ==0)
                offsety++;

            callback.moveWindow(cur_list_3dviewer.at(i),(i%col)*(xRez_o+xRez),offsety*(yRez_o+yRez));
            callback.setHideDisplayControlButton(cur_list_3dviewer.at(i));
           // callback.setResizeEvent(cur_list_3dviewer.at(i),768,600);
        }


    }
    else if (menu_name == tr("set configuration (tile_multi_windows options only)"))
    {
             bool ok1, ok2, ok3, ok4;
             unsigned int col=1, row=1, xRez=1, yRez=1;

             col = QInputDialog::getInt(parent, "",
                                           "#tiles per row:",
                                           1, 1, 100, 1, &ok1);

             if(ok1)
             {
                 row = QInputDialog::getInt(parent, "",
                                               "display from column:",
                                               1, 1, 100, 1, &ok2);
             }
             else
                 return;

             if(ok2)
             {
                 xRez = QInputDialog::getInt(parent, " ",
                                               "offset X:",
                                               1, -10000, 10000, 1, &ok3);
             }
             else
                 return;

             if(ok3)
             {
                 yRez = QInputDialog::getInt(parent, " ",
                                               "offset Y:",
                                               1, -10000, 10000, 1, &ok4);
             }
             else
                 return;

             QSettings settings("HHMI", "Vaa3D");
             settings.setValue("multi_windows_col", col);
             settings.setValue("multi_windows_row", row);
             settings.setValue("multi_windows_xRez", xRez);
             settings.setValue("multi_windows_yRez", yRez);

             v3d_msg("Configuration Done!");
    }
//    else if(menu_name==tr("BigScreen Display"))
//    {
//        unsigned int updateInterval=30;
//        bool ok=true;
//        updateInterval = QInputDialog::getInteger(parent, "",
//                                      "#update interval time (minutes) :",
//                                      30, 0, 10000, 1, &ok);
//        if(ok)
//            MethodForBigScreenDisplay(callback,parent,updateInterval);
//        else
//            return;
//    }
//    else if(menu_name==tr("BigScreen Display Update"))
//    {
//        MethodForUpdateSWCDispaly(callback,parent);
//    }
    else
	{
		v3d_msg(tr("This Plugin will tile neuron to display. "
			"Developed by Hanbo Chen, 2014-10-28"));
	}
}

void MethodFunForUpdateSWCDispaly(V3DPluginCallback2 &callback, QWidget *parent, const V3DPluginArgList & input)
{
    cout<<"big screen display update"<<endl;
    vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
//    vector<char*>* outlist = NULL;
//    vector<char*>* paralist = NULL;

//    cout<<input.size()<<"   what size is mowwww"<<endl;
    if(input.size() != 2)
    {
        printf("Please specify both input folder .\n");
        return /*false*/;
    }
//    int displayNum = atof(paralist->at(0));

    QString inputfolderName = QString(inlist->at(0));
    QList <V3dR_MainWindow *> cur_list_3dviewer = callback.getListAll3DViewers();
    unsigned int displayNum=9;
    //clear the window
    for(int i=0;i<cur_list_3dviewer.size();i++)
    {
        QList<NeuronTree> * new_treeList = callback.getHandleNeuronTrees_Any3DViewer (cur_list_3dviewer.at(i));

        new_treeList->clear();
        QString file_name="updating....";
        callback.setWindowDataTitle(cur_list_3dviewer.at(i), file_name);
        callback.update_NeuronBoundingBox(cur_list_3dviewer.at(i));
        callback.update_3DViewer(cur_list_3dviewer.at(i));
    }
    //update SWC list
    QStringList swcList = importFileList_addnumbersort(inputfolderName, 1,displayNum);
    WriteNewFinishedNeuronsFileName(swcList);
    for(int i=0;i<swcList.size();i++)
    {
        QList<NeuronTree> * new_treeList = callback.getHandleNeuronTrees_Any3DViewer (cur_list_3dviewer.at(i));
        //new_treeList->clear();

        QString curPathSWC = swcList.at(i);
        QFileInfo curSWCBase(curPathSWC);
        NeuronTree tmp=readSWC_file(curPathSWC);
        new_treeList->push_back(tmp);
        //new3DWindow = callback.open3DViewerForSingleSurfaceFile(curPathSWC);
        //reset window title to basename instead of path name
        callback.setWindowDataTitle(cur_list_3dviewer.at(i),curSWCBase.baseName());
        callback.update_NeuronBoundingBox(cur_list_3dviewer.at(i));
        callback.update_3DViewer(cur_list_3dviewer.at(i));
    }
    //update Combine SWC file

    V3dR_MainWindow * surface_combine_win=cur_list_3dviewer.at(cur_list_3dviewer.size()-1);
    QList<NeuronTree> * new_treeList_combine = callback.getHandleNeuronTrees_Any3DViewer (surface_combine_win);
    QString combine_file_name="Combined Latest Finished Neurons";
    int neuronNum = swcList.size();
    for (V3DLONG i=0;i<neuronNum;i++)
    {
        NeuronTree tmp = readSWC_file(swcList.at(i));
        for(int j=0;j<tmp.listNeuron.size();j++)
            tmp.listNeuron[j].type=i+2;
        new_treeList_combine->push_back(tmp);
    }
    callback.setWindowDataTitle(surface_combine_win, combine_file_name);
    callback.update_NeuronBoundingBox(surface_combine_win);
    callback.update_3DViewer(surface_combine_win);

}
void MethodForUpdateSWCDispaly(V3DPluginCallback2 &callback, QWidget *parent)
{
    QList <V3dR_MainWindow *> cur_list_3dviewer = callback.getListAll3DViewers();
    unsigned int displayNum=9;
    //clear the window
    for(int i=0;i<cur_list_3dviewer.size();i++)
    {
        QList<NeuronTree> * new_treeList = callback.getHandleNeuronTrees_Any3DViewer (cur_list_3dviewer.at(i));

        new_treeList->clear();
        QString file_name="updating....";
        callback.setWindowDataTitle(cur_list_3dviewer.at(i), file_name);
        callback.update_NeuronBoundingBox(cur_list_3dviewer.at(i));
        callback.update_3DViewer(cur_list_3dviewer.at(i));
    }
    //update SWC list
    if(m_InputfolderName.size()==0)
        return;

    QStringList swcList = importFileList_addnumbersort(m_InputfolderName, 1,displayNum);
    WriteNewFinishedNeuronsFileName(swcList);
    for(int i=0;i<swcList.size();i++)
    {
        QList<NeuronTree> * new_treeList = callback.getHandleNeuronTrees_Any3DViewer (cur_list_3dviewer.at(i));
        //new_treeList->clear();

        QString curPathSWC = swcList.at(i);
        QFileInfo curSWCBase(curPathSWC);
        NeuronTree tmp=readSWC_file(curPathSWC);
        new_treeList->push_back(tmp);
        //new3DWindow = callback.open3DViewerForSingleSurfaceFile(curPathSWC);
        //reset window title to basename instead of path name
        callback.setWindowDataTitle(cur_list_3dviewer.at(i),curSWCBase.baseName());
        callback.update_NeuronBoundingBox(cur_list_3dviewer.at(i));
        callback.update_3DViewer(cur_list_3dviewer.at(i));
    }
    //update Combine SWC file

    V3dR_MainWindow * surface_combine_win=cur_list_3dviewer.at(cur_list_3dviewer.size()-1);
    QList<NeuronTree> * new_treeList_combine = callback.getHandleNeuronTrees_Any3DViewer (surface_combine_win);
    QString combine_file_name="Combined Latest Finished Neurons";
    int neuronNum = swcList.size();
    for (V3DLONG i=0;i<neuronNum;i++)
    {
        NeuronTree tmp = readSWC_file(swcList.at(i));
        for(int j=0;j<tmp.listNeuron.size();j++)
            tmp.listNeuron[j].type=i+2;
        new_treeList_combine->push_back(tmp);
    }
    callback.setWindowDataTitle(surface_combine_win, combine_file_name);
    callback.update_NeuronBoundingBox(surface_combine_win);
    callback.update_3DViewer(surface_combine_win);
}


void MethodForCombineSWCDisplay(V3DPluginCallback2 &callback, QWidget *parent,QStringList nameList)
{
    V3dR_MainWindow * surface_win = callback.createEmpty3DViewer();
    if (!surface_win)
    {
        v3d_msg(QString("Failed to open an empty window!"));
        return;
    }

    QList<NeuronTree> * new_treeList = callback.getHandleNeuronTrees_Any3DViewer (surface_win);

    new_treeList->clear();
    QString file_name="Combined Latest Finished Neurons";

    int neuronNum = nameList.size();
    for (V3DLONG i=0;i<neuronNum;i++)
    {
        NeuronTree tmp = readSWC_file(nameList.at(i));
        for(int j=0;j<tmp.listNeuron.size();j++)
            tmp.listNeuron[j].type=i+2;
        new_treeList->push_back(tmp);
    }

    callback.setWindowDataTitle(surface_win, file_name);
    callback.update_NeuronBoundingBox(surface_win);
    callback.update_3DViewer(surface_win);
}

//designed by shengdian.08282018
void MethodForBigScreenDisplay(V3DPluginCallback2 &callback, QWidget *parent,int updateInterval=30)
{
    /*QString*/ m_InputfolderName = QFileDialog::getExistingDirectory(parent, QObject::tr("Choose the directory including all finished annotation files "),
                                                                  QDir::currentPath(),
                                                                  QFileDialog::ShowDirsOnly);



    int displayNum=9;
    int col=5, /*row=3,*/ xRez=3840, yRez=2160;
    //
    switch (displayNum) {
    case 9:
        col=3;
        break;
    case 10:
        col=5;
        break;
    default:
        break;
    }

    QStringList swcList = importFileList_addnumbersort(m_InputfolderName, 1,displayNum);
    WriteNewFinishedNeuronsFileName(swcList);

    V3dR_MainWindow * new3DWindow = NULL;
    int offsety = -1;
    for(V3DLONG i = 0; i < swcList.size(); i++)
    {
        QString curPathSWC = swcList.at(i);
        QFileInfo curSWCBase(curPathSWC);
        new3DWindow = callback.open3DViewerForSingleSurfaceFile(curPathSWC);
        //reset window title to basename instead of path name
        callback.setWindowDataTitle(new3DWindow,curSWCBase.baseName());
    }
    MethodForCombineSWCDisplay(callback,parent,swcList);
    LoadAnchorFile();

    QList <V3dR_MainWindow *> cur_list_3dviewer = callback.getListAll3DViewers();
    qDebug("new window size is %d",cur_list_3dviewer.size());
    int xdim=1920;int ydim=1080;
    for (V3DLONG i = 0; i < cur_list_3dviewer.size(); i++)
    {
        if( (i%col)/**xRez*/ ==0)
            offsety++;

        if(i<cur_list_3dviewer.size()-1)
        {
            callback.moveWindow(cur_list_3dviewer.at(i),xRez+(i%col)*xdim,(offsety)*ydim);
            callback.resizeWindow(cur_list_3dviewer.at(i),xdim,ydim);
        }
        else
        {
            //int ydim=1030;int xdim=1920;
            callback.moveWindow(cur_list_3dviewer.at(i),xRez+xdim*3,0);
            callback.resizeWindow(cur_list_3dviewer.at(i),xdim,ydim);
        }
        callback.setHideDisplayControlButton(cur_list_3dviewer.at(i));
    }
    int samplingRate=ZmovieMaker(callback,parent);
    int updateCount=0;
    while(true)
    {
        //sleep(updateInterval);
        if(updateCount==updateInterval)
        {
            MethodForUpdateSWCDispaly(callback,parent);
            updateCount=0;
        }
        ZmovieMaker(callback,parent,samplingRate);
        updateCount++;
    }

}

void MethodFunForBigScreenDisplay(V3DPluginCallback2 &callback, QWidget *parent/*,int displayNum=9*/, const V3DPluginArgList & input)
{

    cout<<"Welcome to big screen display"<<endl;
    vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
    //QString anchorFileName = QString(inlist->at(1));
    if(input.size() != 2)
    {
        printf("Please specify both input folder and the number of new finished Neurons.\n");
        printHelpForBigScreenUsage();
        return /*false*/;
    }
    int displayNum = 9;

    /*QString*/ m_InputfolderName = QString(inlist->at(0));
    /*QString*/ /*m_InputfolderName = QFileDialog::getExistingDirectory(parent, QObject::tr("Choose the directory including all finished annotation files "),
                                                                  QDir::currentPath(),
                                                                  QFileDialog::ShowDirsOnly);*/


    int col=5, /*row=3,*/ xRez=3840, yRez=2160;
    //
    switch (displayNum) {
    case 9:
        col=3;
        break;
    case 10:
        col=5;
        break;
    default:
        break;
    }

    QStringList swcList = importFileList_addnumbersort(m_InputfolderName, 1,displayNum);

    //write new finished nine neurons to file
    WriteNewFinishedNeuronsFileName(swcList);

    V3dR_MainWindow * new3DWindow = NULL;
    int offsety = -1;
    for(V3DLONG i = 0; i < swcList.size(); i++)
    {
        QString curPathSWC = swcList.at(i);
        QFileInfo curSWCBase(curPathSWC);
        new3DWindow = callback.open3DViewerForSingleSurfaceFile(curPathSWC);
        //reset window title to basename instead of path name
        callback.setWindowDataTitle(new3DWindow,curSWCBase.baseName());
    }
    MethodForCombineSWCDisplay(callback,parent,swcList);

    QList <V3dR_MainWindow *> cur_list_3dviewer = callback.getListAll3DViewers();
    qDebug("new window size is %d",cur_list_3dviewer.size());
    int xdim=1920;int ydim=1080;
    for (V3DLONG i = 0; i < cur_list_3dviewer.size(); i++)
    {
        if( (i%col)/**xRez*/ ==0)
            offsety++;

        if(i<cur_list_3dviewer.size()-1)
        {
            callback.moveWindow(cur_list_3dviewer.at(i),xRez+(i%col)*xdim,(offsety)*ydim);
            callback.resizeWindow(cur_list_3dviewer.at(i),xdim,ydim);
        }
        else
        {
            //int ydim=1030;int xdim=1920;
            callback.moveWindow(cur_list_3dviewer.at(i),xRez+xdim*3,0);
            callback.resizeWindow(cur_list_3dviewer.at(i),xdim,ydim);
        }
        callback.setHideDisplayControlButton(cur_list_3dviewer.at(i));
    }
    LoadAnchorFile(input);
    ZmovieMaker(callback,parent,input);
}

void printHelpForBigScreenUsage()
{
    printf("\nVaa3D plugin: BigScreenDisplay, including: \n");
    printf("\t1) display nine new finished neurons of one mouse brain\n");
    printf("\t2) combine and display nine new finished neurons together to a window \n");
    printf("\t3) real time update                                                   \n");
    printf("\t#i <neuron_finished_folder_name> 1:   input finished neurons (all in one folder) of one brain \n");
    printf("\t#i <neuron_finished_folder_name> 2:   anchor file for zmovieMaker \n");
    printf("\t#p <update interval time> :  1. update 10 windows every interval time.\n");
    printf("\t                                if not specified, use 30 \"\n");
    printf("\t                             2. zMoiveMaker Sampling Rate 1-1000 \"\n");
    printf("\t                                if not specified, use 100 \"\n");
    printf("Usage (linux): vaa3d -x tile_display_multiple_neurons -f BigScreenDisplay -i /home/data/SEUAllenJointDataCenter/finished_annotations/17545_finished_neurons/ /home/penglab/Data/jsd/anchorPointFile.txt -p 30 500\" \n");
    printf("Usage (windows): vaa3d_msvc.exe /x tile_display_multiple_neurons /f BigScreenDisplay /i d:/home/data/SEUAllenJointDataCenter/finished_annotations/17545_finished_neurons/ d:/home/penglab/Data/jsd/anchorPointFile.txt /p 30 500\" \n");
    printf("\n                                                           \n");

}

void LoadAnchorFile(const V3DPluginArgList & input)
{
    vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
    QString fileOpenName = QString(inlist->at(1));
    if(input.size() != 2)
    {
        printf("Please specify both input folder and the number of new finished Neurons.\n");
        printHelpForBigScreenUsage();
        return /*false*/;
    }
    if(fileOpenName.isEmpty())
    {
        cout<<"Anchor file is not existed or empty!";
        return;
    }

    if (fileOpenName.size()>0)
    {
        //list_anchors->clear();
        ifstream ifs(fileOpenName.toLatin1());
        string points;
        while(ifs && getline(ifs, points))
        {
            std::istringstream iss(points);
            iss >>anchorPara[anchorParaSize].xRot >>anchorPara[anchorParaSize].yRot >>anchorPara[anchorParaSize].zRot >>
                  anchorPara[anchorParaSize].xShift >> anchorPara[anchorParaSize].yShift >> anchorPara[anchorParaSize].zShift >>
                  anchorPara[anchorParaSize].zoom >>
                   anchorPara[anchorParaSize].xCut0 >> anchorPara[anchorParaSize].xCut1 >>
                   anchorPara[anchorParaSize].yCut0 >> anchorPara[anchorParaSize].yCut1 >>
                   anchorPara[anchorParaSize].zCut0 >> anchorPara[anchorParaSize].zCut1 >>
                   anchorPara[anchorParaSize].channelR >> anchorPara[anchorParaSize].channelG >> anchorPara[anchorParaSize].channelB >>
                   anchorPara[anchorParaSize].showSurf >>
                   anchorPara[anchorParaSize].xClip0 >> anchorPara[anchorParaSize].xClip1 >> anchorPara[anchorParaSize].yClip0 >>
                   anchorPara[anchorParaSize].yClip1 >> anchorPara[anchorParaSize].zClip0 >> anchorPara[anchorParaSize].zClip1 >>
                   anchorPara[anchorParaSize].frontCut >>
                   anchorPara[anchorParaSize].timePoint;
            anchorParaSize++;
            if(anchorParaSize>MAXSIZE)
            {
                cout<<"Anchor file size is too big to load for displaying."<<endl;
                break;
            }
        }
    }
    return;
}

void LoadAnchorFile()
{
    QString fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
                                                        "",
                                                        QObject::tr("Anchor Point File (*.apftxt *.txt *.apf)"
                                                                    ));
    if(fileOpenName.isEmpty())
    {
        cout<<"Anchor file is not existed or empty!";
        return;
    }

    if (fileOpenName.size()>0)
    {
        //list_anchors->clear();
        ifstream ifs(fileOpenName.toLatin1());
        string points;
        while(ifs && getline(ifs, points))
        {
            std::istringstream iss(points);
            iss >>anchorPara[anchorParaSize].xRot >>anchorPara[anchorParaSize].yRot >>anchorPara[anchorParaSize].zRot >>
                  anchorPara[anchorParaSize].xShift >> anchorPara[anchorParaSize].yShift >> anchorPara[anchorParaSize].zShift >>
                  anchorPara[anchorParaSize].zoom >>
                   anchorPara[anchorParaSize].xCut0 >> anchorPara[anchorParaSize].xCut1 >>
                   anchorPara[anchorParaSize].yCut0 >> anchorPara[anchorParaSize].yCut1 >>
                   anchorPara[anchorParaSize].zCut0 >> anchorPara[anchorParaSize].zCut1 >>
                   anchorPara[anchorParaSize].channelR >> anchorPara[anchorParaSize].channelG >> anchorPara[anchorParaSize].channelB >>
                   anchorPara[anchorParaSize].showSurf >>
                   anchorPara[anchorParaSize].xClip0 >> anchorPara[anchorParaSize].xClip1 >> anchorPara[anchorParaSize].yClip0 >>
                   anchorPara[anchorParaSize].yClip1 >> anchorPara[anchorParaSize].zClip0 >> anchorPara[anchorParaSize].zClip1 >>
                   anchorPara[anchorParaSize].frontCut >>
                   anchorPara[anchorParaSize].timePoint;
            anchorParaSize++;
            if(anchorParaSize>MAXSIZE)
            {
                cout<<"Anchor file size is too big to load for displaying."<<endl;
                break;
            }
        }
    }
    return;
}

int ZmovieMaker(V3DPluginCallback2 &callback, QWidget *parent/*,QStringList namelist*/)
{
    View3DControl *view;
    v3dhandle curwin;
    QList <V3dR_MainWindow *> list_3dviewer;
    V3dR_MainWindow *surface_win;
    v3dhandleList list_triview;
    view=0;curwin=0;
    list_triview = callback.getImageWindowList();
    list_3dviewer = callback.getListAll3DViewers();
    if(anchorParaSize<=0) return 0;
    unsigned int N=30;
    bool ok=true;
    N = QInputDialog::getInt(parent, "",
                                  "#Sampling Rate is :",
                                  100, 1, 10000, 1, &ok);
    if(!ok)
        return 0;
    //int  N =anchorParaSize;
    for (int i=0; i<list_3dviewer.count(); i++)
    {
        surface_win = list_3dviewer[i];
        if(surface_win)
        {
            view = callback.getView3DControl_Any3DViewer(surface_win);
            if (!view) return 0;
            MYFLOAT xRot, yRot, zRot,
                    xShift, yShift, zShift,
                    zoom,
                    xCut0, xCut1,
                    yCut0, yCut1,
                    zCut0, zCut1,
                    frontCut;
            int showSurf, showSurf_last,
                timePoint,timePoint_last;
            bool channelR, channelG, channelB,
                    channelR_last, channelG_last, channelB_last;
            MYFLOAT xRot_last, yRot_last,zRot_last,
                    xShift_last,yShift_last,zShift_last,
                    zoom_last,
                    xCut0_last,xCut1_last,
                    yCut0_last,yCut1_last,
                    zCut0_last,zCut1_last,
                    frontCut_last;
            int xClip0,xClip1,yClip0,
                    yClip1,zClip0,zClip1;
            int xClip0_last,xClip1_last,
                    yClip0_last,yClip1_last,
                    zClip0_last,zClip1_last;

        //    // added by Hanchuan Peng, 2013-Dec-14 for debugging
            MYFLOAT xShift_current;
            MYFLOAT yShift_current;
            MYFLOAT zShift_current;
            MYFLOAT zoom_current;
            MYFLOAT channel_current;
            MYFLOAT xClip0_current;
            MYFLOAT xClip1_current;
            MYFLOAT yClip0_current;
            MYFLOAT yClip1_current;
            MYFLOAT zClip0_current;
            MYFLOAT zClip1_current;
            MYFLOAT xCut0_current;
            MYFLOAT xCut1_current;
            MYFLOAT yCut0_current;
            MYFLOAT yCut1_current;
            MYFLOAT zCut0_current;
            MYFLOAT zCut1_current;
            MYFLOAT frontCut_current;
            MYFLOAT timePoint_current;
            //

            MYFLOAT q1[4],q2[4],q_sample[4];
            MYFLOAT Rot_current[3];
            //QRegExp rx("(\\ |\\,|\\.|\\:|\\t)");
            if(anchorParaSize==0)
            {
                cout<<"please load anchor file first."<<endl;
                return 0;
            }
            for(int row = 0; row < anchorParaSize; row++)
            {
                AnchorPointPARA PARA_temp=anchorPara[row];
                GET_PARA;
                if(row==0)
                {
                    SET_3DVIEW;
                }
                else
                {
                    for (int i=1; i<=N; i++)
                    {
                        INTERPOLATION_PARA;
                    }
                }
                UPDATE_PARA;
            }
        }
        else
            return 0;
    }
    return N;
}

//This is for domenu update
void ZmovieMaker(V3DPluginCallback2 &callback, QWidget *parent,int samplingRate)
{
    int N = samplingRate;
    cout<<"Sampling Rate is "<<N<<endl;

    View3DControl *view;
    v3dhandle curwin;
    QList <V3dR_MainWindow *> list_3dviewer;
    V3dR_MainWindow *surface_win;
    v3dhandleList list_triview;
    view=0;curwin=0;
    list_triview = callback.getImageWindowList();
    list_3dviewer = callback.getListAll3DViewers();
    if(anchorParaSize<=0) return;
    //int  N =anchorParaSize;
    for (int i=0; i<list_3dviewer.count(); i++)
    {
        surface_win = list_3dviewer[i];
        if(surface_win)
        {
            view = callback.getView3DControl_Any3DViewer(surface_win);
            if (!view) return;
            MYFLOAT xRot, yRot, zRot,
                    xShift, yShift, zShift,
                    zoom,
                    xCut0, xCut1,
                    yCut0, yCut1,
                    zCut0, zCut1,
                    frontCut;
            int showSurf, showSurf_last,
                timePoint,timePoint_last;
            bool channelR, channelG, channelB,
                    channelR_last, channelG_last, channelB_last;
            MYFLOAT xRot_last, yRot_last,zRot_last,
                    xShift_last,yShift_last,zShift_last,
                    zoom_last,
                    xCut0_last,xCut1_last,
                    yCut0_last,yCut1_last,
                    zCut0_last,zCut1_last,
                    frontCut_last;
            int xClip0,xClip1,yClip0,
                    yClip1,zClip0,zClip1;
            int xClip0_last,xClip1_last,
                    yClip0_last,yClip1_last,
                    zClip0_last,zClip1_last;

        //    // added by Hanchuan Peng, 2013-Dec-14 for debugging
            MYFLOAT xShift_current;
            MYFLOAT yShift_current;
            MYFLOAT zShift_current;
            MYFLOAT zoom_current;
            MYFLOAT channel_current;
            MYFLOAT xClip0_current;
            MYFLOAT xClip1_current;
            MYFLOAT yClip0_current;
            MYFLOAT yClip1_current;
            MYFLOAT zClip0_current;
            MYFLOAT zClip1_current;
            MYFLOAT xCut0_current;
            MYFLOAT xCut1_current;
            MYFLOAT yCut0_current;
            MYFLOAT yCut1_current;
            MYFLOAT zCut0_current;
            MYFLOAT zCut1_current;
            MYFLOAT frontCut_current;
            MYFLOAT timePoint_current;
            //

            MYFLOAT q1[4],q2[4],q_sample[4];
            MYFLOAT Rot_current[3];
            //QRegExp rx("(\\ |\\,|\\.|\\:|\\t)");
            if(anchorParaSize==0)
            {
                cout<<"please load anchor file first."<<endl;
                return;
            }
            for(int row = 0; row < anchorParaSize; row++)
            {
        //        QString currentPoint = list_anchors->item(row)->text();
        //        QStringList currentParas = currentPoint.split(rx);
                AnchorPointPARA PARA_temp=anchorPara[row];
                GET_PARA;
                if(row==0)
                {
                    SET_3DVIEW;
                }
                else
                {
                    for (int i=1; i<=N; i++)
                    {
                        INTERPOLATION_PARA;
                    }
                }
                UPDATE_PARA;
            }
        }
        else
            return;
    }

//    return;
}
void ZmovieMaker(V3DPluginCallback2 &callback, QWidget *parent,const V3DPluginArgList & input)
{
    vector<char*> /*infiles,*/ inparas/*, outfiles*/;
//	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);

    if(input.size() != 2)
    {
        printf("Please specify both input folder and the number of new finished Neurons.\n");
        printHelpForBigScreenUsage();
        return;
    }
    int N = atof(inparas.at(1));
    cout<<"Sampling Rate is "<<N<<endl;

    View3DControl *view;
    v3dhandle curwin;
    QList <V3dR_MainWindow *> list_3dviewer;
    V3dR_MainWindow *surface_win;
    v3dhandleList list_triview;
    view=0;curwin=0;
    list_triview = callback.getImageWindowList();
    list_3dviewer = callback.getListAll3DViewers();
    if(anchorParaSize<=0) return;
    //int  N =anchorParaSize;
    for (int i=0; i<list_3dviewer.count(); i++)
    {
        surface_win = list_3dviewer[i];
        if(surface_win)
        {
            view = callback.getView3DControl_Any3DViewer(surface_win);
            if (!view) return;
            MYFLOAT xRot, yRot, zRot,
                    xShift, yShift, zShift,
                    zoom,
                    xCut0, xCut1,
                    yCut0, yCut1,
                    zCut0, zCut1,
                    frontCut;
            int showSurf, showSurf_last,
                timePoint,timePoint_last;
            bool channelR, channelG, channelB,
                    channelR_last, channelG_last, channelB_last;
            MYFLOAT xRot_last, yRot_last,zRot_last,
                    xShift_last,yShift_last,zShift_last,
                    zoom_last,
                    xCut0_last,xCut1_last,
                    yCut0_last,yCut1_last,
                    zCut0_last,zCut1_last,
                    frontCut_last;
            int xClip0,xClip1,yClip0,
                    yClip1,zClip0,zClip1;
            int xClip0_last,xClip1_last,
                    yClip0_last,yClip1_last,
                    zClip0_last,zClip1_last;

        //    // added by Hanchuan Peng, 2013-Dec-14 for debugging
            MYFLOAT xShift_current;
            MYFLOAT yShift_current;
            MYFLOAT zShift_current;
            MYFLOAT zoom_current;
            MYFLOAT channel_current;
            MYFLOAT xClip0_current;
            MYFLOAT xClip1_current;
            MYFLOAT yClip0_current;
            MYFLOAT yClip1_current;
            MYFLOAT zClip0_current;
            MYFLOAT zClip1_current;
            MYFLOAT xCut0_current;
            MYFLOAT xCut1_current;
            MYFLOAT yCut0_current;
            MYFLOAT yCut1_current;
            MYFLOAT zCut0_current;
            MYFLOAT zCut1_current;
            MYFLOAT frontCut_current;
            MYFLOAT timePoint_current;
            //

            MYFLOAT q1[4],q2[4],q_sample[4];
            MYFLOAT Rot_current[3];
            //QRegExp rx("(\\ |\\,|\\.|\\:|\\t)");
            if(anchorParaSize==0)
            {
                cout<<"please load anchor file first."<<endl;
                return;
            }
            for(int row = 0; row < anchorParaSize; row++)
            {
        //        QString currentPoint = list_anchors->item(row)->text();
        //        QStringList currentParas = currentPoint.split(rx);
                AnchorPointPARA PARA_temp=anchorPara[row];
                GET_PARA;
                if(row==0)
                {
                    SET_3DVIEW;
                }
                else
                {
                    for (int i=1; i<=N; i++)
                    {
                        INTERPOLATION_PARA;
                    }
                }
                UPDATE_PARA;
            }
        }
        else
            return;
    }

//    return;
}

bool neuron_tile_display::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{

    vector<char*> /*infiles,*/ inparas/*, outfiles*/;
//	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
//	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    if(input.size() != 2)
    {
        printf("Please specify both input folder and the number of new finished Neurons.\n");
        printHelpForBigScreenUsage();
        return false;
    }
    if (inparas.size()!=2)
    {
        printf("Please specify only one parameter - the number of new finished Neurons.\n");
        printHelpForBigScreenUsage();
        return false;
    }

    int updateInterval = atof(inparas.at(0));

	if (func_name == tr("help"))
	{
        printHelpForBigScreenUsage();
	}
    else if(func_name==tr("BigScreenDisplay"))
    {
//        MethodFunForBigScreenDisplay(callback,parent,input);
//        int updateCount=0;

//        while(true)
//        {
//            //sleep(updateInterval);
//            if(updateCount==updateInterval)
//            {
//                MethodFunForUpdateSWCDispaly(callback,parent,input);
//                updateCount=0;
//            }
//            ZmovieMaker(callback,parent,input);
//            updateCount++;
//        }

    }
    else if(func_name==tr("BigScreenDisplayUpdate"))
    {
        cout<<"move to display update"<<endl;
        //MethodForUpdateSWCDispaly(callback,parent);
    }
	else return false;

	return true;
}

void neuron_tile_display::dotile(V3DPluginCallback2 &callback, QWidget *parent)
{
    //select the window to operate
    QList <V3dR_MainWindow *> allWindowList = callback.getListAll3DViewers();
    QList <V3dR_MainWindow *> selectWindowList;
    V3dR_MainWindow * v3dwin;
    QList<NeuronTree> * ntTreeList;
    int winid;
    qDebug("search for 3D windows");
    for (V3DLONG i=0;i<allWindowList.size();i++)
    {
        ntTreeList = callback.getHandleNeuronTrees_Any3DViewer(allWindowList[i]);
        if(ntTreeList->size()>1)
            selectWindowList.append(allWindowList[i]);
    }
    qDebug("match and select 3D windows");
    if(selectWindowList.size()<1){
        v3d_msg("Cannot find 3D view with SWC file. Please load the SWC files you want to tile in the same 3D view");
        return;
    }else if(selectWindowList.size()>1){
        //pop up a window to select

        QStringList items;
        for(int i=0; i<selectWindowList.size(); i++){
            items.append(callback.getImageName(selectWindowList[i]));
        }
        bool ok;
        QString selectitem = QInputDialog::getItem(parent, QString::fromUtf8("Neuron Tile Viewer"), QString::fromUtf8("Select A Window to Operate"), items, 0, false, &ok);
        if(!ok) return;
        for(int i=0; i<selectWindowList.size(); i++){
            if(selectitem==callback.getImageName(selectWindowList[i]))
            {
                winid=i;
                break;
            }
        }
    }else{
        winid=0;
    }
    v3dwin = selectWindowList[winid];

    NeuronTileDisplayDialog * myDialog = NULL;
    myDialog = new NeuronTileDisplayDialog(&callback, v3dwin);
    myDialog->show();
}


void neuron_tile_display::dotile_groups(V3DPluginCallback2 &callback, QWidget *parent)
{
    QStringList ano_file_list = QFileDialog::getOpenFileNames(parent,"ano files",
                      QDir::currentPath(),"ano files (*.ano);;All files (*.*)" );


    V3dR_MainWindow * surface_win = callback.createEmpty3DViewer();
    if (!surface_win)
    {
        v3d_msg(QString("Failed to open an empty window!"));
        return;
    }
    callback.setWindowDataTitle(surface_win, QFileInfo(ano_file_list[0]).absoluteDir().absolutePath());
    NeuronTileGroupsDialog * myDialog = NULL;
    myDialog = new NeuronTileGroupsDialog(&callback, surface_win);
    myDialog->setAnoFileList(ano_file_list);
    myDialog->show();

}


void neuron_tile_display::doxytile(V3DPluginCallback2 &callback, QWidget *parent)
{
    //select the window to operate
    QList <V3dR_MainWindow *> allWindowList = callback.getListAll3DViewers();
    QList <V3dR_MainWindow *> selectWindowList;
    V3dR_MainWindow * v3dwin;
    QList<NeuronTree> * ntTreeList;
    int winid;
    qDebug("search for 3D windows");
    for (V3DLONG i=0;i<allWindowList.size();i++)
    {
        ntTreeList = callback.getHandleNeuronTrees_Any3DViewer(allWindowList[i]);
        if(ntTreeList->size()>1)
            selectWindowList.append(allWindowList[i]);
    }
    qDebug("match and select 3D windows");
    if(selectWindowList.size()<1){
        v3d_msg("Cannot find 3D view with SWC file. Please load the SWC files you want to tile in the same 3D view");
        return;
    }else if(selectWindowList.size()>1){
        //pop up a window to select

        QStringList items;
        for(int i=0; i<selectWindowList.size(); i++){
            items.append(callback.getImageName(selectWindowList[i]));
        }
        bool ok;
        QString selectitem = QInputDialog::getItem(parent, QString::fromUtf8("Neuron Tile Viewer"), QString::fromUtf8("Select A Window to Operate"), items, 0, false, &ok);
        if(!ok) return;
        for(int i=0; i<selectWindowList.size(); i++){
            if(selectitem==callback.getImageName(selectWindowList[i]))
            {
                winid=i;
                break;
            }
        }
    }else{
        winid=0;
    }
    v3dwin = selectWindowList[winid];

    NeuronXYTileDialog * myDialog = NULL;
    myDialog = new NeuronXYTileDialog(&callback, v3dwin);
    myDialog->show();
}
void angles_to_quaternions(MYFLOAT q[], MYFLOAT xRot, MYFLOAT yRot,MYFLOAT zRot)
{
    MYFLOAT xRot_Rad = xRot * (pi/180.0);  // if(xRot_Rad>pi) xRot_Rad -= 2*pi;
    MYFLOAT yRot_Rad = yRot * (pi/180.0);  // if(yRot_Rad>pi) yRot_Rad -= 2*pi;
    MYFLOAT zRot_Rad = zRot * (pi/180.0);  // if(zRot_Rad>pi) zRot_Rad -= 2*pi;

   /* q[0] = cos(xRot_Rad/2)*cos(yRot_Rad/2)*cos(zRot_Rad/2)+sin(xRot_Rad/2)*sin(yRot_Rad/2)*sin(zRot_Rad/2);
    q[1] = sin(xRot_Rad/2)*cos(yRot_Rad/2)*cos(zRot_Rad/2)-cos(xRot_Rad/2)*sin(yRot_Rad/2)*sin(zRot_Rad/2);
    q[2] = cos(xRot_Rad/2)*sin(yRot_Rad/2)*cos(zRot_Rad/2)+sin(xRot_Rad/2)*cos(yRot_Rad/2)*sin(zRot_Rad/2);
    q[3] = cos(xRot_Rad/2)*cos(yRot_Rad/2)*sin(zRot_Rad/2)-sin(xRot_Rad/2)*sin(yRot_Rad/2)*cos(zRot_Rad/2);*/

    q[0] = -sin(xRot_Rad/2)*sin(yRot_Rad/2)*sin(zRot_Rad/2)+ cos(xRot_Rad/2)*cos(yRot_Rad/2)*cos(zRot_Rad/2);
    q[1] = sin(xRot_Rad/2)*cos(yRot_Rad/2)*cos(zRot_Rad/2)+sin(yRot_Rad/2)*sin(zRot_Rad/2)*cos(xRot_Rad/2);
    q[2] = -sin(xRot_Rad/2)*sin(zRot_Rad/2)*cos(yRot_Rad/2)+sin(yRot_Rad/2)*cos(xRot_Rad/2)*cos(zRot_Rad/2);
    q[3] = sin(xRot_Rad/2)*sin(yRot_Rad/2)*cos(zRot_Rad/2)+sin(zRot_Rad/2)*cos(xRot_Rad/2)*cos(yRot_Rad/2);

    return;

}

void angles_to_quaternions_3DRotation(MYFLOAT q[], MYFLOAT xRot, MYFLOAT yRot,MYFLOAT zRot)
{
    MYFLOAT xRot_Rad = xRot * (pi/180.0);  //if(xRot_Rad>pi) xRot_Rad -= 2*pi;
    MYFLOAT yRot_Rad = yRot * (pi/180.0);  //if(yRot_Rad>pi) yRot_Rad -= 2*pi;
    MYFLOAT zRot_Rad = zRot * (pi/180.0);  //if(zRot_Rad>pi) zRot_Rad -= 2*pi;

    MYFLOAT R[3][3];

    const MYFLOAT cosAngle1 = cosf( xRot_Rad ),  sinAngle1 = sinf( xRot_Rad );
    const MYFLOAT cosAngle2 = cosf( yRot_Rad ),  sinAngle2 = sinf( yRot_Rad );
    const MYFLOAT cosAngle3 = cosf( zRot_Rad ),  sinAngle3 = sinf( zRot_Rad );

    // Repeated calculations (for efficiency)
    MYFLOAT s1c3 = sinAngle1 * cosAngle3;
    MYFLOAT s3c1 = sinAngle3 * cosAngle1;
    MYFLOAT s1s3 = sinAngle1 * sinAngle3;
    MYFLOAT c1c3 = cosAngle1 * cosAngle3;

    const int i = 0;
    const int j = 1;
    const int k = 2;

    R[i][i] =  cosAngle2 * cosAngle3;
    R[i][j] = -sinAngle3 * cosAngle2;
    R[i][k] =  sinAngle2;
    R[j][i] =  s3c1 + sinAngle2 * s1c3;
    R[j][j] =  c1c3 - sinAngle2 * s1s3;
    R[j][k] = -sinAngle1 * cosAngle2;
    R[k][i] =  s1s3 - sinAngle2 * c1c3;
    R[k][j] =  s1c3 + sinAngle2 * s3c1;
    R[k][k] =  cosAngle1 * cosAngle2;

    const MYFLOAT tr = R[i][i] + R[j][j] + R[k][k];
    if( tr >= R[0][0]  &&  tr >= R[1][1]  &&  tr >= R[2][2] )
    {
        q[0] = 1 + tr;
        q[1] = R[2][1] - R[1][2];
        q[2] = R[0][2] - R[2][0];
        q[3] = R[1][0] - R[0][1];

        // Check if R[0][0] is largest along the diagonal
    }
    else if( R[0][0] >= R[1][1]  &&  R[0][0] >= R[2][2]  )
    {
        q[0] = R[2][1] - R[1][2];
        q[1] = 1 - (tr - 2*R[0][0]);
        q[2] = R[0][1]+R[1][0];
        q[3] = R[0][2]+R[2][0];

        // Check if R[1][1] is largest along the diagonal
    }
    else if( R[1][1] >= R[2][2] )
    {
        q[0] = R[0][2] - R[2][0];
        q[1] = R[0][1] + R[1][0];
        q[2] = 1 - (tr - 2*R[1][1]);
        q[3] = R[1][2] + R[2][1];

        // R[2][2] is largest along the diagonal
    }
    else
    {
        q[0] = R[1][0] - R[0][1];
        q[1] = R[0][2] + R[2][0];
        q[2] = R[1][2] + R[2][1];
        q[3] = 1 - (tr - 2*R[2][2]);
    }

    // Scale to unit length
    int tmpi; //change i to tmpi as 'i' has ben defined above.
    MYFLOAT scale = 0.0;
    for (tmpi = 0; tmpi < 4; tmpi++)
        scale += q[tmpi] * q[tmpi];
    scale = 1.0/std::sqrt(scale);
    if( q[0] < 0 )  scale = -scale;   // canonicalize
    for (tmpi = 0; tmpi < 4; tmpi++)
        q[tmpi] *= scale;

    return;
}

void slerp_zhi(MYFLOAT q1[], MYFLOAT q2[], MYFLOAT t, MYFLOAT q_sample[])
{
    MYFLOAT cos_omega = dot_multi_normalized(q1, q2);
    MYFLOAT omega, theta;
    MYFLOAT c1,  c2;
    bool flag = false;

    omega = acos(cos_omega);
    if (cos_omega<0)
    {
        flag = true;
        omega = omega - pi;
    }

    theta = t*omega;
    double sin_omega = sin(omega);
    if (fabs(sin_omega)<0.001) //use linear interpolation when the angle is close to 0
    {
        c1 = 1.0-t;
        c2 = t;
    }
    else
    {
        c1 = sin(omega - theta)/sin_omega;
        c2 = sin(theta)/sin_omega;
    }

    if (flag)
        c2 = -c2; //equivalent to negative of one end of q1 or q2 (in this case, it is q2) for the interpolation below

    printf("slerp result is (cos_t=%f, omega=%f, flag=%d, angle=%f, c1=%f, c2=%f)\n", cos_omega, omega, int(flag), omega/pi*180.0, c1, c2);

    MYFLOAT scale = 0;
    for(int i= 0; i<4;i++)
    {
        q_sample[i] = c1*q1[i] + c2*q2[i];
        scale += q_sample[i] * q_sample[i];
    }

    scale = 1.0/std::sqrt(scale);

    for (int i = 0; i < 4; i++)
    {
        q_sample[i] *= scale;
    }

    printf("current t=%f, current angle with p1=%f, current angle with p2=%f. sum angle=%f. \n\n",
           t, acos(dot_multi_normalized(q1, q_sample))/pi*180.0, acos(dot_multi_normalized(q_sample, q2))/pi*180.0,
           acos(dot_multi_normalized(q1, q_sample))/pi*180.0 + acos(dot_multi_normalized(q_sample, q2))/pi*180.0);

    return;
}

void quaternions_to_angles(MYFLOAT Rot_current[], MYFLOAT q_sample[])
{
  /*  MYFLOAT rot_x = atan2f(2.0*(q_sample[0]*q_sample[1]+q_sample[2]*q_sample[3]), 1.0-2.0*(q_sample[1]*q_sample[1]+q_sample[2]*q_sample[2]));
    MYFLOAT rot_y = asinf(2.0*(q_sample[0]*q_sample[2]-q_sample[3]*q_sample[1]));
    MYFLOAT rot_z = atan2f(2.0*(q_sample[0]*q_sample[3]+q_sample[1]*q_sample[2]), 1.0-2.0*(q_sample[2]*q_sample[2]+q_sample[3]*q_sample[3]));*/

    MYFLOAT rot_x = atan2f(-2.0*(q_sample[2]*q_sample[3])+2.0*(q_sample[0]*q_sample[1]), 1.0-2.0*(q_sample[1]*q_sample[1]+q_sample[2]*q_sample[2]));
    MYFLOAT rot_y = asinf(2.0*(q_sample[1]*q_sample[3]+q_sample[0]*q_sample[2]));
    MYFLOAT rot_z = atan2f(-2.0*(q_sample[1]*q_sample[2])+2*(q_sample[0]*q_sample[3]), 1.0-2.0*(q_sample[2]*q_sample[2]+q_sample[3]*q_sample[3]));

    Rot_current[0] = rot_x * (180.0/pi); // if(Rot_current[0]<0) Rot_current[0] = 360.0 + Rot_current[0];
    Rot_current[1] = rot_y * (180.0/pi);  // if(Rot_current[0]<0) Rot_current[1] = 360.0 + Rot_current[1];
    Rot_current[2] = rot_z * (180.0/pi);  //if(Rot_current[0]<0) Rot_current[2] = 360.0 + Rot_current[2];
}

void quaternions_to_angles_3DRotation(MYFLOAT Rot_current[], MYFLOAT q[])
{
    MYFLOAT q00 = q[0]*q[0], q11=q[1]*q[1], q22=q[2]*q[2], q33=q[3]*q[3];
    MYFLOAT q01 = q[0]*q[1], q02=q[0]*q[2], q03=q[0]*q[3];
    MYFLOAT q12 = q[1]*q[2], q13=q[1]*q[3], q23=q[2]*q[3];

    MYFLOAT R[3][3];

    int i = 0;
    int j = 1;
    int k = 2;

    R[i][i] = q00+q11-q22-q33;
    R[i][j] = 2*(q12-q03);
    R[i][k] = 2*(q13+q02);
    R[j][i] = 2*(q12+q03);
    R[j][j] = q00-q11+q22-q33;
    R[j][k] = 2*(q23-q01);
    R[k][i] = 2*(q13-q02);
    R[k][j] = 2*(q23+q01);
    R[k][k] = q00-q11-q22+q33;

    MYFLOAT rot_x = atan2f(R[1][2],R[2][2]);
    MYFLOAT rot_y = atan2f(-R[0][2], -sqrt(R[0][0]*R[0][0]+R[0][1]*R[0][1]));
    MYFLOAT rot_z = atan2f(sinf(rot_x)* R[2][0] - cosf(rot_x)*R[1][0], cosf(rot_x)*R[1][1] - sinf(rot_x)*R[2][1]);

    Rot_current[0] = rot_x * (180.0/pi);
    Rot_current[1] = rot_y * (180.0/pi);
    Rot_current[2] = rot_z * (180.0/pi);

    return;
}
MYFLOAT dot_multi(MYFLOAT q1[], MYFLOAT q2[])
{
    MYFLOAT result = 0;

    for(int i= 0; i<4;i++)
    {
        result += q1[i] * q2[i];
    }

    return result;
}

MYFLOAT dot_multi_normalized(MYFLOAT q1[], MYFLOAT q2[])
{
    MYFLOAT result = 0.0, r1=0.0, r2=0.0;

    for(int i= 0; i<4;i++)
    {
        result += q1[i] * q2[i];
        r1 += q1[i]*q1[i];
        r2 += q2[i]*q2[i];
    }

    return result/sqrt(r1*r2);
}
