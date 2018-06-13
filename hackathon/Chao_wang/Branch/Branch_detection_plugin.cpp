/* Branch_detection_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-5-20 : by Chao Wang
 */
#include <iostream>
#include "basic_surf_objs.h"
#include "v3d_message.h"
#include <vector>
#include "Branch_detection_plugin.h"
#include "new_ray-shooting.h"
//#include "../../zhi/APP2_large_scale/my_surf_objs.h"
#define PI 3.1415926
using namespace std;
Q_EXPORT_PLUGIN2(Branch_detection, TestPlugin)
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
        <<tr("Branch_detection")
        <<tr("creat_ray-shooting_model")
        <<tr("curve points detection")
        <<tr(" Gaussian Filtering ")
        <<tr("calculate the MIP of a subset of Z slices")
		<<tr("about");
}

QStringList TestPlugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
        if (menu_name == tr("Branch_detection"))
        {
           int flag=branch_detection(callback,parent);
           if (flag==1)
           {
              v3d_msg(tr("branch detection completed!"));
           }
        }
        else if (menu_name == tr("creat_ray-shooting_model"))
        {
            int flag= rayshoot_model(callback,parent);
            if(flag==1)
            v3d_msg("ray-shooting model creat.");
        }
        else if (menu_name == tr("curve points detection"))
        {
            int flag= curve_detection(callback,parent);
            if(flag==1)
            v3d_msg("curve points detection was ok ");
        }
        else if (menu_name == tr(" Gaussian Filtering "))
        {
            int flag= gassion_filter(callback,parent);
            if(flag==1)
            v3d_msg("Gaussian Filtering was ok ");
        }
        else if(menu_name==tr("calculate the MIP of a subset of Z slices"))
        {
            int flag=MIPZSlices(callback,parent);
            if(flag==1)
            v3d_msg("MIP was ok ");
        }

	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Chao Wang, 2018-5-20"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("func2"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}
