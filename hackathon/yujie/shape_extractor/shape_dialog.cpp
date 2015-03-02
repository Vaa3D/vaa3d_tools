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

#ifdef _MSC_VER
inline long lroundf(float num) { return static_cast<long>(num > 0 ? num + 0.5f : ceilf(num - 0.5f)); }
#endif

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
    QToolButton *button_volume=new QToolButton;
    button_volume->setText("Get volume");
    button_volume->setGeometry(0,0,10,20);
    QToolButton *button_para=new QToolButton;
    button_para->setText("Parameter setting");
    button_para->setGeometry(0,0,10,20);
//    QToolButton *button_clear=new QToolButton;
//    button_clear->setText("Clear markers");
//    button_clear->setGeometry(0,0,10,20);
    QToolButton *button_swc=new QToolButton;
    button_swc->setText("Load swc file");
    button_swc->setGeometry(0,0,10,20);

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
    tool->addWidget(button_volume);
    tool->addSeparator();
//    tool->addWidget(button_clear);
//    tool->addSeparator();
    tool->addWidget(button_para);
    tool->addSeparator();
    tool->addWidget(button_swc);

    QHBoxLayout *vlayout = new QHBoxLayout;
    vlayout->addWidget(tool);

    QLabel *label= new QLabel;
    label->setText("Selected markers:");
    label->setMaximumHeight(50);

    edit=new QPlainTextEdit();
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
     //connect(button_clear,SIGNAL(clicked()),this,SLOT(clear_markers()));
     connect(button_volume, SIGNAL(clicked()), this, SLOT(display_volume()));
     connect(button_swc, SIGNAL(clicked()), this, SLOT(load_swc_filter()));
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

    resetdata();

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


    callback->close3DWindow(curwin);
    callback->setImageName(curwin, NAME_INWIN);
    callback->open3DWindow(curwin);
    callback->pushObjectIn3DWindow(curwin);
    callback->updateImageWindow(curwin);

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
    qDebug()<<"end of resetdata";
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

//void shape_dialog::clear_markers()
//{

//    if (LList.size()<=0){
//        v3d_msg("No markers were selected.");
//        return;
//    }
//    if (LList.size()>0)
//        LList.clear();
//    if (LList_new_center.size()>0)
//        LList_new_center.clear();
//    if (LList_fetch.size()>0)
//        LList_fetch.clear();

//    edit->clear();
//    bool winfound=false;
//    bool win_out_found=false;
//    v3dhandleList v3dhandleList_current=callback->getImageWindowList();
//    for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
//    {
//        if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_INWIN))
//        {
//            winfound=true;
//        }
//        if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_OUTWIN))
//        {
//            win_out_found=true;
//            callback->close3DWindow(v3dhandleList_current[i]);
//        }
//    }

//    if(!winfound){ //open a window if none is open
//        v3d_msg("Please reload the image");
//        return;
//    }
//    else{
//        v3dhandleList_current=callback->getImageWindowList();
//        for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
//        {
//            if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_INWIN))
//            {
//                callback->setLandmark(v3dhandleList_current[i],LList);
//                callback->pushObjectIn3DWindow(v3dhandleList_current[i]);
//            }
//        }
//    }
//}


void shape_dialog::extract()
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

    qDebug()<<"Properly displayed";
    V3DLONG size_page = sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];
    int convolute_iter=spin_conviter->value();
    int bg_thr=spin_bgthr->value();

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

    if (shape_ext_obj.mask1D!=0)
    {
       memory_free_uchar1D(shape_ext_obj.mask1D);
    }
    shape_ext_obj.mask1D=memory_allocate_uchar1D(size_page*sizeof(unsigned char));
    memset(shape_ext_obj.mask1D,0,size_page*sizeof(unsigned char));

    qDebug()<<"Label,image1Dc_out ready";
    V3DLONG sumrsize=0;
    volume.clear();
    for (int j=0;j<poss_landmark.size();j++){
        qDebug()<<"j:"<<j;

        V3DLONG rsize=shape_ext_obj.extract(x_all, y_all,z_all,label,poss_landmark[j],
                                       convolute_iter,bg_thr,j+1);
        if(rsize<=0) continue;
        mass_center=shape_ext_obj.get_mass_center(x_all,y_all,z_all);
        LocationSimple tmp(mass_center[0]+1,mass_center[1]+1,mass_center[2]+1);
        LList_new_center.append(tmp);

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

     }

    qDebug()<<"sumrsize:"<<sumrsize;

    if (sumrsize>0) updateOutputWindow();
    else
    v3d_msg("Nothing were found. Please change marker or adjust parameters.");
}

