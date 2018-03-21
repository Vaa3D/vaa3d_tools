#include "manual_proof_is.h"
#include "common.h"
#define MAINWINNAME "IS Quantifier_Proofread"
#define SEGMENT_VIEW "proofread by segment"
#define SEGMENT_VIEW_PROCESS "proofread by segment_processed"

//LList comments status:
//1 not reviewed_dendrite
//2 not reviewed_spine
//3 accept_dendrite
//4 accept_spine
//5 reject_dendrite
//6 reject_spine

QDataStream &operator<<(QDataStream &out, const init_para_is &tmp)
{
    out << quint8(tmp.spine_bgthr)<<quint8(tmp.is_bgthr)<<quint8(tmp.spine_channel)<<
           quint8(tmp.is_channel)<<quint8(tmp.max_dis)<<quint8(tmp.min_voxel)<<quint8(tmp.seg_id)
        << quint8(tmp.marker_id)<<quint8(tmp.erodeNB)<<quint8(tmp.dilateNB);
    return out;
}

QDataStream &operator>>(QDataStream &in, init_para_is &tmp)
{
    quint8 spine_bgthr,is_bgthr,spine_channel,is_channel,max_dis,min_voxel,seg_id,marker_id
            ,erodeNB,dilateNB;

    in >> spine_bgthr>>is_bgthr>>spine_channel>>is_channel>>max_dis>>min_voxel
            >> seg_id>>marker_id>>erodeNB>>dilateNB;
    tmp = init_para_is((int)spine_bgthr,(int)is_bgthr,(int)spine_channel,(int)is_channel,(int)max_dis,(int)min_voxel,(int)seg_id,(int)marker_id
                       ,(int)erodeNB,(int)dilateNB);
    return in;
}

QDataStream &operator<<(QDataStream &out, const GOI &tmp)
{
    QVector<quint32> newvoxels;
    for (int i=0;i<tmp.voxel.size();i++)
        newvoxels.push_back(quint32(tmp.voxel.at(i)));
    out<<newvoxels<<qint32(tmp.nearest_node)<<qint32(tmp.label_id)<<tmp.on_dendrite;

    return out;
}

QDataStream &operator>>(QDataStream &in, GOI &tmp)
{
    qint32 nearest_id,label_id;
    QVector<quint32> voxels;
    vector<V3DLONG> oldstylevoxels;
    bool on_dendrite;

    in >>voxels>> nearest_id >> label_id >>on_dendrite;

    for (int i=0;i<voxels.size();i++)
        oldstylevoxels.push_back((V3DLONG)voxels.at(i));
    tmp=GOI(oldstylevoxels,(int)nearest_id,(int)label_id,on_dendrite);
    return in;
}


QDataStream &operator<<(QDataStream &out, const LocationSimple &input)
{
    out << (qint16) input.x<<(qint16)input.y <<(qint16) input.z << QString::fromStdString(input.name)
        << QString::fromStdString(input.comments) << (quint8)input.color.r << (quint8)input.color.g <<(quint8)input.color.b
        << (quint16)input.category;
    return out;
}

QDataStream &operator>>(QDataStream &in, LocationSimple &output)
{
    qint16 x,y,z;
    quint8 r,g,b;
    quint16 category;
    QString comments, name;

    in >> x>>y>>z>>name>>comments>>r>>g>>b>>category;
    output = LocationSimple((int)x,(int)y,(int)z);
    output.color.r=r; output.color.g=g; output.color.b=b;
    output.category=(int)category;
    output.comments = comments.toStdString();
    output.name=name.toStdString();
    return in;
}

manual_proof_is::manual_proof_is(V3DPluginCallback2 *cb, bool menu_flag)
{
    callback=cb;
    seg_edit_flag=false;
    edit_flag=false;
    image_trun=0;
    image_is=0;
    dialog=0;
    debugF=true;
    override_img=false;
    override_swc=false;
    //auto para initailization are done in to auto-interface
    if(debugF)
        qDebug()<<"in constructor";

    green = QColor(170, 255,127);
    red = QColor(255,106,108);

    checked_neuron=0;

    if (menu_flag)
    {
        create();
        initDlg();
        check_btn();
    }
}

void manual_proof_is::create()
{
    if(debugF)
        qDebug()<<"in create";
    QGridLayout *mygridLayout = new QGridLayout();
    QLabel* label_load = new QLabel(QObject::tr("Load project:"));
    mygridLayout->addWidget(label_load,0,0,1,1);
    edit_load = new QLineEdit("");
    edit_load->setReadOnly(true);
    mygridLayout->addWidget(edit_load,0,1,1,6);
    QPushButton *btn_load = new QPushButton("...");
    mygridLayout->addWidget(btn_load,0,7,1,1);
    btn_run = new QPushButton("Run");

    QLabel* label_image = new QLabel(QObject::tr("Load Image:"));
    mygridLayout->addWidget(label_image,1,0,1,1);
    edit_load_img = new QLineEdit("");
    edit_load_img->setReadOnly(true);
    mygridLayout->addWidget(edit_load_img,1,1,1,6);
    QPushButton *btn_load_img = new QPushButton("...");
    mygridLayout->addWidget(btn_load_img,1,7,1,1);

    QLabel* label_swc = new QLabel(QObject::tr("Load swc:"));
    mygridLayout->addWidget(label_swc,2,0,1,1);
    edit_load_swc = new QLineEdit("");
    edit_load_swc->setReadOnly(true);
    mygridLayout->addWidget(edit_load_swc,2,1,1,6);
    QPushButton *btn_swc = new QPushButton("...");
    mygridLayout->addWidget(btn_swc,2,7,1,1);

    QPushButton *cancel = new QPushButton("Cancel");
    mygridLayout->addWidget(btn_run,16,2,1,2);
    mygridLayout->addWidget(cancel,16,5,1,2);
    this->setLayout(mygridLayout);
    this->setWindowTitle("IS_proofreading");
    this->show();
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(btn_load, SIGNAL(clicked()), this, SLOT(get_proj_name()));
    connect(btn_run,SIGNAL(clicked()),this,SLOT(run_saved_proj()));
    connect(btn_load_img, SIGNAL(clicked()), this, SLOT(get_image_name()));
    connect(btn_swc, SIGNAL(clicked()), this, SLOT(get_swc_name()));
    if(debugF)
        qDebug()<<"end of create";
}

bool manual_proof_is::get_image_name()
{
    QString fileOpenName=this->edit_load_img->text();
    input_image_name2 = QFileDialog::getOpenFileName(0, QObject::tr("Choose the input image "),
             fileOpenName,QObject::tr("Images (*.raw *.tif *.lsm *.v3dpbd *.v3draw);;All(*)"));
    if (input_image_name2.isEmpty())
    {
        override_img=false;
        qDebug()<<"fname not valid";
        return false;
    }
    else
    {
        override_img=true;
        edit_load_img->setText(input_image_name2);
        qDebug()<<"fname:"<<input_image_name2;
        return true;
    }
}

bool manual_proof_is::get_swc_name()
{
    QString fileOpenName=this->edit_load_swc->text();
    input_swc_name2 = QFileDialog::getOpenFileName(0, 0,fileOpenName,"Supported file (*.swc *.eswc)" ";;Neuron structure(*.swc *eswc)",0,0);

    if(input_swc_name2.isEmpty())
    {
        override_swc=false;
        v3d_msg("No swc file loaded.");
        return false;
    }
    else{
        override_swc=true;
        edit_load_swc->setText(input_swc_name2);
        qDebug()<<"input swc name:"<<input_swc_name2;
        return true;
    }
}

bool manual_proof_is::check_btn()
{
    if (this->edit_load->text().isEmpty())
    {
        btn_run->setEnabled(false);
        return false;
    }
    else
    {
        btn_run->setEnabled(true);
        return true;
    }
}

void manual_proof_is::initDlg()
{
    if(debugF)
        qDebug()<<"in initDlg";
    QSettings settings("V3D plugin","IS_quantifier");
    if (settings.contains("fname_proj"))
    {
        this->edit_load->setText(settings.value("fname_proj").toString());
        input_proj_name=edit_load->text();
    }
    if(debugF)
        qDebug()<<"end of initDlg";
}

bool manual_proof_is::get_proj_name()
{
    QString fileOpenName=this->edit_load->text();
    input_proj_name = QFileDialog::getOpenFileName(0, QObject::tr("Choose the project "),
             fileOpenName,QObject::tr("Supported file (*.txt)"));
    if (input_proj_name.isEmpty())
    {
        qDebug()<<"project name not valid";
        return false;
    }
    else
    {
        edit_load->setText(input_proj_name);
        check_btn();
        qDebug()<<"name:"<<input_proj_name;
        QSettings settings("V3D plugin","IS_quantifier");
        settings.setValue("fname_proj",edit_load->text());
        return true;
    }
}

