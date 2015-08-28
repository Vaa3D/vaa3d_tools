#include "file_io_dialog.h"
#include "common.h"
#include "manual_proofread_dialog.h"
#define WINTITLE "SpineDetector_Automatic"

file_io_dialog::file_io_dialog(V3DPluginCallback2 *cb, int code)
{
    callback=cb;
    image1Dc_in=0;
    if (code==1)
        create();
    else
        create_is();
    initDlg();
    check_button();
}


void file_io_dialog::create()
{
    QGridLayout *mygridLayout = new QGridLayout;
    QLabel* label_load = new QLabel(QObject::tr("Load Image:"));
    mygridLayout->addWidget(label_load,0,0,1,1);
    edit_load = new QLineEdit;
    edit_load->setText(""); edit_load->setReadOnly(true);
    mygridLayout->addWidget(edit_load,0,1,1,6);
    btn_load = new QPushButton("...");
    mygridLayout->addWidget(btn_load,0,7,1,1);

    QLabel* label_swc = new QLabel(QObject::tr("Load swc:"));
    mygridLayout->addWidget(label_swc,1,0,1,1);
    edit_swc = new QLineEdit;
    edit_swc->setText(""); edit_swc->setReadOnly(true);
    mygridLayout->addWidget(edit_swc,1,1,1,6);
    btn_swc = new QPushButton("...");
    mygridLayout->addWidget(btn_swc,1,7,1,1);

    QLabel* label_csv = new QLabel(QObject::tr("Output directory:"));
    mygridLayout->addWidget(label_csv,2,0,1,1);
    edit_csv = new QLineEdit;
    edit_csv->setText(""); edit_csv->setReadOnly(true);
    mygridLayout->addWidget(edit_csv,2,1,1,6);
    btn_csv = new QPushButton("...");
    mygridLayout->addWidget(btn_csv,2,7,1,1);

//    QLabel *label_project=new QLabel(QObject::tr("Project name:(No space)"));
//    mygridLayout->addWidget(label_project,3,0,1,1);
//    edit_project=new QLineEdit(tr("project1"));
//    mygridLayout->addWidget(edit_project,3,1,1,7);

    QLabel *channel = new QLabel(tr("Which channel to use?"));
    channel_menu = new QComboBox;
    channel_menu->addItem("red");
    channel_menu->addItem("green");
    channel_menu->addItem("blue");
    mygridLayout->addWidget(channel,4,0,1,2);
    mygridLayout->addWidget(channel_menu,4,3,1,5);

    //para setting
    QFrame *line_1 = new QFrame();
    line_1->setFrameShape(QFrame::HLine);
    line_1->setFrameShadow(QFrame::Sunken);
    mygridLayout->addWidget(line_1,5,0,1,8);

    QLabel *bg_thr = new QLabel(tr("Background threshold:"));
    mygridLayout->addWidget(bg_thr,6,0,1,6);
    QLabel *max_pixel=new QLabel (tr("Max spine volume:"));
    mygridLayout->addWidget(max_pixel,7,0,1,6);
    QLabel *min_pixel=new QLabel (tr("Min spine volume:"));
    mygridLayout->addWidget(min_pixel,8,0,1,6);
    QLabel *max_dis=new QLabel(tr("Max spine distance to surface:"));
    mygridLayout->addWidget(max_dis,9,0,1,6);
    QLabel *width_thr=new QLabel(tr("Max spine width:"));
    mygridLayout->addWidget(width_thr,10,0,1,6);

    spin_max_dis=new QSpinBox;
    spin_max_dis->setRange(5,80);
    spin_max_dis->setValue(40);
    spin_min_pixel=new QSpinBox;
    spin_min_pixel->setRange(10,100);
    spin_min_pixel->setValue(30);
    spin_bg_thr=new QSpinBox;
    spin_bg_thr->setRange(1,255);
    spin_bg_thr->setValue(90);
    spin_max_pixel=new QSpinBox;
    spin_max_pixel->setRange(2000,8000);
    spin_max_pixel->setValue(7000);
    spin_width_thr=new QSpinBox;
    spin_width_thr->setRange(10,100);
    spin_width_thr->setValue(35);
    mygridLayout->addWidget(spin_bg_thr,6,6,1,2);
    mygridLayout->addWidget(spin_max_pixel,7,6,1,2);
    mygridLayout->addWidget(spin_min_pixel,8,6,1,2);
    mygridLayout->addWidget(spin_max_dis,9,6,1,2);
    mygridLayout->addWidget(spin_width_thr,10,6,1,2);

    QFrame *line_2 = new QFrame();
    line_2->setFrameShape(QFrame::HLine);
    line_2->setFrameShadow(QFrame::Sunken);
    mygridLayout->addWidget(line_2,15,0,1,8);
    btn_run    = new QPushButton("Run");
    QPushButton *cancel = new QPushButton("Cancel");
    mygridLayout->addWidget(btn_run,16,2,1,2);
    mygridLayout->addWidget(cancel,16,5,1,2);

    //operation zone
    QFrame *line_3 = new QFrame();
    line_3->setFrameShape(QFrame::HLine);
    line_3->setFrameShadow(QFrame::Sunken);
    mygridLayout->addWidget(line_3,17,0,1,8);
    QString info=">> Spine detector first perform automatic spines detections near dendrites,then allows"
            " users to do manual proofreading.<br>";
    info+=">> <b>Background threshold: </b> All voxels above the background threshold are foreground, others are background voxles (voxels not considered spines).<br>";
    info+=">> <b>Max spine volume:</b> Maximum number of voxels allowed for a spine. Groups with larger volumes are considered not spines.<br>";
    info+=">> <b>Min spine volume:</b> Minumum number of voxels allowed for a spine.<br>";
    info+=">> <b>Max spine distance:</b> Spine distance is defined as the distance from the surface of dendrite. Automatic spine search will only occur in the region"
              " defined by the max spine distance.<br>";
    info+=">> <b>Max spine width:</b> Maximum width allowed for each spine.<br>";
    info+=">> For further questions, please contact Yujie Li at yujie.jade@gmail.com)";

    QTextEdit *text_info = new QTextEdit;
    text_info->setText(info);
    text_info->setReadOnly(true);
    mygridLayout->addWidget(text_info,18,0,1,9);

    this->setLayout(mygridLayout);
    this->setWindowTitle(WINTITLE);
    this->show();
    connect(btn_run,SIGNAL(clicked()), this, SLOT(run()));
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(btn_load, SIGNAL(clicked()), this, SLOT(get_image_name()));
    connect(btn_swc,SIGNAL(clicked()),this,SLOT(get_swc_name()));
    connect(btn_csv,SIGNAL(clicked()),this,SLOT(csv_out()));
}

