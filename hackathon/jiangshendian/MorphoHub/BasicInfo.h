#ifndef BASICINFO_H
#define BASICINFO_H
#include <QtGui>
using namespace std;
enum AnnotationProtocolFunction{
    ApfRead=0,
    File=1,
    Update=2,
    Commit=3,
    CommitAndFile=4,
    Check=5,
    CheckAndFile=6,
    Skip=7,
    SkipAndFile=8,
    Rollback=9,
    RollbackAndFile=10,
    Reassign=11,
    ReassignAndFile=12,
    Release=13
};

enum AuthorPriority{
    APvisitor=0,
    APannotate=1,
    APjuniorcheck=2,
    APseniorcheck=3,
    APAdministrater=4
};
enum AnnotationProtocolLevel{
    Labeled,
    AutoTraced,
    UnAssigned,
    Assigned1,
    L1A,
    L1ACheck,
    L1B,
    L1C,
    L1CCheck,
    L1D,
    Assigned2,
    L2A,
    L2ACheck,
    L2B,
    L2C,
    L2CCheck,
    L2D
};

struct SourceData{
    QString Name;
    QString ParentDir;
    QString Comments;
    QStringList childlist;
    SourceData(){
        Name=ParentDir=Comments="";
        childlist.clear();
    }
    SourceData(const QString& name,const QString& pdir){
        Name=name;ParentDir=pdir;Comments="";
        childlist<<"Somalist"<<"Undo"<<"Finished"<<"AutoTraced"<<"NeuronArchives";
    }
};

struct Annotationlevel{
    QString Name;
    QString ParentDir;
    QString Comments;
    QStringList childlist;
    Annotationlevel(){
        Name=ParentDir=Comments="";
        childlist.clear();
    }
    Annotationlevel(const QString& name,const QString& pdir)
    {
        Name=name;ParentDir=pdir;Comments="";
    }
};

struct ArchitechureofMorphoHub{
    QList<SourceData> basicData;
    QStringList originForder;
    QStringList initworkingspaceTablist;
    QList<Annotationlevel> finished;
    QList<Annotationlevel> workingSpace;
    ArchitechureofMorphoHub(){
        basicData.clear();
        originForder<<"Brain"<<"WorkingSpace"<<"Finished"<<"Configuration"<<"Configuration/BrainConf"<<"Configuration/WorkingSpace_Conf"<<"Release"<<"Registration";
        initworkingspaceTablist<<"L1A"<<"L1D"<<"L2A"<<"L2D";
        QStringList flevel;
        flevel<<"L1"<<"L2";
        for(int i=0;i<flevel.size();i++)
        {
            Annotationlevel tmplevel;
            tmplevel.Name=flevel.at(i);
            tmplevel.ParentDir="Finished";
            tmplevel.Comments="";
            finished.append(tmplevel);
        }
        QStringList wlevel;
        wlevel<<"Assigned1"<<"L1A"<<"L1ACheck"<<"L1B"<<"L1C"<<"L1CCheck"<<"L1D"
             <<"Assigned2"<<"L2A"<<"L2ACheck"<<"L2B"<<"L2C"<<"L2CCheck"<<"L2D"
            <<"QuestionZone";
        for(int i=0;i<wlevel.size();i++)
        {
            Annotationlevel tmplevel;
            tmplevel.Name=wlevel.at(i);
            if(tmplevel.Name.compare("Assigned1")==0||
                    tmplevel.Name.compare("Assigned2")==0)
            {
                tmplevel.childlist.append("Annotator");
                tmplevel.childlist.append("Priority");
            }
            tmplevel.ParentDir="WorkingSpace";
            tmplevel.Comments="";
            workingSpace.append(tmplevel);
        }
    }
    ArchitechureofMorphoHub(QString mtype){
        if(mtype.compare("minimal")==0)
            ArchitechureofMorphoHub();
        basicData.clear();
        originForder<<"Brain"<<"WorkingSpace"<<"Finished"<<"Configuration"<<"Configuration/BrainConf"<<"Configuration/WorkingSpace_Conf";
        initworkingspaceTablist<<"L1A"<<"L1D"<<"L2A"<<"L2D";
        QStringList flevel;
        flevel<<"L1"<<"L2";
        for(int i=0;i<flevel.size();i++)
        {
            Annotationlevel tmplevel;
            tmplevel.Name=flevel.at(i);
            tmplevel.ParentDir="Finished";
            tmplevel.Comments="";
            finished.append(tmplevel);
        }
        QStringList wlevel;
        wlevel<<"L1A"<<"L1ACheck"<<"L1D"
             <<"L2A"<<"L2ACheck"<<"L2D"
            <<"QuestionZone";
        for(int i=0;i<wlevel.size();i++)
        {
            Annotationlevel tmplevel;
            tmplevel.Name=wlevel.at(i);
            tmplevel.ParentDir="WorkingSpace";
            tmplevel.Comments="";
            workingSpace.append(tmplevel);
        }
    }
};

