#include "morphohub_dbms_basic.h"
/*For the initialization of database*/
bool mDatabase::dbInitialization(bool start)
{
    if(start)
    {
        db_init_1stlayer.clear();db_init_2stlayer_metadata.clear();db_init_3stlayer_metadata.clear();
        dbpath="";
        db_init_1stlayer<<"metadata"<<"morphometry"<<"level0"<<"module";
        db_init_2stlayer_metadata<<"image"<<"soma"<<"morphometry"<<"user";
        db_init_3stlayer_metadata<<"basic"<<"detail"<<"supplementary";
    }
    else
    {
        return loadDB(this->dbpath);
    }
    return true;
}
bool mDatabase::imageInitialization(bool start)
{
    if(start)
    {
        imgdbpath=db_img_metadata_path=img_metadata_path="";
        listImages.clear();mImagePointer=new mImage();
    }
    else
    {
        this->db_img_metadata_path=this->getImg_metadata_path();
        //init when load the database, listImages from db_img_metadata_path
        QDir img_metadata_Dir(this->db_img_metadata_path);
        if(!this->db_img_metadata_path.isEmpty()&&img_metadata_Dir.exists())
        {
            //load from this image.metadata
            this->listImages=getImagelist(this->db_img_metadata_path,true);
            if(!this->listImages.size())
            {
                QMessageBox::warning(0,"Image dataset empth",QObject::tr("Without image datasets at %1 !").arg(this->db_img_metadata_path));
                return false;
            }
        }
        else
            return false;
    }
    return true;
}
bool mDatabase::createDB(const QString &inpath)
{
    QDir inDir(inpath);
    if(inpath.isEmpty()||!inDir.exists())
        return false;
    this->dbpath=inpath;
    /*first layer*/
    for(V3DLONG i=0;i<db_init_1stlayer.size();i++)
    {
        QString thispath=inpath+"/"+db_init_1stlayer[i];
        QDir thisDir(thispath);
        if(!thisDir.exists())
             inDir.mkdir(db_init_1stlayer[i]);
        if(!thisDir.exists())
            return false;
    }
    /*metadata layer*/
    for(V3DLONG i=0;i<db_init_2stlayer_metadata.size();i++)
    {
        QDir dstDir(inpath+"/"+db_init_1stlayer[METADATA]);
        QString thispath=inpath+"/"+db_init_1stlayer[METADATA]+"/"+db_init_2stlayer_metadata[i];
        QDir thisDir(thispath);
        if(!thisDir.exists())
             dstDir.mkdir(db_init_2stlayer_metadata[i]);
        if(!thisDir.exists())
            return false;
    }

    /*metadata inside layer*/
    for(V3DLONG i=0;i<db_init_2stlayer_metadata.size();i++)
    {
        QDir dstDir(inpath+"/"+db_init_1stlayer[METADATA]+"/"+db_init_2stlayer_metadata[i]);
        for(V3DLONG j=0;j<db_init_3stlayer_metadata.size();j++)
        {
            QString thispath=inpath+"/"+db_init_1stlayer[METADATA]+"/"+db_init_2stlayer_metadata[i]+"/"+
                    db_init_3stlayer_metadata[j];
            QDir thisDir(thispath);
            if(!thisDir.exists())
                 dstDir.mkdir(db_init_3stlayer_metadata[j]);
            if(!thisDir.exists())
                return false;
        }
    }
    return true;
}
bool mDatabase::loadDB(const QString &inpath)
{
    return createDB(inpath);
}
QString mDatabase::getImg_metadata_path()
{
    QString outpath="";QDir dbDir(dbpath);
    if(dbpath.isEmpty()||!dbDir.exists())
        return outpath;
    outpath=dbpath+"/"+db_init_1stlayer[METADATA]+"/"+db_init_2stlayer_metadata[IMAGE]+"/"+
            db_init_3stlayer_metadata[BASIC]+"/"+db_init_2stlayer_metadata[IMAGE]+".metadata";
    return outpath;
}
QString mDatabase::getImg_metadata_path(MImageType qImageID)
{
    QString outpath=""; QDir dbDir(dbpath);
    if(dbpath.isEmpty()||!dbDir.exists()||qImageID<MinImageID||qImageID>MaxImageID)
        return outpath;
    V3DLONG liz=this->listImages.size();
    for(V3DLONG i=0;i<liz;i++)
    {
        mImage temp=this->listImages.at(i);
        if(qImageID==temp.mImageID)
        {
            outpath= dbpath+"/"+db_init_1stlayer[METADATA]+"/"+db_init_2stlayer_metadata[IMAGE]+"/"+
                    db_init_3stlayer_metadata[DETAIL]+"/"+QString::number(qImageID)+"/"+
                    db_init_2stlayer_metadata[IMAGE]+".metadata";
            break;
        }
    }
    QFile metaFile(outpath);
    if(!metaFile.exists())
    {
        qDebug()<<"Requested image metadata of "<<QString::number(qImageID)<<" is not existed.";
    }
    return outpath;
}
QString mDatabase::getImg_metadata_path(const QString &insampleID)
{
    QString outpath="";
    V3DLONG liz=this->listImages.size();
    for(V3DLONG i=0;i<liz;i++)
    {
        mImage temp=this->listImages.at(i);
        if(QString::compare(insampleID,temp.sampleID))
            return this->getImg_metadata_path(temp.mImageID);
    }
    return outpath;
}
bool mDatabase::getImagePointer(MImageType qImageID)
{
    this->img_metadata_path=this->getImg_metadata_path(qImageID);
    if(this->img_metadata_path.isEmpty())
        return false;
    this->mImagePointer=getImage(this->img_metadata_path);
    return true;
}
bool mDatabase::getImagePointer(const QString &insampleID)
{
    V3DLONG liz=this->listImages.size();
    for(V3DLONG i=0;i<liz;i++)
    {
        mImage temp=this->listImages.at(i);
        if(QString::compare(insampleID,temp.sampleID))
            return this->getImagePointer(temp.mImageID);
    }
    return false;
}
bool mDatabase::addImage(const QString & insampleID)
{
    /*1. check the validity of the input `sampleID`
      2. assign a new `mImageID`.
      3. create a metadata file at `<db>/metadata/image/detail/<newID>/image.metadata`
      4. update info into `listmImages`
      5. update metadata file of all the images in db.*/
    /*1*/
    V3DLONG liz=this->listImages.size();
    for(V3DLONG i=0;i<liz;i++)
    {
        mImage temp=this->listImages.at(i);
        if(QString::compare(temp.sampleID,insampleID))
        {
            qDebug()<<"Error: Already has image "<<insampleID;
            return false;
        }
    }
    /*2*/
    mImage newImage;
    newImage.mImageID=MinImageID;
    QList<MImageType> listimageID; listimageID.clear();
    for(V3DLONG i=0;i<liz;i++)
        listimageID.append(this->listImages.at(i).mImageID);
    for(V3DLONG i=MinImageID;i<MaxImageID+1;i++)
        if(!listimageID.contains(i))
        {
            newImage.mImageID=i;
            break;
        }
    /*3*/
    QList<mImage> outImage;outImage.clear();
    newImage.init(insampleID);
    outImage.append(newImage);
    QDir dstdir;bool ok=false;
    if(!QDir(QFileInfo(this->getImg_metadata_path(newImage.mImageID)).absolutePath()).exists())
        dstdir.mkpath(QFileInfo(this->getImg_metadata_path(newImage.mImageID)).absolutePath());
    if(QDir(QFileInfo(this->getImg_metadata_path(newImage.mImageID)).absolutePath()).exists())
        ok=writeImagelistToFile(this->getImg_metadata_path(newImage.mImageID),outImage);
    if(ok)
    {
        /*4 and 5*/
        this->listImages.append(newImage);
        if(this->listImages.size()==liz+1)
            return writeImagelistToFile(this->getImg_metadata_path(),this->listImages,true);
    }
    else
        dstdir.rmpath(QFileInfo(this->getImg_metadata_path(newImage.mImageID)).absolutePath());
    return false;
}
bool mDatabase::deleteImage(MImageType removeID)
{
    /*1. check the validity of the input `mImageID` or `sampleID`
      2. delete metadata file at `get_img_metadata_path(mImageID)`
      3. update info into `listmImages`
      4. update metadata file of all the images in db.
      5. delete all the related files and metadata
    */
    /*1*/
    V3DLONG liz=this->listImages.size();
    bool ok=false;
    QList<mImage> tmplistImages;tmplistImages.clear();
    for(V3DLONG i=0;i<liz;i++)
    {
        mImage temp=this->listImages.at(i);
        if(removeID==temp.mImageID)
        {
            ok=true;
            continue;
        }
        tmplistImages.append(this->listImages.at(i));
    }
    if(!ok)
        return false;
    /*2*/
    QDir dstdir;
    dstdir.rmpath(QFileInfo(this->getImg_metadata_path(removeID)).absolutePath());
    if(QDir(QFileInfo(this->getImg_metadata_path(newImage.mImageID)).absolutePath()).exists())
        return false;
    /*3*/
    this->listImages.clear();
    for(V3DLONG i=0;i<tmplistImages.size();i++)
        this->listImages.append(tmplistImages.at(i));
    /*4*/
    if(liz==this->listImages.size()-1)
        return writeImagelistToFile(this->getImg_metadata_path(),this->listImages);
    /*5*/
    //come here later
    //get soma and morphometry metadata and remove
    //remove morphometry database
    return false;
}
bool mDatabase::deleteImage(const QString &insampleID)
{
    V3DLONG liz=this->listImages.size();
    for(V3DLONG i=0;i<liz;i++)
    {
        mImage temp=this->listImages.at(i);
        if(QString::compare(insampleID,temp.sampleID))
            return this->deleteImage(temp.mImageID);
    }
    return false;
}
bool mDatabase::updateImage(mImage inImage)
{
    /*1. check the validity of the input `mImageID` and `sampleID`
      2. get metadata file at `get_img_metadata_path(mImageID)` and read to `mImagePointer`
      3. revise operation here
      4. update and write to metadata file at `get_img_metadata_path(mImageID)`*/
    /*1 and 2*/
    bool validatedID=getImagePointer(inImage.mImageID);
    if(validatedID)
    {
        this->mImagePointer=inImage;
        QList<mImage> outImage;outImage.clear();
        outImage.append(inImage);
        QDir dstdir;
        if(!QDir(QFileInfo(this->getImg_metadata_path(inImage.mImageID)).absolutePath()).exists())
            dstdir.mkpath(QFileInfo(this->getImg_metadata_path(inImage.mImageID)).absolutePath());
        if(QDir(QFileInfo(this->getImg_metadata_path(inImage.mImageID)).absolutePath()).exists())
            validatedID=writeImagelistToFile(this->getImg_metadata_path(inImage.mImageID),outImage);
    }
    return validatedID;
}
bool mDatabase::createMorpho(MImageType imageID=MinImageID,MSomaType somaID=MinSomaID)
{
    /* 1. check the input validity from listmMorphometry
     * 2. if from scratch, only first two paras are needed.
     * 2.1 register at <db>/metadata/morphometry/basic/imageID/somaID/morphometry.metadata
     * 2.2 update the new morphometry to listmMorphometry
     * 2.3 create an init version with the metadata of soma inside <file:imageID_somaID_MinMorphometryID_soma.metadata>
     *          at <db>/morphometry/imageID/somaID/MinMorphometryID/
     * 3. if not, assign new morphometryID
     *
     * question is that: i have to scan all the metadata files in Metadata_morphometry path, sometimes processing ten of thousands of metadata to get the list. find a more effiency way!
    */
}
QString mDatabase::getMorpho_db_path()
{
    QString outpath="";
    QDir dbDir(dbpath);
    if(dbpath.isEmpty()||!dbDir.exists())
        return outpath;
    outpath=dbpath+"/"+db_init_1stlayer[MORPHOMETRYDB];
    return outpath;
}
QString mDatabase::getMorpho_path(MImageType qImageID)
{
    /*check the validity of query image id, from listImages*/
    QString outpath="";
    for(V3DLONG i=0;i<this->listImages.size();i++)
    {
        if(this->listImages.at(i).mImageID==qImageID)
        {
            outpath=this->getMorpho_db_path()+"/"+QString::number(qImageID);
            break;
        }
    }
    return outpath;
}
QString mDatabase::getMorpho_path(MImageType qImageID,MSomaType qSomaID)
{
    /*check the validity of query image id (from listImages) and query soma id (from listSomata)*/
    QString outpath="";
    for(V3DLONG j=0;j<this->listmSomata.size();j++)
    {
        if(this->listmSomata.at(j).mImageID==qImageID)
        {
            QList<mSoma> listSomata=this->listmSomata.at(j).listSomata;
            outpath=(this->getMorpho_db_path().isEmpty())
                    ?this->getMorpho_db_path():(this->getMorpho_db_path()+"/"+QString::number(qImageID));
            if(outpath.isEmpty())
                return outpath;
            for(V3DLONG i=0;i<listSomata.size();i++)
            {
                if(listSomata.at(i).mSomaID==qSomaID)
                {
                    outpath+=("/"+QString::number(qSomaID));
                    return outpath;
                }
            }
        }
    }
    return outpath;
}
QString mDatabase::getMorpho_path(MImageType qImageID,MSomaType qSomaID,MMorphoType qMorphoID)
{
    /*check the validity of query image id (from listImages), query soma id (from listSomata)
     * and query morphometry id (from listmMorphometry)*/
    QString outpath="";
    outpath=this->getMorpho_path(qImageID,qSomaID);
    if(outpath.isEmpty())
        return outpath;
    for(V3DLONG j=0;j<this->listmMorphometry.size();j++)
    {
        mMorphometrylist temp=listmMorphometry.at(j);
        if(temp.mImageID==qImageID&&temp.mSomaID==qSomaID)
        {
            QList<mMorphometry> listmorphometry=temp.listMorphometry;
            for(V3DLONG i=0;i<listmorphometry.size();i++)
            {
                mMorphometry tempMorpho=listmorphometry.at(i);
                if(tempMorpho.mMorphometryID==qMorphoID)
                {
                    outpath+=("/"+QString::number(qMorphoID));
                    return outpath;
                }
            }
        }
    }
    return outpath;
}
QString mDatabase::getMorpho_metadata_path()
{
    QString outpath="";
    QDir dbDir(dbpath);
    if(dbpath.isEmpty()||!dbDir.exists())
        return outpath;
    outpath=dbpath+"/"+db_init_1stlayer[METADATA]+"/"+db_init_2stlayer_metadata[MORPHOMETRY]+"/"+
            db_init_3stlayer_metadata[BASIC];
    return outpath;
}
QString mDatabase::getMorpho_metadata_path(MImageType qImageID)
{
    QString outpath="";
    outpath=this->getMorpho_metadata_path();
    if(outpath.isEmpty())
        return outpath;
    V3DLONG lmz=this->listmMorphometry.size();
    for(V3DLONG i=0;i<lmz;i++)
    {
        mMorphometrylist temp=this->listmMorphometry.at(i);
        if(temp.mImageID==qImageID)
        {
            outpath+=("/"+QString::number(qImageID));
            return outpath;
        }
    }
    return outpath;
}
QString mDatabase::getMorpho_metadata_path(MImageType qImageID,MSomaType qSomaID)
{
    QString outpath="";
    outpath=this->getMorpho_metadata_path(qImageID);
    if(outpath.isEmpty())
        return outpath;
    V3DLONG lmz=this->listmMorphometry.size();
    for(V3DLONG i=0;i<lmz;i++)
    {
        mMorphometrylist temp=this->listmMorphometry.at(i);
        if(temp.mSomaID==qSomaID)
            outpath+=("/"+QString::number(qSomaID)+"/"+db_init_2stlayer_metadata[MORPHOMETRY]+".metadata");
    }
    return outpath;
}
bool mDatabase::somaInitialization(bool start=true)
{
    if(start)
    {
        db_soma_metadata_path=soma_metadata_path="";
        listSomata.clear();mSomataPointer.clear();
    }
    else
    {
        QDir soma_metadata_Dir(this->soma_metadata_path);
        if(!this->soma_metadata_path.isEmpty()&&soma_metadata_Dir.exists())
        {
            this->listmSomata=getSomaMorpholist();
            //3. init of morphometry list
            if(!this->morpho_metadata_path.isEmpty())
            {
                //come here later
            }
        }
        else
            qDebug()<<"Warnning:meta data of soma is empty.";
    }
    return true;
}
QString mDatabase::getSoma_metadata_path()
{
    /*return the path of all the somata list*/
    QString outpath="";
    QDir dbDir(dbpath);
    if(dbpath.isEmpty()||!dbDir.exists())
        return outpath;
    outpath=dbpath+"/"+db_init_1stlayer[METADATA]+"/"+db_init_2stlayer_metadata[SOMA]+"/"+
            db_init_3stlayer_metadata[BASIC];
    return outpath;
}
QString mDatabase::getSoma_metadata_path(MImageType queryID)
{
    QString outpath="";
    outpath=this->getSoma_metadata_path();
    if(outpath.isEmpty())
        return outpath;
    V3DLONG lsz=this->listmSomata.size();
    for(V3DLONG i=0;i<lsz;i++)
    {
        mSomaMorpho temp=this->listmSomata.at(i);
        if(temp.mImageID==queryID)
            outpath+=("/"+QString::number(queryID)+"/"+db_init_2stlayer_metadata[SOMA]+".metadata");
    }
    return outpath;
}
QString mDatabase::getSoma_metadata_path(const QString &insampleID)
{
    QString outpath="";
    V3DLONG liz=this->listImages.size();
    for(V3DLONG i=0;i<liz;i++)
    {
        mImage temp=this->listImages.at(i);
        if(QString::compare(insampleID,temp.sampleID))
            return getSoma_metadata_path(temp.mImageID);
    }
    return outpath;
}
QList<mSomaMorpho> mDatabase::getSomaMorpholist()
{
    return this->getSomaMorpholist(this->listImages);
}
QList<mSomaMorpho> mDatabase::getSomaMorpholist(QList<mImage> queryImagelist)
{
    /*for getting or loading all the soma metadata of input image datasets*/
    QList<mSomaMorpho> outlist; outlist.clear();
    if(queryImagelist.size())
        return outlist;
    //2. for each image, get the metadata path
    for(V3DLONG i=0;i<queryImagelist.size();i++)
    {
        mImage temp=queryImagelist.at(i);
        QString metadataPath=this->getSoma_metadata_path(QString::number(temp.mImageID));
        QDir inDir(metadataPath);
        if(!inDir.exists())
            continue;
        mSomaMorpho tempmSomaMorpho=getSomaMorpho(metadataPath,temp.mImageID);
        outlist.append(tempmSomaMorpho);
    }
    return outlist;
}
bool mDatabase::deleteSomaMorpho(MImageType removeID)
{
    QList<mSomaMorpho> newlistmSomata;newlistmSomata.clear();
    //1. make sure it is an userful id
    if(removeID<MinImageID||removeID>MaxImageID)
        return false;
    V3DLONG lsz=this->listmSomata.size();
    for(V3DLONG i=0;i<lsz;i++)
    {
        mSomaMorpho temp=this->listmSomata.at(i);
        if(temp.mImageID==removeID)
            continue;
        newlistmSomata.append(temp);
    }
    if(newlistmSomata.size()==this->listmSomata.size()+1)
    {
        //update listmSomata
        this->listmSomata.clear();
        for(V3DLONG i=0;i<newlistmSomata.size();i++)
            this->listmSomata.append(newlistmSomata.at(i));
        //detele path and soma.metada
        QFile rmfile(this->getSoma_metadata_path(removeID)); rmfile.remove();
        QDir rmpath(this->getSoma_metadata_path());
        return rmpath.rmdir(QString::number(removeID));
    }
    return false;
}
bool mDatabase::deleteSomaMorpho(const QString &insampleID)
{
    //use mlistImage get the mImageID
    V3DLONG liz=this->listImages.size();
    for(V3DLONG i=0;i<liz;i++)
    {
        mImage temp=this->listImages.at(i);
        if(QString::compare(insampleID,temp.sampleID))
            return deleteSomaMorpho(temp.mImageID);
    }
    return false;
}
bool mDatabase::updateSomaMorpho_fromApo(const QString &inapo,MImageType inmImageID)
{
    QList<mSomaMorpho> updatedlist; updatedlist.clear();
    mSomaMorpho inmSomaMorpho; inmSomaMorpho.listSomata.clear();
    if(inmImageID>MaxImageID||inmImageID<MinImageID||inapo.isEmpty())
        return false;
    V3DLONG liz=this->listImages.size();
    for(V3DLONG i=0;i<liz;i++)
    {
        mImage temp=this->listImages.at(i);
        if(temp.mImageID==inmImageID)
            break;
        if(i==liz-1)
            return false;
    }
    QString saveTo=this->getSoma_metadata_path(inmImageID);
    QFile saveTofile(saveTo);
    if(!saveTofile.exists())
        return createNewSomaMorpho_fromApo(inapo,inmImageID);
    else
    {
        QList <CellAPO> inapolist=readAPO_file(inapo);
        if(inapolist.size()+MinSomaID>MaxSomaID)
        {
            QMessageBox::warning(0,"Input Error",QObject::tr("Input apo size is over 90k!"));
            return false;
        }
        //get exited mSomaMorpho
        V3DLONG lsz=this->listmSomata.size();
        for(V3DLONG i=0;i<lsz;i++)
        {
            mSomaMorpho updatedSomaMorpho=this->listmSomata.at(i);
            if(updatedSomaMorpho.mImageID==inmImageID)
            {
                inmSomaMorpho.listSomata=updatedSomaMorpho.listSomata;
                inmSomaMorpho.mImageID=inmImageID;
                continue;
            }
            updatedlist.append(updatedSomaMorpho);
        }
        //get max somaid
        MSomaType maxsomaid=inmSomaMorpho.listSomata.size()+MinSomaID;
        for(V3DLONG i=0;i<inmSomaMorpho.listSomata.size();i++)
            if(maxsomaid<inmSomaMorpho.listSomata.at(i).mSomaID)
                maxsomaid=inmSomaMorpho.listSomata.at(i).mSomaID+1;
        for(V3DLONG i=0;i<inapolist.size();i++)
        {
            CellAPO tempapo=inapolist.at(i);
            mSoma tomSoma=apo2mSoma(tempapo);
            tomSoma.mSomaID=maxsomaid+i;
            inmSomaMorpho.listSomata.append(tomSoma);
        }
        //save new list to listmSomata
        updatedlist.append(inmSomaMorpho);
        this->listmSomata=updatedlist;
        //save to file
        return writeSomaMorphoToFile(saveTo,inmSomaMorpho);
    }
}
/*scan apo file, convert apo to mSomaMorpho, add to listSomata, save to metadata*/
bool mDatabase::createNewSomaMorpho_fromApo(const QString &inapo,MImageType inmImageID)
{
    mSomaMorpho inmSomaMorpho; inmSomaMorpho.listSomata.clear();
    if(inmImageID<MinImageID||inapo.isEmpty())
        return false;
    V3DLONG liz=this->listImages.size();
    for(V3DLONG i=0;i<liz;i++)
    {
        mImage temp=this->listImages.at(i);
        if(temp.mImageID==inmImageID)
            break;
        if(i==liz-1)
            return false;
    }
    QString saveTo=this->getSoma_metadata_path(inmImageID);

    QList <CellAPO> inapolist=readAPO_file(inapo);
    if(inapolist.size()+MinSomaID>MaxSomaID)
    {
        QMessageBox::warning(0,"Input Error",QObject::tr("Input apo size is over 90k!"));
        return false;
    }
    for(V3DLONG i=0;i<inapolist.size();i++)
    {
        CellAPO tempapo=inapolist.at(i);
        mSoma tomSoma=apo2mSoma(tempapo);
        tomSoma.mSomaID=MinSomaID+i;
        inmSomaMorpho.listSomata.append(tomSoma);
    }
    return writeSomaMorphoToFile(saveTo,inmSomaMorpho);
}
QStringList mImage::getDataNumber(bool basic)
{
    QStringList out;out.clear();
    if(basic)
    {
        out.append(QString::number(this->mImageID));
        out.append(this->getName());
        out.append(this->sampleID);
        return out;
    }
    else
    {
        out.append(QString::number(this->mImageID));
        out.append(this->getName());
        out.append(this->sampleID);
        out.append(this->sourceID);
        out.append(this->mFormatList[this->mFormatID]);
        out.append(this->mObjectList[this->mObjectID]);
        out.append(QString::number(this->mIS.sx));
        out.append(QString::number(this->mIS.sy));
        out.append(QString::number(this->mIS.sz));
        out.append(QString::number(this->mVR.rx));
        out.append(QString::number(this->mVR.ry));
        out.append(QString::number(this->mVR.rz));
        out.append(this->comments);
        return out;
    }
}
QStringList mSoma::getDataNumber(bool basic)
{
    QStringList out;out.clear();
    if(basic)
    {
        out.append(QString::number(this->mImageID));
        out.append(QString::number(this->mSomaID));
        out.append(this->getName());
        return out;
    }
    out.append(QString::number(this->mImageID));
    out.append(QString::number(this->mSomaID));
    out.append(this->getName());
    out.append(QString::number(this->position.mx));
    out.append(QString::number(this->position.my));
    out.append(QString::number(this->position.mz));
    out.append(QString::number(this->radius));
    out.append(QString::number(this->color.r));
    out.append(QString::number(this->color.g));
    out.append(QString::number(this->color.b));
    out.append(QString::number(this->color.a));
    out.append(QString::number(this->intensity));
    out.append(QString::number(this->volume));
    out.append(this->comments);
    out.append(this->region);
    out.append(this->cellType);
    return out;
}
QStringList mMorphometry::getDataNumber()
{
    QStringList out;out.clear();
    out.append(QString::number(this->mMorphometryID));
    out.append(this->name);
    out.append(this->morphometryType);
    out.append(this->tag);
    out.append(this->comments);
    QString outfiles=(!this->morphoFileNamelist.size())?"":this->morphoFileNamelist.at(0);
    for(int i=1; i<this->morphoFileNamelist.size();i++)
        outfiles+=(" "+this->morphoFileNamelist.at(i));
    out.append(outfiles);
    return out;
}
void mMorphometrylist::reNamelistMorphometry()
{
    /*mImageID+mSomaID+mmID*/
    if(this->listMorphometry.size())
    {
        for(V3DLONG i=0;i<listMorphometry.size();i++)
        {
            this->listMorphometry.at(i).name="Image_"+QString::number(this->mImageID)
                    +"_soma_"+QString::number(this->mSomaID)
                    +"_type_"+this->listMorphometry.at(i).morphometryType;
        }
    }
}
mSoma createSomaMorphoFromQSL(QStringList inlist,bool basic)
{
    mSoma out;
    if(basic)
    {
        out.mImageID=inlist.at(0).toLong();
        out.mSomaID=inlist.at(1).toULong();
        out.name=inlist.at(2);
        return out;
    }
    out.mImageID=inlist.at(0).toLong();
    out.mSomaID=inlist.at(1).toULong();
    out.name=inlist.at(2);
    out.position.mx=inlist.at(3).toFloat();
    out.position.my=inlist.at(4).toFloat();
    out.position.mz=inlist.at(5).toFloat();
    out.radius=inlist.at(6).toFloat();
    out.color.r=inlist.at(7).toUInt();
    out.color.g=inlist.at(8).toUInt();
    out.color.b=inlist.at(9).toUInt();
    out.color.a=inlist.at(10).toUInt();
    out.intensity=inlist.at(11).toFloat();
    out.volume=inlist.at(12).toFloat();
    out.comments=inlist.at(13);
    out.region=inlist.at(14);
    out.cellType=inlist.at(15);
    return out;
}
QList<mSoma> getSomalist(const QString& inpath,bool basic)
{
    QList<mSoma> outSomalist;outSomalist.clear();
    //1.scan conf path.
    QFile scanconffile(inpath);
    if(!scanconffile.exists())
        QMessageBox::warning(0,"File Not Found","Can't find configuration file at Input path!");
    else
    {
        //2.get all lines from the conf file.
        if (scanconffile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            while (! scanconffile.atEnd())
            {
                char _buf[1000], *buf;
                scanconffile.readLine(_buf, sizeof(_buf));
                for (buf=_buf; (*buf && *buf==' '); buf++); //skip space
                if (buf[0]=='\0')	continue;
                if (buf[0]=='#') continue;
                QStringList qsl = QString(buf).trimmed().split(",");
                if (qsl.size()==0)   continue;
                mSoma tempi=createSomaMorphoFromQSL(qsl,basic);
                outSomalist.append(tempi);
            }
        }
    }
    return outSomalist;
}
bool writeSomalistToFile(const QString& topath,QList<mSoma> &insomalist,bool basic)
{
    if (topath.isEmpty()||insomalist.size()==0)
        return false;
    QFile scanconffile(topath);
    //get title
    mSoma temp;QString confTitle="#";
    temp.setDataNumberTitle(basic);
    if(temp.dataNumberTitle.size())
    {
        confTitle+=temp.dataNumberTitle.at(0);
        for (V3DLONG i=1;i<temp.dataNumberTitle.size();i++)
            confTitle+=(","+temp.dataNumberTitle.at(i));
    }
    confTitle+="\n";
    if(scanconffile.exists())
        QMessageBox::warning(0,"File overwrite",QObject::tr("already exist %1 !").arg(topath));
    if(scanconffile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        //title
        scanconffile.write(confTitle.toAscii());
        //inside
        for(V3DLONG i=0;i<insomalist.size();i++)
        {
            mSoma tempi=insomalist.at(i);
            tempi.getDataNumber(basic);
            QString data=tempi.dataNumber.at(0);
            for(int j=1;j<tempi.dataNumber.size();j++)
                data+=(","+tempi.dataNumber.at(j));
            data+="\n";
            scanconffile.write(data.toAscii());
        }
        scanconffile.close();
        return true;
    }
    return false;
}
mSoma apo2mSoma(CellAPO inapo)
{
    mSoma outmSoma;
    outmSoma.comments=inapo.comment;
    outmSoma.color=inapo.color;
    outmSoma.intensity=inapo.intensity;
    outmSoma.volume=inapo.volsize;
    outmSoma.position.mx=inapo.x;
    outmSoma.position.my=inapo.y;
    outmSoma.position.mz=inapo.z;
    return outmSoma;
}
mImage createImageFromQSL(QStringList inlist,bool basic)
{
    mImage out;out.setDataNumberTitle(basic);
    if(inlist.size()!=out.dataNumberTitle.size())
        return out;
    if(basic)
    {
        out.mImageID=inlist.at(0).toULong();
        out.name=inlist.at(1);
        out.sampleID=inlist.at(2);
    }
    else
    {
        out.mImageID=inlist.at(0).toULong();
        out.name=inlist.at(1);
        out.sampleID=inlist.at(2);
        out.sourceID=inlist.at(3);
        if(out.mFormatList.contains(inlist.at(3)))
            out.mFormatID=out.mFormatList.indexOf(inlist.at(3));
        if(out.mObjectList.contains(inlist.at(4)))
            out.mFormatID=out.mObjectList.indexOf(inlist.at(4));
        out.mIS.setmIS(inlist.at(5).toULong(),inlist.at(6).toULong(),inlist.at(7).toULong());
        out.mVR.setmVR(inlist.at(8).toDouble(),inlist.at(9).toDouble(),inlist.at(10).toDouble());
        out.comments=inlist.at(11);
    }
    return out;
}
QList<mImage> getImagelist(const QString& inpath,bool basic)
{
    QList<mImage> outlist;
    outlist.clear();
    //1.scan conf path.
    QFile scanconffile(inpath);
    if(!scanconffile.exists())
        QMessageBox::warning(0,"File Not Found","Can't find configuration file at Input path!");
    else
    {
        //2.get all lines from the conf file.
        if (scanconffile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            while (! scanconffile.atEnd())
            {
                char _buf[1000], *buf;
                scanconffile.readLine(_buf, sizeof(_buf));
                for (buf=_buf; (*buf && *buf==' '); buf++); //skip space
                if (buf[0]=='\0')	continue;
                if (buf[0]=='#') continue;
                QStringList qsl = QString(buf).trimmed().split(",");
                if (qsl.size()==0)   continue;
                mImage tempi=createImageFromQSL(qsl,basic);
                outlist.append(tempi);
            }
        }
    }
    //3.write to Qlist
    return outlist;
}
mImage getImage(const QString& inpath)
{
    mImage out=new mImage();
    QFile scanconffile(inpath);
    if(!scanconffile.exists())
        QMessageBox::warning(0,"File Not Found","Can't find configuration file at Input path!");
    else
    {
        //2.get all lines from the conf file.
        if (scanconffile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            while (! scanconffile.atEnd())
            {
                char _buf[1000], *buf;
                scanconffile.readLine(_buf, sizeof(_buf));
                for (buf=_buf; (*buf && *buf==' '); buf++); //skip space
                if (buf[0]=='\0')	continue;
                if (buf[0]=='#') continue;
                QStringList qsl = QString(buf).trimmed().split(",");
                if (qsl.size()==0)   continue;
                out=createImageFromQSL(qsl);
            }
        }
    }
    return out;
}
bool writeImagelistToFile(const QString &topath, QList<mImage> &inlist,bool basic)
{
    if (topath.isEmpty()||inlist.size()==0)
        return false;
    QFile scanconffile(topath);
    //get title
    mImage temp;temp.setDataNumberTitle(basic);
    QString confTitle="#";
    if(temp.dataNumberTitle.size())
    {
        confTitle+=temp.dataNumberTitle.at(0);
        for (V3DLONG i=1;i<temp.dataNumberTitle.size();i++)
            confTitle+=(","+temp.dataNumberTitle.at(i));
    }
    confTitle+="\n";
    if(scanconffile.exists())
        QMessageBox::warning(0,"File overwrite",QObject::tr("already exist %1 !").arg(topath));
    if(scanconffile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        //title
        scanconffile.write(confTitle.toAscii());
        //inside
        for(V3DLONG i=0;i<inlist.size();i++)
        {
            mImage tempi=inlist.at(i);
            tempi.getDataNumber(basic);
            QString data=tempi.dataNumber.at(0);
            for(int j=1;j<tempi.dataNumber.size();j++)
                data+=(","+tempi.dataNumber.at(j));
            data+="\n";
            scanconffile.write(data.toAscii());
        }
        scanconffile.close();
        return true;
    }
    return false;
}
