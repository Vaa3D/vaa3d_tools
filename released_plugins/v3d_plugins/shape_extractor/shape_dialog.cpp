#include "shape_dialog.h"
#include "v3d_message.h"
#include <vector>
#include <math.h>
#include "string"
#include "sstream"
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <algorithm>
#include <basic_landmark.h>
#include <map>


shape_dialog::shape_dialog(V3DPluginCallback2 *cb)
{
    callback=cb;
    image1Dc_in=0;
    image1Dc_out=0;
    label=0;
    intype=0;
    sz_img[0]=sz_img[1]=sz_img[2]=sz_img[3]=0;
    LList.clear();
    LList_new_center.clear();
    count_img=0;
//    create();
    datasource=0;
//    label_m=0;
}


void shape_dialog::core()
{
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
        shape_ext_obj.pushNewData<unsigned char>((unsigned char*)image1Dc_in, sz_img);
    }

    else if (pixeltype == 2) //V3D_UINT16;
    {
        shape_ext_obj.pushNewData<unsigned short>((unsigned short*)image1Dc_in, sz_img);
        convert2UINT8((unsigned short*)image1Dc_in, image1Dc_in, size_tmp);
    }
    else if(pixeltype == 4) //V3D_FLOAT32;
    {
        shape_ext_obj.pushNewData<float>((float*)image1Dc_in, sz_img);
        convert2UINT8((float*)image1Dc_in, image1Dc_in, size_tmp);
    }
    else
    {
       QMessageBox::information(0,"","Currently this program only supports UINT8, UINT16, and FLOAT32 data type.");
       return;
    }

    bg_thr=70;
    //set parameter
    QDialog *mydialog_para=new QDialog;
    QLabel *label_info=new QLabel;
    label_info->setText("Please set background threshold");
    QLabel *label_bg=new QLabel;
    label_bg->setText("Background threshold:");
    QGridLayout *layout2=new QGridLayout;
    QSpinBox *para_bg=new QSpinBox;
    para_bg->setRange(0,255);
    para_bg->setValue(70);
    layout2->addWidget(label_info,0,0,1,2);
    layout2->addWidget(label_bg,1,0,1,1);
    layout2->addWidget(para_bg,1,1,1,1);
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
        bg_thr=para_bg->value();
    }

    //copy the landmarks in LList
    for(int i=0; i<LList_in.size(); i++){
        LList.append(LList_in.at(i));
        LList[i].color.r=196;
        LList[i].color.g=LList[i].color.b=0;
    }
    extract();

   //start region grow
}

