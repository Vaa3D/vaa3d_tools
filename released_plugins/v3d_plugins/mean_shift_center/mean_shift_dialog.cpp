#include "mean_shift_dialog.h"

mean_shift_dialog::mean_shift_dialog(V3DPluginCallback2 *cb)
{
    callback=cb;
    image1Dc_in=0;
}

void mean_shift_dialog::core()
{
    v3dhandleList v3dhandleList_current=callback->getImageWindowList();
    QList <V3dR_MainWindow *> cur_list_3dviewer = callback->getListAll3DViewers();

    if (v3dhandleList_current.size()==0){
        v3d_msg("Please open image and select markers");
        return;
    }
    else if (v3dhandleList_current.size()==1)  //One window open
    {
        //get markers and check markers
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
    label_info->setText("Please set the search window radius (2-30)");
    QLabel *label_radius=new QLabel;
    label_radius->setText("Search window radius:");
    QGridLayout *layout2=new QGridLayout;
    QSpinBox *para_radius=new QSpinBox;
    para_radius->setRange(2,30);
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
    Image4DSimple image4d;
    unsigned char* image1D_input=memory_allocate_uchar1D(size_tmp);
    memcpy(image1D_input, image1Dc_in, size_tmp*sizeof(unsigned char));
    image4d.setData(image1D_input, sz_img[0], sz_img[1], sz_img[2], sz_img[3], V3D_UINT8);

    v3dhandle v3dhandle_main=callback->newImageWindow();
    callback->setImage(v3dhandle_main, &image4d);
    callback->setLandmark(v3dhandle_main,LList_new_center);
    callback->setImageName(v3dhandle_main, "mean_shift_"+callback->getImageName(curwin));
    callback->updateImageWindow(v3dhandle_main);
    callback->open3DWindow(v3dhandle_main);
    callback->pushObjectIn3DWindow(v3dhandle_main);

}

void mean_shift_dialog::create_sphere()
{
    unsigned char * intensity=new unsigned char[100*200*100];
    memset(intensity,0,100*200*100*sizeof(unsigned char));
    int x,y,z;
    x=y=z=50;
    for (int dz=z-10; dz<z+36;dz++){
        for (int dy=y-15;dy<y+21;dy++){
            for (int dx=x-4; dx<x+30;dx++){
                float dis_tmp=(z-dz)*(z-dz)+(y-dy)*(y-dy)+(x-dx)*(x-dx);
                float dis=sqrt(dis_tmp);
                if (dis>20) continue;
                intensity[dx+100*dy+200*100*dz]=255-dis/20*255;
            }
        }
    }
  Image4DSimple image4d;
  v3dhandle v3dhandle_main=callback->newImageWindow();
  image4d.setData(intensity,100,200,100,1,V3D_UINT8);
  callback->setImage(v3dhandle_main, &image4d);
  callback->open3DWindow(v3dhandle_main);

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