int branch_detection(V3DPluginCallback2 &callback, QWidget *parent)
{
            // 1 - Obtain the current 4D image pointer
            v3dhandle curwin = callback.currentImageWindow();
            if(!curwin)
            {
                    v3d_msg("No image is open.");
                    return -1;
            }
            Image4DSimple *p4DImage = callback.getImage(curwin);

                    // 2 - Ask for parameters
                    // asking for the subject channel



            int ray_numbers_2d = 128;
            int thres_2d = 45;
            int ray_length_2d = 16;
            int size_gaussion=3;


            //set update the dialog
            QDialog * dialog = new QDialog();


            if(p4DImage->getZDim() > 1)
                    dialog->setWindowTitle("3D neuron image tip point detection Based on Ray-shooting algorithm");
            else
                    dialog->setWindowTitle("2D neuron image tip point detection Based on Ray-shooting algorithm");

        QGridLayout * layout = new QGridLayout();

        QSpinBox * ray_numbers_2d_spinbox = new QSpinBox();
        ray_numbers_2d_spinbox->setRange(1,1000);
        ray_numbers_2d_spinbox->setValue(ray_numbers_2d);

        QSpinBox * thres_2d_spinbox = new QSpinBox();
        thres_2d_spinbox->setRange(-1, 255);
        thres_2d_spinbox->setValue(thres_2d);

        QSpinBox * ray_length_2d_spinbox = new QSpinBox();
        ray_length_2d_spinbox->setRange(1,p4DImage->getXDim());
        ray_length_2d_spinbox->setValue(ray_length_2d);

        QSpinBox * size_gaussion_spinbox = new QSpinBox();
        size_gaussion_spinbox->setRange(1,255);
        size_gaussion_spinbox->setValue(size_gaussion);

        layout->addWidget(new QLabel("ray numbers"),0,0);
        layout->addWidget(ray_numbers_2d_spinbox, 0,1,1,5);

        layout->addWidget(new QLabel("intensity threshold"),1,0);
        layout->addWidget(thres_2d_spinbox, 1,1,1,5);

        layout->addWidget(new QLabel("ray length"),2,0);
        layout->addWidget(ray_length_2d_spinbox, 2,1,1,5);

        layout->addWidget(new QLabel("size of guassion window"),3,0);
        layout->addWidget(size_gaussion_spinbox, 3,1,1,5);


        QHBoxLayout * hbox2 = new QHBoxLayout();
        QPushButton * ok = new QPushButton(" ok ");
        ok->setDefault(true);
        QPushButton * cancel = new QPushButton("cancel");
        hbox2->addWidget(cancel);
        hbox2->addWidget(ok);

        layout->addLayout(hbox2,6,0,1,6);
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

        //get the dialog return values
        ray_numbers_2d = ray_numbers_2d_spinbox->value();
        thres_2d = thres_2d_spinbox->value();
        ray_length_2d = ray_length_2d_spinbox->value();
        size_gaussion = size_gaussion_spinbox->value();

        if (dialog)
            {
                    delete dialog;
                    dialog=0;
                    cout<<"delete dialog"<<endl;


            }

       V3DLONG sz[3];
       sz[0] = p4DImage->getXDim();
       sz[1] = p4DImage->getYDim();
       sz[2] = p4DImage->getZDim();

       unsigned char* datald=0;
       datald = p4DImage->getRawData();
       V3DLONG size_image=sz[0]*sz[1]*sz[2];

       cout<<"segment"<<endl;

       unsigned char *image_binary=0;
       try{image_binary=new unsigned char [size_image];}
       catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}
       for(V3DLONG i = 0; i < size_image; i++)
       {
           if(datald[i] > thres_2d)
               image_binary[i] = 255;
           else
               image_binary[i] = 0;
       }
       v3d_msg("segment was complete");

       v3d_msg("start find 2D cureve points");
       V3DLONG nx=sz[0];
       V3DLONG ny=sz[1];
       V3DLONG nz=sz[2];
       V3DLONG spage=nx*ny*nz;

       vector<NeuronSWC> point_pool;
       unsigned char *new_datald=0;
       try{new_datald=new unsigned char [spage];}
       catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

       unsigned char *imagesobelx=0;
       try{imagesobelx=new unsigned char [spage];}
       catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

       unsigned char *imagesobely=0;
       try{imagesobely=new unsigned char [spage];}
       catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

       float *xx=0;
       try{xx=new float [spage];}
       catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

       float *yy=0;
       try{yy=new float [spage];}
       catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

       float *xy=0;
       try{xy=new float [spage];}
       catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

       float *Gxx=0;
       try{Gxx=new float [spage];}
       catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

       float *Gyy=0;
       try{Gyy=new float [spage];}
       catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

       float *Gxy=0;
       try{Gxy=new float [spage];}
       catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

       float *Hresult=0;
       try{Hresult=new float [spage];}
       catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}
       v3d_msg("allocate the memory");


    for(V3DLONG z_size=0;z_size<nz;z_size++)
    {

                  for (V3DLONG i = 0; i < ny ; i++)
                  {
                      for (V3DLONG j = 1; j < nx ; j++)
                      {
                           new_datald[i*nx+j]=image_binary[z_size*nx*ny+i*nx+j];

                      }
                  }

                  for (V3DLONG i = 1; i < ny - 1; i++)
                  {
                       for (V3DLONG j = 1; j < nx - 1; j++)
                        {
                                //通过指针遍历图像上每一个像素
                                double gradY = new_datald[(i + 1)*nx + j - 1] + new_datald[(i + 1)*nx + j] * 2 + new_datald[(i + 1)*nx + j + 1] - new_datald[(i - 1)*nx + j - 1] - new_datald[(i - 1)*nx + j] * 2 - new_datald[(i - 1)*nx + j + 1];
                                imagesobelx[i*nx + j] = abs(gradY);

                                double gradX = new_datald[(i - 1)*nx + j + 1] + new_datald[i*nx + j + 1] * 2 + new_datald[(i + 1)*nx + j + 1] - new_datald[(i - 1)*nx + j - 1] - new_datald[i*nx + j - 1] * 2 - new_datald[(i + 1)*nx + j - 1];
                                imagesobely[i*nx + j] = abs(gradX);
                        }
                  }

                     //将梯度数组转换成8位无符号整型
                    //convertScaleAbs(imageSobelX, imageSobelX);
                    //convertScaleAbs(imageSobelY, imageSobelY);

                     mul(imagesobelx,xx,nx,ny);  //x方向梯度的平方
                     mul(imagesobely,yy,nx,ny);  //y方向梯度的平方
                     mul(imagesobelx,imagesobely,xy,nx,ny);  //xy方向梯度的平方

                     //gaussion filter
                     MyGaussianBlur(xx,Gxx,size_gaussion,sz[0],sz[1]);
                     MyGaussianBlur(yy,Gyy,size_gaussion,sz[0],sz[1]);
                     MyGaussianBlur(xy,Gxy,size_gaussion,sz[0],sz[1]);

                     harrisResponse(Gxx,Gyy,Gxy,Hresult,0.05,sz[0],sz[1]);

                     int r = size_gaussion / 2;

                     for (int i = r; i < ny-r; i++)
                     {
                         for (int j = r; j < nx - r; j++)
                         {
                             if (
                                 Hresult[i*nx+j] > Hresult[(i-1)*nx+j] &&
                                 Hresult[i*nx+j] > Hresult[(i-1)*nx+j-1] &&
                                 Hresult[i*nx+j] > Hresult[(i-1)*nx+j+1] &&
                                 Hresult[i*nx+j] > Hresult[i*nx+j-1] &&
                                 Hresult[i*nx+j] > Hresult[i*nx+j+1] &&
                                 Hresult[i*nx+j] > Hresult[(i+1)*nx+j-1] &&
                                 Hresult[i*nx+j] > Hresult[(i+1)*nx+j] &&
                                 Hresult[i*nx+j] > Hresult[(i+1)*nx+j+1])
                             {

                                 //v3d_msg(QString("Hresult[i*nx+j] is %1").arg(Hresult[i*nx+j]));
                                 if (Hresult[i*nx+j] > 10000)
                                 {
                                     NeuronSWC pp;
                                     pp.x=j+1;
                                     pp.y=i+1;
                                     pp.z=z_size;
                                     pp.radius=1;
                                     pp.color = random_rgba8(255);
                                     point_pool.push_back(pp);
                                 }
                             }

                         }
                     }

    }
    v3d_msg(QString("numble of marker is %1").arg(point_pool.size()));
    v3d_msg("2D cureve points were complete");


       vector<vector<float> > ray_x(ray_numbers_2d,vector<float>(ray_length_2d)), ray_y(ray_numbers_2d,vector<float>(ray_length_2d));

       cout<<"create 2D_ray"<<endl;

       float ang = 2*PI/ray_numbers_2d;
       float x_dis, y_dis;

       for(int i = 0; i < ray_numbers_2d; i++)
       {
           x_dis = cos(ang*(i+1));
           y_dis = sin(ang*(i+1));
           for(int j = 0; j<ray_length_2d; j++)
               {
                   ray_x[i][j] = x_dis*(j+1);
                   ray_y[i][j] = y_dis*(j+1);
               }
       }

       cout<<"create 2D_ray success"<<endl;


       v3d_msg("mip");

      // v3d_msg(QString("sz[0] is %1,sz[1] is %2, sz[2] is %3").arg(N).arg(M).arg(P));
       unsigned char *image_mip=0;
       try{image_mip=new unsigned char [spage];}
       catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}
       for(V3DLONG iy = 0; iy < ny; iy++)
       {
           V3DLONG offsetj = iy*nx;
           for(V3DLONG ix = 0; ix < nx; ix++)
           {
               int max_mip = 0;
               for(V3DLONG iz = 0; iz < nz; iz++)
               {
                   V3DLONG offsetk = iz*nx*ny;
                   if(image_binary[offsetk + offsetj + ix] >= max_mip)
                   {
                       image_mip[iy*nx + ix] = image_binary[offsetk + offsetj + ix];
                       max_mip = image_binary[offsetk + offsetj + ix];
                   //    v3d_msg(QString("max_mip is %1").arg(max_mip));
                   }
               }
           }
       }

       v3d_msg("mip have complete");

       cout<<"begin circle "<<endl;

       LandmarkList curlist;
       LocationSimple s;


       for(V3DLONG k=1;k<sz[1]-1;k++)
       {
           for(V3DLONG i=1;i<sz[0]-1;i++)
           {

                   int project_value=get_2D_ValueUINT8(k, i,image_binary,sz[0],sz[1]);
                   //v3d_msg(QString("project_value is %1").arg(project_value));
                   //v3d_msg(QString("y is %1,x is %2").arg(k).arg(i));
                   if(project_value > thres_2d)
                       {
                           bool flag=rayinten_2D(k,i,ray_numbers_2d ,ray_length_2d,ray_x, ray_y, image_binary,sz[0],sz[1]);
                          // v3d_msg(QString("flag is %1").arg(flag));
                           if (flag==true)
                           {
//                                   for(V3DLONG bb=0;bb<point_pool.size();bb++)
//                                   {
//                                        //v3d_msg(QString("pool_size is %1").arg(point_pool.size()));
//                                       if((k==point_pool[bb].y)&&(i==point_pool[bb].x))
//                                       {

//                                           s.x=i+1;
//                                           s.y=k+1;
//                                           s.z=point_pool[bb].z;
//                                           s.radius=1;
//                                           s.color = random_rgba8(255);
//                                           curlist << s;
//                                           //v3d_msg(QString("z is %1").arg(point_pool[bb]->z));
//                                       }
//                                   }

                                   s.x=i+1;
                                   s.y=k+1;
                                   s.z=10;
                                   s.radius=1;
                                   s.color = random_rgba8(255);
                                   curlist << s;


                           }

                       }
             }
         }

       //v3d_msg(QString("numble of marker is %1").arg(curlist.size()));
       for(int i=0;i<curlist.size();i++)
           for(int j=i+1;j<curlist.size();j++)
       {

           {
               if(square(curlist[j].x-curlist[i].x)+square(curlist[j].y-curlist[i].y)+square(curlist[j].z-curlist[i].z)<150)
                   {
                       curlist.removeAt(j);
                       j = j - 1;
                   }
           }
       }
       v3d_msg(QString("numble of marker is %1").arg(curlist.size()));
       v3d_msg("express marker");
       callback.setLandmark(curwin, curlist);

       if(image_binary) {delete []image_binary; image_binary = 0;}
       if(image_mip) {delete []image_mip; image_mip = 0;}
       //if(datald){delete []datald;datald=0;}

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

