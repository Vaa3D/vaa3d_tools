/****************************************************************************
**
** neuron_tipspicker_dialog.cpp
** by Hanbo Chen 2014.12.14
**
****************************************************************************/


#include "neuron_tipspicker_dialog.h"


#define checkwindow() \
    if (!callback){ \
        this->hide(); \
        return; \
    } \
    bool isclosed = true; \
    QList <V3dR_MainWindow *> allWindowList = callback->getListAll3DViewers(); \
    for (V3DLONG i=0;i<allWindowList.size();i++) \
    { \
        if(allWindowList.at(i)==v3dwin){ \
            isclosed = false; \
            break; \
        } \
    } \
    if(isclosed){ \
        this->hide(); \
        return; \
    }

void neuron_tipspicker_dialog::checkStatus()
{
    if(cb_tips->count()>mList->size()){
        v3d_msg("The number of markers does not match, please click Update or Reload button.");
        cb_tips->setEnabled(false);
        btn_accept->setEnabled(false);
        btn_skip->setEnabled(false);
        btn_reject->setEnabled(false);
    }else if(cb_tips->count()>0){
        cb_tips->setEnabled(true);
        btn_accept->setEnabled(true);
        btn_skip->setEnabled(true);
        btn_reject->setEnabled(true);
    }else{
        btn_accept->setEnabled(false);
        btn_skip->setEnabled(false);
        btn_reject->setEnabled(false);
    }
}

neuron_tipspicker_dialog::neuron_tipspicker_dialog(V3DPluginCallback2 * cb, V3dR_MainWindow* inwin)
{
    if(!inwin)
        return;
    v3dwin=inwin;
    callback = cb;
    v3dcontrol = callback->getView3DControl_Any3DViewer(v3dwin);

    ntList=cb->getHandleNeuronTrees_Any3DViewer(v3dwin);
    if(ntList->size()<=0){ //this should not happen
        v3d_msg("No neuron in the window!");
        return;
    }
    //reset the color of neuron list
    mList = cb->getHandleLandmarkList_Any3DViewer(v3dwin);

    creat();

    //update markers
    link_new_marker_neuron();
    update_tip();

    curIdx=0;
    checkStatus();
}

