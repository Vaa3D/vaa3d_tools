#ifndef MORPHOHUB_DBMS_BASIC_H
#define MORPHOHUB_DBMS_BASIC_H
#include <QtGui>
#include "v3d_basicdatatype.h"
#include "basic_surf_objs.h"
using namespace std;

/*Definition of different data types*/
struct mXYZ
{
    unsigned long mx,my,mz;
    mXYZ() {mx=my=mz=0;}
};
/*Soma morphometry*/
struct mSoma{
    unsigned long mSomaID; //start from 10000 and end at 99999.
    QString cellName,comments,reion,cellType;
    RGBA8 color;
    mXYZ position;
    double intensity,radius,volume;
    mSoma(){
        mSomaID=10000;
        cellName=comments=reion=cellType="";
        color.r=color.g=color.b=color.a=255;
        intensity=radius=volume=0.0;
    }
};
struct mSomaMorpho{
    QList<mSoma> listSomata;
    unsigned long mImageID;//entance to image dataset
    mSomaMorpho(){
        listSomata.clear();
        mImageID=10000;
    }
};
/*Image releated data structures*/
struct mVoxelResolution{
    double rx,ry,rz;
    mVoxelResolution(){
        rx=0.0;ry=0.0;rz=0.0;
    }
    void setmVR(double irx,double iry,double irz)
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
    unsigned long mImageID; //start from 10000 and end at 99999.
    unsigned short mObjectID; // 0:mouse; 1: monkey?; ...
    unsigned short mFormatID; // 0:terafly; 1: 2D; 2: v3draw; 3: tif; 4: tiff. maybe add more here
    unsigned short bit;
    /*sampleID: hust id; sourceID: allen id;*/
    QString sampleID,sourceID,name,comments,creline;
    mVoxelResolution mVR;
    mImageSize mIS;
    QStringList mObjectList,mFormatList;
    QStringList dataNumberTitle,dataNumber;
    mImage(){
        name=sampleID=sourceID=comments=creline="";
        mImageID=10000;mObjectID=mFormatID=0;
        bit=8;
        mObjectList.clear();mFormatList.clear();
        dataNumberTitle.clear();dataNumber.clear();
        mObjectList<<"mouse"<<"monkey"<<"unknown";
        mFormatList<<"terafly"<<"v3draw"<<"tif"<<"tiff"<<"unset";
        dataNumberTitle<<"ImageID"<<"Name"<<"SampleID"<<"SourceID"
                 <<"Format"<<"Object"<<"SizeX"<<"SizeY"<<"SizeZ"<<
                   "ResolutionX"<<"ResolutionY"<<"ResolutionZ"<<"Bit"<<"Comments";
        mVR.setmVR(0.0,0.0,0.0);mIS.setmIS(0,0,0);
    }
    mImage(unsigned long inmImageID,const QString& insampleID,const QString& insourceID,mVoxelResolution mvr,mImageSize mis,const QString& incomments="",const QString& increline="",unsigned short inbit=8,unsigned short inmObjectID=0,unsigned short inmFormatID=0){
        this->mImageID=inmImageID;
        this->sampleID=insampleID;
        this->sourceID=insourceID;
        this->mVR=mvr;
        this->mIS=mis;
        this->comments=incomments;
        this->creline=increline;
        this->bit=inbit;
        this->mObjectID=inmObjectID;
        this->mFormatID=inmFormatID;
        this->getName();
    }
    void init(const QString& insampleID,const QString& insourceID,mVoxelResolution mvr,mImageSize mis,const QString& incomments="",const QString& increline="",unsigned short inbit=8,unsigned short inmObjectID=0,unsigned short inmFormatID=0){
        this->sampleID=insampleID;
        this->sourceID=insourceID;
        this->mVR=mvr;
        this->mIS=mis;
        this->comments=incomments;
        this->creline=increline;
        this->bit=inbit;
        this->mObjectID=inmObjectID;
        this->mFormatID=inmFormatID;
        this->getName();
    }
    void init(const QString& insampleID,const QString& insourceID){
        this->sampleID=insampleID;
        this->sourceID=insourceID;
        this->getName();
    }
    void init(const QString& insampleID){
        this->sampleID=insampleID;
        this->getName();
    }
    QString getName(){
        /* name=object_sampleID_format*/
        this->name=mObjectList[mObjectID]+"_"+sampleID+"_"+mFormatList[mFormatID];
        return this->name;
    }
    QStringList getDataNumber();
};
mImage createImageFromQSL(QStringList inlist);
QList<mImage> getImagelist(const QString& inpath);
bool writeImagelistToFile(const QString& topath,QList<mImage> &inlist);
/*Definition of the MorphoHub-Database*/
enum db_init_1stlayer_index{
    METADATA=0,
    MORPHOMETRYDB=1,
    LEVEL0DB=2,
    MODULEDB=3
};
enum db_init_2stlayer_metadata_index{
    IMAGE=0,
    SOMA=1,
    MORPHOMETRY=2,
    USER=3
};
enum db_init_3stlayer_metadata_index{
    BASIC=0,
    DETAIL=1,
    SUPPLEMENTARY=2
};
struct mDatabase{
    //database part
    QString dbpath ;
    QStringList db_init_1stlayer;
    QStringList db_init_2stlayer_metadata;
    QStringList db_init_3stlayer_metadata;

    //soma morphometry part
    QList<mSomaMorpho> listSomata;
    //morphometry part
    QString morpho_db_path;
    QString morpho_metadata_path,soma_metadata_path;

    //image part
    QString imgdbpath;
    QString img_metadata_path;
    QList<mImage> listImages; //entrance to image datasets

    mDatabase(){
        db_init_1stlayer.clear();db_init_2stlayer_metadata.clear();db_init_3stlayer_metadata.clear();
        dbpath=morpho_db_path=morpho_metadata_path=soma_metadata_path=imgdbpath=img_metadata_path="";
        db_init_1stlayer<<"metadata"<<"morphometry"<<"level0"<<"module";
        db_init_2stlayer_metadata<<"image"<<"soma"<<"morphometry"<<"user";
        db_init_3stlayer_metadata<<"basic"<<"detail"<<"supplementary";
        //soma
        listSomata.clear();
    }
    bool initialization();
    bool createDB(const QString &inpath);
    bool loadDB(const QString &inpath);
    /*Image part*/
    QString getImg_metadata_path();
    bool creatNewImage(const QString &insampleID);
    bool deleteImage(unsigned long removeID);
    bool deleteImageBySampleID(const QString &insampleID);
    //get the image path, come here later
};

#endif // MORPHOHUB_DBMS_BASIC_H