void shape_dialog::create()
{
    QBoxLayout *boxlayout=new QBoxLayout(QBoxLayout::TopToBottom);
    QToolButton *button_load=new QToolButton;
    button_load->setText("Load");
    button_load->setGeometry(0,0,10,20);
    QToolButton *button_extract=new QToolButton;
    button_extract->setText("Region grow");
    button_extract->setGeometry(0,0,10,20);
    QToolButton *button_return=new QToolButton;
    button_return->setText("Region grow with optimized markers");
    button_return->setGeometry(0,0,10,20);
    QToolButton *button_fetch=new QToolButton;
    button_fetch->setText("Fetch");
    button_fetch->setGeometry(0,0,10,20);
    QToolButton *button_volume=new QToolButton;
    button_volume->setText("Get volume");
    button_volume->setGeometry(0,0,10,20);
    QToolButton *button_para=new QToolButton;
    button_para->setText("Parameter setting");
    button_para->setGeometry(0,0,10,20);
    QToolButton *button_help=new QToolButton;
    button_help->setText("Help");
    button_help->setGeometry(0,0,10,20);

    //Build the subdialog for parameter settings
    subDialog = new QDialog;
    subDialog->setWindowTitle("Parameter setting");
    connect(button_para, SIGNAL(clicked()), subDialog, SLOT(show()));
    connect(subDialog,SIGNAL(finished(int)),this,SLOT(dialoguefinish(int)));

    //parameters
    QGridLayout *gridLayout=new QGridLayout;
    spin_bgthr = new QSpinBox();
    spin_bgthr->setRange(0,255); spin_bgthr->setValue(70);
    spin_radius = new QSpinBox();
    spin_radius->setRange(0,50); spin_radius->setValue(10);
    spin_conviter = new QSpinBox();
    spin_conviter->setRange(0,100); spin_conviter->setValue(1);


    QLabel* label_0 = new QLabel("Background Threshold (0~255):");
    gridLayout->addWidget(label_0,0,0,1,2);
    gridLayout->addWidget(spin_bgthr,0,3,1,1);

    QLabel* label_1 = new QLabel("Search Window Radius (0~50):");
    gridLayout->addWidget(label_1,1,0,1,2);
    gridLayout->addWidget(spin_radius,1,3,1,1);

    QLabel* label_2 = new QLabel("Convolute Iteration (contrast factor): ");
    gridLayout->addWidget(label_2,2,0,1,2);
    gridLayout->addWidget(spin_conviter,2,3,1,1);

    QPushButton *button_ok=new QPushButton;
    button_ok->setText("OK");
    QPushButton *button_cancel=new QPushButton;
    button_cancel->setText("Cancel");
    QHBoxLayout *hlayout=new QHBoxLayout;
    hlayout->addWidget(button_ok);
    hlayout->addWidget(button_cancel);
    gridLayout->addLayout(hlayout,3,0,1,4);
    subDialog->setLayout(gridLayout);

    QToolBar *tool = new QToolBar;
    tool->setGeometry(0,0,200,20);
    tool->addWidget(button_load);
    tool->addSeparator();
    tool->addWidget(button_fetch);
    tool->addSeparator();
    tool->addWidget(button_extract);
    tool->addSeparator();
    tool->addWidget(button_return);
    tool->addSeparator();
    tool->addWidget(button_volume);
    tool->addSeparator();
    tool->addWidget(button_para);
    tool->addSeparator();
    tool->addWidget(button_help);
    tool->addSeparator();

    QHBoxLayout *vlayout = new QHBoxLayout;
    vlayout->addWidget(tool);

    label0= new QLabel();
    label0->setText("Filename:");
    label0->setMaximumHeight(30);

    QLabel *label= new QLabel;
    label->setText("Selected markers:");
    label->setMaximumHeight(30);

    edit=new QPlainTextEdit();
    edit->setPlainText("");
    edit->setReadOnly(true);
    edit->setMinimumHeight(30);

    boxlayout->addLayout(vlayout);
    boxlayout->addWidget(label0);
    boxlayout->addWidget(label);
    boxlayout->addWidget(edit);
    setLayout(boxlayout);
//    int height=tool->height()+label->height()+edit->height();
//    this->setFixedHeight(height+80);

     connect(button_load, SIGNAL(clicked()), this, SLOT(load()));
     connect(button_fetch,SIGNAL(clicked()),this, SLOT(fetch()));
     connect(button_extract,SIGNAL(clicked()),this,SLOT(extract()));
     connect(button_return,SIGNAL(clicked()),this,SLOT(extract_mean_shift()));
     connect(button_ok,SIGNAL(clicked()),subDialog,SLOT(accept()));
     connect(button_cancel,SIGNAL(clicked()),subDialog,SLOT(reject()));
     connect(button_help,SIGNAL(clicked()),this,SLOT(help()));
     connect(button_volume, SIGNAL(clicked()), this, SLOT(display_volume()));
     //connect(button_swc, SIGNAL(clicked()), this, SLOT(load_swc_filter()));
     prev_bgthr=spin_bgthr->value();
    // prev_distance=spin_distance->value();
     prev_conviter=spin_conviter->value();
    // prev_percent=spin_percent->value();
}


void shape_dialog::dialoguefinish(int)
{
    if (subDialog->result()==QDialog::Accepted)
      {
        //qDebug()<<"Accepted";
        prev_bgthr=spin_bgthr->value();
        prev_conviter=spin_conviter->value();
    }

    else{
        //qDebug()<<"Not accepting";
       spin_bgthr->setValue(prev_bgthr);
       //spin_distance->setValue(prev_distance);
       spin_conviter->setValue(prev_conviter);
       //spin_percent->setValue(prev_percent);
    }
}

bool shape_dialog::load()
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
            shape_ext_obj.pushNewData<unsigned char>((unsigned char*)image1Dc_in, sz_img);
        }

        else if (intype == 2) //V3D_UINT16;
        {
            shape_ext_obj.pushNewData<unsigned short>((unsigned short*)image1Dc_in, sz_img);
            convert2UINT8((unsigned short*)image1Dc_in, image1Dc_in, size_tmp);
        }
        else if(intype == 4) //V3D_FLOAT32;
        {
            shape_ext_obj.pushNewData<float>((float*)image1Dc_in, sz_img);
            convert2UINT8((float*)image1Dc_in, image1Dc_in, size_tmp);
        }
        else
        {
           QMessageBox::information(0,"","Currently this program only supports UINT8, UINT16, and FLOAT32 data type.");
           return false;
        }

        edit->clear();
        label0->setText(QString("File:")+fileName);
        updateInputWindow();
        return true;
    }
    return false;

}