void file_io_dialog::run()
{
    if(!check_button()){
        v3d_msg("You have not provided valid input/output");
        return;
    }
    folder_output=edit_csv->text();
    this->accept();
    qDebug()<<"load and initialize";
    if (!load_image())
    {
        v3d_msg("Loading image error");
        return;
    }
    if (!load_swc())
    {
        v3d_msg("Loading swc error");
        return;
    }

    get_para();
    if(!auto_spine_detect_invoke())
    {
        v3d_msg("Errors with automatic detection");
        return;
    }
    int ret=maybe_proofread();
    if (ret==1 ||ret ==2)
    {
        manual_proofread_dialog *dialog=new manual_proofread_dialog(callback,false);
        dialog->run_interface_with_auto(ret,neuron,eswc_flag,LList_in,label_group,image1Dc_in,sz_img
         ,all_para.bgthr,all_para.max_dis,sel_channel,input_swc_name,input_image_name,folder_output);


    }
    else if (ret==4)
    {
        qDebug()<<"discard or error";
        return;
    }
}

bool file_io_dialog::auto_spine_detect_invoke()
{
    int progress_id=0;
    QProgressDialog progress("Auto spine detection starts....","Abort",
                                                 0,10,this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setValue(0);

    //qDebug()<<"~~~~Auto spine detection starts....";
    spine_fun spine_obj(callback,all_para,sel_channel);
    if (!spine_obj.pushImageData(image1Dc_in,sz_img))
        return false;
    spine_obj.pushSWCData(neuron);

    progress_id=progress_id+6;
    progress.setValue(progress_id); //6

    if(!spine_obj.init()){
        v3d_msg("No spine candidates were found. Please check image and swc file");
        return false;
    }
    progress.setValue(++progress_id); //7
    if(!spine_obj.reverse_dst_grow())
    {
        v3d_msg("No spines candidates were found; Please check image and swc file");
        return false;
    }
    progress.setValue(++progress_id);//8
    spine_obj.run_intensityGroup();
    progress.setValue(++progress_id);//9
    spine_obj.conn_comp_nb6();
    progress.setValue(++progress_id);//10
    LList_in = spine_obj.get_center_landmarks();
    label_group = spine_obj.get_group_label();
    //spine_obj.saveResult();

    progress.setValue(++progress_id);
    progress.close();
    qDebug()<<"auto spine_detect complete";
    return true;
}
void file_io_dialog::initDlg()
{
    QSettings settings("V3D plugin","spine_detector");

    if (settings.contains("fname_swc"))
        this->edit_swc->setText(settings.value("fname_swc").toString());
    if (settings.contains("fname_img"))
        this->edit_load->setText(settings.value("fname_img").toString());
    if (settings.contains("output_folder"))
        this->edit_csv->setText(settings.value("output_folder").toString());
}

bool file_io_dialog::get_image_name()
{
    QString fileOpenName=this->edit_load->text();
    input_image_name = QFileDialog::getOpenFileName(0, QObject::tr("Choose the input image "),
             fileOpenName,QObject::tr("Images (*.raw *.tif *.lsm *.v3dpbd *.v3draw);;All(*)"));
    if (input_image_name.isEmpty())
    {
        qDebug()<<"fname not valid";
        return false;
    }
    else
    {
        edit_load->setText(input_image_name);
        qDebug()<<"fname:"<<input_image_name;
        QSettings settings("V3D plugin","spine_detector");
        settings.setValue("fname_img",edit_load->text());
        check_button();
        return true;
    }
}

bool file_io_dialog::get_swc_name()
{
    QString fileOpenName=this->edit_swc->text();
    input_swc_name = QFileDialog::getOpenFileName(0, 0,fileOpenName,"Supported file (*.swc *.eswc)" ";;Neuron structure(*.swc *eswc)",0,0);

    if(input_swc_name.isEmpty())
    {
        v3d_msg("No swc file loaded.");
        return false;
    }
    else{
        edit_swc->setText(input_swc_name);
        qDebug()<<"input swc name:"<<input_swc_name;
        QSettings settings("V3D plugin","spine_detector");
        settings.setValue("fname_swc",edit_swc->text());
        check_button();
        return true;
    }
}

bool file_io_dialog::csv_out()  //need to create a dir for project
{
    QString fileOpenName=this->edit_csv->text();
    QString tmp_dir = QFileDialog::getExistingDirectory(0, QObject::tr("Select Directory to Save Results"),
            fileOpenName,QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

#if defined(Q_OS_MAC)
        //detect if there is a Qt redundant folder bug, if yes, then make a correction
        // This fix is done by PHC, 2015May14. This should work in most cases. However
        //if a user choose a strange tmp folder with the name "/abc/abc/abc" then this fix
        //will wrongly go to the parent folder "abc/abc".
        QDir tmp1(fileSaveDir);
        QString tmp2 = tmp1.dirName();
        if (fileSaveDir.endsWith(tmp2+'/'+tmp2))
        {
            fileSaveDir = fileSaveDir.left(fileSaveDir.size() - tmp2.size() - 1);
        }
#endif
        if(tmp_dir.isEmpty())
            return false;
    else{
        folder_output=tmp_dir;
        qDebug()<<"folder_output:"<<folder_output;
        edit_csv->setText(folder_output);
        QSettings settings("V3D plugin","spine_detector");
        settings.setValue("output_folder",edit_csv->text());
        check_button();
        return true;
    }
}

bool file_io_dialog::check_button()
{
    if (this->edit_load->text().isEmpty() || this->edit_swc->text().isEmpty()
            ||this->edit_csv->text().isEmpty())
    {
        btn_run->setEnabled(false);
        return false;
    }else{
        btn_run->setEnabled(true);
        return true;
    }
}

bool file_io_dialog::load_image()
{
    qDebug()<<"in load image:";
    input_image_name=edit_load->text();
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

bool file_io_dialog::load_swc()
{
    eswc_flag=true;
    input_swc_name=edit_swc->text();
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
//            qDebug()<<"I:"<<ii<<"seg_id:"<<neuron.listNeuron[ii].seg_id;//<<":"<<neuron.listNeuron[ii].fea_val.size();
//            qDebug()<<" level:"<<neuron.listNeuron[ii].level;
//            qDebug()<<"fea1:"<<neuron.listNeuron[ii].fea_val[0]<<"size:"<<neuron.listNeuron[ii].fea_val.size();
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

void file_io_dialog::get_para()
{
    //obtain all para
    all_para.bgthr=spin_bg_thr->value();
    all_para.max_dis=spin_max_dis->value();
    all_para.intensity_max_pixel=spin_max_pixel->value();
    all_para.min_pixel=spin_min_pixel->value();
    all_para.width_thr=spin_width_thr->value();
    sel_channel=channel_menu->currentIndex();
    all_para.dst_max_pixel=2000;
}

bool file_io_dialog::save_project()
{
    QString output_label_name="auto_label.v3draw";
    QString output_marker_name="auto.marker";
    QString output_csv_name="auto.csv";
    if (!save_project_results(callback,sz_img,label_group,folder_output,input_swc_name,input_image_name,eswc_flag,neuron,
                                 LList_in,sel_channel,all_para.bgthr,all_para.max_dis,0,0,output_label_name,
                              output_marker_name,output_csv_name))
    {
        return false;
    }
    return true;
}


int file_io_dialog::maybe_proofread()
{
    qDebug()<<"in maybe proofread dialog";
    QMessageBox mybox;
    int size=label_group.size();
    mybox.setWindowTitle(WINTITLE);
    mybox.setText("SpineDetector automatic detection completes");
    QString info="The automatic spine detector finds "+ QString::number(size)+" spines."+
            "Would you like to start proofreading?";
    mybox.setText(info);
    QPushButton *seg_view=mybox.addButton(tr("Proofread by segment"),QMessageBox::ActionRole);
    QPushButton *spine_view=mybox.addButton(tr("Proofread by spine"),QMessageBox::ActionRole);
    //QPushButton *save_do_later=mybox.addButton(tr("Save for later"),QMessageBox::ActionRole);
    QPushButton *exit_button=mybox.addButton(tr("Exit without saving"),QMessageBox::ActionRole);

    mybox.setDefaultButton(seg_view);
    mybox.exec();
    if (mybox.clickedButton() == seg_view) {
        return 1;  //invoke manual proofread....
    }else if (mybox.clickedButton()==spine_view)
    {
        return 2; //invoke manual proofread
    }
//    else if (mybox.clickedButton() == save_do_later) {
//        if (!save_project())
//        {
//            v3d_msg("Errors with saving the data");
//            return 4;
//        }
//        QString info="The spine project profile is saved at "+ edit_csv->text();
//        QMessageBox::information(0,"spine_detector",info,QMessageBox::Ok);

//        //prepare image
//        V3DLONG size_page=sz_img[0]*sz_img[1]*sz_img[2];
//        unsigned char *image_input = new unsigned char[size_page*3];
//        memset(image_input,0,size_page*3);
//        memcpy(image_input,image1Dc_in,size_page);
//        for(int i=0; i<label_group.size(); i++)
//        {
//            for (int j=0; j<label_group[i].size(); j++)
//                image_input[label_group[i][j]->pos+size_page] = 255;
//        }
//        Image4DSimple image4d;
//        image4d.setData(image_input,sz_img[0],sz_img[1],sz_img[2],3,V3D_UINT8);
//        v3dhandle new_win=callback->newImageWindow(WINTITLE);
//        callback->setImage(new_win,&image4d);
//        callback->setLandmark(new_win,LList_in);
//        callback->open3DWindow(new_win);
//        callback->pushObjectIn3DWindow(new_win);
//        return 3;
//    }
    else if (mybox.clickedButton()==exit_button)
    {
        return 4;
    }
}

void file_io_dialog::create_is()
{
    QGridLayout *mygridLayout = new QGridLayout;
    QLabel* label_load = new QLabel(QObject::tr("Load Image:"));
    mygridLayout->addWidget(label_load,0,0,1,1);
    edit_load = new QLineEdit;
    edit_load->setText(""); edit_load->setReadOnly(true);
    mygridLayout->addWidget(edit_load,0,1,1,6);
    btn_load = new QPushButton("...");
    mygridLayout->addWidget(btn_load,0,7,1,1);

    QLabel* label_swc = new QLabel(QObject::tr("Load swc:"));
    mygridLayout->addWidget(label_swc,1,0,1,1);
    edit_swc = new QLineEdit;
    edit_swc->setText(""); edit_swc->setReadOnly(true);
    mygridLayout->addWidget(edit_swc,1,1,1,6);
    btn_swc = new QPushButton("...");
    mygridLayout->addWidget(btn_swc,1,7,1,1);

    QLabel* label_csv = new QLabel(QObject::tr("Output directory:"));
    mygridLayout->addWidget(label_csv,2,0,1,1);
    edit_csv = new QLineEdit;
    edit_csv->setText(""); edit_csv->setReadOnly(true);
    mygridLayout->addWidget(edit_csv,2,1,1,6);
    btn_csv = new QPushButton("...");
    mygridLayout->addWidget(btn_csv,2,7,1,1);

    QLabel *channel_cell = new QLabel(tr("Cell channel:"));
    channel_menu = new QComboBox;
    channel_menu->addItem("red");
    channel_menu->addItem("green");
    channel_menu->addItem("blue");
    channel_menu->setCurrentIndex(0);
    mygridLayout->addWidget(channel_cell,3,0,1,2);
    mygridLayout->addWidget(channel_menu,3,3,1,5);

    QLabel *channel_is =new QLabel(tr("Inhibitory Synapses(IS) channel:"));
    channel_is_menu = new QComboBox;
    channel_is_menu->addItem("red");
    channel_is_menu->addItem("green");
    channel_is_menu->addItem("blue");
    channel_is_menu->setCurrentIndex(1);
    mygridLayout->addWidget(channel_is,4,0,1,2);
    mygridLayout->addWidget(channel_is_menu,4,3,1,5);

    //para setting
    QFrame *line_1 = new QFrame();
    line_1->setFrameShape(QFrame::HLine);
    line_1->setFrameShadow(QFrame::Sunken);
    mygridLayout->addWidget(line_1,5,0,1,8);

    QLabel *para_label=new QLabel(tr("Parameter setting:"));
    mygridLayout->addWidget(para_label,6,0,1,2);
    QLabel *cell_bgthr = new QLabel(tr("Cell channel background threshold:"));
    mygridLayout->addWidget(cell_bgthr,8,0,1,6);
    QLabel *is_bgthr=new QLabel (tr("IS channel background threshold:"));
    mygridLayout->addWidget(is_bgthr,9,0,1,6);
    QLabel *min_pixel=new QLabel (tr("Min IS volume:"));
    mygridLayout->addWidget(min_pixel,10,0,1,6);
    QLabel *max_dis=new QLabel(tr("Max search distance to surface:"));
    mygridLayout->addWidget(max_dis,11,0,1,6);

    spin_max_dis=new QSpinBox;
    spin_max_dis->setRange(5,80);
    spin_max_dis->setValue(40);
    spin_min_pixel=new QSpinBox;
    spin_min_pixel->setRange(1,30);
    spin_min_pixel->setValue(5);
    spin_bg_thr=new QSpinBox;  //cell channel
    spin_bg_thr->setRange(1,255);
    spin_bg_thr->setValue(90);
    spin_bg_thr2=new QSpinBox; //IS channel
    spin_bg_thr2->setRange(1,255);
    spin_bg_thr2->setValue(70);

    mygridLayout->addWidget(spin_bg_thr,8,6,1,2);
    mygridLayout->addWidget(spin_bg_thr2,9,6,1,2);
    mygridLayout->addWidget(spin_min_pixel,10,6,1,2);
    mygridLayout->addWidget(spin_max_dis,11,6,1,2);

    QFrame *line_2 = new QFrame();
    line_2->setFrameShape(QFrame::HLine);
    line_2->setFrameShadow(QFrame::Sunken);
    mygridLayout->addWidget(line_2,15,0,1,8);

    btn_run    = new QPushButton("Run");
    QPushButton *cancel = new QPushButton("Cancel");
    mygridLayout->addWidget(btn_run,16,1,1,2);
    mygridLayout->addWidget(cancel,16,5,1,2);

    this->setLayout(mygridLayout);
    this->setWindowTitle("Inhibitory synapses detection");
    this->show();
    connect(btn_run,SIGNAL(clicked()), this, SLOT(run_is()));
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(btn_load, SIGNAL(clicked()), this, SLOT(get_image_name()));
    connect(btn_swc,SIGNAL(clicked()),this,SLOT(get_swc_name()));
    connect(btn_csv,SIGNAL(clicked()),this,SLOT(csv_out()));
}

void file_io_dialog::run_is()
{
    if(!check_button()){
        v3d_msg("You have not provided valid input/output");
        return;
    }
    folder_output=edit_csv->text();
    this->accept();
    qDebug()<<"load and initialize";
    if (!load_image())
    {
        v3d_msg("Loading image error");
        return;
    }
    if (!load_swc())
    {
        v3d_msg("Loading swc error");
        return;
    }
    if (!is_detect_invoke())
    {
        v3d_msg("IS detection error");
        return;
    }
    qDebug()<<"run is finished";

}

bool file_io_dialog::is_detect_invoke()
{
    QStringList name_list;
    name_list<<input_image_name<<input_swc_name<<folder_output;
    is_analysis_fun* is_obj = new is_analysis_fun(callback,name_list);
    qDebug()<<"name list size:"<<name_list.size();
    if (!is_obj->pushImageData(image1Dc_in,sz_img))
        return false;
    is_obj->pushSWCData(neuron);
    is_obj->run();
    is_obj->show();
    return true;
}