void neuron_tipspicker_dialog::creat()
{
    gridLayout = new QGridLayout();

    //border tips zone
    group_marker = new QGroupBox("search for border tips");
    group_marker->setCheckable(false);
    cb_dir = new QComboBox(); cb_dir->addItem("x"); cb_dir->addItem("y"); cb_dir->addItem("z");
    cb_dir->setCurrentIndex(2);
    spin_zscale = new QDoubleSpinBox();
    spin_zscale->setRange(0,100000); spin_zscale->setValue(1);
    spin_searchspan = new QDoubleSpinBox();
    spin_searchspan->setRange(0,100000); spin_searchspan->setValue(20);
    spin_segthr = new QDoubleSpinBox();
    spin_segthr->setRange(0,100000); spin_segthr->setValue(0);
    spin_gapthr = new QDoubleSpinBox();
    spin_gapthr->setRange(0,100000); spin_gapthr->setValue(0);
    spin_angthr = new QDoubleSpinBox();
    spin_angthr->setRange(0,180); spin_angthr->setValue(100);
    spin_spineLen = new QSpinBox();
    spin_spineLen->setRange(0,100000); spin_spineLen->setValue(5); spin_spineLen->setEnabled(false);
    spin_spineAng = new QDoubleSpinBox();
    spin_spineAng->setRange(0,180); spin_spineAng->setValue(30); spin_spineAng->setEnabled(false);
    spin_spineRadius = new QDoubleSpinBox();
    spin_spineRadius->setRange(0,100000); spin_spineRadius->setValue(3); spin_spineRadius->setEnabled(false);
    check_spine = new QCheckBox("filter spines when searching:");
    check_spine -> setChecked(false);
    cb_side = new QComboBox(); cb_side->addItem("search side A"); cb_side->addItem("search side B");
    cb_side->setCurrentIndex(0);
    btn_search = new QPushButton("Search"); btn_search->setAutoDefault(false);

    connect(btn_search, SIGNAL(clicked()), this, SLOT(search()));
    connect(check_spine, SIGNAL(stateChanged(int)), this, SLOT(spineCheck(int)));

    QGridLayout* groupLayout = new QGridLayout();
    QLabel* label_0 = new QLabel("stacking direction: ");
    groupLayout->addWidget(label_0,9,0,1,2,Qt::AlignRight);
    groupLayout->addWidget(cb_dir,9,2,1,1);
    QLabel* label_1 = new QLabel("resacle stacking direction: ");
    groupLayout->addWidget(label_1,9,3,1,2,Qt::AlignRight);
    groupLayout->addWidget(spin_zscale,9,5,1,1);
    QLabel* label_4 = new QLabel("border tips searching span: ");
    groupLayout->addWidget(label_4,13,0,1,2,Qt::AlignRight);
    groupLayout->addWidget(spin_searchspan,13,2,1,1);
    QLabel* label_62 = new QLabel("small gap filter (gap size): ");
    groupLayout->addWidget(label_62,13,3,1,2,Qt::AlignRight);
    groupLayout->addWidget(spin_gapthr,13,5,1,1);
    QLabel* label_6 = new QLabel("small segment filter (0=keep all): ");
    groupLayout->addWidget(label_6,14,0,1,2,Qt::AlignRight);
    groupLayout->addWidget(spin_segthr,14,2,1,1);
    QLabel* label_3 = new QLabel("max angle with section plan (0-180): ");
    groupLayout->addWidget(label_3,14,3,1,2,Qt::AlignRight);
    groupLayout->addWidget(spin_angthr,14,5,1,1);
    groupLayout->addWidget(check_spine,15,0,1,2);
    QLabel* label_7 = new QLabel("point #:");
    groupLayout->addWidget(label_7,16,0,1,1,Qt::AlignRight);
    groupLayout->addWidget(spin_spineLen,16,1,1,1);
    QLabel* label_8 = new QLabel("turning angle:");
    groupLayout->addWidget(label_8,16,2,1,1,Qt::AlignRight);
    groupLayout->addWidget(spin_spineAng,16,3,1,1);
    QLabel* label_9 = new QLabel("radius:");
    groupLayout->addWidget(label_9,16,4,1,1,Qt::AlignRight);
    groupLayout->addWidget(spin_spineRadius,16,5,1,1);
    groupLayout->addWidget(cb_side,17,3,1,2);
    groupLayout->addWidget(btn_search,17,5,1,1);
    group_marker->setLayout(groupLayout);
    gridLayout->addWidget(group_marker, 1,0,1,6);

    //selection zone
    group_check = new QGroupBox("check border tips");
    group_check->setCheckable(false);
    cb_tips = new QComboBox();
    btn_reset = new QPushButton("Reset"); btn_reset->setAutoDefault(false);
    btn_update = new QPushButton("Update"); btn_update->setAutoDefault(false);
    btn_accept = new QPushButton("Accept"); btn_accept->setAutoDefault(false);
    btn_skip = new QPushButton("Skip"); btn_skip->setAutoDefault(false);
    btn_reject = new QPushButton("Reject"); btn_reject->setAutoDefault(false);
    check_syncTriView = new QCheckBox("Sync View"); check_syncTriView->setChecked(true);
    spin_roisize = new QSpinBox();
    spin_roisize->setRange(0,100000); spin_roisize->setValue(50);

    connect(btn_reset, SIGNAL(clicked()), this, SLOT(reset_tip()));
    connect(btn_update, SIGNAL(clicked()), this, SLOT(update_tip()));
    connect(btn_accept, SIGNAL(clicked()), this, SLOT(accept_tip()));
    connect(btn_skip, SIGNAL(clicked()), this, SLOT(skip_tip()));
    connect(btn_reject, SIGNAL(clicked()), this, SLOT(reject_tip()));
    connect(cb_tips, SIGNAL(currentIndexChanged(int)), this, SLOT(change_tip(int)));

    QGridLayout* checkLayout = new QGridLayout();
    checkLayout->addWidget(btn_reset,1,0,1,1);
    checkLayout->addWidget(btn_update,1,1,1,1);
    checkLayout->addWidget(check_syncTriView,1,2,1,1);
    QLabel* label_10 = new QLabel("View Size");
    checkLayout->addWidget(label_10,1,3,1,1,Qt::AlignRight);
    checkLayout->addWidget(spin_roisize,1,4,1,1);
    checkLayout->addWidget(cb_tips,2,0,1,1);
    checkLayout->addWidget(btn_accept,2,1,1,1);
    checkLayout->addWidget(btn_skip,2,2,1,1);
    checkLayout->addWidget(btn_reject,2,3,1,1);
    group_check->setLayout(checkLayout);
    gridLayout->addWidget(group_check, 2,0,1,6);


    //output zone
    btn_save = new QPushButton("Save"); btn_save->setAutoDefault(false);
    btn_quit = new QPushButton("Quit"); btn_quit->setAutoDefault(false);

    connect(btn_save, SIGNAL(clicked()), this, SLOT(save()));
    connect(btn_quit, SIGNAL(clicked()), this, SLOT(reject()));

    gridLayout->addWidget(btn_save,3,4,1,1);
    gridLayout->addWidget(btn_quit,3,5,1,1);

    setLayout(gridLayout);
//    qDebug()<<"dialog created";
}

