#include "file_io_dialog.h"

file_io_dialog::file_io_dialog(V3DPluginCallback2 *cb)
{
    callback=cb;
    image1Dc_in=0;
    create();
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

    QLabel *channel = new QLabel(tr("Which channel to use?"));
    channel_menu = new QComboBox;
    channel_menu->addItem("red");
    channel_menu->addItem("green");
    channel_menu->addItem("blue");
    mygridLayout->addWidget(channel,3,0,1,2);
    mygridLayout->addWidget(channel_menu,3,3,1,5);

    //para setting
    QFrame *line_1 = new QFrame();
    line_1->setFrameShape(QFrame::HLine);
    line_1->setFrameShadow(QFrame::Sunken);
    mygridLayout->addWidget(line_1,4,0,1,8);

    QLabel *bg_thr = new QLabel(tr("Background threshold:"));
    mygridLayout->addWidget(bg_thr,5,0,1,6);
    QLabel *max_pixel=new QLabel (tr("Max spine volume:"));
    mygridLayout->addWidget(max_pixel,6,0,1,6);
    QLabel *min_pixel=new QLabel (tr("Min spine volume:"));
    mygridLayout->addWidget(min_pixel,7,0,1,6);
    QLabel *max_dis=new QLabel(tr("Max spine distance to surface:"));
    mygridLayout->addWidget(max_dis,8,0,1,6);
    QLabel *width_thr=new QLabel(tr("Max spine width:"));
    mygridLayout->addWidget(width_thr,9,0,1,6);

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
    mygridLayout->addWidget(spin_bg_thr,5,6,1,2);
    mygridLayout->addWidget(spin_max_pixel,6,6,1,2);
    mygridLayout->addWidget(spin_min_pixel,7,6,1,2);
    mygridLayout->addWidget(spin_max_dis,8,6,1,2);
    mygridLayout->addWidget(spin_width_thr,9,6,1,2);

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
    this->setWindowTitle("SpineDetector_AutomaticDetection");
    this->show();
//    if (view_code==1)
//        connect(btn_run,SIGNAL(clicked()), this, SLOT(run_view_by_seg()));
//    else if(view_code==0)
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
    if (!save_auto_detect_result())
    {
        v3d_msg("Errors with saving the data");
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
    //LList_in = spine_obj.get_center_landmarks();
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

bool file_io_dialog::save_auto_detect_result()
{
    qDebug()<<"saving profiles " <<"eswc flag:"<<eswc_flag;
    //save label image
    V3DLONG size_page=sz_img[0]*sz_img[1]*sz_img[2];
    V3DLONG label_sz[4];
    label_sz[0]=sz_img[0];
    label_sz[1]=sz_img[1];
    label_sz[2]=sz_img[2];
    label_sz[3]=2;
    unsigned short *tmp_label = new unsigned short[size_page*2];
    memset(tmp_label,0,size_page*sizeof(unsigned short));

//    image1Dc_spine = new unsigned char [size_page*3];
//    memset(image1Dc_spine,0,size_page*3);
//    memcpy(image1Dc_spine,image1Dc_in,size_page);

    for(int i=0; i<label_group.size(); i++)
    {
        GOV tmp = label_group[i];
        for (int j=0; j<tmp.size(); j++)
        {
            tmp_label[tmp.at(j)->pos]=tmp.at(j)->intensity_label;
            tmp_label[tmp.at(j)->pos+size_page]=tmp.at(j)->dst;
        }
    }
    //maybe not to save dst...too large memory cost
    QString tmp_name="auto_label.v3draw";
    QString fname_output = QDir(folder_output).filePath(tmp_name);
    if (!simple_saveimage_wrapper(*callback, fname_output.toStdString().c_str(), (unsigned char *)tmp_label, label_sz, 2))
        return false;

    //save csv file
    tmp_name="auto.csv";
    QString csv_file=QDir(folder_output).filePath(tmp_name);
    FILE *fp2=fopen(csv_file.toAscii(),"wt");
    if (eswc_flag)
        fprintf(fp2,"##yes/no_spine,auto_detect_id,volume,max_dis,min_dis,center_dis,center_x,center_y,center_z,skel_node,skel_type,skel_node_seg,skel_node_branch,dis_to_root,tree_id\n");
    else
        fprintf(fp2,"##yes/no_spine,auto_detect_id,volume,max_dis,min_dis,center_dis,center_x,center_y,center_z,skel_node\n");

    for (int i=0;i<label_group.size();i++)
    {
        GOV tmp=label_group[i];
        if (tmp.size()<=0) continue;
        sort(tmp.begin(),tmp.end(),sortfunc_dst);
        int group_id=tmp.front()->intensity_label;
        int max_dis=tmp.front()->dst;
        int min_dis=tmp.back()->dst;
        int volume=tmp.size();

        V3DLONG sum_x,sum_y,sum_z,sum_dis;
        sum_x=sum_y=sum_z=sum_dis=0;

        for (int j=0;j<tmp.size();j++)
        {
            sum_x+=tmp[j]->x;
            sum_y+=tmp[j]->y;
            sum_z+=tmp[j]->z;
            sum_dis+=tmp[j]->dst;
        }
        int center_x=sum_x/tmp.size();
        int center_y=sum_y/tmp.size();
        int center_z=sum_z/tmp.size();
        int center_dis=sum_dis/tmp.size();

        //qDebug()<<"size:"<<tmp.size()<<" skel_id size:"<<skel_id_vector.size();

        int skel_id=calc_nearest_node(center_x,center_y,center_z);
        //qDebug()<<"skel_id_size:"<<skel_id_size<<" skel_id:"<<skel_id;
        if (eswc_flag)
        {
            fprintf(fp2,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%.2f,%d\n",0,group_id,volume,max_dis,
                    min_dis,center_dis,(int)center_x,(int)center_y,(int)center_z,skel_id,
                    neuron.listNeuron.at(skel_id).type,(int)neuron.listNeuron.at(skel_id).seg_id,
                    (int)neuron.listNeuron.at(skel_id).level, neuron.listNeuron.at(skel_id).fea_val[1],
                    (int)neuron.listNeuron.at(skel_id).fea_val[0]);
        }
        else
            fprintf(fp2,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",0,group_id,volume,max_dis,min_dis
                    ,center_dis,(int)center_x,(int)center_y,(int)center_z,skel_id);
    }
    fclose(fp2);

    //get the project profile txt
    tmp_name="project.txt";
    fname_output=QDir(folder_output).filePath(tmp_name);
    QFile qfile(fname_output);
    if (!qfile.open(QIODevice::WriteOnly))
    {
        v3d_msg("Cannot open txt file for writing!");
        return false;
    }
    QTextStream out(&qfile);
    out<<"RAWIMG="<<input_image_name<<endl;
    out<<"SWCFILE="<<input_swc_name<<endl;
    out<<"MASKIMG="<<fname_output<<endl;
    out<<"CSV="<<csv_file<<endl;
    out<<"PARA_BG="<<all_para.bgthr<<endl;
    out<<"PARA_MAXDIS="<<all_para.max_dis<<endl;
    out<<"PARA_CHANNEL"<<sel_channel<<endl;
    qfile.close();
    qDebug()<<"file complete wrriting";
}

int file_io_dialog::calc_nearest_node(float center_x,float center_y,float center_z)
{
    float distance=1e6;
    int nearest_node_id=0;
    for (int i=0;i<neuron.listNeuron.size();i++)
    {
        float tmp_dis=(center_x-neuron.listNeuron[i].x)*(center_x-neuron.listNeuron[i].x)+
           (center_y-neuron.listNeuron[i].y)*(center_y-neuron.listNeuron[i].y)+
                (center_z-neuron.listNeuron[i].z)*(center_z-neuron.listNeuron[i].z);
        if (tmp_dis<distance)
        {
            distance=tmp_dis;
            nearest_node_id=i;
        }
    }
    return nearest_node_id;
}
