#include "assemble_neuron_live_dialog.h"

#include <map>

using namespace std;

assemble_neuron_live_dialog::assemble_neuron_live_dialog(V3DPluginCallback2 * cb, QList<NeuronTree> &ntList, Image4DSimple * p_img4d_in, QWidget *parent) :
    QDialog(parent)
{
    callback = cb;

    p_img4d = p_img4d_in;

    noffset=0;
    coffset=0;
    if(ntList.size()>0){
        initNeuron(ntList);
    }else{
        v3d_msg("Empty neuron file. Nothing to assemble.");
        return;
    }

    creat(parent);

    updateDisplay();
}

void assemble_neuron_live_dialog::creat(QWidget *parent)
{
    QGridLayout * layout = new QGridLayout(this);

    btn_link = new QPushButton("search connections");
    btn_loop = new QPushButton("search loops");
    layout->addWidget(btn_link,1,0,1,1);
    layout->addWidget(btn_loop,1,1,1,1);
    cb_color = new QComboBox();
    cb_color->addItem("color by type");
    cb_color->addItem("color by segment");
    cb_color->setCurrentIndex(1);
    layout->addWidget(cb_color,2,1,1,1);

    connect(btn_link, SIGNAL(clicked()), this, SLOT(searchPair()));
    connect(cb_color, SIGNAL(currentIndexChanged(int)), this, SLOT(setColor(int)));

    tab = new QTabWidget(this);
    //neuron connector
    QDialog * dialog_conn = new QDialog(tab);
    QGridLayout * layout_conn = new QGridLayout();
    list_link = new QListWidget();
    btn_connect = new QPushButton("connect select pair");
    btn_manuallink = new QPushButton("add connection pair");
    btn_deletelink = new QPushButton("delete connection pair");
    check_loop = new QCheckBox("Check loop before connect"); check_loop->setChecked(true);
    layout_conn->addWidget(list_link,0,0,6,2);
    layout_conn->addWidget(btn_connect,0,2,1,1);
    layout_conn->addWidget(btn_manuallink,1,2,1,1);
    layout_conn->addWidget(btn_deletelink,2,2,1,1);
    layout_conn->addWidget(check_loop,3,2,1,1);
    dialog_conn->setLayout(layout_conn);
    tab->addTab(dialog_conn,tr("connect"));

    connect(list_link, SIGNAL(currentRowChanged(int)), this, SLOT(highlightPair()));
    connect(btn_connect, SIGNAL(clicked()), this, SLOT(connectPair()));
    connect(btn_manuallink,SIGNAL(clicked()),this,SLOT(pairMarker()));
    connect(btn_deletelink,SIGNAL(clicked()),this,SLOT(delPair()));

    //neuron breaker
    QDialog * dialog_break = new QDialog(tab);
    QGridLayout * layout_break = new QGridLayout();
    cb_loop = new QComboBox();
    btn_colorloop = new QPushButton("highlight the loop");
    list_marker = new QListWidget();
    btn_break = new QPushButton("break at select");
    btn_syncmarker = new QPushButton("update markers");
    layout_break->addWidget(list_marker,0,0,6,2);
    layout_break->addWidget(btn_break,0,2,1,1);
    layout_break->addWidget(btn_syncmarker,1,2,1,1);
    layout_break->addWidget(cb_loop, 3,2,1,1);
    layout_break->addWidget(btn_colorloop, 4,2,1,1);
    dialog_break->setLayout(layout_break);
    tab->addTab(dialog_break,tr("break"));

    connect(btn_syncmarker,SIGNAL(clicked()),this,SLOT(syncMarker()));
    connect(list_marker, SIGNAL(currentRowChanged(int)), this, SLOT(highlightMarker()));

    layout->addWidget(tab,3,0,1,2);

    //save, quit
    btn_save = new QPushButton("sort and save");
    btn_quit = new QPushButton("quit");
    layout->addWidget(btn_save,6,0,1,1);
    layout->addWidget(btn_quit,6,1,1,1);

    setLayout(layout);
}