void shape_dialog::fetch()
{
    qDebug()<<"IN fetch";
    curwin = callback->currentImageWindow();
    if (!curwin){
        QMessageBox::information(0, "", "You don't have any image open in the main window.");
        return;
    }
    if(datasource==1)
    {
        if (callback->getImageName(curwin).contains(NAME_INWIN)||
                callback->getImageName(curwin).contains(NAME_OUTWIN)){
        v3d_msg("You have loaded the image.");
        return;
        }
    }
    if (datasource==2)
    {
        if (callback->getImageName(curwin).contains(NAME_INWIN)||
                   callback->getImageName(curwin).contains(NAME_OUTWIN)){
        v3d_msg("You have fetched the image.");
        return;
        }
    }
    Image4DSimple* p4DImage = callback->getImage(curwin);
    QString filename=callback->getImageName(curwin);
    if (!p4DImage){
        QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
        return;
    }

    resetdata();

    datasource=2;
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
        shape_ext_obj.pushNewData<unsigned char>((unsigned char*)image1Dc_in, sz_img);
    }

    else if (pixeltype == 2) //V3D_UINT16;
    {
        shape_ext_obj.pushNewData<unsigned short>((unsigned short*)image1Dc_in, sz_img);
        convert2UINT8((unsigned short*)image1Dc_in, image1Dc_in, size_tmp);
    }
    else if(pixeltype == 4) //V3D_FLOAT32;
    {
        shape_ext_obj.pushNewData<float>((float*)image1Dc_in, sz_img);
        convert2UINT8((float*)image1Dc_in, image1Dc_in, size_tmp);
    }
    else
    {
       QMessageBox::information(0,"","Currently this program only supports UINT8, UINT16, and FLOAT32 data type.");
       return;
    }

    callback->close3DWindow(curwin);
    callback->setImageName(curwin, NAME_INWIN);
    callback->open3DWindow(curwin);
    callback->pushObjectIn3DWindow(curwin);
    callback->updateImageWindow(curwin);

    v3d_msg("You have successfully fetched the image.");
    label0->setText(QString("File:")+filename);
}

void shape_dialog::resetdata()
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

    if(image1Dc_out!=0){
        delete []image1Dc_out; image1Dc_out=0;
    }
    if(label!=0) {
        delete []label; label=0;
    }
}

void shape_dialog::updateInputWindow()
{
    //search in open windows
    bool winfound=false;
    v3dhandle found_win;
    v3dhandleList v3dhandleList_current=callback->getImageWindowList();
    for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
    {
        if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_INWIN))
        {
            winfound=true;
            found_win=v3dhandleList_current[i];
            break;
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
        }
        else{
            //update the image
            if(!callback->setImage(found_win, &image4d)){
                v3d_msg("Failed to update input image");
                return;
            }
            callback->updateImageWindow(found_win);
            callback->pushImageIn3DWindow(found_win);
        }
    }
 }