void manual_proof_is::create_proofread_panel(bool exist_proj)
{
    if(debugF)
        qDebug()<<"IS manual proof!!!in create proofread_panel";
    mydialog=new QDialog();
    mydialog->setWindowTitle("IS_proofreading");
    mydialog->setFixedWidth(500);
    QGridLayout *layout2=new QGridLayout;

    segments=new QComboBox;
    if (!exist_proj) //interface with automatic detection
    {
        for (int i=0;i<segment_neuronswc.size();i++)
            segments->addItem(QString("Segment ")+QString::number(i+1));
    }

    else  //if this project exist
    {
        for (int i=0;i<segment_neuronswc.size();i++)
        {
            if (finish_flag[i])
                segments->addItem(QString("Segment ")+QString::number(i+1)+" finished");
            else
                segments->addItem(QString("Segment ")+QString::number(i+1));
        }

    }
    segments->setCurrentIndex(auto_para.seg_id);
    layout2->addWidget(segments,0,0,1,3);

    QPushButton *finish_seg=new QPushButton(tr("Finish segment"));
    QPushButton *skip_seg=new QPushButton(tr("Next segment"));
    layout2->addWidget(finish_seg,1,0,1,2);
    layout2->addWidget(skip_seg,1,2,1,2);

    QFrame *line_1 = new QFrame();
    line_1->setFrameShape(QFrame::HLine);
    line_1->setFrameShadow(QFrame::Sunken);
    layout2->addWidget(line_1,2,0,1,6);

    QLabel *is_groups=new QLabel(QApplication::tr("IS groups"));
    layout2->addWidget(is_groups,3,0,1,6);
    QLabel *multiple=new QLabel(QApplication::tr("Press ctrl/shift to select multiple markers"));
    layout2->addWidget(multiple,4,0,1,6);

    list_markers=new QListWidget();
    layout2->addWidget(list_markers,5,0,8,4);
    list_markers->setSelectionMode(QAbstractItemView::ExtendedSelection);
    list_markers->setFocusPolicy(Qt::NoFocus);

    QPushButton *btn_accept=new QPushButton(tr("Accept"));
    QPushButton *btn_reject=new QPushButton("Reject");
    layout2->addWidget(btn_accept,5,4,1,2);
    layout2->addWidget(btn_reject,6,4,1,2);

    QPushButton *dilate = new QPushButton(tr("Dilate"));
    QPushButton *erode =new QPushButton (tr("Erode"));
    QPushButton *reset = new QPushButton (tr("Reset"));
    layout2->addWidget(dilate,7,4,1,2);
    layout2->addWidget(erode,8,4,1,2);
    layout2->addWidget(reset,9,4,1,2);

    QPushButton *set_loc_spine=new QPushButton("On spine");
    layout2->addWidget(set_loc_spine,10,4,1,2);
    QPushButton *set_loc_dendrite=new QPushButton("On dendrite");
    layout2->addWidget(set_loc_dendrite,11,4,1,2);

    QLabel *infobox=new QLabel("Info:");
    layout2->addWidget(infobox,13,0,1,2);
    edit_seg = new QPlainTextEdit;
    edit_seg->setReadOnly(true);
    edit_seg->setFixedHeight(60);
    layout2->addWidget(edit_seg,14,0,1,6);

    QPushButton *button_save=new QPushButton(tr("Save current results"));
    layout2->addWidget(button_save,15,0,1,2);

    QPushButton *button_finish=new QPushButton(tr("Finish proofreading"));
    layout2->addWidget(button_finish,15,2,1,2);
    mydialog->setLayout(layout2);

    QPushButton *button_setting = new QPushButton(tr("Settings"));
    layout2->addWidget(button_setting,15,4,1,2);
    mydialog->setLayout(layout2);

    connect(button_save,SIGNAL(clicked()),this,SLOT(save()));
    connect(button_finish,SIGNAL(clicked()),this,SLOT(finish_dialog()));
    connect(segments,SIGNAL(currentIndexChanged(int)),this,SLOT(segment_change()));
    connect(btn_accept,SIGNAL(clicked()),this,SLOT(accept_marker()));
    connect(btn_reject,SIGNAL(clicked()),this,SLOT(reject_marker()));
    connect(set_loc_dendrite,SIGNAL(clicked()),this,SLOT(set_on_dendrite()));
    connect(set_loc_spine,SIGNAL(clicked()),this,SLOT(set_on_spine()));
    connect(dilate,SIGNAL(clicked()),this,SLOT(dilate()));
    connect(erode,SIGNAL(clicked()),this,SLOT(erode()));
    connect(reset,SIGNAL(clicked()),this,SLOT(reset_marker_clicked()));
    connect(skip_seg,SIGNAL(clicked()),this,SLOT(skip_segment_clicked()));
    connect(finish_seg,SIGNAL(clicked()),this,SLOT(finish_seg_clicked()));
    connect(button_setting,SIGNAL(clicked()),this,SLOT(setting_window()));
    segment_change();
    mydialog->show();

    qDebug()<<"mydialog finished building";
}

void manual_proof_is::setting_window()
{
    if (debugF)
        qDebug()<<"in setting window";
    dialog=new QDialog();
    dialog->setWindowTitle("IS_proofreading settings");
    dialog->setFixedWidth(500);
    QGridLayout *layout2=new QGridLayout;
    erode_nb = new QSpinBox();
    dilate_nb = new QSpinBox();
    QLabel * erode_label = new QLabel(tr("Neighbor threshold for erosion:"));
    QLabel * dilate_label = new QLabel(tr("Neighbor threshold for dilation:"));
    layout2->addWidget(erode_label,0,0,1,3);
    layout2->addWidget(dilate_label,1,0,1,3);
    layout2->addWidget(erode_nb,0,3,1,1);
    layout2->addWidget(dilate_nb,1,3,1,1);
    erode_nb->setMaximum(5);
    erode_nb->setMinimum(1);
    dilate_nb->setMaximum(5);
    dilate_nb->setMinimum(1);
    erode_nb->setValue(auto_para.erodeNB);
    dilate_nb->setValue(auto_para.dilateNB);
    QPushButton *btn_ok=new QPushButton(tr("Ok"));
    QPushButton *btn_cancel=new QPushButton("Cancel");
    layout2->addWidget(btn_ok,2,0,1,2);
    layout2->addWidget(btn_cancel,2,2,1,2);
    connect(btn_ok, SIGNAL(clicked()), this, SLOT(receive_new_para()));
    connect(btn_cancel, SIGNAL(clicked()), this, SLOT(cancel_new_para()));
    dialog->setLayout(layout2);
    dialog->show();
}

void manual_proof_is::receive_new_para()
{
    if(debugF)
        qDebug()<<"in receive new para, new erode_nb"<<erode_nb->value()<<" new dilate nb"<<dilate_nb->value();
    auto_para.erodeNB=erode_nb->value();
    auto_para.dilateNB=dilate_nb->value();
    dialog->close();
}

void manual_proof_is::cancel_new_para()
{
    erode_nb->setValue(auto_para.erodeNB);
    dilate_nb->setValue(auto_para.dilateNB);
    dialog->close();
    if(debugF)
        qDebug()<<"in cancel new para, new erode_nb"<<erode_nb->value()<<" new dilate nb"<<dilate_nb->value();
}

void manual_proof_is::check_each_seg()
{
    qDebug()<<"in check each seg"<<segment_neuronswc.size()<<" LList"<<LList_in.size();

    finish_flag.resize(segment_neuronswc.size(),0);

    QSet<int> seg_candid;
    for (int i=0;i<LList_in.size();i++)
    {
        //qDebug()<<"I:"<<i<<" size:"<<LList_in[i].category;
        if (LList_in[i].category!=0)
            seg_candid<<LList_in[i].category;
    }
    //qDebug()<<"seg_candid:"<<seg_candid.size();
    int count=0;
    QSet<int>::const_iterator i = seg_candid.constBegin();
    while (i != seg_candid.constEnd()) {
        //qDebug()<<*i;
        for (int w=0;w<LList_in.size();w++)
        {
            if (LList_in[w].category==*i && (QString::fromStdString(LList_in[w].comments).contains("1")
                    ||QString::fromStdString(LList_in[w].comments).contains("2")))
            {
                goto stop;
            }
        }
        finish_flag[(*i)-1]=true;
        count++;
        stop: {}
        ++i;
    }
    //qDebug()<<"count:"<<count;
}

void manual_proof_is::run_saved_proj()
{
    qDebug()<<"running saved proj 10-7";

    if (!check_btn())
    {
        v3d_msg("You have not provided valid input/output");
        return;
    }
    this->accept();;
    if (!loadProjectData()) //auto para, LList_in, voxel group set
    {
        v3d_msg("Loading project txt error");
        return;
    }
    if (!loadImage())
    {
        v3d_msg("Invalid image.Error",0);
        QMessageBox::information(0,"Error loading swc","<br>Please go back to load project window and specify path to image file");
        return;
    }
    if (!loadSWC())
    {
        v3d_msg("Invalid swc. Error",0);
        QMessageBox::information(0,"Error loading swc","<br>Please go back to load project window and specify path to swc file");
        return;
    }
    voxel_group_copy=voxel_group;
    V3DLONG size_page=sz_img[0]*sz_img[1]*sz_img[2];

    memset(image_is+2*size_page,0,size_page);
    for(int i=0; i<voxel_group.size(); i++)
    {
        GOI tmp = voxel_group[i];

        for (int j=0; j<tmp.voxel.size(); j++)
        {
            image_is[tmp.voxel.at(j)+2*size_page]=255;
        }
    }

    checked_neuron=check_neuron_tree(neuron,sz_img);
    segment_neuronswc=neurontree_divide_swc(checked_neuron,auto_para.max_dis*6);
    check_each_seg();
    create_proofread_panel(true);
}