void neuron_tipspicker_dialog::search()
{
    checkwindow();
    mList->clear();

    int dir=cb_dir->currentIndex();
    int side=cb_side->currentIndex();
    double zscale=spin_zscale->value();
    double span=spin_searchspan->value();
    double gapThr=spin_gapthr->value();
    double angThr=cos(spin_angthr->value()/180*M_PI);
    double segmentThr=spin_segthr->value();
    double spineLengthThr = 0;
    double spineRadiusThr = 0;
    double spineAngThr = 1;
    if(check_spine->isChecked()){
        spineLengthThr = spin_spineLen->value();
        spineAngThr = cos(spin_spineAng->value()/180*M_PI);
        spineRadiusThr = spin_spineRadius->value();
    }

    QList<Candidate> cands;
    searchBorderTips(ntList, cands, side, dir, zscale, span, gapThr, angThr, segmentThr,
                     spineLengthThr, spineAngThr, spineRadiusThr);

    qDebug()<<"found "<<cands.size()<<" border tips";
    for(int i=0; i<cands.size(); i++){
        V3DLONG nid=cands.at(i).nid;
        V3DLONG pid=cands.at(i).pid;
        LocationSimple S0 = LocationSimple(ntList->at(nid).listNeuron[pid].x,
                                          ntList->at(nid).listNeuron[pid].y,
                                          ntList->at(nid).listNeuron[pid].z);
        S0.color.r = 0; S0.color.g = 0; S0.color.b = 255;
        S0.name = QString::number(i+1).toStdString();
        QString c0 = QString::number(nid)+" "+QString::number(pid)+" 0";
        S0.comments = c0.toStdString();
        mList->append(S0);
    }

    update_tip();
}

void neuron_tipspicker_dialog::spineCheck(int c)
{
    checkwindow();

    if(check_spine->isChecked()){
        spin_spineLen->setEnabled(true);
        spin_spineAng->setEnabled(true);
        spin_spineRadius->setEnabled(true);
    }else{
        spin_spineLen->setEnabled(false);
        spin_spineAng->setEnabled(false);
        spin_spineRadius->setEnabled(false);
    }
}

