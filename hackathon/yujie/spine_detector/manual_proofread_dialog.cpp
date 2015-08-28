#include "manual_proofread_dialog.h"
#include "common.h"
#include <Qset>

#define MAINWINNAME "spineDetector_Proofread"
#define SPINE_VIEW "proofread by spine"
#define SEGMENT_VIEW "proofread by segment"

manual_proofread_dialog::manual_proofread_dialog(V3DPluginCallback2 *cb, bool menu_flag)
{
    callback=cb;
    edit_flag=false;
    seg_edit_flag=false;
    image1Dc_in=0;
    image_trun=0;
    image1Dc_spine=0;
    label=0;
    auto_para.bg_thr=auto_para.channel=auto_para.max_dis=-1;
    checked_neuron=0;

    if (menu_flag)
    {
        create();
        initDlg();
        check_btn();
    }
}

void manual_proofread_dialog::create()
{
    QGridLayout *mygridLayout = new QGridLayout;
    QLabel* label_load = new QLabel(QObject::tr("Load project:"));
    mygridLayout->addWidget(label_load,0,0,1,1);
    edit_load = new QLineEdit;
    edit_load->setText(""); edit_load->setReadOnly(true);
    mygridLayout->addWidget(edit_load,0,1,1,6);
    QPushButton *btn_load = new QPushButton("...");
    mygridLayout->addWidget(btn_load,0,7,1,1);

//    QLabel *label_view = new QLabel(tr("Which proofread view?"));
//    view_menu = new QComboBox;
//    view_menu->addItem("View by spine");
//    view_menu->addItem("View by segment");
//    mygridLayout->addWidget(label_view,3,0,1,2);
//    mygridLayout->addWidget(view_menu,3,3,1,5);

    btn_run = new QPushButton("Run");
    QPushButton *cancel = new QPushButton("Cancel");
    mygridLayout->addWidget(btn_run,16,2,1,2);
    mygridLayout->addWidget(cancel,16,5,1,2);

    this->setLayout(mygridLayout);
    this->setWindowTitle("SpineDetector_proofreading");
    this->show();

    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(btn_load, SIGNAL(clicked()), this, SLOT(get_proj_name()));
    connect(btn_run,SIGNAL(clicked()),this,SLOT(run_saved_proj()));
}

void manual_proofread_dialog::initDlg()
{
    QSettings settings("V3D plugin","spine_detector");
    if (settings.contains("fname_proj"))
    {
        this->edit_load->setText(settings.value("fname_proj").toString());
        input_proj_name=edit_load->text();
    }
}

void manual_proofread_dialog::run_saved_proj()
{
    qDebug()<<"running";

    if (!check_btn())
    {
        v3d_msg("You have not provided valid input/output");
        return;
    }
    this->accept();;
    if (!readProjFile())
    {
        v3d_msg("Loading project txt error");
        return;
    }
    if (!loadImage())
    {
        v3d_msg("Invalid image.Error");
        return;
    }
    if (!loadSWC())
    {
        v3d_msg("Invalid swc. Error");
        return;
    }
    if (!loadLabel())
    {
        v3d_msg("Invalid label.Error");
        return;
    }
    if (!loadMarker(LList_in))
    {
        v3d_msg("Invalid marker.Error");
        return;
    }
    if (!prep_spine_voi())//get ready label_group and image1Dc_spine
    {
        v3d_msg("Initiation error");
        return;
    }
    qDebug()<<"LList set";

    if (auto_para.seg_id==-1)  //view by spine
    {
        view_code=1;
        create_proofread_panel_by_spine();
    }
    else if (auto_para.seg_id>=0) //view by segment
    {
        view_code=2;
        checked_neuron=check_neuron_tree(neuron,sz_img);
        segment_neuronswc=neurontree_divide_swc(checked_neuron,auto_para.max_dis*6);
        check_each_seg(); //set up finish_FLAG
        create_proofread_panel_by_segment(true);
    }
}

void manual_proofread_dialog::check_each_seg()
{
    qDebug()<<"in check each seg"<<segment_neuronswc.size()<<" LList"<<LList_in.size();

    finish_flag.resize(segment_neuronswc.size(),0);

    QSet<int> seg_candid;
    for (int i=0;i<LList_in.size();i++)
    {
        qDebug()<<"I:"<<i<<" size:"<<LList_in[i].category;
        if (LList_in[i].category!=0)
            seg_candid<<LList_in[i].category;
    }
    qDebug()<<"seg_candid:"<<seg_candid.size();
    int count=0;
    QSet<int>::const_iterator i = seg_candid.constBegin();
    while (i != seg_candid.constEnd()) {
        qDebug()<<*i;
        for (int w=0;w<LList_in.size();w++)
        {
            if (LList_in[w].category==*i && QString::fromStdString(LList_in[w].comments).contains("0"))
            {
                goto stop;
            }
        }
        finish_flag[(*i)-1]=true;
        count++;
        stop: {}
        ++i;
    }
    qDebug()<<"count:"<<count;

//    for (int i=0;i<segment_neuronswc.size();i++)
//    {
//        vector<V3DLONG> coord(6,0);
//        coord=image_seg_plan(segment_neuronswc[i],checked_neuron);
//        for (int j=0;j<LList_in.size();j++)
//        {
//            if (LList_in[j].x-1<=coord[0]|| LList_in[j].x-1>=coord[3] || LList_in[j].y-1<=coord[1]
//                || LList_in[j].y-1>=coord[4] || LList_in[j].z-1<=coord[2] ||LList_in[j].z-1>=coord[5] )
//                continue;
//            QString tmp_str=QString::fromStdString(LList_in[j].comments);
//            if(tmp_str.contains("0"))
//            {
//                finish_flag[i]=false;
//                goto stop;
//            }
//        }
//        finish_flag[i]=true;
//        stop: {}
//    }
}

void manual_proofread_dialog::run_interface_with_auto(int ret,NeuronTree auto_neuron,bool eswc_tmp_flag,
    LandmarkList LList,vector<GOV> label_group_in,unsigned char *image1Dc,V3DLONG sz_img_in[4],
    int bg_thr,int max_dis,int channel,QString in_swc_name,QString in_image_name,QString folder_output)
{
    //get related name
    input_swc_name=in_swc_name;
    input_image_name=in_image_name;
    baseDir=folder_output;
    eswc_flag=eswc_tmp_flag;

    //get ready LList, neuron,image,para,label_group,label_group_copy
    LList_in=LList;
    label_group=label_group_in;
    //make copy for label_group
    label_group_copy=label_group;

    neuron.copy(auto_neuron);

    sz_img[0]=sz_img_in[0];
    sz_img[1]=sz_img_in[1];
    sz_img[2]=sz_img_in[2];
    sz_img[3]=1;

    V3DLONG size_page=sz_img[0]*sz_img[1]*sz_img[2];
    image1Dc_spine = new unsigned char [size_page*3];
    memset(image1Dc_spine,0,size_page*3);
    memcpy(image1Dc_spine,image1Dc,size_page);

    for(int i=0; i<label_group.size(); i++)
    {
        GOV tmp = label_group[i];
        for (int j=0; j<tmp.size(); j++)
        {
            image1Dc_spine[tmp.at(j)->pos+size_page]=255;
        }
    }

    //get ready para
    auto_para.bg_thr=bg_thr;
    auto_para.max_dis=max_dis;
    auto_para.marker_id=0;
    auto_para.channel=channel;

    if (ret==1)
    {
        view_code=2;
        auto_para.seg_id=0;
        checked_neuron=check_neuron_tree(neuron,sz_img);
        segment_neuronswc=neurontree_divide_swc(checked_neuron,auto_para.max_dis*6);
        create_proofread_panel_by_segment(false);

    }
    else if (ret==2)
    {
        view_code=1;
        auto_para.seg_id=-1;
        create_proofread_panel_by_spine();
    }
    else
        return;
}

bool manual_proofread_dialog::loadMarker(LandmarkList &LList_in)
{
    QFile qf(input_marker_name);
    if (! qf.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }
    LList_in.clear();
    V3DLONG k=0;
    while (! qf.atEnd())
    {
        char curline[2000];
        qf.readLine(curline, sizeof(curline));
        k++;
        {
            if (curline[0]=='#' || curline[0]=='x' || curline[0]=='X' || curline[0]=='\0') continue;

            QStringList qsl = QString(curline).trimmed().split(",");
            int qsl_count=qsl.size();
            if (qsl_count<3)   continue;
            if (qsl_count<11)
                return false;
            LocationSimple S;

            S.x = qsl[0].toFloat();
            S.y = qsl[1].toFloat();
            S.z = qsl[2].toFloat();
            S.radius = (qsl_count>=4) ? qsl[3].toInt() : 0;
            S.shape = (PxLocationMarkerShape) 0; //(qsl_count>=5) ? qsl[4].toInt() : 1
            S.name = (qsl_count>=6) ? qPrintable(qsl[5].trimmed()) : "";
            S.comments = (qsl_count>=7) ? qPrintable(qsl[6].trimmed()) : "";

            S.color = random_rgba8(255);
            if (qsl_count>=8) S.color.r = qsl[7].toUInt();
            if (qsl_count>=9) S.color.g = qsl[8].toUInt();
            if (qsl_count>=10) S.color.b = qsl[9].toUInt();
            if (qsl_count>=11) S.category=qsl[10].toUInt();  //needs to be super careful

            S.on = true; //listLoc[i].on;        //true;
            LList_in.append(S);
        }
    }
    return true;
}

