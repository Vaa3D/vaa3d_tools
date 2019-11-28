#include "show_segment_gui.h"
#include<math.h>
#include<iostream>
#include<v3d_interface.h>
#include "review.h"
#include<direct.h>
#include<io.h>

bool show_dialog(V3DPluginCallback2 &callback,QWidget *parent){
    ShowDialog dialog(parent);
    if(dialog.exec()!=QDialog::Accepted){
        return false;
    }
    dialog.update();
    show_segment(callback,dialog.current_index,dialog.swc_file,dialog.save_folders);
     return true;
}

bool show_segment(V3DPluginCallback2 &callback,V3DLONG index,QString open_swc,QString save_folders)
{
    //ShowDialog dialog(parent);
    //if(dialog.exec()!=QDialog::Accepted)return false;
    //dialog.show();
    //dialog.setAttribute(Qt::WA_DeleteOnClose);//设置对话框关闭后，自动销毁

    //dialog.update();
    //V3DLONG index=dialog.current_index;
    qDebug()<<"index:"<<index;
    NeuronTree nt;
    QList<CellAPO> markers;
    //vector<NeuronSWC> current_segment;
    NeuronTree seg_tree;
    size_t x0=IN,y0=IN,z0=IN,x1=0,y1=0,z1=0;
    QString link_file;
    QString swc_file;
    QString tiffile;
    QString file_name=QFileInfo(open_swc).fileName();
    file_name=file_name.mid(0,11);
    qDebug()<<file_name;
    //QString open_swc;
    string save_path=save_folders.toStdString().c_str();
    QDir dir(save_path.c_str());
    if(!dir.exists()){
        QMessageBox::warning(0, QObject::tr("ERROR"), QObject::tr("Un existing folder:%1").arg(save_path.c_str()));
        return false;
    }
    else{
        v3d_msg(QString("Save Directory:%1").arg(save_path.c_str()));
    }
    QDir::setCurrent(save_path.c_str());
    cout<<"current path:"<<QDir::current().dirName().toStdString()<<endl;
    QString save_folder=save_folders+"//"+file_name;
    if(_access(save_folder.toStdString().c_str(),0)==-1){
        _mkdir(save_folder.toStdString().c_str());
    }
    QString save_location=save_folder+"//"+QString::number(index)+"//";
    if( _access(save_location.toStdString().c_str(),0)==-1){
        _mkdir(save_location.toStdString().c_str());
        v3d_msg("finish");
    }
    //open_swc=QString(dialog.swc_file.c_str());
    nt=readSWC_file(open_swc);//分段排序完的完整swc/eswc文件

    //nt=callback.getSWCTeraFly();//获取terafly中打开的apo文件
    if(nt.listNeuron.isEmpty()||nt.listNeuron[0].seg_id==-1){
        v3d_msg("please load right swc file in terafly!");
        //return false;
    }
    qDebug()<<nt.listNeuron.size();
    V3DLONG i=0;
    for(;i<nt.listNeuron.size();i++){
        if(nt.listNeuron[i].seg_id<index){
            continue;
        }
        else if(nt.listNeuron[i].seg_id==index){
            NeuronSWC point=nt.listNeuron[i];
            //获取当前段的坐标范围
            x0=(point.x<x0)?point.x:x0;
            x1=(point.x>x1)?point.x:x1;
            y0=(point.y<y0)?point.y:y0;
            y1=(point.y>y1)?point.y:y1;
            z0=(point.z<z0)?point.z:z0;
            z1=(point.z>z1)?point.z:z1;
            seg_tree.listNeuron.push_back(point);

        }
        else
            break;
    }
    if(i==nt.listNeuron.size()&&seg_tree.listNeuron.isEmpty()){
        v3d_msg("index is over range!");
        return false;
    }

    x0-=5;
    y0-=5;
    z0-=5;
    x1+=5;
    y1+=5;
    z1+=5;
    qDebug()<<"x0,x1,y0,y1,z0,z1"<<x0<<x1<<y0<<y1<<z0<<z1;
    V3DLONG sz0=x1-x0;
    V3DLONG sz1=y1-y0;
    V3DLONG sz2=z1-z0;
    //切图获取段的大小
    unsigned char* p1data=0;
    qDebug()<<callback.getPathTeraFly();
    p1data=callback.getSubVolumeTeraFly(callback.getPathTeraFly().toStdString().c_str(),x0,x1,y0,y1,z0,z1);
    if(p1data==0){
        v3d_msg("please open the right brain in terafly!");
    }
    tiffile = save_location+QString::number(x0)+"_"+QString::number(y0)+"_"+QString::number(z0)+".tif";
    V3DLONG sz[4] = {sz0,sz1,sz2,1};
    //保存切下的块
    simple_saveimage_wrapper(callback,tiffile.toStdString().c_str(),p1data,sz,1);
    for(int j=0;j<seg_tree.listNeuron.size();j++){
        //转换为块内的相对坐标
        seg_tree.listNeuron[j].x-=x0;
        seg_tree.listNeuron[j].y-=y0;
        seg_tree.listNeuron[j].z-=z0;
    }
    swc_file=save_location+file_name+".eswc";
    writeESWC_file(swc_file,seg_tree);
    QFile tif_file(tiffile);
    QFile eswc_file(swc_file);
//    while(!tif_file.exists()||!eswc_file.exists()){
//        continue;
//    }
    link_file=save_location+file_name+".ano";
    QFile qf_anofile(link_file);
    if(!qf_anofile.open(QIODevice::WriteOnly))
    {
        v3d_msg("Cannot open file for writing!");
        return false;
    }
    QTextStream out(&qf_anofile);
    out << "SWCFILE=" << QFileInfo(swc_file).fileName()<<endl;
    out << "RAWIMG=" << QFileInfo(tiffile).fileName()<<endl;
    v3d_msg(QString("Save the linker file to: %1 Complete!").arg(link_file));

    //callback.callPluginFunc();
    if(qf_anofile.exists()){
        callback.open3DViewerForLinkerFile(link_file);//打开连接文件ano

    }
    else{
        v3d_msg("ano file is not finish!");
    }


    return true;
    //return dialog.exec();
}
