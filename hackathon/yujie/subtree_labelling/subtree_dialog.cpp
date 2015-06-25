#include "subtree_dialog.h"
#include "sort_swc.h"

#define swc_win_name "subtree_labelling_swc"

#ifndef VOID
#define VOID 1000000000
#endif

subtree_dialog::subtree_dialog(V3DPluginCallback2 *cb)
{
    callback=cb;
}

//bool subtree_dialog::sort_swc_dialog()
//{
//    QDialog *sort_dialog = new QDialog;
//    QGridLayout *mylayout=new QGridLayout;
//    QLabel *sort_info = new QLabel(tr("Before labelling subtree please sort the swc file first. If you have done so,"
//                                      "please skip this step"));
//    QPushButton *btn_sort= new QPushButton(tr("sort"));
//    QPushButton *btn_skip= new QPushButton(tr("skip"));
//    mylayout->addWidget(sort_info,0,0,1,4);
//    mylayout->addWidget(btn_sort,2,0,1,2);
//    mylayout->addWidget(btn_skip,2,2,1,2);
//    sort_dialog->setLayout(mylayout);
//    connect(btn_sort,SIGNAL(clicked()),this,SLOT(sort_slot()));
//    connect(btn_skip,SIGNAL(clicked()),this,SLOT(skip_slot()));
//}

//void subtree_dialog::skip_slot()
//{
//    backupNeuron(*nt,*sorted_nt);
//}

//bool subtree_dialog::sort_slot()
//{
//    QList<NeuronSWC> neuron = nt->listNeuron;

//    V3DLONG rootid;
//    V3DLONG thres;
//    bool ok;
//    rootid = QInputDialog::getInteger(0, "Would you like to specify new root number?","New root number:(If you select 'cancel', the first root in file is set as default)",1,1,neuron.size(),1,&ok);
//    if (!ok)
//            rootid = VOID;
//    thres = QInputDialog::getDouble(0, "Would you like to set a threshold for the newly generated link?","threshold:(If you select 'cancel', all the points will be connected automated; If you set '0', no new link will be generated)",0,0,2147483647,1,&ok);
//    if (!ok)
//            thres = VOID;

//    if (!SortSWC(neuron, result ,rootid, thres))
//    {
//        v3d_msg("Sorting error!");
//        return false;
//    }
//    QHash <int, int>  hashNeuron;
//    for (int i=0;i<result.size();i++)
//    {
//        result[i].level=result[i].seg_id=0;
//        result[i].fea_val.clear();
//        result[i].fea_val.append(0);
//        result[i].fea_val.append(0);
//        hashNeuron.insert(result[i].n,result.size()-1);
//    }
//    sorted_nt->hashNeuron=hashNeuron;

//    QMessageBox::information(0,"Sort complete","",QMessageBox::Ok);

//    return true;
//}