void shape_dialog::extract()
{
//    bool winfound=false;
//    v3dhandle found_win;
//    v3dhandleList v3dhandleList_current=callback->getImageWindowList();
//    for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
//    {
//        if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_INWIN))
//        {
//            winfound=true;
//            found_win=v3dhandleList_current[i];
//            break;
//        }
//    }

//    if (!winfound)
//    {
//        v3d_msg("No image is found. Please load or fetch an image");
//        return;
//    }

//    LList.clear();
//    LandmarkList LList_in = callback->getLandmark(found_win);
//    for(int i=0; i<LList_in.size(); i++){
//        LList.append(LList_in.at(i));
//        LList[i].color.r=196;
//        LList[i].color.g=LList[i].color.b=0;
//    }

//    if(LList.size()<=0)
//    {
//        v3d_msg("No markers were selected");
//        return;
//    }
//    edit->clear();
//    for (int i=0;i<LList.size();i++)
//     {
//        QString tmp="Marker "+ QString::number(i+1)+ ": " + QString::number(LList[i].x)+","+
//                QString::number(LList[i].y)+ "," +QString::number(LList[i].z);
//        edit->appendPlainText(tmp);
//     }


//    int convolute_iter=spin_conviter->value();
//    int bg_thr=spin_bgthr->value();
    int convolute_iter=1;
    V3DLONG size_page = sz_img[0]*sz_img[1]*sz_img[2];
    poss_landmark.clear();
    poss_landmark=landMarkList2poss(LList, sz_img[0], sz_img[0]*sz_img[1]);
    if (image1Dc_out!=0)
    {
        memory_free_uchar1D(image1Dc_out);
    }
    image1Dc_out=memory_allocate_uchar1D(sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3]);
    memset(image1Dc_out,0,sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3]*sizeof(unsigned char));

    if (label!=0)
    {
       memory_free_uchar1D(label);
    }
    label=memory_allocate_uchar1D(size_page*sizeof(unsigned char));
    memset(label,0,size_page*sizeof(unsigned char));

    V3DLONG sumrsize=0;
    //volume.clear();
    for (int j=0;j<poss_landmark.size();j++){
        qDebug()<<"j:"<<j;

        V3DLONG rsize=shape_ext_obj.extract(x_all, y_all,z_all,label,poss_landmark[j],
                                       convolute_iter,bg_thr,j+1);
        if(rsize<=0) continue;
//        mass_center=shape_ext_obj.get_mass_center(x_all,y_all,z_all);
//        LocationSimple tmp(mass_center[0]+1,mass_center[1]+1,mass_center[2]+1);
//        LList_new_center.append(tmp);

//        volume.push_back(x_all.size());

        GetColorRGB(rgb,j);
        V3DLONG finalpos;
        for (int i=0;i<x_all.size();i++)
        {
            finalpos=xyz2pos(x_all[i],y_all[i],z_all[i],sz_img[0],sz_img[0]*sz_img[1]);

                image1Dc_out[finalpos]=rgb[0];
                if (sz_img[3]>1){
                image1Dc_out[finalpos+1*sz_img[0]*sz_img[1]*sz_img[2]]=rgb[1];
                }
                if (sz_img[3]>2){
                image1Dc_out[finalpos+2*sz_img[0]*sz_img[1]*sz_img[2]]=rgb[2];
                }
            sumrsize=rsize+sumrsize;
        }
     }

    qDebug()<<"sumrsize:"<<sumrsize;

    if (sumrsize>0)
    {
        //updateOutputWindow();
        Image4DSimple image4d,image4d_color;

        unsigned char* image1Dc_input=memory_allocate_uchar1D(size_page*sz_img[3]);
        memcpy(image1Dc_input, image1Dc_out, size_page*sz_img[3]*sizeof(unsigned char));
        unsigned char* label_input=memory_allocate_uchar1D(size_page);
        memcpy(label_input, label, size_page*sizeof(unsigned char));

        image4d.setData(label_input, sz_img[0], sz_img[1], sz_img[2], 1, V3D_UINT8);
        image4d_color.setData(image1Dc_input,sz_img[0], sz_img[1], sz_img[2],sz_img[3],V3D_UINT8);

        count_img++;
        v3dhandle v3dhandle_main=callback->newImageWindow();
        callback->setImage(v3dhandle_main, &image4d_color);
        callback->setLandmark(v3dhandle_main, LList);
        callback->setImageName(v3dhandle_main, "extractor_"+QString(callback->getImageName(curwin)));
        callback->updateImageWindow(v3dhandle_main);
        callback->open3DWindow(v3dhandle_main);
        callback->pushObjectIn3DWindow(v3dhandle_main);
    }
    else
    v3d_msg("Nothing were found. Please change marker or adjust parameters.");
}

