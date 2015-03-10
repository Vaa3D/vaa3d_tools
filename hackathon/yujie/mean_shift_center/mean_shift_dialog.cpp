#include "mean_shift_dialog.h"

mean_shift_dialog::mean_shift_dialog(V3DPluginCallback2 *cb)
{
    callback=cb;
    //create();
    datasource=0;
    image1Dc_in=0;
}


void mean_shift_dialog::create()
{
    QBoxLayout *boxlayout=new QBoxLayout(QBoxLayout::TopToBottom);
    QToolButton *button_load=new QToolButton;
    button_load->setText("Load image");
    button_load->setGeometry(0,0,10,20);

    QToolButton *button_marker=new QToolButton;
    button_marker->setText("Load markers");
    button_marker->setGeometry(0,0,10,20);

    QToolButton *button_fetch=new QToolButton;
    button_fetch->setText("Fetch from 3D viewer");
    button_fetch->setGeometry(0,0,10,20);

//    QToolButton *button_push=new QToolButton;
//    button_push->setText("Push back new markers");
//    button_push->setGeometry(0,0,10,20);

    QToolButton *button_center=new QToolButton;
    button_center->setText("Get new centers");
    button_center->setGeometry(0,0,10,20);

    QToolButton *button_para=new QToolButton;
    button_para->setText("Parameter setting");
    button_para->setGeometry(0,0,10,20);

    QToolButton *button_help=new QToolButton;
    button_help->setText("Help");
    button_help->setGeometry(0,0,10,20);

    QToolBar *tool = new QToolBar;
    tool->setGeometry(0,0,200,20);
    tool->addWidget(button_load);
    tool->addSeparator();
//    tool->addWidget(button_marker);
//    tool->addSeparator();
    tool->addWidget(button_fetch);
    tool->addSeparator();
//    tool->addWidget(button_push);
//    tool->addSeparator();
    tool->addWidget(button_center);
    tool->addSeparator();
    tool->addWidget(button_para);
    tool->addSeparator();
    tool->addWidget(button_help);
    tool->addSeparator();

    QHBoxLayout *vlayout = new QHBoxLayout;
    vlayout->addWidget(tool);

    //Build the subdialog for parameter settings
    subDialog = new QDialog();
    subDialog->setWindowTitle("Parameter setting");
    connect(button_para, SIGNAL(clicked()), subDialog, SLOT(show()));
    connect(subDialog,SIGNAL(finished(int)),this,SLOT(dialoguefinish(int)));

    spin_radius = new QSpinBox();
    spin_radius->setRange(0,50); spin_radius->setValue(6);

    QGridLayout *gridLayout=new QGridLayout();
    QLabel* label_0 = new QLabel("search sphere radius (0~50):");
    gridLayout->addWidget(label_0,0,0,1,2);
    gridLayout->addWidget(spin_radius,0,3,1,1);

    QPushButton *button_ok=new QPushButton;
    button_ok->setText("OK");
    QPushButton *button_cancel=new QPushButton;
    button_cancel->setText("Cancel");
    QHBoxLayout *hlayout=new QHBoxLayout;
    hlayout->addWidget(button_ok);
    hlayout->addWidget(button_cancel);
    gridLayout->addLayout(hlayout,2,0,1,4);
    subDialog->setLayout(gridLayout);

    boxlayout->addLayout(vlayout);
    //boxlayout->addWidget(label);
    //boxlayout->addWidget(edit);
    setLayout(boxlayout);
    int height=tool->height();//+label->height()+edit->height();
    this->setFixedHeight(height+80);

    connect(button_load, SIGNAL(clicked()), this, SLOT(load()));
    connect(button_fetch,SIGNAL(clicked()),this, SLOT(fetch()));
    connect(button_center,SIGNAL(clicked()),this,SLOT(mean_shift_center()));
    connect(button_help,SIGNAL(clicked()),this,SLOT(help()));
    connect(button_ok,SIGNAL(clicked()),subDialog,SLOT(accept()));
    connect(button_cancel,SIGNAL(clicked()),subDialog,SLOT(reject()));
    prev_radius=spin_radius->value();
}

void mean_shift_dialog::dialoguefinish(int)
{
    if (subDialog->result()==QDialog::Accepted) {
        prev_radius=spin_radius->value();
        qDebug()<<"accepted";
    }

    else{
        qDebug()<<"Not accepted";
        spin_radius->setValue(prev_radius);
    }
}

