/* Tip_Detection_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-11-11 : by ChaoWang
 */
#include "ray_shooting.h"
#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"
#include "Tip_Detection_plugin.h"
#include"tipdetector.h"
#include <iostream>
#include"my_surf_objs.h"
#define PI 3.1415926
Q_EXPORT_PLUGIN2(Tip_Detection, Tracing);
//#include"../Branch/new_ray-shooting.h"

using namespace std;

QStringList Tracing::menulist() const
{
	return QStringList() 
        <<tr("Termination_Detection")
        <<tr("test1")
		<<tr("about");
}

QStringList Tracing::funclist() const
{
	return QStringList()
		<<tr("tip_detection")
		<<tr("help");
}
void SaveLandmarkList(LandmarkList s,QString filepath)//add 19.8.19
{
    if(s.isEmpty())
        return;
    if(filepath.isEmpty())
    {
        QString filepath=QFileDialog::getSaveFileName(0,"Save as marker",".","Marker Files(*.marker)");
        if(filepath.isEmpty())
            return;
    }

    FILE * fp = fopen(filepath.toLatin1(), "wt");
    if (!fp)
    {
        v3d_msg("Could not open the file to save the marker.");
        return ;
    }
    fprintf(fp, "##x,y,z,radius,shape,name,comment, color_r,color_g,color_b\n");
    for(int i=0;i<s.size();i++)
    {
        LocationSimple p=s.at(i);

        fprintf(fp, "%5.3f,%5.3f,%5.3f,%5.3f,%d,%s,%c,%d,%d,%d \n", p.x,p.y,p.z,p.radius,p.shape,p.name.c_str(),p.comments.c_str(),p.color.r,p.color.g,p.color.b);
    }
     fclose(fp);
     cout<<"save landmarker as "<<filepath.toUtf8().data()<<endl;
    return;


}

void Tracing::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("test1"))
	{    
        TipDetector tp;
        tp.getImgData(callback);
        tp.GUI_input_argu();
        tp.whole_img();
//        int flag = tipdetection(callback,parent);
//        if(flag != -1)
//        v3d_msg(tr("Teminations have been detected !"));

	}
    else if(menu_name == tr("Termination_Detection"))
    {
                int flag = tipdetection(callback,parent);
                if(flag != -1)
                v3d_msg(tr("Teminations have been detected !"));
    }
	else
	{
        v3d_msg(tr("This is a terminations detection plugin "
            "Developed by ChaoWang and Weixun Chen, 2018-11"));
	}
}

bool Tracing::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("Terminations Detection"))
	{
        int flag = tipdetection1(input,output,callback);
        if(flag != -1)
        v3d_msg(tr("Teminations have been detected !"));
		return true;
	}

	else if (func_name == tr("help"))
	{
		printHelp();
		return true;
	}
	
}

