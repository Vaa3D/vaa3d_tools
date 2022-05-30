#ifndef BASICINFO_H
#define BASICINFO_H

#include <QString>

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


struct Handler{
    QString UserID;
    QString Name;
    QString workingplace;
    Handler(){
        UserID=Name="";

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
struct QueryDataInfo:SomaConfInfo
{
    Handler author;
    QString updateTime;
    QString fatherDirName; //dir name for reconstructions
    QString fileName;
    QueryDataInfo(){
        updateTime=fatherDirName=fileName="";
    }
    bool alreadyInit(){
        if(!SdataID.isEmpty()&&!SomaID.isEmpty()&&
                !author.UserID.isEmpty()&&!updateTime.isEmpty()&&!fileName.isEmpty()
                &&!fatherDirName.isEmpty())
            return true;
        return false;
    }
};
class basicInfo
{
public:
    basicInfo();
};

#endif // BASICINFO_H
