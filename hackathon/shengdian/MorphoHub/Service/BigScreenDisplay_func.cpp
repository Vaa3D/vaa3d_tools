//#include 'BigScreenDisplay_func.h'
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <QApplication>
#include <QTime>
#include <QtGui>
#include <cmath>
#include "BigScreenDispaly_func.h"
#include "basic_surf_objs.h"

const double pi = 3.1415926535897;
QString m_InputfolderName="";
QStringList displaySWCList,allSWCFile;
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


//void bigscreenDisplay(V3DPluginCallback2 &callback, QWidget *parent)
//{
//    BigscreenControlDialog bigscreencontroldialog(parent);
//    if(bigscreencontroldialog.exec()!=QDialog::Accepted)
//        return;
//    bigscreencontroldialog.update();
//    DisplayPARA controlPara=bigscreencontroldialog.controlPara;
//    MethodForBigScreenDisplay(callback,parent,controlPara);
//}
void HelpText()
{
    QString helptext=
            "This plugin is developed by Shengdian Jiang. 2018-10-15 <br>"
            "The BigScreenDisplay plugin is used to display specity number of new finished annotation file at screen.<br>"
            "<br>============================================="
            "<H2>Help Information</H2>"
            "=============================================<br>"
            "<br><table border=\"1\">"
            "<tr><td>Choose the display directory </td><td>The folder you chosed should include all the SWC/ESWC file.</td></tr>"
            "<tr><td>start point X </td><td>The start pixel of the height of display panel.</td></tr>"
            "<tr><td>start point Y </td><td>The start pixel of the width of display panel.</td></tr>"
            "<tr><td>display number </td><td>Display number is the  displayed number of annotation file.</td></tr>"
            "<tr><td>Combine checkbox </td><td>Set it true if you want to display the combined file.</td></tr>"
            "<tr><td>Down Sample(combined) checkbox </td><td>Set it true if you want to down sample the combined file.</td></tr>"
            "<tr><td>Window Size </td><td>The Window Size of the displayed window.</td></tr>"
            "<tr><td>update interval </td><td>The plugin will update the window every 'update interval' time.</td></tr>"
            "<tr><td>down sample rate </td><td>Down sample rate of the combined file.</td></tr>"
            "<tr><td>Choose the anchor file </td><td>For the zmoive plugin.</td></tr>"
            "<tr><td>zmovie sampling rate </td><td>For the zmoive plugin.</td></tr>"
            "</table><br>"
            "<H3>Note</H3>"
            "1.SWC/ESWC file should be named like 'BrainID_NeuronID_FirAnnotator_SecAnnotator_FinalAnnotator_stamp_Year_Month_Day_Hour_Min' format.<br>"
            "for example:18454_00098_SYY_YLL_LJL_stamp_2018_1_22_14_36.ano.eswc.<br>"
            "2.If you want to display Level1/Level2 annotation files of different brain, you'd better set the combine checkbox false."
            ;
    QTextEdit *textEdit=new QTextEdit(helptext);textEdit->setWindowTitle("BigScreen Control Document");
    //textEdit->setDocumentTitle("BigScreen Control Document");
    textEdit->resize(850, 700); //use the QWidget function
    textEdit->setReadOnly(true);
    textEdit->setFontPointSize(16);
    textEdit->show();
}

