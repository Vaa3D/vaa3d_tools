/* neuron_connector_plugin.cpp
 * This plugin will connect the segments in swc file that meet the creterian. Only tips will be connected to other segments.
 * 2014-11-03 : by Hanbo Chen
 */
 
#include "v3d_message.h"
#include <vector>
#include <map>
#include "neuron_connector_plugin.h"
#include <iostream>

#define NTDIS(a,b) (((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))
#define NTDOT(a,b) ((a).x*(b).x+(a).y*(b).y+(a).z*(b).z)

using namespace std;

Q_EXPORT_PLUGIN2(neuron_connector, neuron_connector_swc);
 
QStringList neuron_connector_swc::menulist() const
{
	return QStringList() 
		<<tr("connect_neuron_SWC")
		<<tr("about");
}

QStringList neuron_connector_swc::funclist() const
{
	return QStringList()
        <<tr("connect_neuron_SWC")
		<<tr("help");
}

void neuron_connector_swc::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("connect_neuron_SWC"))
	{
        doconnect(callback,parent);
	}
	else
	{
		v3d_msg(tr("This plugin will connect the segments in swc file that meet the creterian. Only tips will be connected to other segments.. "
			"Developed by Hanbo Chen, 2014-11-03"));
	}
}

bool neuron_connector_swc::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("connect_neuron_SWC"))
    {
        //load input
        if(infiles.size()!=1 || outfiles.size()!=1)
        {
            qDebug("ERROR: please set input and output!");
            printHelp();
            return false;
        }
        //load neurons
        QString fname_input = ((vector<char*> *)(input.at(0).p))->at(0);
        QString fname_output = ((vector<char*> *)(output.at(0).p))->at(0);
        NeuronTree* nt = new NeuronTree();
        if (fname_input.toUpper().endsWith(".SWC") || fname_input.toUpper().endsWith(".ESWC")){
            *nt = readSWC_file(fname_input);
        }
        if(nt->listNeuron.size()<=0){
            qDebug()<<"failed to read SWC file: "<<fname_input;
            return false;
        }

        //get parameters
        double angthr=60, disthr=10, xscale=1, yscale=1, zscale=1;
        vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
        if(paras.size()>0){
            double tmp=atof(paras.at(0));
            if(tmp>0 && tmp<180)
                angthr=tmp;
            else
                cerr<<"error: wrong angular threshold: "<<tmp<<"; use default value "<<angthr<<endl;
        }
        if(paras.size()>1){
            double tmp=atof(paras.at(1));
            if(tmp>=0)
                disthr=tmp;
            else
                cerr<<"error: wrong distance threshold value: "<<tmp<<"; use default value "<<disthr<<endl;
        }
        if(paras.size()>2){
            double tmp=atof(paras.at(2));
            zscale=tmp;
        }
        if(paras.size()>3){
            double tmp=atof(paras.at(3));
            xscale = tmp;
        }
        if(paras.size()>4){
            double tmp=atof(paras.at(4));
            yscale = tmp;
        }
        cout<<"angthr="<<angthr<<"; disthr="<<disthr<<"; xscale="<<xscale<<"; yscale="<<yscale<<"; zscale="<<zscale<<endl;

        //do connect
        QList<NeuronSWC> newNeuron;
        connectall(nt, newNeuron, xscale, yscale, zscale, angthr, disthr);

        qDebug()<<"output result";
        //output result
        if(!export_list2file(newNeuron, fname_output)){
            qDebug()<<"error: Cannot open file "<<fname_output<<" for writing!"<<endl;
        }
    }
    else if (func_name == tr("help"))
	{
        printHelp();
	}
	else return false;

	return true;
}

void printHelp()
{
    cout<<"\nUsage: v3d -x neuron_connector -f connect_neuron_SWC -i <input.swc> -o <output.swc> "
          <<"-p <angular threshold=60> <distance threshold=10> <zscale=1> <xscale=1> <yscale=1>"<<endl;
    cout<<"\n";
}