bool subtree_dialog::get_swc_marker()
{
    //v3dhandleList v3dhandleList_current=callback->getImageWindowList();
    QList <V3dR_MainWindow *> cur_list_3dviewer = callback->getListAll3DViewers();
    QList <V3dR_MainWindow *> used_list_3dviwer;
    QStringList items;

    for (int i=0;i<cur_list_3dviewer.size();i++)
    {
        QList<NeuronTree> *swc_list;
        swc_list=callback->getHandleNeuronTrees_Any3DViewer(cur_list_3dviewer[i]);
        if (swc_list->size()==1)
        {
            used_list_3dviwer.push_back(cur_list_3dviewer[i]);
            items << callback->getImageName(cur_list_3dviewer[i]);
        }
    }

    if (items.size()==0){
        v3d_msg("No window with single swc is found");
        return false;
    }
    else if (items.size()==1)
    {
        swc_win=used_list_3dviwer[0];
        QList<NeuronTree> *tmp_nt;
        tmp_nt=callback->getHandleNeuronTrees_Any3DViewer(swc_win);
        nt=(NeuronTree *)&((*tmp_nt)[0]);
        //backup nt
        //backupNeuron((*tmp_nt)[0],*nt);

        //check swc file
        if (nt->listNeuron.size()<=0)
        {
            v3d_msg("Error loading swc file, please check swc file");
            return false;
        }
        for (V3DLONG ii=0; ii<nt->listNeuron.size(); ii++)
        {
            if(nt->listNeuron.at(ii).r<0)
            {
                v3d_msg("You have illeagal radius values. Check your data.");
                return false;
            }
            else
            {
                nt->listNeuron[ii].type=nt->listNeuron[ii].seg_id=nt->listNeuron[ii].level=0;
                nt->listNeuron[ii].fea_val.append(0); //store dendrite id
                nt->listNeuron[ii].fea_val.append(0); //store distance to node
            }
         }

        //get markers and check markers
        //qDebug()<<"Only 1 window open";
        LList_in.clear();
        LList_in=*(callback->getHandleLandmarkList_Any3DViewer(swc_win));
        if (LList_in.size()==0)
        {
            v3d_msg("Please load markers");
            return false;
        }

    }
    else if (items.size()>1)
    {
        QDialog *mydialog=new QDialog;
        QComboBox *combo=new QComboBox();
        combo->insertItems(0,items);
        QLabel *label_win=new QLabel;
        label_win->setText("You have multiple windows open, please select one image:");
        QGridLayout *layout= new QGridLayout;
        layout->addWidget(label_win,0,0,1,1);
        layout->addWidget(combo,1,0,4,1);
        QPushButton *button_d_ok=new QPushButton;
        button_d_ok->setText("Ok");
        button_d_ok->setFixedWidth(100);
        QPushButton *button_d_cancel=new QPushButton;
        button_d_cancel->setText("Cancel");
        button_d_cancel->setFixedWidth(100);
        QHBoxLayout *box=new QHBoxLayout;
        box->addWidget(button_d_ok,Qt::AlignCenter);
        box->addWidget(button_d_cancel,Qt::AlignCenter);
        layout->addLayout(box,5,0,1,1);
        connect(button_d_ok,SIGNAL(clicked()),mydialog,SLOT(accept()));
        connect(button_d_cancel,SIGNAL(clicked()),mydialog,SLOT(reject()));
        mydialog->setLayout(layout);
        mydialog->exec();
        if (mydialog->result()==QDialog::Accepted)
        {
            int tmp=combo->currentIndex();
            swc_win=used_list_3dviwer[tmp];
        }
        else
        {
            v3d_msg("You have not selected a window");
            return false;
        }

        //check swc
        QList<NeuronTree> *tmp_nt;
        tmp_nt=callback->getHandleNeuronTrees_Any3DViewer(swc_win);
        nt=(NeuronTree *)&((*tmp_nt)[0]);
        //backupNeuron((NeuronTree *)&(tmp_nt->at(0)),nt);
//        nt.copy(tmp_nt->at(0));
//        nt.copyGeometry(tmp_nt->at(0));

        if (nt->listNeuron.size()<=0)
        {
            v3d_msg("Error loading swc file, please check swc file");
            return false;
        }
        for (V3DLONG ii=0; ii<nt->listNeuron.size(); ii++)
        {
            if(nt->listNeuron.at(ii).r<0)
            {
                v3d_msg("You have illeagal radius values. Check your data.");
                return false;
            }
            else //initialize
            {
                nt->listNeuron[ii].type=nt->listNeuron[ii].seg_id=nt->listNeuron[ii].level=0;
                nt->listNeuron[ii].fea_val.append(0); //store dendrite id
                nt->listNeuron[ii].fea_val.append(0); //store distance to node
            }
         }

        //get markers and check markers
        LList_in.clear();
        LList_in=*(callback->getHandleLandmarkList_Any3DViewer(swc_win));
        if (LList_in.size()==0)
        {
            v3d_msg("Please load markers");
            return false;
        }
    }
    qDebug()<<"swc set: nt size:"<<nt->listNeuron.size();
    return true;
}