void CheckFileandFloder(DisplayPARA controlPara)
{
//    QStringList /*outSWCFile,*/allSWCFile;
//    allSWCFile.clear();

    QDir displaydir(controlPara.openfolder);
    QString supposeSuffixeswc="eswc";
    QString supposeSuffixswc="swc";
    QString supposeSuffixESWC="ESWC";
    QString supposeSuffixSWC="SWC";
    foreach(QFileInfo mfi,displaydir.entryInfoList())
    {
        if(mfi.isFile())
        {
            //cout<<"File name is "<<mfi.fileName().toStdString()<<endl;
            if(QString::compare(mfi.suffix(), supposeSuffixeswc, Qt::CaseInsensitive) == 0
                    ||QString::compare(mfi.suffix(), supposeSuffixswc, Qt::CaseInsensitive) == 0
                    ||QString::compare(mfi.suffix(), supposeSuffixESWC, Qt::CaseInsensitive) == 0
                    ||QString::compare(mfi.suffix(), supposeSuffixSWC, Qt::CaseInsensitive) == 0)
            {
                allSWCFile+=mfi.absoluteFilePath();
            }
        }
        else
        {
            if(mfi.fileName()=="."|| mfi.fileName()=="..")
                continue;
            cout<<"dir is "<<mfi.absoluteFilePath().toStdString()<<endl;
            DisplayPARA controlParaTemp;
            //controlParaTemp.displaynumber=controlPara.displaynumber;
            controlParaTemp.openfolder=mfi.absoluteFilePath();
            CheckFileandFloder(controlParaTemp);
        }
    }
    if(allSWCFile.size()==0)
        cout<<"can't find any swc or eswc file."<<endl;
    //return allSWCFile;

}
QStringList FindNewSWCFile(QStringList allSWCFile,int displayNumber)
{
    QStringList outSWCFile;
    outSWCFile.clear();
    if(allSWCFile.size()==0)
    {
        cout<<"can't find any swc or eswc file."<<endl;
        return outSWCFile;
    }
    map <int,QDateTime> swcFinishDate;
    for(int i=0;i<allSWCFile.size();i++)
    {

        QFileInfo swcFile(allSWCFile.at(i));
        QString swcBaseName=swcFile.baseName();
        if(swcBaseName.toStdString().find("_stamp_")!=string::npos)
        {
            QStringList swcBaseNameSplit=swcBaseName.split("_stamp_");
            if(!swcBaseNameSplit.size())
            {
                cout<<"SWC file don't have time stamp."<<endl;
                continue;
//                return outSWCFile;
            }
            QString swcDate=swcBaseNameSplit[swcBaseNameSplit.size()-1];
            QStringList swcDateTime=swcDate.split("_");
            if(swcDateTime.size()!=5)
                continue;
            QDateTime theswcdate(QDate(swcDateTime[0].toInt(),swcDateTime[1].toInt(),swcDateTime[2].toInt()),QTime(swcDateTime[3].toInt(),swcDateTime[4].toInt()));
            swcFinishDate[i]=theswcdate;
            //cout<<"swc date is "<<theswcdate.toString("yyyy_MM_dd_hh_mm").toStdString()<<endl;
            //do the date count.
        }
        else
        {
            cout<<"SWC file don't have time stamp."<<endl;
            continue;
//            return outSWCFile;
        }
    }
    //sort the swc file based on finished datetime.
    map<int,int> sortSWCresult;
    for(int d=0;d<displayNumber;d++)
    {

        if(d==0)
        {
            sortSWCresult[d]=0;
            swcFinishDate[allSWCFile.size()+1]=swcFinishDate[d];
            for(int i=0;i<allSWCFile.size();i++)
            {
                if(swcFinishDate[i]>=swcFinishDate[allSWCFile.size()+1])
                {
                    swcFinishDate[allSWCFile.size()+1]=swcFinishDate[i];
                    sortSWCresult[d]=i;
                }
            }
        }
        else
        {
            swcFinishDate[allSWCFile.size()+1]=swcFinishDate[0];
            for(int i=0;i<allSWCFile.size();i++)
            {
                if(swcFinishDate[i]>=swcFinishDate[allSWCFile.size()+1]&&swcFinishDate[i]<swcFinishDate[sortSWCresult[d-1]])
                {
                    swcFinishDate[allSWCFile.size()+1]=swcFinishDate[i];
                    sortSWCresult[d]=i;
                }
            }
        }
    }
    //extract the new finished swc file from allswcfile to outswcfile
    for(int d=0;d<displayNumber;d++)
    {
        outSWCFile+=allSWCFile.at(sortSWCresult[d]);
//        cout<<"swc file name is "<<outSWCFile.at(d).toStdString()<<endl;
    }
    return outSWCFile;
}

