#ifndef FILEMANAGE_H
#define FILEMANAGE_H

#include <QMap>
#include <QVector>
#include <mutex>
#include "v3d_interface.h"
#include "SpLog.h"
#include "GlobalConfig.h"


class FileManage{
public:

    FileManage(const FileManage&) = delete;
    FileManage& operator=(const FileManage&) = delete;

    static FileManage* get_instance() {
        if (!_instance) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!_instance) {
                _instance = new FileManage();
            }
        }
        return _instance;
    }


    void CreateFileInfomation(QString FileName,QVector<float> Info,bool AllowUi=false);
    void RemoveFileInfomation(QString FileName,bool AllowUi=false);
    void UpdateFileInfomation(QString FileName,QVector<float> Info,bool AllowUi=false);


    QVector<float> getFileInformation(QString FileName,bool AllowUi=false);

    QString GenerateImageName(const QVector<long>& boundingbox,QString suffix="",bool AllowUi=false);
    QString GenerateTempSwcName(QString prefix,const QVector<long>& boundingbox);
    QString GenerateFinalSwcName(QString prefix);
    QString GetFuncResultName(QString name,QString func);
private:
    static FileManage* _instance;
    static std::mutex mutex_;
    FileManage() {}


    QMap<QString,QVector<float>> Swc2BBmap;

};

#endif // FILEMANAGE_H