void neuron_tipspicker_dialog::reset_tip()
{
    checkwindow();

    //link marker neuron
    relink_marker_neuron();

    //populate tips
    cb_tips->clear();
    int info[4];

    for(int i=0; i<mList->size(); i++){
        if(get_marker_info(mList->at(i),info)){
            if(info[2]==1)
                cb_tips->addItem("Marker: " + QString::number(i+1) + " accepted");
            else if(info[2]==2)
                cb_tips->addItem("Marker: " + QString::number(i+1) + " rejected" );
            else
                cb_tips->addItem("Marker: " + QString::number(i+1));

        }
    }

    curIdx=0;
    checkStatus();

    //for test
    qDebug()<<"===========UPDATE MARKER============ reset tip";

    updatemarker();
}

void neuron_tipspicker_dialog::update_tip()
{
    checkwindow();

    link_new_marker_neuron();

    //populate tips
    cb_tips->clear();
    int info[4];

    for(int i=0; i<mList->size(); i++){
        if(get_marker_info(mList->at(i),info)){
            if(info[2]==1)
                cb_tips->addItem("Marker: " + QString::number(i+1) + " accepted");
            else if(info[2]==2)
                cb_tips->addItem("Marker: " + QString::number(i+1) + " rejected");
            else
                cb_tips->addItem("Marker: " + QString::number(i+1));

        }
    }

    if(curIdx>=cb_tips->count())
        curIdx=0;
    checkStatus();
}

void neuron_tipspicker_dialog::locate_tip()
{
    checkwindow();
    checkStatus();

    if(!check_syncTriView->isChecked())
        return;
    if(cb_tips->count()==0)
        return;
    if(!v3dwin)
        return;

    int mid=cb_tips->currentIndex();

    v3dhandleList list_triwin = callback->getImageWindowList();
    for(V3DLONG i=0; i<list_triwin.size(); i++){
//        //for test
//        qDebug()<<"cojoc1:"<<callback->getImageName(list_triwin.at(i));
//        qDebug()<<"cojoc2:"<<callback->getImageName(v3dwin);
        if(callback->getImageName(v3dwin).contains(callback->getImageName(list_triwin.at(i)))){
            TriviewControl * p_control = callback->getTriviewControl(list_triwin.at(i));
            p_control->setFocusLocation((long)mList->at(mid).x,(long)mList->at(mid).y,(long)mList->at(mid).z);
        }
    }
}

