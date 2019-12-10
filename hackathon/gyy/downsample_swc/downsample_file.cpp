
#include <QFileDialog>
#include <iostream>
#include <QDir>
#include <QDebug>
#include "gui.cpp"
#include <basic_surf_objs.h>
#include <windows.h>
#include <direct.h>
#include <io.h>
using namespace std;
void find_all_files(QString path, vector<QString> &files);

void downSample_Files(QWidget *parent)
{
    DownsampleDialog dialog(parent);

    if(dialog.exec()!=QDialog::Accepted)return;
    dialog.update();

    double down =(((double) dialog.res_output->value())/dialog.res_input->value());
    cout<<"Downsample: down = "<< down <<endl;
    if(down < 1)
    {
        v3d_msg("This is a Downsample plugin !");
        return;
    }
    if(dialog.file!="" && dialog.files == "")
    {

        if(dialog.file.endsWith(".eswc")||dialog.file.endsWith(".swc"))
        {
            qDebug()<<__LINE__<<" : "<<dialog.fileSave.toStdString().c_str();
            DownsampleDialog :: downsampleEswc(dialog.file, dialog.fileSave, down);
        }
        else if(dialog.file.endsWith(".apo"))
        {
            DownsampleDialog :: downsampleApo(dialog.file, dialog.fileSave, down);
        }
    }
    //input a folder
    else if(dialog.file == "" && dialog.files != "")
    {
        vector<QString> files_downsample;
        find_all_files(dialog.files, files_downsample);
        cout<<"dir size: "<<files_downsample.size()<<endl;
        for(V3DLONG i = 0; i < files_downsample.size(); i ++)
        {
            //QFileInfo file1(files_downsample.at(i));
            cout<<"file_downsample :"<<files_downsample.at(i).toUtf8().data()<<endl;
            QString file1 = dialog.files + "\\" + files_downsample.at(i);
            qDebug()<<"file1 : "<<file1.toStdString().c_str();
//               qDebug()<<"savefile : "<<dialog.folderSave.toStdString().c_str();
            QString file2 = dialog.folderSave + "\\" + files_downsample.at(i);
            qDebug()<<"file2 : "<<file2.toStdString().c_str();
            QString fileSaveName = DownsampleDialog :: nameSaveFile(file2);
            qDebug()<<"savefile2 : "<<fileSaveName.toStdString().c_str();
            if(file1.endsWith(".eswc")||file1.endsWith(".swc") )
            {
                DownsampleDialog :: downsampleEswc(file1, fileSaveName, down);
            }
            else if(file1.endsWith(".apo"))
            {
                DownsampleDialog :: downsampleApo(file1, fileSaveName, down);
            }
            else
                continue;
         }
     }
}

void find_all_files(QString path, vector<QString> &files)
{
    const size_t file_type = 3;
    QString support_format[file_type] = {
        ".swc", ".eswc", ".apo"
    };
    string dir = path.toStdString();
    qDebug()<<__LINE__<<" : "<<dir.c_str();
    const char* lppath = dir.c_str();
    qDebug()<<__LINE__<<" : "<<lppath;
    char szFind[MAX_PATH];
    WIN32_FIND_DATAA FindFileData;
    strcpy(szFind, lppath);
    strcat(szFind, "/*.*");
    HANDLE Hfind = FindFirstFileA(szFind, &FindFileData);//创建搜索句柄

    if(INVALID_HANDLE_VALUE == Hfind)
    {
        qDebug()<<"path is error !";
    }
    do{
        if(!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))//不是目录
        {
            QString lower_name(FindFileData.cFileName);//获取文件名
            for(V3DLONG i =0; i < 3; i ++)
            {
                if(support_format[i].toStdString() == lower_name.toStdString().substr(lower_name.length()-support_format[i].length()))
                {
                    files.push_back(FindFileData.cFileName);
                    break;
                }
            }
        }
        else
            continue;

    }while (FindNextFileA(Hfind, &FindFileData));//当下一个文件目录不存在时退出循环
    FindClose(Hfind);
    qDebug()<<"files size : "<<files.size();
    qDebug()<<"finish find all files";
}
