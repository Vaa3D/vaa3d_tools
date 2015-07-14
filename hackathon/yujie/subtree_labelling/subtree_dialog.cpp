#include "subtree_dialog.h"

#define swc_win_name "subtree_labelling_swc"

#ifndef VOID
#define VOID 1000000000
#endif

// 0-Undefined, 1-Soma, 2-Axon, 3-Dendrite(oblique), 4-Apical_dendrite, 5-apical tufts,
//6-End_point, 7-basal_dendrite 9-custom

//enum type { undefined=0, soma=1, axon=2, oblique_dendrite=3,apical_dendrite=4, basal_dendrite=7 };

subtree_dialog::subtree_dialog(V3DPluginCallback2 *cb)
{
    callback=cb;
}

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
                nt->listNeuron[ii].fea_val.clear();
                nt->listNeuron[ii].fea_val.append(0); //store dendrite id
                nt->listNeuron[ii].fea_val.append(0); //store distance to node
            }
         }

        //get markers and check markers
        //qDebug()<<"Only 1 window open";
//        LList_in.clear();
//        LList_in=*(callback->getHandleLandmarkList_Any3DViewer(swc_win));
//        if (LList_in.size()==0)
//        {
//            v3d_msg("Please load markers");
//            return false;
//        }

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
                nt->listNeuron[ii].fea_val.clear();
                nt->listNeuron[ii].fea_val.append(0); //store dendrite id
                nt->listNeuron[ii].fea_val.append(0); //store distance to node
            }
         }

        //get markers and check markers
//        LList_in.clear();
//        LList_in=*(callback->getHandleLandmarkList_Any3DViewer(swc_win));
//        if (LList_in.size()==0)
//        {
//            v3d_msg("Please load markers");
//            return false;
//        }
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
    markers->setFixedWidth(215);
    markers->setCurrentIndex(0);
    layout2->addWidget(markers,2,0,1,4);
    QPushButton *refresh=new QPushButton(tr("Refresh markers"));
    layout2->addWidget(refresh,2,4,1,2);

    QPushButton *soma=new QPushButton(tr("soma"));
    QPushButton *axon=new QPushButton(tr("axon"));
    QPushButton *apical_dendrite = new QPushButton(tr("apical_dendrite"));
    QPushButton *basal_dendrite =new QPushButton(tr("basal dendrite"));
    QPushButton *oblique = new QPushButton(tr("oblique dendrite"));
    QPushButton *apical_tufts = new QPushButton(tr("apical tufts"));
    QPushButton *custom = new QPushButton(tr("custom"));
    // 0-Undefined, 1-Soma, 2-Axon, 3-Dendrite(oblique), 4-Apical_dendrite, 5-apical tufts ,
    //6-End_point, 7-basal_dendrite 9-custom

    layout2->addWidget(soma,3,0,1,2);
    layout2->addWidget(axon,3,2,1,2);
    layout2->addWidget(apical_dendrite,3,4,1,2);
    layout2->addWidget(apical_tufts,4,0,1,2);
    layout2->addWidget(basal_dendrite,4,2,1,2);
    layout2->addWidget(oblique,4,4,1,2);
    layout2->addWidget(custom,5,0,1,2);

    QFrame *line_3 = new QFrame();
    line_3->setFrameShape(QFrame::HLine);
    line_3->setFrameShadow(QFrame::Sunken);
    layout2->addWidget(line_3,6,0,1,6);

    QPushButton *btn_label     = new QPushButton("Run labelling");
    QPushButton *btn_save = new QPushButton("Save");
    QPushButton *btn_quit= new QPushButton("Quit");
    layout2->addWidget(btn_label,10,0,1,2);
    layout2->addWidget(btn_save,10,2,1,2);
    layout2->addWidget(btn_quit,10,4,1,2);

    connect(soma,SIGNAL(clicked()),this,SLOT(soma_clicked()));
    connect(axon,SIGNAL(clicked()),this,SLOT(axon_clicked()));
    connect(apical_dendrite,SIGNAL(clicked()),this,SLOT(apical_dendrite_clicked()));
    connect(basal_dendrite,SIGNAL(clicked()),this,SLOT(basal_clicked()));
    connect(apical_tufts,SIGNAL(clicked()),this,SLOT(apical_tuft_clicked()));
    connect(oblique,SIGNAL(clicked()),this,SLOT(oblique_clicked()));
    connect(custom,SIGNAL(clicked()),this,SLOT(custom_clicked()));
    connect(btn_quit,SIGNAL(clicked()),this,SLOT(maybe_save()));
    connect(refresh,SIGNAL(clicked()),this,SLOT(refresh_marker()));
    connect(btn_save,SIGNAL(clicked()),this,SLOT(save()));
    connect(btn_label,SIGNAL(clicked()),this,SLOT(run()));

    mydialog2->setWindowTitle("Subtree labelling plug-in");
    mydialog2->setLayout(layout2);
    mydialog2->show();
}