void manual_proof_is::run_interface_with_auto(int ret,NeuronTree auto_neuron,bool eswc_tmp_flag,
    LandmarkList LList,vector<GOI> voxel_group_in,unsigned char *image1Dc,V3DLONG sz_img_in[4],
    int spine_bgthr,int is_bgthr, int max_dis,int min_voxel,int spine_channel,int is_channel,
    QString in_swc_name,QString in_image_name,QString folder_output)
{
    //get related name
    input_swc_name=in_swc_name;
    input_image_name=in_image_name;
    baseDir=folder_output;
    eswc_flag=eswc_tmp_flag;

    //get ready LList, neuron,image,para,label_group,label_group_copy
    LList_in=LList;
    for (int i=0;i<voxel_group_in.size();i++)
    {
        voxel_group.push_back(voxel_group_in.at(i));
        voxel_group_copy.push_back(voxel_group_in.at(i));
    }
//    voxel_group=voxel_group_in;
//    //make copy for label_group
//    voxel_group_copy=voxel_group;

    neuron.copy(auto_neuron);

    sz_img[0]=sz_img_in[0];
    sz_img[1]=sz_img_in[1];
    sz_img[2]=sz_img_in[2];
    sz_img[3]=sz_img_in[3];

    V3DLONG size_page=sz_img[0]*sz_img[1]*sz_img[2];
    image_is = new unsigned char [size_page*3];
    memset(image_is,0,size_page*3);
    memcpy(image_is,image1Dc,size_page*2);

    for(int i=0; i<voxel_group_in.size(); i++)
    {
        GOI tmp = voxel_group[i];

        for (int j=0; j<tmp.voxel.size(); j++)
        {
            image_is[tmp.voxel.at(j)+2*size_page]=255;
        }
    }

    //get ready para
    auto_para.spine_bgthr=spine_bgthr;
    auto_para.is_bgthr=is_bgthr;
    auto_para.max_dis=max_dis;
    auto_para.min_voxel=min_voxel;
    auto_para.marker_id=0;
    auto_para.spine_channel=spine_channel;
    auto_para.is_channel=is_channel;
    auto_para.erodeNB=2;
    auto_para.dilateNB=2; //both erodeNB and dilateNB defaults to 2

    if (ret==1)
    {
        auto_para.seg_id=0;
        checked_neuron=check_neuron_tree(neuron,sz_img);
        segment_neuronswc=neurontree_divide_swc(checked_neuron,auto_para.max_dis*6);
        create_proofread_panel(false);

    }
    else
        return;
}

void manual_proof_is::open_trunc_triview(QString trunc_win_name)
{
    //qDebug()<<"proofIS: in open trunc triview";
    bool window_open_flag=false;
    v3dhandleList list_triwin = callback->getImageWindowList();
    for(V3DLONG i=0; i<list_triwin.size(); i++){
        if(callback->getImageName(list_triwin.at(i)).contains(trunc_win_name))
        {
            window_open_flag=true;
            curwin=list_triwin[i];
            break;
        }
    }
    //qDebug()<<"check window: window_open_flag:"<<window_open_flag;
    if(!window_open_flag)
    {
        curwin=callback->newImageWindow(trunc_win_name);
        unsigned char *image_input=new unsigned char [sz[0]*sz[1]*sz[2]*sz[3]];
        memcpy(image_input,image_trun,sz[0]*sz[1]*sz[2]*sz[3]);
        Image4DSimple image_tmp;
        image_tmp.setData(image_input,sz[0],sz[1],sz[2],sz[3],V3D_UINT8);
        callback->setImage(curwin,&image_tmp);
        callback->updateImageWindow(curwin);
    }
}

void manual_proof_is::open_main_triview(QString main_win_name)  //image1dc_spine
{
    //qDebug()<<"open main_triview window in manual proof IS";
    //backup
    bool window_open_flag=false;
    v3dhandleList list_triwin = callback->getImageWindowList();
    for(V3DLONG i=0; i<list_triwin.size(); i++){
        if(callback->getImageName(list_triwin.at(i)).contains(main_win_name))
        {
            window_open_flag=true;
            main_win=list_triwin[i];
            return;
        }
    }
    if(!window_open_flag)
    {
        main_win = callback->newImageWindow(main_win_name);
        unsigned char *image_input=new unsigned char [sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3]];
        memcpy(image_input,image_is,sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3]);
        Image4DSimple image_main;
        image_main.setData(image_input,sz_img[0],sz_img[1],sz_img[2],sz_img[3],V3D_UINT8);
        callback->setImage(main_win,&image_main);
        callback->updateImageWindow(main_win);
    }
}

vector<V3DLONG> manual_proof_is::image_seg_plan(vector<int> seg, NeuronTree *neuron_tmp)
{
    float extra_length=(float)auto_para.max_dis;
    float start_x,start_y,start_z,end_x,end_y,end_z;
    start_x=start_y=start_z=1e8;
    end_x=end_y=end_z=0;

    for (int i=0;i<seg.size();i++)
    {
        NeuronSWC tmp=neuron_tmp->listNeuron.at(seg[i]);
        if (tmp.x-tmp.r<start_x)
            start_x=tmp.x-tmp.r;
        if (tmp.x+tmp.r>end_x)
            end_x=tmp.x+tmp.r;
        if (tmp.y-tmp.r<start_y)
            start_y=tmp.y-tmp.r;
        if (tmp.y+tmp.r>end_y)
            end_y=tmp.y+tmp.r;
        if (tmp.z-tmp.r<start_z)
            start_z=tmp.z-tmp.r;
        if (tmp.z+tmp.r>end_z)
            end_z=tmp.z+tmp.r;
    }
    start_x=MAX(start_x-extra_length,0);
    start_y=MAX(start_y-extra_length,0);
    start_z=MAX(start_z-extra_length,0);
    end_x=MIN(end_x+extra_length,sz_img[0]-1);
    end_y=MIN(end_y+extra_length,sz_img[1]-1);
    end_z=MIN(end_z+extra_length,sz_img[2]-1);

//    qDebug()<<"xyz_min:"<<start_x<<":"<<start_y<<":"<<start_z;
//    qDebug()<<"xyz max:"<<end_x<<":"<<end_y<<":"<<end_z;
//    qDebug()<<"size:"<<end_x-start_x+1<<":"<<end_y-start_y+1<<":"<<end_z-start_z+1;
    vector<V3DLONG> coord(6,0);
    coord[0]=(V3DLONG)start_x;
    coord[1]=(V3DLONG)start_y;
    coord[2]=(V3DLONG)start_z;
    coord[3]=(V3DLONG)end_x;
    coord[4]=(V3DLONG)end_y;
    coord[5]=(V3DLONG)end_z;
    return coord;
}

void manual_proof_is::set_trunc_image_marker(vector<int> one_seg,int seg_id)
{
    //qDebug()<<"IS proof: in set_trunc_image_marker";
    vector<V3DLONG> coord(6,0);
    coord=image_seg_plan(one_seg,checked_neuron);

    x_start=coord[0];
    y_start=coord[1];
    z_start=coord[2];
    x_end=coord[3];
    y_end=coord[4];
    z_end=coord[5];

//    qDebug()<<"xyz_min:"<<x_start<<":"<<y_start<<":"<<z_start;
//    qDebug()<<"xyz max:"<<x_start<<":"<<y_start<<":"<<z_start;

    sz[0]=x_end-x_start+1;
    sz[1]=y_end-y_start+1;
    sz[2]=z_end-z_start+1;
    sz[3]=3;
    //qDebug()<<"sz:"<<sz[0]<<":"<<sz[1]<<":"<<sz[2];
    if (image_trun!=0)
    {
        delete [] image_trun;
        image_trun=0;
    }
    image_trun=new unsigned char[sz[0]*sz[1]*sz[2]*sz[3]];
    memset(image_trun,0,sz[0]*sz[1]*sz[2]*sz[3]);

    for (V3DLONG dx=x_start;dx<x_end;dx++){
        for (V3DLONG dy=y_start;dy<y_end;dy++){
            for (V3DLONG dz=z_start;dz<z_end;dz++){
                V3DLONG pos=xyz2pos(dx,dy,dz,sz_img[0],sz_img[0]*sz_img[1]);
                V3DLONG pos1=xyz2pos(dx-x_start,dy-y_start,dz-z_start,sz[0],sz[0]*sz[1]);
                image_trun[pos1]=image_is[pos];
                image_trun[pos1+sz[0]*sz[1]*sz[2]]= image_is[pos+sz_img[0]*sz_img[1]*sz_img[2]];
            }
        }
    }

    LList_adj.clear();
    //find all the IS in this set.
    //also need to check if the whole spine is displayed
    for (int j=0;j<LList_in.size();j++)
    {

        if (LList_in[j].x-1<=x_start|| LList_in[j].x-1>=x_end || LList_in[j].y-1<=y_start
            || LList_in[j].y-1>=y_end || LList_in[j].z-1<=z_start ||LList_in[j].z-1>=z_end )
            continue;

        GOI tmp_is= voxel_group [j];
        if (LList_in.at(j).category!=seg_id)  //need to check if the IS is complete
        {
            bool incomplete_flag=false;
            for (int k=0;k<tmp_is.voxel.size();k++)
            {
                vector<V3DLONG> coord = pos2xyz(tmp_is.voxel[k],sz_img[0],sz_img[0]*sz_img[1]);
                if (coord[0]<x_start || coord[0] > x_end || coord[1] <y_start
                        || coord[1] >y_end || coord[2] <z_start || coord[2] >z_end)
                {
                    incomplete_flag=true;
                    break;
                }
            }
            if (incomplete_flag) //if incomplete, not to include the IS
                continue;
            if (LList_in.at(j).category==0)
                LList_in[j].category=seg_id;
        }


        for (int k=0;k<tmp_is.voxel.size();k++)
        {
            vector<V3DLONG> old_coord = pos2xyz(tmp_is.voxel[k],sz_img[0],sz_img[0]*sz_img[1]);
            V3DLONG x=old_coord[0]-x_start;
            V3DLONG y=old_coord[1]-y_start;
            V3DLONG z=old_coord[2]-z_start;
            V3DLONG pos=xyz2pos(x,y,z,sz[0],sz[0]*sz[1]);
            image_trun[pos+2*sz[0]*sz[1]*sz[2]]=255;
        }
        LocationSimple tmp;
        tmp.x=LList_in[j].x-x_start;
        tmp.y=LList_in[j].y-y_start;
        tmp.z=LList_in[j].z-z_start;
        tmp.color.r=LList_in[j].color.r;
        tmp.color.g=LList_in[j].color.g;
        tmp.color.b=LList_in[j].color.b;
        tmp.name=LList_in[j].name;
        tmp.comments=LList_in[j].comments;
        LList_adj.append(tmp);
    }

//    QString filename=QString::number(one_seg[0])+"_"+QString::number(one_seg[1])+".v3draw";
//    unsigned char *image_copy=new unsigned char[sz_seg[0]*sz_seg[1]*sz_seg[2]*sz_seg[3]];
//    memcpy(image_copy,image_seg,sz_seg[0]*sz_seg[1]*sz_seg[2]*sz_seg[3]);
//    simple_saveimage_wrapper(*callback,filename.toStdString().c_str(),image_copy,sz_seg,V3D_UINT8);

    //qDebug()<<"set visualize window."<<LList_adj.size()<<"markers have been found";
}

