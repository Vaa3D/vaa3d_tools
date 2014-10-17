/****************************************************************************
**
** marker_match_dialog.cpp
** by Hanbo Chen 2014.10.16
**
****************************************************************************/

#include "marker_match_dialog.h"

//can only handle two neurons
marker_match_dialog::marker_match_dialog(V3DPluginCallback2 * cb, QList<ImageMarker> * mList_in)
{
    combobox_n0 = new QComboBox();
    combobox_n1 = new QComboBox();
    list_pairs = new QListWidget();

    //obtain markers
    mList=mList_in;
    int info[3];
    m0.clear();
    m1.clear();
    mm0.clear();
    mm1.clear();
    for(int i=0; i<mList->size(); i++){
        if(get_marker_info(mList->at(i),info)){
            if(info[2]<0){ //free marker
                if(info[0]==0){
                    m0.append(i);
                    combobox_n0->addItem(QString::number(i+1));
                }else if(info[0]==1){
                    m1.append(i);
                    combobox_n1->addItem(QString::number(i+1));
                }
            }else{ //matched marker
                if(info[2]>i && info[2]<mList->size()){
                    if(info[0]==0){
                        mm0.append(i);
                        mm1.append(info[2]);
                    }else{
                        mm0.append(info[2]);
                        mm1.append(i);
                    }
                    list_pairs->addItem("Marker: " + QString::number(i+1) + " x Marker: " + QString::number(info[2]+1) );
                }
            }
        }
    }

    //initalize dialog
    btn_add = new QPushButton("<= Match this pair of neurons");
    btn_add->setAutoDefault(false);
    btn_done = new QPushButton("Done");
    btn_done->setAutoDefault(false);
    btn_free = new QPushButton("=> Free selected matching pair");
    btn_free->setAutoDefault(false);

    connect(btn_add,SIGNAL(clicked()), this, SLOT(add()));
    connect(btn_free,SIGNAL(clicked()), this, SLOT(free()));
    connect(btn_done,SIGNAL(clicked()), this, SLOT(accept()));

    gridLayout = new QGridLayout();
    gridLayout->addWidget(list_pairs, 0,0,7,5);
    QLabel * label_1 = new QLabel(QObject::tr("Available Markers"));
    gridLayout->addWidget(label_1, 1,6,1,2);
    QLabel * label_2 = new QLabel(QObject::tr("Neuron Stack 1:"));
    gridLayout->addWidget(label_2, 2,6,1,1);
    gridLayout->addWidget(combobox_n0, 2,7,1,1);
    QLabel * label_3 = new QLabel(QObject::tr("Neuron Stack 2:"));
    gridLayout->addWidget(label_3, 3,6,1,1);
    gridLayout->addWidget(combobox_n1, 3,7,1,1);
    gridLayout->addWidget(btn_add, 4,6,1,2);
    gridLayout->addWidget(btn_free, 5,6,1,2);
    gridLayout->addWidget(btn_done, 6,7,1,1);

    setLayout(gridLayout);
    setWindowTitle(QString("Match Markers"));

    if( (m0.size()==0 || m1.size()==0) )
        if( mm0.size() == 0){
            v3d_msg("There is no marker to match. Please define marker and link them to neuron first.");
            this->reject();
        }
}

void marker_match_dialog::add()
{
    if(m0.size() == 0 || m1.size()==0)
        return;

    int i0 = combobox_n0->currentIndex();
    int i1 = combobox_n1->currentIndex();
    int idx0 = m0[i0];
    int idx1 = m1[i1];

    //update neuron info
    ImageMarker * p;
    int info[3];
    get_marker_info(mList->at(idx0),info);
    info[2] = idx1;
    update_marker_info(mList->at(idx0),info);
    get_marker_info(mList->at(idx1),info);
    info[2] = idx0;
    update_marker_info(mList->at(idx1),info);

    //update dialog
    m0.removeAt(i0);
    m1.removeAt(i1);
    combobox_n0->removeItem(i0);
    combobox_n1->removeItem(i1);
    mm0.append(idx0);
    mm1.append(idx1);
    list_pairs->addItem("Marker: " + QString::number(idx0+1) + " x Marker: " + QString::number(idx1+1) );
}

void marker_match_dialog::free()
{
    int id = list_pairs->currentRow();
    if(id==-1)
    {
        v3d_msg("Please select a matched pair on left.");
        return;
    }

    int idx0 = mm0[id];
    int idx1 = mm1[id];

    //update neuron info
    ImageMarker * p;
    int info[3];
    get_marker_info(mList->at(idx0),info);
    info[2] = -1;
    update_marker_info(mList->at(idx0),info);
    get_marker_info(mList->at(idx1),info);
    info[2] = -1;
    update_marker_info(mList->at(idx1),info);

    //update dialog
    m0.append(idx0);
    m1.append(idx1);
    combobox_n0->addItem(QString::number(idx0+1));
    combobox_n1->addItem(QString::number(idx1+1));
    mm0.removeAt(id);
    mm1.removeAt(id);
    list_pairs->takeItem(id);
}
