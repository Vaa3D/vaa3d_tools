#include "morphohub_dbms_basic.h"
/*For the initialization of database*/
bool mDatabase::initialization()
{
    bool ok=loadDB(this->dbpath);
    if(ok)
    {
        //1. init of image list
        if(!this->img_metadata_path.isEmpty())
        {
            QDir img_metadata_Dir(this->img_metadata_path);
            if(img_metadata_Dir.exists())
            {
                //get image metadata file.
                this->img_metadata_path=getImg_metadata_path();
                //load from this file.
                this->listImages=getImagelist(this->img_metadata_path);
            }
        }
        if(!this->listImages.size())
            QMessageBox::warning(0,"Image dataset empth",QObject::tr("Without image datasets at %1 !").arg(this->img_metadata_path));
        //2.
    }
    else
        return false;
    return true;
}
bool mDatabase::createDB(const QString &inpath)
{
    if(inpath.isEmpty())
        return false;
    QDir inDir(inpath);
    if(!inDir.exists())
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
    bool ok=createDB(inpath);
    return ok;
}
QString mDatabase::getImg_metadata_path()
{
    QString outpath;  outpath="";
    if(dbpath.isEmpty())
        return outpath;
    QDir dbDir(dbpath);
    if(!dbDir.exists())
        return outpath;
    outpath=dbpath+"/"+db_init_1stlayer[METADATA]+"/"+db_init_2stlayer_metadata[IMAGE]+"/"+
            db_init_3stlayer_metadata[BASIC]+"/"+db_init_2stlayer_metadata[IMAGE]+".metadata";
    return outpath;
}
QStringList mImage::getDataNumber()
{
    QStringList out;
    out=QString::number(this->mImageID)+","
            +this->getName()+","
            +this->sampleID+","
            +this->sourceID+","
            +this->mFormatList[this->mFormatID]+","
            +this->mObjectList[this->mObjectID]+","
            +QString::number(this->mIS.sx)+","
            +QString::number(this->mIS.sy)+","
            +QString::number(this->mIS.sz)+","
            +QString::number(this->mVR.rx)+","
            +QString::number(this->mVR.rx)+","
            +QString::number(this->mVR.rx)+","
            +this->comments;
    return out;
}
bool mDatabase::creatNewImage(const QString & insampleID)
{
    /*get the max ImageID; assign imageID*/
    V3DLONG liz=this->listImages.size();
    mImage newImage;
    newImage.mImageID=(liz>0)?10000:(liz+10000);
    for(V3DLONG i=0;i<liz;i++)
    {
        mImage temp=this->listImages.at(i);
        if(temp.mImageID>newImage.mImageID)
            newImage.mImageID=temp.mImageID+1;
    }
    newImage.init(insampleID);
    this->listImages.append(newImage);
    if(this->listImages.size()==liz+1)
        return true;
    return false;
}
bool mDatabase::deleteImage(unsigned long removeID)
{
    QList<mImage> tmplistImages;tmplistImages.clear();
    V3DLONG liz=this->listImages.size();
    for(V3DLONG i=0;i<liz;i++)
    {
        mImage temp=this->listImages.at(i);
        tmplistImages.append(temp);
    }
    this->listImages.clear();
    for(V3DLONG i=0;i<liz;i++)
    {
        mImage temp=this->tmplistImages.at(i);
        if(temp.mImageID==removeID)
            continue;
        this->listImages.append(temp);
    }
    if(liz==this->listImages.size()-1)
        return true;
    return false;
}
bool mDatabase::deleteImageBySampleID(const QString &insampleID)
{
    QList<mImage> tmplistImages;tmplistImages.clear();
    V3DLONG liz=this->listImages.size();
    for(V3DLONG i=0;i<liz;i++)
    {
        mImage temp=this->listImages.at(i);
        tmplistImages.append(temp);
    }
    this->listImages.clear();
    for(V3DLONG i=0;i<liz;i++)
    {
        mImage temp=this->tmplistImages.at(i);
        if(QString::compare(insampleID,temp.sampleID))
            continue;
        this->listImages.append(temp);
    }
    if(liz==this->listImages.size()-1)
        return true;
    return false;
}
mImage createImageFromQSL(QStringList inlist)
{
    mImage out;
    if(inlist.size()!=out.dataNumberTitle.size())
        return out;
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
QList<mImage> getImagelist(const QString& inpath)
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
                mImage tempi=createImageFromQSL(qsl);
                outlist.append(tempi);
            }
        }
    }
    //3.write to Qlist
    return outlist;
}
bool writeImagelistToFile(const QString &topath, QList<mImage> &inlist)
{
    if (topath.isEmpty()||inlist.size()==0)
        return false;
    QFile scanconffile(topath);
    //get title
    mImage temp;QString confTitle="#";
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
            tempi.getDataNumber();
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
