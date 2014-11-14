/* neuron_extractor_plugin.cpp
 * This plugin will extract the neurons selected by markers. The structure connected to each markers will be extracted and saved seperately.
 * 2014-11-2 : by Hanbo Chen
 */
 
#include "v3d_message.h"
#include <vector>
#include "neuron_extractor_plugin.h"
#include <basic_surf_objs.h>
#include <QDialog>
#include <iostream>

#define MY_MATCH_DIS 10
#define NTDIS(a,b) (((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))

using namespace std;
Q_EXPORT_PLUGIN2(neuron_extractor, neuron_extractor_by_marker);
 
QStringList neuron_extractor_by_marker::menulist() const
{
	return QStringList() 
		<<tr("extract_by_markers")
        <<tr("extract_spines")
        <<tr("label_components")
		<<tr("about");
}

QStringList neuron_extractor_by_marker::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void neuron_extractor_by_marker::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("extract_by_markers"))
	{
        //select the window to operate
        QList <V3dR_MainWindow *> allWindowList = callback.getListAll3DViewers();
        QList <V3dR_MainWindow *> selectWindowList;
        V3dR_MainWindow * v3dwin;
        QList<NeuronTree> * ntTreeList;
        LandmarkList* mList;
        int winid;
        qDebug("search for 3D windows");
        for (V3DLONG i=0;i<allWindowList.size();i++)
        {
            ntTreeList = callback.getHandleNeuronTrees_Any3DViewer(allWindowList[i]);
            mList = callback.getHandleLandmarkList_Any3DViewer(allWindowList[i]);
            if(ntTreeList->size()>0 && mList->size()>0)
                selectWindowList.append(allWindowList[i]);
        }
        qDebug("match and select 3D windows");
        if(selectWindowList.size()<1){
            v3d_msg("Cannot find 3D view with SWC file and markers. Please load the SWC file you want to extract in the 3D view and define markers on the structure you want to extract.");
            return;
        }else if(selectWindowList.size()>1){
            //pop up a window to select

            QStringList items;
            for(int i=0; i<selectWindowList.size(); i++){
                items.append(callback.getImageName(selectWindowList[i]));
            }
            bool ok;
            QString selectitem = QInputDialog::getItem(parent, QString::fromUtf8("Neuron Stitcher"), QString::fromUtf8("Select A Window to Operate"), items, 0, false, &ok);
            if(!ok) return;
            for(int i=0; i<selectWindowList.size(); i++){
                if(selectitem==callback.getImageName(selectWindowList[i]))
                {
                    winid=i;
                    break;
                }
            }
        }else{
            winid=0;
        }
        v3dwin = selectWindowList[winid];
        ntTreeList = callback.getHandleNeuronTrees_Any3DViewer(v3dwin);
        mList = callback.getHandleLandmarkList_Any3DViewer(v3dwin);

        extract(ntTreeList, mList);
	}
    else if (menu_name == tr("extract_spines"))
    {
        //input
        QString fileOpenName;
        fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
                "",
                QObject::tr("Supported file (*.swc *.eswc)"
                    ";;Neuron structure	(*.swc)"
                    ";;Extended neuron structure (*.eswc)"
                    ));
        if(fileOpenName.isEmpty())
            return;
        NeuronTree* nt = new NeuronTree();
        if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
        {
            *nt = readSWC_file(fileOpenName);
        }
        if(nt->listNeuron.size()<=0){
            v3d_msg("failed to read SWC file: "+fileOpenName);
            return;
        }

        bool ok;
        int length = QInputDialog::getInt(0, "Spine Extractor", "Please specify the maximum number of points in a spine",3,0,2147483647,1,&ok);
        if(!ok)
            return;

        QList<NeuronSWC> spines;

        //get spine
        extract_spine(*nt, spines, length);

        //output
        QString fileSaveName;
        fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
                "",
                QObject::tr("Supported file (*.swc *.eswc)"
                    ";;Neuron structure	(*.swc)"
                    ";;Extended neuron structure (*.eswc)"
                    ));
        if(fileSaveName.isEmpty())
            return;

        if(!export_list2file(spines,fileSaveName, fileOpenName)){
            v3d_msg("Cannot open file " + fileSaveName + " for writing!");
        }
    }else if(menu_name == tr("label_components")){
        //input
        QString fileOpenName;
        fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
                "",
                QObject::tr("Supported file (*.swc *.eswc)"
                    ";;Neuron structure	(*.swc)"
                    ";;Extended neuron structure (*.eswc)"
                    ));
        if(fileOpenName.isEmpty())
            return;
        NeuronTree nt;
        if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
        {
            nt = readSWC_file(fileOpenName);
        }
        if(nt.listNeuron.size()<=0){
            v3d_msg("failed to read SWC file: "+fileOpenName);
            return;
        }

        assignComponents(nt);

        //output
        QString fileSaveName=fileOpenName + "_component.swc";
        fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
                fileSaveName.toStdString().c_str(),
                QObject::tr("Supported file (*.swc *.eswc)"
                    ";;Neuron structure	(*.swc)"
                    ";;Extended neuron structure (*.eswc)"
                    ));
        if(fileSaveName.isEmpty())
            return;

        if(!export_list2file(nt.listNeuron,fileSaveName, fileOpenName)){
            v3d_msg("Cannot open file " + fileSaveName + " for writing!");
        }
    }
	else
	{
		v3d_msg(tr("This plugin will extract the neurons selected by markers. The structure connected to each markers will be extracted and saved seperately.. "
			"Developed by Hanbo Chen, 2014-11-2"));
	}
}

