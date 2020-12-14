#ifndef BASIC_CONF_H
#define BASIC_CONF_H
#include <QtGui>
#include "v3d_basicdatatype.h"
using namespace std;
/*Definition of User*/
enum AuthorPriority{
    APvisitor=0,
    APannotate=1,
    APjuniorcheck=2,
    APseniorcheck=3,
    APAdministrater=4
};
struct Annotator{
    QString UserID;
    QString Name;
    AuthorPriority priority;/*read=0,annotate=1,Acheck=2,Ccheck=3,Admin=4*/
    QString workingplace;
    QStringList mFeatures;
    QStringList muPriority;
    Annotator(){
        UserID=Name=workingplace="";
        priority=APvisitor;
        mFeatures<<"ID"<<"Name"<<"Workingplace"<<"Priority";
        muPriority<<"Visitor"<<"Annotator"<<"Juniorchecker"<<"SeniorChecker"<<"Administrater";
    }
};
struct mUsers{
    QList<Annotator> listUsers;
    QString mGroupName,mdbPath,mdbconf;
    mUsers(){
        listUsers.clear();
        mGroupName=mdbPath=mdbconf="";
    }
};
QList<Annotator> getAnnotatorlist(const QString& confpath);
bool writeAnnotatorConfToFile(const QString& confpath,QList<Annotator> &inputsdlist);

struct mImageResolution{
    double rx,ry,rz;
    mImageResolution(){
        rx=0.0;ry=0.0;rz=0.0;
    }
    void setmIR(double irx,double iry,double irz)
    {
        rx=irx;ry=iry;rz=irz;
    }
    bool isAvailable(){
        if(rx>0&&ry>0&&rz>0)
            return true;
        return false;
    }
};
struct mImageSize{
    unsigned long sx,sy,sz;
    mImageSize(){
        sx=0;sy=0;sz=0;
    }
    void setmIS(unsigned long isx,unsigned long isy,unsigned long isz)
    {
        sx=isx;sy=isy;sz=isz;
    }
    bool isAvailable(){
        if(sx>0&&sy>0&&sz>0)
            return true;
        return false;
    }
};
struct mImage{
    /*It's a unit struct for image dataset, can be used for describing whole-brain dataset*/
    QString name,comments,mformat,dirname,objectType;//objectType: mouse, fly or monkey; path: the file path of image dataset
    unsigned int id,bit;
    mImageResolution mIR;
    mImageSize mIS;

    QStringList mFeatures;
    mImage(){
        name=objectType=mformat=dirname=comments="";
        id=bit=0;
        mIR.setmIR(0.0,0.0,0.0);mIS.setmIS(0,0,0);
        mFeatures<<"ID"<<"Name"<<"Object"<<"Bit"<<"DirName"<<"Format"<<
                   "SizeX"<<"SizeY"<<"SizeZ"<<
                   "ResolutionX"<<"ResolutionY"<<"ResolutionZ";
    }
};
struct mImageDB{
    QList<mImage> listImage;
    QString mdbName,mdbPath,mdbconf;//mdbPath: image datasets path
    mImageDB(){
        listImage.clear();
        mdbName=mdbconf=mdbPath="";
    }
};
QList<mImage> getImagelist(const QString& inpath);
bool writeImagelistToFile(const QString& topath,QList<mImage> &inlist);
/*morphoHub will load or set a new client based on this.
 *
*/
struct mFileSystem{
    QStringList basicDirs;
    QString imgDBPath,dbPath;//basic img DB Path: image datasets
    QString imgconfPath;//configuration or information of image datasets
    QString userconfPath;//configuration of users
    mFileSystem(){
        basicDirs.clear();
        basicDirs<<"Morphometry"<<"Component"<<"User"<<"Configuration";
        imgDBPath=imgconfPath=userconfPath="";
        dbPath="";
    }
    QString getImgConfPath()
    {
        if(dbPath.isEmpty())
            return imgconfPath;
        QDir path(dbPath);
        if(!path.exists())
            return imgconfPath;
        imgconfPath=dbPath+"/Configuration/image_datasets.conf";
        return imgconfPath;
    }
    QString getUserConfPath()
    {
        if(dbPath.isEmpty())
            return userconfPath;
        QDir path(dbPath);
        if(!path.exists())
            return userconfPath;
        userconfPath=dbPath+"/Configuration/users.conf";
        return userconfPath;
    }

    bool setImgPath(const QString& inpath)
    {
        if (inpath.isEmpty())
            return false;
        QDir path(inpath);
        if(!path.exists())
            return false;
        imgDBPath=inpath;
        return true;
    }
    bool setDBPath(const QString& inpath)
    {
        if (inpath.isEmpty())
            return false;
        QDir path(inpath);
        if(!path.exists())
            return false;
        dbPath=inpath;
        return true;
    }
    //come here later, for loading conf of basic dirs from file.
};

#endif // BASIC_CONF_H