void MethodForBigScreenDisplay(V3DPluginCallback2 &callback, QWidget *parent,DisplayPARA controlPara)
{
    //int displayNum=controlPara.displaynumber;
    int col=controlPara.displaycolumn;
    int row=controlPara.displayrow;
    int xRez=controlPara.spositionX/*3840*/, yRez=controlPara.spositionY/*2160*/;
    int xdim=controlPara.winsizeX;int ydim=controlPara.winsizeY;
    bool iscombined=controlPara.isneuronCombined;
    int updateInterval=controlPara.updateinterval;

    displaySWCList.clear();allSWCFile.clear();
    CheckFileandFloder(controlPara);
     QStringList allswcList =allSWCFile;
    displaySWCList=FindNewSWCFile(allswcList,controlPara.displaynumber);
    V3dR_MainWindow * new3DWindow = NULL;
    int offsety = -1;
    for(V3DLONG i = 0; i < displaySWCList.size(); i++)
    {
        QString curPathSWC = displaySWCList.at(i);
        QFileInfo curSWCBase(curPathSWC);
        new3DWindow = callback.open3DViewerForSingleSurfaceFile(curPathSWC);
        //reset window title to basename instead of path name
        callback.setWindowDataTitle(new3DWindow,curSWCBase.baseName());
    }
    if(iscombined)
        MethodForCombineSWCDisplay(callback,parent,displaySWCList,controlPara);
    LoadAnchorFile(controlPara);
    QList <V3dR_MainWindow *> cur_list_3dviewer = callback.getListAll3DViewers();

    for (V3DLONG i = 0; i < cur_list_3dviewer.size(); i++)
    {
        if( (i%col)/**xRez*/ ==0)
            offsety++;

        if(i<cur_list_3dviewer.size()-1)
        {
            callback.moveWindow(cur_list_3dviewer.at(i),xRez+(i%col)*xdim,(offsety)*ydim);
            callback.resizeWindow(cur_list_3dviewer.at(i),xdim,ydim);
        }
        else if(iscombined)
        {
            callback.moveWindow(cur_list_3dviewer.at(i),xRez+xdim*col,0);
            callback.resizeWindow(cur_list_3dviewer.at(i),xdim,ydim);
        }
        else
        {
            callback.moveWindow(cur_list_3dviewer.at(i),xRez+(i%col)*xdim,(offsety)*ydim);
            callback.resizeWindow(cur_list_3dviewer.at(i),xdim,ydim);
        }
        callback.setHideDisplayControlButton(cur_list_3dviewer.at(i));
    }
    int samplingRate=controlPara.sampleRate;
    if(samplingRate>0)
    {
        ZmovieMaker(callback,parent,samplingRate);
        int updateCount=0;
        while(true)
        {
            //sleep(updateInterval);
            if(updateCount==updateInterval)
            {
                MethodForUpdateSWCDispaly(callback,parent,controlPara);
                updateCount=0;
            }
            ZmovieMaker(callback,parent,samplingRate);
            updateCount++;
        }
    }

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
    printf("Usage (linux): vaa3d -x BigscreenDisplay -f BigScreenDisplay -i /home/data/SEUAllenJointDataCenter/finished_annotations/17545_finished_neurons/ /home/penglab/Data/jsd/anchorPointFile.txt -p 30 500\" \n");
    printf("Usage (windows): vaa3d_msvc.exe /x BigscreenDisplay /f BigScreenDisplay /i d:/home/data/SEUAllenJointDataCenter/finished_annotations/17545_finished_neurons/ d:/home/penglab/Data/jsd/anchorPointFile.txt /p 30 500\" \n");
    printf("\n                                                           \n");

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
void MethodFunForBigScreenDisplay(V3DPluginCallback2 &callback, QWidget *parent, const V3DPluginArgList & input)
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
    m_InputfolderName = QString(inlist->at(0));
    /*QString*/ /*m_InputfolderName = QFileDialog::getExistingDirectory(parent, QObject::tr("Choose the directory including all finished annotation files "),
                                                                  QDir::currentPath(),
                                                                  QFileDialog::ShowDirsOnly);*/


    int col=5, /*row=3,*/ xRez=3840, yRez=2160;
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
    MethodFunForCombineSWCDisplay(callback,parent,swcList);

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
void MethodFunForCombineSWCDisplay(V3DPluginCallback2 &callback, QWidget *parent,QStringList nameList)
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
void MethodFunForUpdateSWCDispaly(V3DPluginCallback2 &callback, QWidget *parent, const V3DPluginArgList & input)
{
    cout<<"big screen display update"<<endl;
    vector<char*>* inlist = (vector<char*>*)(input.at(0).p);

    if(input.size() != 2)
    {
        printf("Please specify both input folder .\n");
        return /*false*/;
    }

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
void MethodForUpdateSWCDispaly(V3DPluginCallback2 &callback, QWidget *parent,DisplayPARA controlPara)
{
    QList <V3dR_MainWindow *> cur_list_3dviewer = callback.getListAll3DViewers();
    //unsigned int displayNum=9;
    int displayNum=controlPara.displaynumber;
    bool iscombined=controlPara.isneuronCombined;
    bool iscombineddownsample=controlPara.isDownSampleCombined;
    int combineResampleRate=controlPara.downsamplerate;
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
    displaySWCList.clear();allSWCFile.clear();
    CheckFileandFloder(controlPara);
    QStringList allswcList = allSWCFile;
    displaySWCList= FindNewSWCFile(allswcList,controlPara.displaynumber);
    //WriteNewFinishedNeuronsFileName(swcList);
    for(int i=0;i<displaySWCList.size();i++)
    {
        QList<NeuronTree> * new_treeList = callback.getHandleNeuronTrees_Any3DViewer (cur_list_3dviewer.at(i));

        QString curPathSWC = displaySWCList.at(i);
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
    if(iscombined)
    {
        V3dR_MainWindow * surface_combine_win=cur_list_3dviewer.at(cur_list_3dviewer.size()-1);
        QList<NeuronTree> * new_treeList_combine = callback.getHandleNeuronTrees_Any3DViewer (surface_combine_win);
        QString combine_file_name="Combined Latest Finished Neurons";
        int neuronNum = displaySWCList.size();
        for (V3DLONG i=0;i<neuronNum;i++)
        {
            NeuronTree tmp = readSWC_file(displaySWCList.at(i));
            for(int j=0;j<tmp.listNeuron.size();j++)
                tmp.listNeuron[j].type=i+2;
            if(iscombineddownsample)
            {
                NeuronTree resampleSwc=resample(tmp,combineResampleRate);
                new_treeList_combine->push_back(resampleSwc);
            }
            else
            {
                new_treeList_combine->push_back(tmp);
            }
            //new_treeList_combine->push_back(tmp);
        }
        callback.setWindowDataTitle(surface_combine_win, combine_file_name);
        callback.update_NeuronBoundingBox(surface_combine_win);
        callback.update_3DViewer(surface_combine_win);
    }
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
void LoadAnchorFile(DisplayPARA controlPara)
{
    QString fileOpenName = controlPara.openanchorfile;/*QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
                                                        "",
                                                        QObject::tr("Anchor Point File (*.apftxt *.txt *.apf)"
                                                                    ));*/
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
void MethodForCombineSWCDisplay(V3DPluginCallback2 &callback, QWidget *parent,QStringList nameList,DisplayPARA controlPara)
{
    bool isCombiedDownSample=controlPara.isDownSampleCombined;
    bool isDownSample=controlPara.isDownSample;
    int combineResampleRate=controlPara.downsamplerate;
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
        if(isCombiedDownSample&&(!isDownSample))
        {
            NeuronTree resampleSwc=resample(tmp,combineResampleRate);
            new_treeList->push_back(resampleSwc);
        }
        else
        {
            new_treeList->push_back(tmp);
        }
    }

    callback.setWindowDataTitle(surface_win, file_name);
    callback.update_NeuronBoundingBox(surface_win);
    callback.update_3DViewer(surface_win);
}
void resample_path(Segment * seg, double step)
{
    char c;
    Segment seg_r;
    double path_length = 0;
    Point* start = seg->at(0);
    Point* seg_par = seg->back()->p;
    V3DLONG iter_old = 0;
    seg_r.push_back(start);
    while (iter_old < seg->size() && start && start->p)
    {
        path_length += DISTP(start,start->p);
        if (path_length<=seg_r.size()*step)
        {
            start = start->p;
            iter_old++;
        }
        else//a new point should be created
        {
            path_length -= DISTP(start,start->p);
            Point* pt = new Point;
            double rate = (seg_r.size()*step-path_length)/(DISTP(start,start->p));
            pt->x = start->x + rate*(start->p->x-start->x);
            pt->y = start->y + rate*(start->p->y-start->y);
            pt->z = start->z + rate*(start->p->z-start->z);
            pt->r = start->r*(1-rate) + start->p->r*rate;//intepolate the radius
            pt->p = start->p;

            if (rate<0.5)
            {
                pt->type = start->type;
                pt->seg_id = start->seg_id;
                pt->level = start->level;
                pt->fea_val = start->fea_val;
            }
            else
            {
                pt->type = start->p->type;
                pt->seg_id = start->p->seg_id;
                pt->level = start->p->level;
                pt->fea_val = start->p->fea_val;

            }
            seg_r.back()->p = pt;
            seg_r.push_back(pt);
            path_length += DISTP(start,pt);
            start = pt;
        }
    }
    seg_r.back()->p = seg_par;
    for (V3DLONG i=0;i<seg->size();i++)
        if (!seg->at(i)) {delete seg->at(i); seg->at(i) = NULL;}
    *seg = seg_r;
}
NeuronTree resample(NeuronTree input, double step)
{
    NeuronTree result;
    V3DLONG siz = input.listNeuron.size();
    Tree tree;
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = input.listNeuron[i];
        Point* pt = new Point;
        pt->x = s.x;
        pt->y = s.y;
        pt->z = s.z;
        pt->r = s.r;
        pt ->type = s.type;
        pt->seg_id = s.seg_id;
        pt->level = s.level;
        pt->fea_val = s.fea_val;
        pt->p = NULL;
        pt->childNum = 0;
        tree.push_back(pt);
    }
    for (V3DLONG i=0;i<siz;i++)
    {
        if (input.listNeuron[i].pn<0) continue;
        V3DLONG pid = input.hashNeuron.value(input.listNeuron[i].pn);
        tree[i]->p = tree[pid];
        tree[pid]->childNum++;
    }
//	printf("tree constructed.\n");
    vector<Segment*> seg_list;
    for (V3DLONG i=0;i<siz;i++)
    {
        if (tree[i]->childNum!=1)//tip or branch point
        {
            Segment* seg = new Segment;
            Point* cur = tree[i];
            do
            {
                seg->push_back(cur);
                cur = cur->p;
            }
            while(cur && cur->childNum==1);
            seg_list.push_back(seg);
        }
    }
//	printf("segment list constructed.\n");
    for (V3DLONG i=0;i<seg_list.size();i++)
    {
        resample_path(seg_list[i], step);
    }

//	printf("resample done.\n");
    tree.clear();
    map<Point*, V3DLONG> index_map;
    for (V3DLONG i=0;i<seg_list.size();i++)
        for (V3DLONG j=0;j<seg_list[i]->size();j++)
        {
            tree.push_back(seg_list[i]->at(j));
            index_map.insert(pair<Point*, V3DLONG>(seg_list[i]->at(j), tree.size()-1));
        }
    for (V3DLONG i=0;i<tree.size();i++)
    {
        NeuronSWC S;
        Point* p = tree[i];
        S.n = i+1;
        if (p->p==NULL) S.pn = -1;
        else
            S.pn = index_map[p->p]+1;
        if (p->p==p) printf("There is loop in the tree!\n");
        S.x = p->x;
        S.y = p->y;
        S.z = p->z;
        S.r = p->r;
        S.type = p->type;
        S.seg_id = p->seg_id;
        S.level = p->level;
        S.fea_val = p->fea_val;
        result.listNeuron.push_back(S);
    }
    for (V3DLONG i=0;i<tree.size();i++)
    {
        if (tree[i]) {delete tree[i]; tree[i]=NULL;}
    }
    for (V3DLONG j=0;j<seg_list.size();j++)
        if (seg_list[j]) {delete seg_list[j]; seg_list[j] = NULL;}
    for (V3DLONG i=0;i<result.listNeuron.size();i++)
        result.hashNeuron.insert(result.listNeuron[i].n, i);
    cout<<"resample already"<<endl;
    return result;
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
        nameOut<<qs<<endl;
        qDebug()<<qs;
    }

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
