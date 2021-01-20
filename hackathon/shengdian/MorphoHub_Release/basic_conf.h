#ifndef BASIC_CONF_H
#define BASIC_CONF_H
#include <QtGui>
#include "v3d_basicdatatype.h"
#include "basic_surf_objs.h"
using namespace std;
/*Definition of dirs in the first layer of MorphoHub file system*/
enum firstDirsInMFS{
    MORPHOMETRY=0,
    CONFIGURATION=1,
    COMPONENT=2,
    USER=3
};
enum MorphometryType{
    SOMA=0,
    AUTOTRACED=1,
    LEVEL0=2,
    LEVEL1=3,
    LEVEL2=4,
    LEVEL3=5
};

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

/*Image releated data structures*/
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
    QStringList getlistImage_name()
    {
        QStringList outlist;outlist.clear();
        if(listImage.size())
            for(V3DLONG i=0;i<listImage.size();i++)
                outlist.append(listImage.at(i).name);
        return outlist;
    }
    QString getImagePath(const QString& qname)
    {
        QString outpath="";
        QStringList listImage_nameList;listImage_nameList.clear();
        listImage_nameList=getlistImage_name();
        if(listImage_nameList.indexOf(qname)>=0)
            outpath=mdbPath+"/"+listImage.at(listImage_nameList.indexOf(qname)).dirname;
        return outpath;
    }
};
QList<mImage> getImagelist(const QString& inpath);
bool writeImagelistToFile(const QString& topath,QList<mImage> &inlist);
/*Morphometry definition: name,type,*/
struct mMorphometry{
    /*the morphometry can be point cloud file (marker,apo) or morphology file (swc).*/
    QString id,name,comments,filename;
    QStringList showlist,showlist_name;
    mMorphometry(){
        id=name=comments=filename="";
        showlist.clear();
        showlist_name<<"ID"<<"Name"<<"Comments"<<"Filename";
    }
    bool getShowlist()
    {
        if(id.isEmpty()&&name.isEmpty())
            return false;
        showlist<<id<<name<<comments<<filename;
        return true;
    }
};

struct mMorphometryDB{
   /*A list of morphometry files*/
    QString name,imageName,time;
    QList<mMorphometry> listMorphometry;
    bool isidExist;
    mMorphometryDB(){
        name=imageName=time="";
        listMorphometry.clear();
        isidExist=true;
    }
    void getlistID()
    {
        if(listMorphometry.size())
        {
            for(V3DLONG i=0;i<listMorphometry.size();i++)
            {
                if(listMorphometry.at(i).id.isEmpty())
                {
                    isidExist=false;
                    break;
                }
            }
            assignlistID();
        }
    }

    void assignlistID()
    {
        QList<mMorphometry> tmplist;tmplist.clear();
        if(listMorphometry.size()&&isidExist)
        {
            for(V3DLONG i=1;i<listMorphometry.size()+1;i++)
            {
                mMorphometry tmp=listMorphometry.at(i-1);
                QString aid="_";
                if(i<10)
                    aid+=("0000"+QString::number(i));
                else if(i>=10&&i<100)
                    aid+=("000"+QString::number(i));
                else if(i>=100&&i<1000)
                    aid+=("00"+QString::number(i));
                else if(i>=1000&&i<10000)
                    aid+=("0"+QString::number(i));
                else
                    aid+=(QString::number(i));
                tmp.id=aid;
                tmp.getShowlist();
                tmplist.append(tmp);
            }
            listMorphometry.clear();
            listMorphometry=tmplist;
        }
    }
};
mMorphometryDB convertconf2mlist(const QString& confpath);
QList<QStringList> getshowlistmMorphometry(mMorphometryDB inlist);

struct pointCloud_morphometry
{
    QString id,name,comments;
    float x,y,z,radius,intensity;
    QStringList showlist,showlist_name;
    pointCloud_morphometry(){
        id=name=comments="";
        x=y=z=radius=intensity=0;
        showlist.clear();
        showlist_name<<"ID"<<"Name"<<"Comments";
    }
    bool getShowlist()
    {
        if(id.isEmpty()&&name.isEmpty())
            return false;
        showlist<<id<<name<<comments;
        return true;
    }
};
struct pointCloud_morphometry_list{
    /*The point cloud file will save at PointCloud dir.*/
    QList<pointCloud_morphometry> listpointCloud;
    QString name,imageName,time;
    Annotator author;
    bool isidExist;
    pointCloud_morphometry_list(){
        name=imageName=time="";
        listpointCloud.clear();
        isidExist=true;
    }
    void getlistID()
    {
        if(listpointCloud.size())
        {
            for(V3DLONG i=0;i<listpointCloud.size();i++)
            {
                if(listpointCloud.at(i).id.isEmpty())
                {
                    isidExist=false;
                    break;
                }
            }
            assignlistID();
        }
    }

