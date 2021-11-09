/* pruning_swc_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2014-05-02 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "pruning_swc_plugin.h"

#include <math.h>
#include "basic_surf_objs.h"
#include <iostream>
#include "my_surf_objs.h"
#include <QInputDialog>
#include <QComboBox>
#include <QPushButton>

static controlPanel *ControlPanel=0;

void pruningControlPanel()
{
    if (ControlPanel)
    {
        delete ControlPanel;
        ControlPanel=0;
    }
}


using namespace std;
#define VOID 1000000000
#define PI 3.14159265359
#define min(a,b) (a)<(b)?(a):(b)
#define max(a,b) (a)>(b)?(a):(b)
#define dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))
#define angle(a,b,c) (acos((((b).x-(a).x)*((c).x-(a).x)+((b).y-(a).y)*((c).y-(a).y)+((b).z-(a).z)*((c).z-(a).z))/(dist(a,b)*dist(a,c)))*180.0/PI)



//Q_EXPORT_PLUGIN2(pruning_swc, pruning_swc);

bool export_list2file(QList<NeuronSWC> & lN, QString fileSaveName, QString fileOpenName)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;
    QTextStream myfile(&file);

    QFile qf(fileOpenName);
    if (! qf.open(QIODevice::ReadOnly | QIODevice::Text))
    {
#ifndef DISABLE_V3D_MSG
        v3d_msg(QString("open file [%1] failed!").arg(fileOpenName));
#endif
        return false;
    }
    QString info;
    while (! qf.atEnd())
    {
        char _buf[1000], *buf;
        qf.readLine(_buf, sizeof(_buf));
        for (buf=_buf; (*buf && *buf==' '); buf++); //skip space

        if (buf[0]=='\0')	continue;
        if (buf[0]=='#')
        {
           info = buf;
           myfile<< info.remove('\n');
        }

    }

    for (V3DLONG i=0;i<lN.size();i++)
        myfile << lN.at(i).n <<" " << lN.at(i).type << " "<< lN.at(i).x <<" "<<lN.at(i).y << " "<< lN.at(i).z << " "<< lN.at(i).r << " " <<lN.at(i).pn << " "
               << lN.at(i).seg_id <<" " << lN.at(i).level << " "<< lN.at(i).creatmode <<" "<<lN.at(i).timestamp << " "<< lN.at(i).tfresindex <<"\n";

    file.close();
    cout<<"swc file "<<fileSaveName.toStdString()<<" has been generated, size: "<<lN.size()<<endl;
    return true;
};

void subtree(int ID, QVector<QVector<V3DLONG> > childs,V3DLONG *flag);
 
QStringList pruning_swc::menulist() const
{
	return QStringList() 
        <<tr("pruning")
        <<tr("pruning_iterative")
        <<tr("pruning based on markers")
		<<tr("about");
}

QStringList pruning_swc::funclist() const
{
	return QStringList()
        <<tr("pruning_iterative")
		<<tr("help");
}

NeuronTree prune_swc_simple(NeuronTree nt, double length, bool& pruned){
    QVector<QVector<V3DLONG> > childs;
    V3DLONG neuronNum = nt.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    V3DLONG *flag = new V3DLONG[neuronNum];
    double *segment_length = new double[neuronNum];
    V3DLONG *parent_id = new V3DLONG[neuronNum];

    for (V3DLONG i=0;i<neuronNum;i++)
    {
        flag[i] = 1;
        segment_length[i] = 100000.00;
        parent_id[i] = -1;
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }

    QList<NeuronSWC> list = nt.listNeuron;
    for (int i=0;i<list.size();i++)
    {
        if (childs[i].size()==0  && list.at(i).parent >=0)
        {
            int parent_tip = getParent(i,nt);
            MyMarker curr_node, parent_node;
            curr_node.x = list.at(i).x;
            curr_node.y = list.at(i).y;
            curr_node.z = list.at(i).z;

            parent_node.x = list.at(parent_tip).x;
            parent_node.y = list.at(parent_tip).y;
            parent_node.z = list.at(parent_tip).z;
            double index_tip = dist(curr_node,parent_node);

            while(childs[parent_tip].size()<2)
            {
                MyMarker curr_node, parent_node;

                curr_node.x = list.at(parent_tip).x;
                curr_node.y = list.at(parent_tip).y;
                curr_node.z = list.at(parent_tip).z;

                int newParent = getParent(parent_tip,nt);
                if(newParent == 1000000000) break;

                parent_node.x = list.at(newParent).x;
                parent_node.y = list.at(newParent).y;
                parent_node.z = list.at(newParent).z;

                index_tip += dist(curr_node,parent_node);

                parent_tip = getParent(parent_tip,nt);

                if(parent_tip == 1000000000)    break;
             }

            int parent_index = parent_tip;

            if(index_tip < length)
            {
                flag[i] = -1;
                segment_length[i] = index_tip;
                parent_id[i] = parent_index;
                int parent_tip = getParent(i,nt);
                while(childs[parent_tip].size()<2)
                {
                    flag[parent_tip] = -1;
                    segment_length[parent_tip] = index_tip;
                    parent_id[parent_tip] = parent_index;
                    parent_tip = getParent(parent_tip,nt);
                    if(parent_tip == 1000000000)
                        break;
                }
                if(parent_tip != 1000000000 && segment_length[parent_tip] > index_tip)
                    segment_length[parent_tip]  = index_tip;
            }
        }
    }

    //NeutronTree structure
    NeuronTree nt_prunned;
    QList <NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();

    //set node

    NeuronSWC S;
    for (int i=0;i<list.size();i++)
    {
        if(flag[i] == 1 || (flag[i] != 1 && (segment_length[i] > segment_length[parent_id[i]])))
        {
             NeuronSWC curr = list.at(i);
             S.n 	= curr.n;
             S.type 	= curr.type;
             S.x 	= curr.x;
             S.y 	= curr.y;
             S.z 	= curr.z;
             S.r 	= curr.r;
             S.pn 	= curr.pn;
             S.seg_id = curr.seg_id;
             S.level = curr.level;
             S.creatmode = curr.creatmode;
             S.timestamp = curr.timestamp;
             S.tfresindex = curr.tfresindex;

             listNeuron.append(S);
             hashNeuron.insert(S.n, listNeuron.size()-1);
        }

   }
    if(listNeuron.size()==0){
        // If after pruning, no branch remains
        qDebug()<<"Reached the last branch, no pruning will be performed.";
        nt_prunned.deepCopy(nt);
        pruned = 0;
    }
    else{
        pruned = (listNeuron.size()<list.size())? 1:0;
        if(listNeuron.size()<list.size()){
            pruned = 1;
        }
        else{
            pruned = 0;
        }
       nt_prunned.n = -1;
       nt_prunned.on = true;
       nt_prunned.listNeuron = listNeuron;
       nt_prunned.hashNeuron = hashNeuron;
    }
   return nt_prunned;

}
NeuronTree prune_swc_iterative(NeuronTree nt, double length, bool& prunned){
    int rounds = 0;
    prunned = 1;
    NeuronTree nt_prunned;
    nt_prunned.deepCopy(nt);
    while(prunned){
        nt_prunned = prune_swc_simple(nt_prunned, length, prunned);
        rounds++;
        qDebug()<<"Iteration "<<rounds<<": current size="<<nt_prunned.listNeuron.size();
    }

    rounds --;
    qDebug()<<(qPrintable(QString::number(rounds)+" iterations of prunning performed"));
    if(rounds>0){
        qDebug()<<("Some branches of SWC have been pruned.");
        prunned = 1;
    }
    else{
        qDebug()<<("No branch has been pruned.");
        prunned = 0;
    }
    return nt_prunned;
}

void pruning_swc::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("pruning"))
    {
                QString fileOpenName;
                fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
                        "",
                        QObject::tr("Supported file (*.swc *.eswc)"
                            ";;Neuron structure	(*.swc)"
                            ";;Extended neuron structure (*.eswc)"
                            ));
                if(fileOpenName.isEmpty())
                    return;
                double length = 0;
                NeuronTree nt;
                if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
                {
                     bool ok;
                     nt = readSWC_file(fileOpenName);
                     length = QInputDialog::getDouble(parent, "Please specify the maximum prunned segment length","segment length:",1,0,2147483647,0.1,&ok);
                     if (!ok)
                         return;
                }

                QVector<QVector<V3DLONG> > childs;


                V3DLONG neuronNum = nt.listNeuron.size();
                childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
                V3DLONG *flag = new V3DLONG[neuronNum];
                double *segment_length = new double[neuronNum];
                V3DLONG *parent_id = new V3DLONG[neuronNum];

                for (V3DLONG i=0;i<neuronNum;i++)
                {
                    flag[i] = 1;
                    segment_length[i] = 100000.00;
                    parent_id[i] = -1;
                    V3DLONG par = nt.listNeuron[i].pn;
                    if (par<0) continue;
                    childs[nt.hashNeuron.value(par)].push_back(i);
                }

                QList<NeuronSWC> list = nt.listNeuron;
                for (int i=0;i<list.size();i++)
                {
                    if (childs[i].size()==0  && list.at(i).parent >=0)
                    {
                        int parent_tip = getParent(i,nt);
                        MyMarker curr_node, parent_node;
                        curr_node.x = list.at(i).x;
                        curr_node.y = list.at(i).y;
                        curr_node.z = list.at(i).z;

                        parent_node.x = list.at(parent_tip).x;
                        parent_node.y = list.at(parent_tip).y;
                        parent_node.z = list.at(parent_tip).z;
                        double index_tip = dist(curr_node,parent_node);

                        while(childs[parent_tip].size()<2)
                        {
                            MyMarker curr_node, parent_node;

                            curr_node.x = list.at(parent_tip).x;
                            curr_node.y = list.at(parent_tip).y;
                            curr_node.z = list.at(parent_tip).z;

                            int newParent = getParent(parent_tip,nt);
                            if(newParent == 1000000000) break;

                            parent_node.x = list.at(newParent).x;
                            parent_node.y = list.at(newParent).y;
                            parent_node.z = list.at(newParent).z;

                            index_tip += dist(curr_node,parent_node);

                            parent_tip = getParent(parent_tip,nt);

                            if(parent_tip == 1000000000)    break;
                         }

                        int parent_index = parent_tip;

                        if(index_tip < length)
                        {
                            flag[i] = -1;
                            segment_length[i] = index_tip;
                            parent_id[i] = parent_index;
                            int parent_tip = getParent(i,nt);
                            while(childs[parent_tip].size()<2)
                            {
                                flag[parent_tip] = -1;
                                segment_length[parent_tip] = index_tip;
                                parent_id[parent_tip] = parent_index;
                                parent_tip = getParent(parent_tip,nt);
                                if(parent_tip == 1000000000)
                                    break;
                            }
                            if(parent_tip != 1000000000 && segment_length[parent_tip] > index_tip)
                                segment_length[parent_tip]  = index_tip;
                        }
                    }
                }

                //NeutronTree structure
                NeuronTree nt_prunned;
                QList <NeuronSWC> listNeuron;
                QHash <int, int>  hashNeuron;
                listNeuron.clear();
                hashNeuron.clear();

                //set node

                NeuronSWC S;
                for (int i=0;i<list.size();i++)
                {
                    if(flag[i] == 1 || (flag[i] != 1 && (segment_length[i] > segment_length[parent_id[i]])))
                    {
                         NeuronSWC curr = list.at(i);
                         S.n 	= curr.n;
                         S.type 	= curr.type;
                         S.x 	= curr.x;
                         S.y 	= curr.y;
                         S.z 	= curr.z;
                         S.r 	= curr.r;
                         S.pn 	= curr.pn;
                         S.seg_id = curr.seg_id;
                         S.level = curr.level;
                         S.creatmode = curr.creatmode;
                         S.timestamp = curr.timestamp;
                         S.tfresindex = curr.tfresindex;

                         listNeuron.append(S);
                         hashNeuron.insert(S.n, listNeuron.size()-1);
                    }

               }
                nt_prunned.n = -1;
                nt_prunned.on = true;
                nt_prunned.listNeuron = listNeuron;
                nt_prunned.hashNeuron = hashNeuron;

                if(flag) {delete[] flag; flag = 0;}
                if(segment_length) {delete[] segment_length; segment_length = 0;}
                if(parent_id) {delete[] parent_id; parent_id = 0;}

                QString fileDefaultName = fileOpenName+QString("_pruned.eswc");
                //write new SWC to file
                QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
                        fileDefaultName,
                        QObject::tr("Supported file (*.eswc)"
                            ";;Neuron structure	(*.eswc)"
                            ));
                if (!export_list2file(nt_prunned.listNeuron,fileSaveName,fileOpenName))
                {
                    v3d_msg("fail to write the output swc file.");
                    return;
                }
    }
    else if (menu_name == tr("pruning_iterative"))
    {
                QString fileOpenName;
                fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
                        "",
                        QObject::tr("Supported file (*.swc *.eswc)"
                            ";;Neuron structure	(*.swc)"
                            ";;Extended neuron structure (*.eswc)"
                            ));
                if(fileOpenName.isEmpty())
                    return;
                double length = 0;
                NeuronTree nt;
                if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
                {
                     bool ok;
                     nt = readSWC_file(fileOpenName);
                     length = QInputDialog::getDouble(parent, "Please specify the maximum prunned segment length","segment length:",1,0,2147483647,0.1,&ok);
                     if (!ok)
                         return;
                }

                bool prunned = 1;
                int rounds = 0;
                NeuronTree nt_prunned;
                nt_prunned.deepCopy(nt);
                while(prunned){
                    nt_prunned = prune_swc_simple(nt_prunned, length, prunned);
                    rounds++;
                    qDebug()<<rounds<<nt_prunned.listNeuron.size();
                }
                v3d_msg(qPrintable(QString::number(rounds)+"iterations of prunning performed"));
                if(prunned){
                    v3d_msg("Some branches of SWC have been pruned.");
                }
                else{
                    v3d_msg("No branch has been pruned.");
                }

                QString fileDefaultName = fileOpenName+QString("_pruned.eswc");
                //write new SWC to file
                QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
                        fileDefaultName,
                        QObject::tr("Supported file (*.eswc)"
                            ";;Neuron structure	(*.eswc)"
                            ));
                if (!export_list2file(nt_prunned.listNeuron,fileSaveName,fileOpenName))
                {
                    v3d_msg("fail to write the output swc file.");
                    return;
                }
    }

    else if (menu_name == tr("pruning based on markers"))
    {
        ControlPanel = new controlPanel(callback, parent);
        if (ControlPanel)
        {
            ControlPanel->show();
            ControlPanel->raise();
            ControlPanel->move(500,50);
            ControlPanel->activateWindow();
        }

    }
	else
	{
        v3d_msg(tr("This is a plugin to prune the swc file. "
			"Developed by Zhi Zhou, 2014-05-02"));
	}
}


void printHelp(const V3DPluginArgList & input, V3DPluginArgList & output)
{
    cout<<"Prune swc branches shorter than the threshold."<<endl;
    cout<<"usage:\n";
    cout<<"-f<func name>:\t\t pruning or pruning_iterative\n";
    cout<<"-i<file name>:\t\t input .swc file\n";
    cout<<"-o<file name>:\t\t output .swc file\n";
    cout<<"-p<pruning threshold>:\t a positive float number (unit: pixel)\n";
    cout<<"Demo1 (remove tiny branches):\t vaa3d -x pruning_swc_simple -f pruning -i test.swc -o test_pruned.swc -p 10 \n";
    cout<<"Demo2 (getting long porjection fibirs):\t vaa3d -x pruning_swc_simple -f pruning_iterative -i test.swc -o test_pruned.swc -p 2000 \n";
}

bool pruning_swc::dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent){

    if (func_name == tr("help"))
    {
        printHelp(input, output);
        return 0;
    }
    // Read input
    cout<<"==========Welcome to pruning_swc_iterative function============="<<endl;
    vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
    vector<char*>* outlist = NULL;
    vector<char*>* paralist = NULL;

    double length = VOID;
    bool hasPara, hasOutput;
    if (input.size()==1)
    {
        cout<<"No new parameter specified.\n";
        hasPara = false;
    }
    else {
        hasPara = true;
        paralist = (vector<char*>*)(input.at(1).p);
    }

    if (inlist->size()!=1)
    {
        cout<<"You must specify 1 input file!"<<endl;
        return 0;
    }

    if (output.size()==0){
        cout<<"No output file specified.\n";
        hasOutput = false;
        return 0;
    }
    else {
        hasOutput = true;
        if (output.size()>1)
        {
            cout<<"You have specified more than 1 output file.\n";
            return 0;
        }
        outlist = (vector<char*>*)(output.at(0).p);
    }

    if (hasPara)
    {
        if (paralist->size()==0)
        {
            cout<< "Threshold not set: No pruning will be performed." << endl;
            return 1;
        }
        else if (paralist->size() == 1)
        {
            length = atof(paralist->at(0));
            cout<<"threshold: "<<length<<endl;
        }
        else if (paralist->size() > 1)
        {
            cout<<"Illegal parameter list."<<endl;
            return 0;
        }

    }
    else{
        cout<< "Threshold not set: No pruning will be performed." << endl;
        return 1;
    }

    QString fileOpenName = QString(inlist->at(0));
    if(fileOpenName.isEmpty())
    {
        return 0;
    }
    NeuronTree nt;
    if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
    {
         nt = readSWC_file(fileOpenName);
    }
    bool prunned;


    // Prune
    if (func_name == tr("pruning_iterative"))
    {
        nt = prune_swc_iterative(nt, length, prunned);
    }
    else if (func_name == tr("pruning_simple"))
    {
        nt = prune_swc_simple(nt, length, prunned);
    }
    else{
        return 0;
    }

    //write new SWC to file
    QString fileSaveName = QString(outlist->at(0));
    if (!export_list2file(nt.listNeuron, fileSaveName, fileOpenName))
    {
        v3d_msg("fail to write the output swc file.");
        return 0;
    }
    return 1;

//    cout<<"==========Welcome to sort_swc function============="<<endl;
//    vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
//    vector<char*>* outlist = NULL;
//    vector<char*>* paralist = NULL;

//    double thres = VOID;
//    QList<NeuronSWC> neuron, result;
//    bool hasPara, hasOutput;
//    if (input.size()==1)
//    {
//        cout<<"No new parameter specified.\n";
//        hasPara = false;
//    }
//    else {
//        hasPara = true;
//        paralist = (vector<char*>*)(input.at(1).p);
//    }

//    if (inlist->size()!=1)
//    {
//        cout<<"You must specify 1 input file!"<<endl;
//        return 0;
//    }

//    if (output.size()==0){
//        cout<<"No output file specified.\n";
//        hasOutput = false;
//    }
//    else {
//        hasOutput = true;
//        if (output.size()>1)
//        {
//            cout<<"You have specified more than 1 output file.\n";
//            return 0;
//        }
//        outlist = (vector<char*>*)(output.at(0).p);
//    }

//    V3DLONG rootid = VOID;
//    if (hasPara)
//    {
//        if (paralist->size()==0)
//        {
//            cout<< "Threshold not set: All points will connected automatically." << endl;
//            cout<<"No root ID is specified: by default will use the first root in the file."  <<endl;
//            rootid = VOID;
//            thres = VOID;
//        }
//        else if (paralist->size() >= 1)
//        {
//            thres = atof(paralist->at(0));
//            cout<<"threshold: "<<thres<<endl;
//            if (paralist->size() ==2 )
//            {
//                rootid = atoi(paralist->at(1));
//                cout<<"root id: "<<rootid<<endl;
//            }
//            else if (paralist->size() >2)
//            {
//                cout<<"Illegal parameter list."<<endl;
//                return 0;
//            }
//        }

//    }
}

void MyComboBox::enterEvent(QEnterEvent *e)
{
    updateList();
    QComboBox::enterEvent(e);
}

void MyComboBox::updateList()
{
    if (!m_v3d)
        return;

    QString lastDisplayfile = currentText();

    QList <V3dR_MainWindow *> cur_list_3dviewer = m_v3d->getListAll3DViewers();

    QStringList items;
    int i;

    for (i=0; i<cur_list_3dviewer.count(); i++)
    {
         items << m_v3d->getImageName(cur_list_3dviewer[i]);
    }

    //update the list now
    clear();
    addItems(items);

    int curDisplayIndex=-1; //-1 for invalid index
    for (i=0; i<items.size(); i++)
        if (items[i]==lastDisplayfile)
        {
            curDisplayIndex = i;
            break;
        }

    if (curDisplayIndex>=0)
        setCurrentIndex(curDisplayIndex);

    //
    update();

    return;
}


controlPanel::controlPanel(V3DPluginCallback2 &_v3d, QWidget *parent) :
    QDialog(parent), m_v3d(_v3d)
{
    //potential bugs for the following two sentences
    list_triview = m_v3d.getImageWindowList();
    list_3dviewer = m_v3d.getListAll3DViewers();

    combo_surface = new MyComboBox(&m_v3d);
    combo_surface->updateList();

    label_surface = new QLabel(QObject::tr("Annotation window: "));
    QPushButton* ok     = new QPushButton("Prune");
    QPushButton* cancel = new QPushButton("Close");
    QPushButton* save = new QPushButton("Save annotation");


    gridLayout = new QGridLayout();
    gridLayout->addWidget(label_surface, 1,0,1,5);
    gridLayout->addWidget(combo_surface, 2,0,1,5);
    gridLayout->addWidget(ok, 4,0);
    gridLayout->addWidget(save, 4,2);
    gridLayout->addWidget(cancel,4,6);
    setLayout(gridLayout);
//    setWindowTitle(QString("Synchronize multiple 3D views"));


    connect(ok,     SIGNAL(clicked()), this, SLOT(_slot_prune()));
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(save, SIGNAL(clicked()), this, SLOT(_slot_save()));

}

controlPanel::~controlPanel()
{
//     if(panel){delete panel; panel=0;}

}

void controlPanel::reject()
{
    pruningControlPanel();

}

void controlPanel::_slot_prune()
{
    if(combo_surface->currentIndex()<0) return;
    list_3dviewer = m_v3d.getListAll3DViewers();
    surface_win = list_3dviewer[combo_surface->currentIndex()];
    QList<NeuronTree> *nt_list = m_v3d.getHandleNeuronTrees_Any3DViewer(surface_win);
    if(nt_list->size()>0)
    {
        NeuronTree nt = nt_list->at(0);
        LandmarkList *markers = m_v3d.getHandleLandmarkList_Any3DViewer(surface_win);
        if(markers->size() == 0)
        {
            v3d_msg("Please define markers for small branches pruning.");
            return;
        }
        int ID1 = 0;
        int ID2 = 0;

        for(V3DLONG i=0; i<nt.listNeuron.size();i++)
        {
            if(dist(nt.listNeuron.at(i),markers->at(0))<2.0)
                ID1 = i;
            if(dist(nt.listNeuron.at(i),markers->at(1))<2.0)
                ID2 = i;

            if(nt.listNeuron.at(i).pn < 0 && i!=0)
            {
                v3d_msg("Please sort your annotation first!");
                return;
            }
        }

        int start_ID,end_ID;
        if(ID1>ID2)
        {
            start_ID = ID2;
            end_ID = ID1;
        }else
        {
            start_ID = ID1;
            end_ID = ID2;
        }
        int ID_p = getParent(end_ID,nt);
        while(ID_p >= start_ID)
        {
            nt.listNeuron[ID_p].type = -1;
            ID_p = getParent(ID_p,nt);
        }

        QVector<QVector<V3DLONG> > childs;

        V3DLONG neuronNum = nt.listNeuron.size();
        childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
        V3DLONG *flag = new V3DLONG[neuronNum];

        for (V3DLONG i=0;i<neuronNum;i++)
        {
            flag[i] = 1;
            V3DLONG par = nt.listNeuron[i].pn;
            if (par<0)  continue;
            childs[nt.hashNeuron.value(par)].push_back(i);
        }

        QList<NeuronSWC> list = nt.listNeuron;
        for (int i=start_ID+1;i<end_ID-1;i++)
        {
            if (childs[i].size()>0)
            {
                for(int j=0; j<childs[i].size();j++)
                {
                    if(list[childs[i].at(j)].type != -1)
                    {
                        subtree(childs[i].at(j),childs,flag);
                    }
                }
            }
      }

       //NeutronTree structure
       NeuronTree nt_prunned;
       QList <NeuronSWC> listNeuron;
       QHash <int, int>  hashNeuron;
       listNeuron.clear();
       hashNeuron.clear();

       //set node

       NeuronSWC S;
       for (int i=0;i<list.size();i++)
       {
           if(flag[i] == 1)
           {
               NeuronSWC curr = list.at(i);
               S.n 	= curr.n;
               S.type 	= curr.type;
               if(curr.type == -1) S.type = 2;
               S.x 	= curr.x;
               S.y 	= curr.y;
               S.z 	= curr.z;
               S.r 	= curr.r;
               S.pn 	= curr.pn;
               listNeuron.append(S);
               hashNeuron.insert(S.n, listNeuron.size()-1);
           }

       }
       nt_prunned.n = -1;
       nt_prunned.on = true;
       nt_prunned.listNeuron = listNeuron;
       nt_prunned.hashNeuron = hashNeuron;

       nt_prunned.color.r = 0;
       nt_prunned.color.g = 0;
       nt_prunned.color.b = 0;
       nt_prunned.color.a = 0;

       nt_list->clear();
       nt_list->push_back(nt_prunned);
       m_v3d.update_NeuronBoundingBox(surface_win);
    }
}

void controlPanel::_slot_save()
{
    if(combo_surface->currentIndex()<0) return;
    list_3dviewer = m_v3d.getListAll3DViewers();
    surface_win = list_3dviewer[combo_surface->currentIndex()];
    QList<NeuronTree> *nt_list = m_v3d.getHandleNeuronTrees_Any3DViewer(surface_win);
    if(nt_list->size()>0)
    {
        NeuronTree nt = nt_list->at(0);
        QString fileDefaultName = QString("pruned.swc");
       //write new SWC to file
       QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
               fileDefaultName,
               QObject::tr("Supported file (*.swc)"
                   ";;Neuron structure	(*.swc)"
                   ));
       if (!export_list2file(nt.listNeuron,fileSaveName,fileSaveName))
       {
           v3d_msg("fail to write the output swc file.");
           return;
       }
    }
}




void subtree(int ID, QVector<QVector<V3DLONG> > childs,V3DLONG *flag)
{
    flag[ID] = -1;
    for(int i=0; i < childs[ID].size();i++)
    {
        subtree(childs[ID].at(i),childs,flag);
    }


}