void shape_dialog::load_swc_filter()
{
//    QString filename = QFileDialog::getOpenFileName(0, QObject::tr("Choose the input image "),
//             QDir::currentPath(),QObject::tr("Neuron structure(*.swc)"));


//    //QString filename = QFileDialog::getOpenFileName(0, "","Supported file (*.swc)" ";;Neuron structure(*.swc)",0,0);
//    if(filename.isEmpty())
//    {
//        v3d_msg("You don't have any SWC file open in the main window.");
//        return;
//    }
//    nt = new NeuronTree();
//    if (filename.toUpper().endsWith(".SWC") || filename.toUpper().endsWith(".ESWC"))
//    {
//        *nt = readSWC_file(filename);
//    }
//    int count=0;

//    for (int i=0; i<nt->listNeuron.size();i++)
//    {
//        float x=nt->listNeuron[i].x;
//        float y=nt->listNeuron[i].y;
//        float z=nt->listNeuron[i].z;
//        float r=nt->listNeuron[i].r;
//        for (float dx=x-r;dx<x+r;dx++){
//            for (float dy=y-r;dy<y+r;dy++){
//                for (float dz=z-r;dz<z+r;dz++){
//                    float tmp=(dx-x)*(dx-x)+(dy-y)*(dy-y)+(dz-z)*(dz-z);
//                    float distance=sqrt(tmp);
//                    if (distance>r) continue;
//                    V3DLONG tmp_pos=xyz2pos(dx-1,dy-1,dz-1,sz_img[0],sz_img[0]*sz_img[1]);
//                    shape_ext_obj.mask1D[tmp_pos]=255;
//                    count++;
//                }
//            }
//        }

//    }
//    qDebug()<<"ListNeurons:"<<nt->listNeuron.size();
//    qDebug()<<"This many voxels have been masked:"<<count;

    swc_to_maskimage(*callback,1 );
//    Image4DSimple image4d;
//    image4d.setData(shape_ext_obj.mask1D,sz_img[0], sz_img[1], sz_img[2], 1, V3D_UINT8);

//        v3dhandle v3dhandle_main=callback->newImageWindow();
//        callback->setImage(v3dhandle_main, &image4d);
//        callback->setImageName(v3dhandle_main, "swc");
//        callback->updateImageWindow(v3dhandle_main);
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
    qDebug()<<"In updateoutputwindow";
    bool winfound=false;
    v3dhandleList v3dhandleList_current=callback->getImageWindowList();
    for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
    {
        if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_OUTWIN))
        {
            winfound=true;
        }
        break;
    }
    if(image1Dc_out != 0){ //image loaded
        //generate a copy and show it
        Image4DSimple image4d,image4d_color;

        V3DLONG size_page=sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];
        unsigned char* image1Dc_input=memory_allocate_uchar1D(size_page);
        memcpy(image1Dc_input, image1Dc_out, size_page*sizeof(unsigned char));
        //image4d.setData(image1Dc_input, sz_img[0], sz_img[1], sz_img[2], sz_img[3], V3D_UINT8);

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
            //update the image
            for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
            {
                if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_OUTWIN))
                {
                    callback->setImage(v3dhandleList_current[i], &image4d);
                    callback->setLandmark(v3dhandleList_current[i], LList);
                    callback->setImageName(v3dhandleList_current[i], NAME_OUTWIN);
                    callback->updateImageWindow(v3dhandleList_current[i]);
                    //callback->pushImageIn3DWindow(v3dhandleList_current[i]);

                    //Open another window for the colorful image for visualization
                    v3dhandle v3dhandle_color=callback->newImageWindow();
                    callback->setImage(v3dhandle_color, &image4d_color);
                    callback->setLandmark(v3dhandle_color, LList);
                    callback->setImageName(v3dhandle_color, "Output_Shape_Extractor_color");
                    callback->open3DWindow(v3dhandle_color);
                    callback->pushObjectIn3DWindow(v3dhandle_color);
                    callback->updateImageWindow(v3dhandle_color);
                }
                break;
            }
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