    void assignlistID()
    {
        QList<pointCloud_morphometry> tmplist;
        if(listpointCloud.size()&&isidExist)
        {
            for(V3DLONG i=1;i<listpointCloud.size()+1;i++)
            {
                pointCloud_morphometry tmp=listpointCloud.at(i-1);
                QString aid="_";
                if(i<10)
                    aid+=("0000"+QString::number(i));
                else if(i>=10&&i<100)
                    aid+=("000"+QString::number(i));
                else if(i>=100&&i<1000)
                    aid+=("00"+QString::number(i));
                else if(i>=1000&&i<10000)
                    aid+=("0"+QString::number(i));
                else
                    aid+=(QString::number(i));
                tmp.id=aid;
                tmp.getShowlist();
                tmplist.append(tmp);
            }
            listpointCloud.clear();
            listpointCloud=tmplist;
        }

    }
};
bool writepointCloud(const QString& inapo,const QString& outpath);
QList<pointCloud_morphometry> convertApo2pc(const QString& inapo);
pointCloud_morphometry_list convertApo2pclist(const QString& inapo);
QList<QStringList> getshowlistpointCloud(QList<pointCloud_morphometry> inpclist);
QList<QStringList> getshowlistpointCloud(pointCloud_morphometry_list inlist);
/*morphoHub will load or set a new client based on this.
 *morphoHub file system:
        *Morphometry
                    *ImageID (_ImageIDList,_17302,_17300,...)
                            *morphometryID (_morphometryIDList,_00001,_00002,...)
                                    *morphometryType (_morphometryTypeList,_L0,_dendrite,_L1,_L2,_L3)
        *Configuration
            image_datasets.conf, should put at 'Morphometry/_ImageIDList' dir
            users.conf
        *
 *future: load from conf file for the architechure
*/
struct mFileSystem{
    QStringList morphometryType,morphometryType_showlist;
    QStringList basicDirs;
    QString imgDBPath,dbPath;//basic img DB Path: image datasets
    QString imgconfPath;//configuration or information of image datasets
    QString userconfPath;//configuration of users
    mImageDB mimagedb;
    mFileSystem(){
        basicDirs.clear();
        /*if the order of the list(basicDirs) is changed, please change with <firstDirsInMFS>*/
        basicDirs<<"Morphometry"<<"Configuration"<<"Component"<<"User";
        morphometryType<<"Soma"<<"AutoTraced"<<"Level0"<<"Level1"<<"Level2"<<"Level3";
        imgDBPath=imgconfPath=userconfPath="";
        dbPath="";
    }
    bool getShowlistType()
    {
        QDir dbDir(dbPath);
        if(!dbPath.isEmpty()&&dbDir.exists())
        {
            morphometryType_showlist<<morphometryType.at(SOMA)
                                      <<morphometryType.at(AUTOTRACED)
                                        <<morphometryType.at(LEVEL1)
                                          <<morphometryType.at(LEVEL2)
                                            <<morphometryType.at(LEVEL3);
        }
        else
            return false;
    }

    bool initImageDB()
    {
        if(imgconfPath.isEmpty())
            return false;
        QFile conffile(imgconfPath);
        if(conffile.exists())
        {
            mimagedb.mdbPath=imgDBPath;
            mimagedb.mdbconf=imgconfPath;
            mimagedb.listImage=getImagelist(mimagedb.mdbconf);
            return true;
        }
        else
        {
            //make new file
            //write init conf to file
            mImage tmpImage;
            QStringList imgconfTitle=tmpImage.mFeatures;
            if(conffile.open(QIODevice::ReadWrite | QIODevice::Text))
            {
                QString data=imgconfTitle[0];
                for(int i=1;i<imgconfTitle.size();i++)
                {
                    data=data+","+imgconfTitle[i];
                }
                conffile.write(data.toAscii());
                conffile.close();
            }
        }
        return false;
    }

    QString getImgConfPath()
    {
        if(dbPath.isEmpty())
            return imgconfPath;
        QDir path(dbPath);
        if(!path.exists())
            return imgconfPath;
        imgconfPath=dbPath+"/"+basicDirs.at(CONFIGURATION)+"/"+"image_datasets.conf";
        return imgconfPath;
    }
    QString getUserConfPath()
    {
        if(dbPath.isEmpty())
            return userconfPath;
        QDir path(dbPath);
        if(!path.exists())
            return userconfPath;
        userconfPath=dbPath+"/"+basicDirs.at(CONFIGURATION)+"/users.conf";
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
        imgconfPath=dbPath+"/"+basicDirs.at(CONFIGURATION)+"/"+"image_datasets.conf";
        userconfPath=dbPath+"/"+basicDirs.at(CONFIGURATION)+"/users.conf";
        initImageDB();
        getShowlistType();
        return true;
    }
};

#endif // BASIC_CONF_H