//LList comments status:
//1 not reviewed_dendrite
//2 not reviewed_spine
//3 accept_dendrite
//4 accept_spine
//5 reject_dendrite
//6 reject_spine
void manual_proof_is::segment_change()
{
    if (seg_edit_flag)
    {
        if (!save_seg_edit())
        {
            return;
        }
    }
    qDebug()<<"in segment change";
    //check whether previous editing needs saving
    edit_flag=false;
    if(segments->count()==0) return;
    edit_seg->clear();
    int seg_id=segments->currentIndex();

    //step 1: check whether main-triview is open
    open_main_triview(MAINWINNAME);

    //step 2:set data ready and open local tri-view/3d window
    set_trunc_image_marker(segment_neuronswc[seg_id],seg_id+1);

    //step 3: check whether local triview is open
    open_trunc_triview(SEGMENT_VIEW);

    //step 4: get the list of markers
    if(debugF)
        qDebug()<<"LList seg size;"<<LList_adj.size();
    list_markers->clear();
    disconnect(list_markers,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(marker_in_one_seg()));
    for (int i=0;i<LList_adj.size();i++)
    {
        //qDebug()<<"LList_adj: "<<i;
        QString tmp_comment=QString::fromStdString(LList_adj[i].comments);
        if (tmp_comment.contains("1"))
            list_markers->addItem("marker "+ QString::number(i+1)+ " on dendrite");
        else if (tmp_comment.contains("2"))
            list_markers->addItem("marker "+QString::number(i+1)+ " on spine");
        else if (tmp_comment.contains("3"))
        {
            QListWidgetItem * newItem = new QListWidgetItem;
            newItem->setText("marker "+ QString::number(i+1)+" on dendrite. Accepted");
            newItem->setBackgroundColor(green);
            list_markers->insertItem(i,newItem);
            //list_markers->addItem("marker "+ QString::number(i+1)+" on dendrite. Accepted");
        }
        else if (tmp_comment.contains("4"))
        {
            QListWidgetItem * newItem = new QListWidgetItem;
            newItem->setText("marker "+ QString::number(i+1)+" on spine. Accepted");
            newItem->setBackgroundColor(green);
            list_markers->insertItem(i,newItem);
        }
        else if (tmp_comment.contains("5"))
        {
            QListWidgetItem * newItem = new QListWidgetItem;
            newItem->setText("marker "+ QString::number(i+1)+" on dendrite. Rejected");
            newItem->setBackgroundColor(red);
            list_markers->insertItem(i,newItem);
        }
        else if (tmp_comment.contains("6"))
        {
            QListWidgetItem * newItem = new QListWidgetItem;
            newItem->setText("marker "+ QString::number(i+1)+" on spine. Rejected");
            newItem->setBackgroundColor(red);
            list_markers->insertItem(i,newItem);
        }
    }
    list_markers->setCurrentItem(list_markers->item(0));
    connect(list_markers,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(marker_in_one_seg()));
    //step 5: get the image ready
    Image4DSimple image4d;
    unsigned char * image_input=new unsigned char [sz[0]*sz[1]*sz[2]*sz[3]];
    memcpy(image_input,image_trun,sz[0]*sz[1]*sz[2]*sz[3]);
    //qDebug()<<"sz_seg size:"<<sz[0]<<":"<<sz[1]<<":"<<sz[2];

    image4d.setData(image_input,sz[0],sz[1],sz[2],sz[3],V3D_UINT8);
    //qDebug()<<"xyz_min:"<<x_start;
    callback->setImage(curwin,&image4d);
    callback->setLandmark(curwin,LList_adj);
    callback->updateImageWindow(curwin);
    callback->close3DWindow(curwin);
    callback->open3DWindow(curwin);
    callback->pushObjectIn3DWindow(curwin);
    prev_seg=seg_id;
}

void manual_proof_is::reject_marker()
{
    open_main_triview(MAINWINNAME);
    open_trunc_triview(SEGMENT_VIEW);
    qDebug()<<"reject marker";
    //update landmarks color and status
    QList<QListWidgetItem*> sel_list=list_markers->selectedItems();
    if (debugF)
        qDebug()<<"this many selected:"<<sel_list.size();

    edit_seg->clear();

    for (int i=0;i<sel_list.size();i++)
    {
        int mid= list_markers->row(sel_list[i]);

        LList_adj[mid].color.g=LList_adj[mid].color.b=0;
        LList_adj[mid].color.r=255;

        QString tmp_name=QString::fromStdString(LList_adj.at(mid).name);
        int idx=tmp_name.toInt()-1;//idx in LList_in starting from 1
        QString tmp_comment = QString::fromStdString((LList_adj.at(mid).comments));

        LList_in[idx].color.g=LList_in[idx].color.b=0;
        LList_in[idx].color.r=255;
        if (tmp_comment.contains("1") || tmp_comment.contains("3"))
        {
            LList_in[idx].comments=QString::number(5).toStdString(); //rejected marker
            LList_adj[mid].comments=QString::number(5).toStdString();
            sel_list[i]->setText("marker " + QString::number(mid+1) + " on dendrite. Rejected");
            sel_list[i]->setBackgroundColor(red);
        }
        else if (tmp_comment.contains("2") || tmp_comment.contains("4"))
        {
            LList_in[idx].comments=QString::number(6).toStdString(); //rejected marker
            LList_adj[mid].comments=QString::number(6).toStdString();
            sel_list[i]->setText("marker " + QString::number(mid+1) + " on spine. Rejected");
            sel_list[i]->setBackgroundColor(red);
        }

        edit_seg->appendPlainText("marker: " + QString::number(mid+1) + " rejected");
    }

    edit_flag=false;
    seg_edit_flag=true;
    callback->setLandmark(curwin,LList_adj);
    callback->pushObjectIn3DWindow(curwin);

}

void manual_proof_is::accept_marker()
{
    open_main_triview(MAINWINNAME);
    open_trunc_triview(SEGMENT_VIEW);
    //qDebug()<<"accept marker llist size:"<<LList_adj.size()<<":"<<LList_in.size();
    //update landmarks color and status

    QList<QListWidgetItem*> sel_list=list_markers->selectedItems();
    edit_seg->clear();

    for (int i=0;i<sel_list.size();i++)
    {
        int mid= list_markers->row(sel_list[i]);

        QString tmp_name=QString::fromStdString(LList_adj.at(mid).name);
        int idx=tmp_name.toInt()-1;
        QString tmp_comment = QString::fromStdString((LList_adj.at(mid).comments));
        LList_adj[mid].color.r =LList_in[idx].color.r =0;
        LList_adj[mid].color.g =LList_in[idx].color.g =255;
        LList_adj[mid].color.b =LList_in[idx].color.b =0;

        if (tmp_comment.contains("1") || tmp_comment.contains("5")) //on dendrite
        {

            LList_in[idx].comments=QString::number(3).toStdString(); //accept marker
            LList_adj[mid].comments=QString::number(3).toStdString();
            sel_list[i]->setText("marker " + QString::number(mid+1) + " on dendrite. Accepted");
            sel_list[i]->setBackgroundColor(green);
        }
        else if (tmp_comment.contains("2") || tmp_comment.contains("6")) //on spine
        {

            LList_in[idx].comments=QString::number(4).toStdString(); //accept marker
            LList_adj[mid].comments=QString::number(4).toStdString();
            sel_list[i]->setText("marker " + QString::number(mid+1) + " on spine. Accepted");
            sel_list[i]->setBackgroundColor(green);
        }

        edit_seg->appendPlainText("marker: " + QString::number(mid+1) + " accepted");
    }

    edit_flag=false;
    seg_edit_flag=true;
    callback->setLandmark(curwin,LList_adj);
    callback->pushObjectIn3DWindow(curwin);
}