NeuronTree readSWC_file(const QString& filename)
{
    NeuronTree nt;
    QFile qf(filename);
    if (! qf.open(QIODevice::ReadOnly | QIODevice::Text))
    {
#ifndef DISABLE_V3D_MSG
        v3d_msg(QString("open file [%1] failed!").arg(filename));
#endif
        return nt;
    }

    int count = 0;
    QList <NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();
    QString name = "";
    QString comment = "";

    qDebug("-------------------------------------------------------");
    while (! qf.atEnd())
    {
        char _buf[1000], *buf;
        qf.readLine(_buf, sizeof(_buf));
        for (buf=_buf; (*buf && *buf==' '); buf++); //skip space

        //  add #name, #comment
        if (buf[0]=='\0')	continue;
        if (buf[0]=='#')
        {
            if (buf[1]=='n'&&buf[2]=='a'&&buf[3]=='m'&&buf[4]=='e'&&buf[5]==' ')
                name = buf+6;
            if (buf[1]=='c'&&buf[2]=='o'&&buf[3]=='m'&&buf[4]=='m'&&buf[5]=='e'&&buf[6]=='n'&&buf[7]=='t'&&buf[8]==' ')
                comment = buf+9;

            continue;
        }

        count++;
        NeuronSWC S;

        QStringList qsl = QString(buf).split(" ",QString::SkipEmptyParts);
        if (qsl.size()==0)   continue;

        for (int i=0; i<qsl.size(); i++)
        {
            qsl[i].truncate(99);
            if (i==0) S.n = qsl[i].toInt();
            else if (i==1) S.type = qsl[i].toInt();
            else if (i==2) S.x = qsl[i].toFloat();
            else if (i==3) S.y = qsl[i].toFloat();
            else if (i==4) S.z = qsl[i].toFloat();
            else if (i==5) S.r = qsl[i].toFloat();
            else if (i==6) S.pn = qsl[i].toInt();
            //the ESWC extension, by PHC, 20120217
            else if (i==7) S.seg_id = qsl[i].toInt();
            else if (i==8) S.level = qsl[i].toInt();
            else if (i==9) S.fea_val = qsl[i].toFloat();
            //qDebug()<<"haha 9"<<endl;
        }

        //if (! listNeuron.contains(S)) // 081024
        {
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);
        }
    }
    qDebug("---------------------read %d lines, %d remained lines", count, listNeuron.size());

    if (listNeuron.size()<1)
        return nt;


    //now update other NeuronTree members

    nt.n = 1; //only one neuron if read from a file
    nt.file = QFileInfo(filename).absoluteFilePath();
    nt.listNeuron = listNeuron;
    nt.hashNeuron = hashNeuron;
    nt.color = XYZW(0,0,0,0); /// alpha==0 means using default neuron color, 081115
    nt.on = true;
    nt.name = name.remove('\n'); if (nt.name.isEmpty()) nt.name = QFileInfo(filename).baseName();
    nt.comment = comment.remove('\n');

    return nt;
}

