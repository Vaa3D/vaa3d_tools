#include "review.h"
#include<math.h>
#include<algorithm>
#include "filter_dialog.h"
#include "review_gui.h"
#include <iostream>
#include "review_neuron_reconstruction_plugin.h"
bool review_neuron(V3DPluginCallback2 &callback,QWidget *parent)
{
    QString swcFile;
    EmployDialog dialog(parent);
    if(dialog.exec()!=QDialog::Accepted)return false;
    dialog.update();

    ReviewNeuronParameter pt;
    pt.SWC_FILE = QString(dialog.swc_file.c_str());
    pt.BRAIN_FILE = QString(dialog.brain_file.c_str());
    pt.SAVE_FOLDER = QString(dialog.save_folder.c_str());
    //QFileInfo SWC_FILE = QFileInfo(pt.SWC_FILE);
    QString swcfile = pt.SWC_FILE;

    QString brainfile = pt.BRAIN_FILE;
    if(swcfile.endsWith(".eswc")||swcfile.endsWith(".swc")){
        swcFile=swcfile;//输入的swc文件
    }
    else{
        v3d_msg("Please select right swc/eswc file!");
        return false;
    }

    string save_folder = dialog.save_folder;
    if(save_folder[0]=='~'){
        save_folder.erase(0,1);
        save_folder = QDir::homePath().toStdString();
    }
    else if(save_folder[1] == ':'){//windows 完整路径，不做处理

    }
    else if(save_folder[0]!='/'&&save_folder[0]!='.'){
        save_folder="./"+save_folder;
    }
    QDir dir(save_folder.c_str());
    if(!dir.exists()){
        QMessageBox::warning(0, QObject::tr("ERROR"), QObject::tr("Un existing folder:%1").arg(save_folder.c_str()));
        return false;
    }
    else{
        v3d_msg(QString("Save Directory:%1").arg(save_folder.c_str()));
    }
    //QString cur_path = QDir::current().dirName();
    cout<<"current path:"<<QDir::current().dirName().toStdString()<<endl;
    QDir::setCurrent(save_folder.c_str());
    cout<<"current path:"<<QDir::current().dirName().toStdString()<<endl;
//    QDir::setCurrent(cur_path);
//    cout<<"current path : "<<QDir::current().dirName().toStdString()<<endl;

    V3DLONG sz0=512,sz1=512,sz2=256;
    vector<NeuronSWC> candidate_point;
    QList<CellAPO> markers;
    NeuronTree new_sequence_tree;
    //swcFile=pt.SWC_FILE;//输入swc文件
    qDebug()<<"swc file:"<<swcFile;
    NeuronTree t = readSWC_file(swcFile);
    QString input_path = pt.BRAIN_FILE;//输入脑的位置
    QString save_path = QString(save_folder.c_str());
    SWCTreeSeg swcTree;
    //获取孩子节点
    swcTree.initialize(t);
    //在整个树中移动块，并在每个块中完成分段和平均灰度值的计算
    move_block(input_path,callback,new_sequence_tree,markers,t,swcTree,sz0,sz1,sz2,candidate_point);//移动块获取候选点分段
    new_sequence_tree.listNeuron.push_back(t.listNeuron[t.hashNeuron.value(swcTree.root.n)]);

    //写文件
    QFileInfo eswcfileinfo;
    eswcfileinfo=QFileInfo(swcFile);
    QString eswcfile=eswcfileinfo.fileName();
    eswcfile.mid(0,eswcfile.indexOf("."));
    //nt1.listNeuron.push_back(orig.listNeuron[orig.hashNeuron.value(swcTree.root.n)]);
    writeESWC_file(save_path+"//"+eswcfile+".eswc",new_sequence_tree);
    //writeAPO_file("C://Users//penglab//Desktop//17302-00001//review_test1//marker.apo",markers);
    writeAPO_file(save_path+"//"+eswcfile+".apo",markers);
    v3d_msg("finish write eswc and apo file!");

    return true;
}
