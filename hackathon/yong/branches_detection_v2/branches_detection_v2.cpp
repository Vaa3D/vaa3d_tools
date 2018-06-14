#include "branches_detection_v2.h"
#include "branches_detection_v2_plugin.h"
#include "rayshooting_func.h"
#include "rayshooting_func.cpp"
#include "v3d_interface.h"
#include "../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.h"
#include <iostream>
#include <vector>
using namespace std;
#define PI 3.1415926
#define NTDIS(a,b) (sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z)))
typedef int BIT32_UNIT;
#define b_VERBOSE_PRINT 0
bool branches_detection_v2(V3DPluginCallback2 &callback, QWidget *parent,BRANCH_LS_PARA &P)
{
    //NeuronTree nt_terafly = callback.getSWCTeraFly();
    LandmarkList marker_v,marker_rebase;
    const Image4DSimple *curr_win = callback.getImageTeraFly();

    LandmarkList terafly_landmarks = callback.getLandmarkTeraFly();
    for(V3DLONG i=0;i<terafly_landmarks.size();i++)
    {
        get_sub_terafly(callback,P);
        LandmarkList marker,marker2;
        branch_detection(callback,P,marker);

        cout<<"marker.size = "<<marker.size()<<endl;
        v3d_msg("check marker size");
        cout<<"P"<<P.xb<<"  "<<P.yb<<"  "<<P.zb<<endl;
        for(V3DLONG i=0;i<marker.size();i++)
        {
            marker[i].x = marker[i].x+P.xb;
            marker[i].y = marker[i].y+P.yb;
            marker[i].z = marker[i].z+P.zb;

            cout<<"marker = "<<marker[i].x<<"  "<<marker[i].y<<"  "<<marker[i].y<<"  "<<endl;
        }
        QList<ImageMarker> markerlist;
        QString filename = "save.marker";
        for(V3DLONG i=0;i<marker.size();i++)
        {
            ImageMarker m;
            m.x = marker[i].x;
            m.y = marker[i].y;
            m.z = marker[i].z;
            m.color.a = 0;
            m.color.b = 0;
            //m.color.c = 0;
            m.color.g = 0;
            m.color.r = 0;
            markerlist.push_back(m);
        }
        writeMarker_file(filename,markerlist);
        double ratio = curr_win->getRezX()/curr_win->getXDim();

        for(V3DLONG j=0;j<marker.size();j++)
        {
            int ind = 0;
            for(V3DLONG k=j+1;k<marker.size();k++)
            {
                double dist = NTDIS(marker[j],marker[k]);
                cout<<"dist___________"<<dist/ratio<<endl;
                if(dist/ratio<4)    //need to be modified
                {
                    ind = 1;
                }
            }
            if(ind==0)
            {
                marker2.push_back(marker[j]);
            }
            cout<<"marker2.size = "<<marker2.size()<<endl;
        }
        //marker2 = marker; //add temp
        if(marker_v.size()==0)
        {
            for(V3DLONG k=0;k<marker2.size();k++)
            {
                marker_v.push_back(marker2[k]);
            }
        }
        else
        {
            for(V3DLONG j=0;j<marker_v.size();j++)
            {
                cout<<"hahhahaa = "<<marker_v.size()<<endl;
                for(V3DLONG k=0;k<marker2.size();k++)
                {
                    double dis = NTDIS(marker2[k],marker_v[j]);
                    cout<<"dis = "<<dis<<endl;
                    if(dis>4)
                    {
                        marker_rebase.push_back(marker2[k]);
                    }
                }
            }
            for(V3DLONG j=0;j<marker_rebase.size();j++)
            {
                marker_v.push_back(marker_rebase[j]);
            }
        }
    }


    if(marker_v.size()>0)
    {
        callback.setLandmarkTeraFly(marker_v);
        return true;
    }
    else
    {
        v3d_msg("this area don't have any branch");return false;
    }

}