void ComputemaskImage(NeuronTree neurons,
                      unsigned char* pImMask, //output mask image
                      unsigned char* ImMark,  //an indicator image to show whether or not a pixel has been visited/processed
                      V3DLONG sx,V3DLONG sy,V3DLONG sz,int method_code)
{
    qDebug()<<"In computemaskImage";
    NeuronSWC *p_cur = 0;
    //double check the data to ensure it is correct!
    for (V3DLONG ii=0; ii<neurons.listNeuron.size(); ii++)
    {
        p_cur = (NeuronSWC *)(&(neurons.listNeuron.at(ii)));
    //	v3d_msg(QString("x %1 y %2 z %3 r %4\n").arg(p_cur->x).arg(p_cur->y).arg(p_cur->z).arg(p_cur->r),0);

        if (p_cur->x<0 || p_cur->y<0 || p_cur->z<0 || p_cur->r<0)
        {
            v3d_msg("You have illeagal x,y,z coordinates or radius values. Check your data.");
            return;
        }
    }
    //create a LUT
    QHash<V3DLONG, V3DLONG> neuron_id_table = NeuronNextPn(neurons);

    //compute mask
    double xs = 0, ys = 0, zs = 0, xe = 0, ye = 0, ze = 0, rs = 0, re = 0;
    V3DLONG pagesz = sx*sy;
    for (V3DLONG ii=0; ii<neurons.listNeuron.size(); ii++)
    {
        V3DLONG i,j,k;
        p_cur = (NeuronSWC *)(&(neurons.listNeuron.at(ii)));

        xs = p_cur->x;
        ys = p_cur->y;
        zs = p_cur->z;
        rs = p_cur->r;
        if (rs<1)
        {
            //rs=2-rs;
        }
        double ballx0, ballx1, bally0, bally1, ballz0, ballz1, tmpf;

        ballx0 = xs - rs; ballx0 = qBound(double(0), ballx0, double(sx-1));
        ballx1 = xs + rs; ballx1 = qBound(double(0), ballx1, double(sx-1));
        if (ballx0>ballx1) {tmpf = ballx0; ballx0 = ballx1; ballx1 = tmpf;}

        bally0 = ys - rs; bally0 = qBound(double(0), bally0, double(sy-1));
        bally1 = ys + rs; bally1 = qBound(double(0), bally1, double(sy-1));
        if (bally0>bally1) {tmpf = bally0; bally0 = bally1; bally1 = tmpf;}

        ballz0 = zs - rs; ballz0 = qBound(double(0), ballz0, double(sz-1));
        ballz1 = zs + rs; ballz1 = qBound(double(0), ballz1, double(sz-1));
        if (ballz0>ballz1) {tmpf = ballz0; ballz0 = ballz1; ballz1 = tmpf;}

        //marker all voxels close to the swc node(s)

        for (k = ballz0; k <= ballz1; k++)
        {
            for (j = bally0; j <= bally1; j++)
            {
                for (i = ballx0; i <= ballx1; i++)
                {
                    double norms10 = (xs-i)*(xs-i) + (ys-j)*(ys-j) + (zs-k)*(zs-k);
                    double dt = sqrt(norms10);

                    V3DLONG ind = (k)*pagesz + (j)*sx + i;

                    if(dt <=rs || dt<=1)
                    {
//                        if (method_code == 1)
//                        {
                            pImMask[ind] = 255;

//                        }else if (method_code ==2)
//                        {
//                            ImMark[ind] = 1;

//                        }

                    }
                }
            }
        }

        //find previous node
        if (p_cur->pn < 0) //then it is root node already
        {
            continue;
        }

        //get the parent info

        const NeuronSWC & pp  = neurons.listNeuron.at(neuron_id_table.value(p_cur->pn));
        xe = pp.x;
        ye = pp.y;
        ze = pp.z;
        re = pp.r;
        if (re<1)
        {
            //re=2-re;
        }
        //judge if two points overlap, if yes, then do nothing as the sphere has already been drawn
        if (xe==xs && ye==ys && ze==zs)
        {
            v3d_msg(QString("Detect overlapping coordinates of node [%1]\n").arg(p_cur->n), 0);
            continue;
        }

//		//only set the current point's value in the mask image
//		pImMask[V3DLONG(zs)*sx*sy + V3DLONG(ys)*sx + V3DLONG(xs)] = random()%250 + 1;
//		continue;

        double l =sqrt((xe-xs)*(xe-xs)+(ye-ys)*(ye-ys)+(ze-zs)*(ze-zs));

        V3DLONG xn,yn,zn;
        double dx = (xe - xs);
        double dy = (ye - ys);
        double dz = (ze - zs);
        double x = xs;
        double y = ys;
        double z = zs;

        int steps = lroundf(l);

        steps = (steps < fabs(dx))? fabs(dx):steps;
        steps = (steps < fabs(dy))? fabs(dy):steps;
        steps = (steps < fabs(dz))? fabs(dz):steps;
        if (steps<1)
        {
            steps =1;
        }
    //	printf("steps=%ld\n",steps);
        qDebug()<<"jade: steps:"<<steps;
        double xIncrement = double(dx) / (steps*2);
        double yIncrement = double(dy) / (steps*2);
        double zIncrement = double(dz) / (steps*2);
         qDebug()<<"jade: before lround";

        V3DLONG idex1=lroundf(z)*sx*sy + lroundf(y)*sx + lroundf(x);
        qDebug()<<"jade: after lround"<<"idex1:"<<idex1;
//        if (method_code == 1)
//        {
            pImMask[idex1] = 255;
//        }

        qDebug()<<"jade:Pimmask";
//        else if (method_code ==2)
//        {
//            ImMark[idex1] = 1;
//        }
        for (int i = 0; i <= steps; i++)
        {
            x += xIncrement;
            y += yIncrement;
            z += zIncrement;

            x = ( x > sx )? sx : x;
            y = ( y > sy )? sy : y;
            z = ( z > sz )? sz : z;

            V3DLONG idex=lroundf(z)*sx*sy + lroundf(y)*sx + lroundf(x);

//            if (method_code == 1)
//            {
                pImMask[idex] = 255;
//            }
//            else if (method_code ==2)
//            {
//                ImMark[idex] = 1;
//            }

        }

        //finding the envelope of the current line segment
        qDebug()<<"jade:find the evelop";
        double rbox = (rs>re) ? rs : re;
        double x_down = (xs < xe) ? xs : xe; x_down -= rbox; x_down = V3DLONG(x_down); if (x_down<0) x_down=0; if (x_down>=sx-1) x_down = sx-1;
        double x_top  = (xs > xe) ? xs : xe; x_top  += rbox; x_top  = V3DLONG(x_top ); if (x_top<0)  x_top=0;  if (x_top>=sx-1)  x_top  = sx-1;
        double y_down = (ys < ye) ? ys : ye; y_down -= rbox; y_down = V3DLONG(y_down); if (y_down<0) y_down=0; if (y_down>=sy-1) y_down = sy-1;
        double y_top  = (ys > ye) ? ys : ye; y_top  += rbox; y_top  = V3DLONG(y_top ); if (y_top<0)  y_top=0;  if (y_top>=sy-1)  y_top = sy-1;
        double z_down = (zs < ze) ? zs : ze; z_down -= rbox; z_down = V3DLONG(z_down); if (z_down<0) z_down=0; if (z_down>=sz-1) z_down = sz-1;
        double z_top  = (zs > ze) ? zs : ze; z_top  += rbox; z_top  = V3DLONG(z_top ); if (z_top<0)  z_top=0;  if (z_top>=sz-1)  z_top = sz-1;

        //compute cylinder and flag mask

        for (k=z_down; k<=z_top; k++)
        {
            for (j=y_down; j<=y_top; j++)
            {
                for (i=x_down; i<=x_top; i++)
                {
                    double rr = 0;
                    double countxsi = (xs-i);
                    double countysj = (ys-j);
                    double countzsk = (zs-k);
                    double countxes = (xe-xs);
                    double countyes = (ye-ys);
                    double countzes = (ze-zs);
                    double norms10 = countxsi * countxsi + countysj * countysj + countzsk * countzsk;
                    double norms21 = countxes * countxes + countyes * countyes + countzes * countzes;
                    double dots1021 = countxsi * countxes + countysj * countyes + countzsk * countzes;
                    double dist = sqrt( norms10 - (dots1021*dots1021)/(norms21) );
                    double t1 = -dots1021/norms21;
                //	printf("t1=%lf\n",t1);
                    if(t1<0)
                        dist = sqrt(norms10);
                    else if(t1>1)
                        dist = sqrt((xe-i)*(xe-i) + (ye-j)*(ye-j) + (ze-k)*(ze-k));
                    //compute rr
                    if (rs==re)
                    {
                        rr =rs;

                    }else
                    {
                        // compute point of intersection
                        double v1 = xe - xs;
                        double v2 = ye - ys;
                        double v3 = ze - zs;
                        double vpt = v1*v1 + v2*v2 +v3*v3;
                        double t = (double(i-xs)*v1 +double(j-ys)*v2 + double(k-zs)*v3)/vpt;
                        double xc = xs + v1*t;
                        double yc = ys + v2*t;
                        double zc = zs + v3*t;
                        double normssc = sqrt((xs-xc)*(xs-xc)+(ys-yc)*(ys-yc)+(zs-zc)*(zs-zc));
                        double normsce = sqrt((xe-xc)*(xe-xc)+(ye-yc)*(ye-yc)+(ze-zc)*(ze-zc));
                        rr = (rs >= re) ? (rs - ((rs - re)/sqrt(norms21))*normssc) : (re - ((re-rs)/sqrt(norms21))*normsce);
                    }
                    V3DLONG ind1 = (k)*sx*sy + (j)*sx + i;
                    //printf("rr=%lf dist=%lf \n",rr,dist);
                    if (dist <= rr || dist<=1)
                    {
//                        if (method_code == 1)
//                        {
                            pImMask[ind1] = 255;
//                        }
//                        else if (method_code ==2)
//                        {
//                            ImMark[ind1] = 1;
//                        }
                    }
                }
            }
        }
    }
    qDebug()<<"jade:the end";
//    if (method_code == 2)
//    {
//        for (V3DLONG k = 0; k < sz; k++)
//        {
//            for(V3DLONG j = 0; j < sy; j++)
//            {
//                for(V3DLONG i = 0; i < sx; i ++)
//                {
//                    pImMask[k*sx*sy + j*sx +i] += ImMark[k*sx*sy + j*sx +i];

//                }
//            }
//        }
//    }


}


