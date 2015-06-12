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
    sz_img[0]=sz_img[1]=sz_img[2]=sz_img[3]=0;
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
    LList.clear();
    for(int i=0; i<LList_in.size(); i++){
        LList.append(LList_in.at(i));
        LList[i].color.r=196;
        LList[i].color.g=LList[i].color.b=0;
    }
    extract();
}

void shape_dialog::extract()
{
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
    label=memory_allocate_uchar1D(size_page);
    memset(label,0,size_page*sizeof(unsigned char));

    V3DLONG sumrsize=0;
    //volume.clear();
    for (int j=0;j<poss_landmark.size();j++){
        qDebug()<<"j:"<<j;
        if (label[j]>0) continue;
        V3DLONG rsize=shape_ext_obj.extract(x_all, y_all,z_all,label,poss_landmark[j],
                                       convolute_iter,bg_thr,j+1);
        if(rsize<=1) continue;

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

        v3dhandle v3dhandle_color=callback->newImageWindow();
        callback->setImage(v3dhandle_color, &image4d);
        callback->setLandmark(v3dhandle_color, LList);
        callback->setImageName(v3dhandle_color, "extractor_index_"+QString(callback->getImageName(curwin)));
        callback->updateImageWindow(v3dhandle_color);

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