bool neuron_extractor_by_marker::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("func1"))
    {
        v3d_msg("To be implemented.");
    }
    else if (func_name == tr("func2"))
    {
        v3d_msg("To be implemented.");
    }
    else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

void neuron_extractor_by_marker::extract_spine(const NeuronTree& nt, QList<NeuronSWC>& spine, int length)
{
    QVector<int> childNum(nt.listNeuron.size(), 0);
    QVector<int> connNum(nt.listNeuron.size(), 0);
    QList<int> vertexType; //-1/-2: root/child tips; 1: path; 2: branch
    QList<int> pList; //the parent index of each node
    spine.clear();
    for(V3DLONG i=0; i<nt.listNeuron.size(); i++){
        if(nt.listNeuron.at(i).pn<0){
            connNum[i]--; //root that only have 1 clide will also be a dead end
            pList.append(-1);
        }
        else{
            int pid = nt.hashNeuron.value(nt.listNeuron.at(i).pn);
            childNum[pid]++;
            connNum[pid]++;
            pList.append(pid);
        }
    }
    for(V3DLONG i=0; i<nt.listNeuron.size(); i++){
        int type=0;
        if(connNum.at(i)==0){ //terminal
            if(childNum.at(i)>0){ //should be 1, single child root
                type=-1;
            }else{ //tip
                type=-2;
            }
        }else{
            if(childNum.at(i)==1){ //path
                type=1;
            }else{ //branch
                type=2;
            }
        }
        vertexType.append(type);
    }
    V3DLONG idx=vertexType.indexOf(-2);
    V3DLONG curtid=0;
    while(idx>=0){
        int count = 0;
        int pid=idx;
        do{
            count++;
            pid=pList.at(pid);
        }while(pid>=0 && count<=length && vertexType.at(pid)==1);


        if(count<length && pid >= 0){ //is spine
            pid=idx;
            do{
                NeuronSWC S;
                S.x = nt.listNeuron.at(pid).x;
                S.y = nt.listNeuron.at(pid).y;
                S.z = nt.listNeuron.at(pid).z;
                S.n = spine.size()+1;
                S.type=curtid;
                if(pid==idx){
                    S.pn=-1;
                }else{
                    S.pn=spine.size();
                }
                spine.append(S);
                pid=pList.at(pid);
            }while(pid>=0 &&  vertexType.at(pid)==1);
            curtid++;
        }


        idx=vertexType.indexOf(-2,idx+1);
    }
    idx=vertexType.indexOf(-1);
    while(idx>=0){
        int count = 0;
        int pid=idx;
        do{
            count++;
            pid=pList.indexOf(pid);
        }while(pid>=0 && count<=length && vertexType.at(pid)==1);
        if(count<length && pid >= 0){ //is spine
            pid=idx;
            do{
                NeuronSWC S;
                S.x = nt.listNeuron.at(pid).x;
                S.y = nt.listNeuron.at(pid).y;
                S.z = nt.listNeuron.at(pid).z;
                S.n = spine.size()+1;
                S.type=curtid;
                if(pid==idx){
                    S.pn=-1;
                }else{
                    S.pn=spine.size();
                }
                spine.append(S);
                pid=pList.indexOf(pid);
            }while(pid>=0 &&  vertexType.at(pid)==1);
            curtid++;
        }
        idx=vertexType.indexOf(-1,idx+1);
    }
}