void manual_proof_is::set_on_spine()
{
    //qDebug()<<"set on spine";
    open_main_triview(MAINWINNAME);
    open_trunc_triview(SEGMENT_VIEW);

    //update landmarks color and status
    //color for spine purple 255 0 255
    QList<QListWidgetItem*> sel_list=list_markers->selectedItems();
    //qDebug()<<"this many selected:"<<sel_list.size();
    edit_seg->clear();
    for (int i=0;i<sel_list.size();i++)
    {
        int mid= list_markers->row(sel_list[i]);
        QString tmp_name=QString::fromStdString(LList_adj.at(mid).name);
        int idx=tmp_name.toInt()-1;
        QString tmp_comment = QString::fromStdString((LList_adj.at(mid).comments));
        voxel_group[idx].on_dendrite=false;

        if (tmp_comment.contains("1")||tmp_comment.contains("3")||tmp_comment.contains("5"))
        {
            LList_adj[mid].color.r =LList_in[idx].color.r =255;
            LList_adj[mid].color.b =LList_in[idx].color.b =255;
            LList_adj[mid].color.g =LList_in[idx].color.g =0;
            LList_in[idx].comments=QString::number(2).toStdString(); //type set to on spine
            LList_adj[mid].comments=QString::number(2).toStdString();
            sel_list[i]->setText("marker " + QString::number(mid+1) + " on spine");
            sel_list[i]->setBackgroundColor(Qt::white);
            edit_seg->appendPlainText("marker: " + QString::number(mid+1) + " set on spine.");
        }
        else if (tmp_comment.contains("2") || tmp_comment.contains("4")|| tmp_comment.contains("6")) //on spine
        {
            edit_seg->appendPlainText("marker: " + QString::number(mid+1) + " already on spine. No changes made.");
        }

    }

    edit_flag=false;
    seg_edit_flag=true;
    callback->setLandmark(curwin,LList_adj);
    callback->pushObjectIn3DWindow(curwin);
}

void manual_proof_is::set_on_dendrite()
{
    //qDebug()<<"set on dendrite";
    open_main_triview(MAINWINNAME);
    open_trunc_triview(SEGMENT_VIEW);

    //update landmarks color and status
    //color for dendrite blue 85,170,255
    QList<QListWidgetItem*> sel_list=list_markers->selectedItems();
    //qDebug()<<"this many selected:"<<sel_list.size();
    edit_seg->clear();
    for (int i=0;i<sel_list.size();i++)
    {
        int mid= list_markers->row(sel_list[i]);
        QString tmp_name=QString::fromStdString(LList_adj.at(mid).name);
        int idx=tmp_name.toInt()-1;
        QString tmp_comment = QString::fromStdString((LList_adj.at(mid).comments));
        voxel_group[idx].on_dendrite=true;

        if (tmp_comment.contains("2")||tmp_comment.contains("4")||tmp_comment.contains("6"))
        {
            LList_adj[mid].color.r =LList_in[idx].color.r =85;
            LList_adj[mid].color.b =LList_in[idx].color.b =170;
            LList_adj[mid].color.g =LList_in[idx].color.g =255;
            LList_in[idx].comments=QString::number(1).toStdString(); //type set to on spine
            LList_adj[mid].comments=QString::number(1).toStdString();
            sel_list[i]->setText("marker " + QString::number(mid+1) + " on dendrite");
            sel_list[i]->setBackgroundColor(Qt::white);
            edit_seg->appendPlainText("marker: " + QString::number(mid+1) + " set on dendrite.");
        }
        else if (tmp_comment.contains("1") || tmp_comment.contains("3")|| tmp_comment.contains("5")) //on spine
        {
            edit_seg->appendPlainText("marker: " + QString::number(mid+1) + " already on dendrite. No changes made.");
        }

    }

    edit_flag=false;
    seg_edit_flag=true;
    callback->setLandmark(curwin,LList_adj);
    callback->pushObjectIn3DWindow(curwin);
}

bool manual_proof_is::save_seg_edit()
{
    QMessageBox mybox;
    mybox.setText("<b>Have you finished editing this segment? <\b>");
    QString info="-Yes: all IS not rejected will be accepted<br> -Discard: all changes to segment will be discarded<br>"
            "-Cancel: return to proofreading panel";
    mybox.setInformativeText(info);
    mybox.addButton(QMessageBox::Yes);
    QPushButton *skip_button=mybox.addButton(tr("Discard"),QMessageBox::ActionRole);
    mybox.addButton(QMessageBox::Cancel);

    mybox.setDefaultButton(QMessageBox::Yes);
    int ret=mybox.exec();
    if (ret==QMessageBox::Yes)
    {
        for (int i=0;i<LList_adj.size();i++)
        {
            QString tmp_comment= QString::fromStdString(LList_adj[i].comments);
            QString tmp_name=QString::fromStdString(LList_adj[i].name);
            int idx=tmp_name.toInt()-1;
            if(tmp_comment.contains("1"))
            {
              LList_in[idx].comments=QString::number(3).toStdString();
              LList_in[idx].color.r=LList_adj[i].color.r=255;
              LList_in[idx].color.g=LList_adj[i].color.g=0;
              LList_in[idx].color.b=LList_adj[i].color.b=255;;
             }
            else if (tmp_comment.contains("2"))
            {
              LList_in[idx].comments=QString::number(4).toStdString();
              LList_in[idx].color.r=85;LList_in[idx].color.g=255;
              LList_in[idx].color.b=175;
              LList_adj[i].color.r=85;LList_adj[i].color.g=255;
              LList_adj[i].color.b=175;
            }
        }
        segments->setItemText(prev_seg,"Segment "+ QString::number(prev_seg+1)+" finished");
        return true;
    }
    else if (ret==QMessageBox::Cancel)
    {
        disconnect(segments,SIGNAL(currentIndexChanged(int)),this,SLOT(segment_change()));
        segments->setCurrentIndex(prev_seg);
        connect(segments,SIGNAL(currentIndexChanged(int)),this,SLOT(segment_change()));
        return false;
    }

    else if (mybox.clickedButton()==skip_button)
    {
        reset_segment();
        return true;
    }
}

void manual_proof_is::erode()
{
    open_main_triview(MAINWINNAME);
    open_trunc_triview(SEGMENT_VIEW);
    edit_seg->clear();
    if(debugF)
        qDebug()<<"in erode now"<<sz_img[0]<<":"<<sz_img[1]<<":"<<sz_img[2];
    V3DLONG size_page=sz_img[0]*sz_img[1]*sz_img[2];
    V3DLONG z_offset=sz_img[0]*sz_img[1];

    int mid=list_markers->currentRow();
    QListWidgetItem* sel=list_markers->item(mid);

    LList_adj[mid].color.r=LList_adj[mid].color.b=255;
    LList_adj[mid].color.g=255;
    QString tmp_comment = QString::fromStdString((LList_adj.at(mid).comments));
    if(debugF)
        qDebug()<<"tmp_comment:"<<tmp_comment<<" mid:"<<mid;
    if (tmp_comment.contains("1")||tmp_comment.contains("3")||tmp_comment.contains("5"))
    {
        sel->setText("marker " + QString::number(mid+1) + " on dendrite");
        qDebug()<<"3";
    }
    else
    {
      sel->setText("marker " + QString::number(mid+1) + " on spine");
      qDebug()<<"4";
    }

    QString tmp_name=QString::fromStdString(LList_adj[mid].name);
    int idx=tmp_name.toInt()-1;//idx in LList_in starting from 1
    GOI tmp_group = voxel_group[idx];
    qDebug()<<"in erode 2 size: "<<tmp_group.voxel.size();
    if (tmp_group.voxel.size()==0)
    {
        edit_seg->setPlainText("No more voxels left. Cannot erode");
        return;
    }
    vector<V3DLONG> erodeList, remainList;
    QHash<V3DLONG, bool> hash;
    for (int i=0;i<tmp_group.voxel.size();i++)  //in the hash fg-true, bg-false
    {
        hash.insert(tmp_group.voxel.at(i),true);
    }
    int bg_count;
    int bg_thr=auto_para.erodeNB;
    for (int i=0;i<tmp_group.voxel.size();i++)
    {

        V3DLONG pos= tmp_group.voxel[i];
        bg_count=0;
        //look at 6 nbs and if one is < bgthr_is, it will be eroded this round
        if(pos-1>=0 && !hash.contains(pos-1))
        {            
            bg_count++;
        }
        if(pos+1<size_page && !hash.contains(pos+1))
        {
            bg_count++;
        }
        if(pos-sz_img[0]>=0 &&!hash.contains(pos-sz_img[0]))
        {
            bg_count++;
        }
        if(pos+sz_img[0]<size_page && !hash.contains(pos+sz_img[0]))
        {
            bg_count++;
        }
        if(pos-z_offset>=0 && !hash.contains(pos-z_offset))
        {
            bg_count++;
        }
        if(pos+z_offset<size_page && !hash.contains(pos+z_offset))
        {
            bg_count++;
        }
        if (bg_count<bg_thr)
            remainList.push_back(pos);
        else
            erodeList.push_back(pos);
    }

    if (erodeList.size()==0)
    {
       edit_seg->setPlainText("No more voxels left. Cannot erode");
       return;
    }

    edit_seg->setPlainText(QString::number(erodeList.size())+" voxels deleted in this round of erosion. "
                           +QString::number(remainList.size())+ " voxels left");
    qDebug()<<"~~~erode~~~~decrease:"<<erodeList.size();

    for (int i =0;i<erodeList.size();i++)
    {
        vector<V3DLONG> coord= pos2xyz(erodeList.at(i),sz_img[0],sz_img[0]*sz_img[1]);
        //image_is[erodeList.at(i)+2*size_page]=0; //do we need to maintain image_is?
        V3DLONG x= coord[0]-x_start;
        V3DLONG y= coord[1]-y_start;
        V3DLONG z= coord[2]-z_start;
        V3DLONG trun_pos = xyz2pos(x,y,z,sz[0],sz[0]*sz[1]);
        image_trun[trun_pos+2*sz[0]*sz[1]*sz[2]]=0;
    }

    voxel_group[idx].voxel.clear();
    voxel_group[idx].voxel=remainList;
    qDebug()<<"~~~erode~~~~remained:"<<remainList.size();

    unsigned char *erode_tmp =new unsigned char [sz[0]*sz[1]*sz[2]*sz[3]];
    memcpy(erode_tmp,image_trun,sz[0]*sz[1]*sz[2]*3);

    Image4DSimple image4d_tmp;
    image4d_tmp.setData(erode_tmp,sz[0],sz[1],sz[2],sz[3],V3D_UINT8);
    callback->setImage(curwin,&image4d_tmp);
    callback->updateImageWindow(curwin);
    callback->close3DWindow(curwin);
    callback->setLandmark(curwin,LList_adj);
    callback->open3DWindow(curwin);
    callback->pushObjectIn3DWindow(curwin);
    edit_flag=true;
    seg_edit_flag=true;
    LList_adj[mid].color.g=LList_in[idx].color.g;
    LList_adj[mid].color.b=LList_in[idx].color.b;
    LList_adj[mid].color.r=LList_in[idx].color.r;
    prev_idx=mid;
}