void assemble_neuron_live_dialog::initNeuron(QList<NeuronTree> &ntList)
{
    V3DLONG nmax=0;
    noffset=0;
    QMultiHash<NOI*,V3DLONG> parents;
    //init nodes
    nodes.clear();
    for(int i=0; i<ntList.size(); i++){
        for(int j=0; j<ntList.at(i).listNeuron.size(); j++){
            V3DLONG n=ntList.at(i).listNeuron.at(j).n+noffset;
            NOI* node;
            if(!nodes.contains(n)){
                node = new NOI();
                node->n=n;
                node->x=ntList.at(i).listNeuron.at(j).x;
                node->y=ntList.at(i).listNeuron.at(j).y;
                node->z=ntList.at(i).listNeuron.at(j).z;
                node->type=ntList.at(i).listNeuron.at(j).type;
                node->r=ntList.at(i).listNeuron.at(j).r;
                node->fea_val=ntList.at(i).listNeuron.at(j).fea_val;
                node->cid = -1;
                nodes.insert(n,node);
            }else{
                node=nodes.value(n);
            }

            if(ntList.at(i).listNeuron.at(j).pn>=0){
                parents.insert(node, ntList.at(i).listNeuron.at(j).pn+noffset);
            }
            nmax=MAX(nmax,ntList.at(i).listNeuron.at(j).n);
        }
        noffset=nmax+1;
    }
    //init connections
    for(QMultiHash<NOI*,V3DLONG>::Iterator iter=parents.begin(); iter!=parents.end(); iter++){
        V3DLONG pn=iter.value();

        if(nodes.contains(pn) && pn!=iter.key()->n){ //skip unexist node or self loop
            iter.key()->conn.insert(nodes.value(pn));
            nodes.value(pn)->conn.insert(iter.key());
        }
    }
    //remove duplicated node
    for(QHash<V3DLONG, NOI*>::Iterator iter = nodes.begin(); iter!=nodes.end(); ){
        NOI * node=iter.value();
        bool mask_rm = false;
        for(QSet<NOI *>::Iterator iter_c=node->conn.begin(); iter_c!=node->conn.end(); iter_c++){
            NOI * nei=*iter_c;
            if(fabs(node->x-nei->x)+fabs(node->y-nei->y)+fabs(node->z-nei->z) < 1e-3){//merge them
                nei->conn.remove(node);
                for(QSet<NOI *>::Iterator iter_n=node->conn.begin(); iter_n!=node->conn.end(); iter_n++){
                    (*iter_n)->conn.remove(node);
                    if(nei == *iter_n) //no self loop
                        continue;
                    nei->conn.insert(*iter_n);
                    (*iter_n)->conn.insert(nei);
                }
                iter=nodes.erase(iter);
                mask_rm=true;
                break;
            }
        }
        if(!mask_rm)
            iter++;
    }
    //init connected components
    {
        V3DLONG cid=0;
        for(QHash<V3DLONG, NOI*>::Iterator iter = nodes.begin(); iter!=nodes.end(); iter++){
            NOI * node=iter.value();
            if(node->cid>=0)
                continue;
            QQueue<NOI*> seeds;
            node->cid=cid;
            seeds.push_back(node);
            while(seeds.size()>0){
                node = seeds.dequeue();
                for(QSet<NOI *>::Iterator iter_n = node->conn.begin(); iter_n != node->conn.end(); iter_n++){
                    if((*iter_n)->cid<0){
                        (*iter_n)->cid=cid;
                        seeds.push_back(*iter_n);
                    }
                }
            }
            cid++;
        }
        coffset=cid;
        qDebug()<<"NeuronAssembler: "<<cid<<" fragements identified";
    }

    //init NeuronTree
    nt.file=ntList.at(0).file;
    nt.color = XYZW(0,0,0,0);
    nt.on = true;
    nt.n = 1;
    nt.listNeuron.clear();
    nt.hashNeuron.clear();
    {
        QHashIterator<V3DLONG, NOI*> iter(nodes);
        while(iter.hasNext()){
            iter.next();
            NOI * node=iter.value();
            for(QSet<NOI *>::Iterator iter_c=node->conn.begin(); iter_c!=node->conn.end(); iter_c++){
                NeuronSWC neuron;
                neuron.n=node->n;
                neuron.type=node->type;
                neuron.x=node->x;
                neuron.y=node->y;
                neuron.z=node->z;
                neuron.r=node->r;
                neuron.pn=(*iter_c)->n;
                nt.listNeuron.push_back(neuron);
                nt.hashNeuron.insert(neuron.n, nt.listNeuron.size()-1);
            }
        }
    }
}

void assemble_neuron_live_dialog::searchPair()
{
    //check if need to clean exist node
    LandmarkList * mList = getMarkerList();
    if(list_link->count()!=0){
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Search Connection", "Do you want to clean the list of node to connect?",
                                      QMessageBox::Yes|QMessageBox::No);
        if(reply == QMessageBox::Yes){
            mList->clear();
            list_link->clear();
            update3DView();
        }
    }

    //get parameters
    connect_param_dialog cpDlg;
    if(cpDlg.exec()==QDialog::Rejected){
        return;
    }

    //search for connections
    QSet<QPair<V3DLONG, V3DLONG> > linknew = searchConnection(cpDlg.spin_xscale->value(), cpDlg.spin_yscale->value(), cpDlg.spin_zscale->value(),
                                                               cos((180-cpDlg.spin_ang->value())/180*M_PI), cpDlg.spin_dis->value(),
                                                               cpDlg.cb_matchType->currentIndex(), cpDlg.cb_distanceType->currentIndex()==0);

    //update link list and markers
    if(linknew.size()==0){
        v3d_msg("No qualified connection found, please change parameter.");
        return;
    }

    //find exist links
    QSet<QPair<V3DLONG,V3DLONG> > links;
    QList<int> info;
    QHash<int, int> markerTable;
    for(int mid=0; mid<mList->size(); mid++){
        get_marker_info(mList->at(mid), info);
        markerTable.insert(info.at(0),mid);
    }
    for(int mid=0; mid<mList->size(); mid++){
        get_marker_info(mList->at(mid), info);
        QList<int>::iterator iter = info.begin();
        iter++;
        while(iter!=info.end()){
            if(info.at(0)!=*iter && markerTable.contains(*iter)){
                QPair<V3DLONG,V3DLONG> tmp(MIN(info.at(0), *iter),MAX(info.at(0), *iter));
                links.insert(tmp);
                iter++;
            }else{
                iter=info.erase(iter);
                update_marker_info(mList->at(mid), info);
            }
        }
    }

    //update marker and link
    for(QSet<QPair<V3DLONG,V3DLONG> >::Iterator iter = linknew.begin(); iter!=linknew.end(); iter++){
        //check marker
        int mid1, mid2;
        V3DLONG pid1 = (*iter).first;
        V3DLONG pid2 = (*iter).second;
        QList<int> info;
        if(markerTable.contains(pid1)){
            mid1=markerTable.value(pid1);
            get_marker_info(mList->at(mid1),info);
            if(!info.contains(pid2)){
                info.push_back(pid2);
                update_marker_info(mList->at(mid1),info);
            }
        }else{
            LocationSimple S(nodes.value(pid1)->x,nodes.value(pid1)->y,nodes.value(pid1)->z);
            S.color=XYZW(0,128,0,0);
            S.name=QString::number(pid1).toStdString();
            S.comments=QString::number(pid2).toStdString();
            markerTable.insert(pid1,mList->size());
            mList->push_back(S);
        }
        if(markerTable.contains(pid2)){
            mid2=markerTable.value(pid2);
            get_marker_info(mList->at(mid2),info);
            if(!info.contains(pid1)){
                info.push_back(pid1);
                update_marker_info(mList->at(mid2),info);
            }
        }else{
            LocationSimple S(nodes.value(pid2)->x,nodes.value(pid2)->y,nodes.value(pid2)->z);
            S.color=XYZW(0,128,0,0);
            S.name=QString::number(pid2).toStdString();
            S.comments=QString::number(pid1).toStdString();
            markerTable.insert(pid2,mList->size());
            mList->push_back(S);
        }
        //update list
        links.insert(*iter);
    }

    //update list widget
    list_link->clear();
    //sort the connections
    multimap<int, QPair<V3DLONG, V3DLONG> > mlinks;
    for(QSet<QPair<V3DLONG, V3DLONG> >::iterator iter = links.begin(); iter!=links.end(); iter++){
        V3DLONG pid1, pid2;
        pid1=iter->first;
        pid2=iter->second;
        int mid1 = markerTable[pid1]+1;
        int mid2 = markerTable[pid2]+1;
        if(mid1<mid2){
            mlinks.insert(pair<int, QPair<V3DLONG, V3DLONG> >(mid1*markerTable.size()+mid2, QPair<V3DLONG,V3DLONG>(pid1, pid2)));
        }else{
            mlinks.insert(pair<int, QPair<V3DLONG, V3DLONG> >(mid2*markerTable.size()+mid1, QPair<V3DLONG,V3DLONG>(pid2, pid1)));
        }
    }
    //update list
    for(multimap<int, QPair<V3DLONG, V3DLONG> >::iterator iter = mlinks.begin(); iter!=mlinks.end(); iter++){
        V3DLONG pid1, pid2;
        pid1=iter->second.first;
        pid2=iter->second.second;
        int mid1 = markerTable[pid1]+1;
        int mid2 = markerTable[pid2]+1;
        QString tmp = "Marker "+QString::number(mid1)+" x "+
                QString::number(mid2)+" (Node "+QString::number(pid1)+" x "+
                QString::number(pid2)+" )";
        list_link->addItem(tmp);
    }
    if(list_link->count()>0 && list_link->currentRow()!=0){
        list_link->setCurrentRow(0);
    }

    update3DView();
}