bool mean_shift_dialog::load()
{
    qDebug()<<"In load";
    QString fileName;
    fileName = QFileDialog::getOpenFileName(0, QObject::tr("Choose the input image "),
             QDir::currentPath(),QObject::tr("Images (*.raw *.tif *.lsm *.v3dpbd *.v3draw);;All(*)"));

    if (!fileName.isEmpty())
    {
        resetdata();

        if (!simple_loadimage_wrapper(*callback, fileName.toStdString().c_str(), image1Dc_in, sz_img, intype))
        {
            QMessageBox::information(0,"","load image "+fileName+" error!");
            return false;
        }

        datasource=1;
        V3DLONG size_tmp=sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];

        if(intype==1)//V3D_UNIT8
        {
            mean_shift_obj.pushNewData<unsigned char>((unsigned char*)image1Dc_in, sz_img);
        }

        else if (intype == 2) //V3D_UINT16;
        {
            mean_shift_obj.pushNewData<unsigned short>((unsigned short*)image1Dc_in, sz_img);
            convert2UINT8((unsigned short*)image1Dc_in, image1Dc_in, size_tmp);
        }
        else if(intype == 4) //V3D_FLOAT32;
        {
            mean_shift_obj.pushNewData<float>((float*)image1Dc_in, sz_img);
            convert2UINT8((float*)image1Dc_in, image1Dc_in, size_tmp);
        }
        else
        {
           QMessageBox::information(0,"","Currently this program only supports UINT8, UINT16, and FLOAT32 data type.");
           return false;
        }

        updateInputWindow();
        return true;
    }
    return false;

}

void mean_shift_dialog::fetch()
{

    v3dhandle curwin;
    v3dhandleList v3dhandleList_current=callback->getImageWindowList();
    QList <V3dR_MainWindow *> cur_list_3dviewer = callback->getListAll3DViewers();
    qDebug()<<"size:"<<v3dhandleList_current.size();

    if (v3dhandleList_current.size()==0){
        v3d_msg("Please open image and select markers");
        return;
    }
    else if (v3dhandleList_current.size()==1)
    {
        //get markers and check markers
        qDebug()<<"Only 1 window open";
        LList_in.clear();
        LList_in = callback->getLandmark(v3dhandleList_current[0]);
        if (LList_in.size()==0)
        {
            v3d_msg("Please load markers");
            return;
        }
        curwin=v3dhandleList_current[0];
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
        qDebug()<<"Number of items:"<<items.size();

        QDialog *mydialog=new QDialog;
        combo=new QComboBox();
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
            curwin=v3dhandleList_current[tmp];
        }
        else
        {
            v3d_msg("You have not selected a window");
            return;
        }
        //get markers and check markers
        LList_in.clear();
        LList_in = callback->getLandmark(curwin);
        if (LList_in.size()==0)
        {
            v3d_msg("Please load markers");
            return;
        }
    }

    //Get the image info
    Image4DSimple* p4DImage = callback->getImage(curwin);
    if (!p4DImage){
        QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
        return;
    }
        //resetdata();
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
    image1Dc_in = p4DImage->getRawData();
    ImagePixelType pixeltype = p4DImage->getDatatype();

    if(pixeltype==1)//V3D_UNIT8
    {
        mean_shift_obj.pushNewData<unsigned char>((unsigned char*)image1Dc_in, sz_img);
    }

    else if (pixeltype == 2) //V3D_UINT16;
    {
        mean_shift_obj.pushNewData<unsigned short>((unsigned short*)image1Dc_in, sz_img);
        convert2UINT8((unsigned short*)image1Dc_in, image1Dc_in, size_tmp);
    }
    else if(pixeltype == 4) //V3D_FLOAT32;
    {
        mean_shift_obj.pushNewData<float>((float*)image1Dc_in, sz_img);
        convert2UINT8((float*)image1Dc_in, image1Dc_in, size_tmp);
    }
    else
    {
       QMessageBox::information(0,"","Currently this program only supports UINT8, UINT16, and FLOAT32 data type.");
       return;
    }

    int windowradius=15;
    //set parameter
    QDialog *mydialog_para=new QDialog;
    QLabel *label_info=new QLabel;
    label_info->setText("Please set the search window radius");
    QLabel *label_radius=new QLabel;
    label_radius->setText("Search window radius:");
    QGridLayout *layout2=new QGridLayout;
    QSpinBox *para_radius=new QSpinBox;
    para_radius->setRange(1,30);
    para_radius->setValue(15);
    layout2->addWidget(label_info,0,0,1,2);
    layout2->addWidget(label_radius,1,0,1,1);
    layout2->addWidget(para_radius,1,1,1,1);
    QPushButton *button_p_ok=new QPushButton;
    button_p_ok->setText("Ok");
    button_p_ok->setFixedWidth(100);
    QPushButton *button_p_cancel=new QPushButton;
    button_p_cancel->setText("Cancel");
    button_p_cancel->setFixedWidth(100);
    layout2->addWidget(button_p_ok,2,0,1,1);
    layout2->addWidget(button_p_cancel,2,1,1,1);
    connect(button_p_ok,SIGNAL(clicked()),mydialog_para,SLOT(accept()));
    connect(button_p_cancel,SIGNAL(clicked()),mydialog_para,SLOT(reject()));

    mydialog_para->setLayout(layout2);
    mydialog_para->exec();
    if (mydialog_para->result()==QDialog::Accepted)
    {
        windowradius=para_radius->value();
    }

    //copy the landmarks in LList
    for(int i=0; i<LList_in.size(); i++){
        LList.append(LList_in.at(i));
        LList[i].color.r=196;
        LList[i].color.g=LList[i].color.b=0;
    }

    //start mean-shift
    poss_landmark.clear();
    poss_landmark=landMarkList2poss(LList, sz_img[0], sz_img[0]*sz_img[1]);
    for (int j=0;j<poss_landmark.size();j++)
    {
        qDebug()<<"_______j:____________"<<j;
        mass_center=mean_shift_obj.calc_mean_shift_center(poss_landmark[j],windowradius);
        LocationSimple tmp(mass_center[0]+1,mass_center[1]+1,mass_center[2]+1);
        LList_new_center.append(tmp);
    }
    //visualize