void shape_dialog::extract_mean_shift()
{
    bool winfound=false;
    v3dhandle found_win;
    v3dhandleList v3dhandleList_current=callback->getImageWindowList();
    for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
    {
        if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_INWIN))
        {
            winfound=true;
            found_win=v3dhandleList_current[i];
            break;
        }
    }

    if (!winfound)
    {
        v3d_msg("No image is found. Please load or fetch an image");
        return;
    }

    LList.clear();
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
    edit->clear();
    for (int i=0;i<LList.size();i++)
     {
        QString tmp="Marker "+ QString::number(i+1)+ ": " + QString::number(LList[i].x)+","+
                QString::number(LList[i].y)+ "," +QString::number(LList[i].z);
        edit->appendPlainText(tmp);
     }

    V3DLONG size_page = sz_img[0]*sz_img[1]*sz_img[2];
    int convolute_iter=spin_conviter->value();
    int bg_thr=spin_bgthr->value();
    int windowradius=spin_radius->value();

    poss_landmark.clear();
    poss_landmark=landMarkList2poss(LList, sz_img[0], sz_img[0]*sz_img[1]);

    //__________________________Using mean_shift to optimize markers

    LList_new_center.clear();
    for (int j=0;j<poss_landmark.size();j++)
    {
        mass_center=shape_ext_obj.calc_mean_shift_center(poss_landmark[j],windowradius);
        LocationSimple tmp(mass_center[0]+1,mass_center[1]+1,mass_center[2]+1);
        LList_new_center.append(tmp);
    }

    //Start region growing
    poss_landmark.clear();
    poss_landmark=landMarkList2poss(LList_new_center, sz_img[0], sz_img[0]*sz_img[1]);

    if (image1Dc_out!=0) memory_free_uchar1D(image1Dc_out);
    image1Dc_out=memory_allocate_uchar1D(sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3]);
    memset(image1Dc_out,0,sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3]*sizeof(unsigned char));

    if (label!=0) memory_free_uchar1D(label);
    label=memory_allocate_uchar1D(size_page*sizeof(unsigned char));
    memset(label,0,size_page*sizeof(unsigned char));

    V3DLONG sumrsize=0;
    volume.clear();
    for (int j=0;j<poss_landmark.size();j++){
        qDebug()<<"j:"<<j;

        V3DLONG rsize=shape_ext_obj.extract(x_all, y_all,z_all,label_m,poss_landmark[j],
                                       convolute_iter,bg_thr,j+1);
        if(rsize<=0) continue;
        volume.push_back(x_all.size());

        GetColorRGB(rgb,j);
        V3DLONG finalpos;
        for (int i=0;i<x_all.size();i++)
        {
            finalpos=xyz2pos(x_all[i],y_all[i],z_all[i],sz_img[0],sz_img[0]*sz_img[1]);

                image1Dc_out[finalpos]=rgb[0];
                if (sz_img[3]>1){
                image1Dc_out[finalpos+1*sz_img[0]*sz_img[1]*sz_img[2]]=rgb[1];
                }
                if (sz_img[3]>2){
                image1Dc_out[finalpos+2*sz_img[0]*sz_img[1]*sz_img[2]]=rgb[2];
                }
            sumrsize=rsize+sumrsize;
        }

    qDebug()<<"sumrsize:"<<sumrsize;
    }
    if (sumrsize>0)
    {
        LList.clear();
        for(int i=0; i<LList_new_center.size(); i++){
            LList.append(LList_new_center.at(i));
            LList[i].color.r=196;
            LList[i].color.g=LList[i].color.b=0;
        }
        updateOutputWindow();


    }
    else
    v3d_msg("Nothing were found. Please change marker or adjust parameters.");

}


