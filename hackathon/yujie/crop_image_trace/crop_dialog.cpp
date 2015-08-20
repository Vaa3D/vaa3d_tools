#include "crop_dialog.h"
#include "basic_surf_objs.h"

#ifndef ABS
#define ABS(x) ((x) > 0 ? (x) : (-(x)))
#endif

crop_dialog::crop_dialog(V3DPluginCallback2 *cb)
{
    callback=cb;
    image1Dc_in=0;
}

crop_dialog::~crop_dialog()
{
    if (image1Dc_in!=0)
    {
        delete[] image1Dc_in;
        image1Dc_in=0;
    }
}

void crop_dialog::getData()
{
    ImagePixelType pixeltype;
    if (!load_data(callback,image1Dc_in,pixeltype,sz_img,neuron,curwin))
        return;
    qDebug()<<"sz_img:"<<sz_img[0]<<":"<<sz_img[1]<<":"<<sz_img[2]<<sz_img[3];

    //create dialog to input para
    mydialog2=new QDialog();
    mydialog2->setWindowTitle("CropImageTrace");
    QGridLayout *layout= new QGridLayout();

    QLabel* label_csv = new QLabel(tr("Output directory:"));
    layout->addWidget(label_csv,0,0,1,1);
    edit_csv = new QLineEdit;
    edit_csv->setText("");
    edit_csv->setReadOnly(true);
    layout->addWidget(edit_csv,0,1,1,5);
    QPushButton *btn_csv = new QPushButton("...");
    layout->addWidget(btn_csv,0,6,1,1);

    QLabel *label1=new QLabel(tr("min"));
    label1->setAlignment(Qt::AlignHCenter);
    QLabel *label2=new QLabel(tr("max"));
    label2->setAlignment(Qt::AlignHCenter);
    QLabel *label3=new QLabel(tr("x:"));
    label3->setAlignment(Qt::AlignHCenter);
    QLabel *label4=new QLabel(tr("y:"));
    label4->setAlignment(Qt::AlignHCenter);
    QLabel *label5=new QLabel(tr("z:"));
    label5->setAlignment(Qt::AlignHCenter);

    layout->addWidget(label1,2,1,1,3);
    layout->addWidget(label2,2,4,1,3);
    layout->addWidget(label3,3,0,1,1);
    layout->addWidget(label4,4,0,1,1);
    layout->addWidget(label5,5,0,1,1);

    spin_x_min= new QSpinBox();
    spin_x_max= new QSpinBox();
    spin_y_min= new QSpinBox();
    spin_y_max= new QSpinBox();
    spin_z_min= new QSpinBox();
    spin_z_max= new QSpinBox();
    layout->addWidget(spin_x_min,3,1,1,3);
    layout->addWidget(spin_x_max,3,4,1,3);
    layout->addWidget(spin_y_min,4,1,1,3);
    layout->addWidget(spin_y_max,4,4,1,3);
    layout->addWidget(spin_z_min,5,1,1,3);
    layout->addWidget(spin_z_max,5,4,1,3);

    spin_x_min->setRange(1,(int)sz_img[0]);
    spin_x_min->setValue(1);
    spin_x_max->setRange(1,(int)sz_img[0]);
    spin_x_max->setValue((int)sz_img[0]);
    spin_y_min->setRange(1,(int)sz_img[1]);
    spin_y_min->setValue(1);
    spin_y_max->setRange(1,(int)sz_img[1]);
    spin_y_max->setValue((int)sz_img[1]);
    spin_z_min->setRange(1,(int)sz_img[2]);
    spin_z_min->setValue(1);
    spin_z_max->setRange(1,(int)sz_img[2]);
    spin_z_max->setValue((int)sz_img[2]);

    QLabel *color_channel=new QLabel(tr("channels:"));
    channel_menu=new QComboBox();
    channel_menu->addItem("Red");
    channel_menu->addItem("Green");
    channel_menu->addItem("Blue");
    channel_menu->addItem("All");
    layout->addWidget(color_channel,7,0,1,1);
    layout->addWidget(channel_menu,7,1,1,6);

    ok = new QPushButton(tr("Run and save"));
    QPushButton *cancel=new QPushButton(tr("Cancel"));
    layout->addWidget(ok,8,2,1,2);
    layout->addWidget(cancel,8,5,1,2);
    mydialog2->setLayout(layout);
    mydialog2->show();

    connect(cancel,SIGNAL(clicked()),this,SLOT(reject()));
    connect(ok,SIGNAL(clicked()),this,SLOT(crop()));
    connect(btn_csv,SIGNAL(clicked()),this,SLOT(csv_out()));

    initDlg();
    check_button();
}