//    callback->setLandmark(curwin, LList_new_center);
//    callback->updateImageWindow(curwin);
//    callback->pushObjectIn3DWindow(curwin);
    Image4DSimple image4d;
    unsigned char* image1D_input=memory_allocate_uchar1D(size_tmp);
    memcpy(image1D_input, image1Dc_in, size_tmp*sizeof(unsigned char));
    image4d.setData(image1D_input, sz_img[0], sz_img[1], sz_img[2], sz_img[3], V3D_UINT8);

    v3dhandle v3dhandle_main=callback->newImageWindow();
    callback->setImage(v3dhandle_main, &image4d);
    callback->setLandmark(v3dhandle_main,LList_new_center);
    callback->setImageName(v3dhandle_main, "output_"+callback->getImageName(curwin));
    callback->updateImageWindow(v3dhandle_main);
    callback->open3DWindow(v3dhandle_main);
    callback->pushObjectIn3DWindow(v3dhandle_main);


    qDebug()<<"The end of fetch";
}

void mean_shift_dialog::resetdata()
{
    qDebug()<<"in resetdata";
    if(image1Dc_in != 0){
       if(datasource==1) {
        delete []image1Dc_in; image1Dc_in=0;
        }
       else if(datasource==2){
        image1Dc_in=0;
       }
    }
    sz_img[0]=sz_img[1]=sz_img[2]=sz_img[3]=0;

    LList.clear();
    LList_new_center.clear();

}

void mean_shift_dialog::updateInputWindow()
{
    //search in open windows
    qDebug()<<"In updateinputwindow";
    bool winfound=false;
    v3dhandleList v3dhandleList_current=callback->getImageWindowList();
    for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
    {
        if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_INWIN))
        {
            winfound=true;
        }
    }
    if(image1Dc_in != 0){ //image loaded
        //generate a copy and show it

        Image4DSimple image4d;
        V3DLONG size_page = sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];
        unsigned char* image1D_input=memory_allocate_uchar1D(size_page);
        memcpy(image1D_input, image1Dc_in, size_page*sizeof(unsigned char));
        image4d.setData(image1D_input, sz_img[0], sz_img[1], sz_img[2], sz_img[3], V3D_UINT8);
        if(!winfound){ //open a window if none is open
            v3dhandle v3dhandle_main=callback->newImageWindow();
            //update the image
            callback->setImage(v3dhandle_main, &image4d);
            callback->setImageName(v3dhandle_main, NAME_INWIN);
            callback->updateImageWindow(v3dhandle_main);
            callback->open3DWindow(v3dhandle_main);
        }else{

            v3dhandleList_current=callback->getImageWindowList();
            for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
            {
                if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_INWIN))
                {
                    //update the image
                    if(!callback->setImage(v3dhandleList_current[i], &image4d)){
                        v3d_msg("Failed to update input image");
                    }
                    callback->updateImageWindow(v3dhandleList_current[i]);
                    callback->pushImageIn3DWindow(v3dhandleList_current[i]);
                    callback->open3DWindow(v3dhandleList_current[i]);
                }
            }
        }
    }
 }


