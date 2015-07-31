#include "assemble_neuron_live_dialog.h"

#include <map>

using namespace std;

assemble_neuron_live_dialog::assemble_neuron_live_dialog(V3DPluginCallback2 * cb, QList<NeuronTree> &ntList, Image4DSimple * p_img4d_in, QWidget *parent) :
    QDialog(parent)
{
    callback = cb;
    winname_main = WINNAME_ASSEM;
    winname_3d = WINNAME_ASSEM;
    winname_roi = WINNAME_ASSEM;

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
    btn_loop = new QPushButton("check circle");
    layout->addWidget(btn_link,1,0,1,2);
    layout->addWidget(btn_loop,1,2,1,2);
    check_zoomin = new QCheckBox("auto sync zoomin view");
    spin_zoomin = new QSpinBox; spin_zoomin->setValue(30); spin_zoomin->setRange(0,10000000);
    btn_zoomin = new QPushButton("sync zoomin view");
    cb_color = new QComboBox();
    cb_color->addItem("color by type");
    cb_color->addItem("color by segment");
    cb_color->setCurrentIndex(1);
    layout->addWidget(check_zoomin,2,0,1,2);
    layout->addWidget(cb_color,2,2,1,2);
    layout->addWidget(btn_zoomin,3,0,1,2);
    QLabel * label_zoomin = new QLabel("window margin:");
    layout->addWidget(label_zoomin,3,2,1,1, Qt::AlignRight);
    layout->addWidget(spin_zoomin,3,3,1,1);

    connect(btn_link, SIGNAL(clicked()), this, SLOT(searchPair()));
    connect(btn_loop, SIGNAL(clicked()), this, SLOT(searchLoop()));
    connect(cb_color, SIGNAL(currentIndexChanged(int)), this, SLOT(setColor(int)));
    connect(btn_zoomin, SIGNAL(clicked()), this, SLOT(zoomin()));

    tab = new QTabWidget(this);
    //neuron connector
    QDialog * dialog_conn = new QDialog(tab);
    QGridLayout * layout_conn = new QGridLayout();
    list_link = new QListWidget();
    btn_connect = new QPushButton("connect select pair");
    btn_manuallink = new QPushButton("add connection pair");
    btn_deletelink = new QPushButton("delete connection pair");
    check_loop = new QCheckBox("Check loop before connect"); check_loop->setChecked(true);
    btn_connectall = new QPushButton("connect all pairs");
    layout_conn->addWidget(list_link,0,0,6,2);
    layout_conn->addWidget(btn_connect,0,2,1,1);
    layout_conn->addWidget(btn_manuallink,1,2,1,1);
    layout_conn->addWidget(btn_deletelink,2,2,1,1);
    layout_conn->addWidget(check_loop,3,2,1,1);
    layout_conn->addWidget(btn_connectall,5,2,1,1);
    dialog_conn->setLayout(layout_conn);
    tab->addTab(dialog_conn,tr("connect"));

    connect(list_link, SIGNAL(currentRowChanged(int)), this, SLOT(highlightPair()));
    connect(btn_connect, SIGNAL(clicked()), this, SLOT(connectPair()));
    connect(btn_manuallink,SIGNAL(clicked()),this,SLOT(pairMarker()));
    connect(btn_deletelink,SIGNAL(clicked()),this,SLOT(delPair()));
    connect(btn_connectall, SIGNAL(clicked()), this, SLOT(connectAll()));


    //neuron breaker
    QDialog * dialog_break = new QDialog(tab);
    QGridLayout * layout_break = new QGridLayout();
    list_edge = new QListWidget();
    btn_break = new QPushButton("break at select");
    btn_syncmarker = new QPushButton("load edges");
    layout_break->addWidget(list_edge,0,0,6,2);
    layout_break->addWidget(btn_break,0,2,1,1);
    layout_break->addWidget(btn_syncmarker,1,2,1,1);
    dialog_break->setLayout(layout_break);
    tab->addTab(dialog_break,tr("break"));

    connect(btn_syncmarker,SIGNAL(clicked()),this,SLOT(syncMarker()));
    connect(btn_break,SIGNAL(clicked()),this,SLOT(breakEdge()));
    connect(list_edge, SIGNAL(currentRowChanged(int)), this, SLOT(highlightEdge()));

    layout->addWidget(tab,4,0,1,4);

    //save, quit
    btn_save = new QPushButton("sort and save");
    btn_quit = new QPushButton("quit");
    layout->addWidget(btn_save,6,0,1,2);
    layout->addWidget(btn_quit,6,2,1,2);

    connect(btn_save,SIGNAL(clicked()),this,SLOT(sortsaveSWC()));
    connect(btn_quit,SIGNAL(clicked()),this,SLOT(reject()));

    setLayout(layout);
}