void manual_proof_is::dilate()
{
    open_main_triview(MAINWINNAME);
    open_trunc_triview(SEGMENT_VIEW);
    edit_seg->clear();
    qDebug()<<"in dilate now"<<sz_img[0]<<":"<<sz_img[1]<<":"<<sz_img[2];
    V3DLONG size_page=sz_img[0]*sz_img[1]*sz_img[2];
    V3DLONG z_offset=sz_img[0]*sz_img[1];

    int mid=list_markers->currentRow();
    QListWidgetItem *sel=list_markers->item(mid);
    LList_adj[mid].color.r=LList_adj[mid].color.b=255;
    LList_adj[mid].color.g=255;
    QString tmp_comment = QString::fromStdString((LList_adj.at(mid).comments));

    if (tmp_comment.contains("1")||tmp_comment.contains("3")||tmp_comment.contains("5"))
      sel->setText("marker " + QString::number(mid+1) + " on dendrite");
    else
      sel->setText("marker " + QString::number(mid+1) + " on spine");

    QString tmp_name=QString::fromStdString(LList_adj[mid].name);
    int idx=tmp_name.toInt()-1;//idx in LList_in starting from 1
    GOI tmp_group = voxel_group[idx];

    vector<V3DLONG> dilateList,totalList;
    QHash<V3DLONG, bool> hash;
    for (int i=0;i<tmp_group.voxel.size();i++)
        hash.insert(tmp_group.voxel.at(i),true);

    int dilate_count;
    vector<V3DLONG> coord;
    for (int i=0;i<tmp_group.voxel.size();i++)
    {

        V3DLONG pos= tmp_group.voxel[i];
        dilate_count=0;
        coord= pos2xyz(pos,sz_img[0],sz_img[0]*sz_img[1]);
        V3DLONG x=coord[0];
        V3DLONG y=coord[1];
        V3DLONG z=coord[2];
        //look at 6 nbs and if one is < bgthr_is, it will be dilated this round
        if(pos-1>=0 && !hash.contains(pos-1)&& x-1>=x_start && x-1<=x_end)
        {
            dilateList.push_back(pos-1);
            dilate_count++;
        }
        if(pos+1<size_page && !hash.contains(pos+1)&& x+1>=x_start && x+1<=x_end)
        {
            dilateList.push_back(pos+1);
            dilate_count++;
        }
        if(pos-sz_img[0]>=0 && !hash.contains(pos-sz_img[0]) && y-1>=y_start && y+1<=y_end)
        {
            dilateList.push_back(pos-sz_img[0]);
            dilate_count++;
        }
        if(pos+sz_img[0]<size_page && !hash.contains(pos+sz_img[0]) &&y+1>=y_start && y+1>=y_end)
        {
            dilateList.push_back(pos+sz_img[0]);
            dilate_count++;
        }
        if(pos-z_offset>=0 && !hash.contains(pos-z_offset)  &&z-1>=z_start && z-1>=z_end)
        {
            dilateList.push_back(pos-z_offset);
            dilate_count++;
        }
        if(pos+z_offset<size_page && !hash.contains(pos+z_offset)&&z+1>=z_start && z+1>=z_end)
        {
            dilateList.push_back(pos+z_offset);
            dilate_count++;
        }
        if (dilate_count<auto_para.dilateNB)
            dilateList.clear();
        else
        {
            totalList.insert(totalList.end(),dilateList.begin(),dilateList.end());
            for (int k=0;k<dilateList.size();k++)
                hash.insert(dilateList[k],true);
        }
    }
    if (totalList.size()==0)
    {
       edit_seg->setPlainText("No voxels available. Cannot dilate");
       return;
    }

    edit_seg->setPlainText(QString::number(totalList.size())+" voxels increased in this round of dilation. "
                           +QString::number(totalList.size()+tmp_group.voxel.size())+ " voxels in total");
    qDebug()<<"~~~dilate~~~~increase:"<<totalList.size();

    for (int i =0;i<totalList.size();i++)
    {
        vector<V3DLONG> coord= pos2xyz(totalList.at(i),sz_img[0],sz_img[0]*sz_img[1]);
        //image_is[dilateList.at(i)+2*size_page]=255; //need to maintain image_is here?
        V3DLONG x= coord[0]-x_start;
        V3DLONG y= coord[1]-y_start;
        V3DLONG z= coord[2]-z_start;
        V3DLONG trun_pos = xyz2pos(x,y,z,sz[0],sz[0]*sz[1]);
        image_trun[trun_pos+2*sz[0]*sz[1]*sz[2]]=255;
    }
    totalList.insert(totalList.end(),tmp_group.voxel.begin(),tmp_group.voxel.end());
    voxel_group[idx].voxel.clear();
    voxel_group[idx].voxel=totalList;
    unsigned char *dilate_tmp =new unsigned char [sz[0]*sz[1]*sz[2]*sz[3]];
    memcpy(dilate_tmp,image_trun,sz[0]*sz[1]*sz[2]*3);

    Image4DSimple image4d_tmp;
    image4d_tmp.setData(dilate_tmp,sz[0],sz[1],sz[2],sz[3],V3D_UINT8);
    callback->setImage(curwin,&image4d_tmp);
    callback->updateImageWindow(curwin);
    callback->close3DWindow(curwin);
    callback->setLandmark(curwin,LList_adj);
    callback->open3DWindow(curwin);
    callback->pushObjectIn3DWindow(curwin);
    edit_flag=true;
    seg_edit_flag=true;
    LList_adj[mid].color.g=LList_in[idx].color.g;
    LList_adj[mid].color.b=LList_in[idx].color.b;
    LList_adj[mid].color.r=LList_in[idx].color.r;
    prev_idx=mid;
}

void manual_proof_is::marker_in_one_seg()
{
    qDebug()<<"in markerin one seg";
    edit_seg->clear();
    qDebug()<<"edit_flag:"<<edit_flag;
    int mid=list_markers->currentRow();
    QString tmp_name=QString::fromStdString(LList_adj.at(mid).name);
    int idx=tmp_name.toInt()-1;//idx in LList_in starting from 1

    if (edit_flag)
    {
       int ret;
       ret=save_edit();
       if (ret==4)
       {
           list_markers->setCurrentItem(list_markers->item(prev_idx));
           return;
       }
       else
        list_markers->setCurrentItem(list_markers->item(mid));
    }

//    //Focus on this marker on tri-view
    TriviewControl * p_control = callback->getTriviewControl(curwin);
    p_control->setFocusLocation((long)LList_adj.at(mid).x+1,
                                (long)LList_adj.at(mid).y+1,(long)LList_adj.at(mid).z+1);

    //update marker in 2 tri-view and one 3D
    // if this markers is not determined,Landmark color change

    LList_adj[mid].color.r=LList_adj[mid].color.b=255;
    LList_adj[mid].color.g=255;

    callback->setLandmark(curwin,LList_adj);
    callback->updateImageWindow(curwin);
    callback->pushObjectIn3DWindow(curwin);
    callback->setLandmark(main_win,LList_in);

    //Step 6: reset marker color back to original color
//    LList_in[idx].color.r=LList_in[idx].color.b=LList_in[idx].color.g=0;
    LList_adj[mid].color.r=LList_in[idx].color.r;
    LList_adj[mid].color.b=LList_in[idx].color.b;
    LList_adj[mid].color.g=LList_in[idx].color.g;
    //qDebug()<<"~~~~marker roi finished";
    prev_idx=mid;
}

int manual_proof_is::save_edit()
{
    QMessageBox mybox;
    mybox.setText("<b>The spine has been edited.<\b>");
    QString info="-Accept: save edit and keep the spine<br> -Delete: delete the spine<br>"
            "-Discard: discard the edit";
    mybox.setInformativeText(info);

    QPushButton *accept_button=mybox.addButton(tr("Accept"),QMessageBox::ActionRole);
    QPushButton *delete_button=mybox.addButton(tr("Delete"),QMessageBox::ActionRole);
    QPushButton *discard_button=mybox.addButton(QMessageBox::Discard);
    QPushButton *cancel_button=mybox.addButton(QMessageBox::Cancel);

    mybox.setDefaultButton(accept_button);
    mybox.exec();

    list_markers->setCurrentItem(list_markers->item(prev_idx));

     if (mybox.clickedButton() == accept_button) {
         edit_flag=false;
         accept_marker();
         return 1;
     } else if (mybox.clickedButton() == cancel_button) {
         return 4;
     }
     else if (mybox.clickedButton() == discard_button) {
         reset_edit(false,prev_idx);
         edit_flag=false;

         return 3;
     } else if (mybox.clickedButton()==delete_button)
     {
         edit_flag=false;
         reject_marker();
         return 2;
     }
}

