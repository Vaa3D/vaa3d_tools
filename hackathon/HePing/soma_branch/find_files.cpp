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

void find_all_swc(QString path,vector<QString> &swcfiles){
    const size_t FILE_TYPE=1;
    string support_format[FILE_TYPE]={
        ".swc"
    };
    string dir=path.toStdString();
    qDebug()<<dir.c_str();
    //std::string trim_path=TrimPath(dir);
    const char* lppath=dir.c_str();
    qDebug()<<lppath;
    char szFind[MAX_PATH];
    //int n=0;
    WIN32_FIND_DATAA FindFileData;
    strcpy(szFind,lppath);
//    qDebug()<<szFind<<"---";
    strcat(szFind,"/*.*");
//    qDebug()<<szFind<<"---";
    HANDLE Hfind=FindFirstFileA(szFind,&FindFileData);//创建搜索句柄

    if(INVALID_HANDLE_VALUE==Hfind){
        qDebug()<<"path is error!";
    }
    do{
        if(!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){//不是目录
            //bool is_v3draw,is_sw

//            qDebug()<<FindFileData.cFileName;
            string lower_name(FindFileData.cFileName);//获取文件名
            //const char* lower_char=lower_name.c_str();
            //qDebug()<<support_format[0].length();
            //是否v3draw
            if(support_format[0]==lower_name.substr(lower_name.length()-support_format[0].length())){
                //v3draw图像
                swcfiles.push_back(FindFileData.cFileName);
            }
            //是否swc文件

            else
                continue;
        }
        else{//是子目录
//            qDebug()<<FindFileData.cFileName;
//            qDebug()<<"---";
            continue;

        }
    }while(FindNextFileA(Hfind,&FindFileData));//当下一个文件目录不存在时退出循环
    FindClose(Hfind);//关闭搜索句柄

    qDebug()<<"finish find all swc files!";
}


//找到文件夹下所有的v3draw和swc
/*
 * v3draw :x_y_z
 * swc : id_x_num_y_num_z_num,x y z数值是在次级分辨率下的坐标
 */
void find_files(QString path,vector<QString> &swcfiles,vector<QString> &v3drawfiles){
    const size_t FILE_TYPE=2;
    string support_format[FILE_TYPE]={
        ".v3draw",".swc"
    };
    QString dir=path;
    qDebug()<<dir;
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
            string lower_name(FindFileData.cFileName);//获取文件名
            //const char* lower_char=lower_name.c_str();
            //qDebug()<<support_format[0].length();
            //是否v3draw
            if(support_format[0]==lower_name.substr(lower_name.length()-support_format[0].length())){
                //v3draw图像
                v3drawfiles.push_back(FindFileData.cFileName);
            }
            //是否swc文件
            else if(support_format[1]==lower_name.substr(lower_name.length()-support_format[1].length())){
                //swc
                swcfiles.push_back(FindFileData.cFileName);
            }
            else
                continue;
        }
        else{//是子目录
//            qDebug()<<FindFileData.cFileName;
//            qDebug()<<"---";
            continue;

        }
    }while(FindNextFileA(Hfind,&FindFileData));//当下一个文件目录不存在时退出循环
    FindClose(Hfind);//关闭搜索句柄

    qDebug()<<"finish find all files!";
}

//在指定目录下根据swc文件找到相应的v3draw图像
/*
 * swc: id_x_num_y_num_z_num.swc
 * v3draw: 2*x_2*y_z*z.v3draw 现在的函数的名字x y z坐标不是整数
*/
bool find_corresponding_file(QString dir,QString &swcfile,QString &v3drawfile){
    //QStringList ts=swcfile.split(QRegExp("[_|\\.]"));
    QStringList ts=swcfile.split("_");
    qDebug()<<ts;
    vector<double> cord;
    bool flag=false;
    //一个swc文件的坐标获取
    qDebug()<<"swc coord:";
    for(int j=0;j<ts.size();j++){//获取id,x,y,z
        string tmpval=ts[j].toStdString();
        //qDebug()<<tmpval.c_str();
        double num=0.0;
        //x,y,z
        int count=0;
        for(int m=0;m<tmpval.length();m++){

            if(tmpval[m]>='0'&&tmpval[m]<='9'){
                if(count!=0){
                    num+=(tmpval[m]-'0')*pow(10,-1*count);
                    count++;
                }
                else
                    num=num*10+(tmpval[m]-'0');
            }
            else if(tmpval[m]=='.'){
                count++;
                continue;
            }
            else{
                if(num==0){
                    flag=true;
                }

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
    if(cord.size()<3){
        qDebug()<<"the swc file name is not x_y_z fromat!";
        return false;}
    double x=cord[1]*2;
    double y=cord[2]*2;
    double z=cord[3]*2;
    V3DLONG x_down=floor(x);
    V3DLONG x_up=ceil(x);
    V3DLONG y_down=floor(y);
    V3DLONG y_up=ceil(y);
    V3DLONG z_down=floor(z);
    V3DLONG z_up=ceil(z);
    vector<QString> candite_v3drawfiles;
    QString candidte_v3draw=QString::number(x_down)+"_"+QString::number(y_down)+"_"+QString::number(z_down)+".v3draw";
    candite_v3drawfiles.push_back(candidte_v3draw);
    candidte_v3draw=QString::number(x_up)+"_"+QString::number(y_down)+"_"+QString::number(z_down)+".v3draw";
    candite_v3drawfiles.push_back(candidte_v3draw);
    candidte_v3draw=QString::number(x_down)+"_"+QString::number(y_up)+"_"+QString::number(z_down)+".v3draw";
    candite_v3drawfiles.push_back(candidte_v3draw);
    candidte_v3draw=QString::number(x_down)+"_"+QString::number(y_down)+"_"+QString::number(z_up)+".v3draw";
    candite_v3drawfiles.push_back(candidte_v3draw);
    candidte_v3draw=QString::number(x_up)+"_"+QString::number(y_up)+"_"+QString::number(z_down)+".v3draw";
    candite_v3drawfiles.push_back(candidte_v3draw);
    candidte_v3draw=QString::number(x_down)+"_"+QString::number(y_up)+"_"+QString::number(z_up)+".v3draw";
    candite_v3drawfiles.push_back(candidte_v3draw);
    candidte_v3draw=QString::number(x_up)+"_"+QString::number(y_down)+"_"+QString::number(z_up)+".v3draw";
    candite_v3drawfiles.push_back(candidte_v3draw);
    candidte_v3draw=QString::number(x_up)+"_"+QString::number(y_up)+"_"+QString::number(z_up)+".v3draw";
    candite_v3drawfiles.push_back(candidte_v3draw);
    //v3drawfile=QString::number(cord[0])+"_"+QString::number(cord[1])+"_"+QString::number(cord[2])+".v3draw";

    for(int ii=0;ii<8;ii++){
        v3drawfile=dir+"\\"+candite_v3drawfiles[ii];
        if(_access(v3drawfile.toStdString().c_str(),0)==-1)
            continue;
        else{
            qDebug()<<v3drawfile;
            return true;
           }
    }
    return false;
//    v3drawfile=candite_v3drawfiles[];
//    QString v3drawfilepath=dir+"\\"+v3drawfile;

//    if( _access(v3drawfilepath.toStdString().c_str(),0)!=-1){
//        return true;
//    }
//    else{

//        return false;}

}