void neuron_tipspicker_dialog::roi_tip()
{
    checkwindow();
    checkStatus();

    int roiWinSize=spin_roisize->value();

    if(!check_syncTriView->isChecked())
        return;
    if(cb_tips->count()==0)
        return;
    if(!v3dwin)
        return;

    int mid=cb_tips->currentIndex();

    v3dhandleList list_triwin = callback->getImageWindowList();
    for(V3DLONG i=0; i<list_triwin.size(); i++){
        if(callback->getImageName(v3dwin).contains(callback->getImageName(list_triwin.at(i)))){
            //reset ROI
            ROIList pRoiList=callback->getROI(list_triwin.at(i));
            for(int j=0;j<3;j++){
                pRoiList[j].clear();
            }
            pRoiList[0] << QPoint((long)mList->at(mid).x-roiWinSize,(long)mList->at(mid).y-roiWinSize);
            pRoiList[0] << QPoint((long)mList->at(mid).x+roiWinSize,(long)mList->at(mid).y-roiWinSize);
            pRoiList[0] << QPoint((long)mList->at(mid).x+roiWinSize,(long)mList->at(mid).y+roiWinSize);
            pRoiList[0] << QPoint((long)mList->at(mid).x-roiWinSize,(long)mList->at(mid).y+roiWinSize);
            pRoiList[1] << QPoint((long)mList->at(mid).z-roiWinSize,(long)mList->at(mid).y-roiWinSize);
            pRoiList[1] << QPoint((long)mList->at(mid).z+roiWinSize,(long)mList->at(mid).y-roiWinSize);
            pRoiList[1] << QPoint((long)mList->at(mid).z+roiWinSize,(long)mList->at(mid).y+roiWinSize);
            pRoiList[1] << QPoint((long)mList->at(mid).z-roiWinSize,(long)mList->at(mid).y+roiWinSize);
            pRoiList[2] << QPoint((long)mList->at(mid).x-roiWinSize,(long)mList->at(mid).z-roiWinSize);
            pRoiList[2] << QPoint((long)mList->at(mid).x+roiWinSize,(long)mList->at(mid).z-roiWinSize);
            pRoiList[2] << QPoint((long)mList->at(mid).x+roiWinSize,(long)mList->at(mid).z+roiWinSize);
            pRoiList[2] << QPoint((long)mList->at(mid).x-roiWinSize,(long)mList->at(mid).z+roiWinSize);



            if(callback->setROI(list_triwin.at(i),pRoiList)){
                callback->updateImageWindow(list_triwin.at(i));
            }else{
                qDebug()<<"error: failed to set ROI";
                return;
            }
            callback->closeROI3DWindow(list_triwin.at(i));
            callback->openROI3DWindow(list_triwin.at(i));

            QString fname_roiwin="Local 3D View [" + callback->getImageName(list_triwin.at(i)) +"]";
            V3dR_MainWindow * mainwin_roi = 0;
            qDebug()<<"===========ROI============= searching: "<<fname_roiwin;
            QList <V3dR_MainWindow *> tmpwinlist = callback->getListAll3DViewers();
            for(int j=0; j<tmpwinlist.size(); j++){
                qDebug()<<"===========ROI============= "<<callback->getImageName(tmpwinlist[j]);
                if(callback->getImageName(tmpwinlist[j]).contains(fname_roiwin)){
                    mainwin_roi = tmpwinlist[j];
                    break;
                }
            }
            if(!mainwin_roi){
                qDebug()<<"===========ROI============= failed open ROI window";
                return;
            }

            qDebug()<<"===========ROI============= update ROI window with triview";
            View3DControl * v3dlocalcontrol = callback->getLocalView3DControl(list_triwin.at(i));
            v3dlocalcontrol->updateLandmark();

            QList <NeuronTree> * p_nttmp = callback->getHandleNeuronTrees_Any3DViewer(mainwin_roi);
            if(p_nttmp->size()!=ntList->size()){
                qDebug()<<"===========ROI============= copy neuron tree to 3Dview";
                p_nttmp->clear();
                for(int j=0; j<ntList->size(); j++){
                    p_nttmp->push_back((*ntList)[j]);
                }
                qDebug()<<"===========ROI============= copied neuron tree: "<<p_nttmp->size();

                callback->update_NeuronBoundingBox(mainwin_roi);
            }
            callback->update_3DViewer(mainwin_roi);

            break;
        }
    }

    qDebug()<<"===========ROI============= done";
}

void neuron_tipspicker_dialog::accept_tip()
{
    checkwindow();
    checkStatus();

    int info[4];
    int color[3]={0,255,0};
    int i=cb_tips->currentIndex();
    get_marker_info(mList->at(i),info);
    info[2]=1;
    update_marker_info(mList->at(i),info,color);
    cb_tips->setItemText(i, "Marker: " + QString::number(i+1) + " accepted");

    if(i+1<cb_tips->count()){
        cb_tips->setCurrentIndex(i+1);
    }else{
        //for test
        qDebug()<<"===========UPDATE MARKER============ accept tip";

        updatemarker();
    }
}

void neuron_tipspicker_dialog::reject_tip()
{
    checkwindow();
    checkStatus();

    int info[4];
    int color[3]={255,0,0};
    int i=cb_tips->currentIndex();
    get_marker_info(mList->at(i),info);
    info[2]=2;
    update_marker_info(mList->at(i),info,color);
    cb_tips->setItemText(i, "Marker: " + QString::number(i+1) + " rejected");

    if(i+1<cb_tips->count()){
        cb_tips->setCurrentIndex(i+1);
    }else{
        //for test
        qDebug()<<"===========UPDATE MARKER============ reject tip";

        updatemarker();
    }
}