QSet<QPair<V3DLONG, V3DLONG> > assemble_neuron_live_dialog::searchConnection
(double xscale, double yscale, double zscale, double angThr, double disThr, int matchType, bool b_minusradius)
{
    QSet<QPair<V3DLONG, V3DLONG> > links;
    bool b_somaaxon = false;
    if(disThr<0){
        disThr=getNeuronDiameter();
    }
    //init
    qDebug()<<"Init neuron and tips";
    QHash<NOI *, XYZ> scaledXYZ;
    QList<NOI *> cand;
    QHash<NOI *, XYZ> canddir;
    QHash<int, int> clink;
    QHash<int, QList<NOI *> > components;
    for(NodeIter iter=nodes.begin(); iter!=nodes.end(); iter++){
        NOI * node = iter.value();

        //scale xyz
        XYZ S;
        S.x = node->x*xscale;
        S.y = node->y*yscale;
        S.z = node->z*zscale;
        scaledXYZ.insert(iter.value(), S);

        //link component to it self
        clink.insert(node->cid,node->cid);
        if(components.contains(node->cid)){
            components[node->cid].push_back(node);
        }else{
            QList<NOI *> tmp;
            tmp.push_back(node);
            components.insert(node->cid,tmp);
        }

        //find tips
        if(node->conn.size()<=1){
            cand.push_back(node);
            //get direction
            XYZ tmpdir(0,0,0);
            if(node->conn.size()==1){
                tmpdir.x += scaledXYZ[node].x-scaledXYZ[*(node->conn.begin())].x;
                tmpdir.y += scaledXYZ[node].y-scaledXYZ[*(node->conn.begin())].y;
                tmpdir.z += scaledXYZ[node].z-scaledXYZ[*(node->conn.begin())].z;
                double tmpNorm = sqrt(tmpdir.x*tmpdir.x+tmpdir.y*tmpdir.y+tmpdir.z*tmpdir.z);
                if(tmpNorm>1e-16){
                    tmpdir.x/=tmpNorm;
                    tmpdir.y/=tmpNorm;
                    tmpdir.z/=tmpNorm;
                }
            }
            canddir.insert(node,tmpdir);
        }
    }

    qDebug()<<"match tips";
    //match tips
    multimap<double, QPair<NOI *, NOI *> > connMap;
    for(V3DLONG tid=0; tid<cand.size(); tid++){
        NOI * tnode = cand.at(tid);
        for(V3DLONG cid=0; cid<coffset; cid++){
            if(cid==tnode->cid) continue;
            if(!components.contains(cid) || components.value(cid).size()<1) continue;
            double mvdis=disThr, mtdis=disThr;
            NOI * mvnode=0, * mtnode=0;
            for(QList<NOI *>::Iterator iter = components[cid].begin(); iter!= components[cid].end(); iter++){
                NOI * node = *iter;
                if(matchType==1){ //must be the same type to connect
                    if(node->type!=tnode->type){
                        continue;
                    }
                }
                if(matchType==2){ //must be the same type except soma
                    if(node->type!=tnode->type &&
                            node->type!=1 && tnode->type!=1){
                        continue;
                    }
                }
                double dis=NTDIS(scaledXYZ[tnode],scaledXYZ[node]);
                if(b_minusradius){
                    dis-=node->radius;
                    dis-=tnode->radius;
                }
                if(dis<mvdis){
                    mvdis=dis;
                    mvnode=node;
                }
                if(dis<mtdis){
                    if(node->conn.size()<=1){//tips
                        if(NTDOT(canddir[tnode],canddir[node])<angThr){
                            mtdis=dis;
                            mtnode=node;
                        }
                    }
                }
            }
            if(mvnode!=0){
                connMap.insert(pair<double, QPair<NOI *, NOI * > >(mvdis+disThr,QPair<NOI *, NOI *>(tnode, mvnode)));
            }
            if(mtnode!=0){
                connMap.insert(pair<double, QPair<NOI *, NOI * > >(mtdis,QPair<NOI *, NOI *>(tnode, mtnode)));
            }
        }
    }

    qDebug()<<"connecting tips";
    //find the best solution for connecting tips
    for(multimap<double, QPair<NOI *, NOI * > >::iterator iter=connMap.begin(); iter!=connMap.end(); iter++){
        NOI * node1 = iter->second.first;
        NOI * node2 = iter->second.second;
        if(clink.value(node1->cid)==clink.value(node2->cid)) //already connected
            continue;
        if(matchType==2){ //check soma axon connection
            if(node1->type * node2->type == 2){ //is soma axon connection
                if(b_somaaxon) //already has soma axon connection
                    continue;
                else
                    b_somaaxon=true;
            }
        }
        links.insert(QPair<V3DLONG, V3DLONG>(MIN(node1->n, node2->n),MAX(node1->n, node2->n)));
        int cid1=clink.value(node1->cid);
        int cid2=clink.value(node2->cid);
        for(QHash<int, int>::Iterator citer=clink.begin(); citer!=clink.end(); citer++){
            if(citer.value()==cid2){
                citer.value()=cid1;
            }
        }
    }

    return links;
}