void manual_proof_is::reset_marker_clicked()
{
    int mid=list_markers->currentRow();
    reset_edit(true,mid);
}

void manual_proof_is::reset_edit(bool visual_flag,int mid)
{
    //qDebug()<<"in reset_edit"<<"trun image size:"<<sz[1]<<":"<<sz[3]<<"visual flag:"<<visual_flag;
    //qDebug()<<"xyz_min:"<<x_start<<":"<<y_start<<":"<<z_start;
    edit_seg->clear();

    if (visual_flag)
    {
        LList_adj[mid].color.r=LList_adj[mid].color.b=255;
        LList_adj[mid].color.g=255;
        LList_adj[mid].comments=QString("0").toStdString().c_str();
        edit_seg->setPlainText("marker: "+ QString::number(mid+1)+ " reset");
    }

    QString tmp_name=QString::fromStdString(LList_adj.at(mid).name);
    int idx=tmp_name.toInt()-1;//idx in LList_in starting from 1

    V3DLONG size_page=sz[0]*sz[1]*sz[2];
    //1 not reviewed_dendrite
    //2 not reviewed_spine
    GOI tmp_group=voxel_group[idx];
    if (tmp_group.on_dendrite)
    {
        //qDebug()<<"On dendrite";
        list_markers->item(mid)->setText("marker " + QString::number(mid+1) + " on dendrite");
        list_markers->item(mid)->setBackgroundColor(Qt::white);
        LList_in[idx].comments=QString("1").toStdString().c_str();
        LList_adj[mid].comments=QString("1").toStdString().c_str();
        LList_in[idx].color.r=LList_adj[mid].color.r=85;
        LList_in[idx].color.g=LList_adj[mid].color.g=170;
        LList_in[idx].color.b=LList_adj[mid].color.b=255;
    }
    else
    {
        //qDebug()<<"On spine";
        list_markers->item(mid)->setText("marker " + QString::number(mid+1) + " on spine");
        list_markers->item(mid)->setBackgroundColor(Qt::white);
        LList_in[idx].comments=QString("2").toStdString().c_str();
        LList_adj[mid].comments=QString("2").toStdString().c_str();
        LList_in[idx].color.r=LList_adj[mid].color.r=255;
        LList_in[idx].color.g=LList_adj[mid].color.g=0;
        LList_in[idx].color.b=LList_adj[mid].color.b=255;
    }

    //update image_seg
    for (int i=0;i<tmp_group.voxel.size();i++)
    {
        vector<V3DLONG> coord= pos2xyz(tmp_group.voxel.at(i),sz_img[0],sz_img[0]*sz_img[1]);
        V3DLONG x= coord[0]-x_start;
        V3DLONG y= coord[1]-y_start;
        V3DLONG z= coord[2]-z_start;
        V3DLONG trun_pos = xyz2pos(x,y,z,sz[0],sz[0]*sz[1]);
        image_trun[trun_pos+2*sz[0]*sz[1]*sz[2]]=0;
    }

    tmp_group=voxel_group_copy[idx];

    for (int i=0;i<tmp_group.voxel.size();i++)
    {
        vector<V3DLONG> coord= pos2xyz(tmp_group.voxel.at(i),sz_img[0],sz_img[0]*sz_img[1]);
        //image_is[dilateList.at(i)+2*size_page]=255; //need to maintain image_is here?
        V3DLONG x= coord[0]-x_start;
        V3DLONG y= coord[1]-y_start;
        V3DLONG z= coord[2]-z_start;
        V3DLONG trun_pos = xyz2pos(x,y,z,sz[0],sz[0]*sz[1]);
        image_trun[trun_pos+2*sz[0]*sz[1]*sz[2]]=255;
    }
    //qDebug()<<"original voxels:"<<voxel_group[idx].voxel.size()<< "new size:"<<tmp_group.voxel.size();
    voxel_group[idx].voxel.clear();
    voxel_group[idx].voxel=tmp_group.voxel;

    unsigned char *reset_tmp =new unsigned char [size_page*sz[3]];
    memcpy(reset_tmp,image_trun,sz[0]*sz[1]*sz[2]*sz[3]);

    edit_flag=false;
    seg_edit_flag=true;

    if (!visual_flag)
        return;
    else
    {
        Image4DSimple image4d_tmp;
        image4d_tmp.setData(reset_tmp,sz[0],sz[1],sz[2],sz[3],V3D_UINT8);
        callback->setImage(curwin,&image4d_tmp);
        callback->updateImageWindow(curwin);
        callback->close3DWindow(curwin);
        callback->setLandmark(curwin,LList_adj);
        callback->open3DWindow(curwin);
        callback->pushObjectIn3DWindow(curwin);
    }
    LList_adj[mid].color.r= LList_in[idx].color.r;
    LList_adj[mid].color.b= LList_in[idx].color.b;
    LList_adj[mid].color.g= LList_in[idx].color.g;
}

void manual_proof_is::skip_segment_clicked()
{
    int seg_id=segments->currentIndex();
    if (seg_id+1<segment_neuronswc.size())
        segments->setCurrentIndex(seg_id+1);
    else
    {
        edit_seg->clear();
        edit_seg->setPlainText("This is the last segment.");
        return;
    }
}

void manual_proof_is::finish_seg_clicked()
{
    for (int i=0;i<LList_adj.size();i++)
    {
        QString tmp_comment= QString::fromStdString(LList_adj[i].comments);
        QString tmp_name=QString::fromStdString(LList_adj[i].name);
        int idx=tmp_name.toInt()-1;
        if(tmp_comment.contains("1"))
        {
          LList_adj[i].color.r =LList_in[idx].color.r =0;
          LList_adj[i].color.b =LList_in[idx].color.b =0;
          LList_adj[i].color.g =LList_in[idx].color.g =255;
          LList_in[idx].comments=QString::number(3).toStdString(); //accept marker
          LList_adj[i].comments=QString::number(3).toStdString();
          list_markers->item(i)->setText("marker "+ QString::number(i+1)+ " on dendrite. Accepted");
          list_markers->item(i)->setBackgroundColor(green);
        }
        else if (tmp_comment.contains("2"))
        {
            LList_adj[i].color.r =LList_in[idx].color.r =0;
            LList_adj[i].color.b =LList_in[idx].color.b =0;
            LList_adj[i].color.g =LList_in[idx].color.g =255;
            LList_in[idx].comments=QString::number(4).toStdString(); //accept marker
            LList_adj[i].comments=QString::number(4).toStdString();
            list_markers->item(i)->setText("marker " + QString::number(i+1) + " on spine. Accepted");
            list_markers->item(i)->setBackgroundColor(green);
        }

    }
    int seg_idx=segments->currentIndex();
    segments->setItemText(seg_idx,"Segment "+ QString::number(seg_idx+1)+" finished");
    seg_edit_flag=false;
    callback->setLandmark(curwin,LList_adj);
    callback->pushObjectIn3DWindow(curwin);
    return;
}

void manual_proof_is::reset_segment()
{
    for (int i=0;i<LList_adj.size();i++)
    {
        reset_edit(false,i);
    }
    qDebug()<<"segment reset";
    seg_edit_flag=false;
}

void manual_proof_is::reset_segment_clicked()
{
    reset_segment();
    seg_edit_flag=false;
    int seg_idx=segments->currentIndex();
    segments->setItemText(seg_idx,"Segment "+ QString::number(seg_idx+1));
    segment_change();
}


void manual_proof_is::save()
{
    backUpProjectData();
    int count=csv_generate();
    QString content = "Result saved successfully to project txt "+ baseDir;
    QMessageBox::information(0,"IS Quantifer proofread",content,
                             QMessageBox::Ok);
}

void manual_proof_is::backUpProjectData()
{
//    RAWIMGname=
//    SWCFILEname=
//    init_para_is
//    voxel_group
//    landmarks
    qDebug()<<"in save now";

//    GOI tmpGOI=voxel_group.back(); //for test
//    LocationSimple l=LList_in.at(0); //for test
//    qDebug()<<"size of the first GOI:"<<tmpGOI.voxel.size()<<"nearest node:"<<tmpGOI.nearest_node
//           <<" id:"<<tmpGOI.label_id<<" dendrite? "<<tmpGOI.on_dendrite;
//    qDebug()<<"landmarklist size: "<< LList_in.size()<<"xyz: "<<l.x<<":"
//          <<l.y<<":" <<l.z<<" rgb:"<<l.color.r<<":"<<l.color.g<<":"<<l.color.b;
    auto_para.seg_id=segments->currentIndex();
    auto_para.marker_id=list_markers->currentRow();
    QString fileDefaultName ="IS_quantifier_project.txt";
    QString completName=QDir(baseDir).filePath(fileDefaultName);

    QFile file(completName);

    if (!file.open(QIODevice::WriteOnly))
    {
        v3d_msg("Could not open file");
        return;
    }
    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_4_7);
    out<<input_image_name<<input_swc_name<<auto_para<<voxel_group<<LList_in;
    file.flush();
    file.close();
}

