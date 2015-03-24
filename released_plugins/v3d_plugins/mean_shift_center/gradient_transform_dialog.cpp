#include "gradient_transform_dialog.h"

gradient_transform_dialog::gradient_transform_dialog(V3DPluginCallback2 *cb)
{
    callback=cb;
    image1Dc_in=0;
    pixeltype=V3D_UNKNOWN;
    sz_img[0]=sz_img[1]=sz_img[2]=sz_img[3]=0;
}

void gradient_transform_dialog::core()
{
    if (!load_data(callback,image1Dc_in,LList,pixeltype,sz_img,curwin))
        return;
    V3DLONG size_tmp=sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];

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

    int min_dim=MIN(sz_img[0],sz_img[1]);
    if (sz_img[2]<min_dim) min_dim=sz_img[2];

    //set parameter
    int bg_thr=70;
    int connectiontype=1;
    int z_thickness=1;
    int transform_half_window=MIN(40,min_dim/2);
    int search_window_radius=15;
    QDialog *mydialog_para=new QDialog;
    QGridLayout *layout2=new QGridLayout;

    QLabel *label_info=new QLabel;
    label_info->setText("Please set the following parameters:");
    QLabel *label_bg=new QLabel;
    label_bg->setText("Background threshold (0-255):");
    QSpinBox *para_bg=new QSpinBox;
    para_bg->setRange(0,255);
    para_bg->setValue(70);

    QLabel *label_cnn=new QLabel;
    label_cnn->setText("Connection type (1-3):");
    QSpinBox *para_cnn=new QSpinBox;
    para_cnn->setRange(1,3);
    para_cnn->setValue(2);

    QLabel *label_z=new QLabel;
    label_z->setText("z_thickness (1-10):");
    QSpinBox *para_z=new QSpinBox;
    para_z->setRange(1,10);
    para_z->setValue(1);

    QLabel *label_tran_win=new QLabel;
    label_tran_win->setText("Gradient distance transform half window size:");
    QSpinBox *para_tran_win=new QSpinBox;
    para_tran_win->setRange(10,min_dim/2);
    if (min_dim/2>40)
    para_tran_win->setValue(40);
    else para_tran_win->setValue(min_dim/2);

    QLabel *label_search_r=new QLabel;
    label_search_r->setText("Mean shift search window radius (2-30):");
    QSpinBox *para_search_r=new QSpinBox;
    para_search_r->setRange(2,30);
    para_search_r->setValue(15);

    layout2->addWidget(label_info,0,0,1,2);
    layout2->addWidget(label_bg,1,0,1,1);
    layout2->addWidget(para_bg,1,1,1,1);
    layout2->addWidget(label_cnn,2,0,1,1);
    layout2->addWidget(para_cnn,2,1,1,1);
    layout2->addWidget(label_z,3,0,1,1);
    layout2->addWidget(para_z,3,1,1,1);
    layout2->addWidget(label_tran_win,4,0,1,1);
    layout2->addWidget(para_tran_win,4,1,1,1);
    layout2->addWidget(label_search_r,5,0,1,1);
    layout2->addWidget(para_search_r,5,1,1,1);

    QPushButton *button_p_ok=new QPushButton;
    button_p_ok->setText("Ok");
    button_p_ok->setFixedWidth(100);
    QPushButton *button_p_cancel=new QPushButton;
    button_p_cancel->setText("Cancel");
    button_p_cancel->setFixedWidth(100);
    layout2->addWidget(button_p_ok,6,0,1,1);
    layout2->addWidget(button_p_cancel,6,1,1,1);
    connect(button_p_ok,SIGNAL(clicked()),mydialog_para,SLOT(accept()));
    connect(button_p_cancel,SIGNAL(clicked()),mydialog_para,SLOT(reject()));
    mydialog_para->setLayout(layout2);
    mydialog_para->exec();
    if (mydialog_para->result()==QDialog::Accepted)
    {
        bg_thr=para_bg->value();
        connectiontype=para_cnn->value();
        z_thickness=para_z->value();
        transform_half_window=para_tran_win->value();
        search_window_radius=para_search_r->value();
    }
    else return;

    //start gradient transform
    float *outimg1d=0;

    LList_new_center.clear();
    poss_landmark.clear();
    poss_landmark=landMarkList2poss(LList, sz_img[0], sz_img[0]*sz_img[1]);
    for (int j=0;j<poss_landmark.size();j++)
    {
        qDebug()<<"_______j:____________"<<j;
        mass_center=mean_shift_obj.gradient_transform(outimg1d,poss_landmark[j],bg_thr,connectiontype,
                                                z_thickness,transform_half_window,search_window_radius);
        LocationSimple tmp(mass_center[0]+1.5,mass_center[1]+1.5,mass_center[2]+1.5);
        LList_new_center.append(tmp);
    }
    if (outimg1d!=0) {delete outimg1d;outimg1d=0;}

    //visualize
    //V3DLONG size_page=sz_img[0]*sz_img[1]*sz_img[2];
    Image4DSimple image4d;
    unsigned char* image1D_input=memory_allocate_uchar1D(sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3]);
    memcpy(image1D_input, image1Dc_in, sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3]*sizeof(unsigned char));
//    for (int sid=0;sid<sz_img[0]*sz_img[1]*sz_img[2];sid++)
//    {
//        image1D_input[sid]=(unsigned char)outimg1d[sid];
//    }
    //image4d.setData(image1D_input, sz_img[0],sz_img[1],sz_img[2],sz_img[3], V3D_UINT8);
    image4d.setData(image1D_input, sz_img[0],sz_img[1],sz_img[2],sz_img[3], V3D_UINT8);
    v3dhandle v3dhandle_main=callback->newImageWindow();
    callback->setImage(v3dhandle_main, &image4d);
    callback->setLandmark(v3dhandle_main,LList_new_center);
    callback->setImageName(v3dhandle_main, "gradient_dist_trans"+callback->getImageName(curwin));
    callback->updateImageWindow(v3dhandle_main);
    callback->open3DWindow(v3dhandle_main);
    callback->pushObjectIn3DWindow(v3dhandle_main);

}