void subtree_dialog::assign_marker_type()
{
    mydialog2=new QDialog;
    QGridLayout *layout2=new QGridLayout;
    QLabel *info=new QLabel(tr("Please assign type for each segment denoted by markers"));
    layout2->addWidget(info,0,0,2,6);

    markers=new QComboBox();
    for (int i=0;i<LList_in.size();i++)
        markers->addItem(QString("marker ")+QString::number(i+1));
    markers->setFixedWidth(230);
    markers->setCurrentIndex(0);
    layout2->addWidget(markers,2,0,1,4);
    QPushButton *refresh=new QPushButton(tr("Refresh markers"));
    layout2->addWidget(refresh,2,4,1,2);

    QPushButton *soma=new QPushButton(tr("soma"));
    QPushButton *axon=new QPushButton(tr("axon"));
    QPushButton *apical_dendrite = new QPushButton(tr("apical_dendrite"));
    QPushButton *basal_dendrite =new QPushButton(tr("basal dendrite"));
    QPushButton *oblique = new QPushButton(tr("oblique dendrite"));
    // 0-Undefined, 1-Soma, 2-Axon, 3-Dendrite(oblique), 4-Apical_dendrite, 5-Fork_point,
    //6-End_point, 7-basal_dendrite

    layout2->addWidget(soma,3,0,1,2);
    layout2->addWidget(axon,3,2,1,2);
    layout2->addWidget(apical_dendrite,4,0,1,2);
    layout2->addWidget(basal_dendrite,4,2,1,2);
    layout2->addWidget(oblique,4,4,1,2);

    QFrame *line_3 = new QFrame();
    line_3->setFrameShape(QFrame::HLine);
    line_3->setFrameShadow(QFrame::Sunken);
    layout2->addWidget(line_3,6,0,1,6);

    QPushButton *ok     = new QPushButton("Finish and save");
    QPushButton *cancel = new QPushButton("Quit");
    layout2->addWidget(ok,10,1,1,2);
    layout2->addWidget(cancel,10,3,1,2);

    connect(soma,SIGNAL(clicked()),this,SLOT(soma_clicked()));
    connect(axon,SIGNAL(clicked()),this,SLOT(axon_clicked()));
    connect(apical_dendrite,SIGNAL(clicked()),this,SLOT(apical_dendrite_clicked()));
    connect(basal_dendrite,SIGNAL(clicked()),this,SLOT(basal_clicked()));
    connect(oblique,SIGNAL(clicked()),this,SLOT(oblique_clicked()));
    connect(cancel,SIGNAL(clicked()),this,SLOT(maybe_save()));
    connect(refresh,SIGNAL(clicked()),this,SLOT(refresh_marker()));
    connect(ok,SIGNAL(clicked()),this,SLOT(save()));

    mydialog2->setWindowTitle("Type assignment");
    mydialog2->setLayout(layout2);
    mydialog2->show();
}


bool subtree_dialog::subtree_extract()
{
    int size=nt->listNeuron.size();
    subtree.resize(size);
    for (int i=0;i<size;i++)
    {
        int parent_id=nt->listNeuron.at(i).parent;
        if (parent_id==-1) continue;
        subtree[nt->hashNeuron.value(parent_id)].push_back(i);
    }
//    for (int i=0;i<subtree.size();i++)
//    {
//        vector<int> tmp=subtree[i];
//        for (int j=0;j<tmp.size();j++)
//        {
//            qDebug()<<"i:"<<i+1<< " j:"<<j+1<<" members:"<<tmp[j]+1;
//        }
//    }
    return true;
}

void subtree_dialog::build_connt_LUT()
{
    int size=nt->listNeuron.size();
    subtree.resize(size);
    for (int i=0;i<size;i++)
    {
        int parent_id=nt->listNeuron.at(i).parent;
        if (parent_id==-1) continue;
        subtree[nt->hashNeuron.value(parent_id)].push_back(i);
        subtree[i].push_back(nt->hashNeuron.value(parent_id));
    }

//    for (int i=0;i<subtree.size();i++)
//    {
//        vector<int> tmp=subtree[i];
//        for (int j=0;j<tmp.size();j++)
//        {
//            qDebug()<<"i:"<<i+1<< " j:"<<j+1<<" members:"<<tmp[j]+1;
//        }
//    }
}