bool crop_dialog::csv_out()
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
        QSettings settings("V3D plugin","CropImageTrace");
        settings.setValue("output_folder",edit_csv->text());
        check_button();
        return true;
    }
}

void crop_dialog::initDlg()
{
    QSettings settings("V3D plugin","CropImageTrace");
    if (settings.contains("output_folder"))
    {
        this->edit_csv->setText(settings.value("output_folder").toString());
        folder_output=edit_csv->text();
    }
}

bool crop_dialog::check_button()
{
    if (this->edit_csv->text().isEmpty())
    {
        ok->setEnabled(false);
        return false;
    }else{
        ok->setEnabled(true);
        return true;
    }
}

bool crop_dialog::check_channel()
{
    if (channel_menu->currentIndex()==1 && sz_img[3]<1)
    {
        v3d_msg("No such channel");
        return false;
    }
    else if (channel_menu->currentIndex()==2 && sz_img[3]<2)
    {
        v3d_msg("No such channel");
        return false;
    }
    return true;
}

void crop_dialog::crop()
{
    mydialog2->accept();
    if (!check_channel())
        return;
    V3DLONG crop_sz[4];
    int x_start,y_start,z_start,x_end,y_end,z_end;
    x_start=spin_x_min->value()-1;
    x_end=spin_x_max->value()-1;
    y_start=spin_y_min->value()-1;
    y_end=spin_y_max->value()-1;
    z_start=spin_z_min->value()-1;
    z_end=spin_z_max->value()-1;

    crop_sz[0]=ABS(x_end-x_start+1);
    crop_sz[1]=ABS(y_end-y_start+1);
    crop_sz[2]=ABS(z_end-z_start+1);
    V3DLONG size_page=crop_sz[0]*crop_sz[1]*crop_sz[2];
    int channel=channel_menu->currentIndex();
    if (channel==3)
        crop_sz[3]=sz_img[3];
    else
        crop_sz[3]=1;
    qDebug()<<"channel"<<channel;
    qDebug()<<"crop size:"<<crop_sz[0]<<":"<<crop_sz[1]<<":"<<crop_sz[2]<<crop_sz[3];

    unsigned char * cropped_image=new unsigned char[size_page*crop_sz[3]];
    memset(cropped_image,0,size_page*crop_sz[3]);

    //qDebug()<<"start:"<<x_start<<":"<<y_start<<":"<<z_start<<"end:"<<x_end<<":"<<y_end;
    for (V3DLONG dx=x_start;dx<=x_end;dx++){
        for (V3DLONG dy=y_start;dy<=y_end;dy++){
            for (V3DLONG dz=z_start;dz<=z_end;dz++){
                //qDebug()<<"dx,y,z:"<<dx<<":"<<dy<<":"<<dz;
                V3DLONG pos=xyz2pos(dx,dy,dz,sz_img[0],sz_img[0]*sz_img[1]);
                V3DLONG pos1=xyz2pos(dx-x_start,dy-y_start,dz-z_start,crop_sz[0],crop_sz[0]*crop_sz[1]);
                //qDebug()<<"pos"<<pos<<" pos1"<<pos1<<" crop_sz_max:"<<size_page*crop_sz[3]<<" sz max:"<<sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];
                if (channel==0)
                    cropped_image[pos1]=image1Dc_in[pos];
                else if (channel==1)
                {
                    //qDebug()<<"current index =1";
                    cropped_image[pos1]=image1Dc_in[pos+sz_img[0]*sz_img[1]*sz_img[2]];
                }
                else if (channel==2)
                    cropped_image[pos1]=image1Dc_in[pos+2*sz_img[0]*sz_img[1]*sz_img[2]];
                else if (channel==3)
                {
                    for (int i=0;i<crop_sz[3];i++)
                    {
                        cropped_image[pos1+i*crop_sz[0]*crop_sz[1]*crop_sz[2]]
                             =image1Dc_in[pos+i*sz_img[0]*sz_img[1]*sz_img[2]];
                    }
                }
             }
         }
     }

    bool eswc_flag=false;
    //shift the neurontree
    //check if it is swc or eswc
    for (int i=0;i<neuron.listNeuron.size();i++)
    {
        neuron.listNeuron[i].x=neuron.listNeuron[i].x-x_start;
        neuron.listNeuron[i].y=neuron.listNeuron[i].y-y_start;
        neuron.listNeuron[i].z=neuron.listNeuron[i].z-z_start;
        if (!eswc_flag)
        {
            if (neuron.listNeuron[i].level>0 || neuron.listNeuron[i].seg_id>0
             || neuron.listNeuron[i].fea_val.size()>0)
                eswc_flag=true;
        }
    }

    unsigned char *image_tmp= new unsigned char [size_page*crop_sz[3]];
    memcpy(image_tmp,cropped_image,size_page*crop_sz[3]);
    //visualize image as well as neurontree
    Image4DSimple image4d;
    image4d.setData(image_tmp,crop_sz[0],crop_sz[1],crop_sz[2],crop_sz[3],V3D_UINT8);
    v3dhandle window=callback->newImageWindow("Cropped Image and Trace");
    callback->setImage(window,&image4d);
    callback->open3DWindow(window);
    callback->setSWC(window,neuron);
    callback->pushObjectIn3DWindow(window);

    //save image
    //QString image_name=callback->getImageName(curwin);
    QString filename=QDir(folder_output).filePath("cropped_image.v3draw");
    //qDebug()<<"image name:"<<image_name<<" filename:"<<filename;
    if (!simple_saveimage_wrapper(*callback,filename.toStdString().c_str(),cropped_image,crop_sz,V3D_UINT8))
    {
        v3d_msg("Errors occur when saving the image");
        return;
    }
    //save eswc

    if (eswc_flag)
    {
        QString swc_name=QDir(folder_output).filePath("cropped_trace.eswc");
        if (!writeESWC_file(swc_name,neuron))
            return;
    }
    else
    {
        QString swc_name=QDir(folder_output).filePath("cropped_trace.swc");
        if (!writeSWC_file(swc_name,neuron))
            return;
    }
    if (cropped_image!=0)
    {
        delete[] cropped_image;
        cropped_image=0;
    }
    QMessageBox::information(0,"Success","Image and SWC file are saved at "+folder_output,QMessageBox::Ok);
}