bool manual_proofread_dialog::prep_spine_voi()
{
   qDebug()<<"prep spines";
    spine_fun spine_obj(callback,auto_para.channel,auto_para.bg_thr,auto_para.bg_thr,label);
   if (!spine_obj.pushImageData(image1Dc_in,sz_img))
       return false;
   spine_obj.pushSWCData(neuron);

   //get ready label_group
   if(!spine_obj.init_for_manual_proof(label_group)){
       v3d_msg("No spine candidates were found. Please check image and swc file");
       return false;
   }
   //get a copy
   label_group_copy=label_group;
   V3DLONG size_page=sz_img[0]*sz_img[1]*sz_img[2];
   image1Dc_spine = new unsigned char [size_page*3];
   memset(image1Dc_spine,0,size_page*3);
   memcpy(image1Dc_spine,image1Dc_in,size_page);

   for(int i=0; i<label_group.size(); i++)
   {
       GOV tmp = label_group[i];
       for (int j=0; j<tmp.size(); j++)
       {
           image1Dc_spine[tmp.at(j)->pos+size_page]=255;
       }
   }
   if (image1Dc_in!=0)
   {
       delete[] image1Dc_in; image1Dc_in=0;
   }

   qDebug()<<"initiaion succeed";
   return true;
}


void manual_proofread_dialog::create_proofread_panel_by_spine()
{
    mydialog=new QDialog;
    mydialog->setWindowTitle("SpineDetector_proofreading_by_spine");
    markers=new QComboBox;
    for (int i=0;i<LList_in.size();i++)
    {
        QString tmp;
        tmp=QString::fromStdString(LList_in[i].comments);
        if (tmp.contains("0"))
            markers->addItem("marker "+QString::number(i+1));
        else if (tmp.contains("1"))
            markers->addItem("marker "+QString::number(i+1)+" accepted");
        else if (tmp.contains("2"))
            markers->addItem("marker "+QString::number(i+1)+" rejected");
    }
    markers->setCurrentIndex(auto_para.marker_id);
    QGridLayout *layout2=new QGridLayout;
    edit_status = new QPlainTextEdit;
    edit_status->setReadOnly(true);
    edit_status->setFixedHeight(50);
    layout2->addWidget(markers,0,0,1,4);
    layout2->addWidget(edit_status,1,0,1,6);

    QPushButton *accept=new QPushButton(tr("Accept"));
    QPushButton *reject=new QPushButton(tr("Delete"));
    QPushButton *skip = new QPushButton(tr("Skip"));

    layout2->addWidget(accept,3,0,1,2);
    layout2->addWidget(reject,3,2,1,2);
    layout2->addWidget(skip,3,4,1,2);

    QPushButton *dilate = new QPushButton(tr("Dilate"));
    QPushButton *erode =new QPushButton (tr("Erode"));
    QPushButton *reset = new QPushButton (tr("Reset spine"));
    layout2->addWidget(dilate,4,0,1,2);
    layout2->addWidget(erode,4,2,1,2);
    layout2->addWidget(reset,4,4,1,2);

    QFrame *line_1 = new QFrame();
    line_1->setFrameShape(QFrame::HLine);
    line_1->setFrameShadow(QFrame::Sunken);
    layout2->addWidget(line_1,5,0,1,6);

    QPushButton *button_save=new QPushButton(tr("Save current results"));
    layout2->addWidget(button_save,6,0,1,2);
    QPushButton *button_finish=new QPushButton(tr("Save and finish"));
    layout2->addWidget(button_finish,6,2,1,2);
//    small_remover=new QCheckBox;
//    small_remover->setText(QObject::tr("Remove groups < min spine pixel"));
//    small_remover->setChecked(false);
//    layout2->addWidget(small_remover,6,2,1,4);
//    QPushButton *button_p_cancel=new QPushButton;
//    button_p_cancel->setText("Quit");
//    layout2->addWidget(button_p_cancel,6,2,1,1);

    mydialog->setLayout(layout2);

    connect(button_save,SIGNAL(clicked()),this,SLOT(save_current_project()));
    connect(button_finish,SIGNAL(clicked()),this,SLOT(finish_proof_dialog()));
    connect(markers,SIGNAL(currentIndexChanged(int)),this,SLOT(marker_roi()));
    connect(accept,SIGNAL(clicked()),this,SLOT(accept_marker()));
    connect(reject,SIGNAL(clicked()),this,SLOT(delete_marker()));
    connect(skip,SIGNAL(clicked()),this,SLOT(skip_marker()));
    connect(dilate,SIGNAL(clicked()),this,SLOT(dilate()));
    connect(erode,SIGNAL(clicked()),this,SLOT(erode()));
    connect(reset,SIGNAL(clicked()),this,SLOT(reset_edit()));

    marker_roi();
    mydialog->show();
}

bool manual_proofread_dialog::check_btn()
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


bool manual_proofread_dialog::get_proj_name()
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
        QSettings settings("V3D plugin","spine_detector");
        settings.setValue("proj_name",edit_load->text());
        return true;
    }
}

bool manual_proofread_dialog::readProjFile()
{
    //load all other data
    qDebug()<<"loading projects";
    QFile file(input_proj_name);
    QString baseName = input_proj_name.section('/', -1);
    baseDir = input_proj_name;
    baseDir.chop(baseName.size());

    if (!file.open(QIODevice::ReadOnly))
        return false;
    QString tmp = file.readAll();
    file.close();

    if (tmp.isEmpty())
        return false;

    QStringList tmpList = tmp.split(QString("\n"));
    if (tmpList.size()<=0)
    {
        return false; //must not be a valid file
    }
    qDebug()<<"in load project file"<<tmpList.size();
    int cnt=0;
    for (int i=0;i<tmpList.size(); i++)
    {
        qDebug()<<"i:"<<tmpList.at(i);
        //printf("(%s)\n", tmpList.at(i).toAscii().data());
        //QStringList itemList;

        if (tmpList.at(i).isEmpty() || tmpList.at(i).at(0)=='#') //a comment line, do nothing
            continue;
        else if (tmpList.at(i).contains("RAWIMG"))
        {
            input_image_name=tmpList.at(i).split("=").back();
            cnt++;
        }
        else if (tmpList.at(i).contains("SWCFILE"))
        {
            input_swc_name=tmpList.at(i).split("=").back();
            cnt++;
        }
        else if (tmpList.at(i).contains("MASKIMG"))
        {
            input_label_name=tmpList.at(i).split("=").back();
            cnt++;
        }
        else if (tmpList.at(i).contains("MARKER"))
        {
            input_marker_name=tmpList.at(i).split("=").back();
            cnt++;
        }
        else if (tmpList.at(i).contains("CSV"))
        {
            input_csv_name=tmpList.at(i).split("=").back();
            cnt++;
        }
        else if (tmpList.at(i).contains("PARA_BG"))
        {
            QString bg=tmpList.at(i).split("=").back();
            auto_para.bg_thr=bg.toInt();
            cnt++;
        }
        else if (tmpList.at(i).contains("PARA_MAXDIS"))
        {
            QString max_dis=tmpList.at(i).split("=").back();
            auto_para.max_dis=max_dis.toInt();
            cnt++;
        }
        else if (tmpList.at(i).contains("PARA_CHANNEL"))
        {
            QString channel=tmpList.at(i).split("=").back();
            auto_para.channel=channel.toInt();
            cnt++;
        }
        else if (tmpList.at(i).contains("SEG_ID"))
        {
            QString seg_id_string=tmpList.at(i).split("=").back();
            auto_para.seg_id=seg_id_string.toInt();
            cnt++;
        }
        else if (tmpList.at(i).contains("MARKER_ID"))
        {
            QString marker_id_string=tmpList.at(i).split("=").back();
            auto_para.marker_id=marker_id_string.toInt();
            cnt++;
        }
        else
            continue;
    }
    if (auto_para.bg_thr<0||auto_para.channel<0||auto_para.max_dis<0)
        return false;
    qDebug()<<"image:"<<input_image_name<<endl;
    qDebug()<<"swc:"<<input_swc_name<<endl;
    qDebug()<<"marker:"<<input_marker_name<<endl;
    qDebug()<<"bg_thr:"<<auto_para.bg_thr<<"maxdis:"<<auto_para.max_dis<<"channel:"<<auto_para.channel;
    qDebug()<<"marker id:"<<auto_para.marker_id;
    qDebug()<<"seg id:"<<auto_para.seg_id<<endl;
    qDebug()<<"cnt"<<cnt<<endl;
    QSettings settings("V3D plugin","spine_detector");
    settings.setValue("fname_proj",edit_load->text());
    return (cnt==9) ? true : false;
}