void shape_dialog::swc_to_maskimage(V3DPluginCallback2 &callback, int method_code)
{
    NeuronTree neuron;
    double x_min,x_max,y_min,y_max,z_min,z_max;
    x_min=x_max=y_min=y_max=z_min=z_max=0;
    V3DLONG sx,sy,sz;
    unsigned char* pImMask = 0;
    unsigned char* ImMark = 0;
    QString filename;
    V3DLONG h;
    V3DLONG d;
    V3DLONG nx,ny,nz;
    if (method_code == 1)
    {
        filename = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
                                                "",QObject::tr("Supported file (*.swc)"
                                                            ";;Neuron structure	(*.swc)"));
        if(filename.isEmpty())
        {
            v3d_msg("You don't have any SWC file open in the main window.");
            return;
        }

        NeuronSWC *p_t=0;
        if (filename.size()>0)
        {
            neuron = readSWC_file(filename);
            qDebug()<<"1";
            bool b_subtractMinFromAllNonnegatives = false;
            x_max=sz_img[0]-1; y_max=sz_img[1]-1; z_max=sz_img[2]-1;

//            for (V3DLONG ii=0; ii<neuron.listNeuron.size(); ii++)
//            {
//                p_t = (NeuronSWC *)(&(neuron.listNeuron.at(ii)));
//                p_t->x=(p_t->x < 0)?(p_t->x - x_min):p_t->x;
//                p_t->y=(p_t->y < 0)?(p_t->y - y_min):p_t->y;
//                p_t->z=(p_t->z < 0)?(p_t->z - z_min):p_t->z;

//              //v3d_msg(QString("x %1 y %2 z %3 r %4\n").arg(p_cur->x).arg(p_cur->y).arg(p_cur->z).arg(p_cur->r),0);
//            }
            sx = sz_img[0]; // = (b_subtractMinFromAllNonnegatives || x_min<0) ? V3DLONG(ceil(x_max - x_min + 1)) : V3DLONG(ceil(x_max + 1));
            sy = sz_img[1]; //(b_subtractMinFromAllNonnegatives || y_min<0) ? V3DLONG(ceil(y_max - y_min + 1)) : V3DLONG(ceil(y_max + 1));
            sz = sz_img[2]; //(b_subtractMinFromAllNonnegatives || z_min<0) ? V3DLONG(ceil(z_max - z_min + 1)) : V3DLONG(ceil(z_max + 1));
            qDebug()<<"cojoc:"<<sx<<":"<<sy<<":"<<sz;
            V3DLONG stacksz = sx*sy*sz;
            try
            {
                pImMask = new unsigned char [stacksz];
                ImMark = new unsigned char [stacksz];
            }
            catch (...)
            {
                v3d_msg("Fail to allocate memory.\n");
                return;
            }

            for (V3DLONG i=0; i<stacksz; i++)
                    pImMask[i] = ImMark[i] = 0;

            ComputemaskImage(neuron, pImMask, ImMark, sx, sy, sz,1);
        }
        else
        {
            v3d_msg("You don't have any SWC file open in the main window.");
            return;
        }

    }
    qDebug()<<"pImMask"<<"sx,sy,sz "<<":"<<sx<<":"<<sy<<":"<<sz;
    Image4DSimple tmp;
    tmp.setData(pImMask, sx, sy, sz, 1, V3D_UINT8);
    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, &tmp);
    qDebug()<<"3";
    callback.setImageName(newwin, QString("mask"));
    callback.updateImageWindow(newwin);

}

QHash<V3DLONG, V3DLONG> NeuronNextPn(const NeuronTree &neurons)
{
    QHash<V3DLONG, V3DLONG> neuron_id_table;
    for (V3DLONG i=0;i<neurons.listNeuron.size(); i++)
        neuron_id_table.insert(V3DLONG(neurons.listNeuron.at(i).n), i);
    return neuron_id_table;
}