void subtree_dialog::marker_change()
{
    if(markers->count()==0) return;
    // if this markers is not determined,Landmark color change
//    if (LList_in[mid].comments.empty())
//    {
//        LList_in[mid].color.r=LList_in[mid].color.b=255;
//        LList_in[mid].color.g=70;
//    }

}

void subtree_dialog::soma_clicked()
{
    qDebug()<<"soma clicked";
    int mid=markers->currentIndex();
//    int marker_id=calc_nearest_node_around_marker();
//    nt->listNeuron[marker_id].type=1;
    markers->setItemText(mid, "marker "+QString::number(mid+1)+ " soma");
    LList_in[mid].color.r=LList_in[mid].color.g=LList_in[mid].color.b=0;
    LList_in[mid].comments="soma";
    LList_in[mid].name=QString::number(1).toStdString();

}

void subtree_dialog::axon_clicked()
{
    int mid=markers->currentIndex();
//    int marker_id=calc_nearest_node_around_marker();
//    sort_type_def(2,(float)(mid+1),marker_id);
    markers->setItemText(mid, "marker "+QString::number(mid+1)+ " axon");
//    LList_in[mid].color.r=255;
//    LList_in[mid].color.g=LList_in[mid].color.b=0;
    LList_in[mid].comments="axon";
    LList_in[mid].name=QString::number(2).toStdString();
}

void subtree_dialog::apical_dendrite_clicked()
{
    int mid=markers->currentIndex();
//    int marker_id=calc_nearest_node_around_marker();
//    sort_type_def(4,(float)(mid+1),marker_id);
    markers->setItemText(mid, "marker "+QString::number(mid+1)+ " apical dendrite");
//    LList_in[mid].color.r=LList_in[mid].color.g=LList_in[mid].color.b=0;
    LList_in[mid].comments="apical dendrite";
    LList_in[mid].name=QString::number(4).toStdString();
}

void subtree_dialog::basal_clicked()
{
    int mid=markers->currentIndex();
//    int marker_id=calc_nearest_node_around_marker();
//    sort_type_def(7,(float)(mid+1),marker_id);
    markers->setItemText(mid, "marker "+QString::number(mid+1)+ " basal dendrite");
//    LList_in[mid].color.g=255;
//    LList_in[mid].color.r=LList_in[mid].color.b=0;
    LList_in[mid].comments="basal dendrite";
    LList_in[mid].name=QString::number(7).toStdString();
}

void subtree_dialog::oblique_clicked()
{
    int mid=markers->currentIndex();
//    int marker_id=calc_nearest_node_around_marker();
//    sort_type_def(3,(float)(mid+1),marker_id);
    markers->setItemText(mid, "marker "+QString::number(mid+1)+ " oblique dendrite");
//    LList_in[mid].color.b=255;
//    LList_in[mid].color.r=LList_in[mid].color.g=0;
    LList_in[mid].comments="oblique dendrite";
    LList_in[mid].name=QString::number(3).toStdString();
}

void subtree_dialog::check_window()
{
    bool window_open_flag=false;
    QList <V3dR_MainWindow *> allWindowList = callback->getListAll3DViewers();
    for (V3DLONG i=0;i<allWindowList.size();i++)
    {
        if(callback->getImageName(allWindowList.at(i)).contains(swc_win_name))
        {
            window_open_flag=true;
            swc_win=allWindowList[i];
            break;
        }
    }

    if (!window_open_flag)
    {
        swc_win=callback->createEmpty3DViewer();

        //need a way to reset neurontree and landmarks.
    }
}

int subtree_dialog::calc_nearest_node_around_marker()
{
    int mid=markers->currentIndex();
    V3DLONG x,y,z;
    x=LList_in.at(mid).x-1;
    y=LList_in.at(mid).y-1;
    z=LList_in.at(mid).z-1;
    float min_dis=1e16;
    float dis;
    int close_node;
    for (int i=0;i<nt->listNeuron.size();i++)
    {
        dis=(nt->listNeuron.at(i).x-x)*(nt->listNeuron.at(i).x-x)+
                (nt->listNeuron.at(i).y-y)*(nt->listNeuron.at(i).y-y)+
                (nt->listNeuron.at(i).z-z)*(nt->listNeuron.at(i).z-z);
        if (dis<min_dis)
        {
            min_dis=dis;
            close_node=i;
        }
    }
    return close_node;
}