bool manual_proofread_dialog::loadImage()
{
    qDebug()<<"in load image:";
    int intype;
    if (!simple_loadimage_wrapper(*callback,input_image_name.toStdString().c_str(), image1Dc_in, sz_img, intype))
    {
        QMessageBox::information(0,"","load image "+input_image_name+" error!");
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
        convert2UINT8((unsigned short*)image1Dc_in, image1Dc_in, size_tmp);
    }
    else if(intype == 4) //V3D_FLOAT32;
    {
        convert2UINT8((float*)image1Dc_in, image1Dc_in, size_tmp);
    }
    else
    {
        QMessageBox::information(0,"","Currently this program only supports UINT8, UINT16, and FLOAT32 data type.");
        return false;
    }
    return true;
}

bool manual_proofread_dialog::loadSWC()
{
    eswc_flag=false;
    //load swc
    NeuronSWC *p_cur=0;
    neuron = readSWC_file(input_swc_name);
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

bool manual_proofread_dialog::loadLabel()
{
    qDebug()<<"in load label:";
    V3DLONG label_sz_img[4];
    int intype;
    unsigned char * tmp_img=0;
    if (!simple_loadimage_wrapper(*callback,input_label_name.toStdString().c_str(),tmp_img,label_sz_img, intype))
    {
        QMessageBox::information(0,"","load image "+input_image_name+" error!");
        return false;
    }
    if (label_sz_img[3]>1)
    {
        label_sz_img[3]=1;
        v3d_msg("More than 1 channel loaded."
                                 "The first channel will be applied for analysis.",0);
    }

    if (intype!=2)
    {
        QMessageBox::information(0,"","Currently only take UINT16 label type.");
        delete [] tmp_img; tmp_img=0;
        return false;
    }
    else
    {
        label=(unsigned short *)tmp_img;
        return true;
    }
}

void manual_proofread_dialog::marker_roi()
{
    qDebug()<<"in marker roi";
    if(markers->count()==0) return;
    edit_status->clear();
    int mid=markers->currentIndex();

    //check whether previous editing needs saving
    if (edit_flag) save_edit();
    edit_flag=false;

    //step 1: check whether main-triview is open
    open_main_triview(MAINWINNAME);

    //step 2:set data ready and open local tri-view/3d window
    reset_image_data();
    adjust_LList_to_imagetrun();  //get ready LList_adj

    //step 3: check whether local triview is open
    open_trunc_triview(SPINE_VIEW);

    //Step 4: Focus on this marker on tri-view
    TriviewControl * p_control = callback->getTriviewControl(curwin);
    p_control->setFocusLocation((long)LList_adj.at(mid).x,
                                (long)LList_adj.at(mid).y,(long)LList_adj.at(mid).z);

    //Step 5: update marker in 2 tri-view and one 3D
    // if this markers is not determined,Landmark color change
    if (QString::fromStdString(LList_in[mid].comments).contains("0"))
    {
        LList_adj[mid].color.r=LList_adj[mid].color.b=255;
        LList_adj[mid].color.g=70;
    }
    Image4DSimple image4d;
    unsigned char * image_input=new unsigned char [sz[0]*sz[1]*sz[2]*sz[3]];
    memcpy(image_input,image_trun,sz[0]*sz[1]*sz[2]*sz[3]);
    image4d.setData(image_input,sz[0],sz[1],sz[2],sz[3],V3D_UINT8);
    callback->setImage(curwin,&image4d);
    callback->setLandmark(curwin,LList_adj);
    callback->updateImageWindow(curwin);
    callback->close3DWindow(curwin);
    callback->open3DWindow(curwin);
    callback->pushObjectIn3DWindow(curwin);
    callback->setLandmark(main_win,LList_in);

    //Step 6: reset marker color back to original color
    LList_adj[mid].color.r=LList_adj[mid].color.b=LList_in[mid].color.g=0;
    //qDebug()<<"~~~~marker roi finished";
}

void manual_proofread_dialog::accept_marker()
{
    qDebug()<<"in accept marker";
    open_main_triview(MAINWINNAME);
    open_trunc_triview(SPINE_VIEW);

    //update landmarks color and status
    int mid=markers->currentIndex();
    LList_in[mid].color.r=LList_in[mid].color.b=0;
    LList_in[mid].color.g=255;
    LList_adj[mid].color.r=LList_in[mid].color.b=0;
    LList_adj[mid].color.g=255;

    QString tmp=QString::number(1);
    LList_in[mid].comments=tmp.toStdString(); //accepted marker
    callback->setLandmark(curwin,LList_in);

    //update Qcombobox
    markers->setItemText(mid, "Marker: " + QString::number(mid+1) + " accepted");

    edit_flag=false;
    //go to next marker
    if(mid+1<markers->count()){
        markers->setCurrentIndex(mid+1);
    }
}

void manual_proofread_dialog::delete_marker()
{
    open_main_triview(MAINWINNAME);
    open_trunc_triview(SPINE_VIEW);
    qDebug()<<"delete marker";
    //update landmarks color and status
    int mid=markers->currentIndex();

    LList_in[mid].color.g=LList_in[mid].color.b=0;
    LList_in[mid].color.r=255;
    LList_adj[mid].color.g=LList_in[mid].color.b=0;
    LList_adj[mid].color.r=255;

    LList_in[mid].comments=QString::number(2).toStdString(); //rejected marker

    //update combobox
    markers->setItemText(mid, "Marker: " + QString::number(mid+1) + " rejected");
    edit_flag=false;

    if(mid+1<markers->count()){
        markers->setCurrentIndex(mid+1);
    }
}

void manual_proofread_dialog::skip_marker()
{
    open_main_triview(MAINWINNAME);
    open_trunc_triview(SPINE_VIEW);
    if (edit_flag)
    {
        int ret;
        ret=save_edit();
        if (ret!=3)  //not discard
            return;
    }
    int i=markers->currentIndex();
    if(i+1<markers->count())
        markers->setCurrentIndex(i+1);
}

void manual_proofread_dialog::dilate()
{
    open_main_triview(MAINWINNAME);
    open_trunc_triview(SPINE_VIEW);
    //qDebug()<<"in dilate now";
    int bg_thr=auto_para.bg_thr;
    V3DLONG size_page=sz[0]*sz[1]*sz[2];
    GOV seeds_next;

    int mid=markers->currentIndex();
    LList_adj[mid].color.r=LList_adj[mid].color.b=255;
    LList_adj[mid].color.g=70;
    GOV tmp_group = label_group[mid];
    float sum_x,sum_y,sum_z;
    sum_x=sum_y=sum_z=0;

    for (int sid=0;sid<tmp_group.size();sid++)
    {
        VOI * single_voi = tmp_group[sid];
        sum_x+=single_voi->x;
        sum_y+=single_voi->y;
        sum_z+=single_voi->z;
        int label_id=single_voi->intensity_label;
        for (int neid=0; neid<single_voi->neighbors_6.size();neid++)
        {
            VOI *nb=single_voi->neighbors_6[neid];
            if (image1Dc_spine[nb->pos]>bg_thr && nb->intensity_label<1&&nb->x>=x_start &&nb->y>=y_start
                    &&nb->z>=z_start && nb->x<x_end && nb->y<y_end && nb->z<z_end)
            {
                nb->intensity_label=label_id;
                seeds_next.push_back(nb);
            }
        }
    }

    if (seeds_next.size()==0)
    {
        edit_status->setPlainText("No more voxels available. Cannot dilate");
        return;
    }
    else if (seeds_next.size()>0)
    {
        edit_status->setPlainText(QString::number(seeds_next.size())+" voxels added in this round of dilation");
        tmp_group.insert(tmp_group.end(),seeds_next.begin(),seeds_next.end());
        label_group[mid].clear();
        label_group[mid]=tmp_group;
    }

    for (int i=0;i<seeds_next.size();i++)
    {
        VOI * single_voi=seeds_next[i];
        sum_x+=single_voi->x;
        sum_y+=single_voi->y;
        sum_z+=single_voi->z;
        V3DLONG pos_trun=xyz2pos(single_voi->x-x_start,single_voi->y-y_start,single_voi->z-z_start
                                 ,sz[0],sz[0]*sz[1]);
        image_trun[pos_trun+size_page]=255;
        //image_trun[pos_trun+2*size_page]=255;
    }

    //calculate the new center
    LList_in[mid].x=(float)sum_x/tmp_group.size()+1;
    LList_in[mid].y=(float)sum_y/tmp_group.size()+1;
    LList_in[mid].z=(float)sum_z/tmp_group.size()+1;
    LList_adj[mid].x=LList_in[mid].x-x_start;
    LList_adj[mid].y=LList_in[mid].y-y_start;
    LList_adj[mid].z=LList_in[mid].z-z_start;

    unsigned char *dilate_tmp =new unsigned char [sz[0]*sz[1]*sz[2]*sz[3]];
    memcpy(dilate_tmp,image_trun,sz[0]*sz[1]*sz[2]*sz[3]);
    edit_flag=true;

    Image4DSimple image4d_tmp;
    image4d_tmp.setData(dilate_tmp,sz[0],sz[1],sz[2],sz[3],V3D_UINT8);
    callback->setImage(curwin,&image4d_tmp);
    callback->updateImageWindow(curwin);
    callback->close3DWindow(curwin);
    callback->setLandmark(curwin,LList_adj);
    callback->open3DWindow(curwin);
    callback->pushObjectIn3DWindow(curwin);

    LList_adj[mid].color.r=LList_adj[mid].color.b=LList_adj[mid].color.g=0;
}

void manual_proofread_dialog::erode()
{
    open_main_triview(MAINWINNAME);
    open_trunc_triview(SPINE_VIEW);
    //qDebug()<<"in erode now";
    V3DLONG size_page=sz[0]*sz[1]*sz[2];

    int mid=markers->currentIndex();
    GOV tmp_group = label_group[mid];

    LList_adj[mid].color.b=LList_adj[mid].color.r=255;
    LList_adj[mid].color.g=70;

    if (tmp_group.size()==0)
    {
        edit_status->setPlainText("No more voxels left. Cannot erode");
        return;
    }
    sort(tmp_group.begin(),tmp_group.end(),sortfunc_dst_ascend); //ascending
    //qDebug()<<"sort done:"<<"size:"<<tmp_group.size();

    int min_dis=tmp_group.front()->dst;
    int vid_begin, vid_end;
    vid_begin=vid_end=0;

    while(vid_end<tmp_group.size() && tmp_group[vid_end]->dst==min_dis){
        vid_end++;
        continue;
    }
    if (vid_begin==vid_end)
    {
       edit_status->setPlainText("No more voxels left. Cannot erode");
       return;
    }

    for (int i=vid_begin;i<vid_end;i++)
    {
        VOI * single_voi= tmp_group[i];
        single_voi->intensity_label=-1;
        V3DLONG trun_pos= xyz2pos(single_voi->x-x_start ,single_voi->y-y_start,
                                  single_voi->z-z_start,sz[0],sz[0]*sz[1]);
        image_trun[trun_pos+size_page]=0;
    }
    edit_status->setPlainText(QString::number(vid_end-vid_begin)+" voxels deleted in this round of erosion");
    //qDebug()<<"~~~erode~~~~decrease:"<<vid_end-vid_begin;

    float sum_x,sum_y,sum_z;
    sum_x=sum_y=sum_z=0;
    GOV update_group;
    for (int i=vid_end;i<tmp_group.size();i++)
    {
        sum_x+=tmp_group[i]->x;
        sum_y+=tmp_group[i]->y;
        sum_z+=tmp_group[i]->z;
        update_group.push_back(tmp_group[i]);
    }

    label_group[mid].clear();
    label_group[mid]=update_group;

    //calculate the new center
    LList_in[mid].x=(float)sum_x/update_group.size()+1;
    LList_in[mid].y=(float)sum_y/update_group.size()+1;
    LList_in[mid].z=(float)sum_z/update_group.size()+1;
    LList_adj[mid].x=LList_in[mid].x-x_start;
    LList_adj[mid].y=LList_in[mid].y-y_start;
    LList_adj[mid].z=LList_in[mid].z-z_start;

    //qDebug()<<"new group update";
    unsigned char *erode_tmp =new unsigned char [sz[0]*sz[1]*sz[2]*sz[3]];
    memcpy(erode_tmp,image_trun,sz[0]*sz[1]*sz[2]*sz[3]);
    edit_flag=true;

    Image4DSimple image4d_tmp;
    image4d_tmp.setData(erode_tmp,sz[0],sz[1],sz[2],sz[3],V3D_UINT8);
    callback->setImage(curwin,&image4d_tmp);
    callback->updateImageWindow(curwin);
    callback->close3DWindow(curwin);
    callback->setLandmark(curwin,LList_adj);
    callback->open3DWindow(curwin);
    callback->pushObjectIn3DWindow(curwin);
    LList_adj[mid].color.r=LList_adj[mid].color.b=LList_adj[mid].color.g=0;
}

void manual_proofread_dialog::reset_edit()
{
    reset_label_group();
    reset_image_data();
    int mid=markers->currentIndex();
    edit_flag=false;
    LList_adj[mid].color.b=LList_adj[mid].color.r=255;
    LList_adj[mid].color.g=70;
    unsigned char *reset_tmp =new unsigned char [sz[0]*sz[1]*sz[2]*sz[3]];
    memcpy(reset_tmp,image_trun,sz[0]*sz[1]*sz[2]*sz[3]);
    edit_status->setPlainText("Image reset");

    Image4DSimple image4d_tmp;
    image4d_tmp.setData(reset_tmp,sz[0],sz[1],sz[2],sz[3],V3D_UINT8);
    callback->setImage(curwin,&image4d_tmp);
    callback->updateImageWindow(curwin);
    callback->close3DWindow(curwin);
    callback->setLandmark(curwin,LList_adj);
    callback->open3DWindow(curwin);
    callback->pushObjectIn3DWindow(curwin);
    LList_adj[mid].color.r=LList_adj[mid].color.b=LList_adj[mid].color.g=0;
}

int manual_proofread_dialog::save_edit()
{
    QMessageBox mybox;
    mybox.setText("<b>The spine has been edited.<\b>");
    QString info="-Accept: save edit and keep the spine<br> -Delete: delete the spine<br>"
            "-Discard: discard the edit<br> -Cancel: Retrun to window";
    mybox.setInformativeText(info);

    QPushButton *accept_button=mybox.addButton(tr("Accept"),QMessageBox::ActionRole);
    QPushButton *delete_button=mybox.addButton(tr("Delete"),QMessageBox::ActionRole);
    QPushButton *discard_button=mybox.addButton(QMessageBox::Discard);
    QPushButton *cancel_button=mybox.addButton(QMessageBox::Cancel);

    mybox.setDefaultButton(accept_button);
    mybox.exec();

     if (mybox.clickedButton() == accept_button) {
         accept_marker();
         return 1;
     } else if (mybox.clickedButton() == cancel_button) {
         return 4;
     } else if (mybox.clickedButton() == discard_button) {
         reset_image_data();
         reset_label_group();
         edit_flag=false;
         return 3;
     } else if (mybox.clickedButton()==delete_button)
     {
         delete_marker();
         return 2;
     }
}

void manual_proofread_dialog::reset_image_data()
{
    qDebug()<<"reset image data";
    V3DLONG y_offset=sz_img[0];
    V3DLONG z_offset=sz_img[0]*sz_img[1];
    int mid=markers->currentIndex();//start from zero--marker 1

    int halfwindowsize=30;
    //create bounding box
    V3DLONG x,y,z;
    x=LList_in[mid].x-1;
    y=LList_in[mid].y-1;
    z=LList_in[mid].z-1;

    if (x<halfwindowsize) sz[0]=x+halfwindowsize+1;
    else if (x+halfwindowsize>sz_img[0]) sz[0]=sz_img[0]-x+halfwindowsize;
    else sz[0]=2*halfwindowsize+1;

    if (y<halfwindowsize) sz[1]=y+halfwindowsize+1;
    else if (y+halfwindowsize>sz_img[1]) sz[1]=sz_img[1]-y+halfwindowsize;
    else sz[1]=2*halfwindowsize+1;

    if (z<halfwindowsize) sz[2]=z+halfwindowsize+1;
    else if (z+halfwindowsize>sz_img[2]) sz[2]=sz_img[2]-z+halfwindowsize;
    else sz[2]=2*halfwindowsize+1;

    sz[3]=3;

    if (image_trun!=0)
    {
        delete [] image_trun;
        image_trun=0;
    }
    image_trun=new unsigned char[sz[0]*sz[1]*sz[2]*sz[3]];
    memset(image_trun,0,sz[0]*sz[1]*sz[2]*sz[3]);
    qDebug()<<"image_trun allocated";
    //Store the area around the marker in image_trun
    x_start=MAX(0,x-halfwindowsize);
    y_start=MAX(0,y-halfwindowsize);
    z_start=MAX(0,z-halfwindowsize);
    x_end=MIN(sz_img[0],x+halfwindowsize+1);
    y_end=MIN(sz_img[1],y+halfwindowsize+1);
    z_end=MIN(sz_img[2],z+halfwindowsize+1);
    qDebug()<<"halfsize:"<<halfwindowsize<<" x,y,z:"<<x<<":"<<y<<":"<<z;
    qDebug()<<"sz0,sz1,sz2:"<<sz[0]<<":"<<sz[1]<<":"<<sz[2];

    for (V3DLONG dx=x_start;dx<x_end;dx++){
        for (V3DLONG dy=y_start;dy<y_end;dy++){
            for (V3DLONG dz=z_start;dz<z_end;dz++){
                V3DLONG pos=xyz2pos(dx,dy,dz,y_offset,z_offset);
                V3DLONG pos1=xyz2pos(dx-x_start,dy-y_start,dz-z_start,sz[0],sz[0]*sz[1]);
                image_trun[pos1]=image1Dc_spine[pos];
            }
        }
    }

    GOV tmp_group=label_group[mid];
    qDebug()<<"label_group size;"<<tmp_group.size();
    for (int i=0;i<tmp_group.size();i++)
    {
        V3DLONG new_x=tmp_group[i]->x;
        V3DLONG new_y=tmp_group[i]->y;
        V3DLONG new_z=tmp_group[i]->z;
        V3DLONG trun_pos=xyz2pos(new_x-x_start,new_y-y_start,new_z-z_start,sz[0],sz[0]*sz[1]);
        image_trun[trun_pos+sz[0]*sz[1]*sz[2]]=255;
    }
    qDebug()<<"finish setting up image";
}

void manual_proofread_dialog::reset_label_group()
{
    int mid=markers->currentIndex();
    GOV tmp_group=label_group_copy[mid];
    label_group[mid].clear();
    label_group[mid]=tmp_group;
    //need to update the intensity label???
}

void manual_proofread_dialog::adjust_LList_to_imagetrun()
{
    LList_adj.clear();
    for (int i=0;i<LList_in.size();i++)
    {
        LocationSimple tmp;
        tmp.x=LList_in[i].x-x_start;
        tmp.y=LList_in[i].y-y_start;
        tmp.z=LList_in[i].z-z_start;
        tmp.color.r=LList_in[i].color.r;
        tmp.color.g=LList_in[i].color.g;
        tmp.color.b=LList_in[i].color.b;
        tmp.comments=LList_in[i].comments;
        tmp.name=LList_in[i].name;
        LList_adj.append(tmp);
    }
}

int manual_proofread_dialog::finish_and_save(QString window_name) //needs further work
{
    open_main_triview(MAINWINNAME);
    if (!save_current_project())
        return 0;

    //need to close all image windows //check 3D window
    v3dhandleList list_triwin = callback->getImageWindowList();
    for(V3DLONG i=0; i<list_triwin.size(); i++){
        if(callback->getImageName(list_triwin.at(i)).contains(window_name))
        {
            callback->close3DWindow(list_triwin[i]);
        }
    }
    mydialog->close();
    //prepare image and label
    V3DLONG size_page=sz_img[0]*sz_img[1]*sz_img[2];
    //prepare label
    unsigned char *label_color = new unsigned char [size_page*3];
    memset(label_color,0,size_page*3);
    memset(image1Dc_spine+size_page,0,size_page);
    int rgb[3];
    int count=0;

    for (int i=0;i<LList_in.size();i++)
    {
        QString tmp;
        tmp=QString::fromStdString(LList_in[i].comments);

        if (tmp.contains("1")||tmp.contains("0"))
        {
            if (tmp.contains("1"))
                count++;
            GOV tmp_group=label_group[i];

            GetColorRGB(rgb,i);
            for (int j=0;j<tmp_group.size();j++)
            {
                image1Dc_spine[tmp_group[j]->pos+size_page]=255;
                label_color[tmp_group.at(j)->pos] = rgb[0];
                label_color[tmp_group.at(j)->pos+size_page]=rgb[1];
                label_color[tmp_group.at(j)->pos+2*size_page]=rgb[2];
            }
        }
        else
        {
            LList_in[i].on=false;
            label_group[i].clear();
        }
    }

//    write_spine_profile("auto_proofread_spine_profile.csv");  //for using svm
//    //write_svm_file("training.txt",keep_spine);

    qDebug()<<"before image set"<<sz_img[3];
    //visualize
    Image4DSimple image4d;
    unsigned char *input_image=new unsigned char [size_page*3];
    memcpy(input_image,image1Dc_spine,size_page*3);
    image4d.setData(input_image,sz_img[0],sz_img[1],sz_img[2],3,V3D_UINT8);
    QString final_name=QString(MAINWINNAME) +"_image";
    callback->setImage(curwin,&image4d);
    callback->setImageName(curwin,final_name);
    callback->setLandmark(curwin,LList_in);
    callback->updateImageWindow(curwin);
    callback->open3DWindow(curwin);
    callback->pushObjectIn3DWindow(curwin);

    Image4DSimple image_label;
    image_label.setData(label_color,sz_img[0],sz_img[1],sz_img[2],3,V3D_UINT8);
    QString name=QString(MAINWINNAME)+"_label";
    v3dhandle new_win2=callback->newImageWindow(name);
    callback->setImage(new_win2,&image_label);
    callback->setLandmark(new_win2,LList_in);
    callback->updateImageWindow(new_win2);
    callback->open3DWindow(new_win2);
    callback->pushObjectIn3DWindow(new_win2);
    return count;
}

bool manual_proofread_dialog::finish_proof_dialog()
{
    QMessageBox mybox;
    mybox.setText("Have you finished proofreading?");
    QPushButton *save_button = mybox.addButton(tr("Finish and save"),QMessageBox::ActionRole);
    QPushButton *cancel_button=mybox.addButton(QMessageBox::Cancel);
    QPushButton *discard_button=mybox.addButton(QMessageBox::Discard);
    mybox.setDefaultButton(save_button);
    mybox.exec();

     if (mybox.clickedButton() == save_button) {
         int final_landmarks_num;
         final_landmarks_num = finish_and_save(SPINE_VIEW);
         QMessageBox mynewbox;
         QString info="After proofreading "+ QString::number(final_landmarks_num)+"/"
                + QString::number(LList_in.size()) +" spines were found\n";
         info+="The spine csv profile is saved at "+ baseDir;
         mynewbox.information(0,"spine detector",info,QMessageBox::Ok);
         return true;

     } else if (mybox.clickedButton() == cancel_button) {
         return false;
     }
     else if (mybox.clickedButton()== discard_button) {
         //need to close all image windows //check 3D window
         v3dhandleList list_triwin = callback->getImageWindowList();
         for(V3DLONG i=0; i<list_triwin.size(); i++){
             if(callback->getImageName(list_triwin.at(i)).contains(SPINE_VIEW))
             {
                 callback->close3DWindow(list_triwin[i]);
             }
         }
         mydialog->close();
         return false;
     }
}

bool manual_proofread_dialog::save_current_project()
{
    int seg_id,marker_id;
    if (view_code==1) //view by spine
    {
        seg_id=-1;
        marker_id=markers->currentIndex();
    }
    else if (view_code==2) //view by segment
    {
        seg_id=segments->currentIndex();
        marker_id=list_markers->currentRow();
        qDebug()<<"in save curretn project:"<<marker_id<<":"<<seg_id;
    }
    QString output_label_name="auto_manual_label.v3draw";
    QString output_marker_name="auto_manual.marker";
    QString output_csv_name="auto_manual.csv";
    if (!save_project_results(callback,sz_img,label_group,baseDir,input_swc_name,
        input_image_name,eswc_flag,neuron,LList_in,auto_para.channel,auto_para.bg_thr,auto_para.max_dis,
                           seg_id,marker_id,output_label_name,output_marker_name,output_csv_name))

    {
        QMessageBox::information(0,MAINWINNAME,"Current results saving error!",
                                 QMessageBox::Ok);
        return false;
    }
    else
    {
        QMessageBox::information(0,MAINWINNAME,"Current results successfully saved at"+
                       baseDir,QMessageBox::Ok);
        return true;
    }
}

void manual_proofread_dialog::open_trunc_triview(QString trunc_win_name)
{
    qDebug()<<"in open trunc triview";
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

void manual_proofread_dialog::open_main_triview(QString main_win_name)  //image1dc_spine
{
    qDebug()<<"open main_triview window in manual proofread dialog";
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
        memcpy(image_input,image1Dc_spine,sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3]);
        Image4DSimple image_main;
        image_main.setData(image_input,sz_img[0],sz_img[1],sz_img[2],sz_img[3],V3D_UINT8);
        callback->setImage(main_win,&image_main);
        callback->updateImageWindow(main_win);
    }
}