bool manual_proof_is::loadProjectData()
{
    //QString imagename,swcname;
    //init_para_is tmp;
    //QVector<GOI> voxelGroup;
    //LandmarkList result;
    //QFile file("C:\\Users\\Jade\\Documents\\V3d\\ISquantifer.txt");
    qDebug()<<"in load projectData";
    QFile file(input_proj_name);
    QString baseName = input_proj_name.section('/', -1);
    baseDir = input_proj_name;
    baseDir.chop(baseName.size());
    QString errMsg;
    if (!file.open(QIODevice::ReadOnly))
    {
        errMsg = file.errorString();
        qDebug()<<"error:"<<errMsg;
        return false;
    }

    QDataStream in(&file);
    qDebug()<<"before loading the file";
    in.setVersion(QDataStream::Qt_4_7);
    in>>input_image_name>>input_swc_name>>auto_para>>voxel_group>>LList_in;
    file.close();
    qDebug()<<"already load the file";
    if (input_image_name.isEmpty() || input_swc_name.isEmpty() ||voxel_group.size()<=0) //||LList_in.size()<=0
    {
//        if (LList_in.size()<=0)
//            v3d_msg("LList in size <0",0);
        if (voxel_group.size()<=0)
            v3d_msg("Voxel group<=0",0);
        return false;
    }
    QSettings settings("V3D plugin","IS_Quantifer");
    settings.setValue("fname_proj",edit_load->text());

    if (override_img)
        input_image_name=input_image_name2;
    if (override_swc)
        input_swc_name=input_swc_name2;

    //for testing
//    qDebug()<<"autopara values:"<<imagename<<" tmp values:"<<tmp.is_bgthr<<tmp.seg_id;
//    qDebug()<<"tmpGOI_size:"<<tmpGOI.voxel.size()<<" nearest node:"<<tmpGOI.nearest_node
//           <<" label:"<<tmpGOI.label_id<<" dendrite?"<<tmpGOI.on_dendrite;
//    LocationSimple l=result.at(0);
//    qDebug()<<"landmarklist size: "<< result.size()<<"xyz: "<<l.x<<":"
//          <<l.y<<":" <<l.z<<" rgb:"<<l.color.r<<":"<<l.color.g<<":"<<l.color.b;
    qDebug()<<"image name:"<<input_image_name<<"swc name:"<<input_swc_name;
    return true;
}

int manual_proof_is::csv_generate()
{
    //only get the image name
    qDebug()<<"in csv_generate Nov 8-2016";
    QString baseName = input_image_name.section('/', -1);

    QString fileDefaultName=baseName.remove(".v3draw")+"_ISprofile.csv";
    QString completName=QDir(baseDir).filePath(fileDefaultName);
    csv_out_name=QFileDialog::getSaveFileName(this,tr("Save csv"),completName,tr("Supported file: (*.csv)"),0,0);
    if (csv_out_name.isEmpty())
        return -1;

    //Save file to csv
    QFile file(csv_out_name);
    if (!file.open(QIODevice::WriteOnly))
        return -1;

    QTextStream out(&file);

    out<<"# "<<input_image_name<<endl;
    out<<"# "<<input_swc_name<<endl;
    out<<"# for location: on_dendrite=1; on_spine=0"<<endl;
    out<<"#id,volume,x,y,z,nearest_node,location"<<endl;

    int count=0;
    for (int kk=0;kk<voxel_group.size();kk++)
    {
        QString tmp=QString::fromStdString(LList_in[kk].comments);
        if (tmp.contains("5")||tmp.contains("6"))  //delete
            continue;
        if (tmp.contains("1")||tmp.contains("3"))
            out<<kk+1<<","<<voxel_group[kk].voxel.size()<<","<<LList_in[kk].x<<","<<LList_in[kk].y
             <<","<<LList_in[kk].z<<","<<voxel_group[kk].nearest_node<<","<<1<<endl;

        else
            out<<kk+1<<","<<voxel_group[kk].voxel.size()<<","<<LList_in[kk].x<<","<<LList_in[kk].y
             <<","<<LList_in[kk].z<<","<<voxel_group[kk].nearest_node<<","<<0<<endl;
        count++;
    }
    file.flush();
    file.close();
    return count;
}

void manual_proof_is::finish_dialog()
{
    QMessageBox mybox;
    mybox.setText("Have you finished proofreading?");

    QPushButton *save_button = mybox.addButton(tr("Finish and save"),QMessageBox::ActionRole);
    QPushButton *cancel_button=mybox.addButton(QMessageBox::Cancel);
    QPushButton *discard_button=mybox.addButton(QMessageBox::Discard);

    mybox.setDefaultButton(save_button);
    mybox.exec();

     if (mybox.clickedButton() == save_button) {
         int final_landmarks_num = finish_implement();
         QMessageBox mynewbox;
         QString info="After proofreading "+ QString::number(final_landmarks_num)+" IS were found\n";
         info+="The IS csv profile is saved at "+ csv_out_name;
         mynewbox.information(0,"IS quantifier",info,QMessageBox::Ok);
         return;

     } else if (mybox.clickedButton() == cancel_button) {
         return;
     }
     else if (mybox.clickedButton()== discard_button) {
         //need to close all image windows //check 3D window
         v3dhandleList list_triwin = callback->getImageWindowList();
         for(V3DLONG i=0; i<list_triwin.size(); i++){
             if(callback->getImageName(list_triwin.at(i)).contains(SEGMENT_VIEW_PROCESS))
             {
                 callback->close3DWindow(list_triwin[i]);
             }
         }
         mydialog->close();
         return;
     }
}

int manual_proof_is::finish_implement()
{
    open_main_triview(MAINWINNAME);
    backUpProjectData();
    int count=csv_generate();
    if (count <=0)     //prepare csv
        return 0;
    //prepare final window, only showing the accepted ISs.
    V3DLONG size_page=sz_img[0]*sz_img[1]*sz_img[2];
    Image4DSimple image4d;
    unsigned char *input_image=new unsigned char [size_page*3];
    memset(input_image,0,size_page*2);
    memcpy(input_image,image_is,size_page*2);


    LandmarkList LList_out;
    for (int i=0;i<LList_in.size();i++)
    {
        QString tmp;
        tmp=QString::fromStdString(LList_in[i].comments);

        if (tmp.contains("5")||tmp.contains("6"))
        {
            //qDebug()<<"One Marker rejected";
            continue;
        }
        else
        {
            //LList_in[i].on=true;
            LList_out.append(LList_in[i]);
            GOI tmp_group = voxel_group[i];
            for (int j=0;j<tmp_group.voxel.size();j++)
            {
                input_image[2*size_page+tmp_group.voxel[j]]=255;
            }
        }
    }

    mydialog->close();
//    qDebug()<<"closing dialog";

//    //open a final tri-view and 3D
//    callback->setLandmark(main_win,LList_in);
//    //callback->open3DWindow(main_win);
//    callback->pushObjectIn3DWindow(main_win);

    image4d.setData(input_image,sz_img[0],sz_img[1],sz_img[2],3,V3D_UINT8);
    QString final_name=QString(MAINWINNAME) +"_result";
    //v3dhandle test_win=callback->newImageWindow("newwindowforTest");
    callback->setImageName(main_win,final_name);
    callback->setImage(main_win,&image4d);
    callback->setLandmark(main_win,LList_out);
    callback->updateImageWindow(main_win);
    callback->open3DWindow(main_win);
    callback->pushObjectIn3DWindow(main_win);

    return count;
}

bool manual_proof_is::loadImage()
{
    qDebug()<<"in load image:";
    int intype;
    if (!simple_loadimage_wrapper(*callback,input_image_name.toStdString().c_str(), image_is, sz_img, intype))
    {
        return false;
    }
    if (sz_img[3]>3)
    {
        sz_img[3]=3;
        QMessageBox::information(0,"","More than 3 channels were loaded."
                                 "The first 3 channel will be applied for analysis.");
    }

    V3DLONG size_tmp=sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];
    if(intype==1)
    {
    }
    else if (intype == 2) //V3D_UINT16;
    {
        convert2UINT8((unsigned short*)image_is, image_is, size_tmp);
    }
    else if(intype == 4) //V3D_FLOAT32;
    {
        convert2UINT8((float*)image_is, image_is, size_tmp);
    }
    else
    {
        QMessageBox::information(0,"","Currently this program only supports UINT8, UINT16, and FLOAT32 data type.");
        return false;
    }
    return true;
}

bool manual_proof_is::loadSWC()
{
    eswc_flag=false;
    //load swc
    NeuronSWC *p_cur=0;

    neuron = readSWC_file(input_swc_name);
    if (neuron.listNeuron.size()==0)
        return false;
    //checking
    for (V3DLONG ii=0; ii<neuron.listNeuron.size(); ii++)
    {
        p_cur = (NeuronSWC *)(&(neuron.listNeuron.at(ii)));
        if (p_cur->r<0)
        {
            v3d_msg("You have illeagal radius values. Check your data.");
            return false;
        }
     }

    if (input_swc_name.contains(".eswc"))
    {
        V3DLONG sum_level=0;
        bool possible_eswc=true;
        for (V3DLONG ii=0; ii<neuron.listNeuron.size(); ii++)
        {
            p_cur = (NeuronSWC *)(&(neuron.listNeuron.at(ii)));
            sum_level+=p_cur->level;
            if (p_cur->fea_val.size()<2)
            {
                v3d_msg("No additional node info is provided. The csv output will onlly"
                " produce the basic spine info.");
                possible_eswc=false;
                break;
            }
        }
        if (possible_eswc && sum_level!=0)  //this is a eswc file
            eswc_flag=true;
    }

    qDebug()<<"finished reading"<<neuron.listNeuron.size();
    return true;
}
