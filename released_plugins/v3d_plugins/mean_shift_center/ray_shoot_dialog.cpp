#include "ray_shoot_dialog.h"

ray_shoot_dialog::ray_shoot_dialog(V3DPluginCallback2 *cb)
{
    callback=cb;
    image1Dc_in=0;
    pixeltype=V3D_UNKNOWN;
    sz_img[0]=sz_img[1]=sz_img[2]=sz_img[3]=0;
}


void ray_shoot_dialog::core()
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
    //set parameter
    int bg_thr=70;  
    QDialog *mydialog_para=new QDialog;
    QLabel *label_info=new QLabel;
    label_info->setText("Please set the background threshold (0-255)");
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
        bg_thr=para_bg->value();
    else return;

    //start ray_shoot
    poss_landmark.clear();
    poss_landmark=landMarkList2poss(LList, sz_img[0], sz_img[0]*sz_img[1]);
    for (int j=0;j<poss_landmark.size();j++)
    {
        qDebug()<<"_______j:____________"<<j;
        mass_center=mean_shift_obj.ray_shoot_center(poss_landmark[j],bg_thr,j);
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
    callback->setImageName(v3dhandle_main, "ray_shoot_"+callback->getImageName(curwin));
    callback->updateImageWindow(v3dhandle_main);
    callback->open3DWindow(v3dhandle_main);
    callback->pushObjectIn3DWindow(v3dhandle_main);

}