void manual_proofread_dialog::create_proofread_panel_by_segment(bool exist_proj)
{
    qDebug()<<"in create seg"<<segment_neuronswc.size();
    mydialog=new QDialog();
    mydialog->setWindowTitle("SpineDetector_proofreading_by_segment");
    mydialog->setFixedWidth(500);
    QGridLayout *layout2=new QGridLayout;

    segments=new QComboBox;
    if (!exist_proj) //if this project exist
    {
        for (int i=0;i<segment_neuronswc.size();i++)
            segments->addItem(QString("Segment ")+QString::number(i+1));
    }

    else  //interface with automatic detection
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

    //QPushButton *reset_seg=new QPushButton(tr("Reset segment"));
    QPushButton *finish_seg=new QPushButton(tr("Finish segment"));
    QPushButton *skip_seg=new QPushButton(tr("Next segment"));
    layout2->addWidget(finish_seg,1,0,1,2);
    layout2->addWidget(skip_seg,1,2,1,2);
    //layout2->addWidget(reset_seg,1,4,1,2);

    QFrame *line_1 = new QFrame();
    line_1->setFrameShape(QFrame::HLine);
    line_1->setFrameShadow(QFrame::Sunken);
    layout2->addWidget(line_1,2,0,1,6);

    QLabel *spine_groups=new QLabel(tr("Spine groups:"));
    layout2->addWidget(spine_groups,3,0,1,6);
    QLabel *multiple=new QLabel(tr("Press ctrl/shift to select multiple markers"));
    layout2->addWidget(multiple,4,0,1,6);

    list_markers=new QListWidget();
    layout2->addWidget(list_markers,5,0,8,4);
    list_markers->setSelectionMode(QAbstractItemView::ExtendedSelection);
    list_markers->setFocusPolicy(Qt::NoFocus);

    QPushButton *accept=new QPushButton(tr("Accept"));
    QPushButton *reject=new QPushButton(tr("Reject"));

    layout2->addWidget(accept,5,4,1,2);
    layout2->addWidget(reject,6,4,1,2);

    QPushButton *dilate = new QPushButton(tr("Dilate"));
    QPushButton *erode =new QPushButton (tr("Erode"));
    QPushButton *reset = new QPushButton (tr("Reset"));
    layout2->addWidget(dilate,7,4,1,2);
    layout2->addWidget(erode,8,4,1,2);
    layout2->addWidget(reset,9,4,1,2);

    QLabel *infobox=new QLabel(tr("Info:"));
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

    connect(button_save,SIGNAL(clicked()),this,SLOT(save_current_project()));
    connect(button_finish,SIGNAL(clicked()),this,SLOT(finish_proof_dialog_seg_view()));
    connect(segments,SIGNAL(currentIndexChanged(int)),this,SLOT(segment_change()));
    connect(accept,SIGNAL(clicked()),this,SLOT(accept_marker_for_seg_view()));
    connect(reject,SIGNAL(clicked()),this,SLOT(reject_marker_for_seg_view()));
    connect(dilate,SIGNAL(clicked()),this,SLOT(dilate_for_seg_view()));
    connect(erode,SIGNAL(clicked()),this,SLOT(erode_for_seg_view()));
    connect(reset,SIGNAL(clicked()),this,SLOT(reset_marker_clicked_for_seg_view()));
    connect(skip_seg,SIGNAL(clicked()),this,SLOT(skip_segment_clicked()));
    connect(finish_seg,SIGNAL(clicked()),this,SLOT(finish_seg_clicked()));
    //connect(reset_seg,SIGNAL(clicked()),this,SLOT(reset_segment_clicked()));
    segment_change();
    mydialog->show();
}


