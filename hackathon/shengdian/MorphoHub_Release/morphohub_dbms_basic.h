#ifndef MORPHOHUB_DBMS_BASIC_H
#define MORPHOHUB_DBMS_BASIC_H
#include <QtGui>
#include "v3d_basicdatatype.h"
#include "basic_surf_objs.h"
using namespace std;
#define MinImageID 10000
#define MaxImageID 99999
#define MinSomaID 10000
#define MaxSomaID 99999
#define MinMorphometryID 100
#define MaxMorphometryID 999
#define MinUserID 100
#define MaxUserID 999
#define MImageType unsigned long
#define MSomaType unsigned long
#define MMorphoType unsigned int
#define MUserType unsigned int

struct mMorphometry;
struct mSoma;
struct mImage;
struct mDatabase;

/*User*/
enum UserPriority{
    UPvisitor=0,
    UPannotator=1,
    UPjuniorchecker=2,
    UPseniorchecker=3,
    UPAdministrater=4
};
struct mUser{
    unsigned int mUserID,mGroupID;
    QString name,comments;
    unsigned int priorityID;
    mUser(){
        mUserID=mGroupID=MinUserID;
        priorityID=UPvisitor;
        name="default";
        comments="";
    }
};
/*Definition of different data types*/
struct mXYZ
{
    float mx,my,mz;
    mXYZ() {mx=my=mz=0;}
};
/*Soma morphometry*/
struct mSoma{
    MSomaType mSomaID; //start from 10000 and end at 99999.
    QString cellName,comments,region,cellType;
    RGBA8 color;
    mXYZ position;
    float intensity,radius,volume;
    QStringList dataNumberTitle,dataNumber;
    mSoma(){
        mSomaID=MinSomaID;
        cellName=comments=region=cellType="";
        color.r=color.g=color.b=color.a=255;
        intensity=radius=volume=0.0;
        dataNumber.clear();dataNumberTitle.clear();
        dataNumberTitle<<"SomaID"<<"CellName"
                      <<"X"<<"Y"<<"Z"<<"Radius"
                      <<"ColorR"<<"ColorG"<<"ColorB"<<"ColorA"
                     <<"Intensity"<<"Volume"
                      <<"Comments"<<"Region"<<"CellType";
    }
    QString getName(){
        /* name=Z_X_Y*/
        this->cellName=QString::number(this->position.mz)+"_X_"+QString::number(this->position.mx)
                +"_Y_"+QString::number(this->position.my);
        return this->cellName;
    }
    QStringList getDataNumber();
};
/*somalist or start points of a whole brain dataset */
struct mSomaMorpho{
    QList<mSoma> listSomata;
    MImageType mImageID;//entance to image dataset
    /*unsigned int authorID;// unshield later
    QString updateTime;*/
    mSomaMorpho(){
        listSomata.clear();
        mImageID=MinImageID;
    }
};
/*soma Morphometry read and write*/
//QList<mSomaMorpho> getSomaMorpholist(QList<mImage> queryImagelist,const QString& queryPath);
mSomaMorpho getSomaMorpho(const QString& queryPath,MImageType inmImageID=MinImageID);
bool writeSomaMorphoToFile(const QString& topath,mSomaMorpho inmSomaMorpho);
mSoma createSomaMorphoFromQSL(QStringList inlist);
mSoma apo2mSoma(CellAPO inapo);
/*morphometry morphology part*/
struct mMorphometry{
    /*morphometry of an individual neuron*/
    MMorphoType mMorphometryID;
    QString name,morphometryType,comments,tag;
    /*QString timeStamp;
    int authorID;
    QList<int> listCheckers;*/
    QStringList dataNumberTitle,dataNumber;
    /*store all the morphological files of an individual neuron, split in blank*/
    QStringList morphoFileNamelist;
    mMorphometry(){
        name=comments=tag="";
        morphometryType="unknown";
        mMorphometryID=MinMorphometryID;
        dataNumber.clear();dataNumberTitle.clear();
        morphoFileNamelist.clear();
        dataNumberTitle<<"MorphoID"<<"Name"<<"MorphoType"<<"Tag"<<"Comments"<<"Files";
    }
    QStringList getDataNumber();
};
struct mMorphometrylist{
    /*for managing all the morphometries of an individual neuron*/
    QList<mMorphometry> listMorphometry;
    MImageType mImageID;
    MSomaType mSomaID;
    mMorphometrylist(){
        listMorphometry.clear();
        mImageID=MinImageID;mSomaID=MinSomaID;
    }
    void reNamelistMorphometry();
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
    MImageType mImageID; //start from 10000 and end at 99999.
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
        mImageID=MinImageID;mObjectID=mFormatID=0;
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
    mImage(MImageType inmImageID,const QString& insampleID,const QString& insourceID,mVoxelResolution mvr,mImageSize mis,const QString& incomments="",const QString& increline="",unsigned short inbit=8,unsigned short inmObjectID=0,unsigned short inmFormatID=0){
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
    QStringList getDataNumber(bool basic=false);
    void setDataNumberTitle(bool basic=false){
        dataNumberTitle.clear();
        if(basic)
            dataNumberTitle<<"ImageID"<<"Name"<<"SampleID"<<"SourceID"
                     <<"Format"<<"Object"<<"SizeX"<<"SizeY"<<"SizeZ"<<
                       "ResolutionX"<<"ResolutionY"<<"ResolutionZ"<<"Bit"<<"Comments";
        else
            dataNumberTitle<<"ImageID"<<"SampleID";
    }
};
mImage createImageFromQSL(QStringList inlist);
QList<mImage> getImagelist(const QString& inpath);
bool writeImagelistToFile(const QString& topath,QList<mImage> &inlist,bool basic=false);
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