void neuron_extractor_by_marker::extract(QList<NeuronTree> *ntList, LandmarkList* mList)
{
    if(mList->size()<=0){
        v3d_msg("No markers found. Please define a marker first. The program will extract structure connected to each marker.");
        return;
    }

    if(ntList->size()<=0){
        v3d_msg("No neuron tree identified. Please load SWC file first.");
        return;
    }

    //connected component
    QVector<QList<int> > allParents;
    for(int nid=0; nid<ntList->size(); nid++){
        QList<int> pList;
        pList.clear();
        for(V3DLONG i=0; i<ntList->at(nid).listNeuron.size(); i++){
            if(ntList->at(nid).listNeuron.at(i).pn<0){
                pList.append(-1);
            }
            else{
                int pid = ntList->at(nid).hashNeuron.value(ntList->at(nid).listNeuron.at(i).pn);
                pList.append(pid);
            }
        }
        allParents.append(pList);
    }

    QVector<QList<NeuronSWC> > allTrees;

    //find the components corresponding to each marker and extract neuron tree
    int treecount=0;
    for(int i=0; i<mList->size(); i++){
        double mdis=MY_MATCH_DIS;
        int nid=-1;
        int pid=0;
        //qDebug()<<mList->at(i).x<<":"<<mList->at(i).y<<":"<<mList->at(i).z<<endl;
        for(int j=0; j<ntList->size(); j++){
            for(int k=0; k<ntList->at(j).listNeuron.size(); k++){
                double dis=NTDIS(ntList->at(j).listNeuron[k],mList->at(i));
                if(dis<mdis){
                    mdis=dis;
                    nid=j;
                    pid=k;
                }
            }
        }
        QList<NeuronSWC> tree_tmp;
        if(nid>=0){
            treecount++;
            int curid=0;
            QList<NeuronSWC>* listNeuron=(QList<NeuronSWC>*)&(ntList->at(nid).listNeuron);
            QList<int>* pList = (QList<int>*)&(allParents.at(nid));
            QVector<int> newid(listNeuron->size(), -1);
            QVector<int> newpn(listNeuron->size(), -1);
            QQueue<int> pqueue; pqueue.clear();
            pqueue.enqueue(pid);
            newid[pid]=curid++;
            while(!pqueue.isEmpty()){
                //add current node to the listNeuron
                int oid=pqueue.dequeue();

                NeuronSWC tmpNeuron;
                tmpNeuron.n = newid[oid];
                tmpNeuron.x = listNeuron->at(oid).x;
                tmpNeuron.y = listNeuron->at(oid).y;
                tmpNeuron.z = listNeuron->at(oid).z;
                tmpNeuron.type = listNeuron->at(oid).type;
                tmpNeuron.r = listNeuron->at(oid).r;
                tmpNeuron.pn = newpn.at(oid);
                tree_tmp.append(tmpNeuron);

                //add current node's children/parent to the stack
                if(listNeuron->at(oid).pn>=0){
                    int opid = ntList->at(nid).hashNeuron.value(listNeuron->at(oid).pn);
                    if(newid.at(opid)<0){
                        pqueue.enqueue(opid);
                        newpn[opid]=newid[oid];
                        newid[opid]=curid++;
                    }
                }
                int tmpid=pList->indexOf(oid);
                while(tmpid>=0){
                    if(newid.at(tmpid)<0){
                        pqueue.enqueue(tmpid);
                        newpn[tmpid]=newid[oid];
                        newid[tmpid]=curid++;
                    }
                    tmpid=pList->indexOf(oid,tmpid+1);
                }
            }
        }
        allTrees.append(tree_tmp);
    }

    //output
    v3d_msg("found "+QString::number(treecount)+" from "+
            QString::number(mList->size()) +" markers");
    QString folder_output = QFileDialog::getExistingDirectory(0, "Select Output Folder","");
    if(folder_output.isEmpty())
        return;
    bool ok;
    QString fname_base = QInputDialog::getText(0, "Output Prefix","Assign a prefix for all outputs",QLineEdit::Normal,"",&ok);
    if(!ok)
        return;
    QString fname_output = QDir(folder_output).filePath(fname_base+"_");
    for(int i=0; i<allTrees.size(); i++){
        if(allTrees.at(i).size()<=0){
            continue;
        }
        QString fname_neuron = fname_output + QString::number(i+1) + ".swc";
        if(!export_list2file(allTrees[i],fname_neuron, "")){
            v3d_msg("Cannot open file " + fname_neuron + " for writing!");
        }
    }
}

