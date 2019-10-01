#ifndef BASICINFO_H
#define BASICINFO_H
#include <QtGui>
using namespace std;
struct SourceDataInfo{
    QString SdataID;//Image data ID, like BrainID
    QString SDSize;
    QString SDResolution;
    QString SDBit;
    QString SDState;
    QString SDComments;
    QString SDPriority;/*low=0,normal=1,high=2,super high=3*/
    QString SDLabeledSomaNumber;
    SourceDataInfo(){
        SdataID=SDSize=SDResolution=SDState=SDComments="";
        SDPriority=SDBit="0";
        SDLabeledSomaNumber="0";
    }
};
struct SomaConfInfo: public SourceDataInfo
{
    QString SomaID;
    QString CellName;
    QString Zprojection;
    QString Xcoordinate;
    QString SomaComments;
    QString Ycoordinate;
    QString SomaState;//can be specified by enum
    QString SomaLocation;
    QString CellType;
    QString SomaPriority;/*low=0,normal=1,high=2,super high=3*/
    SomaConfInfo(){
        SomaID=CellName=Zprojection=Xcoordinate=Ycoordinate=SomaComments=SomaState=SomaLocation=CellType="";
        SomaPriority="0";
    }
};
struct Annotator{
    QString UserID;
    QString Name;
    int priority;/*read=0,annotate=1,Acheck=2,Ccheck=3,Admin=4*/
    QString workingplace;
    Annotator(){
        UserID=Name=workingplace="";
        priority=0;
    }
};

struct ReconstructionInfo:SomaConfInfo
{
    QString levelID;
    Annotator author;
    QString checkers;
    QString updateTime;
    QString fatherDirName; //dir name for reconstructions
    QString filePath;
    ReconstructionInfo(){
        levelID=checkers=updateTime=fatherDirName=filePath="";
    }
};
QList<SomaConfInfo> getSomaConf(const QString& scandstpath);
QList<SourceDataInfo> getSourceDatalist(const QString& scandstpath);
QList<Annotator> getAnnotatorlist(const QString& confpath);

#endif // BASICINFO_H