bool assemble_neuron_live_dialog::connectNode(V3DLONG pid1, V3DLONG pid2)
{
    bool modified = false;
    //find the node
    NOI * node1, * node2;
    if(nodes.contains(pid1) && nodes.contains(pid2)){
        node1=nodes.value(pid1);
        node2=nodes.value(pid2);
    }else{
        return modified;
    }
    //connect node
    if(!node1->conn.contains(node2)){
        node1->conn.insert(node2);
        NeuronSWC neuron;
        neuron.n=node1->n;
        neuron.type=node1->type;
        neuron.x=node1->x;
        neuron.y=node1->y;
        neuron.z=node1->z;
        neuron.r=node1->r;
        neuron.pn=node2->n;
        nt.listNeuron.push_back(neuron);
        nt.hashNeuron.insert(neuron.n, nt.listNeuron.size()-1);
        modified = true;
    }
    if(!node2->conn.contains(node1)){
        node2->conn.insert(node1);
        NeuronSWC neuron;
        neuron.n=node2->n;
        neuron.type=node2->type;
        neuron.x=node2->x;
        neuron.y=node2->y;
        neuron.z=node2->z;
        neuron.r=node2->r;
        neuron.pn=node1->n;
        nt.listNeuron.push_back(neuron);
        nt.hashNeuron.insert(neuron.n, nt.listNeuron.size()-1);
        modified = true;
    }
    //update connected component
    if(node1->cid != node2->cid){
        V3DLONG tmpc = node2->cid;
        for(NodeIter iter = nodes.begin(); iter!=nodes.end(); iter++){
            NOI * node = iter.value();
            if(node->cid == tmpc){
                node->cid = node1->cid;
                modified = true;
            }
        }
    }
    return modified;
}

void assemble_neuron_live_dialog::connectPair()
{
    //get the node to connect
    int idx=list_link->currentRow();
    if(idx<0){
        v3d_msg("Please select the node to connect. You can: a) search connection; or b) manually define markers on reconstruction and link them.");
        return;
    }
    QString tmp=list_link->currentItem()->text();
    QStringList items = tmp.split(" ", QString::SkipEmptyParts);
    int pid1, pid2;
    bool check;
    pid1=items.at(5).toInt(&check);
    if(!check || !nodes.contains(pid1)){
        v3d_msg("Encounter unexpected error 1. Plase contact developer.");
        return;
    }
    pid2=items.at(7).toInt(&check);
    if(!check || !nodes.contains(pid2)){
        v3d_msg("Encounter unexpected error 1. Plase contact developer.");
        return;
    }
    //check the loop
    if(check_loop->isChecked()){
        if(nodes.value(pid1)->cid == nodes.value(pid2)->cid){
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "Loop identified", "The nodes selected are from the same neuron. Connect them will result in loop. Are you sure you want to connect them?",
                                          QMessageBox::Yes|QMessageBox::No);
            if(reply == QMessageBox::No){
                return;
            }
        }
    }
    //connect
    if(connectNode(pid1, pid2)){
        updateColor();
    }
    //remove the pair from the list
    delPair();
}

void assemble_neuron_live_dialog::syncMarker()
{
    LandmarkList * mList = getMarkerList();

    list_marker->clear();
    for(V3DLONG i=0; i<mList->size(); i++){
        QString tmp="marker: "+QString::number(i+1);
        list_marker->addItem(tmp);
    }

    if(mList->size()==0){
        v3d_msg("Not marker identified. Please define some in the 3D viewer.");
    }
}

void assemble_neuron_live_dialog::pairMarker()
{
    LandmarkList * mList = getMarkerList();

    if(mList->size()<=0){
        v3d_msg("Not marker identified. Please define some in the 3D viewer.");

        list_link->clear();
        return;
    }

    //find exist links
    QSet<QPair<V3DLONG,V3DLONG> > links;
    QList<int> info;
    QHash<int, int> markerTable;
    for(int mid=0; mid<mList->size(); mid++){
        get_marker_info(mList->at(mid), info);
        markerTable.insert(info.at(0),mid);
    }
    for(int mid=0; mid<mList->size(); mid++){
        get_marker_info(mList->at(mid), info);
        QList<int>::iterator iter = info.begin();
        iter++;
        while(iter!=info.end()){
            if(info.at(0)!=*iter && markerTable.contains(*iter)){
                QPair<V3DLONG,V3DLONG> tmp(MIN(info.at(0), *iter),MAX(info.at(0), *iter));
                links.insert(tmp);
                iter++;
            }else{
                iter=info.erase(iter);
                update_marker_info(mList->at(mid), info);
            }
        }
    }

    //ask for the new link
    pair_marker_dialog pairDlg(mList, this);
    if(pairDlg.exec()){ //accepted
        QList<int> info1, info2;
        int mid1 = pairDlg.list1->currentRow();
        int mid2 = pairDlg.list2->currentRow();
        get_marker_info(mList->at(mid1), info1);
        get_marker_info(mList->at(mid2), info2);
        if(info1.at(0)!=info2.at(0)){
            QPair<V3DLONG,V3DLONG> tmp(MIN(info1.at(0), info2.at(0)),MAX(info1.at(0), info2.at(0)));
            if(!links.contains(tmp)){
                links.insert(tmp);
                info1.push_back(info2.at(0));
                info2.push_back(info1.at(0));
                update_marker_info(mList->at(mid1), info1);
                update_marker_info(mList->at(mid2), info2);
            }
        }
    }else{
        return;
    }

    //update list widget
    list_link->clear();
    //sort the connections
    multimap<int, QPair<V3DLONG, V3DLONG> > mlinks;
    for(QSet<QPair<V3DLONG, V3DLONG> >::iterator iter = links.begin(); iter!=links.end(); iter++){
        V3DLONG pid1, pid2;
        pid1=iter->first;
        pid2=iter->second;
        int mid1 = markerTable[pid1]+1;
        int mid2 = markerTable[pid2]+1;
        if(mid1<mid2){
            mlinks.insert(pair<int, QPair<V3DLONG, V3DLONG> >(mid1*markerTable.size()+mid2, QPair<V3DLONG,V3DLONG>(pid1, pid2)));
        }else{
            mlinks.insert(pair<int, QPair<V3DLONG, V3DLONG> >(mid2*markerTable.size()+mid1, QPair<V3DLONG,V3DLONG>(pid2, pid1)));
        }
    }
    //update list
    for(multimap<int, QPair<V3DLONG, V3DLONG> >::iterator iter = mlinks.begin(); iter!=mlinks.end(); iter++){
        V3DLONG pid1, pid2;
        pid1=iter->second.first;
        pid2=iter->second.second;
        int mid1 = markerTable[pid1]+1;
        int mid2 = markerTable[pid2]+1;
        QString tmp = "Marker "+QString::number(mid1)+" x "+
                QString::number(mid2)+" (Node "+QString::number(pid1)+" x "+
                QString::number(pid2)+" )";
        list_link->addItem(tmp);
    }
    if(list_link->count()>0 && list_link->currentRow()!=0){
        list_link->setCurrentRow(0);
    }

    update3DView();
}