bool get_sub_terafly(V3DPluginCallback2 &callback,BRANCH_LS_PARA &PA)
{
    //QString inimg_file = callback.getPathTeraFly();
    LandmarkList terafly_landmarks = callback.getLandmarkTeraFly();

    for(V3DLONG i=0;i<terafly_landmarks.size();i++)
    {
        LocationSimple t;
        t.x = terafly_landmarks[i].x;
        t.y = terafly_landmarks[i].y;
        t.z = terafly_landmarks[i].z;

        V3DLONG im_cropped_sz[4];

        unsigned char * im_cropped = 0;
        V3DLONG pagesz;
        double l_x = 256;
        double l_y = 256;
        double l_z = 128;

        V3DLONG xb = t.x-l_x;
        V3DLONG xe = t.x+l_x-1;
        V3DLONG yb = t.y-l_y;
        V3DLONG ye = t.y+l_y-1;
        V3DLONG zb = t.z-l_z;
        V3DLONG ze = t.z+l_z-1;
        pagesz = (xe-xb+1)*(ye-yb+1)*(ze-zb+1);
        im_cropped_sz[0] = xe-xb+1;
        im_cropped_sz[1] = ye-yb+1;
        im_cropped_sz[2] = ze-zb+1;
        im_cropped_sz[3] = 1;

        try {im_cropped = new unsigned char [pagesz];}
        catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return false;}

        //cout<<"begin ==================="<<xb<<"  "<<yb<<"  "<<zb<<"  "<<endl;
        //cout<<"end   ==================="<<xe<<"  "<<ye<<"  "<<ze<<"  "<<endl;
        v3d_msg("test!");


        im_cropped = callback.getSubVolumeTeraFly(PA.inimg_file.toStdString(),xb,xe+1,
                                              yb,ye+1,zb,ze+1);


        QString outimg_file;
        outimg_file = "test.tif";
       // outswc_file = "test.swc";
       // writeSWC_file(outswc_file,outswc);

       // export_list2file(outswc,outswc_file,outswc_file);
        simple_saveimage_wrapper(callback, outimg_file.toStdString().c_str(),(unsigned char *)im_cropped,im_cropped_sz,1);
        //if(im_cropped) {delete []im_cropped; im_cropped = 0;}

  //  simple_saveimage_wrapper(callback, outimg_file.toStdString().c_str(),(unsigned char *)im_cropped,im_cropped_sz,1);
    PA.data1d = im_cropped;
    //PA.nt = outswc;
    PA.in_sz[0] = 2*l_x;
    PA.in_sz[1] = 2*l_y;
    PA.in_sz[2] = 2*l_z;
    PA.xb = xb;
    PA.yb = yb;
    PA.zb = zb;
    }

}

int branch_detection(V3DPluginCallback2 &callback,BRANCH_LS_PARA &PA,LandmarkList &curlist)
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


        layout->addWidget(new QLabel("ray numbers"),0,0);
        layout->addWidget(ray_numbers_2d_spinbox, 0,1,1,5);

        layout->addWidget(new QLabel("intensity threshold"),1,0);
        layout->addWidget(thres_2d_spinbox, 1,1,1,5);

        layout->addWidget(new QLabel("ray length"),2,0);
        layout->addWidget(ray_length_2d_spinbox, 2,1,1,5);


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

       vector<NeuronSWC> point_pool;

       for(V3DLONG k=1;k<nz;k++)
       {
          for(V3DLONG j=1;j<ny-1;j++)
          {
              for(V3DLONG i=1;i<nx-1;i++)
              {


                  bool flag5=image_binary[k*nx*ny+nx*j+i];
                  if (flag5)
                  {
                      bool flag1=image_binary[k*nx*ny+nx*j+i-1];
                      bool flag4=image_binary[k*nx*ny+nx*j+i+1];
                      bool flag2=image_binary[k*nx*ny+nx*(j-1)+i];
                      bool flag3=image_binary[k*nx*ny+nx*(j+1)+i];
                      int sum=flag1+flag2+flag3+flag4;
                      if((sum>0)&&(sum<4))
                      {

    //                   v3d_msg(QString("x is %1,y is %2, z is %3").arg(i).arg(j).arg(k));
                         NeuronSWC pp;
                         pp.x=i;
                         pp.y=j;
                         pp.z=k;
                         point_pool.push_back(pp);
    //                   v3d_msg(QString("x is %1,y is %2, z is %3").arg(pp.x).arg(pp.y).arg(pp.z));
                      }

                  }


              }
          }
       }
      v3d_msg("2D cureve points were complete");
      v3d_msg(QString("sum is %1").arg(point_pool.size()));



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
       V3DLONG M=sz[1]; //y
       V3DLONG N=sz[0]; //x
       V3DLONG P=sz[2]; //z
       V3DLONG pagesz=M*N*P;
      // v3d_msg(QString("sz[0] is %1,sz[1] is %2, sz[2] is %3").arg(N).arg(M).arg(P));
       unsigned char *image_mip=0;
       try{image_mip=new unsigned char [pagesz];}
       catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}
       for(V3DLONG iy = 0; iy < M; iy++)
       {
           V3DLONG offsetj = iy*N;
           for(V3DLONG ix = 0; ix < N; ix++)
           {
               int max_mip = 0;
               for(V3DLONG iz = 0; iz < P; iz++)
               {
                   V3DLONG offsetk = iz*M*N;
                   if(image_binary[offsetk + offsetj + ix] >= max_mip)
                   {
                       image_mip[iy*N + ix] = image_binary[offsetk + offsetj + ix];
                       max_mip = image_binary[offsetk + offsetj + ix];
                      // v3d_msg(QString("max_mip is %1").arg(max_mip));
                   }
               }
           }
       }


       cout<<"begin circle "<<endl;

      // LandmarkList curlist;
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
       if(datald){delete []datald;datald=0;}

       return 1;
