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

shape_dialog::shape_dialog(V3DPluginCallback2 *cb)
{
    callback=cb;
    image1Dc_in=0;
    intype=0;
    sz_img[0]=sz_img[1]=sz_img[2]=sz_img[3]=0;
    LList.clear();
    create();
    datasource=0;
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
    button_return->setText("Find mass center");
    button_return->setGeometry(0,0,10,20);
    QToolButton *button_fetch=new QToolButton;
    button_fetch->setText("Fetch");
    button_fetch->setGeometry(0,0,10,20);
    QToolButton *button_para=new QToolButton;
    button_para->setText("Parameter setting");
    button_para->setGeometry(0,0,10,20);
    QToolButton *button_clear=new QToolButton;
    button_clear->setText("Clear markers");
    button_clear->setGeometry(0,0,10,20);

    //Build the subdialog for parameter settings
    subDialog = new QDialog;
    subDialog->setWindowTitle("Parameter setting");
    connect(button_para, SIGNAL(clicked()), subDialog, SLOT(show()));
    connect(subDialog,SIGNAL(finished(int)),this,SLOT(dialoguefinish(int)));

    //parameters
    QGridLayout *gridLayout=new QGridLayout;
    spin_bgthr = new QSpinBox();
    spin_bgthr->setRange(0,255); spin_bgthr->setValue(70);
//    spin_distance = new QSpinBox();
//    spin_distance->setRange(0,100000); spin_distance->setValue(3);
    spin_conviter = new QSpinBox();
    spin_conviter->setRange(0,100); spin_conviter->setValue(1);
//    spin_percent = new QSpinBox();
//    spin_percent->setRange(0,100); spin_percent->setValue(70);

    QLabel* label_0 = new QLabel("Background Threshold (0~255):");
    gridLayout->addWidget(label_0,0,0,1,2);
    gridLayout->addWidget(spin_bgthr,0,3,1,1);
//    QLabel* label_1 = new QLabel("neighbor(cubic) mask size: ");
//    gridLayout->addWidget(label_1,1,0,1,2);
//    gridLayout->addWidget(spin_distance,1,3,1,1);
//    QLabel* label_1 = new QLabel("Percent of Non-cell Pixels in Sphere: ");
//    gridLayout->addWidget(label_1,1,0,1,2);
//    gridLayout->addWidget(spin_percent,1,3,1,1);

    QLabel* label_2 = new QLabel("Convolute Iteration (contrast factor): ");
    gridLayout->addWidget(label_2,1,0,1,2);
    gridLayout->addWidget(spin_conviter,1,3,1,1);

    QPushButton *button_ok=new QPushButton;
    button_ok->setText("OK");
    QPushButton *button_cancel=new QPushButton;
    button_cancel->setText("Cancel");
    QHBoxLayout *hlayout=new QHBoxLayout;
    hlayout->addWidget(button_ok);
    hlayout->addWidget(button_cancel);
    gridLayout->addLayout(hlayout,2,0,1,4);
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
    tool->addWidget(button_clear);
    tool->addSeparator();
    tool->addWidget(button_para);

    QHBoxLayout *vlayout = new QHBoxLayout;
    vlayout->addWidget(tool);

    QLabel *label= new QLabel;
    label->setText("Selected markers:");
    label->setMaximumHeight(50);

    edit=new QPlainTextEdit;
    edit->setPlainText("");
    edit->setReadOnly(true);
    edit->setFixedHeight(100);

    boxlayout->addLayout(vlayout);
    boxlayout->addWidget(label);
    boxlayout->addWidget(edit);
    setLayout(boxlayout);
    int height=tool->height()+label->height()+edit->height();
    this->setFixedHeight(height+80);

     connect(button_load, SIGNAL(clicked()), this, SLOT(load()));
     connect(button_fetch,SIGNAL(clicked()),this, SLOT(fetch()));
     connect(button_extract,SIGNAL(clicked()),this,SLOT(extract()));
     connect(button_return,SIGNAL(clicked()),this,SLOT(display_mass_center()));
     connect(button_ok,SIGNAL(clicked()),subDialog,SLOT(accept()));
     connect(button_cancel,SIGNAL(clicked()),subDialog,SLOT(reject()));
     connect(button_clear,SIGNAL(clicked()),this,SLOT(clear_markers()));
     prev_bgthr=spin_bgthr->value();
    // prev_distance=spin_distance->value();
     prev_conviter=spin_conviter->value();
    // prev_percent=spin_percent->value();
}