int rayshoot_model(V3DPluginCallback2 &callback, QWidget *parent)
{
    // 1 - Obtain the current 4D image pointer
    v3dhandle curwin = callback.currentImageWindow();
    if(!curwin)
    {
        v3d_msg("No image is open.");
        return -1;
    }
    Image4DSimple *p4DImage = callback.getImage(curwin);

    V3DLONG sz[3];
    sz[0] = p4DImage->getXDim();
    sz[1] = p4DImage->getYDim();
    sz[2] = p4DImage->getZDim();
    V3DLONG nx=sz[0],ny=sz[1],nz=sz[2];
    V3DLONG size_image=nx*ny*nz;


    float x_point=0,y_point=0;
    int ray_numble=64;
    int length_numble=8;
    int thres=30;

    QDialog * dialog = new QDialog();
    QGridLayout * layout = new QGridLayout();

    QSpinBox * X_point_spinbox = new QSpinBox();
    X_point_spinbox->setRange(1,sz[0]);
    X_point_spinbox->setValue(x_point);

    QSpinBox * Y_point_spinbox = new QSpinBox();
    Y_point_spinbox->setRange(1, sz[1]);
    Y_point_spinbox->setValue(y_point);

    QSpinBox * thres_spinbox = new QSpinBox();
    thres_spinbox->setRange(1, 255);
    thres_spinbox->setValue(thres);

    QSpinBox * number_ray_spinbox = new QSpinBox();
    number_ray_spinbox->setRange(1,1000);
    number_ray_spinbox->setValue(ray_numble);

    QSpinBox * length_ray_spinbox = new QSpinBox();
    length_ray_spinbox->setRange(1,100);
    length_ray_spinbox->setValue(length_numble);



    layout->addWidget(new QLabel("x laction "),0,0);
    layout->addWidget(X_point_spinbox, 0,1,1,5);

    layout->addWidget(new QLabel("y location"),1,0);
    layout->addWidget(Y_point_spinbox, 1,1,1,5);

    layout->addWidget(new QLabel("threhold"),2,0);
    layout->addWidget(thres_spinbox, 2,1,1,5);

    layout->addWidget(new QLabel("ray_numble of model"),3,0);
    layout->addWidget(number_ray_spinbox, 3,1,1,5);

    layout->addWidget(new QLabel("length_numbers of model"),4,0);
    layout->addWidget(length_ray_spinbox, 4,1,1,5);

    QHBoxLayout * hbox2 = new QHBoxLayout();
    QPushButton * ok = new QPushButton(" ok ");
    ok->setDefault(true);
    QPushButton * cancel = new QPushButton("cancel");
    hbox2->addWidget(cancel);
    hbox2->addWidget(ok);

    layout->addLayout(hbox2,6,0,1,6);
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

    //get the dialog return values
    x_point = X_point_spinbox->value();
    y_point = Y_point_spinbox->value();
    ray_numble = number_ray_spinbox->value();
    length_numble = length_ray_spinbox->value();
    thres = thres_spinbox->value();

    if (dialog)
    {
        delete dialog;
        dialog=0;
        cout<<"delete dialog"<<endl;
    }

    unsigned char * datald=0;
    datald=p4DImage->getRawData();


    v3d_msg("mip ");
    unsigned char *image_mip=0;
    try{image_mip=new unsigned char [size_image];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}
    for(V3DLONG iy = 0; iy < ny; iy++)
    {
        V3DLONG offsetj = iy*nx;
        for(V3DLONG ix = 0; ix < nx; ix++)
        {
            int max_mip = 0;
            for(V3DLONG iz = 0; iz < nz; iz++)
            {
                V3DLONG offsetk = iz*nx*ny;
                if(datald[offsetk + offsetj + ix] >= max_mip)
                {
                    image_mip[iy*nx + ix] = datald[offsetk + offsetj + ix];
                    max_mip = datald[offsetk + offsetj + ix];
                //    v3d_msg(QString("max_mip is %1").arg(max_mip));
                }
            }
        }
    }
   // v3d_msg("mip have complete");


    //v3d_msg("segment");
    unsigned char *image_binary=0;
    try{image_binary=new unsigned char [size_image];}
    catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}
    for(V3DLONG i = 0; i < nx*ny; i++)
    {
        if(image_mip[i] > thres)
            image_binary[i] = 255;
        else
            image_binary[i] = 0;
    }
   // v3d_msg("segment was complete");

    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *)image_binary, p4DImage->getXDim(), p4DImage->getYDim(), 1, p4DImage->getCDim(), p4DImage->getDatatype());
    v3dhandle newwin = callback.newImageWindow();
    //v3dhandle newwin = callback.currentImageWindow();
    callback.setImage(newwin, new4DImage);
    callback.setImageName(newwin, "maximum intensity projection image");
    callback.updateImageWindow(newwin);