void mean_shift_dialog::mean_shift_center()
{
    LList.clear();
    LList_new_center.clear();

    qDebug()<<"In meanshiftcenter now";
    v3dhandle found_win;
    bool winfound=false;
    v3dhandleList v3dhandleList_current=callback->getImageWindowList();
    for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
    {
        if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_INWIN))
        {
            found_win=v3dhandleList_current[i];
            winfound=true;
            break;
        }
    }

    if (!winfound)
    {
        v3d_msg("No image is found. Have you fetched image?");
        return;
    }

    LandmarkList LList_in = callback->getLandmark(found_win);
    for(int i=0; i<LList_in.size(); i++){
        LList.append(LList_in.at(i));
        LList[i].color.r=196;
        LList[i].color.g=LList[i].color.b=0;
    }

    if(LList.size()<=0)
    {
        v3d_msg("No markers were selected");
        return;
    }

    int windowradius=spin_radius->value();
    poss_landmark.clear();
    poss_landmark=landMarkList2poss(LList, sz_img[0], sz_img[0]*sz_img[1]);

    for (int j=0;j<poss_landmark.size();j++)
    {
        qDebug()<<j<<":"<<windowradius;
        mass_center=mean_shift_obj.calc_mean_shift_center(poss_landmark[j],windowradius);
        LocationSimple tmp(mass_center[0]+1,mass_center[1]+1,mass_center[2]+1);
        LList_new_center.append(tmp);
    }
    //qDebug()<<"LList_new center:"<<LList_new_center.size();
    updateOutputWindow();
}

void mean_shift_dialog::updateOutputWindow()
{
    qDebug()<<"In updateoutputwindow";
    bool winfound=false;
    v3dhandleList v3dhandleList_current=callback->getImageWindowList();
    v3dhandle found_win;
    for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
    {
        if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_OUTWIN))
        {
            winfound=true;
            found_win=v3dhandleList_current[i];
            break;
        }
    }
    if(image1Dc_in != 0){ //image loaded
        //generate a copy and show it
        Image4DSimple image4d;

        V3DLONG size_page=sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];
        unsigned char* image1Dc_input=memory_allocate_uchar1D(size_page);
        memcpy(image1Dc_input, image1Dc_in, size_page*sizeof(unsigned char));
        image4d.setData(image1Dc_input, sz_img[0], sz_img[1], sz_img[2], sz_img[3], V3D_UINT8);
        if(!winfound){ //open a window if none is open
            v3dhandle v3dhandle_main=callback->newImageWindow();
            callback->setImage(v3dhandle_main, &image4d);
            callback->setLandmark(v3dhandle_main, LList_new_center);
            callback->setImageName(v3dhandle_main, NAME_OUTWIN);
            callback->open3DWindow(v3dhandle_main);
            callback->pushObjectIn3DWindow(v3dhandle_main);
            return;
        }
        else{
            //update the image
            callback->setImage(found_win,&image4d);
            callback->setLandmark(found_win,LList_new_center);
            callback->updateImageWindow(found_win);
            }
        }
}

void mean_shift_dialog::help()
{
    QMessageBox::about(this, tr("How to use mean_shift_center_finder"),
    tr("<p>The <b>Mean shift center finder</b> performs searches around each of the user-input markers and "
       "returns the locations of local maxima of intensity as new markers in the output window.<p>"
       "<b>Load/Fetch from 3D viewers</b> -- Users can choose to load the image from local image files or fetch from current"
       " Vaa3D main window.<br>"
       "<b>Get new centers</b> -- Return the local maxima of intensity for each marker using mean shift method."
       " The default search area is a sphere with the radius of 10 pixels. <br>"
       "<p>For further questions, please contact Yujie Li at yujie.jade@gmail.com)</p>"));
}

void convert2UINT8(unsigned short *pre1d, unsigned char *pPost, V3DLONG imsz)
{
    unsigned short* pPre = (unsigned short*)pre1d;
    unsigned short max_v=0, min_v = 255;
    for(V3DLONG i=0; i<imsz; i++)
    {
        if(max_v<pPre[i]) max_v = pPre[i];
        if(min_v>pPre[i]) min_v = pPre[i];
    }
    max_v -= min_v;
    if(max_v>0)
    {
        for(V3DLONG i=0; i<imsz; i++)
        {
            pPost[i] = (unsigned char) 255*(double)(pPre[i] - min_v)/max_v;
        }
    }
    else
    {
        for(V3DLONG i=0; i<imsz; i++)
        {
            pPost[i] = (unsigned char) pPre[i];
        }
    }
}

void convert2UINT8(float *pre1d, unsigned char *pPost, V3DLONG imsz)
{
    float* pPre = (float*)pre1d;
    float max_v=0, min_v = 65535;
    for(V3DLONG i=0; i<imsz; i++)
    {
        if(max_v<pPre[i]) max_v = pPre[i];
        if(min_v>pPre[i]) min_v = pPre[i];
    }
    max_v -= min_v;
    if(max_v>0)
    {
        for(V3DLONG i=0; i<imsz; i++)
        {
            pPost[i] = (unsigned char) 255*(double)(pPre[i] - min_v)/max_v;
        }
    }
    else
    {
        for(V3DLONG i=0; i<imsz; i++)
        {
            pPost[i] = (unsigned char) pPre[i];
        }
    }
}