bool load_data(V3DPluginCallback2 *cb,unsigned char * & image1Dc_in,ImagePixelType &pixeltype,
               V3DLONG sz_img[4],NeuronTree &neuron,v3dhandle &curwin)
{
    V3DPluginCallback2 *callback=cb;
    v3dhandleList v3dhandleList_current=callback->getImageWindowList();
    QList <V3dR_MainWindow *> cur_list_3dviewer = callback->getListAll3DViewers();

    if (v3dhandleList_current.size()==0){
        v3d_msg("Please load image and swc file");
        return false;
    }
    else if (v3dhandleList_current.size()==1)  //One window open
    {
        //check swc file
        QList<NeuronTree> *swc_list;
        swc_list=callback->getHandleNeuronTrees_3DGlobalViewer(v3dhandleList_current[0]);

        if (swc_list->size()==1)
        {
            curwin=v3dhandleList_current[0];
            backupNeuron(swc_list->at(0),neuron);
        }
        else if (swc_list->size()==0)
        {
            v3d_msg("Please load swc file");
            return false;
        }
        else
        {
            v3d_msg("Multiple swc files are found. Please load one");
            return false;
        }
    }
    else if (v3dhandleList_current.size()>1)
    {
        QStringList items;
        int i;
        for (i=0; i<v3dhandleList_current.size(); i++)
            items << callback->getImageName(v3dhandleList_current[i]);

        for (i=0; i<cur_list_3dviewer.count(); i++)
        {
            QString curname = callback->getImageName(cur_list_3dviewer[i]).remove("3D View [").remove("]");
            bool b_found=false;
            for (int j=0; j<v3dhandleList_current.size(); j++)
                if (curname==callback->getImageName(v3dhandleList_current[j]))
                {
                    b_found=true;
                    break;
                }

            if (!b_found)
                items << callback->getImageName(cur_list_3dviewer[i]);
        }
        //qDebug()<<"Number of items:"<<items.size();

        QDialog *mydialog=new QDialog;
        QComboBox *combo=new QComboBox;
        combo->insertItems(0,items);
        QLabel *label_win=new QLabel;
        label_win->setText("You have multiple windows open, please select one image:");
        QGridLayout *layout= new QGridLayout;
        layout->addWidget(label_win,0,0,1,1);
        layout->addWidget(combo,1,0,4,1);
        QPushButton *button_d_ok=new QPushButton("Ok");
        button_d_ok->setFixedWidth(100);
        QPushButton *button_d_cancel=new QPushButton("Cancel");
        button_d_cancel->setFixedWidth(100);
        QHBoxLayout *box=new QHBoxLayout;
        box->addWidget(button_d_ok,Qt::AlignCenter);
        box->addWidget(button_d_cancel,Qt::AlignCenter);
        layout->addLayout(box,5,0,1,1);
        QDialog::connect(button_d_ok,SIGNAL(clicked()),mydialog,SLOT(accept()));
        QDialog::connect(button_d_cancel,SIGNAL(clicked()),mydialog,SLOT(reject()));

        mydialog->setLayout(layout);
        mydialog->exec();
        if (mydialog->result()==QDialog::Accepted)
        {
            int tmp=combo->currentIndex();
            curwin=v3dhandleList_current[tmp];
        }
        else
        {
            v3d_msg("You have not selected a window");
            return false;
        }
        //check and get swc file
        QList<NeuronTree> *swc_list;
        swc_list=callback->getHandleNeuronTrees_3DGlobalViewer(curwin);

        if (swc_list->size()==0)
        {
            v3d_msg("Please load swc file");
            return false;
        }
        else if (swc_list->size()==1)
        {
            backupNeuron(swc_list->at(0),neuron);
        }
        else
        {
            v3d_msg("Multiple swc files are found. Please load one");
            return false;
        }
    }

    //check neurontree
    if (neuron.listNeuron.size()<=0)
    {
        v3d_msg("Error loading swc file, please check swc file");
        return false;
    }
    for (V3DLONG ii=0; ii<neuron.listNeuron.size(); ii++)
    {
        if(neuron.listNeuron.at(ii).r<0)
        {
            v3d_msg("You have illeagal radius values. Check your data.");
            return false;
        }
     }

    //Get the image info

    Image4DSimple* p4DImage = callback->getImage(curwin);
    pixeltype = p4DImage->getDatatype();
    if (pixeltype!=1)
    {
        v3d_msg("This image format cannot be handled. We only support UINT8 image now.");
        return false;
    }
    if (!p4DImage){
        QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
        return false;
    }

    sz_img[0]=p4DImage->getXDim();
    sz_img[1]=p4DImage->getYDim();
    sz_img[2]=p4DImage->getZDim();
    sz_img[3]=p4DImage->getCDim();

    if (sz_img[3]>3){
        sz_img[3]=3;
        QMessageBox::information(0,"","More than 3 channels were loaded."
                                 "The first 3 channel will be applied for analysis.");
    }

    V3DLONG size_tmp=sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];
    image1Dc_in=new unsigned char [size_tmp];
    unsigned char * ptmp=p4DImage->getRawData();
    memcpy(image1Dc_in,ptmp,size_tmp*pixeltype);

    return true;
}

void backupNeuron(const NeuronTree & source, const NeuronTree & backup)
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

vector<V3DLONG> pos2xyz(const V3DLONG _pos_input, const V3DLONG _offset_Y, const V3DLONG _offset_Z)
{
    vector<V3DLONG> pos3_result (3, -1);
    pos3_result[2]=floor(_pos_input/(double)_offset_Z);
    pos3_result[1]=floor((_pos_input-pos3_result[2]*_offset_Z)/(double)_offset_Y);
    pos3_result[0]=_pos_input-pos3_result[2]*_offset_Z-pos3_result[1]*_offset_Y;
    return pos3_result;
}

V3DLONG xyz2pos(const V3DLONG _x, const V3DLONG _y, const V3DLONG _z, const V3DLONG _offset_Y, const V3DLONG _offset_Z)
{
    return _z*_offset_Z+_y*_offset_Y+_x;
}