//       if(data1d)
//       {
//           delete []data1d;
//           data1d =0;
//       }
       return true;
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

int raymodel(V3DPluginCallback2 &callback, QWidget *parent)
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


    float x_point=0,y_point=0,z_point=0;
    int ray_numble=64;
    int length_numble=8;

    QDialog * dialog = new QDialog();
    QGridLayout * layout = new QGridLayout();

    QSpinBox * X_point_spinbox = new QSpinBox();
    X_point_spinbox->setRange(1,sz[0]);
    X_point_spinbox->setValue(x_point);

    QSpinBox * Y_point_spinbox = new QSpinBox();
    Y_point_spinbox->setRange(1, sz[1]);
    Y_point_spinbox->setValue(y_point);

    QSpinBox * Z_point_spinbox = new QSpinBox();
    Z_point_spinbox->setRange(1,sz[2]);
    Z_point_spinbox->setValue(z_point);

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

    layout->addWidget(new QLabel("z location"),2,0);
    layout->addWidget(Z_point_spinbox, 2,1,1,5);

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
    z_point = Z_point_spinbox->value();
    ray_numble = number_ray_spinbox->value();
    length_numble = length_ray_spinbox->value();

    if (dialog)
    {
        delete dialog;
        dialog=0;
        cout<<"delete dialog"<<endl;
    }


    cout<<"complete this dialog"<<endl;

    cout<<"start the ray-shooting model"<<endl;

    vector<float> x_lac;
    vector<float> y_lac;
    float ang = 2*PI/ray_numble;

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

    cout<<"start save swc"<<endl;
    NeuronTree nt;
    QList <NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();

    for(V3DLONG i=0;i<x_lac.size();i++)
    {
        NeuronSWC s;
        s.x=  x_lac[i];
        s.y = y_lac[i];
        s.z = sz[2]/2;
        s.radius = 1;
        s.type = 3;
        listNeuron.append(s);
        hashNeuron.insert(s.n, listNeuron.size()-1);
    }

    nt.n = -1;
    nt.on = true;
    nt.listNeuron = listNeuron;
    nt.hashNeuron = hashNeuron;
    QString outswc_file;
    outswc_file = QString(p4DImage->getFileName())  + "ray_model.swc";
    //QString raynkdh = QString ("ray-shooting.swc");
    writeSWC_file(outswc_file,nt);

   // v3d_msg(QString("okokookokokoko"));
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
                         if(Gxx[0]) {delete []Gxx; Gxx = 0;}
                         if(new_datald[0]) {delete []new_datald; new_datald = 0;}
                         return 1;
  }