void manual_proofread_dialog::segment_change()
{
    if (seg_edit_flag)
    {
        if (!save_seg_edit_for_seg_view())
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
    qDebug()<<"LList seg size;"<<LList_adj.size();
    list_markers->clear();
    disconnect(list_markers,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(marker_in_one_seg()));
    for (int i=0;i<LList_adj.size();i++)
    {
        QString tmp_comment=QString::fromStdString(LList_adj[i].comments);
        if (tmp_comment.contains("1"))
            list_markers->addItem("marker "+ QString::number(i+1)+ " accepted");
        else if (tmp_comment.contains("2"))
            list_markers->addItem("marker "+QString::number(i+1)+ " rejected");
        else
            list_markers->addItem("marker "+ QString::number(i+1));
    }
    list_markers->setCurrentItem(list_markers->item(0));
    connect(list_markers,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(marker_in_one_seg()));

    //step 5: get the image ready
    Image4DSimple image4d;
    unsigned char * image_input=new unsigned char [sz[0]*sz[1]*sz[2]*sz[3]];
    memcpy(image_input,image_trun,sz[0]*sz[1]*sz[2]*sz[3]);
    qDebug()<<"sz_seg size:"<<sz[0]<<":"<<sz[1]<<":"<<sz[2];

    image4d.setData(image_input,sz[0],sz[1],sz[2],sz[3],V3D_UINT8);
    qDebug()<<"xyz_min:"<<x_start;
    callback->setImage(curwin,&image4d);
    callback->setLandmark(curwin,LList_adj);
    callback->updateImageWindow(curwin);
    callback->close3DWindow(curwin);
    callback->open3DWindow(curwin);
    callback->pushObjectIn3DWindow(curwin);
    prev_seg=seg_id;
}

void manual_proofread_dialog::set_trunc_image_marker(vector<int> one_seg,int seg_id)
{
    qDebug()<<"in set_trunc_image_marker";
    vector<V3DLONG> coord(6,0);
    coord=image_seg_plan(one_seg,checked_neuron);

    x_start=coord[0];
    y_start=coord[1];
    z_start=coord[2];
    x_end=coord[3];
    y_end=coord[4];
    z_end=coord[5];

    qDebug()<<"xyz_min:"<<x_start<<":"<<y_start<<":"<<z_start;
    qDebug()<<"xyz max:"<<x_start<<":"<<y_start<<":"<<z_start;

    sz[0]=x_end-x_start+1;
    sz[1]=y_end-y_start+1;
    sz[2]=z_end-z_start+1;
    sz[3]=3;
    qDebug()<<"sz:"<<sz[0]<<":"<<sz[1]<<":"<<sz[2];
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
                image_trun[pos1]=image1Dc_spine[pos];
            }
        }
    }

    LList_adj.clear();
    //find all the spines in this set.
    //also need to check if the whole spine is displayed
    for (int j=0;j<LList_in.size();j++)
    {

        if (LList_in[j].x-1<=x_start|| LList_in[j].x-1>=x_end || LList_in[j].y-1<=y_start
            || LList_in[j].y-1>=y_end || LList_in[j].z-1<=z_start ||LList_in[j].z-1>=z_end )
            continue;

        GOV tmp_spine= label_group [j];
        if (LList_in.at(j).category!=seg_id)  //need to check if the spine is complete
        {
            bool incomplete_flag=false;
            for (int k=0;k<tmp_spine.size();k++)
            {
                if (tmp_spine[k]->x<x_start || tmp_spine[k]->x > x_end || tmp_spine[k]->y <y_start
                        || tmp_spine[k]->y>y_end || tmp_spine[k]->z<z_start || tmp_spine[k]->z>z_end)
                {
                    incomplete_flag=true;
                    break;
                }
            }
            if (incomplete_flag) //if incomplete, not to include the spine
                continue;
            if (LList_in.at(j).category==0)
                LList_in[j].category=seg_id;
        }


        for (int k=0;k<tmp_spine.size();k++)
        {
            V3DLONG x=tmp_spine[k]->x-x_start;
            V3DLONG y=tmp_spine[k]->y-y_start;
            V3DLONG z=tmp_spine[k]->z-z_start;
            V3DLONG pos=xyz2pos(x,y,z,sz[0],sz[0]*sz[1]);
            image_trun[pos+sz[0]*sz[1]*sz[2]]=255;
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

    qDebug()<<"set visualize window."<<LList_adj.size()<<"markers have been found";
}

vector<V3DLONG> manual_proofread_dialog::image_seg_plan(vector<int> seg, NeuronTree *neuron_tmp)
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

void manual_proofread_dialog::marker_in_one_seg()
{
    qDebug()<<"in markerin one seg";
    edit_seg->clear();
    qDebug()<<"edit_flag:"<<edit_flag;
    int mid=list_markers->currentRow();
    if (edit_flag)
    {
       int ret;
       ret=save_edit_for_seg_view();
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
    QString tmp_name=QString::fromStdString(LList_adj.at(mid).name);
    int idx=tmp_name.toInt()-1;//idx in LList_in starting from 1
    QString status=QString::fromStdString(LList_in[idx].comments);
    qDebug()<<"mid:"<<mid<<"idx"<<idx<<"comments:"<<status;
    if (status.contains("0"))
    {
        LList_adj[mid].color.r=LList_adj[mid].color.b=255;
        LList_adj[mid].color.g=70;
    }

    qDebug()<<"before setting up image";
    Image4DSimple image4d;
    unsigned char * image_input=new unsigned char [sz[0]*sz[1]*sz[2]*sz[3]];
    memcpy(image_input,image_trun,sz[0]*sz[1]*sz[2]*sz[3]);

    //highlight the selected group
    GOV tmp_spine=label_group[idx];
    qDebug()<<"number of voxels in this spine:"<<tmp_spine.size();
    qDebug()<<"xyz_min:"<<x_start<<":"<<y_start<<":"<<z_start;
    for (int k=0;k<tmp_spine.size();k++)
    {
        V3DLONG x=tmp_spine[k]->x-x_start;
        V3DLONG y=tmp_spine[k]->y-y_start;
        V3DLONG z=tmp_spine[k]->z-z_start;
        V3DLONG pos=xyz2pos(x,y,z,sz[0],sz[0]*sz[1]);
        image_input[pos+2*sz[0]*sz[1]*sz[2]]=255;
    }
    qDebug()<<"before image4d setdata"<<sz[0]<<":"<<sz[1];
    image4d.setData(image_input,sz[0],sz[1],sz[2],sz[3],V3D_UINT8);
    callback->setImage(curwin,&image4d);
    callback->setLandmark(curwin,LList_adj);
    callback->updateImageWindow(curwin);
    callback->close3DWindow(curwin);
    callback->open3DWindow(curwin);
    callback->pushObjectIn3DWindow(curwin);
    callback->setLandmark(main_win,LList_in);

    //Step 6: reset marker color back to original color
//    LList_in[idx].color.r=LList_in[idx].color.b=LList_in[idx].color.g=0;
    LList_adj[mid].color.r=LList_adj[mid].color.b=LList_adj[mid].color.g=0;
    //qDebug()<<"~~~~marker roi finished";
    prev_idx=mid;
}

void manual_proofread_dialog::reset_segment()
{
    for (int i=0;i<LList_adj.size();i++)
    {
        QString tmp_name=QString::fromStdString(LList_adj.at(i).name);
        int idx=tmp_name.toInt()-1;//idx in LList_in starting from 1
        LList_in[idx].comments=QString("0").toStdString().c_str();
        LList_in[idx].color.r=LList_in[idx].color.g=LList_in[idx].color.b=LList_in[idx].category=0;
        list_markers->item(i)->setText("marker " + QString::number(i+1));
        GOV tmp_group=label_group[idx];
        for (int j=0;j<tmp_group.size();j++)
        {
            tmp_group[j]->intensity_label=-1;
        }
        tmp_group.clear();
        tmp_group=label_group_copy[idx];
        int label_id=idx+1;
        for (int j=0;j<tmp_group.size();j++)
        {
            tmp_group[j]->intensity_label=label_id;
        }
        label_group[idx].clear();
        label_group[idx]=tmp_group;
    }
    seg_edit_flag=false;
    qDebug()<<"segment reset";
}

void manual_proofread_dialog::reset_segment_clicked()
{
    reset_segment();
    seg_edit_flag=false;
    int seg_idx=segments->currentIndex();
    segments->setItemText(seg_idx,"Segment "+ QString::number(seg_idx+1));
    segment_change();
}

void manual_proofread_dialog::skip_segment_clicked()
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


void manual_proofread_dialog::finish_seg_clicked()
{
    for (int i=0;i<LList_adj.size();i++)
    {
        QString tmp_comment= QString::fromStdString(LList_adj[i].comments);
        QString tmp_name=QString::fromStdString(LList_adj[i].name);
        int idx=tmp_name.toInt()-1;
        if (tmp_comment.contains("0")) //tmp_comment.contains("1")||tmp_comment.contains("0"))
        {
          LList_in[idx].comments=QString::number(1).toStdString();
          LList_in[idx].color.r=LList_in[idx].color.b=0;
          LList_in[idx].color.g=255;
          LList_adj[i].color.r=LList_adj[i].color.b=0;
          LList_adj[i].color.g=255;
          list_markers->item(i)->setText("marker "+ QString::number(i+1)+ " accepted");

         }
    }
    int seg_idx=segments->currentIndex();
    segments->setItemText(seg_idx,"Segment "+ QString::number(seg_idx+1)+" finished");
    seg_edit_flag=false;
    callback->setLandmark(curwin,LList_adj);
    callback->pushObjectIn3DWindow(curwin);
    return;
}

void manual_proofread_dialog::reject_marker_for_seg_view()
{
    open_main_triview(MAINWINNAME);
    open_trunc_triview(SEGMENT_VIEW);
    qDebug()<<"delete marker";
    //update landmarks color and status
    QList<QListWidgetItem*> sel_list=list_markers->selectedItems();
    qDebug()<<"this many selected:"<<sel_list.size();
    edit_seg->clear();
    for (int i=0;i<sel_list.size();i++)
    {
        int mid= list_markers->row(sel_list[i]);

        LList_adj[mid].color.g=LList_adj[mid].color.b=0;
        LList_adj[mid].color.r=255;

        QString tmp_name=QString::fromStdString(LList_adj.at(mid).name);
        int idx=tmp_name.toInt()-1;//idx in LList_in starting from 1

        LList_in[idx].color.g=LList_in[idx].color.b=0;
        LList_in[idx].color.r=255;
        LList_in[idx].comments=QString::number(2).toStdString(); //rejected marker
        LList_adj[mid].comments=QString::number(2).toStdString();

        sel_list[i]->setText("marker " + QString::number(mid+1) + " rejected");
        edit_seg->appendPlainText("marker: " + QString::number(mid+1) + " rejected");
    }

    edit_flag=false;
    seg_edit_flag=true;
    callback->setLandmark(curwin,LList_adj);
    callback->pushObjectIn3DWindow(curwin);

}

void manual_proofread_dialog::accept_marker_for_seg_view()
{
    open_main_triview(MAINWINNAME);
    open_trunc_triview(SEGMENT_VIEW);
    qDebug()<<"accept marker";
    //update landmarks color and status
    QList<QListWidgetItem*> sel_list=list_markers->selectedItems();
    edit_seg->clear();
    for (int i=0;i<sel_list.size();i++)
    {
        int mid= list_markers->row(sel_list[i]);

        LList_adj[mid].color.r=LList_adj[mid].color.b=0;
        LList_adj[mid].color.g=255;

        QString tmp_name=QString::fromStdString(LList_adj.at(mid).name);
        int idx=tmp_name.toInt()-1;

        LList_in[idx].color.r=LList_in[idx].color.b=0;
        LList_in[idx].color.g=255;
        LList_in[idx].comments=QString::number(1).toStdString(); //accept marker
        LList_adj[mid].comments=QString::number(1).toStdString();

        sel_list[i]->setText("marker " + QString::number(mid+1) + " accepted");
        edit_seg->appendPlainText("marker: " + QString::number(mid+1) + " accepted");
    }

    edit_flag=false;
    seg_edit_flag=true;
    callback->setLandmark(curwin,LList_adj);
    callback->pushObjectIn3DWindow(curwin);
}


void manual_proofread_dialog::dilate_for_seg_view()
{
    open_main_triview(MAINWINNAME);
    open_trunc_triview(SEGMENT_VIEW);
    edit_seg->clear();
    int bg_thr=auto_para.bg_thr;
    qDebug()<<"in dilate now"<<sz[0]<<":"<<sz[1]<<":"<<sz[2]<<"bg_thr"<<bg_thr;
    V3DLONG size_page=sz[0]*sz[1]*sz[2];
    GOV seeds_next;
    seeds_next.clear();

    int mid=list_markers->currentRow();
    list_markers->item(mid)->setText("marker " + QString::number(mid+1));
    LList_adj[mid].color.r=LList_adj[mid].color.b=255;
    LList_adj[mid].color.g=70;
    QString tmp_name=QString::fromStdString(LList_adj[mid].name);
    int idx=tmp_name.toInt()-1;

    GOV tmp_group = label_group[idx];
    qDebug()<<"before tmp_group"<<tmp_group.size()<<"idx:"<<idx;
    for (int sid=0;sid<tmp_group.size();sid++)
    {
        VOI * single_voi = tmp_group[sid];
        int label_id=single_voi->intensity_label;
        for (int neid=0; neid<single_voi->neighbors_6.size();neid++)
        {
            VOI *nb=single_voi->neighbors_6[neid];
            if (image1Dc_spine[nb->pos]>bg_thr && nb->intensity_label<1&&nb->x>=x_start &&nb->y>=y_start
                    &&nb->z>=z_start && nb->x<x_end && nb->y<y_end && nb->z<z_end)
            {
                nb->intensity_label=label_id;
                seeds_next.push_back(nb);
                //qDebug()<<"newvoxel:"<<nb->pos;
            }
        }
    }

    if (seeds_next.size()<=0)
    {
        edit_seg->setPlainText("No more voxels available. Cannot dilate");
        return;
    }

    edit_seg->setPlainText(QString::number(seeds_next.size())+" voxels added in this round of dilation");
    tmp_group.insert(tmp_group.end(),seeds_next.begin(),seeds_next.end());

    label_group[idx].clear();
    label_group[idx]=tmp_group;

    for (int i=0;i<seeds_next.size();i++)
    {
        VOI * single_voi=seeds_next[i];
        V3DLONG pos_trun=xyz2pos(single_voi->x-x_start,single_voi->y-y_start,single_voi->z-z_start
                                 ,sz[0],sz[0]*sz[1]);
        image_trun[pos_trun+size_page]=255;
    }
    unsigned char *dilate_tmp =new unsigned char [size_page*sz[3]];
    memcpy(dilate_tmp,image_trun,size_page*2);
    memset(dilate_tmp+2*size_page,0,size_page);

    for (int i=0;i<tmp_group.size();i++)
    {
        V3DLONG pos_trun=xyz2pos(tmp_group[i]->x-x_start,tmp_group[i]->y-y_start,tmp_group[i]->z-z_start
                                 ,sz[0],sz[0]*sz[1]);
        dilate_tmp[pos_trun+2*size_page]=255;
    }
    qDebug()<<"tmp_group size:"<<tmp_group.size();
    Image4DSimple image4d_tmp;
    image4d_tmp.setData(dilate_tmp,sz[0],sz[1],sz[2],sz[3],V3D_UINT8);
    callback->setImage(curwin,&image4d_tmp);
    callback->updateImageWindow(curwin);
    callback->close3DWindow(curwin);
    callback->setLandmark(curwin,LList_adj);
    callback->open3DWindow(curwin);
    callback->pushObjectIn3DWindow(curwin);
    edit_flag=true;
    LList_adj[mid].color.r=LList_adj[mid].color.b=LList_adj[mid].color.g=0;;
}

void manual_proofread_dialog::erode_for_seg_view()
{
    open_main_triview(MAINWINNAME);
    open_trunc_triview(SEGMENT_VIEW);
    edit_seg->clear();
    qDebug()<<"in erode now"<<sz[0]<<":"<<sz[1]<<":"<<sz[2];
    V3DLONG size_page=sz[0]*sz[1]*sz[2];

    int mid=list_markers->currentRow();

    LList_adj[mid].color.r=LList_adj[mid].color.b=255;
    LList_adj[mid].color.g=70;
    list_markers->item(mid)->setText("marker " + QString::number(mid+1));

    QString tmp_name=QString::fromStdString(LList_adj[mid].name);
    int idx=tmp_name.toInt()-1;//idx in LList_in starting from 1
    GOV tmp_group = label_group[idx];

    if (tmp_group.size()==0)
    {
        edit_seg->setPlainText("No more voxels left. Cannot erode");
        return;
    }
    sort(tmp_group.begin(),tmp_group.end(),sortfunc_dst_ascend); //ascending
    //qDebug()<<"sort done:"<<"size:"<<tmp_group.size();

    int min_dis=tmp_group.front()->dst;
    int vid_begin, vid_end;
    vid_begin=vid_end=0;

    while(vid_end<tmp_group.size() && tmp_group[vid_end]->dst==min_dis){
        vid_end++;
        continue;
    }
    if (vid_begin==vid_end)
    {
       edit_seg->setPlainText("No more voxels left. Cannot erode");
       return;
    }

    edit_seg->setPlainText(QString::number(vid_end-vid_begin)+" voxels deleted in this round of erosion. "
                           +QString::number(tmp_group.size()-vid_end)+ " voxels left");
    //qDebug()<<"~~~erode~~~~decrease:"<<vid_end-vid_begin;

    for (int i=vid_begin;i<vid_end;i++)
    {
        V3DLONG trun_pos= xyz2pos(tmp_group[i]->x-x_start ,tmp_group[i]->y-y_start,
                                  tmp_group[i]->z-z_start,sz[0],sz[0]*sz[1]);
        image_trun[trun_pos+size_page]=0;
    }

    GOV update_group;
    for (int i=vid_end;i<tmp_group.size();i++)
    {
        tmp_group[i]->intensity_label=-1;
        update_group.push_back(tmp_group[i]);
    }

    label_group[idx].clear();
    label_group[idx]=update_group;

    unsigned char *erode_tmp =new unsigned char [size_page*sz[3]];
    memcpy(erode_tmp,image_trun,size_page*2);
    memset(erode_tmp+2*size_page,0,size_page);

    for (int i=0;i<update_group.size();i++)
    {
        V3DLONG trun_pos= xyz2pos(update_group[i]->x-x_start ,update_group[i]->y-y_start,
                                  update_group[i]->z-z_start,sz[0],sz[0]*sz[1]);
        erode_tmp[trun_pos+2*size_page]=255;
    }

    Image4DSimple image4d_tmp;
    image4d_tmp.setData(erode_tmp,sz[0],sz[1],sz[2],sz[3],V3D_UINT8);
    callback->setImage(curwin,&image4d_tmp);
    callback->updateImageWindow(curwin);
    callback->close3DWindow(curwin);
    callback->setLandmark(curwin,LList_adj);
    callback->open3DWindow(curwin);
    callback->pushObjectIn3DWindow(curwin);
    edit_flag=true;
    LList_adj[mid].color.g=LList_adj[mid].color.b=LList_adj[mid].color.r=0;
}

void manual_proofread_dialog::reset_marker_clicked_for_seg_view()
{
    int mid=list_markers->currentRow();
    reset_edit_for_seg_view(true,mid);
}

void manual_proofread_dialog::reset_edit_for_seg_view(bool visual_flag,int mid)
{
    qDebug()<<"in reset_edit for seg view"<<"trun image size:"<<sz[1]<<":"<<sz[3]<<"visual flag:"<<visual_flag;
    qDebug()<<"xyz_min:"<<x_start<<":"<<y_start<<":"<<z_start;
    edit_seg->clear();

    if (visual_flag)
    {
        LList_adj[mid].color.r=LList_adj[mid].color.b=255;
        LList_adj[mid].color.g=0;
        LList_adj[mid].comments=QString("0").toStdString().c_str();
        edit_seg->setPlainText("marker: "+ QString::number(mid+1)+ " reset");
    }

    QString tmp_name=QString::fromStdString(LList_adj.at(mid).name);
    int idx=tmp_name.toInt()-1;//idx in LList_in starting from 1
    LList_in[idx].comments=QString("0").toStdString().c_str();
    LList_in[idx].color.r=LList_in[idx].color.g=LList_in[idx].color.b=0;
    list_markers->item(mid)->setText("marker " + QString::number(mid+1));

    V3DLONG size_page=sz[0]*sz[1]*sz[2];
    GOV tmp_group=label_group[idx];
    //update image_seg
    for (int i=0;i<tmp_group.size();i++)
    {
        V3DLONG pos_trun=xyz2pos(tmp_group[i]->x-x_start,tmp_group[i]->y-y_start,tmp_group[i]->z-z_start
                                 ,sz[0],sz[0]*sz[1]);
        image_trun[pos_trun+size_page]=0;
        tmp_group[i]->intensity_label=-1;
    }

    tmp_group.clear();
    tmp_group=label_group_copy[idx];
    int label_id=idx+1;

    for (int i=0;i<tmp_group.size();i++)
    {
        V3DLONG pos_trun=xyz2pos(tmp_group[i]->x-x_start,tmp_group[i]->y-y_start,tmp_group[i]->z-z_start
                                 ,sz[0],sz[0]*sz[1]);
        image_trun[pos_trun+size_page]=255;
        tmp_group[i]->intensity_label=label_id;
    }

    label_group[idx].clear();
    label_group[idx]=tmp_group;

    unsigned char *reset_tmp =new unsigned char [size_page*sz[3]];
    memcpy(reset_tmp,image_trun,size_page*2);
    memset(reset_tmp+2*size_page,0,size_page);
    for (int i=0;i<tmp_group.size();i++)
    {
        V3DLONG pos_trun=xyz2pos(tmp_group[i]->x-x_start,tmp_group[i]->y-y_start,tmp_group[i]->z-z_start
                                 ,sz[0],sz[0]*sz[1]);
        reset_tmp[pos_trun+2*size_page]=255;
    }

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
    LList_adj[mid].color.r=LList_adj[mid].color.b=LList_adj[mid].color.g=0;
}

int manual_proofread_dialog::save_edit_for_seg_view()
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
         accept_marker_for_seg_view();
         return 1;
     } else if (mybox.clickedButton() == cancel_button) {
         return 4;
     }
     else if (mybox.clickedButton() == discard_button) {
         reset_edit_for_seg_view(false,prev_idx);
         edit_flag=false;

         return 3;
     } else if (mybox.clickedButton()==delete_button)
     {
         reject_marker_for_seg_view();
         return 2;
     }
}