struct AnnotationProtocol{
    QString protocolName;
    QString protocolComments;
    QStringList protocolLevel;
    QStringList archiveLevel;
    QStringList commitLevel;//commitlevel is the start level of commit function
    QStringList checkLevel;//checklevel is the start level of check function
    QStringList skipLevel;//skiplevel is the start level of skip function
    QStringList finishedLevel;//finishedlevel is the end level of Finished function
    QStringList archiveFinishedLevel;//end level of Archive function
    QStringList rollbackLevel;//start level of rollback level
    QStringList ReconstructionConfItems;
    QString ApConfPath;
    ArchitechureofMorphoHub architechure;
    QHash<QString,AnnotationProtocolFunction> protocolrules;
    AnnotationProtocol(){
        protocolName="InitAp";//Annotation protocol one
        protocolComments=ApConfPath="";
        protocolLevel<<"Assigned1"<<"L1A"<<"L1ACheck"<<"L1B"<<"L1C"<<"L1CCheck"<<"L1D"
                       <<"Assigned2"<<"L2A"<<"L2ACheck"<<"L2B"<<"L2C"<<"L2CCheck"<<"L2D";
        archiveLevel<<"L1A"<<"L1B"<<"L1C"<<"L1D"
                   <<"L2A"<<"L2B"<<"L2C"<<"L2D";
        commitLevel<<"Assigned1"<<"L1ACheck"<<"L1B"<<"L1CCheck"
                  <<"Assigned2"<<"L2ACheck"<<"L2B"<<"L2CCheck";
        skipLevel<<"Assigned1"<<"L1CCheck"<<"L2ACheck";
        checkLevel<<"L1A"<<"L1C"<<"L2A"<<"L2C";
        finishedLevel<<"L1D"<<"L2D";
        archiveFinishedLevel<<"L1"<<"L2";
        rollbackLevel<<"L1ACheck"<<"L1CCheck"
                    <<"L2ACheck"<<"L2CCheck";
        ReconstructionConfItems <<"BrainID"  << "NeuronID" << "Author" << "Checkerlist" << "LevelID"<<"Time"<<"ParentDirName"<<"FileName";
        //initprotocolrules();
    }
    void initprotocolrules(){
        for(int i=0;i<protocolLevel.size();i++)
        {
            QString startlevel=protocolLevel.at(i);
            for(int j=0;j<protocolLevel.size();j++)
            {
                QString endlevel=protocolLevel.at(j);
                QString startlevel_endlevel=startlevel+"_"+endlevel;
                protocolrules[startlevel_endlevel]=ApfRead;
            }
        }
    }
    QString APFRollback(const QString& inputlevel,bool moveforward=true)
    {
        QString outLevel;
        if(!inputlevel.isEmpty())
        {
            int levelindex=rollbackLevel.indexOf(inputlevel);
            if(levelindex!=-1)
            {
                int plevel=protocolLevel.indexOf(inputlevel);
                if(moveforward)
                {
                    outLevel=protocolLevel.at(plevel-2);
                    if(plevel==2||plevel==9)
                        outLevel+="/Annotator";
                }
                else
                    outLevel=protocolLevel.at(plevel-1);//this is for deleting middle level at Neuron Archives
            }
        }
        return outLevel;
    }
    QString APFReassign(const QString& inputlevel,bool moveforward=true)
    {
        QString outLevel;
        if(!inputlevel.isEmpty())
        {
            int levelindex=rollbackLevel.indexOf(inputlevel);
            if(levelindex!=-1)
            {
                int plevel=protocolLevel.indexOf(inputlevel);
                int AS2levelindex=protocolLevel.indexOf("Assigned2");
                if(moveforward)
                {
                    if(plevel<AS2levelindex)
                        outLevel=protocolLevel.at(0);
                    else if(plevel>AS2levelindex)
                        outLevel=protocolLevel.at(AS2levelindex);
                    outLevel+="/Priority";
                }
                else
                {
                    outLevel=protocolLevel.at(plevel-1);//have error here
                }
            }
        }
        return outLevel;
    }
    QString APFCommit(const QString& inputlevel)
    {
        QString outLevel;
        if(!inputlevel.isEmpty())
        {
            int levelindex=commitLevel.indexOf(inputlevel);
            if(levelindex!=-1)
            {
                int plevel=protocolLevel.indexOf(inputlevel);
                outLevel=protocolLevel.at(plevel+1);
            }
        }
        return outLevel;
    }
    QString APFCheck(const QString& inputlevel)
    {
        QString outLevel;
        if(!inputlevel.isEmpty())
        {
            int levelindex=checkLevel.indexOf(inputlevel);
            if(levelindex!=-1)
            {
                int plevel=protocolLevel.indexOf(inputlevel);
                outLevel=protocolLevel.at(plevel+1);
            }
        }
        return outLevel;
    }
    bool APFArchive(const QString& inputlevel)
    {
        if(!inputlevel.isEmpty())
        {
            int levelindex=archiveLevel.indexOf(inputlevel);
            if(levelindex!=-1)
                return true;
        }
        return false;
    }