int tipdetection1(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    cout<<"Welcome to termination detection"<<endl;
    int number_of_rays = 64;
    int background_threshold = 40;
    float magnitude =5.0;
    int minimum_scale=4;
    int number_of_scales=4;
    int step_size=2;
    int intensity_threshold=115;
    float angle_threshold=68;

    QString marker_name_final_save = ((vector<char*> *)(output.at(0).p))->at(0);
    if(marker_name_final_save.isEmpty()){return false;}
    if (input.size()>=2)
    {
        vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
        if(paras.size() >= 1) number_of_rays =  atoi(paras.at(0));
        if(paras.size() >= 2) background_threshold = atoi(paras.at(1));
        if(paras.size() >= 3) magnitude = atoi(paras.at(2));
        if(paras.size() >= 4) minimum_scale = atoi(paras.at(3));
        if(paras.size() >= 5) number_of_scales = atoi(paras.at(4));
        if(paras.size() >= 6) step_size = atoi(paras.at(5));
        if(paras.size() >= 7) intensity_threshold = atoi(paras.at(6));
        if(paras.size() >= 8) angle_threshold = atoi(paras.at(7));
    }

    char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
    cout<<"Get input para over"<<endl;

    //get the dialog return values
//    background_threshold = thres_2d_spinbox->value();
//    number_of_rays = numbers_2d_spinbox->value();
//    magnitude = magnitude_spinbox->value();
//    minimum_scale = minimum_scale_spinbox->value();
//    number_of_scales = number_of_scales_spinbox->value();
//    step_size=step_size_spinbox->value();
//    intensity_threshold=intensity_threshold_spinbox->value();
//    angle_threshold=angle_threshold_spinbox->value();
    Image4DSimple *p4DImage = callback.loadImage(inimg_file);
    if(!p4DImage || !p4DImage->valid())
    {
         v3d_msg("Fail to load the input image.",0);
         if (p4DImage) {delete p4DImage; p4DImage=0;}
         return false;
    }

    angle_threshold=float(angle_threshold*0.01);



    // 2 - get edge point
    unsigned char* datald=0;  //input;
    datald = p4DImage->getRawData();

    V3DLONG sz[4];
    sz[0] = p4DImage->getXDim();
    sz[1] = p4DImage->getYDim();
    sz[2] = p4DImage->getZDim();
    sz[3] = p4DImage->getCDim();

    V3DLONG nx=sz[0],ny=sz[1],nz=sz[2];
    cout<<"test`"<<endl;
    int radiu_block=number_of_scales*step_size+minimum_scale; //the size of the block
    int length_block=2*radiu_block+1;
    int size_block=length_block*length_block*length_block;
    LandmarkList curlist;
    LocationSimple s;
    int xy_flag=0;
    int xz_flag=0;
    int yz_flag=0;
    int sum_flag=0;
    float ang = 2*PI/number_of_rays;
    float x_dis, y_dis;
    V3DLONG new_nx=nx+2*radiu_block,new_ny=ny+2*radiu_block,new_nz=nz+2*radiu_block;
    cout<<"test2"<<endl;

    unsigned char *binar_datald;
    try{binar_datald=new unsigned char [new_nx*new_ny*new_nz];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

    unsigned char *datald_copy;
    try{datald_copy=new unsigned char [nx*ny*nz];}
    catch(...) {v3d_msg("cannot allocate memory for datald_copy."); return 0;}
    for(int i=0; i<nx*ny*nz; i++)
    {
        datald_copy[i]=datald[i];
    }


    unsigned char *enlarged_datald;
    try{enlarged_datald=new unsigned char [new_nx*new_ny*new_nz];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

    unsigned char *block;
    try{block=new unsigned char [size_block];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

    unsigned char *xy_mip_datald;
    try{xy_mip_datald=new unsigned char [nx*ny];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

    unsigned char *yz_mip_datald;
    try{yz_mip_datald=new unsigned char [ny*nz];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

    unsigned char *xz_mip_datald;
    try{xz_mip_datald=new unsigned char [nx*nz];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

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
    cout<<"test3"<<endl;

    V3DLONG num_datald=0;
    for(V3DLONG i=radiu_block;i<new_nz-radiu_block;i++)
    {
      for(V3DLONG j=radiu_block;j<new_ny-radiu_block;j++)
      {
          for(V3DLONG k=radiu_block;k<new_nx-radiu_block;k++)
          {
             enlarged_datald[i*new_nx*new_ny+j*new_nx+k]=datald_copy[num_datald];
             num_datald++;
          }
      }
    }
    cout<<"Expand original iamge successfully"<<endl;

    thres_segment(new_nx*new_ny*new_nz,enlarged_datald,binar_datald,background_threshold);

    cout<<"test4"<<endl;



    vector<vector<float> > ray_x(number_of_rays,vector<float>(radiu_block)), ray_y(number_of_rays,vector<float>(radiu_block));
    v3d_msg(QString("cereat ray_shooting model"));
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
    cout<<"test5"<<endl;

    for(V3DLONG j=radiu_block+1;j<new_nz-radiu_block-1;j++)
    {
         for(V3DLONG i=radiu_block+1;i<new_ny-radiu_block-1;i++)
         {
             for(V3DLONG k=radiu_block+1;k<new_nx-radiu_block-1;k++)
             {
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
    }

    cout<<"test6"<<endl;

    // change the clustering algorithm by chaowang 2018.11.16
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
    for(double i=0;i<curlist.size();i++)
    {
        curlist[i].x-=radiu_block;
        curlist[i].y-=radiu_block;
        curlist[i].z-=radiu_block;
        cout<<" curlist[i].x "<<curlist[i].x<<" curlist[i].y "<<curlist[i].y<<" curlist[i].z "<<curlist[i].z<<endl;
    }


    cout<<"test7"<<endl;

//    Image4DSimple * new4DImage = new Image4DSimple();
//    new4DImage->setData((unsigned char *)binar_datald, new_nx, new_ny, new_nz, p4DImage->getCDim(), p4DImage->getDatatype());
//    v3dhandle newwin = callback.newImageWindow();
//    callback.setImage(newwin, new4DImage);
//    callback.updateImageWindow(newwin);
//    callback.setLandmark(newwin, curlist);
    SaveLandmarkList(curlist,marker_name_final_save);
//    writeMarker_file(
//    writeMarker_file(
//    if(binar_datald) {delete []binar_datald; binar_datald = 0;}

//    if(block) {delete []block; block = 0;}
//    if(xy_mip_datald) {delete []xy_mip_datald; xy_mip_datald = 0;}
//    if(xz_mip_datald) {delete []xz_mip_datald; xz_mip_datald = 0;}
//    if(yz_mip_datald){delete []yz_mip_datald;yz_mip_datald=0;}
//    if(enlarged_datald){delete []enlarged_datald;enlarged_datald=0;}
    //if(datald){delete []datald;datald=0;}
    return 1;
}


int tipdetection(V3DPluginCallback2 &callback, QWidget *parent)
{
    // 1 - Obtain the current 4D image pointer
    v3dhandle curwin = callback.currentImageWindow();
    if(!curwin)
    {
        v3d_msg("No image is open.");
        return -1;
    }
    Image4DSimple *p4DImage = callback.getImage(curwin);

    int number_of_rays = 64;
    int background_threshold = 40;
    float magnitude =5.0;
    int minimum_scale=4;
    int number_of_scales=4;
    int step_size=2;
    int intensity_threshold=115;
    float angle_threshold=68;


    //set update the dialog
    QDialog * dialog = new QDialog();

    if(p4DImage->getZDim() > 1)
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
        return -1;
    }
    cout<<"test0"<<endl;

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


    // 2 - get edge point
    unsigned char* datald=0;  //input;
    datald = p4DImage->getRawData();

    V3DLONG sz[4];
    sz[0] = p4DImage->getXDim();
    sz[1] = p4DImage->getYDim();
    sz[2] = p4DImage->getZDim();
    sz[3] = p4DImage->getCDim();

    V3DLONG nx=sz[0],ny=sz[1],nz=sz[2];
    cout<<"test`"<<endl;
    int radiu_block=number_of_scales*step_size+minimum_scale; //the size of the block
    int length_block=2*radiu_block+1;
    int size_block=length_block*length_block*length_block;
    LandmarkList curlist;
    LocationSimple s;
    int xy_flag=0;
    int xz_flag=0;
    int yz_flag=0;
    int sum_flag=0;
    float ang = 2*PI/number_of_rays;
    float x_dis, y_dis;
    V3DLONG new_nx=nx+2*radiu_block,new_ny=ny+2*radiu_block,new_nz=nz+2*radiu_block;
    cout<<"test2"<<endl;

    unsigned char *binar_datald;
    try{binar_datald=new unsigned char [new_nx*new_ny*new_nz];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

    unsigned char *datald_copy;
    try{datald_copy=new unsigned char [nx*ny*nz];}
    catch(...) {v3d_msg("cannot allocate memory for datald_copy."); return 0;}
    for(int i=0; i<nx*ny*nz; i++)
    {
        datald_copy[i]=datald[i];
    }


    unsigned char *enlarged_datald;
    try{enlarged_datald=new unsigned char [new_nx*new_ny*new_nz];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

    unsigned char *block;
    try{block=new unsigned char [size_block];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

    unsigned char *xy_mip_datald;
    try{xy_mip_datald=new unsigned char [nx*ny];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

    unsigned char *yz_mip_datald;
    try{yz_mip_datald=new unsigned char [ny*nz];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

    unsigned char *xz_mip_datald;
    try{xz_mip_datald=new unsigned char [nx*nz];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

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
    cout<<"test3"<<endl;

    V3DLONG num_datald=0;
    for(V3DLONG i=radiu_block;i<new_nz-radiu_block;i++)
    {
      for(V3DLONG j=radiu_block;j<new_ny-radiu_block;j++)
      {
          for(V3DLONG k=radiu_block;k<new_nx-radiu_block;k++)
          {
             enlarged_datald[i*new_nx*new_ny+j*new_nx+k]=datald_copy[num_datald];
             num_datald++;
          }
      }
    }
    cout<<"Expand original iamge successfully"<<endl;

    thres_segment(new_nx*new_ny*new_nz,enlarged_datald,binar_datald,background_threshold);

    cout<<"test4"<<endl;



    vector<vector<float> > ray_x(number_of_rays,vector<float>(radiu_block)), ray_y(number_of_rays,vector<float>(radiu_block));
    v3d_msg(QString("cereat ray_shooting model"));
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
    cout<<"test5"<<endl;

    for(V3DLONG j=radiu_block+1;j<new_nz-radiu_block-1;j++)
    {
         for(V3DLONG i=radiu_block+1;i<new_ny-radiu_block-1;i++)
         {
             for(V3DLONG k=radiu_block+1;k<new_nx-radiu_block-1;k++)
             {
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
    }

    cout<<"test6"<<endl;

    // change the clustering algorithm by chaowang 2018.11.16
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


    cout<<"test7"<<endl;

    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *)binar_datald, new_nx, new_ny, new_nz, p4DImage->getCDim(), p4DImage->getDatatype());
    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, new4DImage);
    callback.updateImageWindow(newwin);
    callback.setLandmark(newwin, curlist);
//    if(binar_datald) {delete []binar_datald; binar_datald = 0;}

//    if(block) {delete []block; block = 0;}
//    if(xy_mip_datald) {delete []xy_mip_datald; xy_mip_datald = 0;}
//    if(xz_mip_datald) {delete []xz_mip_datald; xz_mip_datald = 0;}
//    if(yz_mip_datald){delete []yz_mip_datald;yz_mip_datald=0;}
//    if(enlarged_datald){delete []enlarged_datald;enlarged_datald=0;}
    //if(datald){delete []datald;datald=0;}
cout<<"test8"<<endl;
	return 1;
}


void printHelp()
{
	cout<<"\nThis is a demo plugin to detect tip point in an image. by Keran Lin 2017-04"<<endl;
	cout<<"\nUsage: v3d -x <example_plugin_name> -f tip_detection -i <input_image_file> -o <output_image_file> -p <subject_color_channel> <threshold>"<<endl;
	cout<<"\t -i <input_image_file>                      input 3D image (tif, raw or lsm)"<<endl;
	cout<<"\t -o <output_image_file>                     output image of the thresholded subject channel"<<endl;
	cout<<"\t                                            the  paras must come in this order"<<endl;
	cout<<"\nDemo: v3d -x libexample_debug.dylib -f image_thresholding -i input.tif -o output.tif -p 0 100\n"<<endl;
	return;
}