void subtree_dialog::build_new_parent_LUT_after_sort()
{
    int size=nt->listNeuron.size();
    parent_LUT.clear();
    parent_LUT.resize(size);
    for (int i=0;i<size;i++)
    {
        int parent_id=nt->listNeuron.at(i).parent;
        if (parent_id==-1) continue;
        parent_LUT[nt->hashNeuron.value(parent_id)].push_back(i);
    }
//    for (int i=0;i<subtree.size();i++)
//    {
//        vector<int> tmp=subtree[i];
//        for (int j=0;j<tmp.size();j++)
//        {
//            qDebug()<<"i:"<<i+1<< " j:"<<j+1<<" members:"<<tmp[j]+1;
//        }
//    }
    qDebug()<<"building new parnet LUT after sort";
}

void subtree_dialog::build_connt_LUT()
{
    int size=nt->listNeuron.size();
    subtree.clear();
    subtree.resize(size);
    for (int i=0;i<size;i++)
    {
        int parent_id=nt->listNeuron.at(i).parent;
        if (parent_id==-1) continue;
        subtree[nt->hashNeuron.value(parent_id)].push_back(i);
        subtree[i].push_back(nt->hashNeuron.value(parent_id));
    }
    qDebug()<<"build_connt_LUT done";
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
    int marker_id=calc_nearest_node_around_marker();
//    nt->listNeuron[marker_id].type=1;
    markers->setItemText(mid, "marker "+QString::number(mid+1)+ " soma");
//    LList_in[mid].color.r=LList_in[mid].color.g=LList_in[mid].color.b=0;
    LList_in[mid].comments="1";
    LList_in[mid].name=QString::number(marker_id).toStdString();
}

void subtree_dialog::axon_clicked()
{
    int mid=markers->currentIndex();
    int marker_id=calc_nearest_node_around_marker();
    markers->setItemText(mid, "marker "+QString::number(mid+1)+ " axon");
    LList_in[mid].comments="2";
    LList_in[mid].name=QString::number(marker_id).toStdString();
}

void subtree_dialog::apical_dendrite_clicked()
{
    int mid=markers->currentIndex();
    int marker_id=calc_nearest_node_around_marker();
//    sort_type_def(4,(float)(mid+1),marker_id);
    markers->setItemText(mid, "marker "+QString::number(mid+1)+ " apical dendrite");
//    LList_in[mid].color.r=LList_in[mid].color.g=LList_in[mid].color.b=0;
    LList_in[mid].comments="4";
    LList_in[mid].name=QString::number(marker_id).toStdString();
}

void subtree_dialog::basal_clicked()
{
    int mid=markers->currentIndex();
    int marker_id=calc_nearest_node_around_marker();
//    sort_type_def(7,(float)(mid+1),marker_id);
    markers->setItemText(mid, "marker "+QString::number(mid+1)+ " basal dendrite");
//    LList_in[mid].color.g=255;
//    LList_in[mid].color.r=LList_in[mid].color.b=0;
    LList_in[mid].comments="7";
    LList_in[mid].name=QString::number(marker_id).toStdString();
}