void assemble_neuron_live_dialog::delPair()
{
    if(list_link->currentRow()>=0 && list_link->currentRow()<list_link->count()){
        QString tmp = list_link->currentItem()->text();
        QStringList items = tmp.split(" ", QString::SkipEmptyParts);
        int mid, pid;
        bool check;
        LandmarkList * mList = getMarkerList();
        mid=items.at(1).toInt(&check)-1;
        if(check && mid>=0 && mid<mList->size()){
            pid=items.at(7).toInt(&check);
            if(check){
                QList<int> info;
                get_marker_info(mList->at(mid), info);
                QList<int>::iterator iter=info.begin();
                iter++;
                while(iter!=info.end()){
                    if(*iter == pid){
                        info.erase(iter);
                    }else{
                        iter++;
                    }
                }
                update_marker_info(mList->at(mid), info);
            }
            qDebug()<<mid<<":"<<pid;
        }
        mid=items.at(3).toInt(&check)-1;
        if(check && mid>=0 && mid<mList->size()){
            pid=items.at(5).toInt(&check);
            if(check){
                QList<int> info;
                get_marker_info(mList->at(mid), info);
                QList<int>::iterator iter=info.begin();
                iter++;
                while(iter!=info.end()){
                    if(*iter == pid){
                        info.erase(iter);
                    }else{
                        iter++;
                    }
                }
                update_marker_info(mList->at(mid), info);
            }
            qDebug()<<mid<<":"<<pid;
        }

        list_link->takeItem(list_link->currentRow());
    }

    update3DView();
}

LandmarkList * assemble_neuron_live_dialog::getMarkerList()
{
    V3dR_MainWindow * _3dwin = check3DWindow();
    LandmarkList * mList = callback->getHandleLandmarkList_Any3DViewer(_3dwin);
    if(mList->size()<=0){
        return mList;
    }

    double dis;
    QVector<V3DLONG> pidList, tidList, midList;
    int nonetip_count=0, cantip_count=0;
    for(int i=0; i<mList->size(); i++){ //find new markers
        QList<int> info;
        get_marker_info(mList->at(i),info);
        if(nodes.contains(info.at(0)))
            continue;
        V3DLONG pid=(*nodes.begin())->n;
        double mdis=NTDIS(**(nodes.begin()),mList->at(i));

        for(NodeIter iter = nodes.begin(); iter != nodes.end(); iter++){
            NOI* node = iter.value();
            dis=NTDIS(*node,mList->at(i));
            if(dis<mdis){
                mdis=dis;
                pid=node->n;
            }
        }

        //to-do, search tips
        V3DLONG tid=pid;
        //tid=matchfunc->search_tip0(nid);
//        if(tid!=pid){
//            nonetip_count++;
//            if(tid>=0) cantip_count++;
//        }

        pidList.append(pid);
        tidList.append(tid);
        midList.append(i);
    }
    if(cantip_count>0){
        QString str_msg = QString::number(pidList.size()) + " new markers were identified.\n" +
                QString::number(nonetip_count) + " of them are not eligible border tips.\n" +
                QString::number(cantip_count) + " of them can be corrected.\n Do you want to correct it?";
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Match Border Tips", str_msg,
                                        QMessageBox::Yes|QMessageBox::No);
        if(reply==QMessageBox::Yes){
            for(int i=0; i<pidList.size(); i++){
                pidList[i]=tidList.at(i);
            }
        }
    }

    //update
    LocationSimple *p = 0;
    for(int j=0; j<pidList.size(); j++){
        int i=midList.at(j);
        V3DLONG pid=pidList.at(j);
        p = (LocationSimple *)&(mList->at(i));
        p->name=QString::number(pid).toStdString();
        p->comments="";
        p->color.r = 0;
        p->color.g = 128;
        p->color.b = 0;
        p->x=nodes[pid]->x;
        p->y=nodes[pid]->y;
        p->z=nodes[pid]->z;
    }

    callback->update_3DViewer(_3dwin);
    View3DControl * _3dcontrol = callback->getView3DControl_Any3DViewer(_3dwin);
    if(_3dcontrol){
        _3dcontrol->updateLandmark();
    }

    return mList;
}

void assemble_neuron_live_dialog::setColor(int i)
{
    updateColor();
}


void assemble_neuron_live_dialog::updateDisplay()
{
    if(p_img4d!=0){
        updateImageWindow();
        updateROIWindow();
    }

    update3DWindow();
    updateColor();
}

void assemble_neuron_live_dialog::update3DView()
{
    V3dR_MainWindow * _3dwin = check3DWindow();
    if(_3dwin){
        callback->update_3DViewer(_3dwin);
        View3DControl * _3dcontrol = callback->getView3DControl_Any3DViewer(_3dwin);
        if(_3dcontrol){
            _3dcontrol->updateLandmark();
        }
    }
}

v3dhandle assemble_neuron_live_dialog::updateImageWindow()
{
    if(!p_img4d)
        return 0;
    v3dhandle winhandle = getImageWindow();
    if(winhandle == 0){
        winhandle = callback->newImageWindow(WINNAME_ASSEM);
        callback->setImage(winhandle, p_img4d);
        callback->updateImageWindow(winhandle);
        callback->open3DWindow(winhandle);
    }else{
        callback->setImage(winhandle, p_img4d);
        callback->updateImageWindow(winhandle);
        callback->open3DWindow(winhandle);
    }

    return winhandle;
}

