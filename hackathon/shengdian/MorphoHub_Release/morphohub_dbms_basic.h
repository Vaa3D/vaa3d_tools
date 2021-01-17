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
#define MaxFileSize 99
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
    MSomaType mSomaID;
    MImageType mImageID;//entance to image dataset
    QString name,cellName,comments,region,cellType;
    RGBA8 color;
    mXYZ position;
    float intensity,radius,volume;
    QStringList dataNumberTitle,dataNumber;
    mSoma(){
        mSomaID=MinSomaID;mImageID=MinImageID;
        name=cellName=comments=region=cellType="";
        color.r=color.g=color.b=color.a=255;
        intensity=radius=volume=0.0;
        dataNumber.clear();this->setDataNumberTitle();
    }
    QString getcellName(){
        /* name=Z_X_Y*/
        this->name=QString::number(this->position.mz)+"_X_"+QString::number(this->position.mx)
                +"_Y_"+QString::number(this->position.my);
        return this->name;
    }
    void setDataNumberTitle(bool basic=false){
        this->dataNumberTitle.clear();
        if(basic)
            this->dataNumberTitle<<"ImageID"<<"SomaID"<<"Name";
        else
            dataNumberTitle<<"ImageID"<<"SomaID"<<"Name"<<"CellName"
                          <<"X"<<"Y"<<"Z"<<"Radius"
                         <<"ColorR"<<"ColorG"<<"ColorB"<<"ColorA"
                        <<"Intensity"<<"Volume"
                       <<"Comments"<<"Region"<<"CellType";
    }
    QStringList getDataNumber(bool basic=false);
};
/*soma Morphometry read and write*/
mSoma apo2mSoma(CellAPO inapo);
CellAPO mSoma2Apo(mSoma insoma);
mSoma createmSomaFromQSL(QStringList inlist,bool basic=false);
QList<mSoma> getSomalist(const QString& inpath,bool basic=false);
bool writeSomalistToFile(const QString& topath,QList<mSoma> &insomalist,bool basic=false);
/*morphometry morphology part*/
struct mMorphometry{
    /*morphometry of an individual neuron*/
    MMorphoType mMorphoID;
    MImageType mImageID;
    MSomaType mSomaID;
    QString name,morphoType,comments,tag;
    /*QString timeStamp;
    int authorID;
    QList<int> listCheckers;*/
    QStringList dataNumberTitle,dataNumber;
    /*store all the morphological files of an individual neuron, split in ';' */
    QStringList morphoFilelist;
    mMorphometry(){
        name=comments=tag="";
        morphoType="unknown";
        mMorphoID=MinMorphometryID;
        mImageID=MinImageID;mSomaID=MinSomaID;
        dataNumber.clear();dataNumberTitle.clear();
        morphoFilelist.clear();
        dataNumberTitle<<"MorphoID"<<"Name"<<"MorphoType"<<"Tag"<<"Comments"<<"Files";
    }
    QString getName(){
        /* name=mImageID_mSomaID_mMorphoID*/
        this->name=QString::number(this->mImageID)+"_"+QString::number(this->mSomaID)
                +"_"+QString::number(this->mMorphoID);
        return this->name;
    }
    void setDataNumberTitle(bool basic=false){
        this->dataNumberTitle.clear();
        if(basic)
            this->dataNumberTitle<<"ImageID"<<"SomaID"<<"MorphoID"<<"Name";
        else
            dataNumberTitle<<"ImageID"<<"SomaID"<<"MorphoID"<<"Name"
                          <<"MorphoType"<<"Tag"<<"Comments"<<"Files";
    }
    QStringList getDataNumber(bool basic=false);
};
mMorphometry createMorphoFromQSL(QStringList inlist, bool basic=false);
QList<mMorphometry> getMorpholist(const QString& inpath,bool basic=false);
bool writeMorpholistToFile(const QString& topath,QList<mMorphometry> &inMorpholist,bool basic=false);
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
        mObjectList.clear();mFormatList.clear();dataNumber.clear();
        this->setDataNumberTitle();
        mObjectList<<"mouse"<<"monkey"<<"unknown";
        mFormatList<<"terafly"<<"v3draw"<<"tif"<<"tiff"<<"unset";
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
        this->dataNumberTitle.clear();
        if(basic)
            this->dataNumberTitle<<"ImageID"<<"Name"<<"SampleID";
        else
            this->dataNumberTitle<<"ImageID"<<"Name"<<"SampleID"<<"SourceID"
                                <<"Format"<<"Object"<<"SizeX"<<"SizeY"<<"SizeZ"<<
                                  "ResolutionX"<<"ResolutionY"<<"ResolutionZ"<<"Bit"<<"Comments";
    }
};
mImage createImageFromQSL(QStringList inlist,bool basic=false);
QList<mImage> getImagelist(const QString& inpath,bool basic=false);
mImage getImage(const QString& inpath);
bool writeImagelistToFile(const QString& topath,QList<mImage> &inlist,bool basic=false);
bool deletePath(const QString &dpath);
bool deldeteFile(const QString &dfile);
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
private:
    QString dbpath;
    QStringList db_init_1stlayer;
    QStringList db_init_2stlayer_metadata;
    QStringList db_init_3stlayer_metadata;
    //image part
    QString imgdbpath;/*optional, can be used for the visualization and annotation of image dataset*/
    QString db_img_metadata_path;/*<db>/metadata/image/basic/image.metadata, for fast-indexing all the images at db*/    
    QString img_metadata_path;/*file path that contains the metadata of a image*/    
    //soma morphometry part
    QString db_soma_metadata_path;
    /*record id of all the image datasets with somalist. initialization and update from `db_soma_metadata_path`*/   
    QString soma_metadata_path;/*<db>/metadata/soma/detail/<imageID>/soma.metadata*/
    //morphometry part
    QString morpho_db_path;
    QString morpho_metadata_path;

