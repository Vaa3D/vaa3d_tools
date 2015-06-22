#include "subtree_dialog.h"

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
        nt=(*tmp_nt).at(0);
        //check swc file
        if (nt.listNeuron.size()<=0)
        {
            v3d_msg("Error loading swc file, please check swc file");
            return false;
        }
        for (V3DLONG ii=0; ii<nt.listNeuron.size(); ii++)
        {
            if(nt.listNeuron.at(ii).r<0)
            {
                v3d_msg("You have illeagal radius values. Check your data.");
                return false;
            }
         }

        //get markers and check markers
        qDebug()<<"Only 1 window open";
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
        nt=(*tmp_nt).at(0);

        if (nt.listNeuron.size()<=0)
        {
            v3d_msg("Error loading swc file, please check swc file");
            return false;
        }
        for (V3DLONG ii=0; ii<nt.listNeuron.size(); ii++)
        {
            if(nt.listNeuron.at(ii).r<0)
            {
                v3d_msg("You have illeagal radius values. Check your data.");
                return false;
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
    return true;
}

void subtree_dialog::assign_marker_type()
{
    QDialog *mydialog2=new QDialog;
    QGridLayout *layout2=new QGridLayout;
    QLabel *info=new QLabel(tr("Please assign type for each segment denoted by markers"));
    layout2->addWidget(info,0,0,2,6);

    markers=new QComboBox();
    for (int i=0;i<LList_in.size();i++)
        markers->addItem(QString("marker ")+QString::number(i+1));
    markers->setFixedWidth(250);
    markers->setCurrentIndex(0);
    layout2->addWidget(markers,2,0,1,4);

    QPushButton *soma=new QPushButton(tr("soma"));
    QPushButton *axon=new QPushButton(tr("axon"));
    QPushButton *apical_dendrite = new QPushButton(tr("apical_dendrite"));
    QPushButton *basal_dendrite =new QPushButton(tr("basal dendrite"));
    QPushButton *oblique = new QPushButton(tr("oblique dendrite"));
    // 0-Undefined, 1-Soma, 2-Axon, 3-Dendrite, 4-Apical_dendrite, 5-Fork_point,
    //6-End_point, 7-basal_dendrite 8-oblique dendrite

    layout2->addWidget(soma,2,4,1,2);
    layout2->addWidget(axon,5,4,1,2);
    layout2->addWidget(apical_dendrite,6,4,1,2);
    layout2->addWidget(basal_dendrite,7,4,1,2);
    layout2->addWidget(oblique,8,4,1,2);

    QPushButton *ok     = new QPushButton("OK");
    QPushButton *cancel = new QPushButton("Cancel");
    layout2->addWidget(ok,10,0,1,2);
    layout2->addWidget(cancel,10,4,1,2);

    mydialog2->setWindowTitle("Type assignment");
    mydialog2->setLayout(layout2);
    mydialog2->show();
}


void subtree_dialog::subtree_extract()
{
    int size=nt.listNeuron.size();
    subtree.resize(size);
    int first_node=0;
    for (int i=0;i<size;i++)
    {
        int parent_id=nt.listNeuron.at(i).parent;
        if (parent_id==-1)
        {
            first_node=i;
        }
        else
            subtree[nt.hashNeuron.value(parent_id)].push_back(i);
    }
//    for (int i=0;i<subtree.size();i++)
//    {
//        vector<int> tmp=subtree[i];
//        for (int j=0;j<tmp.size();j++)
//        {
//            qDebug()<<"i:"<<i<< " j:"<<j<<" members:"<<tmp[j];
//        }
//    }

}

void subtree_dialog::marker_change()
{
    //
}

void subtree_dialog::soma_clicked()
{
    int mid=markers->currentIndex();
    int marker_id=calc_nearest_node_around_marker();
    qDebug()<<"marker id:"<<marker_id;
    type_def(1,mid+1,marker_id);
}


int subtree_dialog::calc_nearest_node_around_marker()
{
    int mid=markers->currentIndex();
    V3DLONG x,y,z;
    x=LList_in.at(mid).x;
    y=LList_in.at(mid).y;
    z=LList_in.at(mid).z;
    float min_dis=1e16;
    float dis;
    int close_node;
    for (int i=0;i<nt.listNeuron.size();i++)
    {
        dis=(nt.listNeuron.at(i).x-x)*(nt.listNeuron.at(i).x-x)+
                (nt.listNeuron.at(i).y-y)*(nt.listNeuron.at(i).y-y)+
                (nt.listNeuron.at(i).z-z)*(nt.listNeuron.at(i).z-z);
        if (dis<min_dis)
        {
            min_dis=dis;
            close_node=i;
        }
    }
    return close_node;
}

void subtree_dialog::type_def(int type, int dendrite_id, int marker_id)
{
    vector<int> seeds;
    int seg_id=1;
    int level=1;
    nt.listNeuron[marker_id].fea_val=dendrite_id;
    nt.listNeuron[marker_id].type=type;
    nt.listNeuron[marker_id].seg_id=seg_id;
    nt.listNeuron[marker_id].level=level;
    seeds.push_back(marker_id);

    int sid=0;
    int parent;
    while (sid<seeds.size())
    {
        //check how many children the seed has, if more than one, seg_id reset,level++
        //the forked node belongs to the previous seg (has the same property has parent)
        parent=seeds[sid];
        if (subtree[parent].size()==0)
        {
            sid++;
            continue;
        }
        else if(subtree[parent].size()==1)
        {
            int child_node=subtree[seeds[sid]][0];
            nt.listNeuron[child_node].type=nt.listNeuron.at(parent).type;
            nt.listNeuron[child_node].fea_val=nt.listNeuron.at(parent).fea_val;
            nt.listNeuron[child_node].seg_id=nt.listNeuron.at(parent).seg_id;
            nt.listNeuron[child_node].level=nt.listNeuron.at(parent).level;
            seeds.push_back(child_node);
        }
        else if (subtree[parent].size()>1)
        {
            for (int i=0;i<subtree[parent].size();i++)
            {
                nt.listNeuron[subtree[parent][i]].type=nt.listNeuron.at(parent).type;
                nt.listNeuron[subtree[parent][i]].fea_val=nt.listNeuron.at(parent).fea_val;
                nt.listNeuron[subtree[parent][i]].level=nt.listNeuron.at(parent).level+1;
                nt.listNeuron[subtree[parent][i]].seg_id=i+1;
            }
            seeds.insert(seeds.end(),subtree[parent].begin(),subtree[parent].end());
        }
        sid++;
    }

}













