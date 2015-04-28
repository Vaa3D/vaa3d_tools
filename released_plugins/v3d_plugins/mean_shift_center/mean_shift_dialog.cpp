#include "mean_shift_dialog.h"

mean_shift_dialog::mean_shift_dialog(V3DPluginCallback2 *cb,int method)
{
    callback=cb;
    image1Dc_in=0;
    pixeltype=V3D_UNKNOWN;
    sz_img[0]=sz_img[1]=sz_img[2]=sz_img[3]=0;
    methodcode=method;
}

LandmarkList mean_shift_dialog::core()
{
    //create an empty list
    LandmarkList emptylist = LandmarkList();

    if (!load_data(callback,image1Dc_in,LList,pixeltype,sz_img,curwin))
        return emptylist;
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
       return(emptylist);
    }

    //set parameter
    int windowradius=15;
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
        windowradius=para_radius->value();
    else
        return(emptylist);

    //start mean-shift
    poss_landmark.clear();
    poss_landmark=landMarkList2poss(LList, sz_img[0], sz_img[0]*sz_img[1]);
    qDebug()<<"methodcode:"<<methodcode;
    for (int j=0;j<poss_landmark.size();j++)
    {
        qDebug()<<"_______j:____________"<<j;
        if (methodcode==2)
        mass_center=mean_shift_obj.mean_shift_with_constraint(poss_landmark[j],windowradius);
        else
        mass_center=mean_shift_obj.mean_shift_center(poss_landmark[j],windowradius);
        LocationSimple tmp(mass_center[0]+1,mass_center[1]+1,mass_center[2]+1);
        if (!LList.at(j).name.empty()) tmp.name=LList.at(j).name;
        LList_new_center.append(tmp);
//        if(fp)
//        fprintf(fp,"%1d %5.1f %5.1f %5.1f\n",j+1,mass_center[0],mass_center[1],mass_center[2]);
    }
    //visualize
    Image4DSimple image4d;
    //V3DLONG size_tmp=sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];
    unsigned char* image1D_input=memory_allocate_uchar1D(size_tmp);
    memcpy(image1D_input, image1Dc_in, size_tmp*sizeof(unsigned char));
    image4d.setData(image1D_input, sz_img[0], sz_img[1], sz_img[2], sz_img[3], V3D_UINT8);

    v3dhandle v3dhandle_main=callback->newImageWindow();
    callback->setImage(v3dhandle_main, &image4d);
    callback->setLandmark(v3dhandle_main,LList_new_center);
    if (methodcode==2)
        callback->setImageName(v3dhandle_main, "mean_shift_constraints_"+callback->getImageName(curwin));
    else
        callback->setImageName(v3dhandle_main, "mean_shift_"+callback->getImageName(curwin));
    callback->updateImageWindow(v3dhandle_main);
    callback->open3DWindow(v3dhandle_main);
    callback->pushObjectIn3DWindow(v3dhandle_main);
    return LList_new_center;

}

//void mean_shift_dialog::create_sphere()
//{
//    unsigned char * intensity=new unsigned char[100*200*100];
//    memset(intensity,0,100*200*100*sizeof(unsigned char));
//    int x,y,z;
//    x=y=z=50;
//    for (int dz=z-10; dz<z+36;dz++){
//        for (int dy=y-15;dy<y+21;dy++){
//            for (int dx=x-4; dx<x+30;dx++){
//                float dis_tmp=(z-dz)*(z-dz)+(y-dy)*(y-dy)+(x-dx)*(x-dx);
//                float dis=sqrt(dis_tmp);
//                if (dis>20) continue;
//                intensity[dx+100*dy+200*100*dz]=255-dis/20*255;
//            }
//        }
//    }
//  Image4DSimple image4d;
//  v3dhandle v3dhandle_main=callback->newImageWindow();
//  image4d.setData(intensity,100,200,100,1,V3D_UINT8);
//  callback->setImage(v3dhandle_main, &image4d);
//  callback->open3DWindow(v3dhandle_main);
//}