void neuron_connector_swc::doconnect(V3DPluginCallback2 &callback, QWidget *parent)
{
    neuron_connector_dialog connectfunc;
    connectfunc.exec();
}

neuron_connector_dialog::neuron_connector_dialog()
{
    creat();
}

void neuron_connector_dialog::creat()
{
    gridLayout = new QGridLayout();

    //input zone
    label_load0 = new QLabel(QObject::tr("Input SWC file:"));
    gridLayout->addWidget(label_load0,0,0,1,7);
    edit_load0 = new QLineEdit();
    edit_load0->setText(fname_input); edit_load0->setReadOnly(true);
    gridLayout->addWidget(edit_load0,1,0,1,6);
    btn_load0 = new QPushButton("...");
    gridLayout->addWidget(btn_load0,1,6,1,1);

    label_load1 = new QLabel(QObject::tr("Output SWC file:"));
    gridLayout->addWidget(label_load1,2,0,1,7);
    edit_load1 = new QLineEdit();
    edit_load1->setText(fname_output); edit_load1->setReadOnly(true);
    gridLayout->addWidget(edit_load1,3,0,1,6);
    btn_load1 = new QPushButton("...");
    gridLayout->addWidget(btn_load1,3,6,1,1);

    connect(btn_load0, SIGNAL(clicked()), this, SLOT(load0()));
    connect(btn_load1, SIGNAL(clicked()), this, SLOT(load1()));

    //parameter zone
    QFrame *line_1 = new QFrame();
    line_1->setFrameShape(QFrame::HLine);
    line_1->setFrameShadow(QFrame::Sunken);
    gridLayout->addWidget(line_1,8,0,1,7);

    spin_xscale = new QDoubleSpinBox();
    spin_xscale->setRange(1,100000); spin_xscale->setValue(1);
    spin_yscale = new QDoubleSpinBox();
    spin_yscale->setRange(1,100000); spin_yscale->setValue(1);
    spin_zscale = new QDoubleSpinBox();
    spin_zscale->setRange(1,100000); spin_zscale->setValue(1);
    spin_ang = new QDoubleSpinBox();
    spin_ang->setRange(0,180); spin_ang->setValue(60);
    spin_dis = new QDoubleSpinBox();
    spin_dis->setRange(0,100000); spin_dis->setValue(10);
    QLabel* label_0 = new QLabel("sacles: ");
    gridLayout->addWidget(label_0,9,0,1,1);
    QLabel* label_1 = new QLabel("X: ");
    gridLayout->addWidget(label_1,9,1,1,1);
    gridLayout->addWidget(spin_xscale,9,2,1,1);
    QLabel* label_2 = new QLabel("Y: ");
    gridLayout->addWidget(label_2,9,3,1,1);
    gridLayout->addWidget(spin_yscale,9,4,1,1);
    QLabel* label_3 = new QLabel("Z: ");
    gridLayout->addWidget(label_3,9,5,1,1);
    gridLayout->addWidget(spin_zscale,9,6,1,1);
    QLabel* label_4 = new QLabel("distance threshold: ");
    gridLayout->addWidget(label_4,10,0,1,6);
    gridLayout->addWidget(spin_dis,10,6,1,1);
    QLabel* label_5 = new QLabel("tip-tip match angular threshold: ");
    gridLayout->addWidget(label_5,11,0,1,6);
    gridLayout->addWidget(spin_ang,11,6,1,1);

    //operation zone
    QFrame *line_2 = new QFrame();
    line_2->setFrameShape(QFrame::HLine);
    line_2->setFrameShadow(QFrame::Sunken);
    gridLayout->addWidget(line_2,15,0,1,7);
    btn_run = new QPushButton("Connect");
    connect(btn_run,     SIGNAL(clicked()), this, SLOT(run()));
    gridLayout->addWidget(btn_run,16,4,1,1);
    btn_quit = new QPushButton("Quit");
    connect(btn_quit,     SIGNAL(clicked()), this, SLOT(reject()));
    gridLayout->addWidget(btn_quit,16,6,1,1);

    setLayout(gridLayout);
}

