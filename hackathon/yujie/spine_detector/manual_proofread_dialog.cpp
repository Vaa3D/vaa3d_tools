#include "manual_proofread_dialog.h"

manual_proofread_dialog::manual_proofread_dialog(V3DPluginCallback2 *cb)
{
    callback=cb;
    edit_flag=false;
    seg_edit_flag=false;
    image1Dc_in=0;
    label=0;
    create();
    check_btn();
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

    QLabel *label_view = new QLabel(tr("Which proofread view?"));
    view_menu = new QComboBox;
    view_menu->addItem("View by spine");
    view_menu->addItem("View by segment");
    mygridLayout->addWidget(label_view,3,0,1,2);
    mygridLayout->addWidget(view_menu,3,3,1,5);

    btn_run = new QPushButton("Run");
    QPushButton *cancel = new QPushButton("Cancel");
    mygridLayout->addWidget(btn_run,16,2,1,2);
    mygridLayout->addWidget(cancel,16,5,1,2);

    this->setLayout(mygridLayout);
    this->setWindowTitle("SpineDetector_proofreading");
    this->show();

    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(btn_load, SIGNAL(clicked()), this, SLOT(get_proj_name()));
    connect(btn_run,SIGNAL(clicked()),this,SLOT(run()));
    //connect(this,SIGNAL(finished(int)),this,SLOT(dialoguefinish(int)));
}

void manual_proofread_dialog::run()
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

}

bool manual_proofread_dialog::prep_data()
{
   spine_fun spine_obj(callback,auto_para.channel,auto_para.bg_thr,auto_para.bg_thr,label);
   if (!spine_obj.pushImageData(image1Dc_in,sz_img))
       return false;
   spine_obj.pushSWCData(neuron);
   if(!spine_obj.init_for_manual_proof()){
       v3d_msg("No spine candidates were found. Please check image and swc file");
       return false;
   }
   //get ready label_group
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
//    QString baseDir = openFileNameLabel;
//    baseDir.chop(baseName.size());

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
        else
            continue;
    }
    qDebug()<<"image:"<<input_image_name<<endl;
    qDebug()<<"swc:"<<input_swc_name<<endl;
    qDebug()<<"csv:"<<input_csv_name<<endl;
    qDebug()<<"bg_thr:"<<auto_para.bg_thr<<"maxdis:"<<auto_para.max_dis<<"channel:"<<auto_para.channel;

    return (cnt==7) ? true : false;
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
        QMessageBox::information(0,"","More than 1 channel loaded."
                                 "The first channel will be applied for analysis.");
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