void subtree_dialog::sort_type_def(int type, float dendrite_id, int marker_id)
{
    qDebug()<<"in type def"<<dendrite_id<<":"<<type<<":"<<marker_id<<":"<<nt->listNeuron.size();
    vector<int> seeds,seeds_next;
    V3DLONG seg_id=1;
    V3DLONG level=1;
    map<int,bool> mark_used;
    nt->listNeuron[marker_id].fea_val[0]=dendrite_id;
    nt->listNeuron[marker_id].type=type;
    nt->listNeuron[marker_id].seg_id=seg_id;
    nt->listNeuron[marker_id].level=level;
    nt->listNeuron[marker_id].parent=-1;
    mark_used[marker_id]=1;
    seeds.push_back(marker_id);

    //qDebug()<<"first seeds set:"<<seeds.size();

    int sid=0;
    V3DLONG parent;
    while (sid<seeds.size())
    {
        qDebug()<<"seeds number:"<<seeds[sid]<<"seeds.size:"<<seeds.size();
        //check how many children the seed has, if more than one, seg_id reset,level++
        //the forked node belongs to the previous seg (has the same property has parent)
        //qDebug()<<"sid:"<<sid;
        seeds_next.clear();
        parent=seeds[sid];

        for (int i=0;i<subtree[parent].size();i++)
        {
            if (mark_used[subtree[parent][i]]==0)
            {
                //qDebug()<<"parent:"<<parent<<"child:"<< subtree[parent][i];
                seeds_next.push_back(subtree[parent][i]);
            }
        }
        qDebug()<<"parent:"<<parent<< "fea value:"<< nt->listNeuron.at(parent).fea_val.at(1)<<"seg_id:"<< nt->listNeuron.at(parent).seg_id
                   <<"level:"<< nt->listNeuron.at(parent).level<<"children:"<<seeds_next.size();
        if (seeds_next.size()==0)
        {
            sid++;
            continue;
        }
        else if(seeds_next.size()>1 && nt->listNeuron[parent].parent==-1)
        {
            for (int i=0;i<seeds_next.size();i++)
            {
                nt->listNeuron[seeds_next[i]].parent=nt->listNeuron[parent].n;
                nt->listNeuron[seeds_next[i]].type=nt->listNeuron.at(parent).type;
                nt->listNeuron[seeds_next[i]].fea_val[0]=(nt->listNeuron.at(parent).fea_val.at(0));
                nt->listNeuron[seeds_next[i]].level=nt->listNeuron.at(parent).level;
                nt->listNeuron[seeds_next[i]].seg_id=i;
                mark_used[seeds_next[i]]=1;
                seeds.insert(seeds.end(),seeds_next.begin(),seeds_next.end());
            }
        }
        else if(seeds_next.size()==1)
        {
            int child_node=seeds_next.front();
            nt->listNeuron[child_node].parent=nt->listNeuron.at(parent).n;
            nt->listNeuron[child_node].type=nt->listNeuron.at(parent).type;
            nt->listNeuron[child_node].fea_val[0]=nt->listNeuron.at(parent).fea_val[0];
            nt->listNeuron[child_node].seg_id=nt->listNeuron.at(parent).seg_id;
            nt->listNeuron[child_node].level=nt->listNeuron.at(parent).level;
            mark_used[child_node]=1;
            seeds.push_back(child_node);
            qDebug()<<"one child__parent:"<<"child:"<<nt->listNeuron[child_node].type<<" seg id:"
                       <<nt->listNeuron[child_node].seg_id<<" level:"<<nt->listNeuron[child_node].level;
        }
        else if (seeds_next.size()>1&& nt->listNeuron[parent].parent!=-1)
        {

            for (int i=0;i<seeds_next.size();i++)
            {
                nt->listNeuron[seeds_next[i]].parent=nt->listNeuron.at(parent).n;
                nt->listNeuron[seeds_next[i]].type=nt->listNeuron.at(parent).type;
                nt->listNeuron[seeds_next[i]].fea_val[0]=(nt->listNeuron.at(parent).fea_val.at(0));
                nt->listNeuron[seeds_next[i]].level=nt->listNeuron.at(parent).level+1;
                nt->listNeuron[seeds_next[i]].seg_id=i+1;
                mark_used[seeds_next[i]]=1;
                seeds.insert(seeds.end(),seeds_next.begin(),seeds_next.end());
                qDebug()<<"2+child__parent:"<<"child "<<seeds_next[i]<< ":"<<nt->listNeuron[seeds_next[i]].type<< " seg id:"
                        <<nt->listNeuron[seeds_next[i]].seg_id<<" level:"<<nt->listNeuron[seeds_next[i]].level;
            }
        }
        sid++;
    }
    //qDebug()<<"seeds size:"<<seeds.size();

}