    QString APFSkip(const QString& inputlevel)
    {
        QString outLevel;
        if(!inputlevel.isEmpty())
        {
            int levelindex=skipLevel.indexOf(inputlevel);
            switch(levelindex)
            {
            case 0:
                outLevel=protocolLevel.at(8);
                break;
            case 1:
                outLevel=protocolLevel.at(13);
                break;
            case 2:
                outLevel=protocolLevel.at(13);
                break;
            case -1:
                break;
            }
        }
        return outLevel;
    }
    QString APFFinished(const QString& inputlevel)
    {
        QString outLevel;
        if(!inputlevel.isEmpty())
        {
            int levelindex=finishedLevel.indexOf(inputlevel);
            if(levelindex!=-1)
                outLevel=archiveFinishedLevel.at(levelindex);
        }
        return outLevel;
    }
};

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
    AuthorPriority priority;/*read=0,annotate=1,Acheck=2,Ccheck=3,Admin=4*/
    QString workingplace;
    Annotator(){
        UserID=Name=workingplace="";
        priority=APvisitor;
    }
};

struct ReconstructionInfo:SomaConfInfo
{
    QString levelID;
    Annotator author;
    QString checkers;
    QString updateTime;
    QString fatherDirName; //dir name for reconstructions
    QString fileName;
    ReconstructionInfo(){
        levelID=checkers=updateTime=fatherDirName=fileName="";        
    }
    bool alreadyInit(){
        if(!SdataID.isEmpty()&&!SomaID.isEmpty()&&!levelID.isEmpty()&&
                !author.UserID.isEmpty()&&!updateTime.isEmpty()&&!fileName.isEmpty()
                &&!fatherDirName.isEmpty())
            return true;
        return false;
    }
};
QList<SomaConfInfo> getSomaConf(const QString& scandstpath);
QList<SourceDataInfo> getSourceDatalist(const QString& scandstpath);
QList<Annotator> getAnnotatorlist(const QString& confpath);
bool WriteAnnotatorConfToFile(const QString& confpath,QList<Annotator> &inputsdlist);
bool WriteSourceDataToFile(const QString& confpath,QList<SourceDataInfo> &inputsdlist);
bool WriteSomalistConfToFile(const QString& confpath,QList<SomaConfInfo> &inputsdlist);
QStringList readAnoFile(const QString& filename);
bool writeAnoFile(const QString& inputanofile, const QStringList& inputlist);
#endif // BASICINFO_H