//    v3dhandle newwin;
//    if(QMessageBox::Yes == QMessageBox::question (0, "",
//        QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
//        newwin = callback.currentImageWindow();
//    else
//        newwin = callback.newImageWindow();

//    callback.setImage(newwin, &p4DImage);
//    callback.setImageName(newwin, m_InputFileName + "_mip_"  + ".v3draw");
//    callback.updateImageWindow(newwin);
// v3d_msg("mip have complete");

    cout<<"start the ray-shooting model"<<endl;

    vector<float> x_lac;
    vector<float> y_lac;
    float ang = 2*PI/ray_numble;

    //get the location
    for(int i=0;i<ray_numble;i++)
    {
        float x_dis=cos(ang*(i+1));
        float y_dis=sin(ang*(i+1));
        for(int j=0;j<length_numble;j++)
        {
             x_lac.push_back(x_point+(j+1)*x_dis);
             y_lac.push_back(y_point+(j+1)*y_dis);
        }
        //v3d_msg(QString("y is %1").arg(y_lac[i]));
    }

    //get the pixel valus
    vector<float> pixel;
    int num_points=ray_numble*length_numble;
    for(int i=0;i<num_points;i++)
    {
        float tt=interp_2d( y_lac[i], x_lac[i],image_binary, sz[0], sz[1]);
        pixel.push_back(tt);
        //v3d_msg(QString("pixel is %1").arg(pixel[i]));
    }

    for(int i=0;i<num_points;i++)
    {
       // v3d_msg(QString("pixel is %1,y is %2,pixel is %3").arg(x_lac[i]).arg(y_lac[i]).arg(pixel[i]));
    }

    //get the sum of pixel of length
    vector<float> sum_ray_pixel;
    float max_ray;
    for(int i=0;i<ray_numble;i++)
    {
        float sum;
        for(int j=0;j<length_numble;j++)
        {
            float sum=0;
            sum=sum+pixel[i*length_numble+j];
        }
        sum_ray_pixel.push_back(sum);
        if(sum>max_ray)
        {
            max_ray=sum;
        }

        //v3d_msg(QString("sum is %1").arg(sum_ray_pixel[i]));
    }

    float max_value=max_ray*0.40;
    v3d_msg("display the ray-shooting model");