void shape_dialog::dialoguefinish(int)
{
    if (subDialog->result()==QDialog::Accepted)
      {
        qDebug()<<"Accepted";
        prev_bgthr=spin_bgthr->value();
        //prev_distance=spin_distance->value();
        prev_conviter=spin_conviter->value();
        //prev_percent=spin_percent->value();
    }

    else{
        qDebug()<<"Not accepting";
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
        LList.clear();
        LList_new_center.clear();
        qDebug()<<"file is not empty";
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
    if (!p4DImage){
        QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
        return;
    }

    //reset data
    LList.clear();
    LList_new_center.clear();
    if (image1Dc_in!=0) image1Dc_in=0;

    datasource=2;
    sz_img[0]=p4DImage->getXDim();
    sz_img[1]=p4DImage->getYDim();
    sz_img[2]=p4DImage->getZDim();
    sz_img[3]=p4DImage->getCDim();
    qDebug()<<"sz_img size:"<<sz_img[0]<<sz_img[1]<<sz_img[2]<<sz_img[3];
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

    LList_fetch.clear();
    LandmarkList LList_in = callback->getLandmark(curwin);
    if (LList_in.size()>0){
        for(int i=0; i<LList_in.size(); i++){
            //LList.append(LList_in.at(i));
            LList_fetch.append(LList_in.at(i));
            //LList[i].color.r=196;
            LList_fetch[i].color.r=196;
            //LList[i].color.g=LList[i].color.b=0;
            LList_fetch[i].color.g=LList_fetch[i].color.b=0;
        }
    }

    callback->close3DWindow(curwin);
    callback->setImageName(curwin, NAME_INWIN);
    callback->open3DWindow(curwin);
    if (LList.size()>=0)
    callback->setLandmark(curwin,LList_fetch);
    callback->pushObjectIn3DWindow(curwin);
    callback->updateImageWindow(curwin);

}

void shape_dialog::updateInputWindow()
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

void shape_dialog::clear_markers()
{

    if (LList.size()<=0){
        v3d_msg("No markers were selected.");
        return;
    }
    if (LList.size()>0)
        LList.clear();
    if (LList_new_center.size()>0)
        LList_new_center.clear();
    if (LList_fetch.size()>0)
        LList_fetch.clear();

    edit->clear();
    bool winfound=false;
    bool win_out_found=false;
    v3dhandleList v3dhandleList_current=callback->getImageWindowList();
    for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
    {
        if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_INWIN))
        {
            winfound=true;
        }
        if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_OUTWIN))
        {
            win_out_found=true;
            callback->close3DWindow(v3dhandleList_current[i]);
        }
    }

    if(!winfound){ //open a window if none is open
        v3d_msg("Please reload the image");
        return;
    }
    else{
        v3dhandleList_current=callback->getImageWindowList();
        for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
        {
            if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_INWIN))
            {
                callback->setLandmark(v3dhandleList_current[i],LList);
                callback->pushObjectIn3DWindow(v3dhandleList_current[i]);
            }
        }
    }
}