    //soma morphometry part, all the somata of database
    QList<mSomaMorpho> listmSomata;
    /*<db>/metadata/soma/basic/<imageID>/soma.metadata*/
    QString soma_metadata_path;

    //morphometry part
    QString morpho_db_path;
    QString morpho_metadata_path;
    /*all the managed morphometries in database is registered here in this list*/
    QList<mMorphometrylist> listmMorphometry;

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
        listmSomata.clear();
        //morphometry
        listmMorphometry.clear();
    }
    /*initialization after loading or getting the db path
     * 1. listImages: scan from <db>/metadata/image/basic/image.metadata
     * 2. listmSomata: scan from <db>/metadata/soma/basic/<all_image>/soma.metadata
     * 3. listmMorphometry: scan from <db>/metadata/morphometry/basic/<all_image>/<all_somata>/morphometry.metadata
    */
    bool initialization();
    bool createDB(const QString &inpath);
    bool loadDB(const QString &inpath);
    /************************Morphometry dataset part*********************/
    /*get the morphometry-list(listmMorphometry), load and write morphometry.metadata*/
    /*create a new morphometry; delete or update a morphometry;*/
    bool createMorpho(MImageType imageID=MinImageID,MSomaType somaID=MinSomaID);
    QString getMorpho_db_path();
    /*return the path of neurons of an image dataset*/
    QString getMorpho_path(MImageType qImageID);
    /*return the path of morphometries of a neuron*/
    QString getMorpho_path(MImageType qImageID,MSomaType qSomaID);
    /*return the path of a morphometry/arbor/version of a neuron*/
    QString getMorpho_path(MImageType qImageID,MSomaType qSomaID,MMorphoType qMorphoID);
    //the path of all the basic metadata of morphometry
    QString getMorpho_metadata_path();
    /*return all the neurons of an image dataset*/
    QString getMorpho_metadata_path(MImageType qImageID);
    /*return all the morphometries of a neuron*/
    QString getMorpho_metadata_path(MImageType qImageID,MSomaType qSomaID);

    /*******************Image part*************************/

    QString getImg_metadata_path();
    /*detail metadata in <db>/metadata/image/detail/*/
    QString getImg_metadata_path(MImageType queryID);
    QString getImg_metadata_path(const QString &insampleID);
    bool creatNewImage(const QString &insampleID);
    bool deleteImage(MImageType removeID);
    bool deleteImage(const QString &insampleID);
    //get the image path, come here later

    /********************************soma morphometry part********************************/
    QString getSoma_metadata_path();
    QString getSoma_metadata_path(MImageType queryID);
    QString getSoma_metadata_path(const QString &insampleID);
    bool createNewSomaMorpho_fromApo(const QString &inapo,MImageType inmImageID=MinImageID);
    bool updateSomaMorpho_fromApo(const QString &inapo,MImageType inmImageID);
    bool deleteSomaMorpho(MImageType removeID);
    bool deleteSomaMorpho(const QString &insampleID);
    /*init of listmsomata*/
    QList<mSomaMorpho> getSomaMorpholist();
    QList<mSomaMorpho> getSomaMorpholist(QList<mImage> queryImagelist);
};

#endif // MORPHOHUB_DBMS_BASIC_H