bool manual_proofread_dialog::save_seg_edit_for_seg_view()
{
    QMessageBox mybox;
    mybox.setText("<b>Have you finished editing this segment? <\b>");
    QString info="-Yes: all spines not rejected will be accepted<br> -Discard: all changes to segment will be discarded<br>"
            "-Cancel: return to proofreading dialog";
    mybox.setInformativeText(info);
    mybox.addButton(QMessageBox::Yes);
    QPushButton *skip_button=mybox.addButton(tr("Discard"),QMessageBox::ActionRole);
    //QPushButton *reset_button=mybox.addButton(tr("reset"),QMessageBox::ActionRole);
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
            if(tmp_comment.contains("1")||tmp_comment.contains("0"))
            {
              LList_in[idx].comments=QString::number(1).toStdString();
              LList_in[idx].color.r=LList_in[idx].color.b=0;
              LList_in[idx].color.g=255;
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

bool manual_proofread_dialog::finish_proof_dialog_seg_view()
{
    QMessageBox mybox;
    mybox.setText("Have you finished proofreading?");
    mybox.setInformativeText("Only the accpeted markers will be kept");

    QPushButton *save_button = mybox.addButton(tr("Finish and save"),QMessageBox::ActionRole);
    QPushButton *cancel_button=mybox.addButton(QMessageBox::Cancel);
    QPushButton *discard_button=mybox.addButton(QMessageBox::Discard);

    mybox.setDefaultButton(save_button);
    mybox.exec();

     if (mybox.clickedButton() == save_button) {
         int final_landmarks_num;
         final_landmarks_num = finish_and_save(SEGMENT_VIEW);
         QMessageBox mynewbox;
         QString info="After proofreading "+ QString::number(final_landmarks_num)+"/"
                 + QString::number(LList_in.size()) +" spines were found\n";
         info+="The spine csv profile is saved at "+ baseDir;
         mynewbox.information(0,"spine detector",info,QMessageBox::Ok);
         return true;

     } else if (mybox.clickedButton() == cancel_button) {
         return false;
     }
     else if (mybox.clickedButton()== discard_button) {
         //need to close all image windows //check 3D window
         v3dhandleList list_triwin = callback->getImageWindowList();
         for(V3DLONG i=0; i<list_triwin.size(); i++){
             if(callback->getImageName(list_triwin.at(i)).contains(SEGMENT_VIEW))
             {
                 callback->close3DWindow(list_triwin[i]);
             }
         }
         mydialog->close();
         return false;
     }
}