v3dhandle assemble_neuron_live_dialog::checkImageWindow()
{
    if(!p_img4d)
        return 0;
    v3dhandle winhandle = getImageWindow();
    if(winhandle == 0){
        winhandle = callback->newImageWindow(WINNAME_ASSEM);
        callback->setImage(winhandle, p_img4d);
        callback->updateImageWindow(winhandle);
        callback->open3DWindow(winhandle);
    }

    return winhandle;
}

v3dhandle assemble_neuron_live_dialog::getImageWindow()
{
    if(!p_img4d)
        return 0;
    v3dhandleList allWindowList = callback->getImageWindowList();
    v3dhandle winhandle = 0;
    for (V3DLONG i=0;i<allWindowList.size();i++)
    {
        if(callback->getImageName(allWindowList.at(i)).contains(WINNAME_ASSEM)){
            winhandle = allWindowList[i];
            break;
        }
    }

    return winhandle;
}

V3dR_MainWindow * assemble_neuron_live_dialog::update3DWindow()
{
    V3dR_MainWindow * _3dwin = get3DWindow();
    if(_3dwin==0){
        //check if there is image, try open 3d window from image window
        if(p_img4d!=0){
            v3dhandle winhandle = getImageWindow();
            if(winhandle==0){
                winhandle = checkImageWindow();
            }else{
                callback->open3DWindow(winhandle);
            }
            _3dwin = get3DWindow();
        }
        //otherwise open a independent 3D window
        if(_3dwin==0){
            _3dwin = callback->createEmpty3DViewer();
            callback->setWindowDataTitle(_3dwin, WINNAME_ASSEM);
        }
    }
    QList<NeuronTree> * tmp_ntList = callback->getHandleNeuronTrees_Any3DViewer(_3dwin);
    tmp_ntList->clear();
    tmp_ntList->push_back(nt);
    callback->update_3DViewer(_3dwin);
    callback->update_NeuronBoundingBox(_3dwin);

    return _3dwin;
}

V3dR_MainWindow * assemble_neuron_live_dialog::check3DWindow()
{
    V3dR_MainWindow * _3dwin = get3DWindow();
    if(_3dwin==0){
        //check if there is image, try open 3d window from image window first
        if(p_img4d!=0){
            v3dhandle winhandle = getImageWindow();
            if(winhandle==0){
                winhandle = checkImageWindow();
            }else{
                callback->open3DWindow(winhandle);
            }
            _3dwin = get3DWindow();
        }
        //otherwise open a independent 3D window
        if(_3dwin==0){
            _3dwin = callback->createEmpty3DViewer();
            callback->setWindowDataTitle(_3dwin, WINNAME_ASSEM);
            QList<NeuronTree> * tmp_ntList = callback->getHandleNeuronTrees_Any3DViewer(_3dwin);
            tmp_ntList->clear();
            tmp_ntList->push_back(nt);
            callback->update_3DViewer(_3dwin);
            callback->update_NeuronBoundingBox(_3dwin);
        }
    }

    return _3dwin;
}

V3dR_MainWindow * assemble_neuron_live_dialog::get3DWindow()
{
    //search 3d window
    V3dR_MainWindow * _3dwin = 0;
    QList <V3dR_MainWindow *> list_3dwin = callback->getListAll3DViewers();
    for(int i=0; i<list_3dwin.size(); i++){
        if(callback->getImageName(list_3dwin[i]).contains(WINNAME_ASSEM)){
            _3dwin = list_3dwin[i];
        }
    }

    return _3dwin;
}

void assemble_neuron_live_dialog::updateColor()
{
    //find 3d window
    V3dR_MainWindow * _3dwin = check3DWindow();
    if(_3dwin==0){
        v3d_msg("Error: failed to open 3D viewer.");
        return;
    }
    //update the neuron stored
    if(cb_color->currentIndex()==0){
        for(V3DLONG idx=0; idx<nt.listNeuron.size(); idx++){
            if(nodes.contains(nt.listNeuron[idx].n))
                nt.listNeuron[idx].type = nodes[nt.listNeuron[idx].n]->type;
        }
    }else{
        for(V3DLONG idx=0; idx<nt.listNeuron.size(); idx++){
            if(nodes.contains(nt.listNeuron[idx].n)){
                int type = nodes[nt.listNeuron[idx].n]->cid % 17;
                if(type<2)
                    type+=2;
                else
                    type+=3;
                nt.listNeuron[idx].type = type;
            }
        }
    }
    //update the neuron in the window
    QList<NeuronTree> * tmp_ntList = callback->getHandleNeuronTrees_Any3DViewer(_3dwin);
    if(tmp_ntList->size()<1 || tmp_ntList->at(0).listNeuron.size()!=nt.listNeuron.size()){
        update3DWindow();
    }else{
        for(V3DLONG idx=0; idx<nt.listNeuron.size(); idx++){
            (*tmp_ntList)[0].listNeuron[idx].type = nt.listNeuron.at(idx).type;
        }
        callback->update_3DViewer(_3dwin);
    }
    qDebug()<<"done color update";
}

void assemble_neuron_live_dialog::highlightPair()
{
    int idx=list_link->currentRow();
    if(idx<0 || idx>=list_link->count()) return;
    QString tmp = list_link->currentItem()->text();
    QStringList items = tmp.split(" ", QString::SkipEmptyParts);
    int mid1, mid2;
    bool check;
    LandmarkList * mList = getMarkerList();
    mid1=items.at(1).toInt(&check)-1;
    if(!check) mid1=-1;
    mid2=items.at(3).toInt(&check)-1;
    if(!check) mid2=-1;

    RGB8 color1; color1.r=color1.b=0; color1.g=128;
    RGB8 color2; color2.r=255; color2.b=color2.g=0;
    for(int i=0; i<mList->size(); i++){
        if(i==mid1 || i==mid2){
            set_marker_color(mList->at(i),color2);
        }else{
            set_marker_color(mList->at(i),color1);
        }
    }

    update3DView();
}