//    LandmarkList curlist;
//    LocationSimple pp;
//    for(int i=0;i<ray_numble;i++)
//    {
//        if(sum_ray_pixel[i]>max_value)
//        {
//            for(int j=0;j<length_numble;j++)
//            {
//                pp.x=x_lac[i*length_numble+j];
//                pp.y=y_lac[i*length_numble+j];
//                pp.radius=1;
//                pp.color=random_rgba8(255);
//            }
//        }
//        else
//        {
//            for(int j=0;j<length_numble;j++)
//            {
//                pp.x=x_lac[i*length_numble+j];
//                pp.y=y_lac[i*length_numble+j];
//                pp.radius=1;
//                pp.color=random_rgba8(255);
//            }
//        }
//     curlist << pp;

//    }
//    v3d_msg(QString("numble of marker is %1").arg(curlist.size()));
//    v3d_msg("express marker");
//    callback.setLandmark(curwin, curlist);

//    if(image_binary) {delete []image_binary; image_binary = 0;}
//    if(image_mip) {delete []image_mip; image_mip = 0;}
    //if(datald){delete []datald;datald=0;}

//    return 1;

    cout<<"start save swc"<<endl;
    NeuronTree nt;
    QList <NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();
    int above=0;
    int low=0;
    for(int i=0;i<sum_ray_pixel.size();i++)
    {
        NeuronSWC pp;
        if(sum_ray_pixel[i]>max_value)
        {
            above++;
            for(int j=0;j<length_numble;j++)
            {
                pp.x=x_lac[i*length_numble+j];
                pp.y=y_lac[i*length_numble+j];
                pp.radius=0.3;
                pp.type=4;
                pp.parent=-1;
                listNeuron.append(pp);
                hashNeuron.insert(pp.n, listNeuron.size()-1);
            }
        }
        else
        {
            low++;
            for(int j=0;j<length_numble;j++)
            {
                pp.x=x_lac[i*length_numble+j];
                pp.y=y_lac[i*length_numble+j];
                pp.radius=0.3;
                pp.type=3;
                pp.parent=-1;
                listNeuron.append(pp);
                hashNeuron.insert(pp.n, listNeuron.size()-1);
            }
        }

//        listNeuron.append(pp);
//        hashNeuron.insert(pp.n, listNeuron.size()-1);
    }
    v3d_msg(QString("above is %1").arg(above));
    v3d_msg(QString("low is %1").arg(low));