void shape_dialog::extract_mean_shift_comparison()
{
    bool winfound=false;
    v3dhandle found_win;
    v3dhandleList v3dhandleList_current=callback->getImageWindowList();
    for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
    {
        if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_INWIN))
        {
            winfound=true;
            found_win=v3dhandleList_current[i];
            break;
        }
    }

    if (!winfound)
    {
        v3d_msg("No image is found. Please load or fetch an image");
        return;
    }

    LList.clear();
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
    edit->clear();
    for (int i=0;i<LList.size();i++)
     {
        QString tmp="Marker "+ QString::number(i+1)+ ": " + QString::number(LList[i].x)+","+
                QString::number(LList[i].y)+ "," +QString::number(LList[i].z);
        edit->appendPlainText(tmp);
     }

    V3DLONG size_page = sz_img[0]*sz_img[1]*sz_img[2];
    int convolute_iter=spin_conviter->value();
    int bg_thr=spin_bgthr->value();
    int windowradius=spin_radius->value();

    //_________________________Start the normal region grow
    poss_landmark.clear();
    poss_landmark=landMarkList2poss(LList, sz_img[0], sz_img[0]*sz_img[1]);

    if (label!=0)
    {
       memory_free_uchar1D(label);
    }
    label=memory_allocate_uchar1D(size_page*sizeof(unsigned char));
    memset(label,0,size_page*sizeof(unsigned char));

    //V3DLONG sumrsize=0;
    for (int j=0;j<poss_landmark.size();j++){
        qDebug()<<"j:"<<j;
        V3DLONG rsize=shape_ext_obj.extract(x_all, y_all,z_all,label,poss_landmark[j],
                                       convolute_iter,bg_thr,j+1);
        if(rsize<=0) continue;

    }

    //__________________________Using mean_shift to optimize markers
    LList_new_center.clear();
    for (int j=0;j<poss_landmark.size();j++)
    {
        mass_center=shape_ext_obj.calc_mean_shift_center(poss_landmark[j],windowradius);
        LocationSimple tmp(mass_center[0]+1,mass_center[1]+1,mass_center[2]+1);
        LList_new_center.append(tmp);
    }

    //Start region growing
    poss_landmark.clear();
    poss_landmark=landMarkList2poss(LList_new_center, sz_img[0], sz_img[0]*sz_img[1]);

    if (image1Dc_out!=0) memory_free_uchar1D(image1Dc_out);
    image1Dc_out=memory_allocate_uchar1D(sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3]);
    memset(image1Dc_out,0,sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3]*sizeof(unsigned char));

    if (label_m!=0) memory_free_uchar1D(label_m);
    label_m=memory_allocate_uchar1D(size_page*sizeof(unsigned char));
    memset(label_m,0,size_page*sizeof(unsigned char));

    V3DLONG sumrsize=0;
    volume.clear();
    for (int j=0;j<poss_landmark.size();j++){
        qDebug()<<"j:"<<j;

        V3DLONG rsize=shape_ext_obj.extract(x_all, y_all,z_all,label_m,poss_landmark[j],
                                       convolute_iter,bg_thr,j+1);
        if(rsize<=0) continue;
        volume.push_back(x_all.size());
    }

    //________________mean_shift extraction finished
    // comparison of label and label_m
    //
    for (int i=0;i<size_page;i++)
    {
        if (label[i]==0 && label_m[i]==0)
            continue;
        else if (label[i]>0 && label_m[i]>0)
            image1Dc_out[i]=image1Dc_out[i+size_page]=image1Dc_out[i+2*size_page]=255;
        else if (label[i]>0 && label_m[i]==0)
        {
            image1Dc_out[i]=196;
            image1Dc_out[i+size_page]=image1Dc_out[i+2*size_page]=0;
        }
        else if (label[i]==0 && label_m[i]>0)
        {
            image1Dc_out[i]=image1Dc_out[i+size_page]=0;
            image1Dc_out[i+2*size_page]=196;
        }
    }

    for(int i=0; i<LList_new_center.size(); i++)
        LList.append(LList_new_center.at(i));

    Image4DSimple image4d;
    unsigned char* image1Dc_input=memory_allocate_uchar1D(size_page*sz_img[3]);
    memcpy(image1Dc_input, image1Dc_out, size_page*sz_img[3]*sizeof(unsigned char));
    image4d.setData(image1Dc_input,sz_img[0],sz_img[1],sz_img[2],sz_img[3],V3D_UINT8);
    v3dhandle v3dhandle_main=callback->newImageWindow();
    callback->setImage(v3dhandle_main, &image4d);
    callback->setLandmark(v3dhandle_main, LList);
    callback->setImageName(v3dhandle_main, NAME_OUTWIN);
    callback->updateImageWindow(v3dhandle_main);
    callback->open3DWindow(v3dhandle_main);
    callback->pushObjectIn3DWindow(v3dhandle_main);

//        GetColorRGB(rgb,j);
//        V3DLONG finalpos;
//        for (int i=0;i<x_all.size();i++)
//        {
//            finalpos=xyz2pos(x_all[i],y_all[i],z_all[i],sz_img[0],sz_img[0]*sz_img[1]);

//                image1Dc_out[finalpos]=rgb[0];
//                if (sz_img[3]>1){
//                image1Dc_out[finalpos+1*sz_img[0]*sz_img[1]*sz_img[2]]=rgb[1];
//                }
//                if (sz_img[3]>2){
//                image1Dc_out[finalpos+2*sz_img[0]*sz_img[1]*sz_img[2]]=rgb[2];
//                }
//            sumrsize=rsize+sumrsize;
//        }
//     }

//    qDebug()<<"sumrsize:"<<sumrsize;

//    if (sumrsize>0)
//    {
//        LList.clear();
//        for(int i=0; i<LList_new_center.size(); i++){
//            LList.append(LList_new_center.at(i));
//            LList[i].color.r=196;
//            LList[i].color.g=LList[i].color.b=0;
//        }
//        updateOutputWindow();
//    }
//    else
//    v3d_msg("Nothing were found. Please change marker or adjust parameters.");

}



void shape_dialog::display_volume()
{
    //qDebug()<<"volume size:"<<volume.size();
    edit->clear();

    if (volume.size()>0)
    {
        for (int i=0;i<volume.size();i++)
         {
            QString tmp="Marker "+ QString::number(i+1)+ ": " +"  Volume:"
                    + QString::number(volume[i])+" voxels";
            edit->appendPlainText(tmp);
         }

    }
}