bool neuron_connector_dialog::load0()
{
    QString fileOpenName;
    fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
            "",
            QObject::tr("Supported file (*.swc *.eswc)"
                ";;Neuron structure	(*.swc)"
                ";;Extended neuron structure (*.eswc)"
                ));
    if(fileOpenName.isEmpty())
        return false;
    nt = new NeuronTree();
    if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
    {
        *nt = readSWC_file(fileOpenName);
    }
    if(nt->listNeuron.size()<=0){
        v3d_msg("failed to read SWC file: "+fileOpenName);
        btn_run->setEnabled(false);
        return false;
    }
    fname_input = fileOpenName;
    edit_load0->setText(fname_input);

    if(fname_input.length() * fname_output.length() != 0){
        btn_run->setEnabled(true);
        edit_load1->setText(fname_output);
    }

    return true;
}

bool neuron_connector_dialog::load1()
{
    QString fileOpenName;
    fileOpenName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
                                                fname_input+"_connected.swc",
            QObject::tr("Supported file (*.swc *.eswc)"
                ";;Neuron structure	(*.swc)"
                ";;Extended neuron structure (*.eswc)"
                ));
    if(fileOpenName.isEmpty()){
        btn_run->setEnabled(false);
        return false;
    }

    fname_output = fileOpenName;
    edit_load1->setText(fname_output);

    if(fname_input.length() * fname_output.length() != 0){
        btn_run->setEnabled(true);
    }

    return true;
}

void neuron_connector_dialog::run()
{
    qDebug()<<"load and initialize";
    //load parameters
    double xscale=spin_xscale->value();
    double yscale=spin_yscale->value();
    double zscale=spin_zscale->value();
    double angThr=cos((180-spin_ang->value())/180*M_PI);
    double disThr=spin_dis->value()*spin_dis->value();

    QList<NeuronSWC> newNeuron;
    connectall(nt, newNeuron, xscale, yscale, zscale, angThr, disThr);

    qDebug()<<"output result";
    //output result
    if(!export_list2file(newNeuron, fname_output)){
        v3d_msg("Cannot open file " + fname_output + " for writing!");
    }

    this->accept();
}

