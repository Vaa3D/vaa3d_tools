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
        imageInitialization(start);
        somaInitialization(start);
        morphoInitialization(start);
    }
    return true;
}
bool mDatabase::imageInitialization(bool start)
{
    if(start)
    {
        imgdbpath=db_img_metadata_path=img_metadata_path="";
        listImages.clear();
    }
    else
    {
        this->db_img_metadata_path=this->getImg_metadata_path();
        //init when load the database, listImages from db_img_metadata_path
        QFile img_metadata_file(this->db_img_metadata_path);
        if(!this->db_img_metadata_path.isEmpty()&&img_metadata_file.exists())
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
bool mDatabase::setImgDBpath(const QString &inpath)
{
    if(inpath.isEmpty()||!QDir(inpath).exists())
        return false;
    this->imgdbpath=inpath;
    return true;
}
QString mDatabase::getImg_metadata_path()
{
    QString outpath="";QDir dbDir(dbpath);
    if(dbpath.isEmpty()||!dbDir.exists())
        return outpath;
    outpath=dbpath+"/"+db_init_1stlayer[METADATA]+"/"+db_init_2stlayer_metadata[IMAGE]+"/"+
            db_init_3stlayer_metadata[BASIC]+"/"+db_init_2stlayer_metadata[IMAGE]+".metadata";
    QFile metaFile(outpath);
    if(!metaFile.exists())
    {
        qDebug()<<"Requested all image metadata is not existed.";
    }
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
    if(this->img_metadata_path.isEmpty()||!QFile(this->img_metadata_path).exists())
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
    if(QDir(QFileInfo(this->getImg_metadata_path(removeID)).absolutePath()).exists())
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
    return this->deleteSomalist(removeID);
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
bool mDatabase::somaInitialization(bool start)
{
    if(start)
    {
        db_soma_metadata_path=soma_metadata_path="";
        listSomata.clear();mSomataPointer.clear();
    }
    else
    {
        this->db_soma_metadata_path=this->getSoma_metadata_path();
        //load from this soma.metadata
        QFile soma_metadata_file(this->db_soma_metadata_path);
        if(!this->db_soma_metadata_path.isEmpty()&&soma_metadata_file.exists())
        {
            this->listSomata=getImagelist(this->db_soma_metadata_path,true);
        }
        if(!this->listSomata.size())
            qDebug()<<"Meta data of somalist is empty.";
    }
    return true;
}
QString mDatabase::getSoma_metadata_path()
{
    /*return the path of all the somata list*/
    QString outpath="";QDir dbDir(dbpath);
    if(dbpath.isEmpty()||!dbDir.exists())
        return outpath;
    outpath=dbpath+"/"+db_init_1stlayer[METADATA]+"/"+db_init_2stlayer_metadata[SOMA]+"/"+
            db_init_3stlayer_metadata[BASIC]+"/"+db_init_2stlayer_metadata[SOMA]+".metadata";
    QFile metaFile(outpath);
    if(!metaFile.exists())
    {
        qDebug()<<"Requested soma metadata is not existed.";
    }
    return outpath;
}
QString mDatabase::getSoma_metadata_path(MImageType qImageID)
{
    QString outpath=""; QDir dbDir(dbpath);
    if(dbpath.isEmpty()||!dbDir.exists()||qImageID<MinImageID||qImageID>MaxImageID)
        return outpath;
    V3DLONG lsz=this->listSomata.size();
    for(V3DLONG i=0;i<lsz;i++)
    {
        mImage temp=this->listSomata.at(i);
        if(qImageID==temp.mImageID)
        {
            outpath= dbpath+"/"+db_init_1stlayer[METADATA]+"/"+db_init_2stlayer_metadata[SOMA]+"/"+
                    db_init_3stlayer_metadata[DETAIL]+"/"+QString::number(qImageID)+"/"+
                    db_init_2stlayer_metadata[SOMA]+".metadata";
            break;
        }
    }
    QFile metaFile(outpath);
    if(!metaFile.exists())
    {
        qDebug()<<"Requested soma metadata of "<<QString::number(qImageID)<<" is not existed.";
    }
    return outpath;
}
QString mDatabase::getSoma_metadata_path(const QString &insampleID)
{
    QString outpath="";
    V3DLONG lsz=this->listSomata.size();
    for(V3DLONG i=0;i<lsz;i++)
    {
        mImage temp=this->listSomata.at(i);
        if(QString::compare(insampleID,temp.sampleID))
            return getSoma_metadata_path(temp.mImageID);
    }
    return outpath;
}
bool mDatabase::getSomaPointer(MImageType qImageID)
{
    this->soma_metadata_path=this->getSoma_metadata_path(qImageID);
    if(this->soma_metadata_path.isEmpty()||QFile(this->soma_metadata_path).exists())
        return false;
    this->mSomataPointer=getSomalist(this->soma_metadata_path);
    if(!this->mSomataPointer.size())
        return false;
    return true;
}
bool mDatabase::getSomaPointer(const QString &insampleID)
{
    V3DLONG lsz=this->listSomata.size();
    for(V3DLONG i=0;i<lsz;i++)
    {
        mImage temp=this->listSomata.at(i);
        if(QString::compare(insampleID,temp.sampleID))
            return this->getSomaPointer(temp.mImageID);
    }
}
bool mDatabase::addSomalist(const QString &inapo,MImageType inmImageID)
{
    /*()
     * 1. check the validity of the input `mImageID` or `sampleID` from `listImages` and `listSomata`
      2. create a metadata file at `<db>/metadata/soma/detail/<newID>/image.metadata`, `get_soma_metadata_path(imageid)`
      3. update `listSomata`
      4. save to file at `get_soma_metadata_path()`*/
    //1
    V3DLONG liz=this->listImages.size();
    V3DLONG lsz=this->listSomata.size();
    QList<mImage> newlistSomata; newlistSomata.clear();
    bool validated=false;
    for(V3DLONG i=0;i<lsz;i++)
    {
        mImage temp=this->listSomata.at(i);
        if(inmImageID==temp.mImageID)
        {
            validated=true;
            break;
        }
        newlistSomata.append(temp);
    }
    if(validated)
        return false;
    QList <CellAPO> inapolist=readAPO_file(inapo);
    if(inapo.isEmpty()||!QFile(inapo).exists()||!inapolist.size())
        return false;
    V3DLONG targetID=MaxImageID+1;validated=false;
    for(V3DLONG i=0;i<liz;i++)
    {
        mImage temp=this->listImages.at(i);
        if(inmImageID==temp.mImageID)
        {
            targetID=i;
            validated=true;
            break;
        }
    }
    if(validated||targetID>MaxImageID)
        return false;
    //3 and 4
    newlistSomata.append(this->listImages.at(targetID));
    if(newlistSomata.size()==lsz-1)
        validated=updateListSomata(newlistSomata);
    //2
    //convert apo to soma
    QList<mSoma> insoma;insoma.clear();
    for(V3DLONG i=0;i<inapolist.size();i++)
    {
        CellAPO tempapo=inapolist.at(i);
        mSoma tomSoma=apo2mSoma(tempapo);
        tomSoma.mImageID=inmImageID;
        tomSoma.mSomaID=MinSomaID+i;
        insoma.append(tomSoma);
    }
    QDir dstdir;
    if(!QDir(QFileInfo(this->getSoma_metadata_path(inmImageID)).absolutePath()).exists())
        dstdir.mkpath(QFileInfo(this->getSoma_metadata_path(inmImageID)).absolutePath());
    if(QDir(QFileInfo(this->getSoma_metadata_path(inmImageID)).absolutePath()).exists())
        validated=writeSomalistToFile(this->getSoma_metadata_path(inmImageID),insoma);
    return validated;
}
bool mDatabase::deleteSomalist(MImageType removeID)
{
    /*1. check the validity of the input `mImageID` or `sampleID` from `listSomata`
      2. delete metadata file at `get_soma_metadata_path(imageid)`
        2.1 remove related data
      3. update `listSomata`
      4. save to file at `get_soma_metadata_path()`*/
    //1
    V3DLONG lsz=this->listSomata.size();
    bool validated=false;
    QList<mImage> newlistSomata;newlistSomata.clear();
    for(V3DLONG i=0;i<lsz;i++)
    {
        mImage temp=this->listSomata.at(i);
        if(removeID==temp.mImageID)
        {
            validated=true;
            continue;
        }
        newlistSomata.append(temp);
    }
    if(!validated)
        return false;
    //2
    QDir dstdir;
    dstdir.rmpath(QFileInfo(this->getSoma_metadata_path(removeID)).absolutePath());
    if(QDir(QFileInfo(this->getSoma_metadata_path(removeID)).absolutePath()).exists())
        return false;
    //come here later
    //3 and 4
    if(newlistSomata.size()==lsz-1)
        validated=updateListSomata(newlistSomata);
    else
        return false;
    return this->deleteMorpho(removeID);
}
bool mDatabase::deleteSomalist(const QString &insampleID)
{
    V3DLONG lsz=this->listSomata.size();
    for(V3DLONG i=0;i<lsz;i++)
    {
        mImage temp=this->listSomata.at(i);
        if(QString::compare(insampleID,temp.sampleID))
            return this->deleteSomalist(temp.mImageID);
    }
}
bool mDatabase::updateListSomata(QList<mImage> newlist)
{
    /*update listSomata with the input list and save to metadata*/
    bool out=false;
    if(newlist.size())
    {
        //write first
        QDir dstdir;
        if(!QDir(this->getSoma_metadata_path()).exists())
            dstdir.mkpath(this->getSoma_metadata_path());
        if(QDir(this->getSoma_metadata_path()).exists())
            out=writeImagelistToFile(this->getSoma_metadata_path(),newlist,true);
        //update second
        if(out)
        {
            this->listSomata.clear();
            for(V3DLONG i=0;i<newlist.size();i++)
                this->listSomata.append(newlist.at(i));
            if(newlist.size()!=this->listSomata.size())
                out=false;
        }
    }
    return out;
}
bool mDatabase::updateSomalist(const QString &inapo,MImageType inmImageID)
{
    /*1. check the validity of the input `mImageID` or `sampleID` from `listSomata`
      2. get metadata file at `get_soma_metadata_path(mImageID)` and read to `mSomataPointer`
      3. revise operation here
      4. update and write to metadata file at `get_soma_metadata_path(mImageID)`*/
    //1
    QList <CellAPO> inapolist=readAPO_file(inapo);
    if(inapo.isEmpty()||!QFile(inapo).exists()||!inapolist.size())
        return false;
    V3DLONG lsz=this->listSomata.size();
    bool validated=false;
    for(V3DLONG i=0;i<lsz;i++)
    {
        mImage temp=this->listSomata.at(i);
        if(inmImageID==temp.mImageID)
        {
            validated=true;
            break;
        }
    }
    if(!validated)
        return false;
    //2
    this->getSomaPointer(inmImageID);
    QList<mSoma> insoma;insoma.clear();
    for(V3DLONG i=0;i<inapolist.size();i++)
    {
        CellAPO tempapo=inapolist.at(i);
        mSoma tomSoma=apo2mSoma(tempapo);
        tomSoma.mImageID=inmImageID;
//        tomSoma.mSomaID=MinSomaID+i;
        tomSoma.mSomaID=MinSomaID;
        insoma.append(tomSoma);
    }
    //
    QList<mSoma> newsomalist;newsomalist.clear();
    QList<MSomaType> somaidlist; somaidlist.clear();
    for(V3DLONG i=0;i<this->mSomataPointer.size();i++)
    {
        newsomalist.append(this->mSomataPointer.at(i));
        somaidlist.append(this->mSomataPointer.at(i).mSomaID);
    }
    //assign somaid to new somata
    for(V3DLONG i=0;i<insoma.size();i++)
    {
        mSoma newSoma=insoma.at(i);
        for(V3DLONG j=MinSomaID;j<MaxSomaID+1;j++)
        {
            if(somaidlist.contains(j))
                continue;
            newSoma.mSomaID=j;
            somaidlist.append(j);
            break;
        }
        newsomalist.append(newSoma);
    }
    if(newsomalist.size()!=insoma.size()+this->mSomataPointer.size())
        return false;
    else
    {
        this->mSomataPointer.clear();
        //update mSomataPointer
        for(V3DLONG i=0;i<newsomalist.size();i++)
            this->mSomataPointer.append(newsomalist.at(i));
    }
    //write to metadata file
    if(newsomalist.size()!=this->mSomataPointer.size())
        return false;
    QDir dstdir;
    if(!QDir(QFileInfo(this->getSoma_metadata_path(inmImageID)).absolutePath()).exists())
        dstdir.mkpath(QFileInfo(this->getSoma_metadata_path(inmImageID)).absolutePath());
    if(QDir(QFileInfo(this->getSoma_metadata_path(inmImageID)).absolutePath()).exists())
        validated=writeSomalistToFile(this->getSoma_metadata_path(inmImageID),this->mSomataPointer);
    return validated;
}
bool mDatabase::updateSomalist(const QString &inapo,const QString &insampleID)
{
    V3DLONG lsz=this->listSomata.size();
    for(V3DLONG i=0;i<lsz;i++)
    {
        mImage temp=this->listSomata.at(i);
        if(QString::compare(insampleID,temp.sampleID))
            return this->updateSomalist(inapo,temp.mImageID);
    }
}
bool mDatabase::updateSomalist(QList<mSoma> insoma,MImageType inmImageID)
{
    if(!insoma.size())
        return false;
    V3DLONG lsz=this->listSomata.size();
    bool validated=false;
    for(V3DLONG i=0;i<lsz;i++)
    {
        mImage temp=this->listSomata.at(i);
        if(inmImageID==temp.mImageID)
        {
            validated=true;
            break;
        }
    }
    if(!validated)
        return false;
    //2
    this->getSomaPointer(inmImageID);
    //
    QList<mSoma> newsomalist;newsomalist.clear();
    QList<MSomaType> somaidlist; somaidlist.clear();
    for(V3DLONG i=0;i<this->mSomataPointer.size();i++)
    {
        newsomalist.append(this->mSomataPointer.at(i));
        somaidlist.append(this->mSomataPointer.at(i).mSomaID);
    }
    //assign somaid to new somata
    for(V3DLONG i=0;i<insoma.size();i++)
    {
        mSoma newSoma=insoma.at(i);
        for(V3DLONG j=MinSomaID;j<MaxSomaID+1;j++)
        {
            if(somaidlist.contains(j))
                continue;
            newSoma.mSomaID=j;
            somaidlist.append(j);
            break;
        }
        newsomalist.append(newSoma);
    }
    if(newsomalist.size()!=insoma.size()+this->mSomataPointer.size())
        return false;
    else
    {
        this->mSomataPointer.clear();
        //update mSomataPointer
        for(V3DLONG i=0;i<newsomalist.size();i++)
            this->mSomataPointer.append(newsomalist.at(i));
    }
    //write to metadata file
    if(newsomalist.size()!=this->mSomataPointer.size())
        return false;
    QDir dstdir;
    if(!QDir(QFileInfo(this->getSoma_metadata_path(inmImageID)).absolutePath()).exists())
        dstdir.mkpath(QFileInfo(this->getSoma_metadata_path(inmImageID)).absolutePath());
    if(QDir(QFileInfo(this->getSoma_metadata_path(inmImageID)).absolutePath()).exists())
        validated=writeSomalistToFile(this->getSoma_metadata_path(inmImageID),this->mSomataPointer);
    return validated;
}
bool mDatabase::morphoInitialization(bool start)
{
    if(start)
    {
        morpho_db_path=morpho_metadata_path="";
        listMorpho.clear();mMorphoPointer.clear();
    }
    else
    {
        this->morpho_metadata_path=this->getMorpho_metadata_path();
        //load from basic morpho.metadata
        QFile metadata_file(this->morpho_metadata_path);
        if(!this->morpho_metadata_path.isEmpty()&&metadata_file.exists())
        {
            this->listMorpho=getSomalist(this->morpho_metadata_path,true);
        }
        if(!this->listMorpho.size())
            qDebug()<<"Meta data of morpho list is empty.";
        //init morpho_db_path
        this->morpho_db_path=this->getMorpho_path();
        if(this->morpho_db_path.isEmpty()||!QDir(this->morpho_db_path).exists())
            qDebug()<<"Morphometry path can't find at database path";
    }
    return true;
}
bool mDatabase::addMorpho(MImageType qImageID,MSomaType qSomaID,QStringList infilelist)
{
    /*   1. check the validity of the input `mImageID`,`mSomaID` from `listMorpho`
         2. get `mMorphoPointer` and assign `morphoid`
         3. ask for managing files or create an apo and ano file at `get_morpho_path`
         4. write the filename to `mMorphoPointer`
         5. save `mMorphoPointer` to `get_morpho_metadata_path(mImageID,mSomaID)`
    */
    //1
    V3DLONG lmz=this->listMorpho.size();
    mSoma thisSoma;
    bool validated=false;
    for(V3DLONG i=0;i<lmz;i++)
    {
        mSoma temp=this->listMorpho.at(i);
        if(qImageID==temp.mImageID&&qSomaID==temp.mSomaID)
        {
            validated=true;
            thisSoma=temp;
            break;
        }
    }
    if(!validated)
        return false;
    //2
    QList<mMorphometry> newMorphoPointer; newMorphoPointer.clear();
    if(this->getMorphoPointer(qImageID,qSomaID))
    {
        mMorphometry newMorpho;
        newMorpho.mImageID=qImageID;
        newMorpho.mSomaID=qSomaID;
        if(!infilelist.size())
            infilelist.append(db_init_2stlayer_metadata[SOMA]+".apo");
        if(this->mMorphoPointer.size())
        {
            //assgin morphoid
            QList<MMorphoType> listmorphoID; listmorphoID.clear();
            for(V3DLONG i=0;i<this->mMorphoPointer.size();i++)
            {
                listmorphoID.append(this->mMorphoPointer.at(i).mMorphoID);
                newMorphoPointer.append(this->mMorphoPointer.at(i));
            }
            for(V3DLONG i=MinMorphometryID;i<MaxMorphometryID+1;i++)
                if(!listmorphoID.contains(i))
                {
                    newMorpho.mMorphoID=i;
                    break;
                }
            //3.managing files
            bool ok=false;
            for(V3DLONG i=0;i<infilelist.size();i++)
            {
                //4.upload to db path
                newMorpho.morphoFilelist.append(QFileInfo(infilelist.at(i)).fileName());
                QString toMorphopath=this->getMorpho_path(qImageID,qSomaID,newMorpho.mMorphoID);
                QDir dstdir;
                if(!QDir(toMorphopath).exists())
                    dstdir.mkpath(toMorphopath);
                if(QDir(toMorphopath).exists()
                        &&QFile(infilelist.at(i)).exists())
                {
                    ok=QFile::copy(infilelist.at(i),
                                toMorphopath+"/"+QFileInfo(infilelist.at(i)).fileName());
                    if(!ok)
                        return false;
                }
                else
                    return false;
            }
        }
        else
        {
            //from scratch
            newMorpho.mMorphoID=MinMorphometryID;
            newMorpho.morphoType="Soma";
            newMorpho.comments="initialization";
            newMorpho.morphoFilelist<<(newMorpho.getName()+".apo");
            //4.upload to db path
            QString toMorphopath=this->getMorpho_path(qImageID,qSomaID,newMorpho.mMorphoID);
            QDir dstdir;bool ok=false;
            if(!QDir(toMorphopath).exists())
                dstdir.mkpath(toMorphopath);
            if(QDir(toMorphopath).exists())
            {
                QList <CellAPO> toapolist; toapolist.clear();
                toapolist.append(mSoma2Apo(thisSoma));
                ok=writeAPO_file(toMorphopath+"/"+QFileInfo(infilelist.at(0)).fileName(),toapolist);
                if(!ok)
                    return false;
            }
            else
                return false;
        }
        newMorphoPointer.append(newMorpho);
        if(newMorphoPointer.size()!=this->mMorphoPointer.size()+1)
            return false;
        //5
        this->mMorphoPointer.clear();
        for(V3DLONG i=0;i<newMorphoPointer.size();i++)
            this->mMorphoPointer.append(newMorphoPointer.at(i));
        QDir dstdir;bool ok=false;
        if(!QDir(QFileInfo(this->getMorpho_metadata_path(qImageID,qSomaID)).absolutePath()).exists())
            dstdir.mkpath(QFileInfo(this->getMorpho_metadata_path(qImageID,qSomaID)).absolutePath());
        if(!QDir(QFileInfo(this->getMorpho_metadata_path(qImageID,qSomaID)).absolutePath()).exists())
            ok=writeMorpholistToFile(this->getMorpho_metadata_path(qImageID,qSomaID),this->mMorphoPointer);
        if(ok)
            return ok;
        else
            dstdir.rmpath(QFileInfo(this->getMorpho_metadata_path(qImageID,qSomaID)).absolutePath());
    }
    return false;
}
bool mDatabase::updateMorpho(MImageType qImageID,MSomaType qSomaID,MMorphoType qMorphoID,mMorphometry inMorpho)
{
    /*come here later*/
    V3DLONG lmz=this->listMorpho.size();
    bool validated=false;
    for(V3DLONG i=0;i<lmz;i++)
    {
        mSoma temp=this->listMorpho.at(i);
        if(qImageID==temp.mImageID&&qSomaID==temp.mSomaID)
        {
            validated=true;
            break;
        }
    }
    if(!validated)
        return false;
    validated=false;
    QList<mMorphometry> newMorphoPointer; newMorphoPointer.clear();
    mMorphometry dstMorpho;
    if(this->getMorphoPointer(qImageID,qSomaID))
    {
        for(V3DLONG i=0;i<this->mMorphoPointer.size();i++)
        {
            mMorphometry temp=this->mMorphoPointer.at(i);
            if(temp.mMorphoID==qMorphoID)
            {
                validated=true;
                dstMorpho=temp;
                continue;
            }
            newMorphoPointer.append(temp);
        }
    }
    newMorphoPointer.append(inMorpho);
    if(newMorphoPointer.size()!=this->mMorphoPointer.size())
        return false;
    this->mMorphoPointer.clear();
    for(V3DLONG i=0;i<newMorphoPointer.size();i++)
        this->mMorphoPointer.append(newMorphoPointer.at(i));
    QDir dstdir;bool ok=false;
    if(!QDir(QFileInfo(this->getMorpho_metadata_path(qImageID,qSomaID)).absolutePath()).exists())
        dstdir.mkpath(QFileInfo(this->getMorpho_metadata_path(qImageID,qSomaID)).absolutePath());
    if(!QDir(QFileInfo(this->getMorpho_metadata_path(qImageID,qSomaID)).absolutePath()).exists())
        ok=writeMorpholistToFile(this->getMorpho_metadata_path(qImageID,qSomaID),this->mMorphoPointer);
    if(ok)
        return ok;
    else
        dstdir.rmpath(QFileInfo(this->getMorpho_metadata_path(qImageID,qSomaID)).absolutePath());
    return true;
}
bool mDatabase::deleteMorpho(MImageType qImageID)
{
    //remove all neurons of one brain
    /* 1. check the validity of the input `mImageID`
     * 2. delete metadata
     * 3. delete db
    */
    //1
    QList<mSoma> newlistMorpho; newlistMorpho.clear();
    V3DLONG lmz=this->listMorpho.size();
    bool validated=false;
    for(V3DLONG i=0;i<lmz;i++)
    {
        mSoma temp=this->listMorpho.at(i);
        if(qImageID==temp.mImageID)
        {
            validated=true;
            continue;
        }
        newlistMorpho.append(temp);
    }
    if(!validated)
        return false;
    this->listMorpho.clear();
    for(V3DLONG i=0;i<newlistMorpho.size();i++)
        this->listMorpho.append(newlistMorpho.at(i));
    //update to file
    validated=writeSomalistToFile(this->getMorpho_metadata_path(),this->listMorpho);
    if(!validated)
        return false;
    //3
    //delete metadata path
    if(!deletePath(this->getMorpho_metadata_path(qImageID)))
        return false;
    //delete db path
    if(!deletePath(this->getMorpho_path(qImageID)))
        return false;
    return true;
}
bool mDatabase::deleteMorpho(MImageType qImageID,MSomaType qSomaID)
{
    //remove all morphometries of one neuron
    /* 1. check the validity of the input `mImageID`,`mSomaID`
     * 2. delete metadata, update listMorpho and save to file.
     * 3. delete morpho database path
    */
    //1
    QList<mSoma> newlistMorpho; newlistMorpho.clear();
    V3DLONG lmz=this->listMorpho.size();
    bool validated=false;
    for(V3DLONG i=0;i<lmz;i++)
    {
        mSoma temp=this->listMorpho.at(i);
        if(qImageID==temp.mImageID&&qSomaID==temp.mSomaID)
        {
            validated=true;
            continue;
        }
        newlistMorpho.append(temp);
    }
    if(!validated||newlistMorpho.size()!=this->listMorpho.size()-1)
        return false;
    //2
    this->listMorpho.clear();
    for(V3DLONG i=0;i<newlistMorpho.size();i++)
        this->listMorpho.append(newlistMorpho.at(i));
    if(newlistMorpho.size()!=this->listMorpho.size())
        return false;
    //update to file
    validated=writeSomalistToFile(this->getMorpho_metadata_path(),this->listMorpho);
    if(!validated)
        return false;
    //3
    //delete metadata path
    if(!deletePath(QFileInfo(this->getMorpho_metadata_path(qImageID,qSomaID)).absolutePath()))
        return false;
    //delete db path
    if(!deletePath(this->getMorpho_path(qImageID,qSomaID)))
        return false;
    return true;
}
bool mDatabase::deleteMorpho(MImageType qImageID,MSomaType qSomaID,MMorphoType qMorphoID)
{
    //remove all the files of one morphometry
    /*1. check the validity of the input `mImageID`,`mSomaID`, `qMorphoID`
     *2. revise mMorphoPointer and save to metadata
     *3. delete morpho db
    */
    //1
    V3DLONG lmz=this->listMorpho.size();
    bool validated=false;
    for(V3DLONG i=0;i<lmz;i++)
    {
        mSoma temp=this->listMorpho.at(i);
        if(qImageID==temp.mImageID&&qSomaID==temp.mSomaID)
        {
            validated=true;
            break;
        }
    }
    if(!validated)
        return false;
    validated=false;
    QList<mMorphometry> newMorphoPointer; newMorphoPointer.clear();
    mMorphometry dstMorpho;
    if(this->getMorphoPointer(qImageID,qSomaID))
    {
        for(V3DLONG i=0;i<this->mMorphoPointer.size();i++)
        {
            mMorphometry temp=this->mMorphoPointer.at(i);
            if(temp.mMorphoID==qMorphoID)
            {
                validated=true;
                dstMorpho=temp;
                continue;
            }
            newMorphoPointer.append(temp);
        }
    }
    if(newMorphoPointer.size()!=this->mMorphoPointer.size()-1)
        return false;
    this->mMorphoPointer.clear();
    for(V3DLONG i=0;i<newMorphoPointer.size();i++)
        this->mMorphoPointer.append(newMorphoPointer.at(i));
    QDir dstdir;bool ok=false;
    if(!QDir(QFileInfo(this->getMorpho_metadata_path(qImageID,qSomaID)).absolutePath()).exists())
        dstdir.mkpath(QFileInfo(this->getMorpho_metadata_path(qImageID,qSomaID)).absolutePath());
    if(!QDir(QFileInfo(this->getMorpho_metadata_path(qImageID,qSomaID)).absolutePath()).exists())
        ok=writeMorpholistToFile(this->getMorpho_metadata_path(qImageID,qSomaID),this->mMorphoPointer);
    if(ok)
        return ok;
    else
        dstdir.rmpath(QFileInfo(this->getMorpho_metadata_path(qImageID,qSomaID)).absolutePath());
    //rm db path
    if(!deletePath(this->getMorpho_path(qImageID,qSomaID,qMorphoID)))
        return false;
    return true;
}
bool mDatabase::deleteMorpho(MImageType qImageID,MSomaType qSomaID,MMorphoType qMorphoID,const QString &deleteFile)
{
    /*1. check the validity of the input `mImageID`,`mSomaID`, `qMorphoID` and `deleteFile`
     *2. revise mMorphoPointer and save to metadata
     *3. delete morpho db
    */
    //1
    V3DLONG lmz=this->listMorpho.size();
    bool validated=false;
    for(V3DLONG i=0;i<lmz;i++)
    {
        mSoma temp=this->listMorpho.at(i);
        if(qImageID==temp.mImageID&&qSomaID==temp.mSomaID)
        {
            validated=true;
            break;
        }
    }
    if(!validated)
        return false;
    validated=false;
    QList<mMorphometry> newMorphoPointer; newMorphoPointer.clear();
    mMorphometry dstMorpho;
    if(this->getMorphoPointer(qImageID,qSomaID))
    {
        for(V3DLONG i=0;i<this->mMorphoPointer.size();i++)
        {
            mMorphometry temp=this->mMorphoPointer.at(i);
            if(temp.mMorphoID==qMorphoID)
            {
                for(int j=0;j<temp.morphoFilelist.size();j++)
                {
                    if(QString::compare(temp.morphoFilelist.at(j),deleteFile))
                    {
                        validated=true;
                        dstMorpho=temp;
                        break;
                    }
                }
                if(!validated)
                    return false;
                else
                    continue;
            }
            newMorphoPointer.append(temp);
        }
    }
    QStringList newFilelist;newFilelist.clear();
    for(int j=0;j<dstMorpho.morphoFilelist.size();j++)
    {
        if(QString::compare(dstMorpho.morphoFilelist.at(j),deleteFile))
            continue;
        newFilelist.append(dstMorpho.morphoFilelist.at(j));
    }
    if(newFilelist.size()!=dstMorpho.morphoFilelist.size()-1)
        return false;
    dstMorpho.morphoFilelist.clear();
    for(int j=0;j<newFilelist.size();j++)
        dstMorpho.morphoFilelist.append(newFilelist.at(j));
    if(newFilelist.size()!=dstMorpho.morphoFilelist.size())
        return false;
    //2        
    newMorphoPointer.append(dstMorpho);
    if(newMorphoPointer.size()!=this->mMorphoPointer.size())
        return false;
    this->mMorphoPointer.clear();
    for(V3DLONG i=0;i<newMorphoPointer.size();i++)
        this->mMorphoPointer.append(newMorphoPointer.at(i));
    QDir dstdir;bool ok=false;
    if(!QDir(QFileInfo(this->getMorpho_metadata_path(qImageID,qSomaID)).absolutePath()).exists())
        dstdir.mkpath(QFileInfo(this->getMorpho_metadata_path(qImageID,qSomaID)).absolutePath());
    if(!QDir(QFileInfo(this->getMorpho_metadata_path(qImageID,qSomaID)).absolutePath()).exists())
        ok=writeMorpholistToFile(this->getMorpho_metadata_path(qImageID,qSomaID),this->mMorphoPointer);
    if(ok)
        return ok;
    else
        dstdir.rmpath(QFileInfo(this->getMorpho_metadata_path(qImageID,qSomaID)).absolutePath());
    //rm db path
    QString rmFilepath=this->getMorpho_path(qImageID,qSomaID,qMorphoID,deleteFile);
    if(QFile(rmFilepath).exists())
        QFile::remove(rmFilepath);
    if(QFile(rmFilepath).exists())
        return false;
    return true;
}
QString mDatabase::getMorpho_path()
{
    QString outpath=""; QDir dbDir(dbpath);
    if(dbpath.isEmpty()||!dbDir.exists())
        return outpath;
    outpath=dbpath+"/"+db_init_1stlayer[MORPHOMETRYDB];
    QDir outdir(outpath);
    if(outpath.isEmpty()||!outdir.exists())
    {
        qDebug()<<"Requested morphometry data path is not existed.";
    }
    return outpath;
}
QString mDatabase::getMorpho_path(MImageType qImageID)
{
    /*check the validity of query image id, from listSomata*/
    QString outpath="";
    for(V3DLONG i=0;i<this->listSomata.size();i++)
    {
        if(this->listSomata.at(i).mImageID==qImageID)
        {
            outpath=this->getMorpho_path()+"/"+QString::number(qImageID);
            break;
        }
    }
    QDir outdir(outpath);
    if(outpath.isEmpty()||!outdir.exists())
    {
        qDebug()<<"Requested morphometry data path of Image "
               <<QString::number(qImageID)<<" is not existed.";
    }
    return outpath;
}
QString mDatabase::getMorpho_path(MImageType qImageID,MSomaType qSomaID)
{
    /*check the validity of query image id and query soma id from listMorpho*/
    QString outpath="";
    outpath=this->getMorpho_path()+"/"+QString::number(qImageID);
    if(outpath.isEmpty()
            ||!QDir(outpath).exists())
    {
        outpath="";
        return outpath;
    }
    V3DLONG lmz=this->listMorpho.size();
    for(V3DLONG i=0;i<lmz;i++)
    {
        mSoma temp=this->listMorpho.at(i);
        if(qImageID==temp.mImageID&&qSomaID==temp.mSomaID)
        {
            outpath+=("/"+QString::number(qSomaID));
            break;
        }
    }
    QDir outdir(outpath);
    if(outpath.isEmpty()||!outdir.exists())
    {
        qDebug()<<"Requested morphometry data path of Image "
               <<QString::number(qImageID)<<"_"<<QString::number(qSomaID)
              <<" is not existed.";
    }
    return outpath;
}
QString mDatabase::getMorpho_path(MImageType qImageID,MSomaType qSomaID,MMorphoType qMorphoID)
{
    /*call getMorpho_path(MImageType qImageID,MSomaType qSomaID)
     * and query morphometry id*/
    QString outpath="";
    outpath=this->getMorpho_path(qImageID,qSomaID);
    if(outpath.isEmpty()||!QDir(outpath).exists()) {outpath="";return outpath;}
    //get the metadata of qMorphoID
    QString morpho_metadata_temp_path=this->getMorpho_metadata_path(qImageID,qSomaID);
    if(morpho_metadata_temp_path.isEmpty()||!QDir(morpho_metadata_temp_path).exists()) {outpath="";return outpath;}
    //read morpho list and init mMorphoPointer
    bool ok=this->getMorphoPointer(qImageID,qSomaID);
    if(!ok)
        return outpath;
    for(V3DLONG j=0;j<this->mMorphoPointer.size();j++)
    {
        mMorphometry temp=this->mMorphoPointer.at(j);
        if(temp.mImageID==qImageID&&temp.mSomaID==qSomaID
                &&temp.mMorphoID==qMorphoID)
        {
            outpath+=("/"+QString::number(qMorphoID));
            break;
        }
    }
    QDir outdir(outpath);
    if(outpath.isEmpty()||!outdir.exists())
    {
        qDebug()<<"Requested morphometry data path of Image "
               <<QString::number(qImageID)<<"_"<<QString::number(qSomaID)<<"_"<<QString::number(qMorphoID)
              <<" is not existed.";
    }
    return outpath;
}
QString mDatabase::getMorpho_path(MImageType qImageID,MSomaType qSomaID,MMorphoType qMorphoID,const QString &qfilename)
{
    QString outpath="";
    outpath=this->getMorpho_path(qImageID,qSomaID,qMorphoID);
    if(outpath.isEmpty()||!QDir(outpath).exists()) {outpath="";return outpath;}
    outpath+=("/"+qfilename);
    return outpath;
}
QString mDatabase::getMorpho_metadata_path()
{
    QString outpath="";QDir dbDir(dbpath);
    if(dbpath.isEmpty()||!dbDir.exists())
        return outpath;
    outpath=dbpath+"/"+db_init_1stlayer[METADATA]+"/"+db_init_2stlayer_metadata[MORPHOMETRY]+"/"+
            db_init_3stlayer_metadata[BASIC]+"/"+db_init_2stlayer_metadata[MORPHOMETRY]+".metadata";
    QFile metaFile(outpath);
    if(!metaFile.exists())
    {
        qDebug()<<"Requested morpho metadata is not existed.";
    }
    return outpath;
}
QString mDatabase::getMorpho_metadata_path(MImageType qImageID)
{
    QString outpath=""; QDir dbDir(dbpath);
    if(dbpath.isEmpty()||!dbDir.exists()
            ||qImageID<MinImageID||qImageID>MaxImageID
            )
        return outpath;
    V3DLONG lmz=this->listMorpho.size();
    for(V3DLONG i=0;i<lmz;i++)
    {
        mSoma temp=this->listMorpho.at(i);
        if(qImageID==temp.mImageID)
        {
            outpath= dbpath+"/"+db_init_1stlayer[METADATA]+"/"+db_init_2stlayer_metadata[MORPHOMETRY]+"/"+
                    db_init_3stlayer_metadata[DETAIL]+"/"+QString::number(qImageID);
            break;
        }
    }
    QFile metaFile(outpath);
    if(!metaFile.exists())
    {
        qDebug()<<"Requested morpho metadata of "
               <<QString::number(qImageID)<<" is not existed.";
    }
    return outpath;
}
QString mDatabase::getMorpho_metadata_path(MImageType qImageID,MSomaType qSomaID)
{
    QString outpath=""; QDir dbDir(dbpath);
    if(dbpath.isEmpty()||!dbDir.exists()
            ||qImageID<MinImageID||qImageID>MaxImageID
            ||qSomaID<MinSomaID||qSomaID>MaxSomaID)
        return outpath;
    V3DLONG lmz=this->listMorpho.size();
    for(V3DLONG i=0;i<lmz;i++)
    {
        mSoma temp=this->listMorpho.at(i);
        if(qImageID==temp.mImageID&&qSomaID==temp.mSomaID)
        {
            outpath= dbpath+"/"+db_init_1stlayer[METADATA]+"/"+db_init_2stlayer_metadata[MORPHOMETRY]+"/"+
                    db_init_3stlayer_metadata[DETAIL]+"/"+QString::number(qImageID)+"/"
                    +QString::number(qSomaID)+"/"+db_init_2stlayer_metadata[MORPHOMETRY]+".metadata";
            break;
        }
    }
    QFile metaFile(outpath);
    if(!metaFile.exists())
    {
        qDebug()<<"Requested morpho metadata of "
               <<QString::number(qImageID)<<"_"<<QString::number(qSomaID)
              <<" is not existed.";
    }
    return outpath;
}
QString mDatabase::getMorpho_metadata_path(const QString &insampleID, unsigned long qSomaID)
{
    QString outpath="";
    V3DLONG lsz=this->listSomata.size();
    for(V3DLONG i=0;i<lsz;i++)
    {
        mImage temp=this->listSomata.at(i);
        if(QString::compare(insampleID,temp.sampleID))
            return this->getMorpho_metadata_path(temp.mImageID,qSomaID);
    }
    return outpath;
}
bool mDatabase::getMorphoPointer(MImageType qImageID,MSomaType qSomaID)
{
    QString meta_path=this->getMorpho_metadata_path(qImageID,qSomaID);
    if(meta_path.isEmpty())
        return false;
    this->mMorphoPointer=getMorpholist(meta_path);
    if(!this->mMorphoPointer.size())
        return false;
    return true;
}
bool mDatabase::getMorphoPointer(const QString &insampleID,MSomaType qSomaID)
{
    V3DLONG lsz=this->listSomata.size();
    for(V3DLONG i=0;i<lsz;i++)
    {
        mImage temp=this->listSomata.at(i);
        if(QString::compare(insampleID,temp.sampleID))
            return this->getMorphoPointer(temp.mImageID,qSomaID);
    }
    return false;
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
//        out.append(this->getName());
        out.append(this->name);
        return out;
    }
    out.append(QString::number(this->mImageID));
    out.append(QString::number(this->mSomaID));
//    out.append(this->getName());
    out.append(this->name);
    out.append(this->getcellName());
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
QStringList mMorphometry::getDataNumber(bool basic)
{
    QStringList out;out.clear();
    out.append(QString::number(this->mImageID));
    out.append(QString::number(this->mSomaID));
    out.append(QString::number(this->mMorphoID));
    out.append(this->getName());
    if(basic)
        return out;
    out.append(this->morphoType);
    out.append(this->tag);
    out.append(this->comments);
    QString outfiles=(this->morphoFilelist.size())?this->morphoFilelist.at(0):("");
    for(int i=1; i<this->morphoFilelist.size();i++)
        outfiles+=(";"+this->morphoFilelist.at(i));
    out.append(outfiles);
    return out;
}
mMorphometry createMorphoFromQSL(QStringList inlist, bool basic)
{
    mMorphometry out;
    out.mImageID=inlist.at(0).toLong();
    out.mSomaID=inlist.at(1).toULong();
    out.mMorphoID=inlist.at(2).toUInt();
    out.name=inlist.at(3);
    if(basic)
        return out;
    out.morphoType=inlist.at(4);
    out.tag=inlist.at(5);
    out.comments=inlist.at(6);
    QString outfiles=inlist.at(7);
    out.morphoFilelist=outfiles.simplified().split(";");
    return out;
}
QList<mMorphometry> getMorpholist(const QString& inpath,bool basic)
{
    QList<mMorphometry> outMorpholist;outMorpholist.clear();
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
                mMorphometry tempi=createMorphoFromQSL(qsl,basic);
                outMorpholist.append(tempi);
            }
        }
    }
    return outMorpholist;
}
bool writeMorpholistToFile(const QString& topath,QList<mMorphometry> &inMorpholist,bool basic)
{
    if (topath.isEmpty()||inMorpholist.size()==0)
        return false;
    QFile scanconffile(topath);
    //get title
    mMorphometry temp;QString confTitle="#";
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
        for(V3DLONG i=0;i<inMorpholist.size();i++)
        {
            mMorphometry tempi=inMorpholist.at(i);
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
mSoma createmSomaFromQSL(QStringList inlist,bool basic)
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
    out.cellName=inlist.at(3);
    out.position.mx=inlist.at(4).toFloat();
    out.position.my=inlist.at(5).toFloat();
    out.position.mz=inlist.at(6).toFloat();
    out.radius=inlist.at(7).toFloat();
    out.color.r=inlist.at(8).toUInt();
    out.color.g=inlist.at(9).toUInt();
    out.color.b=inlist.at(10).toUInt();
    out.color.a=inlist.at(11).toUInt();
    out.intensity=inlist.at(12).toFloat();
    out.volume=inlist.at(13).toFloat();
    out.comments=inlist.at(14);
    out.region=inlist.at(15);
    out.cellType=inlist.at(16);
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
                mSoma tempi=createmSomaFromQSL(qsl,basic);
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
    outmSoma.name=inapo.name;
    outmSoma.comments=inapo.comment;
    outmSoma.color=inapo.color;
    outmSoma.intensity=inapo.intensity;
    outmSoma.volume=inapo.volsize;
    outmSoma.position.mx=inapo.x;
    outmSoma.position.my=inapo.y;
    outmSoma.position.mz=inapo.z;
    return outmSoma;
}
CellAPO mSoma2Apo(mSoma insoma)
{
    CellAPO outapo;
    outapo.name=insoma.name;
    outapo.comment=insoma.comments;
    outapo.color=insoma.color;
    outapo.intensity=insoma.intensity;
    outapo.volsize=insoma.volume;
    outapo.x=insoma.position.mx;
    outapo.y=insoma.position.my;
    outapo.z=insoma.position.mz;
    return outapo;
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
    mImage out;
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
bool deletePath(const QString &dpath)
{
    if(dpath.isEmpty()||!QDir(dpath).exists())
        return false;
    QDir dir(dpath);
    return dir.rmpath(dpath);

}
bool deldeteFile(const QString &dfile)
{
    if(dfile.isEmpty()||!QFile(dfile).exists())
        return false;
    QDir dir(QFileInfo(dfile).absolutePath());
    return dir.remove(QFileInfo(dfile).fileName());
}
