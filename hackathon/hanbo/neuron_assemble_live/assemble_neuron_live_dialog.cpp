#include "assemble_neuron_live_dialog.h"

assemble_neuron_live_dialog::assemble_neuron_live_dialog(V3DPluginCallback2 * cb, QList<NeuronTree> &ntList, Image4DSimple * p_img4d_in, QWidget *parent) :
    QDialog(parent)
{
    callback = cb;

    p_img4d = p_img4d_in;

    noffset=0;
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
    layout->addWidget(cb_color,2,1,1,1);

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
    for(QSet<QPair<V3DLONG,V3DLONG> >::iterator iter = links.begin(); iter!=links.end(); iter++){
        V3DLONG pid1, pid2;
        pid1=iter->first;
        pid2=iter->second;
        int mid1 = markerTable[pid1]+1;
        int mid2 = markerTable[pid2]+1;
        QString tmp;
        if(mid1<mid2){
            tmp = "Marker "+QString::number(mid1)+" x "+
                    QString::number(mid2)+" (Node "+QString::number(pid1)+" x "+
                    QString::number(pid2)+" )";
        }else{
            tmp = "Marker "+QString::number(mid2)+" x "+
                    QString::number(mid1)+" (Node "+QString::number(pid2)+" x "+
                    QString::number(pid1)+" )";
        }
        list_link->addItem(tmp);
    }
    if(list_link->count()>0){
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

void assemble_neuron_live_dialog::updateROIWindow()
{

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