void connectall(NeuronTree* nt, QList<NeuronSWC>& newNeuron, double xscale, double yscale, double zscale, double angThr, double disThr)
{
    newNeuron.clear();
    //rescale neurons
    QList<XYZ> scaledXYZ;
    for(V3DLONG i=0; i<nt->listNeuron.size(); i++){
        XYZ S;
        S.x = nt->listNeuron.at(i).x*xscale;
        S.y = nt->listNeuron.at(i).y*yscale;
        S.z = nt->listNeuron.at(i).z*zscale;
        scaledXYZ.append(S);
    }

    qDebug()<<"search for components and tips";
    //initialize tree components and get all tips
    QList<V3DLONG> cand;
    QList<XYZ> canddir;
    QVector<int> childNum(nt->listNeuron.size(), 0);
    QVector<int> connNum(nt->listNeuron.size(), 0);
    QList<V3DLONG> components;
    QList<V3DLONG> pList;
    V3DLONG curid=0;
    for(V3DLONG i=0; i<nt->listNeuron.size(); i++){
        if(nt->listNeuron.at(i).pn<0){
            connNum[i]--; //root that only have 1 child will also be a dead end
            components.append(curid); curid++;
            pList.append(-1);
        }else{
            V3DLONG pid = nt->hashNeuron.value(nt->listNeuron.at(i).pn);
            childNum[pid]++;
            connNum[pid]++;
            components.append(-1);
            pList.append(pid);
        }
    }
    qDebug()<<"components searching";
    //connected component
    for(V3DLONG cid=0; cid<curid; cid++){
        QStack<V3DLONG> pstack;
        V3DLONG chid;
        if(!components.contains(cid)) //should not happen, just in case
            continue;
        if(components.indexOf(cid)!=components.lastIndexOf(cid)) //should not happen
            qDebug("unexpected multiple tree root, please check the code: neuron_stitch_func.cpp");
        //recursively search for child and mark them as the same component
        pstack.push(components.indexOf(cid));
        while(!pstack.isEmpty()){
            V3DLONG pid=pstack.pop();
            chid = -1;
            chid = pList.indexOf(pid,chid+1);
            while(chid>=0){
                pstack.push(chid);
                components[chid]=cid;
                chid=pList.indexOf(pid,chid+1);
            }
        }
    }
    qDebug()<<"tips searching";
    //get tips
    for(V3DLONG i=0; i<childNum.size(); i++){
        if(connNum.at(i)<1){
            cand.append(i);
            //get direction
            V3DLONG id=i;
            XYZ tmpdir(0,0,0);
            if(childNum[id]==1){ //single child root
                V3DLONG sid = pList.indexOf(id);
                if(sid>=0){
                    tmpdir.x += scaledXYZ.at(id).x-scaledXYZ.at(sid).x;
                    tmpdir.y += scaledXYZ.at(id).y-scaledXYZ.at(sid).y;
                    tmpdir.z += scaledXYZ.at(id).z-scaledXYZ.at(sid).z;
                }
            }else{ //tips
                V3DLONG sid = pList[id];
                if(sid>=0){
                    tmpdir.x += scaledXYZ.at(id).x-scaledXYZ.at(sid).x;
                    tmpdir.y += scaledXYZ.at(id).y-scaledXYZ.at(sid).y;
                    tmpdir.z += scaledXYZ.at(id).z-scaledXYZ.at(sid).z;
                }
            }
            double tmpNorm = sqrt(tmpdir.x*tmpdir.x+tmpdir.y*tmpdir.y+tmpdir.z*tmpdir.z);
            if(tmpNorm>1e-16){
                tmpdir.x/=tmpNorm;
                tmpdir.y/=tmpNorm;
                tmpdir.z/=tmpNorm;
            }
            canddir.append(tmpdir);
        }
    }

    qDebug()<<connNum.size()<<":"<<childNum.size()<<":"<<cand.size();

    qDebug()<<"match tips";
    //match tips
    multimap<double, QVector<V3DLONG> > connMap;
    for(V3DLONG tid=0; tid<cand.size(); tid++){
        V3DLONG tidx=cand.at(tid);
        for(V3DLONG cid=0; cid<curid; cid++){
            if(cid==components.at(cand[tid])) continue;
            double mvdis=disThr, mtdis=disThr;
            V3DLONG mvid=-1, mtid=-1;
            V3DLONG id=components.indexOf(cid);
            while(id>=0){
                double dis=NTDIS(scaledXYZ.at(tidx),scaledXYZ.at(id));
                if(dis<mvdis){
                    mvdis=dis;
                    mvid=id;
                }
                if(dis<mtdis){
                    if(connNum.at(id)<1){//tips
                        V3DLONG tmpid=cand.indexOf(id);
                        if(tmpid<0){//should not happen, just in case
                            qDebug()<<"unexpected error: cannot locate dead end in candidate list, please check code."<<tid<<":"<<cid<<":"<<id;
                            id=components.indexOf(cid, id+1);
                            continue;
                        }
                        if(NTDOT(canddir.at(tid),canddir.at(tmpid))<angThr){
                            mtdis=dis;
                            mtid=id;
                        }
                    }
                }
                id=components.indexOf(cid, id+1);
            }
            if(mvid>=0){
                QVector<V3DLONG> tmp;
                tmp.append(tidx); tmp.append(mvid);
                connMap.insert(pair<double, QVector<V3DLONG> >(mvdis+disThr,tmp));
            }
            if(mtid>=0){
                QVector<V3DLONG> tmp;
                tmp.append(tidx); tmp.append(mtid);
                connMap.insert(pair<double, QVector<V3DLONG> >(mtdis,tmp));
            }
        }
    }

    qDebug()<<"connecting tips";
    //find the best solution for connecting tips
    QMap<V3DLONG, QVector<V3DLONG> > connectPairs;
    for(multimap<double, QVector<V3DLONG> >::iterator iter=connMap.begin(); iter!=connMap.end(); iter++){
        if(components.at(iter->second.at(0))==components.at(iter->second.at(1))) //already connected
            continue;
        if(connectPairs.contains(iter->second.at(0))){
            connectPairs[iter->second.at(0)].append(iter->second.at(1));
        }else{
            QVector<V3DLONG> tmp; tmp.append(iter->second.at(1));
            connectPairs.insert(iter->second.at(0),tmp);
        }
        if(connectPairs.contains(iter->second.at(1))){
            connectPairs[iter->second.at(1)].append(iter->second.at(0));
        }else{
            QVector<V3DLONG> tmp; tmp.append(iter->second.at(0));
            connectPairs.insert(iter->second.at(1),tmp);
        }
        V3DLONG cid_0=components.at(iter->second.at(0));
        V3DLONG cid_1=components.at(iter->second.at(1));
        V3DLONG tmpid=components.indexOf(cid_1);
        while(tmpid>=0){
            components[tmpid]=cid_0;
            tmpid=components.indexOf(cid_1,tmpid+1);
        }
    }

    qDebug()<<"reconstruct neuron tree";
    //reconstruct tree
    QVector<V3DLONG> newid(nt->listNeuron.size(), -1);
    QVector<V3DLONG> newpn(nt->listNeuron.size(), -1); //id starts from 1, -1: not touched, 0: touched but overlap with parent
    curid=1;
    for(V3DLONG i=0; i<nt->listNeuron.size(); i++){
        if(newid[i]>0) continue;
        QQueue<V3DLONG> pqueue; pqueue.clear();
        pqueue.enqueue(i);
        newid[i]=curid++;
        while(!pqueue.isEmpty()){
            //add current node to the listNeuron
            V3DLONG oid=pqueue.dequeue();

            if(newid[oid]>0){
                NeuronSWC tmpNeuron;
                tmpNeuron.n = newid[oid];
                tmpNeuron.x = nt->listNeuron.at(oid).x;
                tmpNeuron.y = nt->listNeuron.at(oid).y;
                tmpNeuron.z = nt->listNeuron.at(oid).z;
                tmpNeuron.type = nt->listNeuron.at(oid).type;
                tmpNeuron.r = nt->listNeuron.at(oid).r;
                tmpNeuron.pn = newpn.at(oid);
                newNeuron.append(tmpNeuron);
            }

            //add current node's children/parent/new-neighbor to the stack
            //parent
            if(nt->listNeuron.at(oid).pn>=0){
                V3DLONG opid = nt->hashNeuron.value(nt->listNeuron.at(oid).pn);
                if(newid.at(opid)<0){
                    pqueue.enqueue(opid);
                    newpn[opid]=newid[oid];
                    newid[opid]=curid++;
                }
            }
            //child
            V3DLONG tmpid=pList.indexOf(oid);
            while(tmpid>=0){
                if(newid.at(tmpid)<0){
                    pqueue.enqueue(tmpid);
                    newpn[tmpid]=newid[oid];
                    newid[tmpid]=curid++;
                }
                tmpid=pList.indexOf(oid,tmpid+1);
            }
            //new-neighbor
            if(connectPairs.contains(oid)){
                for(V3DLONG j=0; j<connectPairs[oid].size(); j++){
                    V3DLONG onid=connectPairs[oid].at(j);
                    if(newid.at(onid)<0){
                        pqueue.enqueue(onid);
                        newpn[onid]=newid[oid];
                        newid[onid]=curid++;
                    }
                }
            }
        }
    }
}

bool export_list2file(const QList<NeuronSWC>& lN, QString fileSaveName)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;
    QTextStream myfile(&file);
    myfile<<"# generated by Vaa3D Plugin resample_swc"<<endl;
    myfile<<"# id,type,x,y,z,r,pid"<<endl;
    for (V3DLONG i=0;i<lN.size();i++)
        myfile << lN.at(i).n <<" " << lN.at(i).type << " "<< lN.at(i).x <<" "<<lN.at(i).y << " "<< lN.at(i).z << " "<< lN.at(i).r << " " <<lN.at(i).pn << "\n";

    file.close();
    return true;
}