void assemble_neuron_live_dialog::initNeuron(QList<NeuronTree> &ntList)
{
    V3DLONG nmax=0;
    noffset=0;
    QMultiHash<NOI*,V3DLONG> parents;
    prev_root=-1;
    //init nodes
    nodes.clear();
    for(int i=0; i<ntList.size(); i++){
        for(int j=0; j<ntList.at(i).listNeuron.size(); j++){
            V3DLONG n=ntList.at(i).listNeuron.at(j).n+noffset;
            if(prev_root<0 && ntList.at(i).listNeuron.at(j).pn<0){
                prev_root=n;
            }
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

void assemble_neuron_live_dialog::searchLoop()
{
    QList<NOI *> loop = search_loop(nodes);
    if(loop.size()<=0){
        v3d_msg("No circle identified. Your reconstruction is good.");
        return;
    }

    //find 3d window
    V3dR_MainWindow * _3dwin = check3DWindow();
    if(_3dwin==0){
        v3d_msg("Error: failed to open 3D viewer.");
        return;
    }
    //update the neuron stored
    for(V3DLONG idx=0; idx<nt.listNeuron.size(); idx++){
        if(loop.contains(nodes[nt.listNeuron[idx].n]))
            nt.listNeuron[idx].type = 4;
        else
            nt.listNeuron[idx].type = 3;
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
    if(check_zoomin->isChecked()){
        QList<V3DLONG> pids;
        for(int i=0; i<loop.size(); i++){
            pids.push_back(loop.at(i)->n);
        }
        updateROIWindow(pids);
    }

    v3d_msg("A circle identified and highlighted in magenta. Please break the loop and search again.");

    tab->setCurrentIndex(1);
}

void assemble_neuron_live_dialog::searchPair()
{
    tab->setCurrentIndex(0);
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

    //recalculate disThr
    if(disThr<0){
        float x_min=(*scaledXYZ.begin()).x;
        float x_max=(*scaledXYZ.begin()).x;
        float y_min=(*scaledXYZ.begin()).y;
        float y_max=(*scaledXYZ.begin()).y;
        float z_min=(*scaledXYZ.begin()).z;
        float z_max=(*scaledXYZ.begin()).z;
        for(QHash<NOI *, XYZ>::Iterator iter = scaledXYZ.begin(); iter!=scaledXYZ.end(); iter++){
            x_min=MIN(iter.value().x, x_min);
            y_min=MIN(iter.value().y, y_min);
            z_min=MIN(iter.value().z, z_min);
            x_max=MAX(iter.value().x, x_max);
            y_max=MAX(iter.value().y, y_max);
            z_max=MAX(iter.value().z, z_max);
        }
        double tmp=(x_max-x_min)*(x_max-x_min);
        tmp+=(y_max-y_min)*(y_max-y_min);
        tmp+=(z_max-z_min)*(z_max-z_min);
        disThr=tmp;
    }else{
       disThr=disThr*disThr;
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

bool assemble_neuron_live_dialog::breakNode(V3DLONG pid1, V3DLONG pid2)
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
    //disconnect node
    node1->conn.remove(node2);
    node2->conn.remove(node1);
    QList<NeuronSWC>::Iterator piter=nt.listNeuron.begin();
    while(piter!=nt.listNeuron.end()){
        if((*piter).n == node1->n && (*piter).pn == node2->n){
            piter=nt.listNeuron.erase(piter);
            modified=true;
        }else if((*piter).pn == node1->n && (*piter).n == node2->n){
            piter=nt.listNeuron.erase(piter);
            modified=true;
        }else
            piter++;
    }
    if(modified){
        nt.hashNeuron.clear();
        for(V3DLONG i=0; i<nt.listNeuron.size(); i++){
            nt.hashNeuron.insert(nt.listNeuron.at(i).n, i);
        }
    }
    //update connected component
    if(node1->cid == node2->cid){ //should be true
        V3DLONG tmpc = coffset;
        QStack<NOI*> seeds;
        seeds.append(node2);
        node2->cid=tmpc;
        while(!seeds.isEmpty()){
            NOI* cur_node=seeds.pop();
            for(QSet<NOI*>::Iterator niter=cur_node->conn.begin(); niter!=cur_node->conn.end(); niter++){
                if((*niter)->cid!=tmpc){
                    (*niter)->cid=tmpc;
                    seeds.append(*niter);
                }
            }
        }
        modified=true;
        coffset++;
    }
    return modified;
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

void assemble_neuron_live_dialog::breakEdge()
{
    //get the node to connect
    int idx=list_edge->currentRow();
    if(idx<0){
        v3d_msg("Please select the edge to break. You can manually define markers on both side of the edge to break and then click load_edge button.");
        return;
    }
    QString tmp=list_edge->currentItem()->text();
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
    //check the edge
    if(!nodes.value(pid1)->conn.contains(nodes.value(pid2))){
        v3d_msg("Edge does not exist in record. This usually should not happen. Please contact developer.");
        return;
    }
    //connect
    if(breakNode(pid1, pid2)){
        updateColor();
    }
    //remove the edge from the list
    list_edge->takeItem(list_edge->currentRow());
    //update the marker and the zoomin view if it is the last pair to connect
    if(list_edge->count()<=0){
        LandmarkList* mList=getMarkerList();
        RGB8 color1; color1.r=color1.b=0; color1.g=128;
        for(int i=0; i<mList->size(); i++){
            set_marker_color(mList->at(i),color1);
        }

        update3DView();

        if(check_zoomin->isChecked()){
            QList<V3DLONG> pids;
            pids.push_back(pid1);
            pids.push_back(pid2);
            updateROIWindow(pids);
        }
    }
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
    //update the marker and the zoomin view if it is the last pair to connect
    if(list_link->count()<=0){
        LandmarkList* mList=getMarkerList();
        RGB8 color1; color1.r=color1.b=0; color1.g=128;
        for(int i=0; i<mList->size(); i++){
            set_marker_color(mList->at(i),color1);
        }

        update3DView();

        if(check_zoomin->isChecked()){
            QList<V3DLONG> pids;
            pids.push_back(pid1);
            pids.push_back(pid2);
            updateROIWindow(pids);
        }
    }
}

void assemble_neuron_live_dialog::connectAll()
{
    //get the node to connect
    if(list_link->count()<0){
        v3d_msg("Please select the node to connect. You can: a) search connection; or b) manually define markers on reconstruction and link them.");
        return;
    }
    QStringList list_tolink;
    QStringList list_conflict;
    for(int i=0; i<list_link->count(); i++)
        list_tolink.append(list_link->item(i)->text());

    //get marker
    LandmarkList * mList = getMarkerList();

    //connect
    for(int idx=0; idx<list_tolink.size(); idx++){
        QString tmp=list_tolink.at(idx);
        QStringList items = tmp.split(" ", QString::SkipEmptyParts);
        int pid1, pid2;
        bool check;
        pid1=items.at(5).toInt(&check);
        if(!check || !nodes.contains(pid1)){
            v3d_msg("Encounter unexpected error 2. Plase contact developer.");
            return;
        }
        pid2=items.at(7).toInt(&check);
        if(!check || !nodes.contains(pid2)){
            v3d_msg("Encounter unexpected error 2. Plase contact developer.");
            return;
        }
        //check the loop
        if(check_loop->isChecked()){
            if(nodes.value(pid1)->cid == nodes.value(pid2)->cid){
                list_conflict.append(tmp);
                continue;
            }
        }
        //connect
        if(connectNode(pid1, pid2)){ //update marker information if success
            int mid, pid;
            bool check;
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
            }
        }else{
            list_conflict.append(tmp);
        }
    }

    //update display
    list_link->clear();
    list_link->addItems(list_conflict);
    if(list_link->count()>0){
        list_link->setCurrentRow(0);
    }else{
        RGB8 color1; color1.r=color1.b=0; color1.g=128;
        for(int i=0; i<mList->size(); i++){
            set_marker_color(mList->at(i),color1);
        }
        update3DView();
    }
    updateColor();

    //message
    v3d_msg(QString::number(list_tolink.size()-list_conflict.size())+" pairs of markers were connected successfully;\n"+
            QString::number(list_conflict.size())+" pairs of markers were left due to loop confliction.");
}

void assemble_neuron_live_dialog::sortsaveSWC()
{
    //get root ids
    V3DLONG type1_root=-1;
    for(NodeIter iter=nodes.begin(); iter!=nodes.end(); iter++){
        if(iter.value()->type==1){
            type1_root=iter.value()->n;
            break;
        }
    }

    LandmarkList * mList = getMarkerList();

    sort_neuron_dialog sortDlg(mList, prev_root, type1_root, this);
    if(!sortDlg.exec())
        return;

    V3DLONG n_root=-1;
    if(sortDlg.radio_marker->isChecked()){
        QList<int> info;
        int i=sortDlg.cb_marker->currentIndex();
        get_marker_info(mList->at(i), info);
        n_root=info.at(0);
    }else if(sortDlg.radio_prev->isChecked()){
        n_root=prev_root;
    }else if(sortDlg.radio_type->isChecked()){
        n_root=type1_root;
    }

    if(!nodes.contains(n_root)){
        v3d_msg("Invalide root node selected.");
        return;
    }

    //generate swc file
    QList<NeuronSWC> NeuronList = generate_swc_typesort(nodes, n_root);

    //save file
    QString fname_output = nt.file + ".assembled.swc";
    fname_output = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
                                                fname_output,
            QObject::tr("Supported file (*.swc *.eswc)"
                ";;Neuron structure	(*.swc)"
                ";;Extended neuron structure (*.eswc)"
                ));
    if(fname_output.isEmpty()){
        return;
    }
    export_list2file(NeuronList, fname_output);
}

void assemble_neuron_live_dialog::syncMarker()
{
    LandmarkList * mList = getMarkerList();

    list_edge->clear();
    for(V3DLONG i=0; i<mList->size(); i++){
        QList<int> info_i;
        get_marker_info(mList->at(i), info_i);
        for(V3DLONG j=i+1; j<mList->size(); j++){
            QList<int> info_j;
            get_marker_info(mList->at(j), info_j);
            if(nodes[info_i.at(0)]->conn.contains(nodes[info_j.at(0)])){
                QString tmp="Marker "+QString::number(i+1)+" x "+
                        QString::number(j+1)+" (Node "+QString::number(info_i.at(0))+" x "+
                        QString::number(info_j.at(0))+" )";
                list_edge->addItem(tmp);
            }
        }
    }

    if(list_edge->count()==0){
        v3d_msg("Not marked edge identified. Please define the edge to break in the 3D viewer by defining markers on both side of the edge that you want to break.");
    }else{
        list_edge->setCurrentRow(0);
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
        if(check_zoomin->isChecked())
            zoomin();
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

        winname_main = WINNAME_ASSEM; winname_main += "_processed";
        winname_3d = "3D View [" + winname_main + "]";
        winname_roi = "Local 3D View [" + winname_main + "]";
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

        winname_main = WINNAME_ASSEM; winname_main += "_processed";
        winname_3d = "3D View [" + winname_main + "]";
        winname_roi = "Local 3D View [" + winname_main + "]";
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
        if(callback->getImageName(allWindowList.at(i))==winname_main){
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

            winname_main = WINNAME_ASSEM; winname_main+="_processed";
            winname_3d = "3D View ["; winname_3d+=WINNAME_ASSEM; winname_3d+="]";
            winname_roi = "3D View [" + winname_main + "]";
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
                updateColor();
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

            winname_main = WINNAME_ASSEM; winname_main+="_processed";
            winname_3d = "3D View ["; winname_3d+=WINNAME_ASSEM; winname_3d+="]";
            winname_roi = "3D View [" + winname_main + "]";
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
        if(callback->getImageName(list_3dwin[i])==winname_3d){
            _3dwin = list_3dwin[i];
        }
    }

    //test
    qDebug()<<"searching for: "<<winname_3d;

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

    if(check_zoomin->isChecked()){
        zoomin();
    }
}

void assemble_neuron_live_dialog::highlightEdge()
{
    int idx=list_edge->currentRow();
    if(idx<0 || idx>=list_edge->count()) return;
    QString tmp = list_edge->currentItem()->text();
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

    if(check_zoomin->isChecked()){
        zoomin();
    }
}

void assemble_neuron_live_dialog::zoomin()
{
    V3DLONG pid1=-1, pid2=-1;
    QString tmp="";
    //get the node to zoomin
    if(tab->currentIndex()==0){
        int idx=list_link->currentRow();
        if(idx<0){
            v3d_msg("Please select the pair of node to zoomin. You can: a) search connection; or b) manually define markers on reconstruction and link them.");
            return;
        }
        tmp=list_link->currentItem()->text();
    }else{
        int idx=list_edge->currentRow();
        if(idx<0){
            v3d_msg("Please select the edge to zoomin. You can manually define markers on both side of the edge and then click load_edge button.");
            return;
        }
        tmp=list_edge->currentItem()->text();
    }
    QStringList items = tmp.split(" ", QString::SkipEmptyParts);
    if(items.size()>8){
        bool check;
        pid1=items.at(5).toInt(&check);
        if(!check || !nodes.contains(pid1)){
            v3d_msg("Encounter unexpected error 4. Plase contact developer.");
            return;
        }
        pid2=items.at(7).toInt(&check);
        if(!check || !nodes.contains(pid2)){
            v3d_msg("Encounter unexpected error 4. Plase contact developer.");
            return;
        }
    }
    if(pid1<0 || pid2<0){
        v3d_msg("Please select the pair of node to zoomin.");
        return;
    }
    QList<V3DLONG> pids;
    pids.push_back(pid1);
    pids.push_back(pid2);

    //zoom in
    updateROIWindow(pids);
}

void assemble_neuron_live_dialog::updateROIWindow(const QList<V3DLONG>& pids)
{
    if(pids.size()<=0)
        return;
    //find the ROI window
    V3DLONG x_min, x_max, y_min, y_max, z_min, z_max;
    x_min=x_max=nodes[pids.at(0)]->x;
    y_min=y_max=nodes[pids.at(0)]->y;
    z_min=z_max=nodes[pids.at(0)]->z;
    for(int i=1; i<pids.size(); i++){
        x_min=MIN(x_min,nodes[pids.at(i)]->x);
        y_min=MIN(y_min,nodes[pids.at(i)]->y);
        z_min=MIN(z_min,nodes[pids.at(i)]->z);
        x_max=MAX(x_max,nodes[pids.at(i)]->x);
        y_max=MAX(y_max,nodes[pids.at(i)]->y);
        z_max=MAX(z_max,nodes[pids.at(i)]->z);
    }
    x_min=x_min-spin_zoomin->value();
    y_min=y_min-spin_zoomin->value();
    z_min=z_min-spin_zoomin->value();
    x_max=x_max+spin_zoomin->value();
    y_max=y_max+spin_zoomin->value();
    z_max=z_max+spin_zoomin->value();
    if(p_img4d!=0){
        v3dhandle winhandle = getImageWindow();
        if(winhandle==0){
            winhandle = checkImageWindow();
        }
        //reset ROI
        ROIList pRoiList=callback->getROI(winhandle);
        for(int j=0;j<3;j++){
            pRoiList[j].clear();
        }
        pRoiList[0] << QPoint(x_min,y_min);
        pRoiList[0] << QPoint(x_max,y_min);
        pRoiList[0] << QPoint(x_max,y_max);
        pRoiList[0] << QPoint(x_min,y_max);
        pRoiList[1] << QPoint(z_min,y_min);
        pRoiList[1] << QPoint(z_max,y_min);
        pRoiList[1] << QPoint(z_max,y_max);
        pRoiList[1] << QPoint(z_min,y_max);
        pRoiList[2] << QPoint(x_min,z_min);
        pRoiList[2] << QPoint(x_max,z_min);
        pRoiList[2] << QPoint(x_max,z_max);
        pRoiList[2] << QPoint(x_min,z_max);

        if(callback->setROI(winhandle,pRoiList)){
            callback->updateImageWindow(winhandle);
        }else{
            qDebug()<<"error: failed to set ROI";
            return;
        }
        callback->closeROI3DWindow(winhandle);
        callback->openROI3DWindow(winhandle);

        V3dR_MainWindow * mainwin_roi = 0;
        //qDebug()<<"===========ROI============= searching: "<<winname_roi;
        QList <V3dR_MainWindow *> tmpwinlist = callback->getListAll3DViewers();
        for(int j=0; j<tmpwinlist.size(); j++){
            //qDebug()<<"===========ROI============= "<<callback->getImageName(tmpwinlist[j]);
            if(callback->getImageName(tmpwinlist[j])==winname_roi){
                mainwin_roi = tmpwinlist[j];
                break;
            }
        }
        if(!mainwin_roi){
            v3d_msg("Error: failed open ROI window");
            return;
        }

        //qDebug()<<"===========ROI============= update ROI window with triview";
        View3DControl * v3dlocalcontrol = callback->getLocalView3DControl(winhandle);
        v3dlocalcontrol->updateLandmark();

        QList <NeuronTree> * p_nttmp = callback->getHandleNeuronTrees_Any3DViewer(mainwin_roi);
        if(p_nttmp->size()<=0 || p_nttmp->at(0).listNeuron.size()!=nt.listNeuron.size()){
            //qDebug()<<"===========ROI============= copy neuron tree to 3Dview";
            p_nttmp->clear();
            p_nttmp->push_back(nt);
            callback->update_NeuronBoundingBox(mainwin_roi);
        }else{
            //qDebug()<<"===========ROI============= copy neuron tree type to 3Dview";
            for(V3DLONG i=0; i<nt.listNeuron.size(); i++){
                (*p_nttmp)[0].listNeuron[i].type = nt.listNeuron.at(i).type;
            }
        }
        callback->update_3DViewer(mainwin_roi);
    }else{
        qDebug()<<"===========ROI============= init data";
        V3DLONG winsize[4];
        winsize[0]=x_max-x_min;
        winsize[1]=y_max-y_min;
        winsize[2]=z_max-z_min;
        winsize[3]=1;
        //image
        Image4DSimple * tmp_image = new Image4DSimple();
        unsigned char * p_img = new unsigned char[winsize[0]*winsize[1]*winsize[2]*winsize[3]];
        memset(p_img,0,winsize[0]*winsize[1]*winsize[2]*winsize[3]);
        tmp_image->setFileName(WINNAME_ASSEM);
        tmp_image->setData(p_img, winsize[0], winsize[1], winsize[2], winsize[3], (ImagePixelType)1);
        //neuron SWC
        NeuronTree local_nt;
        local_nt.editable=nt.editable;
        local_nt.file=nt.file;
        local_nt.linemode=nt.linemode;
        local_nt.n=nt.n;
        local_nt.on=nt.on;
        local_nt.selected=nt.selected;
        local_nt.name=WINNAME_ASSEM;
        local_nt.listNeuron.clear();
        local_nt.hashNeuron.clear();
        local_nt.color.r = local_nt.color.g = local_nt.color.b = local_nt.color.a = 0;
        for(int i=0; i<nt.listNeuron.size(); i++){
            NeuronSWC S;
            S.n = nt.listNeuron[i].n;
            S.type = nt.listNeuron[i].type;
            S.x = nt.listNeuron[i].x-x_min-1;
            S.y = nt.listNeuron[i].y-y_min-1;
            S.z = nt.listNeuron[i].z-z_min-1;
            S.r = nt.listNeuron[i].r;
            S.pn = nt.listNeuron[i].pn;
            local_nt.listNeuron.append(S);
            local_nt.hashNeuron.insert(S.n, local_nt.listNeuron.size()-1);
        }
        //markers
        LandmarkList local_landmark;
        LandmarkList* mList = getMarkerList();
        for(int i=0; i<mList->size(); i++){
            LocationSimple SP;
            SP.x=mList->at(i).x-x_min;
            SP.y=mList->at(i).y-y_min;
            SP.z=mList->at(i).z-z_min;
            SP.color.r=mList->at(i).color.r;
            SP.color.g=mList->at(i).color.g;
            SP.color.b=mList->at(i).color.b;
            SP.color.a=mList->at(i).color.a;
            SP.comments=mList->at(i).comments;
            SP.name=mList->at(i).name;
            local_landmark.append(SP);
        }
        //push object into the window
        //locate 3d window
        v3dhandleList allWindowList = callback->getImageWindowList();
        v3dhandle localwin = 0;
        for (V3DLONG i=0;i<allWindowList.size();i++)
        {
            if(callback->getImageName(allWindowList.at(i))==winname_main){
                localwin = allWindowList[i];
                break;
            }
        }
        if(localwin == 0){
            localwin = callback->newImageWindow(WINNAME_ASSEM);
            callback->setImage(localwin, tmp_image);
            callback->setLandmark(localwin, local_landmark);
            callback->updateImageWindow(localwin);
            callback->open3DWindow(localwin);
            callback->pushObjectIn3DWindow(localwin);

            QList<NeuronTree> * local_ntList = callback->getHandleNeuronTrees_3DGlobalViewer(localwin);
            local_ntList->clear();
            local_ntList->push_back(local_nt);
            V3dR_MainWindow * local3dwin = callback->find3DViewerByName(winname_roi);
            if(local3dwin)
                callback->update_3DViewer(local3dwin);
        }else{
            callback->setImage(localwin, tmp_image);
            callback->setLandmark(localwin, local_landmark);
            callback->updateImageWindow(localwin);
            callback->open3DWindow(localwin);
            callback->pushImageIn3DWindow(localwin);
            callback->pushObjectIn3DWindow(localwin);

            QList<NeuronTree> * local_ntList = callback->getHandleNeuronTrees_3DGlobalViewer(localwin);
            local_ntList->clear();
            local_ntList->push_back(local_nt);
            V3dR_MainWindow * local3dwin = callback->find3DViewerByName(winname_roi);
            if(local3dwin)
                callback->update_3DViewer(local3dwin);
        }
    }
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
    tmp+=(y_max-y_min)*(y_max-y_min);
    tmp+=(z_max-z_min)*(z_max-z_min);
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

//~~~~~~~~~~sort param dialog~~~~~~~~~~~
sort_neuron_dialog::sort_neuron_dialog(LandmarkList * mList, V3DLONG prev_root, V3DLONG type1_root, QWidget *parent)
{
    QGridLayout * layout = new QGridLayout(this);

    QGroupBox *group_root = new QGroupBox("Define root");
    radio_marker = new QRadioButton("Select a marker as root: ");
    radio_type = new QRadioButton("Pick a node with type 1 (soma) as root (Node " + QString::number(type1_root) + ")" );
    radio_prev = new QRadioButton("Use the first root in original input SWC file (Node " + QString::number(prev_root) + ")");
    cb_marker = new QComboBox();
    QGridLayout * grouplayout = new QGridLayout();
    grouplayout->addWidget(radio_marker, 0, 0, 1, 1);
    grouplayout->addWidget(cb_marker, 0, 1, 1, 1);
    grouplayout->addWidget(radio_type, 1, 0, 1, 2);
    grouplayout->addWidget(radio_prev, 2, 0, 1, 2);
    group_root->setLayout(grouplayout);
    layout->addWidget(group_root, 1, 0, 1, 2);

    btn_yes = new QPushButton("Start Sort");
    btn_no = new QPushButton("Cancel");
    layout->addWidget(btn_yes, 2,0,1,1);
    layout->addWidget(btn_no, 2,1,1,1);

    int disable_count=0;

    for(V3DLONG i=0; i<mList->size(); i++){
        QList<int> info;
        get_marker_info(mList->at(i), info);
        QString tmp="Marker "+QString::number(i+1)+"; Node "+QString::number(info.at(0));
        cb_marker->addItem(tmp);
    }
    if(mList->size()>0){
        cb_marker->setCurrentIndex(0);
        radio_marker->setChecked(true);
    }else{
        cb_marker->setEnabled(false);
        radio_marker->setEnabled(false);
        disable_count++;
    }

    if(type1_root<0){
        radio_type->setEnabled(false);
        disable_count++;
    }else{
        radio_type->setChecked(true);
    }

    if(prev_root<0){
        radio_prev->setEnabled(false);
        disable_count++;
    }else{
        radio_prev->setChecked(true);
    }

    if(disable_count>=3){
        btn_yes->setDisabled(true);
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

QList<NOI*> search_loop(QHash<V3DLONG, NOI *> &nodes)
{
    QList<NOI*> loop;
    QHash<NOI*, NOI*> hash_child_parent;
    for(NodeIter iter=nodes.begin(); iter!=nodes.end(); iter++){
        if(!hash_child_parent.contains(iter.value())){
            QStack<NOI*> stack_seeds;
            hash_child_parent.insert(iter.value(),0);
            stack_seeds.push_back(iter.value());
            while(!stack_seeds.isEmpty()){
                NOI* cur_node=stack_seeds.pop();
                for(QSet<NOI*>::Iterator niter=cur_node->conn.begin(); niter!=cur_node->conn.end(); niter++){
                    if((*niter)==hash_child_parent[cur_node])
                        continue;
                    if(hash_child_parent.contains(*niter)){ //found the loop!!
                        NOI* p_node=cur_node;
                        NOI* conflict_node=hash_child_parent[*niter];
                        while(p_node!=0 && p_node!=conflict_node){
                            loop.push_back(p_node);
                            p_node=hash_child_parent[p_node];
                        }
                        if(p_node==0){//should not happen
                            v3d_msg("Encounter unexpected error 3. Plase contact developer.");
                            return loop;
                        }else{
                            loop.push_back(p_node);
                            loop.push_back(*niter);
                            break;
                        }
                    }
                    hash_child_parent.insert(*niter,cur_node);
                    stack_seeds.push_back(*niter);
                }
                if(loop.size()>0)
                    break;
            }
        }
        if(loop.size()>0)
            break;
    }

    return loop;
}

QList<NeuronSWC> generate_swc_typesort(QHash<V3DLONG, NOI*>& nodes, V3DLONG n_root)
{
    //init components root
    QList<int> components;
    QList<V3DLONG> c_roots;
    components.push_back(nodes[n_root]->cid);
    c_roots.push_back(n_root);
    for(NodeIter iter=nodes.begin(); iter!=nodes.end(); iter++){
        if(!components.contains(iter.value()->cid)){
            components.push_back(iter.value()->cid);
            c_roots.push_back(iter.value()->n);
        }else if(iter.value()->type==1){
            c_roots[components.indexOf(iter.value()->cid)]=iter.value()->n;
        }
    }

    //put the root nodes on the top of the SWC
    QHash<V3DLONG, V3DLONG> hash_old_new;
    QList<NeuronSWC> neuronList;
    for(int i=0; i<c_roots.size(); i++){
        hash_old_new.insert(c_roots.at(i),neuronList.size()+1);
        NeuronSWC S;
        S.n=neuronList.size()+1;
        S.x=nodes[c_roots.at(i)]->x;
        S.y=nodes[c_roots.at(i)]->y;
        S.z=nodes[c_roots.at(i)]->z;
        S.type=nodes[c_roots.at(i)]->type;
        S.r=nodes[c_roots.at(i)]->r;
        S.pn=-1;
        neuronList.push_back(S);
    }

    //start grow from each node by type
    QQueue<NOI*> seeds_cur;
    QList<QQueue<QPair<NOI*,V3DLONG> > > seeds_next;
    for(int i=0; i<c_roots.size(); i++){
        QQueue<QPair<NOI*,V3DLONG> > seeds_tmp;
        seeds_tmp.append(QPair<NOI*,V3DLONG>(nodes[c_roots.at(i)],-1));
        seeds_next.push_back(seeds_tmp);
    }
    for(int type=1; type<20; type++){ //sort with type priority
        int emptycount=0;
        for(int cidx=0; cidx<seeds_next.size(); cidx++){
            QQueue<QPair<NOI*,V3DLONG> > seeds_pre=seeds_next[cidx];
            seeds_next[cidx].clear();
            if(seeds_pre.isEmpty()){
                emptycount++;
                continue;
            }
            seeds_cur.clear();
            while(!seeds_pre.isEmpty()){
                QPair<NOI*,V3DLONG> pair_cur = seeds_pre.dequeue();
                if(pair_cur.first->type > type){
                    seeds_next[cidx].append(pair_cur);
                    continue;
                }
                if(!hash_old_new.contains(pair_cur.first->n)){
                    hash_old_new.insert(pair_cur.first->n,neuronList.size()+1);
                    NeuronSWC S;
                    S.n=neuronList.size()+1;
                    S.x=pair_cur.first->x;
                    S.y=pair_cur.first->y;
                    S.z=pair_cur.first->z;
                    S.type=pair_cur.first->type;
                    S.r=pair_cur.first->r;
                    S.pn=pair_cur.second;
                    neuronList.push_back(S);
                }
                seeds_cur.append(pair_cur.first);
            }

            while(!seeds_cur.isEmpty()){
                NOI* node_cur=seeds_cur.dequeue();
                for(QSet<NOI*>::Iterator niter=node_cur->conn.begin(); niter!=node_cur->conn.end(); niter++){
                    if(hash_old_new.contains((*niter)->n))
                        continue;
                    if((*niter)->type<=type){
                        hash_old_new.insert((*niter)->n,neuronList.size()+1);
                        NeuronSWC S;
                        S.n=neuronList.size()+1;
                        S.x=(*niter)->x;
                        S.y=(*niter)->y;
                        S.z=(*niter)->z;
                        S.type=(*niter)->type;
                        S.r=(*niter)->r;
                        S.pn=hash_old_new[node_cur->n];
                        neuronList.push_back(S);
                        seeds_cur.push_back(*niter);
                    }else{
                        QPair<NOI*,V3DLONG> pair_cur(*niter, hash_old_new[node_cur->n]);
                        seeds_next[cidx].append(pair_cur);
                    }
                }
            }
        }
        if(emptycount>=seeds_next.size()){
            break;
        }
    }

    return neuronList;
}

bool export_list2file(const QList<NeuronSWC>& lN, QString fileSaveName)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;
    QTextStream myfile(&file);
    myfile<<"# generated by Vaa3D Plugin assemble_neuron_live"<<endl;
    myfile<<"# date "<<QDate::currentDate().toString("yyyy.MM.dd")<<endl;
    myfile<<"# id,type,x,y,z,r,pid"<<endl;
    for (V3DLONG i=0;i<lN.size();i++)
        myfile << lN.at(i).n <<" " << lN.at(i).type << " "<< lN.at(i).x <<" "<<lN.at(i).y << " "<< lN.at(i).z << " "<< lN.at(i).r << " " <<lN.at(i).pn << "\n";

    file.close();
    return true;
}
