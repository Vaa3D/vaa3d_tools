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

    tab = new QTabWidget(this);
    //neuron connector
    QDialog * dialog_conn = new QDialog(tab);
    QGridLayout * layout_conn = new QGridLayout();
    list_link = new QListWidget();
    btn_connect = new QPushButton("connect select pair");
    btn_manuallink = new QPushButton("add/del connection pair");
    layout_conn->addWidget(list_link,0,0,6,2);
    layout_conn->addWidget(btn_connect,0,2,1,1);
    layout_conn->addWidget(btn_manuallink,1,2,1,1);
    dialog_conn->setLayout(layout_conn);
    tab->addTab(dialog_conn,tr("connect"));

    //neuron breaker
    QDialog * dialog_break = new QDialog(tab);
    QGridLayout * layout_break = new QGridLayout();
    list_marker = new QListWidget();
    btn_break = new QPushButton("break at select");
    btn_syncmarker = new QPushButton("update markers");
    layout_break->addWidget(list_marker,0,0,6,2);
    layout_break->addWidget(btn_break,0,2,1,1);
    layout_break->addWidget(btn_syncmarker,1,2,1,1);
    dialog_break->setLayout(layout_break);
    tab->addTab(dialog_break,tr("break"));

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

void assemble_neuron_live_dialog::updateDisplay()
{
    if(p_img4d!=0){
        updateImageWindow();
        updateROIWindow();
    }

    update3DWindow();
}

void assemble_neuron_live_dialog::updateImageWindow()
{
    v3dhandleList allWindowList = callback->getImageWindowList();
    v3dhandle winhandle = 0;
    for (V3DLONG i=0;i<allWindowList.size();i++)
    {
        if(callback->getImageName(allWindowList.at(i)).contains(WINNAME_ASSEM)){
            winhandle = allWindowList[i];
            break;
        }
    }
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
}

void assemble_neuron_live_dialog::update3DWindow()
{
    V3dR_MainWindow * _3dwin = 0;
    QList <V3dR_MainWindow *> list_3dwin = callback->getListAll3DViewers();
    for(int i=0; i<list_3dwin.size(); i++){
        if(callback->getImageName(list_3dwin[i]).contains(WINNAME_ASSEM)){
            _3dwin = list_3dwin[i];
        }
    }
    if(_3dwin){
        QList<NeuronTree> * tmp_ntList = callback->getHandleNeuronTrees_Any3DViewer(_3dwin);
        tmp_ntList->clear();
        tmp_ntList->push_back(nt);
        callback->update_3DViewer(_3dwin);
        callback->update_NeuronBoundingBox(_3dwin);
    }else{
        _3dwin = callback->createEmpty3DViewer();
        callback->setWindowDataTitle(_3dwin, WINNAME_ASSEM);
        QList<NeuronTree> * tmp_ntList = callback->getHandleNeuronTrees_Any3DViewer(_3dwin);
        tmp_ntList->clear();
        tmp_ntList->push_back(nt);
        callback->update_3DViewer(_3dwin);
        callback->update_NeuronBoundingBox(_3dwin);
    }

//    //for test
//    qDebug()<<nt.listNeuron.size();
//    for(int i=0; i<nt.listNeuron.size(); i++){
//        qDebug()<<nt.listNeuron.at(i).n<<" "<<nt.listNeuron.at(i).type<<" "<<nt.listNeuron.at(i).x<<" "<<nt.listNeuron.at(i).y<<" "
//               <<nt.listNeuron.at(i).z<<" "<<nt.listNeuron.at(i).type<<" "
//               <<nt.listNeuron.at(i).pn;
//    }
}

void assemble_neuron_live_dialog::updateROIWindow()
{

}