//    for(V3DLONG i=0;i<ray_numble*length_numble;i++)
//    {
//        NeuronSWC s;
//        if(sum_ray_pixel[i]>max_value)
//        {
//            for(int j=0;j<length_numble;j++)
//            {
//                s.x=x_lac[i];
//                s.y=y_lac[i];
//                s.radius=0.3;
//                s.type=4;
//                s.parent=-1;
//            }
//        }
//        else
//        {
//            for(int j=0;j<length_numble;j++)
//            {
//                s.x=x_lac[i*length_numble+j];
//                s.y=y_lac[i*length_numble+j];
//                s.radius=0.3;
//                s.type=4;
//                s.parent=-1;
//            }
//        }

//        listNeuron.append(s);
//        hashNeuron.insert(s.n, listNeuron.size()-1);
//     }

    nt.n = -1;
    nt.on = true;
    nt.listNeuron = listNeuron;
    nt.hashNeuron = hashNeuron;
    QString outswc_file;
    outswc_file = QString(p4DImage->getFileName())  + "ray_model.swc";
    //QString raynkdh = QString ("ray-shooting.swc");
    writeSWC_file(outswc_file,nt);
    return 1;

}

int curve_detection(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandle curwin = callback.currentImageWindow();
    if(!curwin)
    {
            v3d_msg("No image is open.");
            return -1;
    }

    Image4DSimple *p4DImage = callback.getImage(curwin);
    V3DLONG sz[3];
    sz[0] = p4DImage->getXDim();
    sz[1] = p4DImage->getYDim();
    sz[2] = p4DImage->getZDim();


    //int ray_numbers_2d = 128;
    int thres_2d = 45;
    //int ray_length_2d = 16;


    //set update the dialog
    QDialog * dialog = new QDialog();


    if(p4DImage->getZDim() > 1)
            dialog->setWindowTitle("3D neuron image tip point detection Based on Ray-shooting algorithm");
    else
            dialog->setWindowTitle("2D neuron image tip point detection Based on Ray-shooting algorithm");

QGridLayout * layout = new QGridLayout();

//QSpinBox * ray_numbers_2d_spinbox = new QSpinBox();
//ray_numbers_2d_spinbox->setRange(1,1000);
//ray_numbers_2d_spinbox->setValue(ray_numbers_2d);

QSpinBox * thres_2d_spinbox = new QSpinBox();
thres_2d_spinbox->setRange(-1, 255);
thres_2d_spinbox->setValue(thres_2d);

//QSpinBox * ray_length_2d_spinbox = new QSpinBox();
//ray_length_2d_spinbox->setRange(1,p4DImage->getXDim());
//ray_length_2d_spinbox->setValue(ray_length_2d);


//layout->addWidget(new QLabel("ray numbers"),0,0);
//layout->addWidget(ray_numbers_2d_spinbox, 0,1,1,5);

layout->addWidget(new QLabel("intensity threshold"),1,0);
layout->addWidget(thres_2d_spinbox, 1,1,1,5);

//layout->addWidget(new QLabel("ray length"),2,0);
//layout->addWidget(ray_length_2d_spinbox, 2,1,1,5);


QHBoxLayout * hbox2 = new QHBoxLayout();
QPushButton * ok = new QPushButton(" ok ");
ok->setDefault(true);
QPushButton * cancel = new QPushButton("cancel");
hbox2->addWidget(cancel);
hbox2->addWidget(ok);

layout->addLayout(hbox2,6,0,1,6);
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
//get the dialog return values
//ray_numbers_2d = ray_numbers_2d_spinbox->value();
thres_2d = thres_2d_spinbox->value();
//ray_length_2d = ray_length_2d_spinbox->value();
if (dialog)
    {
            delete dialog;
            dialog=0;
            cout<<"delete dialog"<<endl;

    }

    if (dialog)
    {
        delete dialog;
        dialog=0;
        cout<<"delete dialog"<<endl;
    }

    unsigned char* datald=0;
    datald = p4DImage->getRawData();
    V3DLONG size_image=sz[0]*sz[1]*sz[2];

    v3d_msg("start complete");
    unsigned char *image_binary=0;
    try{image_binary=new unsigned char [size_image];}
    catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}
    for(V3DLONG i = 0; i < size_image; i++)
    {
        if(datald[i] > thres_2d)
            image_binary[i] = 255;
        else
            image_binary[i] = 0;
    }
    v3d_msg("segment was complete");

    v3d_msg("start find 2D cureve points");
    V3DLONG nx=sz[0];
    V3DLONG ny=sz[1];
    V3DLONG nz=sz[2];

     LandmarkList curlist;
     LocationSimple s;
              unsigned char *new_datald=new unsigned char [nx*ny];
              unsigned char *imagesobelx=new unsigned char [nx*ny];
              unsigned char *imagesobely=new unsigned char [nx*ny];
              float  *xx=new float[nx*ny];
              float *yy=new float[nx*ny];
              float *xy=new float[nx*ny];
              float *Gxx=new float[nx*ny];
              float *Gyy=new float[nx*ny];
              float *Gxy=new float[nx*ny];
              float *Hresult=new float[nx*ny];
     for(int z_size=0;z_size<nz;z_size++)
     {
         for (int i = 0; i < ny ; i++)
         {
             for (int j = 1; j < nx ; j++)
             {
                  new_datald[i*nx+j]=image_binary[z_size*nx*ny+i*nx+j];


             }
         }
         for (int i = 1; i < ny - 1; i++)
                 {
                      for (int j = 1; j < nx - 1; j++)
                       {
                               //通过指针遍历图像上每一个像素
                               double gradY = new_datald[(i + 1)*nx + j - 1] + new_datald[(i + 1)*nx + j] * 2 + new_datald[(i + 1)*nx + j + 1] - new_datald[(i - 1)*nx + j - 1] - new_datald[(i - 1)*nx + j] * 2 - new_datald[(i - 1)*nx + j + 1];
                               imagesobelx[i*nx + j] = abs(gradY);
                               //v3d_msg(QString("clone is %1").arg(dst[200]));
                              // v3d_msg(QString("imagesobelx is %1").arg(dst[200]));

                               double gradX = new_datald[(i - 1)*nx + j + 1] + new_datald[i*nx + j + 1] * 2 + new_datald[(i + 1)*nx + j + 1] - new_datald[(i - 1)*nx + j - 1] - new_datald[i*nx + j - 1] * 2 - new_datald[(i + 1)*nx + j - 1];
                               imagesobely[i*nx + j] = abs(gradX);
                       }
                 }
//         v3d_msg(QString("new_datald is %1").arg(new_datald[200]));
//         v3d_msg(QString("imagesobelx is %1").arg(imagesobelx[200]));
//         v3d_msg(QString("imagesobely is %1").arg(imagesobely[200]));
                        //将梯度数组转换成8位无符号整型
                       //convertScaleAbs(imageSobelX, imageSobelX);
                       //convertScaleAbs(imageSobelY, imageSobelY);


                        for (int i = 0; i < ny; i++)
                         {
                              for (int j = 0; j < nx; j++)
                              {
                                   xx[i*nx+j] = imagesobelx[i*nx+j]*imagesobelx[i*nx+j];
                              }
                         }

                        for (int i = 0; i < ny; i++)
                         {
                              for (int j = 0; j < nx; j++)
                              {
                                   yy[i*nx+j] = imagesobely[i*nx+j]*imagesobely[i*nx+j];
                              }
                         }

                        for (int i = 0; i < ny; i++)
                         {
                              for (int j = 0; j < nx; j++)
                              {
                                   xy[i*nx+j] = imagesobelx[i*nx+j]*imagesobely[i*nx+j];
                              }
                         }

                        MyGaussianBlur(xx,Gxx,3,sz[0],sz[1]);
                        MyGaussianBlur(yy,Gyy,3,sz[0],sz[1]);
                        MyGaussianBlur(xy,Gxy,3,sz[0],sz[1]);
                        harrisResponse(Gxx,Gyy,Gxy,Hresult,0.05,sz[0],sz[1]);

                        int r = 3 / 2;

                        for (int i = r; i < ny-r; i++)
                        {
                            for (int j = r; j < nx - r; j++)
                            {
                                if (
                                    Hresult[i*nx+j] > Hresult[(i-1)*nx+j] &&
                                    Hresult[i*nx+j] > Hresult[(i-1)*nx+j-1] &&
                                    Hresult[i*nx+j] > Hresult[(i-1)*nx+j+1] &&
                                    Hresult[i*nx+j] > Hresult[i*nx+j-1] &&
                                    Hresult[i*nx+j] > Hresult[i*nx+j+1] &&
                                    Hresult[i*nx+j] > Hresult[(i+1)*nx+j-1] &&
                                    Hresult[i*nx+j] > Hresult[(i+1)*nx+j] &&
                                    Hresult[i*nx+j] > Hresult[(i+1)*nx+j+1])
                                {

                                    //v3d_msg(QString("Hresult[i*nx+j] is %1").arg(Hresult[i*nx+j]));
                                    if (Hresult[i*nx+j] > 10000)
                                    {
                                        s.x=j+1;
                                        s.y=i+1;
                                        s.z=z_size;
                                        s.radius=1;
                                        s.color = random_rgba8(255);
                                        curlist << s;
                                    }
                                }

                            }
                        }

     }
  v3d_msg(QString("numble of marker is %1").arg(curlist.size()));
  v3d_msg("2D cureve points were complete");