void assemble_neuron_live_dialog::highlightMarker()
{
    int idx=list_marker->currentRow();
    if(idx<0 || idx>=list_marker->count()) return;
    QString tmp = list_marker->currentItem()->text();
    QStringList items = tmp.split(" ", QString::SkipEmptyParts);
    int mid;
    bool check;
    LandmarkList * mList = getMarkerList();
    mid=items.at(1).toInt(&check)-1;
    if(!check) mid=-1;

    RGB8 color1; color1.r=color1.b=0; color1.g=128;
    RGB8 color2; color2.r=255; color2.b=color2.g=0;
    for(int i=0; i<mList->size(); i++){
        if(i==mid){
            set_marker_color(mList->at(i),color2);
        }else{
            set_marker_color(mList->at(i),color1);
        }
    }

    update3DView();
}

void assemble_neuron_live_dialog::updateROIWindow()
{

}

double assemble_neuron_live_dialog::getNeuronDiameter()
{
    if(nodes.size()<=0)
        return 0;
    float x_min=(*nodes.begin())->x;
    float x_max=(*nodes.begin())->x;
    float y_min=(*nodes.begin())->y;
    float y_max=(*nodes.begin())->y;
    float z_min=(*nodes.begin())->z;
    float z_max=(*nodes.begin())->z;
    for(NodeIter iter = nodes.begin(); iter!=nodes.end(); iter++){
        x_min=MIN(iter.value()->x, x_min);
        y_min=MIN(iter.value()->y, y_min);
        z_min=MIN(iter.value()->z, z_min);
        x_max=MAX(iter.value()->x, x_max);
        y_max=MAX(iter.value()->y, y_max);
        z_max=MAX(iter.value()->z, z_max);
    }
    double tmp=(x_max-x_min)*(x_max-x_min);
    tmp=(y_max-y_min)*(y_max-y_min);
    tmp=(z_max-z_min)*(z_max-z_min);
    tmp=sqrt(tmp);
    return tmp;
}

//~~~~~~~~~~pair dialog~~~~~~~~~~~
pair_marker_dialog::pair_marker_dialog(LandmarkList * mList,QWidget *parent)
{
    QGridLayout * layout = new QGridLayout(this);

    QLabel *label1= new QLabel("Marker 1:");
    QLabel *label2= new QLabel("Marker 2:");
    list1 = new QListWidget();
    list2 = new QListWidget();
    btn_yes = new QPushButton("Pair Select");
    btn_no = new QPushButton("Cancel");

    layout->addWidget(label1, 0,0,1,1);
    layout->addWidget(label2, 0,1,1,1);
    layout->addWidget(list1, 1,0,1,1);
    layout->addWidget(list2, 1,1,1,1);
    layout->addWidget(btn_yes, 2,0,1,1);
    layout->addWidget(btn_no, 2,1,1,1);

    for(V3DLONG i=0; i<mList->size(); i++){
        QList<int> info;
        get_marker_info(mList->at(i), info);
        QString tmp="Marker "+QString::number(i+1)+"; Node "+QString::number(info.at(0));
        list1->addItem(tmp);
        list2->addItem(tmp);
    }

    if(mList->size()>0){
        list1->setCurrentRow(0);
        list2->setCurrentRow(0);
    }

    connect(btn_yes,SIGNAL(clicked()),this,SLOT(accept()));
    connect(btn_no,SIGNAL(clicked()),this,SLOT(reject()));
}

//~~~~~~~~~~connect param dialog~~

connect_param_dialog::connect_param_dialog()
{
    creat();
    initDlg();
}

connect_param_dialog::~connect_param_dialog()
{
    QSettings settings("V3D plugin","neuronLiveAssemble_ConnParam");

    settings.setValue("disthr",this->spin_dis->value());
    settings.setValue("angthr",this->spin_ang->value());
    settings.setValue("xscale",this->spin_xscale->value());
    settings.setValue("yscale",this->spin_yscale->value());
    settings.setValue("zscale",this->spin_zscale->value());
    settings.setValue("distype",this->cb_distanceType->currentIndex());
    settings.setValue("matchtype",this->cb_matchType->currentIndex());
}

void connect_param_dialog::initDlg()
{
    QSettings settings("V3D plugin","neuronLiveAssemble_ConnParam");

    if(settings.contains("disthr"))
        this->spin_dis->setValue(settings.value("disthr").toDouble());
    if(settings.contains("angthr"))
        this->spin_ang->setValue(settings.value("angthr").toDouble());
    if(settings.contains("xscale"))
        this->spin_xscale->setValue(settings.value("xscale").toDouble());
    if(settings.contains("yscale"))
        this->spin_yscale->setValue(settings.value("yscale").toDouble());
    if(settings.contains("zscale"))
        this->spin_zscale->setValue(settings.value("zscale").toDouble());
    if(settings.contains("distype"))
        this->cb_distanceType->setCurrentIndex(settings.value("distype").toInt());
    if(settings.contains("matchtype"))
        this->cb_matchType->setCurrentIndex(settings.value("matchtype").toInt());
}