void subtree_dialog::oblique_clicked()
{
    int mid=markers->currentIndex();
    int marker_id=calc_nearest_node_around_marker();
//    sort_type_def(3,(float)(mid+1),marker_id);
    markers->setItemText(mid, "marker "+QString::number(mid+1)+ " oblique dendrite");
//    LList_in[mid].color.b=255;
//    LList_in[mid].color.r=LList_in[mid].color.g=0;
    LList_in[mid].comments="3";
    LList_in[mid].name=QString::number(marker_id).toStdString();
}

void subtree_dialog::apical_tuft_clicked()
{
    int mid=markers->currentIndex();
    int marker_id=calc_nearest_node_around_marker();
    markers->setItemText(mid, "marker "+QString::number(mid+1)+ " apical tuft");
    LList_in[mid].comments="5";
    LList_in[mid].name=QString::number(marker_id).toStdString();
}

void subtree_dialog::custom_clicked()
{
    int mid=markers->currentIndex();
    int marker_id=calc_nearest_node_around_marker();
    markers->setItemText(mid, "marker "+QString::number(mid+1)+ " custom");
    LList_in[mid].comments="9";
    LList_in[mid].name=QString::number(marker_id).toStdString();
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
        //qDebug()<<"seeds number:"<<seeds[sid]<<"seeds.size:"<<seeds.size();
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
//        qDebug()<<"parent:"<<parent<< "fea value:"<< nt->listNeuron.at(parent).fea_val.at(1)<<"seg_id:"<< nt->listNeuron.at(parent).seg_id
//                   <<"level:"<< nt->listNeuron.at(parent).level<<"children:"<<seeds_next.size();
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
                nt->listNeuron[seeds_next[i]].seg_id=i+1;
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
//            qDebug()<<"one child__parent:"<<"child:"<<nt->listNeuron[child_node].type<<" seg id:"
//                       <<nt->listNeuron[child_node].seg_id<<" level:"<<nt->listNeuron[child_node].level;
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


bool subtree_dialog::save()
{
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
        v3d_msg("swc window is closed.Please relaunch the plug-in");
        return;
    }

    for (int i=0;i<LList_new.size();i++)
    {
        LocationSimple tmp=LList_new[i];
        for (int j=0;j<LList_in.size();j++)
        {
            if (tmp==LList_in[j])
            {
                //qDebug()<<"found the same marker";
                //found_flag=true;
                LList_new[i].comments=LList_in[j].comments;
                LList_new[i].name=LList_in[j].name;
                break;
            }
        }
    }
    markers->clear();
    LList_in.clear();
    //do a deep copy
    for (int i=0;i<LList_new.size();i++)
    {
        LocationSimple new_tmp;
        new_tmp.x=LList_new[i].x;
        new_tmp.y=LList_new[i].y;
        new_tmp.z=LList_new[i].z;
        new_tmp.comments=LList_new[i].comments;
        new_tmp.name=LList_new[i].name;
        LList_in.append(new_tmp);
    }

    for (int i=0;i<LList_in.size();i++)
    {
        QString tmp_str;
        tmp_str=QString::fromStdString((LList_in[i].comments));
        if (tmp_str.contains("1"))
            markers->addItem(QString("marker ")+QString::number(i+1)+" soma");
        else if (tmp_str.contains("2"))
            markers->addItem(QString("marker ")+QString::number(i+1)+" axon");
        else if (tmp_str.contains("4"))
            markers->addItem(QString("marker ")+QString::number(i+1)+" apical dendrite");
        else if (tmp_str.contains("7"))
            markers->addItem(QString("marker ")+QString::number(i+1)+" basal dendrite");
        else if (tmp_str.contains("3"))
            markers->addItem(QString("marker ")+QString::number(i+1)+" oblique dendrite");
        else if (tmp_str.contains("5"))
            markers->addItem(QString("marker ")+QString::number(i+1)+" apical tufts");
        else if (tmp_str.contains("9"))
            markers->addItem(QString("marker ")+QString::number(i+1)+" custom");
        else
            markers->addItem(QString("marker ")+QString::number(i+1));
    }
    qDebug()<<"refresh markers finished";
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



void subtree_dialog::connected_components()
{
    vector<int> seeds;
    for (int i=0;i<nt->listNeuron.size();i++)
    {
        if(mask[i]>0) continue;
        seeds.clear();
        seeds.push_back(i);
        mask[i]=all_trees.size()+1;
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
        tree new_tree;
        new_tree.tree_id=all_trees.size()+1;
        all_trees.push_back(new_tree);
    }
    qDebug()<<"In connected components. We have "<<all_trees.size()<<" trees!";
}

void subtree_dialog::define_sort_id_for_trees()
{

    qDebug()<<"define sort id for trees";
    //check each tree how many, which type of the markers
    for (int i=0;i<all_trees.size();i++)
    {
        all_trees[i].belong_markers.clear();
        for (int j=0;j<LList_in.size();j++)
        {
            QString tmp;
            tmp=QString::fromStdString(LList_in[j].name);
            int close_node=tmp.toInt();
            if (mask[close_node]==all_trees[i].tree_id)
                all_trees[i].belong_markers.push_back(j);
        }
        if (all_trees[i].belong_markers.size()==0) //no need to sort
        {
            all_trees[i].sort_node=-1;
            all_trees[i].soma_marker=-1;
            continue;
        }

        else
        {
            bool soma_defined=false;
            for (int k=0;k<all_trees[i].belong_markers.size();k++)
            {
                QString tmp;
                tmp=QString::fromStdString(LList_in[all_trees[i].belong_markers[k]].comments);
                int type_id=tmp.toInt();
                if (type_id==1)
                {
                    //sort from this node
                    soma_defined=true;
                    tmp=QString::fromStdString(LList_in[all_trees[i].belong_markers[k]].name);
                    all_trees[i].sort_node=tmp.toInt();
                    all_trees[i].soma_marker=all_trees[i].belong_markers[k];
                    break;
                }
            }
            if (!soma_defined)
            {
                all_trees[i].soma_marker=-1;
                QString tmp;
                tmp=QString::fromStdString(LList_in[all_trees[i].belong_markers[0]].name);
                all_trees[i].sort_node= tmp.toInt();
            }

        }
        //qDebug()<<"tree:"<<i+1<<"# of markers:"<<all_trees[i].belong_markers.size()<<"sort_node:"<<all_trees[i].sort_node;
    }
}

void subtree_dialog::sort_all_trees()
{

    qDebug()<<"sorting all trees...."<<":"<<all_trees.size();
    for (int i=0;i<all_trees.size();i++)
    {
        tree this_tree=all_trees[i];
        //qDebug()<<"sort node:"<<this_tree.sort_node;
        if (this_tree.sort_node==-1) //no marker on this tree, no need to sort
            continue;
        vector<int> seeds,seeds_next;
        map<int,bool> mark_used;
        seeds.push_back(this_tree.sort_node);
        mark_used[this_tree.sort_node]=1;
        nt->listNeuron[this_tree.sort_node].parent=-1;

        int sid=0;
        V3DLONG parent;
        while (sid<seeds.size())
        {
            //qDebug()<<"seeds number:"<<seeds[sid]<<"seeds.size:"<<seeds.size()<<"seeds_next size:"<<seeds_next.size();

            seeds_next.clear();
            parent=seeds[sid];

            for (int j=0;j<subtree[parent].size();j++)
            {
                if (mark_used[subtree[parent][j]]==0)
                {
                    //qDebug()<<"parent:"<<parent<<"child:"<< subtree[parent][i];
                    seeds_next.push_back(subtree[parent][j]);
                }
            }

            if (seeds_next.size()==0)
            {
                sid++;
                continue;
            }
            else
            {
                for (int j=0;j<seeds_next.size();j++)
                {
                    nt->listNeuron[seeds_next[j]].parent=nt->listNeuron[parent].n;
                    mark_used[seeds_next[j]]=1;
                }
                seeds.insert(seeds.end(),seeds_next.begin(),seeds_next.end());
            }

            sid++;
        }
        //qDebug()<<"tree:"<<i+1<<" sorted"<<" size:"<<seeds.size()<<"map size:"<<mark_used.size();
    }

}

void subtree_dialog::run()
{
//    build_connt_LUT(); //get ready subtree
//    connected_components(); //build all_trees nodes/tree_id
    //reset the neurontree
    for (V3DLONG ii=0; ii<nt->listNeuron.size(); ii++)
    {
        nt->listNeuron[ii].type=nt->listNeuron[ii].seg_id=nt->listNeuron[ii].level=0;
        nt->listNeuron[ii].fea_val.clear();
        nt->listNeuron[ii].fea_val.append(0); //store dendrite id
        nt->listNeuron[ii].fea_val.append(0); //store distance to node
    }
    refresh_marker();
    define_sort_id_for_trees(); //build sort node and soma marker for all_trees
    sort_all_trees();
    build_new_parent_LUT_after_sort();
    subtree_define();
}


void subtree_dialog::subtree_define()
{
    qDebug()<<"in subtree defined";
    for (int i=0;i<all_trees.size();i++)
    {
        tree this_tree=all_trees[i];
        if (this_tree.sort_node==-1) //no marker on this tree, no need to sort
            continue;

        for (int j=0;j<this_tree.belong_markers.size();j++)
        {
            //qDebug()<<"number of markers on this tree:"<<this_tree.belong_markers.size()<<":"<<j;
            QString type=QString::fromStdString(LList_in[this_tree.belong_markers[j]].comments);
            int seg_type=type.toInt();
            QString node=QString::fromStdString(LList_in[this_tree.belong_markers[j]].name);
            int node_id=node.toInt();

            if (seg_type==1)
            {
                nt->listNeuron[node_id].type=1;
                continue;
            }
            else
            {
                label_single_tree(seg_type,this_tree.tree_id,node_id);
            }
        }
    }
    qDebug()<<"all subtree defined";
}


void subtree_dialog::label_single_tree(int type, int tree_id, int marker_id)
{
    qDebug()<<"label single tree";
    vector<int> seeds;
    V3DLONG seg_id=1;
    V3DLONG level=1;
    nt->listNeuron[marker_id].fea_val[0]=(float)tree_id;
    nt->listNeuron[marker_id].type=type;
    nt->listNeuron[marker_id].seg_id=seg_id;
    nt->listNeuron[marker_id].level=level;
    seeds.push_back(marker_id);
    //qDebug()<<"first seeds set:"<<seeds.size();
    int sid=0;
    V3DLONG parent;
    while (sid<seeds.size())
    {
        //check how many children the seed has, if more than one, seg_id reset,level++
        //the forked node belongs to the previous seg (has the same property has parent)
        //qDebug()<<"sid:"<<sid;
        parent=seeds[sid];
        V3DLONG p_x=nt->listNeuron[parent].x;
        V3DLONG p_y=nt->listNeuron[parent].y;
        V3DLONG p_z=nt->listNeuron[parent].z;
        float p_dis=nt->listNeuron[parent].fea_val[1];

        if (parent_LUT[parent].size()==0)
        {
            sid++;
            continue;
        }
        else if(parent_LUT[parent].size()==1)
        {
            int child_node=parent_LUT[seeds[sid]][0];
            nt->listNeuron[child_node].type=nt->listNeuron.at(parent).type;
            nt->listNeuron[child_node].fea_val[0]=nt->listNeuron.at(parent).fea_val[0];
            nt->listNeuron[child_node].seg_id=nt->listNeuron.at(parent).seg_id;
            nt->listNeuron[child_node].level=nt->listNeuron.at(parent).level;
            V3DLONG c_x=nt->listNeuron[child_node].x;
            V3DLONG c_y=nt->listNeuron[child_node].y;
            V3DLONG c_z=nt->listNeuron[child_node].z;
            float distance=(c_x-p_x)*(c_x-p_x)+(c_y-p_y)*(c_y-p_y)+(c_z-p_z)*(c_z-p_z);
            nt->listNeuron[child_node].fea_val[1]=(sqrt(distance)+p_dis);
            seeds.push_back(child_node);

//            qDebug()<<"one child__parent:"<<parent<<"fea value:"<< nt->listNeuron.at(parent).fea_val.at(0)<<
//                       "child:"<<nt->listNeuron[child_node].type<<" seg id:"
//                       <<nt->listNeuron[child_node].seg_id<<" level:"<<nt->listNeuron[child_node].level;
        }
        else if (parent_LUT[parent].size()>1 && nt->listNeuron[parent].parent!=-1)
        {
            for (int i=0;i<parent_LUT[parent].size();i++)
            {
                int child_node=parent_LUT[parent][i];
                nt->listNeuron[child_node].type=nt->listNeuron.at(parent).type;
                nt->listNeuron[child_node].fea_val[0]=(nt->listNeuron.at(parent).fea_val.at(0));
                nt->listNeuron[child_node].level=nt->listNeuron.at(parent).level+1;
                nt->listNeuron[child_node].seg_id=i+1;
                V3DLONG c_x=nt->listNeuron[child_node].x;
                V3DLONG c_y=nt->listNeuron[child_node].y;
                V3DLONG c_z=nt->listNeuron[child_node].z;
                float distance=(c_x-p_x)*(c_x-p_x)+(c_y-p_y)*(c_y-p_y)+(c_z-p_z)*(c_z-p_z);
                nt->listNeuron[child_node].fea_val[1]=(sqrt(distance)+p_dis);

//                qDebug()<<"2+child__parent:"<<parent<<"fea value:"<< nt->listNeuron.at(parent).fea_val.at(0)
//                           << " child "<<i << ":"<<nt->listNeuron[parent_LUT[parent][i]].type<< " seg id:"
//                           <<nt->listNeuron[parent_LUT[parent][i]].seg_id<<" level:"<<nt->listNeuron[parent_LUT[parent][i]].level;
            }
            seeds.insert(seeds.end(),parent_LUT[parent].begin(),parent_LUT[parent].end());
        }

        else if (parent_LUT[parent].size()>1 && nt->listNeuron[parent].parent==-1)
        {
            for (int i=0;i<parent_LUT[parent].size();i++)
            {
                int child_node=parent_LUT[parent][i];
                nt->listNeuron[child_node].type=nt->listNeuron.at(parent).type;
                nt->listNeuron[child_node].fea_val[0]=(nt->listNeuron.at(parent).fea_val.at(0));
                nt->listNeuron[child_node].level=nt->listNeuron.at(parent).level;
                nt->listNeuron[child_node].seg_id=i+1;
                V3DLONG c_x=nt->listNeuron[child_node].x;
                V3DLONG c_y=nt->listNeuron[child_node].y;
                V3DLONG c_z=nt->listNeuron[child_node].z;
                float distance=(c_x-p_x)*(c_x-p_x)+(c_y-p_y)*(c_y-p_y)+(c_z-p_z)*(c_z-p_z);
                nt->listNeuron[child_node].fea_val[1]=(sqrt(distance)+p_dis);

//                qDebug()<<"2+child__parent:"<<parent<<"fea value:"<< nt->listNeuron.at(parent).fea_val.at(0)
//                           << " child "<<i << ":"<<nt->listNeuron[parent_LUT[parent][i]].type<< " seg id:"
//                           <<nt->listNeuron[parent_LUT[parent][i]].seg_id<<" level:"<<nt->listNeuron[parent_LUT[parent][i]].level;
            }
            seeds.insert(seeds.end(),parent_LUT[parent].begin(),parent_LUT[parent].end());

        }
        sid++;
    }
    //qDebug()<<"label single tree finished. seeds size:"<<seeds.size();
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
            //qDebug()<<"sid:"<<sid<<" seeds size:"<<seeds.size();
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

