void backupNeuron(NeuronTree & source, NeuronTree & backup)
{
    NeuronTree *np = (NeuronTree *)(&backup);
    np->n=source.n; np->on=source.on; np->selected=source.selected; np->name=source.name; np->comment=source.comment;
    np->color.r=source.color.r; np->color.g=source.color.g; np->color.b=source.color.b; np->color.a=source.color.a;
    np->listNeuron.clear();
    for(V3DLONG i=0; i<source.listNeuron.size(); i++)
    {
        NeuronSWC S;
        S.n = source.listNeuron[i].n;
        S.type = source.listNeuron[i].type;
        S.x = source.listNeuron[i].x;
        S.y = source.listNeuron[i].y;
        S.z = source.listNeuron[i].z;
        S.r = source.listNeuron[i].r;
        S.pn = source.listNeuron[i].pn;
        S.seg_id = source.listNeuron[i].seg_id;
        S.level = source.listNeuron[i].level;
        S.fea_val = source.listNeuron[i].fea_val;
        np->listNeuron.append(S);
    }
    np->hashNeuron = source.hashNeuron;
    np->file     = source.file;
    np->editable = source.editable;
    np->linemode = source.linemode;
}


bool subtree_dialog::save()
{
    calc_distance_to_subtree_root();
    QString filename="";
    if (!writeESWC_file(filename,*nt))
        return false;
    else
    {
        mydialog2->close();
        return true;
    }
}

void subtree_dialog::refresh_marker()
{
    LandmarkList LList_new;
    if (swc_win!=0)
    {
        LList_new=*(callback->getHandleLandmarkList_Any3DViewer(swc_win));
        if (LList_new.size()==0)
        {
            v3d_msg("No markers were found");
            return;
        }
    }
    else
    {
        v3d_msg("swc window is closed");
        return;
    }

    for (int i=0;i<LList_new.size();i++)
    {
        LocationSimple tmp=LList_new[i];
        bool found_flag=false;
        for (int j=0;j<LList_in.size();j++)
        {
            if (tmp==LList_in[j])
            {
                found_flag=true;
                break;
            }
        }
        if(!found_flag)
            LList_in.append(tmp);
    }
    markers->clear();
    for (int i=0;i<LList_in.size();i++)
    {
        QString tmp_str;
        tmp_str=QString::fromStdString((LList_in[i].comments));
        if (tmp_str.contains("soma"))
            markers->addItem(QString("marker ")+QString::number(i+1)+" soma");
        else if (tmp_str.contains("axon"))
            markers->addItem(QString("marker ")+QString::number(i+1)+" axon");
        else if (tmp_str.contains("apical dendrite"))
            markers->addItem(QString("marker ")+QString::number(i+1)+" apical dendrite");
        else if (tmp_str.contains("basal dendrite"))
            markers->addItem(QString("marker ")+QString::number(i+1)+" basal dendrite");
        else if (tmp_str.contains("oblique dendrite"))
            markers->addItem(QString("marker ")+QString::number(i+1)+" oblique dendrite");
        else
            markers->addItem(QString("marker ")+QString::number(i+1));
    }

}