public:
    QList<mImage> listImages; //entrance to image datasets
    mImage mImagePointer; //entrance to metadata of one image dataset
    QList<mImage> listSomata;
    QList<mSoma> mSomataPointer;//entrance to soma metadata of one image dataset
    QList<mSoma> listMorpho;
    QList<mMorphometry> mMorphoPointer;

    mDatabase(){
        dbInitialization();
        imageInitialization();
        somaInitialization();
        morphoInitialization();
    }
    bool dbInitialization(bool start=true);
    QString getDBpath(){return this->dbpath;}
    QString getImgDBpath(){return this->imgdbpath;}
    bool setImgDBpath(const QString &inpath);
    bool createDB(const QString &inpath);
    bool loadDB(const QString &inpath);
    /*******************Image function part*************************/
    bool imageInitialization(bool start=true);
    QString getImg_metadata_path();
    QString getImg_metadata_path(MImageType qImageID);/*detail metadata in <db>/metadata/image/detail/*/
    QString getImg_metadata_path(const QString &insampleID);
    bool getImagePointer(MImageType qImageID);
    bool getImagePointer(const QString &insampleID);
    bool addImage(const QString &insampleID);
    bool deleteImage(MImageType removeID);
    bool deleteImage(const QString &insampleID);
    bool updateImage(mImage inImage);
    /*(`Advanced, optional, next version`)
    >   + get one of the resolution path of image dataset
    >   + get the image block
    >       + crop function : with a crop-center coordinates (3D) and crop size.*/
    /********************soma morphometry part********************/
    bool somaInitialization(bool start=true);
    QString getSoma_metadata_path();//`basic/soma.metadata`
    QString getSoma_metadata_path(MImageType qImageID);//`detail/<imageid>/soma.metadata`
    QString getSoma_metadata_path(const QString &insampleID);
    bool getSomaPointer(MImageType qImageID);
    bool getSomaPointer(const QString &insampleID);
    bool updateListSomata(QList<mImage> newlist);
    bool addSomalist(const QString &inapo,MImageType inmImageID);
    bool deleteSomalist(MImageType removeID);
    bool deleteSomalist(const QString &insampleID);
    bool updateSomalist(const QString &inapo,MImageType inmImageID);
    bool updateSomalist(const QString &inapo,const QString &insampleID);
    bool updateSomalist(QList<mSoma> insoma,MImageType inmImageID);
    /*******************Morphometry dataset part*********************/
    bool morphoInitialization(bool start=true);
    QString getMorpho_path();
    QString getMorpho_path(MImageType qImageID);/*return the path of neurons of an image dataset*/
    QString getMorpho_path(MImageType qImageID,MSomaType qSomaID);/*return the path of morphometries of a neuron*/
    /*return the path of a morphometry/arbor/version of a neuron*/
    QString getMorpho_path(MImageType qImageID,MSomaType qSomaID,MMorphoType qMorphoID);
    QString getMorpho_path(MImageType qImageID,MSomaType qSomaID,MMorphoType qMorphoID,const QString &qfilename);
    QString getMorpho_metadata_path();//the path of all the basic metadata of morphometry
    QString getMorpho_metadata_path(MImageType qImageID);
    QString getMorpho_metadata_path(MImageType qImageID,MSomaType qSomaID);/*return all the morphometries of a neuron*/
    QString getMorpho_metadata_path(const QString &insampleID,MSomaType qSomaID);
    /*get the morphometry-list(listmMorphometry), load and write morphometry.metadata*/
    /*create a new morphometry; delete or update a morphometry;*/
    bool getMorphoPointer(MImageType qImageID,MSomaType qSomaID);
    bool getMorphoPointer(const QString &insampleID,MSomaType qSomaID);
    bool addMorpho(MImageType qImageID,MSomaType qSomaID,QStringList infilelist);    
    bool deleteMorpho(MImageType qImageID);
    bool deleteMorpho(MImageType qImageID,MSomaType qSomaID);
    bool deleteMorpho(MImageType qImageID,MSomaType qSomaID,MMorphoType qMorphoID);
    bool deleteMorpho(MImageType qImageID,MSomaType qSomaID,MMorphoType qMorphoID,const QString &deleteFile);
    bool updateMorpho(MImageType qImageID,MSomaType qSomaID,MMorphoType qMorphoID,mMorphometry inMorpho);
};

#endif // MORPHOHUB_DBMS_BASIC_H
