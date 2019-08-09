#include "tipdetector.h"
#include<iostream>
#define PI 3.1415926
//two class one forinput para

using namespace std;

TipDetector::TipDetector()
{
     datald=0;
     number_of_rays = 64;
     background_threshold = 40;
     magnitude =5.0;
     minimum_scale=4;
     number_of_scales=4;
     step_size=2;
     intensity_threshold=115;
     angle_threshold=68;

}
TipDetector::~TipDetector()
{
    if(block) {delete []block; block = 0;}
    if(xy_mip_datald) {delete []xy_mip_datald; xy_mip_datald = 0;}
    if(xz_mip_datald) {delete []xz_mip_datald; xz_mip_datald = 0;}
    if(yz_mip_datald){delete []yz_mip_datald;yz_mip_datald=0;}
    if(enlarged_datald){delete []enlarged_datald;enlarged_datald=0;}
}
LandmarkList TipDetector::OutputTipPoint()
{
    return curlist;
}
bool TipDetector::TipDetect_onePoint(LocationSimple CandiPoint)
{
    int xy_flag=0;
    int xz_flag=0;
    int yz_flag=0;
    int sum_flag=0;
    V3DLONG k=CandiPoint.x;
    V3DLONG i=CandiPoint.y;
    V3DLONG j=CandiPoint.z;

    unsigned char pixe=binar_datald[j*new_nx*new_ny+i*new_nx+k];
    V3DLONG num_block=0;
    if(pixe>background_threshold)
    {
        for(V3DLONG a=j-radiu_block;a<=j+radiu_block;a++)
        {
             V3DLONG z_location=a*new_nx*new_ny;
             for(V3DLONG b=i-radiu_block;b<=i+radiu_block;b++)
             {
                 V3DLONG y_location=b*new_nx;
                 for(V3DLONG c=k-radiu_block;c<=k+radiu_block;c++)
                 {
                     unsigned char block_pixe=binar_datald[z_location+y_location+c];
                       block[num_block]=block_pixe;
                       num_block++;

                 }
             }
        }

        XY_mip(length_block,length_block,length_block,block,xy_mip_datald);
        XZ_mip(length_block,length_block,length_block,block,xz_mip_datald);
        YZ_mip(length_block,length_block,length_block,block,yz_mip_datald);

        xy_flag=rayinten_2D(radiu_block,radiu_block,number_of_rays,minimum_scale,ray_x,ray_y,length_block,length_block,xy_mip_datald,intensity_threshold,angle_threshold);
        xz_flag=rayinten_2D(radiu_block,radiu_block,number_of_rays,minimum_scale,ray_x,ray_y,length_block,length_block,xz_mip_datald,intensity_threshold,angle_threshold);
        yz_flag=rayinten_2D(radiu_block,radiu_block,number_of_rays,minimum_scale,ray_x,ray_y,length_block,length_block,yz_mip_datald,intensity_threshold,angle_threshold);

        sum_flag=xy_flag+xz_flag+yz_flag;
        if(sum_flag>=2)
        {
            return true;
        }
    }
    return false;

}
void TipDetector::whole_img()
{
    //before it
    for(V3DLONG j=radiu_block+1;j<new_nz-radiu_block-1;j++)
    {
         for(V3DLONG i=radiu_block+1;i<new_ny-radiu_block-1;i++)
         {
             for(V3DLONG k=radiu_block+1;k<new_nx-radiu_block-1;k++)
             {
                 LocationSimple candypoint;
                 candypoint.x=k;
                 candypoint.y=i;
                 candypoint.z=j;
                 bool isTip;
                 isTip=this->TipDetect_onePoint(candypoint);
                  //test TipDetect_onePoint function


                 if(isTip)
                 {
                     s.x=k;
                     s.y=i;
                     s.z=j;
                     s.color=random_rgba8(255);
                     s.size=1;
                     curlist<<s;
                 }

             }
         }
    }


//    // change the clustering algorithm by chaowang 2018.11.16
    for(double i=0;i<curlist.size();i++)
    {
        for(double j=i+1;j<curlist.size();j++)
        {
            if(square(curlist[j].x-curlist[i].x)+square(curlist[j].y-curlist[i].y)+square(curlist[j].z-curlist[i].z)<60)
            {
                curlist[i].x=(curlist[j].x+curlist[i].x)/2;
                curlist[i].y=(curlist[j].y+curlist[i].y)/2;
                curlist[i].z=(curlist[j].z+curlist[i].z)/2;
                curlist.removeAt(j);
                j=j-1;
            }
        }
    }
// // display output
    if(datald){delete []datald;datald=0;}
    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *)binar_datald, new_nx, new_ny, new_nz, p4DImage->getCDim(), p4DImage->getDatatype());
    v3dhandle newwin = callback->newImageWindow();
    callback->setImage(newwin, new4DImage);
    callback->updateImageWindow(newwin);
    callback->setLandmark(newwin, curlist);

    return ;
}
float TipDetector::getPluginVersion()
{
    return 1.1f;
}
void TipDetector::help()
{
    cout<<"\nThis is a demo plugin to detect tip point in an image. by Keran Lin 2017-04"<<endl;
    cout<<"\nUsage: v3d -x <example_plugin_name> -f tip_detection -i <input_image_file> -o <output_image_file> -p <subject_color_channel> <threshold>"<<endl;
    cout<<"\t -i <input_image_file>                      input 3D image (tif, raw or lsm)"<<endl;
    cout<<"\t -o <output_image_file>                     output image of the thresholded subject channel"<<endl;
    cout<<"\t                                            the  paras must come in this order"<<endl;
    cout<<"\nDemo: v3d -x libexample_debug.dylib -f image_thresholding -i input.tif -o output.tif -p 0 100\n"<<endl;
    return;
}
void TipDetector::GUI_input_argu()
{
    if(p4DImage==NULL)
    {
        v3d_msg("please input img data first");
        return;
    }
    //set update the dialog
    QDialog * dialog = new QDialog();

    if(p4DImage->getZDim() > 1 )
        dialog->setWindowTitle("3D neuron image tip point detection Based on Ray-shooting algorithm");
    else
        dialog->setWindowTitle("2D neuron image tip point detection Based on Ray-shooting algorithm");

    QGridLayout * layout = new QGridLayout();

    QSpinBox * numbers_2d_spinbox = new QSpinBox();
    numbers_2d_spinbox->setRange(1,1000);
    numbers_2d_spinbox->setValue(number_of_rays);

    QSpinBox * thres_2d_spinbox = new QSpinBox();
    thres_2d_spinbox->setRange(-1, 255);
    thres_2d_spinbox->setValue(background_threshold);

    QSpinBox * magnitude_spinbox = new QSpinBox();
    magnitude_spinbox->setRange(0,10);
    magnitude_spinbox->setValue(magnitude);

    QSpinBox * minimum_scale_spinbox = new QSpinBox();
    minimum_scale_spinbox->setRange(0,100);
    minimum_scale_spinbox->setValue(minimum_scale);

    QSpinBox * number_of_scales_spinbox = new QSpinBox();
    number_of_scales_spinbox->setRange(0,100);
    number_of_scales_spinbox->setValue(number_of_scales);

    QSpinBox * step_size_spinbox = new QSpinBox();
    step_size_spinbox->setRange(0,100);
    step_size_spinbox->setValue(step_size);

    QSpinBox * intensity_threshold_spinbox = new QSpinBox();
    intensity_threshold_spinbox->setRange(0,1000);
    intensity_threshold_spinbox->setValue(intensity_threshold);

    QSpinBox * angle_threshold_spinbox = new QSpinBox();
    angle_threshold_spinbox->setRange(0,180);
    angle_threshold_spinbox->setValue(angle_threshold);

    layout->addWidget(new QLabel("ray numbers"),0,0);
    layout->addWidget(numbers_2d_spinbox, 0,1,1,5);

    layout->addWidget(new QLabel("background threshold"),1,0);
    layout->addWidget(thres_2d_spinbox, 1,1,1,5);

    layout->addWidget(new QLabel("lamda"),2,0);
    layout->addWidget(magnitude_spinbox, 2,1,1,5);

    layout->addWidget(new QLabel("number of scales"),3,0);
    layout->addWidget(number_of_scales_spinbox, 3,1,1,5);

    layout->addWidget(new QLabel("miu"),4,0);
    layout->addWidget(minimum_scale_spinbox, 4,1,1,5);

    layout->addWidget(new QLabel("yita"),5,0);
    layout->addWidget(step_size_spinbox, 5,1,1,5);

    layout->addWidget(new QLabel("T0"),6,0);
    layout->addWidget(intensity_threshold_spinbox, 6,1,1,5);

    layout->addWidget(new QLabel("T1"),7,0);
    layout->addWidget(angle_threshold_spinbox, 7,1,1,5);

    QHBoxLayout * hbox2 = new QHBoxLayout();
    QPushButton * ok = new QPushButton(" ok ");
    ok->setDefault(true);
    QPushButton * cancel = new QPushButton("cancel");
    hbox2->addWidget(cancel);
    hbox2->addWidget(ok);

    layout->addLayout(hbox2,8,0,1,6);
    dialog->setLayout(layout);
    QObject::connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
    QObject::connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));

    //run the dialog
    if(dialog->exec() != QDialog::Accepted)
    {
        if (dialog)
        {
            delete dialog;
            dialog=0;
            cout<<"delete dialog"<<endl;
        }
        return ;
    }

    //get the dialog return values
    background_threshold = thres_2d_spinbox->value();
    number_of_rays = numbers_2d_spinbox->value();
    magnitude = magnitude_spinbox->value();
    minimum_scale = minimum_scale_spinbox->value();
    number_of_scales = number_of_scales_spinbox->value();
    step_size=step_size_spinbox->value();
    intensity_threshold=intensity_threshold_spinbox->value();
    angle_threshold=angle_threshold_spinbox->value();

    angle_threshold=float(angle_threshold*0.01);


    if (dialog)
    {
        delete dialog;
        dialog=0;
        cout<<"delete dialog"<<endl;
    }
}
void TipDetector::getImgData(V3DPluginCallback2& callback2)
{
    // it can be  rewrite for input Image4DSimple
    callback=&callback2;
    v3dhandle curwin = callback2.currentImageWindow();
    if(!curwin)
    {
        v3d_msg("No image is open.");
        return ;
    }
    p4DImage = callback2.getImage(curwin);
    datald = p4DImage->getRawData();

    sz[0] = p4DImage->getXDim();
    sz[1] = p4DImage->getYDim();
    sz[2] = p4DImage->getZDim();
    sz[3] = p4DImage->getCDim();

    V3DLONG nx=sz[0],ny=sz[1],nz=sz[2];
    radiu_block=number_of_scales*step_size+minimum_scale; //the size of the block
    length_block=2*radiu_block+1;
    int size_block=length_block*length_block*length_block;


    float ang = 2*PI/number_of_rays;
    float x_dis, y_dis;
    new_nx=nx+2*radiu_block;new_ny=ny+2*radiu_block;new_nz=nz+2*radiu_block;

    try{binar_datald=new unsigned char [new_nx*new_ny*new_nz];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return ;}

    try{enlarged_datald=new unsigned char [new_nx*new_ny*new_nz];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return ;}

    for(V3DLONG i=0;i<new_nz;i++)
    {
        for(V3DLONG j=0;j<new_ny;j++)
        {
            for(V3DLONG k=0;k<new_nx;k++)
            {
                enlarged_datald[i*new_nx*new_ny+j*new_nx+k]=0;
            }
        }
    }

    V3DLONG num_datald=0;
    for(V3DLONG i=radiu_block;i<new_nz-radiu_block;i++)
    {
      for(V3DLONG j=radiu_block;j<new_ny-radiu_block;j++)
      {
          for(V3DLONG k=radiu_block;k<new_nx-radiu_block;k++)
          {
             enlarged_datald[i*new_nx*new_ny+j*new_nx+k]=datald[num_datald];
             num_datald++;
          }
      }
    }
    cout<<"Expand original iamge successfully"<<endl;

    thres_segment(new_nx*new_ny*new_nz,enlarged_datald,binar_datald,background_threshold);

    try{block=new unsigned char [size_block];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return ;}

    try{xy_mip_datald=new unsigned char [nx*ny];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return ;}

    try{yz_mip_datald=new unsigned char [ny*nz];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return ;}

    try{xz_mip_datald=new unsigned char [nx*nz];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return ;}

//    vector<vector<float> > ray_x(number_of_rays,vector<float>(radiu_block)), ray_y(number_of_rays,vector<float>(radiu_block));
    ray_x.resize(number_of_rays);
    ray_y.resize(number_of_rays);
    for(int i=0;i<number_of_rays;i++)
    {
        ray_x[i].resize(radiu_block);
        ray_y[i].resize(radiu_block);
    }
//    v3d_msg(QString("cereat ray_shooting model"));
    //ray_shooting(numbers_2d ,length_2d,ray_x,ray_y);
    for(int i = 0; i < number_of_rays; i++)
    {
        x_dis = cos(ang*(i+1));
        y_dis = sin(ang*(i+1));
        for(int j = 0; j<radiu_block; j++)
            {
                ray_x[i][j] = x_dis*(j+1);
                ray_y[i][j] = y_dis*(j+1);

            }
    }
    // in this step what we need is image
}