void shape_dialog::extract()
{
    if (datasource==1)
    {
        LList.clear();
        v3dhandleList v3dhandleList_current=callback->getImageWindowList();
        for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
        {
            if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_INWIN))
            {
                LandmarkList LList_in = callback->getLandmark(v3dhandleList_current[i]);
                for(int i=0; i<LList_in.size(); i++){
                    LList.append(LList_in.at(i));
                    LList[i].color.r=196;
                    LList[i].color.g=LList[i].color.b=0;
                }
                break;
            }
        }
    }

    if(datasource==2){
        LList.clear();
        v3dhandleList v3dhandleList_current=callback->getImageWindowList();
        for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
        {
            if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_INWIN))
            {
                LandmarkList LList_in = callback->getLandmark(v3dhandleList_current[i]);
                if (LList_in.size()==LList_fetch.size()){
                    for(int i=0; i<LList_in.size(); i++){
                        LList.append(LList_fetch.at(i));
                        LList[i].color.r=196;
                        LList[i].color.g=LList[i].color.b=0;
                    }
                }

                else if (LList_in.size()>LList_fetch.size())
                {
                    for(int i=0; i<LList_in.size(); i++){
                        LList.append(LList_in.at(i));
                        LList[i].color.r=196;
                        LList[i].color.g=LList[i].color.b=0;
                    }
                    break;
                }
            }
        }
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
    int convolute_iter=spin_conviter->value();
    //int neighbor_size=spin_distance->value();
    int bg_thr=spin_bgthr->value();
    //int percent=spin_percent->value();
    //double percent_thr=percent*1.0/100.0;
    poss_landmark.clear();
    poss_landmark=landMarkList2poss(LList, sz_img[0], sz_img[0]*sz_img[1]);
    image1Dc_out=memory_allocate_uchar1D(sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3]);
    //memcpy(image1Dc_out,image1Dc_in, sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3]*sizeof(unsigned char));
    memset(image1Dc_out,0,sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3]*sizeof(unsigned char));


    V3DLONG sumrsize=0;
    for (int j=0;j<poss_landmark.size();j++){
        qDebug()<<"j:"<<j;
        V3DLONG rsize=shape_ext_obj.extract(x_all, y_all,z_all,poss_landmark[j],
                                       convolute_iter,bg_thr);
        if(rsize<=0) continue;
        mass_center=shape_ext_obj.get_mass_center(x_all,y_all,z_all);
        LocationSimple tmp(mass_center[0]+1,mass_center[1]+1,mass_center[2]+1);
        LList_new_center.append(tmp);

        GetColorRGB(rgb,j);
        V3DLONG finalpos;
        for (int i=0;i<x_all.size();i++)
        {
            finalpos=xyz2pos(x_all[i],y_all[i],z_all[i],sz_img[0],sz_img[0]*sz_img[1]);
//            for (int z=0;z<sz_img[3];z++)
                image1Dc_out[finalpos]=rgb[0];
                image1Dc_out[finalpos+1*sz_img[0]*sz_img[1]*sz_img[2]]=rgb[1];
                image1Dc_out[finalpos+2*sz_img[0]*sz_img[1]*sz_img[2]]=rgb[2];
            sumrsize=rsize+sumrsize;
        }

     }

    qDebug()<<"sumrsize:"<<sumrsize;
    if (sumrsize>0) updateOutputWindow();
    else
    v3d_msg("Nothing were found. Please change marker or adjust parameters.");
}

void shape_dialog::display_mass_center()
{
    if(LList.size()<=0) {
        v3d_msg("No markers were selected.");
        return;
    }
    if(LList_new_center.size()<=0 && LList.size()>0)
    {
        v3d_msg("You need to find the edge first");
        return;
    }
    else
    {
        //Append the new markers to the old marker list and show in blue
        for(int i=0; i<LList_new_center.size(); i++){
            LList.append(LList_new_center.at(i));
            LList[LList.size()-1].color.b=255;
            LList[LList.size()-1].color.r=LList[LList.size()-1].color.g=0;
        }

    //Update the current window of new markers
        updateOutputWindow();
    }
}


void shape_dialog::updateOutputWindow()
{
    //search in open windows
    bool winfound=false;
    v3dhandleList v3dhandleList_current=callback->getImageWindowList();
    for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
    {
        if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_OUTWIN))
        {
            winfound=true;
        }
    }
    if(image1Dc_out != 0){ //image loaded
        //generate a copy and show it
        Image4DSimple image4d;
        V3DLONG size_page = sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];
        unsigned char* image1Dc_input=memory_allocate_uchar1D(size_page);
        memcpy(image1Dc_input, image1Dc_out, size_page*sizeof(unsigned char));
        image4d.setData(image1Dc_input, sz_img[0], sz_img[1], sz_img[2], sz_img[3], V3D_UINT8);
        if(!winfound){ //open a window if none is open
            v3dhandle v3dhandle_main=callback->newImageWindow();
            callback->setImage(v3dhandle_main, &image4d);
            callback->setLandmark(v3dhandle_main, LList);
            callback->setImageName(v3dhandle_main, NAME_OUTWIN);
            callback->updateImageWindow(v3dhandle_main);
            callback->open3DWindow(v3dhandle_main);
            callback->pushObjectIn3DWindow(v3dhandle_main);
        }
        else{
            //update the image
            for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
            {
                if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_OUTWIN))
                {
                    callback->setImage(v3dhandleList_current[i], &image4d);
                    callback->setLandmark(v3dhandleList_current[i], LList);
                    callback->updateImageWindow(v3dhandleList_current[i]);
                    callback->pushImageIn3DWindow(v3dhandleList_current[i]);
                    callback->open3DWindow(v3dhandleList_current[i]);
                    callback->pushObjectIn3DWindow(v3dhandleList_current[i]);
                }
            }
        }
    }
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

void shape_dialog::GetColorRGB(int* rgb, int idx)
{
    idx=idx+1;
    if(idx>=0){
    idx = idx > 0 ? idx % 128 : 128 - abs(idx % 128);
    int colorLib[128][3] = {
            {55, 173, 188},
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
            {39, 16, 2},
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
