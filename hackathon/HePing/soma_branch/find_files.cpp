#include<vector>
#include<fstream>
#include<iterator>
#include<string>
#include"v3d_message.h"
#include<v3d_interface.h>
#include<windows.h>
#include<direct.h>
#include<io.h>
using namespace std;

void find_files(QString path,vector<QString> &swcfiles,vector<QString> &v3drawfiles){
    const size_t FILE_TYPE=2;
    string support_format[FILE_TYPE]={
        ".v3draw",".swc"
    };
    QString dir=path;
    qDebug()<<dir;
//    vector<QString> swcfiles;
//    vector<QString> v3drawfiles;
    //std::string trim_path=TrimPath(dir);
    const char* lppath=dir.toStdString().c_str();
    char szFind[MAX_PATH];
    //int n=0;
    WIN32_FIND_DATAA FindFileData;
    strcpy_s(szFind,lppath);
    strcat_s(szFind,"/*.*");
    HANDLE Hfind=FindFirstFileA(szFind,&FindFileData);//创建搜索句柄

    if(INVALID_HANDLE_VALUE==Hfind){
        qDebug()<<"path is error!";
    }
    //qDebug()<<"*****111******";

    do{
        if(!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){//不是目录
            //bool is_v3draw,is_sw

            //qDebug()<<FindFileData.cFileName;
            string lower_name(FindFileData.cFileName);
//                    qDebug()<<lower_name.c_str();
//                    qDebug()<<"--------------";
            //const char* lower_char=lower_name.c_str();
            //qDebug()<<support_format[0].length();
            if(support_format[0]==lower_name.substr(lower_name.length()-support_format[0].length())){
                //v3draw图像
                v3drawfiles.push_back(FindFileData.cFileName);
            }
            else if(support_format[1]==lower_name.substr(lower_name.length()-support_format[1].length())){
                //swc
                swcfiles.push_back(FindFileData.cFileName);
            }
            else
                continue;
        }
        else{//是子目录
            qDebug()<<FindFileData.cFileName;
            qDebug()<<"---";
            continue;

        }
    }while(FindNextFileA(Hfind,&FindFileData));//当下一个文件目录不存在时退出循环
    FindClose(Hfind);//关闭搜索句柄

    qDebug()<<"finish find all files!";
}

bool find_corresponding_file(QString dir,QString &swcfile,QString &v3drawfile){
    QStringList ts=swcfile.split(QRegExp("[_|\\.]"));
    qDebug()<<ts;
    vector<V3DLONG> cord;
    bool flag=false;
    //一个swc文件的坐标获取
    qDebug()<<"swc coord:";
    for(int j=0;j<ts.size();j++){//获取id,x,y,z
        string tmpval=ts[j].toStdString();
        //qDebug()<<tmpval.c_str();
        V3DLONG num=0;
        //x,y,z
        for(int m=0;m<tmpval.length();m++){
            if(tmpval[m]>='0'&&tmpval[m]<='9'){
                num=num*10+(tmpval[m]-'0');
            }
            else{
                flag=true;
                break;
            }


        }
        if(flag==true){
            flag=false;
        }else{
            cord.push_back(num);
            qDebug()<<num;

        }

    }
    v3drawfile=QString::number(cord[1]*2)+"_"+QString::number(cord[2]*2)+"_"+QString::number(cord[3]*2)+".v3draw";
    QString v3drawfilepath=dir+"\\"+v3drawfile;
    if( _access(v3drawfilepath.toStdString().c_str(),0)!=-1){
        return true;
    }
    else
        return false;

}