void connect_param_dialog::creat()
{
    gridLayout = new QGridLayout();

    cb_conf = new QComboBox();
    cb_conf->addItem("custom config");
    cb_conf->addItem("connect all, shortest distance");
    cb_conf->addItem("connect all, tip-tip only");
    cb_conf->addItem("connect local joint only");
    cb_conf->addItem("other config A");
    connect(cb_conf, SIGNAL(currentIndexChanged(int)), this, SLOT(myconfig()));
    spin_xscale = new QDoubleSpinBox();
    spin_xscale->setRange(1,100000); spin_xscale->setValue(1);
    spin_yscale = new QDoubleSpinBox();
    spin_yscale->setRange(1,100000); spin_yscale->setValue(1);
    spin_zscale = new QDoubleSpinBox();
    spin_zscale->setRange(1,100000); spin_zscale->setValue(1);
    spin_ang = new QDoubleSpinBox();
    spin_ang->setRange(0,180); spin_ang->setValue(60);
    spin_dis = new QDoubleSpinBox();
    spin_dis->setRange(-1,100000); spin_dis->setValue(10);
    cb_matchType = new QComboBox();
    cb_matchType->addItem("no type rule");
    cb_matchType->addItem("must be the same type");
    cb_matchType->addItem("topology constrain");
    cb_matchType->setCurrentIndex(2);
    cb_distanceType = new QComboBox();
    cb_distanceType->addItem("between surface (suggested)");
    cb_distanceType->addItem("between skeleton");
    QLabel* label_0 = new QLabel("sacles: ");
    gridLayout->addWidget(label_0,8,0,1,1);
    QLabel* label_1 = new QLabel("X: ");
    gridLayout->addWidget(label_1,8,1,1,1);
    gridLayout->addWidget(spin_xscale,8,2,1,1);
    QLabel* label_2 = new QLabel("Y: ");
    gridLayout->addWidget(label_2,8,3,1,1);
    gridLayout->addWidget(spin_yscale,8,4,1,1);
    QLabel* label_3 = new QLabel("Z: ");
    gridLayout->addWidget(label_3,8,5,1,1);
    gridLayout->addWidget(spin_zscale,8,6,1,1);
    QLabel* label_conf = new QLabel("connection configuration: ");
    gridLayout->addWidget(label_conf,9,0,1,4);
    gridLayout->addWidget(cb_conf,9,4,1,3);
    QLabel* label_4 = new QLabel("maximum connection distance (-1=connect all): ");
    gridLayout->addWidget(label_4,10,0,1,6);
    gridLayout->addWidget(spin_dis,10,6,1,1);
    QLabel* label_5 = new QLabel("tip-tip match angular threshold: ");
    gridLayout->addWidget(label_5,11,0,1,6);
    gridLayout->addWidget(spin_ang,11,6,1,1);
    QLabel* label_6 = new QLabel("type of distance measurment: ");
    gridLayout->addWidget(label_6,12,0,1,4);
    gridLayout->addWidget(cb_distanceType,12,4,1,3);
    QLabel* label_7 = new QLabel("rules of connected fragment type: ");
    gridLayout->addWidget(label_7,13,0,1,4);
    gridLayout->addWidget(cb_matchType,13,4,1,3);

    //operation zone
    QFrame *line_2 = new QFrame();
    line_2->setFrameShape(QFrame::HLine);
    line_2->setFrameShadow(QFrame::Sunken);
    gridLayout->addWidget(line_2,15,0,1,7);
    btn_run = new QPushButton("Search");
    connect(btn_run,     SIGNAL(clicked()), this, SLOT(accept()));
    gridLayout->addWidget(btn_run,16,4,1,1);
    btn_quit = new QPushButton("Cancel");
    connect(btn_quit,     SIGNAL(clicked()), this, SLOT(reject()));
    gridLayout->addWidget(btn_quit,16,6,1,1);

    //operation zone
    QFrame *line_3 = new QFrame();
    line_3->setFrameShape(QFrame::HLine);
    line_3->setFrameShadow(QFrame::Sunken);
    gridLayout->addWidget(line_3,20,0,1,7);
    QString info=">> Set parameters to search fragment connections in SWC file to construct tree by adding new connects.\n";
    info+=">> angular threshold: Will first search for connections between tips with angle smaller than angthr degrees within disthr. Then will search for tip to segment connections. Set to 0 for no preference of tip to tip connection. If distance threshold is -1, there is no preference to tip-tip match.\n";
    info+=">> scale: the coordinate will be multiplied by scale before computing.\n";
    info+=">> topology constrain for connection rule: except soma (1), only fragments with the same type will be matched. And only 1 connection between axon and soma\n";
    text_info = new QTextEdit;
    text_info->setText(info);
    text_info->setReadOnly(true);
    gridLayout->addWidget(text_info,21,0,1,7);

    setLayout(gridLayout);
}

void connect_param_dialog::myconfig()
{
    if(cb_conf->currentIndex()==0){
        spin_ang->setEnabled(true);
        spin_dis->setEnabled(true);
    }else{
        spin_ang->setEnabled(false);
        spin_dis->setEnabled(false);
        switch (cb_conf->currentIndex()) {
        case 1:
            spin_ang->setValue(0);
            spin_dis->setValue(-1);
            break;
        case 2:
            spin_ang->setValue(180);
            spin_dis->setValue(-1);
            break;
        case 3:
            spin_ang->setValue(0);
            spin_dis->setValue(1);
            break;
        case 4:
            spin_ang->setValue(20);
            spin_dis->setValue(60);
            break;
        default:
            break;
        }
    }
}

//~~~~~~~~~~functions~~~~~~~~~~~~~

void update_marker_info(const LocationSimple& mk, QList<int> & info) //info[0]=node id, info[i]=point id to connect
{
    LocationSimple *p;
    p = (LocationSimple *)&mk;
    QString tmp;
    for(int i=1; i<info.size(); i++)
        tmp+=QString::number(info.at(i)) + " ";
    p->comments=tmp.toStdString();
    if(info.size()>0)
        p->name=QString::number(info.at(0)).toStdString();
    else
        p->name="-1";
}

void update_marker_info(const LocationSimple& mk, QList<int> & info, int* color) //info[0]=neuron id, info[1]=point id, info[2]=matching marker, info[3]=marker name/id
{
    LocationSimple *p;
    p = (LocationSimple *)&mk;
    QString tmp;
    for(int i=1; i<info.size(); i++)
        tmp+=QString::number(info.at(i)) + " ";
    p->comments=tmp.toStdString();
    if(info.size()>0)
        p->name=QString::number(info.at(0)).toStdString();
    else
        p->name="-1";
    p->color.r = color[0];
    p->color.g = color[1];
    p->color.b = color[2];
}

void set_marker_color(const LocationSimple& mk, RGB8 color)
{
    LocationSimple *p;
    p = (LocationSimple *)&mk;
    p->color.r = color.r;
    p->color.g = color.g;
    p->color.b = color.b;
}

bool get_marker_info(const LocationSimple& mk, QList<int> & info) //info[0]=neuron id, info[1]=point id, info[2]=matching marker, info[3]=marker name
{
    info.clear();
    info.push_back(-1);
    QString tmp;
    bool check;
    int val;
    tmp=QString::fromStdString(mk.name);
    val=tmp.toInt(&check, 10);
    if(!check)
        return false;
    else
        info[0]=val;
    tmp=QString::fromStdString(mk.comments);
    QStringList items = tmp.split(" ", QString::SkipEmptyParts);
    for(int i=0; i<items.size(); i++){
        val=items[i].toInt(&check, 10);
        if(check)
            info.push_back(val);
    }

    return true;
}