//void shape_dialog::display_mass_center()
//{
//    if(LList.size()<=0) {
//        v3d_msg("No markers were selected.");
//        return;
//    }
//    if(LList_new_center.size()<=0 && LList.size()>0)
//    {
//        v3d_msg("You need to find the edge first");
//        return;
//    }
//    else
//    {
//        //Append the new markers to the old marker list and show in blue
//        for(int i=0; i<LList_new_center.size(); i++){
//            LList.append(LList_new_center.at(i));
//            LList[LList.size()-1].color.b=255;
//            LList[LList.size()-1].color.r=LList[LList.size()-1].color.g=0;
//        }

//    //Update the current window of new markers
//        updateOutputWindow();
//    }
//}


void shape_dialog::updateOutputWindow()
{
    qDebug()<<"In updateoutputwindow";
    bool winfound=false;
    v3dhandle found_win;
    v3dhandleList v3dhandleList_current=callback->getImageWindowList();
    for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
    {
        if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_OUTWIN))
        {
            winfound=true;
            found_win=v3dhandleList_current[i];
            break;
        }
    }
    if(image1Dc_out != 0){ //image loaded
        //generate a copy and show it
        Image4DSimple image4d,image4d_color;

        V3DLONG size_page=sz_img[0]*sz_img[1]*sz_img[2];
        unsigned char* image1Dc_input=memory_allocate_uchar1D(size_page*sz_img[3]);
        memcpy(image1Dc_input, image1Dc_out, size_page*sz_img[3]*sizeof(unsigned char));
        unsigned char* label_input=memory_allocate_uchar1D(size_page);
        memcpy(label_input, label, size_page*sizeof(unsigned char));

        image4d.setData(label_input, sz_img[0], sz_img[1], sz_img[2], 1, V3D_UINT8);
        image4d_color.setData(image1Dc_input,sz_img[0], sz_img[1], sz_img[2],sz_img[3],V3D_UINT8);
        if(!winfound){ //open a window if none is open
            v3dhandle v3dhandle_main=callback->newImageWindow();
            callback->setImage(v3dhandle_main, &image4d);
            callback->setLandmark(v3dhandle_main, LList);
            callback->setImageName(v3dhandle_main, NAME_OUTWIN);
            callback->updateImageWindow(v3dhandle_main);

            //Open another window for the colorful image for visualization
            v3dhandle v3dhandle_color=callback->newImageWindow();
            callback->setImage(v3dhandle_color, &image4d_color);
            callback->setLandmark(v3dhandle_color, LList);
            callback->setImageName(v3dhandle_color, "Output_Shape_Extractor_color");
            callback->open3DWindow(v3dhandle_color);
            callback->pushObjectIn3DWindow(v3dhandle_color);
            callback->updateImageWindow(v3dhandle_color);
        }
        else{

            callback->setImage(found_win, &image4d);
            callback->setLandmark(found_win, LList);
            callback->setImageName(found_win, NAME_OUTWIN);
            callback->updateImageWindow(found_win);

            //Open another window for the colorful image for visualization
            v3dhandle v3dhandle_color=callback->newImageWindow();
            callback->setImage(v3dhandle_color, &image4d_color);
            callback->setLandmark(v3dhandle_color, LList);
            callback->setImageName(v3dhandle_color, "Output_Shape_Extractor_color");
            callback->open3DWindow(v3dhandle_color);
            callback->pushObjectIn3DWindow(v3dhandle_color);
            callback->updateImageWindow(v3dhandle_color);
        }
    }
    qDebug()<<"Finish outputwindow";
}


void shape_dialog::convert2UINT8(unsigned short *pre1d, unsigned char *pPost, V3DLONG imsz)
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

void shape_dialog::convert2UINT8(float *pre1d, unsigned char *pPost, V3DLONG imsz)
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

void shape_dialog::help()
{
    QMessageBox::about(this, tr("How to use Shape extractor"),
    tr("<p>The <b>Shape Extractor</b> is a tool to extract the shape of individual cells user defined.<p>"
               "<b>Load/Fetch</b> -- Users can choose to load the image from local image files or fetch from current"
               " Vaa3D main window.<br>"
               "<b>Region grow</b> -- Based on the user defined markers in the image, region grow function will generate"
               " corresponding cell shapes marked by different colors in window named 'output_shape_extractor_color'.<br>"
               " Each region is labeld using the marker number in Tri-view window named 'output_shape_extractor'.<br>"
               "<b>Get mass center</b> -- Calculate the mass center of the cell and display as new markers.<br>"
               "<b>Get volume</b> -- Calculate the volume of every extracted cell and display them in the text editor.<br>"
               "<b>Parameter setting</b>-- User can set the background threshold and the times of convolution that can impact"
               " the calculation of pixel intensity. <br>"
               "<p>For further questions, please contact Yujie Li at yujie.jade@gmail.com)</p>"));
}