void neuron_extractor_by_marker::assignComponents(NeuronTree& nt)
{
    QList<int> components;
    QList<int> pList;
    int curid=0;
    //connected component
    for(V3DLONG i=0; i<nt.listNeuron.size(); i++){
        if(nt.listNeuron.at(i).pn<0){
            components.append(curid); curid++;
            pList.append(-1);
        }
        else{
            int pid = nt.hashNeuron.value(nt.listNeuron.at(i).pn);
            components.append(-1);
            pList.append(pid);
        }
    }
    //connected component
    for(V3DLONG cid=0; cid<curid; cid++){
        QStack<int> pstack;
        int chid, size = 0;
        if(!components.contains(cid)) //should not happen, just in case
            continue;
        if(components.indexOf(cid)!=components.lastIndexOf(cid)) //should not happen
            qDebug("unexpected multiple tree root, please check the code: neuron_stitch_func.cpp");
        //recursively search for child and mark them as the same component
        pstack.push(components.indexOf(cid));
        size++;
        while(!pstack.isEmpty()){
            int pid=pstack.pop();
            chid = -1;
            chid = pList.indexOf(pid,chid+1);
            while(chid>=0){
                pstack.push(chid);
                components[chid]=cid;
                chid=pList.indexOf(pid,chid+1);
                size++;
            }
        }
    }
    //update type with component
    for(V3DLONG i=0; i<nt.listNeuron.size(); i++){
        nt.listNeuron[i].type=components.at(i);
    }
}

bool export_list2file(const QList<NeuronSWC>& lN, QString fileSaveName, QString oldFileName)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;
    QTextStream myfile(&file);
    myfile<<"# generated by Vaa3D Plugin neuron_extractor"<<endl;
    myfile<<"# from file: "<<oldFileName<<endl;
    myfile<<"# id,type,x,y,z,r,pid"<<endl;
    for (V3DLONG i=0;i<lN.size();i++)
        myfile << lN.at(i).n <<" " << lN.at(i).type << " "<< lN.at(i).x <<" "<<lN.at(i).y << " "<< lN.at(i).z << " "<< lN.at(i).r << " " <<lN.at(i).pn << "\n";

    file.close();
    return true;
}

