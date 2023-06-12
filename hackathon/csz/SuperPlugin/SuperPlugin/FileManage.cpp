#include "FileManage.h"

FileManage *FileManage::_instance = nullptr;
std::mutex FileManage::mutex_ = std::mutex();

void FileManage::CreateFileInfomation(QString FileName, QVector<float> Info, bool AllowUi)
{
    if(Swc2BBmap.find(FileName)!=Swc2BBmap.end()){
        if(AllowUi){
            v3d_msg("The file you try to create is already exist!");
        }else{
            qDebug()<<"The file you try to create is already exist!";
        }
        return;
    }

    Swc2BBmap.insert(FileName,Info);
}

void FileManage::RemoveFileInfomation(QString FileName, bool AllowUi)
{
    if(Swc2BBmap.find(FileName)==Swc2BBmap.end()){
        if(AllowUi){
            v3d_msg("The file you try to remove doesnt exist!");
        }else{
            qDebug()<<"The file you try to remove doesnt exist!";
        }
        return;
    }
    Swc2BBmap.remove(FileName);
}

void FileManage::UpdateFileInfomation(QString FileName, QVector<float> Info, bool AllowUi)
{
    if(Swc2BBmap.find(FileName)==Swc2BBmap.end()){
        if(AllowUi){
            v3d_msg("The file you try to update doesnt exist!");
        }else{
            qDebug()<<"The file you try to update doesnt exist!";
        }
        return;
    }
    Swc2BBmap[FileName]=Info;
}

QVector<float> FileManage::getFileInformation(QString FileName, bool AllowUi)
{
    if(Swc2BBmap.find(FileName)==Swc2BBmap.end()){
        if(AllowUi){
            v3d_msg("The file you try to get doesnt exist!");
        }else{
            qDebug()<<"The file you try to get doesnt exist!";
        }
        return QVector<float>();
    }

    return Swc2BBmap[FileName];
}

QString FileManage::GenerateImageName(const QVector<long> &boundingbox,QString suffix,bool AllowUi)
{
    QString name=suffix;
    name=name+"_"+QString::number((boundingbox[3]+boundingbox[0])/2)+"_"+QString::number((boundingbox[4]+boundingbox[1])/2)+"_"+QString::number((boundingbox[5]+boundingbox[2])/2)+".tiff";
    if(Swc2BBmap.find(name)!=Swc2BBmap.end()){
        if(AllowUi){
            v3d_msg("The Image exists!");
        }else{
            qDebug()<<"The Image exists!";
        }
        return "";
    }
    return name;
}

QString FileManage::GenerateTempSwcName(QString prefix, const QVector<long> &boundingbox)
{
    QString name=prefix+"_"+QString::number((boundingbox[3]+boundingbox[0])/2)+"_"+QString::number((boundingbox[4]+boundingbox[1])/2)+"_"+QString::number((boundingbox[5]+boundingbox[2])/2)+".swc";
    return name;
}

QString FileManage::GenerateFinalSwcName(QString prefix)
{
    QString name=prefix+"_final.swc";
    return name;
}

QString FileManage::GetFuncResultName(QString name, QString func)
{

}