void shape_dialog::GetColorRGB(int* rgb, int idx)
{
    idx=idx+1;
    if(idx>=0){
    idx = idx > 0 ? idx % 128 : 128 - abs(idx % 128);
    int colorLib[128][3] = {
            {39,16,2}, //{55, 173, 188},
            {3, 91, 61},
            {237, 195, 97},
            {175, 178, 151},
            {245, 199, 137},
            {24, 226, 66},
            {118, 84, 193},
            {205, 205, 224},
            {22, 170, 128},
            {86, 150, 90},
            {53, 67, 25},
            {38, 82, 152},
            {55,173,188}, //{39, 16, 2},
            {197, 174, 31},
            {183, 41, 31},
            {174, 37, 15},
            {34, 4, 48},
            {18, 10, 79},
            {132, 100, 254},
            {112, 39, 198},
            {189, 22, 120},
            {75, 104, 110},
            {222, 180, 4},
            {6, 60, 153},
            {236, 85, 113},
            {25, 182, 184},
            {200, 240, 12},
            {204, 119, 71},
            {11, 201, 229},
            {198, 214, 88},
            {49, 97, 65},
            {5, 75, 61},
            {52, 89, 137},
            {32, 49, 125},
            {104, 187, 76},
            {206, 233, 195},
            {85, 183, 9},
            {157, 29, 13},
            {14, 7, 244},
            {134, 193, 150},
            {188, 81, 216},
            {126, 29, 207},
            {170, 97, 183},
            {107, 14, 149},
            {76, 104, 149},
            {80, 38, 253},
            {27, 85, 10},
            {235, 95, 252},
            {139, 144, 48},
            {55, 124, 56},
            {239, 71, 243},
            {208, 89, 6},
            {87, 98, 24},
            {247, 48, 42},
            {129, 130, 13},
            {94, 149, 254},
            {111, 177, 4},
            {39, 229, 12},
            {7, 146, 87},
            {56, 231, 174},
            {95, 102, 52},
            {61, 226, 235},
            {199, 62, 149},
            {51, 32, 175},
            {232, 191, 210},
            {57, 99, 107},
            {239, 27, 135},
            {158, 71, 50},
            {104, 92, 41},
            {228, 112, 171},
            {54, 120, 13},
            {126, 69, 174},
            {191, 100, 143},
            {187, 156, 148},
            {18, 95, 9},
            {104, 168, 147},
            {249, 113, 198},
            {145, 5, 131},
            {104, 56, 59},
            {112, 235, 81},
            {73, 93, 127},
            {207, 60, 6},
            {77, 76, 211},
            {35, 208, 220},
            {141, 5, 129},
            {182, 178, 228},
            {179, 239, 3},
            {209, 9, 132},
            {167, 192, 71},
            {154, 227, 164},
            {200, 125, 103},
            {224, 181, 211},
            {3, 24, 139},
            {218, 67, 198},
            {163, 185, 228},
            {196, 53, 11},
            {39, 183, 33},
            {56, 116, 119},
            {23, 129, 20},
            {42, 191, 85},
            {78, 209, 250},
            {247, 147, 60},
            {74, 172, 146},
            {51, 244, 86},
            {38, 84, 29},
            {197, 202, 150},
            {20, 49, 198},
            {90, 214, 34},
            {178, 49, 101},
            {86, 235, 214},
            {161, 221, 55},
            {17, 173, 136},
            {132, 65, 217},
            {210, 4, 121},
            {241, 117, 217},
            {137, 111, 6},
            {129, 224, 232},
            {73, 34, 0},
            {81, 135, 211},
            {172, 233, 193},
            {43, 246, 89},
            {153, 187, 222},
            {85, 118, 43},
            {119, 116, 33},
            {163, 229, 109},
            {45, 75, 15},
            {15, 7, 140},
            {144, 78, 192}
    };
    for(int k = 0; k < 3; k++)
        rgb[k] = colorLib[idx][k];
    }
    else{
        rgb[0]=0;
        rgb[1]=0;
        rgb[2]=0;
    }
}