void neuron_tipspicker_dialog::skip_tip()
{
    checkwindow();
    checkStatus();

    int i=cb_tips->currentIndex();
    if(i+1<cb_tips->count()){
        cb_tips->setCurrentIndex(i+1);
    }
}

void neuron_tipspicker_dialog::change_tip(int c)
{
    checkwindow();
    checkStatus();

    int info[4];
    if(curIdx<mList->size())
    {
        get_marker_info(mList->at(curIdx),info);
        int color[3]={0};
        if(info[2]==1)
            color[1]=255;
        else if(info[2]==2)
            color[0]=255;
        else
            color[2]=255;
        update_marker_info(mList->at(curIdx),info,color);
    }

    if(cb_tips->count()>0)
    {
        int color[3]={255,0,255};
        int i=cb_tips->currentIndex();
        get_marker_info(mList->at(i),info);
        update_marker_info(mList->at(i),info,color);
        curIdx=i;
    }

    //for test
    qDebug()<<"===========UPDATE MARKER============ change tip";

    updatemarker();
}

void neuron_tipspicker_dialog::save()
{
    checkwindow();

    QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
                                                        QObject::tr(""),
            QObject::tr("Supported file (*.marker)"
                ";;Marker    (*.marker)"
                ));
    if(!fileSaveName.isEmpty()){
        output_markers(fileSaveName);
    }
}

void neuron_tipspicker_dialog::output_markers(QString fname)
{
    QFile file(fname);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text)){
        v3d_msg("cannot open "+ fname +" for write");
        return;
    }
    QTextStream myfile(&file);
    myfile<<"##x,y,z,radius,shape,name,comment, color_r,color_g,color_b"<<endl;
    //matched candidates first
    int info[4];
    for(int i=0; i<mList->size(); i++){
        get_marker_info(mList->at(i), info);
        if(info[2]==1)
            myfile<<mList->at(i).x<<","<<mList->at(i).y<<","<<mList->at(i).z<<",0,1, , ,0,255,0"<<endl;
    }
    file.close();
}

void neuron_tipspicker_dialog::link_new_marker_neuron()
{
    if(mList->size()<=0){
        return;
    }

    int color[3]={0,0,255};

    double dis;
    for(int i=0; i<mList->size(); i++){
        int info[4];
        if(get_marker_info(mList->at(i),info))
            continue;
        double mdis=1e10;
        int nid=-1;
        int pid=0;

        for(int j=0; j<ntList->size(); j++){
            for(int k=0; k<ntList->at(j).listNeuron.size(); k++){
                dis=NTDIS(ntList->at(j).listNeuron[k],mList->at(i));
                if(dis<mdis){
                    mdis=dis;
                    nid=j;
                    pid=k;
                }
            }
        }
        info[0]=nid;
        info[1]=pid;
        info[2]=info[3]=0;
        update_marker_info((*mList)[i],info,color);
    }
}

void neuron_tipspicker_dialog::relink_marker_neuron()
{
    if(mList->size()<=0){
        return;
    }

    int color[3]={0,0,255};

    double dis;
    for(int i=0; i<mList->size(); i++){
        int info[4];
        double mdis=1e10;
        int nid=-1;
        int pid=0;

        for(int j=0; j<ntList->size(); j++){
            for(int k=0; k<ntList->at(j).listNeuron.size(); k++){
                dis=NTDIS(ntList->at(j).listNeuron[k],mList->at(i));
                if(dis<mdis){
                    mdis=dis;
                    nid=j;
                    pid=k;
                }
            }
        }
        info[0]=nid;
        info[1]=pid;
        info[2]=info[3]=0;
        update_marker_info((*mList)[i],info,color);
    }
}

void neuron_tipspicker_dialog::updatemarker()
{
    checkwindow();

    if(v3dcontrol){
        v3dcontrol->updateLandmark();
    }

    locate_tip();
    roi_tip();
}
