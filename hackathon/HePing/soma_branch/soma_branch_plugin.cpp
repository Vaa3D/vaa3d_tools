/* soma_branch_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-11-20 : by heping
 */
 
#include "v3d_message.h"
#include <vector>
#include "soma_branch_plugin.h"
#include "branch_count_soma.h"
#include<v3d_interface.h>
#include<eigen_3_3_4/Eigen/Dense>
#include<fstream>
#include<direct.h>
#include<io.h>
#include<windows.h>
using namespace std;
#define MAX_PATH 60
//extern template<class T> bool compute_marker_pca_hp(vector<T> markers,V3DLONG r,double &pc1,double &pc2,double &pc3);
Q_EXPORT_PLUGIN2(soma_branch, BranchCount);
 
QStringList BranchCount::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList BranchCount::funclist() const
{
	return QStringList()
        <<tr("branch_count")
        <<tr("mean_shift_soma")
		<<tr("help");
}

void BranchCount::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("menu1"))
	{
        //branch_count_menu(callback,parent);
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by heping, 2019-11-20"));
	}
}

bool BranchCount::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("branch_count"))
    {//读取swc获得soma位置和估计半径，再根据原图信息对soma位置进行mean-shift
        //根据新的soma位置计算branch
        //QString swcfile=(infiles.size()>=1)?infiles[0]:"";
        QString apofile=(outfiles.size()>=1)?outfiles[0]:"";

        if(_access(apofile.toStdString().c_str(),0)==-1)
            _mkdir(apofile.toStdString().c_str());
        string path=apofile.toStdString()+"//data.txt";
        string numpath=apofile.toStdString()+"//branchnum.txt";
        ofstream datafile(path,ios::app);
        ofstream branchNumfile(numpath,ios::app);
        if(datafile.fail()||branchNumfile.fail()){
            qDebug()<<"file can't open!";
        }
        V3DLONG size=infiles.size();
        if(size==1){//所有文件在同一个文件夹下
            // -i directory(v3draw,swc)
            vector<QString> swcfiles;
            vector<QString> v3drawfiles;
            QString dir=infiles[0];
            find_files(infiles[0],swcfiles,v3drawfiles);
            for(int i=0;i<swcfiles.size();i++){//所有的swc文件处理, id_x_y_z
                QString imgfile;

                QString swcfilepath=dir+"\\"+swcfiles[i];
                NeuronTree nt=readSWC_file(swcfilepath);
                if(nt.listNeuron.size()<50)continue;
                bool flag=false;
                flag=find_corresponding_file(dir,swcfiles[i],imgfile);
                if(flag){//找到
                    const char* v3drawfile=imgfile.toStdString().c_str();

                    unsigned char* p1data=0;
                    unsigned char*** imag3d=0;
                    NeuronSWC soma;
                    V3DLONG sz[4]={0,0,0,0};
                    int datatype=0;
                    double radius=0;
                    SWCTree t;
                    QList<ImageMarker> markers;
                    vector<location> points;
                    double pc1=0,pc2=0,pc3=0;
                    double pc13=0;
                    simple_loadimage_wrapper(callback,v3drawfile,p1data,sz,datatype);//读取v3draw
                    p1data_to_image3d(p1data,sz,imag3d);
                    find_soma(nt,soma,radius);//是否要坐标变换
                    if(radius<10)radius=10;
                    qDebug()<<"radius:"<<radius;
                    location blocko(soma.x-128,soma.y-128,soma.z-64);
                    soma.x=128;
                    soma.y=128;
                    soma.z=64;
                    mean_shift_soma_location_hp(imag3d,soma,sz[0],sz[1],sz[2],radius,45);
                    soma.x+=blocko.x;
                    soma.y+=blocko.y;
                    soma.z+=blocko.z;

                    t.count_branch_location(nt,markers,points,soma,radius);
                    V3DLONG max_r=(V3DLONG)(radius+1);
                    pc13=compute_marker_pca_hp(points,max_r,pc1,pc2,pc3);

                    qDebug()<<"sigma1/sigma2:"<<pc13;
                    datafile<<pc13<<endl;

                    qDebug()<<"branch num:"<<markers.size()-1;//已添加移动后soma的位置
                    branchNumfile<<markers.size()-1<<endl;
                    writeMarker_file(apofile+"//"+swcfiles[i]+".marker",markers);
                    for(int i=0;i<sz[2];i++){
                        for(int j=0;j<sz[1];j++){
                             delete[]imag3d[i][j];
                           }
                            delete[]imag3d[i];
                        }
                       delete[] imag3d;
                    qDebug()<<"--------------";

                }
                else
                    continue;
    }
        }
    else{






//                for(int k=0;k<v3drawfiles.size();k++){//所有V3DRAW，x_y_z

//                    if(v3drawfiles[k].contains(QString::number(cord[1]*2)+"_"+QString::number(cord[2]*2)+"_"+QString::number(cord[3]*2),Qt::CaseSensitive)==true){
//                        qDebug()<<v3drawfiles[k];

//                    QStringList drawf=v3drawfiles[k].split(QRegExp("[_|\\.]"));
//                    qDebug()<<drawf;
//                    vector<V3DLONG> cordd;
//                    //一个V3DRAW文件坐标获取
//                    for(int l=0;l<drawf.size();l++ ){
//                        string tmpdraw=drawf[l].toStdString();
//                        V3DLONG num2=0;
//                        bool flag2=false;
//                        //x,y,z
//                        for(int ii=0;ii<tmpdraw.length();ii++){
//                            if(tmpdraw[ii]>='0'&&tmpdraw[ii]<='9'){
//                                num2=num2*10+(tmpdraw[ii]-'0');
//                            }
//                            else {
//                                flag2=true;
//                                break;}
//                        }
//                        if(flag2==true){
//                            flag2=false;
//                        }
//                        else{
//                            cordd.push_back(num2);
//                            qDebug()<<num2;
//                        }

//                    }
 //                   if((cord[1]==cordd[0]/2)&& (cord[2]==cordd[1]/2 && cord[3]==cordd[2]/2)){//x
                        //找到两个文件

//                        //QString imgfile=dir+"\\"+v3drawfiles[k];
//                        const char* v3drawfile=imgfile.toStdString().c_str();

//                        unsigned char* p1data=0;
//                        unsigned char*** imag3d=0;
//                        NeuronSWC soma;
//                        V3DLONG sz[4]={0,0,0,0};
//                        int datatype=0;
//                        double radius=0;
//                        SWCTree t;
//                        QList<ImageMarker> markers;
//                        vector<location> points;
//                        double pc1=0,pc2=0,pc3=0;
//                        double pc13=0;
//                        simple_loadimage_wrapper(callback,v3drawfile,p1data,sz,datatype);//读取v3draw
//                        p1data_to_image3d(p1data,sz,imag3d);
//                        find_soma(nt,soma,radius);//是否要坐标变换
//                        if(radius<10)radius=10;
//                        qDebug()<<"radius:"<<radius;
//                        location blocko(soma.x-128,soma.y-128,soma.z-64);
//                        soma.x=128;
//                        soma.y=128;
//                        soma.z=64;
//                        mean_shift_soma_location_hp(imag3d,soma,sz[0],sz[1],sz[2],radius,45);
//                        soma.x+=blocko.x;
//                        soma.y+=blocko.y;
//                        soma.z+=blocko.z;

//                        t.count_branch_location(nt,markers,points,soma,radius);
//                        V3DLONG max_r=(V3DLONG)(radius+1);
//                        pc13=compute_marker_pca_hp(points,max_r,pc1,pc2,pc3);

//                        qDebug()<<"sigma1/sigma2:"<<pc13;
//                        datafile<<pc13<<endl;

//                        qDebug()<<"branch num:"<<markers.size()-1;//已添加移动后soma的位置
//                        branchNumfile<<markers.size()-1<<endl;
//                        writeMarker_file(apofile+"//"+swcfiles[i]+".marker",markers);
//                        for(int i=0;i<sz[2];i++){
//                            for(int j=0;j<sz[1];j++){
//                                 delete[]imag3d[i][j];
//                               }
//                                delete[]imag3d[i];
//                            }
//                           delete[] imag3d;
//                        qDebug()<<"--------------";
//                        break;

//                    }
//        //            else continue;
//                }



//            }
//            datafile.close();
//            branchNumfile.close();


//        }
//        for(int i=0;i<size;i++){
//            QString swcfile=infiles[i];
//            NeuronTree nt=readSWC_file(swcfile);
//            //QString origfile=infiles[i+1];
//            NeuronSWC soma;
//            double radius;
//            find_soma(nt,soma,radius);
//            if(radius<15)radius=15;
//            qDebug()<<"radius:"<<radius;
//            qDebug()<<"founded soma location!";
//            //qDebug()<<"22:"<<imag3d[0][0][0];
//            SWCTree t;
//            QList<ImageMarker> markers;
//            vector<location> points;
//            double pc1=0,pc2=0,pc3=0;
//            double pc13=0;
//            //double max_radius;
//            QFileInfo eswcfileinfo;
//            eswcfileinfo=QFileInfo(swcfile);
//            QString eswcfile=eswcfileinfo.fileName();
//            eswcfile.mid(0,eswcfile.indexOf("."));
//            t.count_branch_location(nt,markers,points,soma,radius);
//            V3DLONG max_r=(V3DLONG)(radius+1);
//            compute_marker_pca_hp(points,max_r,pc1,pc2,pc3);
//            qDebug()<<"pc num:"<<pc1<<pc2<<pc3;
//            if(pc1==0||pc2==0||pc3==0){
//                qDebug()<<"it is pc problem!"<<endl;
//                pc13=1;
//            }
//            else{
//                double num1,num2;//min,max
//                num1=(pc1<pc2)?pc1:pc2;
//                num1=(num1<pc3)?num1:pc3;
//                num2=(pc1<pc2)?pc2:pc1;
//                num2=(num2<pc3)?pc3:num2;
//                pc13=num1/num2;
//            }
//            qDebug()<<"sigma1/sigma2:"<<pc13;
//            //datafile<<markers.size()<<" "<<pc13<<endl;
//            datafile<<pc13<<endl;

//            qDebug()<<"branch num:"<<markers.size()-1;//已添加移动后soma的位置
//            branchNumfile<<markers.size()-1<<endl;
//            writeMarker_file(apofile+"//"+eswcfile+".marker",markers);

//            qDebug()<<"--------------";

//        }

        // -i x.swc  x.v3draw
        if(size%2==1)size-=1;
        for(int i=0;i<size;i+=2){
            QString swcfile=infiles[i];
            NeuronTree nt=readSWC_file(swcfile);
            //QString origfile=infiles[i+1];
            const char* v3drawfile=infiles[i+1];
            qDebug()<<"image:"<<v3drawfile<<endl;
            unsigned char* p1data=0;
            unsigned char*** imag3d=0;
            NeuronSWC soma;
            V3DLONG sz[4]={0,0,0,0};
            int datatype=0;
            double radius;
            //读取v3draw图
            //p1data=callback.loadImage(v3drawfile,1);
            simple_loadimage_wrapper(callback,v3drawfile,p1data,sz,datatype);//读取v3draw
            qDebug()<<"finish read v3draw!";
            //将一维转换为三维
            qDebug()<<"XDim,YDim,ZDim:"<<sz[0]<<sz[1]<<sz[2];
            p1data_to_image3d(p1data,sz,imag3d);
            find_soma(nt,soma,radius);
            if(radius<15)radius=15;
            qDebug()<<"radius:"<<radius;
            qDebug()<<"founded soma location!";
            //qDebug()<<"22:"<<imag3d[0][0][0];
            mean_shift_soma_location_hp(imag3d,soma,sz[0],sz[1],sz[2],radius,45);
            SWCTree t;
            QList<ImageMarker> markers;
            vector<location> points;
            double pc1=0,pc2=0,pc3=0;
            double pc13=0;
            //double max_radius;
            QFileInfo eswcfileinfo;
            eswcfileinfo=QFileInfo(swcfile);
            QString eswcfile=eswcfileinfo.fileName();
            eswcfile.mid(0,eswcfile.indexOf("."));
            t.count_branch_location(nt,markers,points,soma,radius);
            V3DLONG max_r=(V3DLONG)(radius+1);
            compute_marker_pca_hp(points,max_r,pc1,pc2,pc3);
            qDebug()<<"pc num:"<<pc1<<pc2<<pc3;
            pc13=compute_marker_pca_hp(points,max_r,pc1,pc2,pc3);
            qDebug()<<"sigma1/sigma2:"<<pc13;
            //datafile<<markers.size()<<" "<<pc13<<endl;
            datafile<<pc13<<endl;
    //        bool flag=four_point(points);
    //        if(flag==true) qDebug()<<"yes!";
    //        else qDebug()<<"no!";
            qDebug()<<"branch num:"<<markers.size()-1;//已添加移动后soma的位置
            branchNumfile<<markers.size()-1<<endl;
            writeMarker_file(apofile+"//"+eswcfile+".marker",markers);
            for(int i=0;i<sz[2];i++){
                for(int j=0;j<sz[1];j++){
                    delete[]imag3d[i][j];
                }
                delete[]imag3d[i];
            }
            delete[] imag3d;
            qDebug()<<"--------------";

        }
      }
        datafile.close();
        branchNumfile.close();



	}
    else if (func_name == tr("mean_shift_soma"))
    {//根据原图和soma位置重新调整soma位置写入原soma文件
        //输入v3draw，soma的marker文件  -i x.v3draw x.marker
        V3DLONG size=infiles.size();
        for(int i=0;i<size;i++){
            const char* v3drawfile=infiles[i];
            QString somafile=infiles[i+1];
            QList<ImageMarker> somaMarker=readMarker_file(somafile);
            ImageMarker somaMk=somaMarker[0];
            NeuronSWC soma;
            soma.x=somaMk.x-1;
            soma.y=somaMk.y-1;
            soma.z=somaMk.z-1;

            qDebug()<<"image:"<<v3drawfile<<endl;
            unsigned char* p1data=0;
            unsigned char*** imag3d=0;
            V3DLONG sz[4]={0,0,0,0};
            int datatype=0;
            double radius=15;//固定半径
            //读取v3draw图
            //p1data=callback.loadImage(v3drawfile,1);
            simple_loadimage_wrapper(callback,v3drawfile,p1data,sz,datatype);//读取v3draw
            qDebug()<<"finish read v3draw!";
            //将一维转换为三维
            qDebug()<<"XDim,YDim,ZDim:"<<sz[0]<<sz[1]<<sz[2];
            p1data_to_image3d(p1data,sz,imag3d);
            mean_shift_soma_location_hp(imag3d,soma,sz[0],sz[1],sz[2],radius,45);

            ImageMarker* newsomaMk=new ImageMarker();
            newsomaMk->x=soma.x+1;
            newsomaMk->y=soma.y+1;
            newsomaMk->z=soma.z+1;
            newsomaMk->color.r=0;
            newsomaMk->color.g=255;
            newsomaMk->color.b=0;
            QList<ImageMarker> NewsomaMarker;
            NewsomaMarker.push_back(*newsomaMk);
            writeMarker_file(somafile,NewsomaMarker);

            delete newsomaMk;


        }
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