bool subtree_dialog::maybe_save()
{
    QMessageBox mybox;
    mybox.setText("Do you want to exit without saving?");

    QPushButton *save_button = mybox.addButton(QMessageBox::Save);
    QPushButton *cancel_button=mybox.addButton(QMessageBox::Cancel);
    QPushButton *discard_button=mybox.addButton(QMessageBox::Discard);

    mybox.setDefaultButton(QMessageBox::Save);
    mybox.exec();

     if (mybox.clickedButton() == save_button) {
         save();
         return true;
     } else if (mybox.clickedButton() == cancel_button) {
         return false;
     }
     else if (mybox.clickedButton()== discard_button) {
         mydialog2->close();
         return false;
     }
}

void subtree_dialog::calc_distance_to_subtree_root()
{
    qDebug()<<"in calc dis to soma";
    for (int i=0;i<LList_in.size();i++)
    {
        QString tmp;
        tmp=QString::fromStdString(LList_in[i].name);
        if (tmp.size()==0) continue;
        V3DLONG root_id=tmp.toInt();
        qDebug()<<"root_id"<<root_id;
        map<int,bool> mark_used;

        vector<int> seeds,seeds_next;
        seeds.push_back(root_id);
        mark_used[root_id]=1;

        int sid=0;
        V3DLONG parent;
        while (sid<seeds.size())
        {
            //check how many children the seed has, if more than one, seg_id reset,level++
            //the forked node belongs to the previous seg (has the same property has parent)
            qDebug()<<"sid:"<<sid<<" seeds size:"<<seeds.size();
            seeds_next.clear();
            parent=seeds[sid];
            V3DLONG p_x=nt->listNeuron[parent].x;
            V3DLONG p_y=nt->listNeuron[parent].y;
            V3DLONG p_z=nt->listNeuron[parent].z;
            float p_dis=nt->listNeuron[parent].fea_val[1];

            for (int i=0;i<subtree[parent].size();i++)
            {
                if (mark_used[subtree[parent][i]]==0)
                    seeds_next.push_back(subtree[parent][i]);
            }

            if (seeds_next.size()==0)
            {
                sid++;
                continue;
            }

            else if (seeds_next.size()>=1)
            {
                for (int i=0;i<seeds_next.size();i++)
                {
                    V3DLONG child_node=seeds_next[i];
                    V3DLONG c_x=nt->listNeuron[child_node].x;
                    V3DLONG c_y=nt->listNeuron[child_node].y;
                    V3DLONG c_z=nt->listNeuron[child_node].z;
                    float distance=(c_x-p_x)*(c_x-p_x)+(c_y-p_y)*(c_y-p_y)+(c_z-p_z)*(c_z-p_z);
                    nt->listNeuron[child_node].fea_val[1]=(sqrt(distance)+p_dis);
                    mark_used[seeds_next[i]]=1;
                }
                seeds.insert(seeds.end(),seeds_next.begin(),seeds_next.end());
            }
            sid++;
        }
    }
    qDebug()<<"finish calc distance to node";
}

void subtree_dialog::connected_components()
{
    map<int,int> mask;;
    vector<int> seeds;

    for (int i=0;i<nt->listNeuron.size();i++)
    {
        if(mask[i]>0) continue;
        seeds.push_back(i);
        mask[i]=tree_comp.size()+1;
        int sid=0;
        while (sid<seeds.size())
        {
            vector<int> nb_seeds=subtree[seeds[sid]];
            for (int nid=0;nid<nb_seeds.size();nid++)
            {
                if (mask[nb_seeds[nid]]<=0)
                {
                    mask[nb_seeds[nid]]=mask[seeds[sid]];
                    seeds.push_back(nb_seeds[nid]);
                }
            }
            sid++;
        }
        tree_comp.push_back(seeds);
    }
}

void subtree_dialog::run()
{
    //check nearest node for each marker
    vector<int> close_nodes=calc_nearest_node_around_marker();

    //check each tree how many, which and type of the markers
    for (int i=0;i<tree_comp.size();i++)
    {
        int marker_num=0;
        for (int j=0;j<close_nodes.size();j++)
        {
            if (mask[close_nodes[j]]==i+1)
                marker_num++;
        }

    }
    //
}