//  for(int i=0;i<curlist.size();i++)
//      for(int j=i+1;j<curlist.size();j++)
//  {

//      {
//          if(square(curlist[j].x-curlist[i].x)+square(curlist[j].y-curlist[i].y)+square(curlist[j].z-curlist[i].z)<150)
//              {
//                  curlist.removeAt(j);
//                  j = j - 1;
//              }
//      }
//  }

  callback.setLandmark(curwin, curlist);
  if(image_binary) {delete []image_binary; image_binary = 0;}
  return 1;



}
int gassion_filter(V3DPluginCallback2 &callback, QWidget *parent)
{

                  v3dhandle curwin = callback.currentImageWindow();
                  if(!curwin)
                  {
                                  v3d_msg("No image is open.");
                                  return -1;
                  }

                                        Image4DSimple *p4DImage = callback.getImage(curwin);
                                        V3DLONG sz[3];
                                        sz[0] = p4DImage->getXDim();
                                        sz[1] = p4DImage->getYDim();
                                        sz[2] = p4DImage->getZDim();
                                        V3DLONG nx=sz[0],ny=sz[1],nz=sz[2];

                                        unsigned char* datald=0;
                                        datald = p4DImage->getRawData();
                                        unsigned char *new_new_datald=new unsigned char[nx*ny*nz];
                                        float *new_datald=new float[nx*ny*nz];
                                        float *Gxx=new float [nx*ny*nz];


                         for(V3DLONG k=0;k<nz;k++)
                            {

                                    for(V3DLONG j=0;j<ny;j++)
                                       {
                                           for(V3DLONG i=0;i<nx;i++)
                                               {
                                                 new_datald[j*nx+i]=datald[k*nx*ny+j*nx+i];

                                               }
                                       }


                                     MyGaussianBlur(new_datald,Gxx,5,sz[0],sz[1]);
                                    // v3d_msg(QString("new guassion is %1").arg(Gxx[200]));

                                        for(V3DLONG j=0;j<ny;j++)
                                           {
                                               for(V3DLONG i=0;i<nx;i++)
                                                   {
                                                       new_new_datald[k*nx*ny+j*nx+i]=Gxx[j*nx+i];
                                                   }

                                           }

                             }

                         v3d_msg("display the filer result");
                         ImagePixelType pixeltype = p4DImage->getDatatype();
                         Image4DSimple * new4DImage = new Image4DSimple();
                         new4DImage->setData((unsigned char *)new_new_datald, sz[0], sz[1], sz[2], 1, pixeltype);
                         v3dhandle newwin = callback.newImageWindow();
                         callback.setImage(newwin, new4DImage);
                         callback.setImageName(newwin, "3D adaptive enhancement result");
                         callback.updateImageWindow(newwin);
                         v3d_msg("free memory");
                        // if(Gxx[0]) {delete []Gxx; Gxx = 0;}
                        // if(new_datald[0]) {delete []new_datald; new_datald = 0;}
                         return 1;
  }
int MIPZSlices( V3DPluginCallback2 &callback, QWidget *parent)
{

        v3dhandle curwin = callback.currentImageWindow();
        Image4DSimple* pp = callback.getImage(curwin);
        if (!curwin || !pp)
        {
            v3d_msg("You don't have any image open in the main window.");
            return 0;
        }

        V3DLONG sz0 = pp->getXDim();
        V3DLONG sz1 = pp->getYDim();
        V3DLONG sz2 = pp->getZDim();
        V3DLONG sz3 = pp->getCDim();

        //

        V3DLONG startnum=0, increment=1, endnum=sz2-1;

        QString m_InputFileName = callback.getImageName(curwin);

        //copy data
        //V3DLONG sz2_new = ceil(double(endnum-startnum+1)/increment);

        Image4DSimple p4DImage;
        if (!mip_z_slices(pp, p4DImage, startnum, increment, endnum))
            return 0;

        v3dhandle newwin;
        if(QMessageBox::Yes == QMessageBox::question (0, "",
            QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
            newwin = callback.currentImageWindow();
        else
            newwin = callback.newImageWindow();

        callback.setImage(newwin, &p4DImage);
        callback.setImageName(newwin, m_InputFileName + "_mip_"  + ".v3draw");
        callback.updateImageWindow(newwin);
        return 1;
}




