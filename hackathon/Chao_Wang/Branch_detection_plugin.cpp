/* Branch_detection_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-5-20 : by Chao Wang
 */
#include"Branch_detection_plugin.h"
#include <iostream>
#include "basic_surf_objs.h"
#include "v3d_message.h"
#include <vector>
#include "new_ray-shooting.h"
#include "../../../../vaa3d_tools/released_plugins/v3d_plugins/bigneuron_siqi_rivuletv3d/fastmarching/fastmarching_dt.h"
#define PI 3.1415926
#define based_num 1;
#include <algorithm>
#include"ClusterAnalysis.h"
#include"branch_detection_model.h"

using namespace std;
struct delete_piont
{
    V3DLONG xx;
    V3DLONG yy;
};

Q_EXPORT_PLUGIN2(Branch_detection, TestPlugin)
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
        <<tr("one_plane_3D_branch_poins_detection")
        <<tr("2D_branch_points_detection")
        <<tr("thin_branch_points_detection")
        <<tr("display")
        <<tr("display_length")
        <<tr("create groundtruth")
        <<tr("2D rotate algorithm display")
        <<tr("3D rotate algorithm display")
        <<tr("rotate 2D image")
        <<tr("rotate 3D image")
        <<tr("fast rotate 3D image")
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
        if (menu_name == tr("one_plane_3D_branch_poins_detection"))
        {
           int flag=branch_detection_single(callback,parent);
           if (flag==1)
           {
              v3d_msg(tr("3D branch points detection completed!"));
           }
        }
        else if(menu_name==tr("2D_branch_points_detection"))
        {
            int flag=use_rayshooting(callback,parent);
            if(flag==1)
            v3d_msg("2D branch points completed ");
        }
        else if(menu_name==tr("display"))
        {
            int flag=display_rayshooting_model_in_xyproject(callback,parent);
            if(flag==1)
            v3d_msg("2D branch points completed ");
        }
        else if(menu_name==tr("display_length"))
        {
            int flag=display_length_model(callback,parent);
            if(flag==1)
            v3d_msg("length have dispalyed ");
        }

        else if(menu_name==tr("thin_branch_points_detection"))
        {
            int flag=thin(callback,parent);
            if(flag==1)
            v3d_msg("2D branch points completed ");
        }
        else if(menu_name==tr("create groundtruth"))
        {
            int flag=create_groundtruth(callback,parent);
            if(flag==1)
            v3d_msg("2D branch points completed ");
        }
        else if(menu_name==tr("2D rotate algorithm display"))
        {
            int flag=rotate_2D_display_point(callback,parent);
            if(flag==1)
            v3d_msg("2D branch points completed ");
        }
        else if(menu_name==tr("3D rotate algorithm display"))
        {
            int flag=rotate_3D_display_point(callback,parent);
            if(flag==1)
            v3d_msg("3D branch points have displaied ");
        }
        else if(menu_name==tr("rotate 2D image"))
        {
            int flag=rotate_2D_image(callback,parent);
            if(flag==1)
            v3d_msg("2D branch points completed ");
        }
        else if(menu_name==tr("rotate 3D image"))
        {
            int flag=rotate_3D_image(callback,parent);
            if(flag==1)
            v3d_msg("3D branch points completed ");
        }
        else if(menu_name==tr("fast rotate 3D image"))
        {
            int flag=rotate_3D_slice_image(callback,parent);
            if(flag==1)
            v3d_msg("3D branch points completed ");
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
int branch_detection_single(V3DPluginCallback2 &callback, QWidget *parent)
{
            // 1 - Obtain the current 4D image pointer
            v3dhandle curwin = callback.currentImageWindow();
            if(!curwin)
            {
                    v3d_msg("No image is open.");
                    return -1;
            }
            Image4DSimple *p4DImage = callback.getImage(curwin);
            int ray_numbers_2d = 64;
            int thres_2d = 30;
            int ray_length_2d = 64;
            int num_layer=1;


            //set update the dialog
            QDialog * dialog = new QDialog();

            if(p4DImage->getZDim() > 1)
                    dialog->setWindowTitle("3D neuron image branch point detection Based on Ray-shooting algorithm");
            else
                    dialog->setWindowTitle("2D neuron image branch point detection Based on Ray-shooting algorithm");

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

        QSpinBox * num_layer_spinbox = new QSpinBox();
        num_layer_spinbox->setRange(1,255);
        num_layer_spinbox->setValue(num_layer);

        layout->addWidget(new QLabel("ray numbers"),0,0);
        layout->addWidget(ray_numbers_2d_spinbox, 0,1,1,5);

        layout->addWidget(new QLabel("intensity threshold"),1,0);
        layout->addWidget(thres_2d_spinbox, 1,1,1,5);

        layout->addWidget(new QLabel("ray length"),2,0);
        layout->addWidget(ray_length_2d_spinbox, 2,1,1,5);

        layout->addWidget(new QLabel("size of layer"),3,0);
        layout->addWidget(num_layer_spinbox, 3,1,1,5);


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
        num_layer = num_layer_spinbox->value();

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
       V3DLONG size_image=sz[0]*sz[1];

       V3DLONG nx=sz[0];
       V3DLONG ny=sz[1];
       V3DLONG nz=sz[2];
       V3DLONG spage=nx*ny*nz;


       cout<<"create 2D_ray"<<endl;

       vector<vector<float> > ray_x(ray_numbers_2d,vector<float>(ray_length_2d)), ray_y(ray_numbers_2d,vector<float>(ray_length_2d));
       vector<vector<float> > ray_x_left(ray_numbers_2d,vector<float>(ray_length_2d)), ray_y_left(ray_numbers_2d,vector<float>(ray_length_2d));
       vector<vector<float> > ray_x_right(ray_numbers_2d,vector<float>(ray_length_2d)), ray_y_right(ray_numbers_2d,vector<float>(ray_length_2d));

       float ang = 2*PI/ray_numbers_2d;
       float x_dis, y_dis, lx_dis, ly_dis, rx_dis, ry_dis;
       for(int i = 0; i < ray_numbers_2d; i++)
       {
           x_dis = cos(ang*(i+1));
           y_dis = sin(ang*(i+1));
           lx_dis = -sin(ang*(i+1));
           ly_dis = cos(ang*(i+1));
           rx_dis = sin(ang*(i+1));
           ry_dis = -cos(ang*(i+1));

           for(int j = 0; j < ray_length_2d; j++)
           {
               ray_x[i][j] = x_dis*(j+1);
               ray_y[i][j] = y_dis*(j+1);

               ray_x_left[i][j] = x_dis*(j+1)+lx_dis;
               ray_y_left[i][j] = y_dis*(j+1)+ly_dis;

               ray_x_right[i][j] = x_dis*(j+1)+rx_dis;
               ray_y_right[i][j] = y_dis*(j+1)+ry_dis;
           }
       }

       unsigned char *image_mip;
       unsigned char *image_binary;
       unsigned char *old_image_binary;

       try{image_mip=new unsigned char [spage];}
       catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

       try{image_binary=new unsigned char [size_image];}
       catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

       try{old_image_binary=new unsigned char [size_image];}
       catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

       int new_neighbor[9];
       LandmarkList curlist;
       LocationSimple s;
       for(int size_z=0;size_z<nz;size_z=size_z+num_layer)
       {
           if((size_z+num_layer)<nz)
           {
               mip(nx,ny,size_z,datald,image_mip,num_layer);
               thres_segment(size_image,image_mip,image_binary,thres_2d);
               for(V3DLONG a=0;a<nx*ny;a++)
               {
                   old_image_binary[a]=image_binary[a];
               }
               skeletonization(nx,ny,image_binary);
               for(V3DLONG j=1;j<ny-1;j++)
               {
                   for(V3DLONG i=1;i<nx-1;i++)
                   {
                       if(image_binary[j*nx+i]>0)
                       {
                           if (image_binary[(j-1)*nx+i-1] == 255) new_neighbor[1] = 1;
                           else  new_neighbor[1] = 0;
                           if (image_binary[(j-1)*nx+i] == 255) new_neighbor[2] = 1;
                           else  new_neighbor[2] = 0;
                           if (image_binary[(j-1)*nx+i+1] == 255) new_neighbor[3] = 1;
                           else  new_neighbor[3] = 0;
                           if (image_binary[(j)*nx+i + 1] == 255) new_neighbor[4] = 1;
                           else  new_neighbor[4] = 0;
                           if (image_binary[(j+1)*nx+i+1] == 255) new_neighbor[5] = 1;
                           else  new_neighbor[5] = 0;
                           if (image_binary[(j+1)*nx+i] == 255) new_neighbor[6] = 1;
                           else  new_neighbor[6] = 0;
                           if (image_binary[(j+1)*nx+i-1] == 255) new_neighbor[7] = 1;
                           else  new_neighbor[7] = 0;
                           if (image_binary[(j)*nx+i-1] == 255) new_neighbor[8] = 1;
                           else  new_neighbor[8] = 0;

                           int sum=0;
                           int cnp;
                           for(int k=1;k<=7;k++)
                           {
                               sum=sum+abs(new_neighbor[k]-new_neighbor[k+1]);
                           }
                           int num=abs(new_neighbor[1]-new_neighbor[8]);
                           cnp=0.5*(sum+num);
                           if(cnp==3)
                           {
                               int flag1=rayinten_2D(j,i,ray_numbers_2d ,ray_length_2d,ray_x, ray_y, ray_x_left, ray_y_left, ray_x_right, ray_y_right, old_image_binary,nx,ny);
                               int flag2=rayinten_2D(j-1,i-1,ray_numbers_2d ,ray_length_2d,ray_x, ray_y, ray_x_left, ray_y_left, ray_x_right, ray_y_right, old_image_binary,nx,ny);

                               int flag3=rayinten_2D(j-1,i,ray_numbers_2d ,ray_length_2d,ray_x, ray_y, ray_x_left, ray_y_left, ray_x_right, ray_y_right, old_image_binary,nx,ny);

                               int flag4=rayinten_2D(j-1,i+1,ray_numbers_2d ,ray_length_2d,ray_x, ray_y, ray_x_left, ray_y_left, ray_x_right, ray_y_right, old_image_binary,nx,ny);

                               int flag5=rayinten_2D(j,i-1,ray_numbers_2d ,ray_length_2d,ray_x, ray_y, ray_x_left, ray_y_left, ray_x_right, ray_y_right, old_image_binary,nx,ny);

                               int flag6=rayinten_2D(j,i+1,ray_numbers_2d ,ray_length_2d,ray_x, ray_y, ray_x_left, ray_y_left, ray_x_right, ray_y_right, old_image_binary,nx,ny);

                               int flag7=rayinten_2D(j+1,i-1,ray_numbers_2d ,ray_length_2d,ray_x, ray_y, ray_x_left, ray_y_left, ray_x_right, ray_y_right, old_image_binary,nx,ny);

                               int flag8=rayinten_2D(j+1,i,ray_numbers_2d ,ray_length_2d,ray_x, ray_y, ray_x_left, ray_y_left, ray_x_right, ray_y_right, old_image_binary,nx,ny);
                               int flag9=rayinten_2D(j+1,i+1,ray_numbers_2d ,ray_length_2d,ray_x, ray_y, ray_x_left, ray_y_left, ray_x_right, ray_y_right, old_image_binary,nx,ny);

                               int sum_flag=flag1+flag2+flag3+flag4+flag5+flag6+flag7+flag8+flag9;

                               cout<<"sum_flag is "<<sum_flag<<endl;
                               if(sum_flag>=0)
                               {
                                   s.x=i;
                                   s.y=j;
                                   unsigned char max_z=0;
                                   int loc_z;
                                   for(int k=size_z;k<size_z+num_layer;k++)
                                   {
                                       unsigned char sum_pixe;
                                       sum_pixe=datald[k*nx*ny+j*nx+i]+datald[k*nx*ny+j*nx+i+1]+datald[k*nx*ny+j*nx+i-1]+
                                               datald[k*nx*ny+(j+1)*nx+i]+datald[k*nx*ny+(j-1)*nx+i]+
                                               datald[k*nx*ny+(j+1)*nx+i-1]+datald[k*nx*ny+(j+1)*nx+i+1]+
                                               datald[k*nx*ny+(j-1)*nx+i-1]+datald[k*nx*ny+(j-1)*nx+i+1];

                                       if(sum_pixe>max_z)
                                       {
                                           max_z=sum_pixe;
                                           //v3d_msg(QString("max_z is %1").arg(max_z));
                                           loc_z=k;
                                           s.z=k;
                                       }
                                   }
                                   s.radius=1;
                                   s.color = random_rgba8(255);
                                   curlist << s;
                                   num++;

                               }
                       }
                     }

                   }
               }


           }
         }

       for(V3DLONG i=0;i<curlist.size();i++)
       {
           unsigned char max_pixe=0;
           for(V3DLONG j=1;j<nz;j++)
           {
               unsigned char z_pixe=p4DImage->getValueUINT8(curlist[i].x,curlist[i].y,j,0);
               if(z_pixe>max_pixe)
               {
                   max_pixe=z_pixe;
               }

           }
           unsigned char new_pixe=p4DImage->getValueUINT8(curlist[i].x,curlist[i].y,curlist[i].z,0);
           if(new_pixe<max_pixe)
           {
              curlist.removeAt(i);
           }

       }


       for(double i=0;i<curlist.size();i++)
       {
           for(double j=i+1;j<curlist.size();j++)
           {
               if(square(curlist[j].x-curlist[i].x)+square(curlist[j].y-curlist[i].y)+square(curlist[j].z-curlist[i].z)<200)
               {
                   curlist[i].x=(curlist[j].x+curlist[i].x)/2;
                   curlist[i].y=(curlist[j].y+curlist[i].y)/2;
                   curlist[i].z=(curlist[j].z+curlist[i].z)/2;
                   curlist.removeAt(j);
                   j=j-1;
               }
           }
       }


        callback.setImage(curwin, p4DImage);
        callback.updateImageWindow(curwin);
        v3d_msg(QString("numble of marker is %1").arg(curlist.size()));
        callback.setLandmark(curwin, curlist);
//       if(image_binary) {delete []image_binary; image_binary = 0;}
//        if(image_mip) {delete []image_mip; image_mip = 0;}
//        if(datald){delete []datald;datald=0;}
       if(old_image_binary){delete []old_image_binary;old_image_binary=0;}

        return 1;
}

void printHelp()
{
    cout<<"\nThis is a demo plugin to branch point in an image. by Chao Wang  2018-04"<<endl;
    cout<<"if you have any question please contact hunchaowang@hnu.edu.cn"<<endl;
    return;
}

int use_rayshooting(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandle curwin = callback.currentImageWindow();
    if(!curwin)
    {
                    v3d_msg("No image is open.");
                    return -1;
    }
      Image4DSimple *p4DImage = callback.getImage(curwin);

      int ray_numbers_2d = 64;
      int thres_2d = 25;
      int ray_length_2d = 64;
      int window_size=2;


      //set update the dialog
      QDialog * dialog = new QDialog();


      if(p4DImage->getZDim() > 1)
              dialog->setWindowTitle("3D neuron image branch point detection Based on Ray-shooting algorithm");
      else
              dialog->setWindowTitle("2D neuron image branch point detection Based on Ray-shooting algorithm");

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

  QSpinBox * window_size_spinbox = new QSpinBox();
  window_size_spinbox->setRange(1,1000);
  window_size_spinbox->setValue(window_size);

  layout->addWidget(new QLabel("ray numbers"),0,0);
  layout->addWidget(ray_numbers_2d_spinbox, 0,1,1,5);

  layout->addWidget(new QLabel("intensity threshold"),1,0);
  layout->addWidget(thres_2d_spinbox, 1,1,1,5);

  layout->addWidget(new QLabel("ray length"),2,0);
  layout->addWidget(ray_length_2d_spinbox, 2,1,1,5);

  layout->addWidget(new QLabel("window size "),3,0);
  layout->addWidget(window_size_spinbox, 3,1,1,5);


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
      V3DLONG nx=sz[0],ny=sz[1],nz=sz[2];
      V3DLONG size_image=nx*ny;

      unsigned char* datald=0;
      datald = p4DImage->getRawData();


      vector<vector<float> > ray_x(ray_numbers_2d,vector<float>(ray_length_2d)), ray_y(ray_numbers_2d,vector<float>(ray_length_2d));
      vector<vector<float> > ray_x_left(ray_numbers_2d,vector<float>(ray_length_2d)), ray_y_left(ray_numbers_2d,vector<float>(ray_length_2d));
      vector<vector<float> > ray_x_right(ray_numbers_2d,vector<float>(ray_length_2d)), ray_y_right(ray_numbers_2d,vector<float>(ray_length_2d));

      float ang = 2*PI/ray_numbers_2d;
      float x_dis, y_dis, lx_dis, ly_dis, rx_dis, ry_dis;
      for(int i = 0; i < ray_numbers_2d; i++)
      {
          x_dis = cos(ang*(i+1));
          y_dis = sin(ang*(i+1));
          lx_dis = -sin(ang*(i+1));
          ly_dis = cos(ang*(i+1));
          rx_dis = sin(ang*(i+1));
          ry_dis = -cos(ang*(i+1));

          for(int j = 0; j < ray_length_2d; j++)
          {
              ray_x[i][j] = x_dis*(j+1);
              ray_y[i][j] = y_dis*(j+1);

              ray_x_left[i][j] = x_dis*(j+1)+lx_dis;
              ray_y_left[i][j] = y_dis*(j+1)+ly_dis;

              ray_x_right[i][j] = x_dis*(j+1)+rx_dis;
              ray_y_right[i][j] = y_dis*(j+1)+ry_dis;
          }
      }

      cout<<"create 2D_ray success"<<endl;


      unsigned char *image_mip=0;
      try{image_mip=new unsigned char [nx*ny];}
      catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}
      Z_mip(nx,ny,nz,datald,image_mip);

      unsigned char *image_binary=0;
      try{image_binary=new unsigned char [nx*ny];}
      catch(...) {v3d_msg("cannot allocate memory for imag"
                          "e_binary."); return 0;}

      unsigned char *old_image_binary=0;
      try{old_image_binary=new unsigned char [nx*ny];}
      catch(...) {v3d_msg("cannot allocate memory for imag"
                          "e_binary."); return 0;}

      thres_segment(size_image,image_mip,image_binary,thres_2d);
      v3d_msg("segment have complete");

      for(V3DLONG i=0;i<nx*ny;i++)
      {
          old_image_binary[i]=image_binary[i];
      }


      LandmarkList curlist1;
      LandmarkList curlist2;
      LocationSimple s;

      int new_neighbor[10];
      skeletonization(nx,ny,image_binary);

      for(V3DLONG j=1;j<ny-1;j++)
      {
          for(V3DLONG i=1;i<nx-1;i++)
          {
              if(image_binary[j*nx+i]>0)
              {
                  if (image_binary[(j-1)*nx+i-1] == 255) new_neighbor[1] = 1;
                  else  new_neighbor[1] = 0;
                  if (image_binary[(j-1)*nx+i] == 255) new_neighbor[2] = 1;
                  else  new_neighbor[2] = 0;
                  if (image_binary[(j-1)*nx+i+1] == 255) new_neighbor[3] = 1;
                  else  new_neighbor[3] = 0;
                  if (image_binary[(j)*nx+i + 1] == 255) new_neighbor[4] = 1;
                  else  new_neighbor[4] = 0;
                  if (image_binary[(j+1)*nx+i+1] == 255) new_neighbor[5] = 1;
                  else  new_neighbor[5] = 0;
                  if (image_binary[(j+1)*nx+i] == 255) new_neighbor[6] = 1;
                  else  new_neighbor[6] = 0;
                  if (image_binary[(j+1)*nx+i-1] == 255) new_neighbor[7] = 1;
                  else  new_neighbor[7] = 0;
                  if (image_binary[(j)*nx+i-1] == 255) new_neighbor[8] = 1;
                  else  new_neighbor[8] = 0;

                  int sum=0;
                  int cnp;
                  for(int k=1;k<=7;k++)
                  {
                      sum=sum+abs(new_neighbor[k]-new_neighbor[k+1]);
                  }
                  int num=abs(new_neighbor[1]-new_neighbor[8]);
                  cnp=0.5*(sum+num);
                  if(cnp==3)     //  if cnp==3 ,the (i,j) point is the 2D candidate point.
                  {

                      /* move branch point candidate points to center*/
                      if(((i-window_size)<=0)||((j-window_size)<=0)||((j+window_size)>=ny)||((i+window_size)>=nx))
                      {
                          continue;
                      }
                      /* using chaowang algrothm to detect the centre point*/
                      vector<double> Radiu_value;

                      for(int column=-window_size;column<=window_size;column++)
                      {
                          for(int row=-window_size;row<=window_size;row++)
                          {
                              double radius=0;
                              radius=markerRadius_hanchuan_XY(old_image_binary,nx,ny,i+row,j+column,thres_2d);
                              Radiu_value.push_back(radius);
                          }
                      }
                      double max_value=*max_element(Radiu_value.begin(),Radiu_value.end());
                      //Radiu_value.clear();
                      int count=0;
                      int add_x=0;
                      int add_y=0;
                      for(int column=-window_size;column<=window_size;column++)
                      {
                          for(int row=-window_size;row<=window_size;row++)
                          {
                              if(Radiu_value.at(count)==max_value)
                              {
                                  add_x=row;
                                  add_y=column;
                              }
                              count++;

                          }
                      }


                      int new_flag=rayshooting_modle_length(j+add_y,i+add_x,ray_numbers_2d,ray_length_2d,ray_x,ray_y,old_image_binary,nx,ny,thres_2d,max_value);

                      //int flag=rayinten_2D(j+add_y,i+add_x,ray_numbers_2d ,ray_length_2d,ray_x, ray_y, ray_x_left, ray_y_left, ray_x_right, ray_y_right, old_image_binary,nx,ny);
                      cout<<"flag is :"<<new_flag<<endl;
                      if(new_flag==1)
                      {
                          s.x=i+add_x+1;
                          s.y=j+add_y+1;
                          s.z=1;
                          s.radius=1;
                          s.color=random_rgba8(255);
                          curlist2<<s;
                          num++;
                      }


                   }
            }

          }
      }
      for(double i=0;i<curlist1.size();i++)
      {
          for(double j=i+1;j<curlist1.size();j++)
          {
              if(square(curlist1[j].x-curlist1[i].x)+square(curlist1[j].y-curlist1[i].y)+square(curlist1[j].z-curlist1[i].z)<200)
              {
                  curlist1[i].x=(curlist1[j].x+curlist1[i].x)/2;
                  curlist1[i].y=(curlist1[j].y+curlist1[i].y)/2;
                  curlist1[i].z=(curlist1[j].z+curlist1[i].z)/2;
                  curlist1.removeAt(j);
                  j=j-1;
              }
          }
      }

       if(image_binary) {delete []image_binary; image_binary = 0;}
       v3d_msg(QString("single numble of marker is %1").arg(curlist2.size()));
       Image4DSimple * new4DImage = new Image4DSimple();
       new4DImage->setData((unsigned char *)old_image_binary, nx, ny, 1, p4DImage->getCDim(), p4DImage->getDatatype());
       v3dhandle newwin = callback.newImageWindow();
       callback.setImage(newwin, new4DImage);
       callback.updateImageWindow(newwin);
       callback.setLandmark(newwin, curlist2);
      return 1;
}

int thin(V3DPluginCallback2 &callback, QWidget *parent)
{

    Branch_Detection_Model pp;
    pp.Get_Image_Date(callback);
    pp.Gui_input_Rorate_Method();

    // set the parameter
    int thres_2d=pp.thres_2d;
    int window_size=pp.window_size;
    int length_point=pp.length_point;

    V3DLONG nx=pp.sz[0];
    V3DLONG ny=pp.sz[1];
    V3DLONG nz=pp.sz[2];

    Image4DSimple *p4DImage = pp.p4DImage;
    V3DLONG size_image=pp.size_2D_image;

    unsigned char* datald=0;
    datald=pp.datald;

      unsigned char *image_mip=0;
      try{image_mip=new unsigned char [size_image];}
      catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}
      Z_mip(nx,ny,nz,datald,image_mip);
      v3d_msg("mip have complete");

      /* start to distance change based on APP2*/

      /******************************************************************************
       * Fast marching based distance transformation
       *
       * Input : image_mip     original input grayscale image
       *         cnn_type    the connection type
       *         bkg_thresh  the background threshold, less or equal then bkg_thresh will considered as background
       *
       * Output : phi       distance tranformed output image
       *
       * Notice :
       * 1. the input pixel number should not be larger than 2G if sizeof(long) == 4
       * 2. The background point is of intensity 0

       * *****************************************************************************/

      // the distance is not suitable of our algorithm


      unsigned char *image_binary=0;
      try{image_binary=new unsigned char [size_image];}
      catch(...) {v3d_msg("cannot allocate memory for imag"
                          "e_binary."); return 0;}
      thres_segment(size_image,image_mip,image_binary,thres_2d);
      v3d_msg("segment have complete");

      unsigned char *old_image_binary=0;
      try{old_image_binary=new unsigned char [size_image];}
      catch(...) {v3d_msg("cannot allocate memory for imag"
                          "e_binary."); return 0;}

//      float * phi=NULL;
//      int cnn_type=2;
//      fastmarching_dt_XY(image_binary,phi,nx,ny,1,cnn_type,20);
//      cout<<"distance transformation is sucessful ohye!"<<endl;

      for(V3DLONG i=0;i<size_image;i++)
      {
          old_image_binary[i]=image_binary[i];
      }


      LandmarkList curlist;
      LocationSimple s;


      int new_neighbor[10];
      skeletonization(nx,ny,image_binary);
      float x_dis1, y_dis1;
      int size_point=32;   // the length_points multiplied by size_point represent the energy of center points!
      float ang1=(2*PI)/size_point;
      int window_length=2*window_size+1;
      int changed_x_location=0;
      int changed_y_location=0;
      vector<vector<float> > point_x(size_point,vector<float>(length_point)), point_y(size_point,vector<float>(length_point));
//      for(float i = 0; i < size_point; i++)
//      {
//          x_dis1 = cos(ang1*(i+1));
//          y_dis1 = sin(ang1*(i+1));

//          for(float j = 0; j < length_point; j=j+0.1)
//          {
//              point_x[i][j] = x_dis1*(j+1);
//              point_y[i][j] = y_dis1*(j+1);
//          }
//      }


      for(V3DLONG j=1;j<ny-1;j++)
      {
          for(V3DLONG i=1;i<nx-1;i++)
          {
              if(image_binary[j*nx+i]>0)
              {
                  if (image_binary[(j-1)*nx+i-1] == 255) new_neighbor[1] = 1;
                  else  new_neighbor[1] = 0;
                  if (image_binary[(j-1)*nx+i] == 255) new_neighbor[2] = 1;
                  else  new_neighbor[2] = 0;
                  if (image_binary[(j-1)*nx+i+1] == 255) new_neighbor[3] = 1;
                  else  new_neighbor[3] = 0;
                  if (image_binary[(j)*nx+i + 1] == 255) new_neighbor[4] = 1;
                  else  new_neighbor[4] = 0;
                  if (image_binary[(j+1)*nx+i+1] == 255) new_neighbor[5] = 1;
                  else  new_neighbor[5] = 0;
                  if (image_binary[(j+1)*nx+i] == 255) new_neighbor[6] = 1;
                  else  new_neighbor[6] = 0;
                  if (image_binary[(j+1)*nx+i-1] == 255) new_neighbor[7] = 1;
                  else  new_neighbor[7] = 0;
                  if (image_binary[(j)*nx+i-1] == 255) new_neighbor[8] = 1;
                  else  new_neighbor[8] = 0;

                  int sum=0;
                  int cnp;
                  for(int k=1;k<=7;k++)
                  {
                      sum=sum+abs(new_neighbor[k]-new_neighbor[k+1]);
                  }
                  int num=abs(new_neighbor[1]-new_neighbor[8]);
                  cnp=0.5*(sum+num);
                  if(cnp==3)
                  {
                      if(((i-window_size)<=0)||((j-window_size)<=0)||((j+window_size)>=ny)||((i+window_size)>=nx))
                      {
                          continue;
                      }
                      /* using chaowang algrothm to detect the centre point*/
                      vector<double> Radiu_value;
//                      for(int column=-window_size;column<=window_size;column++)
//                      {
//                          for(int row=-window_size;row<=window_size;row++)
//                          {

//                              float sum_value=0;
//                              for(float a=0;a<size_point;a=a+1)
//                              {
//                                  for(float b=0;b<length_point;b=b+0.1)
//                                  {
//                                      float temporary_value=project_interp_2d(j+column+point_y[a][b],i+row+point_x[a][b],old_image_binary,nx,ny,i+row,j+column);
//                                      sum_value=sum_value+temporary_value;
//                                  }
//                              }
//                              Sum_value.push_back(sum_value);
//                              cout<<"the x location is :"<<i+row<<" "<<"the y location is :"<<j+column<<" "<<"the sum of value is :"<<sum_value<<endl;
//                          }
//                      }
//                      int max_value=*max_element(Sum_value.begin(),Sum_value.end());
//                      cout<<"the max_value is :"<<max_value<<endl;

                      /* using Spherical growth method designed by PHC*/

                     // std::vector<float>::iterator biggest=std::max_element(std::begin(Sum_value),std::end(Sum_value));
                      for(int column=-window_size;column<=window_size;column++)
                      {
                          for(int row=-window_size;row<=window_size;row++)
                          {
                              double radius;
                              radius=markerRadius_hanchuan_XY(old_image_binary,nx,ny,i+row,j+column,thres_2d);
                              Radiu_value.push_back(radius);
                              //cout<<"the x location is :"<<i+row<<" "<<"the y location is :"<<j+column<<" "<<"the radiu is :"<<radius<<endl;
                          }
                      }
                      double max_value=*max_element(Radiu_value.begin(),Radiu_value.end());
                      //cout<<"the max radiu is :"<<max_value<<endl;
                      int count=0;
                      int add_x=0;
                      int add_y=0;
                      for(int column=-window_size;column<=window_size;column++)
                      {
                          for(int row=-window_size;row<=window_size;row++)
                          {
                              if(Radiu_value.at(count)==max_value)
                              {
                                  add_x=row;
                                  add_y=column;
                              }
                              count++;
                          }
                      }
                          s.x=i+add_x+based_num;
                          s.y=j+add_y+based_num;
                          s.z=1;
                          s.radius=1;
                          s.color = random_rgba8(255);
                          curlist << s;
                          num++;

                   }
            }

          }
      }
      v3d_msg(QString("do you need show the iamge"));

      if(image_binary) {delete []image_binary; image_binary = 0;}
      if(image_mip) {delete []image_mip; image_mip = 0;}

        Image4DSimple * new4DImage = new Image4DSimple();
        new4DImage->setData((unsigned char *)old_image_binary, nx, ny, 1, p4DImage->getCDim(), p4DImage->getDatatype());
        v3dhandle newwin = callback.newImageWindow();
        callback.setImage(newwin, new4DImage);
        //callback.setImageName(newwin, title);
        callback.updateImageWindow(newwin);
        callback.setLandmark(newwin, curlist);

      return 1;
}

int display_rayshooting_model_in_xyproject(V3DPluginCallback2 &callback, QWidget *parent)
{
    // 1 - Obtain the current 4D image pointer
    v3dhandle curwin = callback.currentImageWindow();
    if(!curwin)
    {
            v3d_msg("No image is open.");
            return -1;
    }
    Image4DSimple *p4DImage = callback.getImage(curwin);

    int x_location=100;
    int y_location=100;
    int ray_numbers_2d = 64;
    int thres_2d =15;
    int ray_length_2d = 64;

    //set update the dialog
    QDialog * dialog = new QDialog();


    if(p4DImage->getZDim() > 1)
            dialog->setWindowTitle("3D neuron image branch point detection Based on Ray-shooting algorithm");
    else
            dialog->setWindowTitle("2D neuron image branch point detection Based on Ray-shooting algorithm");
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

        QSpinBox * x_location_spinbox = new QSpinBox();
        x_location_spinbox->setRange(1,10000);
        x_location_spinbox->setValue(x_location);

        QSpinBox * y_location_spinbox = new QSpinBox();
        y_location_spinbox->setRange(1,10000);
        y_location_spinbox->setValue(y_location);


        layout->addWidget(new QLabel("ray numbers"),0,0);
        layout->addWidget(ray_numbers_2d_spinbox, 0,1,1,5);

        layout->addWidget(new QLabel("intensity threshold"),1,0);
        layout->addWidget(thres_2d_spinbox, 1,1,1,5);

        layout->addWidget(new QLabel("ray length"),2,0);
        layout->addWidget(ray_length_2d_spinbox, 2,1,1,5);

        layout->addWidget(new QLabel("x location"),3,0);
        layout->addWidget(x_location_spinbox, 3,1,1,5);

        layout->addWidget(new QLabel("y location"),4,0);
        layout->addWidget(y_location_spinbox, 4,1,1,5);

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
        y_location=y_location_spinbox->value();
        x_location=x_location_spinbox->value();


        if (dialog)
            {
                    delete dialog;
                    dialog=0;
                    cout<<"delete dialog"<<endl;
            }

        V3DLONG sz[4];
        sz[0] = p4DImage->getXDim();
        sz[1] = p4DImage->getYDim();
        sz[2] = p4DImage->getZDim();
        sz[3]=1;

        unsigned char* datald=0;
        datald = p4DImage->getRawData();


        V3DLONG nx=sz[0];
        V3DLONG ny=sz[1];
        V3DLONG nz=sz[2];

        vector<vector<float> > ray_x(ray_numbers_2d,vector<float>(ray_length_2d)), ray_y(ray_numbers_2d,vector<float>(ray_length_2d));
        vector<vector<float> > ray_x_left(ray_numbers_2d,vector<float>(ray_length_2d)), ray_y_left(ray_numbers_2d,vector<float>(ray_length_2d));
        vector<vector<float> > ray_x_right(ray_numbers_2d,vector<float>(ray_length_2d)), ray_y_right(ray_numbers_2d,vector<float>(ray_length_2d));

        float ang = 2*PI/ray_numbers_2d;
        float x_dis, y_dis, lx_dis, ly_dis, rx_dis, ry_dis;

        for(int i = 0; i < ray_numbers_2d; i++)
        {
            x_dis = cos(ang*(i+1));
            y_dis = sin(ang*(i+1));
            lx_dis = -sin(ang*(i+1));
            ly_dis = cos(ang*(i+1));
            rx_dis = sin(ang*(i+1));
            ry_dis = -cos(ang*(i+1));

            for(int j = 0; j < ray_length_2d; j++)
            {
                ray_x[i][j] = x_dis*(j+1);
                ray_y[i][j] = y_dis*(j+1);

                ray_x_left[i][j] = x_dis*(j+1)+lx_dis;
                ray_y_left[i][j] = y_dis*(j+1)+ly_dis;

                ray_x_right[i][j] = x_dis*(j+1)+rx_dis;
                ray_y_right[i][j] = y_dis*(j+1)+ry_dis;
            }
        }

        cout<<"create 2D_ray success"<<endl;


        unsigned char *datald_mip;
        try{datald_mip=new unsigned char [nx*ny];}
        catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}


        unsigned char *seg_datald_mip;
        try{seg_datald_mip=new unsigned char [nx*ny];}
        catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

        cout<<"allocate memory  success"<<endl;
       // v3d_msg(QString("x is %1, y is %2, z is %3").arg(x_location).arg(y_location).arg(z_location));


        Z_mip(nx,ny,nz,datald,datald_mip);
        thres_segment(nx*ny,datald_mip,seg_datald_mip,thres_2d);

        LandmarkList curlist;
        LocationSimple s;


        vector<float> x_loc;
        vector<float> y_loc;
        v3d_msg(QString("x is %1, y is %2, The pixe of source point is %3").arg(x_location-1).arg(y_location-1).arg(datald_mip[(y_location-1)*nx+x_location-1]));
        float pixel = 0.0;
        float pixel_left = 0.0;
        float pixel_right = 0.0;
        float sum=0;
        float max_indd=0;
        int branch_flag=0;
        int gap_num=0;
        vector<int> ind1;//ind1 is the all piex of each ray

        int raylength_threshold = 10;
        for(int i = 0; i < ray_numbers_2d; i++)   //m is the numble of the ray
        {
            sum=0;
            gap_num=0;
            for(int j = 0; j < ray_length_2d; j++)    // n is the numble of the points of the each ray
            {
                pixel = project_interp_2d((y_location-1)+ray_y[i][j], (x_location-1)+ray_x[i][j], seg_datald_mip, nx,ny , (x_location-1), (y_location-1));
                //v3d_msg(QString("x is %1, y is %2, pixe is %3").arg(point_x+ray_x[i][j]).arg(point_y+ray_y[i][j]).arg(pixel));
                pixel_left = project_interp_2d((y_location-1)+ray_y_left[i][j], (x_location-1)+ray_x_left[i][j],seg_datald_mip, nx,ny , (x_location-1), (y_location-1));
                pixel_right = project_interp_2d((y_location-1)+ray_y_right[i][j], (x_location-1)+ray_x_right[i][j],seg_datald_mip, nx,ny , (x_location-1), (y_location-1));
                sum=sum+pixel+pixel_left+pixel_right;
                if(pixel<60)
                {
                    gap_num=gap_num+1;
                }
                if(gap_num>=3)
                {
                    break;
                }

            }
            ind1.push_back(sum);
        }
        for(int s=0;s<ind1.size();s++)
        {
            if(ind1[s]>max_indd)
            {
                max_indd=ind1[s];
            }
        }
        for(int k=2;k<ind1.size()-2;k++)  //for 3 to 62
        {
            if((ind1.at(k)>0.33*max_indd)&&(ind1.at(k)>raylength_threshold*255))
            {
                if(ind1.at(k)>ind1.at(k-1)&&ind1.at(k)>ind1.at(k+1)&&ind1.at(k)>ind1.at(k+2)&&ind1.at(k)>ind1.at(k-2))
                {
                    cout<<"max ray  is "<<k<<endl;
                    branch_flag=branch_flag+1;
                }
            }
        }
        int size=ind1.size();
        if(ind1.at(size-1)>raylength_threshold*255)  // 64
        {
            if(ind1.at(size-1)>ind1.at(size-2)&&ind1.at(size-1)>ind1.at(0)&&ind1.at(size-1)>ind1.at(1)&&ind1.at(size-1)>ind1.at(size-2))
            {
                cout<<"end ray is the max pxie "<<size-1<<endl;
                branch_flag=branch_flag+1;
            }
        }

       if(ind1.at(0)>raylength_threshold*255)   // 1
       {
           if(ind1.at(0)>ind1.at(size-1)&&ind1.at(0)>ind1.at(1)&&ind1.at(0)>ind1.at(2)&&ind1.at(0)>ind1.at(size-2))
           {
               cout<<"start ray  is the max pixe "<<size-1<<endl;
               branch_flag=branch_flag+1;
           }

       }

       if(ind1.at(size-2)>raylength_threshold*255) //63
       {
           if(ind1.at(size-2)>ind1.at(size-3)&&ind1.at(size-2)>ind1.at(size-3)&&ind1.at(size-2)>ind1.at(size-1)&&ind1.at(size-2)>ind1.at(0))
           {
               cout<<"63 is the max pxie "<<size-2<<endl;
               branch_flag=branch_flag+1;
           }
       }

       if(ind1.at(1)>raylength_threshold*255)    // 2
       {
           if(ind1.at(1)>ind1.at(2)&&ind1.at(1)>ind1.at(3)&&ind1.at(1)>ind1.at(0)&&ind1.at(1)>ind1.at(size-1))
           {
               cout<<"2 is the max pxie "<<1<<endl;
               branch_flag=branch_flag+1;
           }
       }

        for(V3DLONG i=0;i<x_loc.size();i++)
        {
            s.x=  x_loc[i];
            s.y = y_loc[i];
            s.z = 1;
            s.radius = 1;
            s.color=random_rgba8(255);
            curlist<<s;
        }
        v3d_msg(QString("flag is %1").arg(branch_flag));
//        Image4DSimple * new4DImage = new Image4DSimple();
//        new4DImage->setData((unsigned char *)seg_datald_mip, nx, ny, 1, p4DImage->getCDim(), p4DImage->getDatatype());
//        v3dhandle newwin = callback.newImageWindow();
//        callback.setImage(newwin, new4DImage);
//        callback.updateImageWindow(newwin);
//        callback.setLandmark(newwin, curlist);
        return 1;
}

int display_length_model(V3DPluginCallback2 &callback,QWidget *parent)
{
    // 1 - Obtain the current 4D image pointer
     v3dhandle curwin = callback.currentImageWindow();
     if(!curwin)
     {
             v3d_msg("No image is open.");
             return -1;
     }
     Image4DSimple *p4DImage = callback.getImage(curwin);

     int x_location=100;
     int y_location=100;
     int z_location=100;
     int ray_numbers_2d = 64;
     int thres_2d =45;
     int ray_length_2d = 64;

     //set update the dialog
     QDialog * dialog = new QDialog();


     if(p4DImage->getZDim() > 1)
             dialog->setWindowTitle("3D neuron image branch point detection Based on Ray-shooting algorithm");
     else
             dialog->setWindowTitle("2D neuron image branch point detection Based on Ray-shooting algorithm");
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

         QSpinBox * x_location_spinbox = new QSpinBox();
         x_location_spinbox->setRange(1,10000);
         x_location_spinbox->setValue(x_location);

         QSpinBox * y_location_spinbox = new QSpinBox();
         y_location_spinbox->setRange(1,10000);
         y_location_spinbox->setValue(y_location);


         layout->addWidget(new QLabel("ray numbers"),0,0);
         layout->addWidget(ray_numbers_2d_spinbox, 0,1,1,5);

         layout->addWidget(new QLabel("intensity threshold"),1,0);
         layout->addWidget(thres_2d_spinbox, 1,1,1,5);

         layout->addWidget(new QLabel("ray length"),2,0);
         layout->addWidget(ray_length_2d_spinbox, 2,1,1,5);

         layout->addWidget(new QLabel("x location"),3,0);
         layout->addWidget(x_location_spinbox, 3,1,1,5);

         layout->addWidget(new QLabel("y location"),4,0);
         layout->addWidget(y_location_spinbox, 4,1,1,5);

 //        layout->addWidget(new QLabel("z location"),5,0);
 //        layout->addWidget(z_location_spinbox, 5,1,1,5);

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
         y_location=y_location_spinbox->value();
         x_location=x_location_spinbox->value();


         if (dialog)
             {
                     delete dialog;
                     dialog=0;
                     cout<<"delete dialog"<<endl;
             }

         V3DLONG sz[4];
         sz[0] = p4DImage->getXDim();
         sz[1] = p4DImage->getYDim();
         sz[2] = p4DImage->getZDim();
         sz[3]=1;

         unsigned char* datald=0;
         datald = p4DImage->getRawData();


         V3DLONG nx=sz[0];
         V3DLONG ny=sz[1];
         V3DLONG nz=sz[2];

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


         unsigned char *datald_mip;
         try{datald_mip=new unsigned char [nx*ny];}
         catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}


         unsigned char *seg_datald_mip;
         try{seg_datald_mip=new unsigned char [nx*ny];}
         catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

         cout<<"allocate memory  success"<<endl;
        // v3d_msg(QString("x is %1, y is %2, z is %3").arg(x_location).arg(y_location).arg(z_location));


         Z_mip(nx,ny,nz,datald,datald_mip);
         thres_segment(nx*ny,datald_mip,seg_datald_mip,thres_2d);

         LandmarkList curlist;
         LocationSimple s;

         float sum=0.0;
         float pixe=0;
         float source_pixe;
         vector<int> indd,ind1;//ind1 is the all piex of each ray
         float max_indd=0.0;
         int flag=0;
         int count=0;
         vector<float> x_loc,temp_pixel;
         vector<float> y_loc;
         int gap_pixe;
         v3d_msg(QString("x is %1, y is %2, The pixe of source point is %3").arg(x_location-1).arg(y_location-1).arg(datald_mip[(y_location-1)*nx+x_location-1]));
         vector<V3DLONG> adaptive_ray_length;
         for(int i = 0; i <ray_numbers_2d; i++)   //m is the numble of the ray
         {
             sum=0;
             gap_pixe=0;
             for(int j = 0; j < ray_length_2d; j++)    // n is the numble of the points of the each ray
             {
                 //source_pixe=project_interp_2d(y_location-1,x_location-1,datald_mip,nx,ny,x_location,y_location);
                 //v3d_msg(QString("The pixe of source point is %1").arg(source_pixe));
                 pixe = project_interp_2d((y_location-1)+ray_y[i][j], (x_location-1)+ray_x[i][j], seg_datald_mip, nx,ny , (x_location-1), (y_location-1));
                 sum=sum+pixe;
                 temp_pixel.push_back(pixe);
                 x_loc.push_back(x_location-1+ray_x[i][j]);
                 y_loc.push_back(y_location-1+ray_y[i][j]);
                 if(pixe<100)
                 {
                     gap_pixe=gap_pixe+1;
                 }
                 if(gap_pixe>=3)
                 //if(gap_pixe>=10)
                 {
                     adaptive_ray_length.push_back(j+1);//cout<<j+1<<endl;//the length of each ray
                     break;
                 }
             }
             ind1.push_back(sum);
             //v3d_msg(QString("sum of pixe is %1").arg(sum));
         }

         //find the max ray
         for(int s=0;s<ind1.size();s++)
         {
             if (s==1)printf(" ind1[%d] is %d\n ind1[%d] is %d\n ind1[%d] is %d\n ind1[%d] is %d\n ind1[%d] is %d\n ",63,ind1[63],0,ind1[0],1,ind1[1],2,ind1[2],3,ind1[3]);
             if(ind1[s]>max_indd)
             {
                 max_indd=ind1[s];
             }
         }
         v3d_msg(QString("the max ray is %1").arg(max_indd));

         vector<V3DLONG> detected_branch;
         float branch_threshold = 0.1;//the intensity threshold of each ray
         int raylength_threshold =7;//the length threshold of each ray

         for(V3DLONG k=2;k<ind1.size()-2;k++)  //for 2 to 61
         {
             if(adaptive_ray_length[k]>raylength_threshold
                && ind1.at(k)>(branch_threshold*max_indd))
             {
                 if(ind1.at(k)>ind1.at(k-1)&&ind1.at(k)>ind1.at(k+1)&&ind1.at(k)>ind1.at(k+2)&&ind1.at(k)>ind1.at(k-2))
                 {
                     flag=flag+1;detected_branch.push_back(k);
                     cout<<"The branch ind is "<<k<<" "<<"the length is "<<ind1.at(k)<<endl;
                 }

             }
         }
 //        int size=ind1.size();
 //        v3d_msg(QString("the size is %1").arg(size));
         if(adaptive_ray_length[62]>raylength_threshold
            && ind1.at(62)>(branch_threshold*max_indd))
         {
             if(ind1.at(62)>ind1.at(61)&&ind1.at(62)>ind1.at(60)&&ind1.at(62)>ind1.at(63)&&ind1.at(62)>ind1.at(0))
             {
                 flag=flag+1;detected_branch.push_back(62);printf("The branch ind is 62\n");
             }
         }
         if(adaptive_ray_length[63]>raylength_threshold
            && ind1.at(63)>(branch_threshold*max_indd))
         {
             if(ind1.at(63)>ind1.at(62)&&ind1.at(63)>ind1.at(0)&&ind1.at(63)>ind1.at(1)&&ind1.at(63)>ind1.at(61))
             {
                 flag=flag+1;detected_branch.push_back(63);printf("The branch ind is 63\n");
             }
         }
         if(adaptive_ray_length[0]>raylength_threshold
            && ind1.at(0)>(branch_threshold*max_indd))
         {
             if(ind1.at(0)>ind1.at(63)&&ind1.at(0)>ind1.at(1)&&ind1.at(0)>ind1.at(2)&&ind1.at(0)>ind1.at(62))
             {
                 flag=flag+1;detected_branch.push_back(0);printf("The branch ind is 0\n");
             }
         }
         if(adaptive_ray_length[1]>raylength_threshold
            && ind1.at(1)>(branch_threshold*max_indd))
         {
             if(ind1.at(1)>ind1.at(2)&&ind1.at(1)>ind1.at(3)&&ind1.at(1)>ind1.at(0)&&ind1.at(1)>ind1.at(63))
             {
                 flag=flag+1;detected_branch.push_back(1);printf("The branch ind is 1\n");
             }
         }

         RGBA8 red; red.r=255; red.g=0; red.b=0;
         RGBA8 green; green.r=0; green.g=255; green.b=0;
         RGBA8 blue; blue.r=0; blue.g=0; blue.b=255;
         vector<V3DLONG> detected_branch_ind(10);
         //detected_branch = bubbleSort(detected_branch);//order the detected branch ind because it starts from 2 to 61, and then adds the 62, 63, 0, 1
         for(V3DLONG i = 0; i < detected_branch.size(); i++)
         {
             printf("detected_branch after bubbleSort is %d\n",detected_branch[i]);
             for(V3DLONG j = 0; j < detected_branch[i]; j++)
             {
                 detected_branch_ind[i] = detected_branch_ind[i]+adaptive_ray_length[j];//v3d_msg(QString("detected_branch_ind is %1").arg(detected_branch_ind[i]));
             }
         }

         vector<V3DLONG> adaptive_raygap_ind;
         V3DLONG raygap_sum = 0;
         for(V3DLONG i = 0; i < adaptive_ray_length.size(); i++)
         {
            raygap_sum += adaptive_ray_length[i];
            adaptive_raygap_ind.push_back(raygap_sum);
         }

         V3DLONG k=0;
         V3DLONG j_detected_branch=0;
         v3d_msg(QString("the x_loc size is %1").arg(x_loc.size()));
         for(V3DLONG i = 0; i < x_loc.size(); i++)
         {
             s.x=  x_loc[i];
             s.y = y_loc[i];
             s.z = z_location;
             s.radius = 1;
             s.color = blue;//random_rgba8();

             //v3d_msg(QString("the ray ind is %1 and %2").arg(detected_branch_ind[j]).arg(detected_branch_ind[j]+adaptive_ray_length[detected_branch[j]]));
             if(j_detected_branch<detected_branch.size()&&(i >= detected_branch_ind[j_detected_branch] && i <= detected_branch_ind[j_detected_branch]+adaptive_ray_length[detected_branch[j_detected_branch]]))
             {
                 printf("The pixel of branch of ray[%d] is %f\n",i,temp_pixel[i]);
                 s.color = green;
                 if(i == detected_branch_ind[j_detected_branch]+adaptive_ray_length[detected_branch[j_detected_branch]]-1)
                     j_detected_branch++;
             }
             if(k < adaptive_ray_length.size()&&i == adaptive_raygap_ind[k]-1)
             {
                 s.color = red;
                 //cout<<"adaptive_raygap_ind :"<<adaptive_raygap_ind[k]<<endl;
                 k++;
             }
             curlist<<s;
         }
         v3d_msg(QString("curlist is %1").arg(curlist.size()));
         v3d_msg(QString("flag is %1").arg(flag));
         Image4DSimple * new4DImage = new Image4DSimple();
         new4DImage->setData((unsigned char *)seg_datald_mip, nx, ny, 1, p4DImage->getCDim(), p4DImage->getDatatype());
         v3dhandle newwin = callback.newImageWindow();
         callback.setImage(newwin, new4DImage);
         callback.updateImageWindow(newwin);
         callback.setLandmark(newwin, curlist);

         return 1;
}

int create_groundtruth(V3DPluginCallback2 &callback,QWidget *parent)
{
    v3dhandle curwin = callback.currentImageWindow();
    if(!curwin)
    {
            v3d_msg("No image is open.");
            return -1;
    }
    Image4DSimple *p4DImage = callback.getImage(curwin);

    int ray_numbers_2d = 64;
    int thres_2d =45;
    int ray_length_2d = 64;

    //set update the dialog
    QDialog * dialog = new QDialog();

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

        V3DLONG sz[4];
        sz[0] = p4DImage->getXDim();
        sz[1] = p4DImage->getYDim();
        sz[2] = p4DImage->getZDim();
        sz[3]=1;

        unsigned char* datald=0;
        datald = p4DImage->getRawData();
        V3DLONG nx=sz[0];
        V3DLONG ny=sz[1];
        V3DLONG nz=sz[2];

        vector<vector<float> > ray_x(ray_numbers_2d,vector<float>(ray_length_2d)), ray_y(ray_numbers_2d,vector<float>(ray_length_2d));

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

        unsigned char *datald_mip;
        try{datald_mip=new unsigned char [nx*ny];}
        catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

        unsigned char *seg_datald_mip;
        try{seg_datald_mip=new unsigned char [nx*ny];}
        catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}


        Z_mip(nx,ny,nz,datald,datald_mip);
        thres_segment(nx*ny,datald_mip,seg_datald_mip,thres_2d);

        vector<MyMarker> file_inmarkers;
        list<string> infostring;
        string file="C:/Users/Administrator/Desktop/point/in_point.txt";
        string out_file="C:/Users/Administrator/Desktop/point_feature/groundtruth_marker.txt";
        file_inmarkers=readMarker_file(file);
        vector<vector<int>> out_matrix(file_inmarkers.size(),vector<int> (64));
        for(int num=0;num<file_inmarkers.size();num++)
        {
            float sum=0.0;
            float pixe=0;
            vector<float> temp_pixel;
            int gap_pixe;
            for(int i = 0; i <ray_numbers_2d; i++)   //m is the numble of the ray
            {
                sum=0;
                gap_pixe=0;
                for(int j = 0; j < ray_length_2d; j++)    // n is the numble of the points of the each ray
                {
                    pixe = project_interp_2d((file_inmarkers[num].y-1)+ray_y[i][j], (file_inmarkers[num].x-1)+ray_x[i][j], seg_datald_mip, nx,ny , (file_inmarkers[num].x-1), (file_inmarkers[num].y-1));
                    sum=sum+pixe;
                    temp_pixel.push_back(pixe);
                    if(pixe<100)
                    {
                        gap_pixe=gap_pixe+1;
                    }
                    if(gap_pixe>=3)
                    {
                        out_matrix[num][i]=j+1;
                        break;
                    }
                }
            }
        }
        save_groundtruth_marker(out_file,out_matrix,infostring);
        return 1;
}

int rotate_2D_display_point(V3DPluginCallback2 &callback,QWidget *parent)
{
    ClusterAnalysis mycluster;
    Branch_Detection_Model pp;
    pp.Get_Image_Date(callback);
    pp.Gui_input_Rorate_display_Method();

    int x_location=pp.x_coordinate, y_location=pp.y_coordinate,thres_2d=pp.thres_2d,ray_numbers_2d=pp.ray_numbers_2d;
    int based_distance=pp.based_distance,window_size=pp.window_size, block_size=pp.block_size;

    V3DLONG nx=pp.sz[0],ny=pp.sz[1],nz=pp.sz[2];

    Image4DSimple *p4DImage = pp.p4DImage;
    V3DLONG size_image=pp.size_2D_image;

    unsigned char* datald=0;
    datald=pp.datald;

         unsigned char *datald_mip;
         try{datald_mip=new unsigned char [size_image];}
         catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

         unsigned char *seg_datald_mip;
         try{seg_datald_mip=new unsigned char [size_image];}
         catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

         Z_mip(nx,ny,nz,datald,datald_mip);
         thres_segment(nx*ny,datald_mip,seg_datald_mip,thres_2d);

         /* using Spherical growth method designed by PHC*/
         double radiu;
         V3DLONG adjusted_x, adjusted_y;
         find_neighborhood_maximum_radius(x_location,y_location,seg_datald_mip,window_size,adjusted_x,adjusted_y,radiu,nx,ny,thres_2d);
         //cout<<"x is :"<<adjusted_x<<" "<<"y is :"<<adjusted_y<<" "<<"the radiu  is :"<<radiu<<endl;

         vector<vector<float> > ray_x(ray_numbers_2d,vector<float>(100)), ray_y(ray_numbers_2d,vector<float>(100));
         float ang = 2*PI/ray_numbers_2d;
         float x_dis, y_dis;

         for(int i = 0; i < ray_numbers_2d; i++)
         {
            x_dis = cos(ang*(i+1));
            y_dis = sin(ang*(i+1));
            for(int j = 0; j<30 ; j++)
                {
                    ray_x[i][j] = x_dis*(j+1);
                    ray_y[i][j] = y_dis*(j+1);
                }
         }
         vector<float> x_loc;  //foreground point
         vector<float> y_loc;
         vector<float> dis_x_loc; // background point
         vector<float> dis_y_loc;
         vector<float> Pixe;

         //create a new mip
         //int block_radiu=(radiu+based_distance+10);
         int block_radiu=radiu+based_distance+block_size;
         int block_length=block_radiu*2+1;
         int num_block=0;
          unsigned char *block=new unsigned char[block_length*block_length];
          for(V3DLONG b=adjusted_y-block_radiu;b<=adjusted_y+block_radiu;b++)
          {
              for(V3DLONG c=adjusted_x-block_radiu;c<=adjusted_x+block_radiu;c++)
              {
                  unsigned char block_pixe=seg_datald_mip[b*nx+c];
                    block[num_block]=block_pixe;
                    num_block++;
              }
          }
          delete_small_area(block_length,block_length,block);// connected domain denoising

          for(int i = 0; i <ray_numbers_2d; i++)   //i is the numble of the ray
          {
              for(int j = radiu+based_distance; j < radiu+based_distance+2; j++)    // j is the numble of circle
              {
                  double pixe = project_interp_2d(block_radiu+ray_y[i][j], block_radiu+ray_x[i][j], block, block_length,block_length , (block_radiu), (block_radiu));
                  if(pixe>=100)
                  {
                      x_loc.push_back(block_radiu+ray_x[i][j]);
                      y_loc.push_back(block_radiu+ray_y[i][j]);
                  }
                  dis_x_loc.push_back(block_radiu+ray_x[i][j]);
                  dis_y_loc.push_back(block_radiu+ray_y[i][j]);
                  Pixe.push_back(pixe);
              }

          }

          float change_x1=cos(ang*3)*(radiu+based_distance+2)-cos(ang*2)*(radiu+based_distance+2);
          float change_x=pow(change_x1,2);
          float change_y1=sin(ang*3)*(radiu+based_distance+2)-sin(ang*2)*(radiu+based_distance+2);
          float change_y=pow(change_y1,2);
          float DB_radius=sqrt(change_x+change_y)+1;
          cout<<"the DB radius is:"<<DB_radius<<endl;
          mycluster.Read_from_coordiante(x_loc,y_loc,DB_radius,3);
          int flag=mycluster.DoDBSCANRecursive();
          v3d_msg(QString("the cluster is %1").arg(flag));

          RGBA8 red; red.r=255; red.g=0; red.b=0;
          RGBA8 green; green.r=0; green.g=255; green.b=0;
          RGBA8 blue; blue.r=0; blue.g=0; blue.b=255;

          LandmarkList curlist;
          LocationSimple s;

          for(V3DLONG i = 0; i < dis_x_loc.size(); i++)
          {
              s.x=  dis_x_loc[i]+1;
              s.y = dis_y_loc[i]+1;
              s.z = 1;
              s.radius = 1;
              if(Pixe.at(i)<100)
              {
                  s.color=blue;
              }
              else {
                  s.color = red;
              }
              curlist<<s;
          }

          v3d_msg(QString("the points is %1").arg(curlist.size()));
          Image4DSimple * new4DImage = new Image4DSimple();
          new4DImage->setData((unsigned char *)block, block_length, block_length, 1, p4DImage->getCDim(), p4DImage->getDatatype());
          v3dhandle newwin = callback.newImageWindow();
          callback.setImage(newwin, new4DImage);
          callback.updateImageWindow(newwin);
          callback.setLandmark(newwin, curlist);
          return 1;

}

int rotate_2D_image(V3DPluginCallback2 &callback,QWidget *parent)
{

     ClusterAnalysis mycluster;
     Branch_Detection_Model pp;
     pp.Get_Image_Date(callback);
     pp.Gui_input_Rorate_Method();

     // set the parameter
     int thres_2d=pp.thres_2d;
     int window_size=pp.window_size;
     int ray_numbers_2d=pp.ray_numbers_2d;
     int based_distance=pp.based_distance;
     int block_size=pp.block_size;
     V3DLONG nx=pp.sz[0];
     V3DLONG ny=pp.sz[1];
     V3DLONG nz=pp.sz[2];

     Image4DSimple *p4DImage = pp.p4DImage;
     V3DLONG size_image=pp.size_2D_image;

     unsigned char* datald=0;
     datald=pp.datald;

         unsigned char *datald_mip;
         try{datald_mip=new unsigned char [size_image];}
         catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}


         unsigned char *image_binary;
         try{image_binary=new unsigned char [size_image];}
         catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

         unsigned char *old_image_binary=0;
         try{old_image_binary=new unsigned char [size_image];}
         catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

         Z_mip(nx,ny,nz,datald,datald_mip);
         thres_segment(nx*ny,datald_mip,image_binary,thres_2d);

         for(V3DLONG i=0;i<nx*ny;i++)
         {
             old_image_binary[i]=image_binary[i];
         }

         LandmarkList curlist;
         LocationSimple s;
         vector<V3DLONG> X_candidate;
         vector<V3DLONG> Y_candidate;
         vector<double> Max_value;

         int count=0; // the number of candidate
         vector<V3DLONG> X_loc; //save the non-adjusted x coordinate;
         vector<V3DLONG> Y_loc; //save the non-adjusted y coordinate;
         skeletonization(nx,ny,image_binary);
         seek_2D_candidate_points(nx,ny,image_binary,X_loc,Y_loc,count);
         if(count>0)
         {
             for(int i=0;i<count;i++)
             {
                 if(((X_loc.at(i)-window_size)<=0)||((Y_loc.at(i)-window_size)<=0)||((Y_loc.at(i)+window_size)>=ny)||((X_loc.at(i)+window_size)>=nx))
                 {
                     continue;
                 }
                 /* using Spherical growth method designed by Chaowang*/
                 V3DLONG adjusted_x, adjusted_y;
                 double radiu;
                 find_neighborhood_maximum_radius(X_loc.at(i),Y_loc.at(i),old_image_binary,window_size,adjusted_x,adjusted_y,radiu,nx,ny,thres_2d);
                 X_candidate.push_back(adjusted_x);  // all adjusted x coordinate of candidate points
                 Y_candidate.push_back(adjusted_y);  // all adjusted y coordinate of candidate points
                 Max_value.push_back(radiu);    // all radiu coordinate of candidate points
                 //cout<<"x is :"<<adjusted_x<<" "<<"y is :"<<adjusted_y<<" "<<"the radiu  is :"<<radiu<<endl;

             }
         }
         else {
             cout<<"no candidate points in this MIP"<<endl;
         }

         cout<<"the all candidate points have detected"<<endl;

         /* create the ratate matrix */
         vector<vector<float> > ray_x(ray_numbers_2d,vector<float>(100)), ray_y(ray_numbers_2d,vector<float>(100));
         float ang = 2*PI/ray_numbers_2d;
         float x_dis, y_dis;
         int max_length=90;
         for(int i = 0; i < ray_numbers_2d; i++)
         {
            x_dis = cos(ang*(i+1));
            y_dis = sin(ang*(i+1));
            for(int j = 0; j<max_length ; j++)
                {
                    ray_x[i][j] = x_dis*(j+1);
                    ray_y[i][j] = y_dis*(j+1);
                }
         }

         /* difine the color */
         RGBA8 red; red.r=255; red.g=0; red.b=0;
         RGBA8 green; green.r=0; green.g=255; green.b=0;
         RGBA8 blue; blue.r=0; blue.g=0; blue.b=255;

         for(V3DLONG k = 0; k < X_candidate.size(); k++)
         {
             vector<float> x_loc;
             vector<float> y_loc;

             //create a new mip
             //int block_radiu=(Max_value.at(k)+based_distance+10);
             int block_radiu=Max_value.at(k)+based_distance+block_size;
             int block_length=block_radiu*2+1;
             unsigned char *block=0;
             try{block=new unsigned char [block_length*block_length*block_radiu];}

             catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}
//             //cout<<"x is :"<<X_candidate.at(k)<<" "<<" y is :"<<Y_candidate.at(k)<<" "<<"the length is "<<block_length<<endl;
             if(((X_candidate.at(k)+block_radiu)>nx)||((X_candidate.at(k)-block_radiu)<0)||((Y_candidate.at(k)-block_radiu)<0)||((Y_candidate.at(k)+block_radiu)>ny))
             {
                 //cout<<"this points have beyong the image"<<endl;
                 continue;
             }
             int num_block=0;
             for(V3DLONG b=Y_candidate.at(k)-block_radiu;b<=Y_candidate.at(k)+block_radiu;b++)
             {
                  for(V3DLONG c=X_candidate.at(k)-block_radiu;c<=X_candidate.at(k)+block_radiu;c++)
                  {
                      unsigned char block_pixe=old_image_binary[b*nx+c];
                      block[num_block]=block_pixe;
                      num_block++;
                  }
              }
              delete_small_area(block_length,block_length,block);// connected domain denoising

             /* using the ratate algorithm designed by chaowang 2019/1/15 */
              //v3d_msg(QString("the block size is %1").arg(Max_value.at(k)));
              //v3d_msg(QString("the block size is %1,the ray length is %2").arg(block_radiu).arg(Max_value.at(k)+new_based_distance+3));
             for(int i = 0; i <ray_numbers_2d; i++)   //m is the numble of the ray
             {
                 for(int j = Max_value.at(k)+based_distance; j < Max_value.at(k)+based_distance+2; j++)    // n is the numble of the points of the each ray
                 {
                     double pixe = project_interp_2d(block_radiu+ray_y[i][j], block_radiu+ray_x[i][j], block, block_length, block_length , block_radiu,  block_radiu);
                     if(pixe>=100)
                     {
                         x_loc.push_back(block_radiu+ray_x[i][j]);
                         y_loc.push_back(block_radiu+ray_y[i][j]);

                     }
                 }
             }

//           /* calculate the DBscan radiu */
             float change_x1=cos(ang*3)*(Max_value.at(k)+based_distance+3)-cos(ang*2)*(Max_value.at(k)+based_distance+3);
             float change_x=pow(change_x1,2);
             float change_y1=sin(ang*3)*(Max_value.at(k)+based_distance+3)-sin(ang*2)*(Max_value.at(k)+based_distance+3);
             float change_y=pow(change_y1,2);
             float DB_radius=sqrt(change_x+change_y)+1;
             mycluster.Read_from_coordiante(x_loc,y_loc,DB_radius,3);
             int flag=mycluster.DoDBSCANRecursive();
             if(flag==3)
             {
                              s.x=  X_candidate[k]+1;
                              s.y = Y_candidate[k]+1;
                              s.z = 1;
                              s.radius = 1;
                              s.color = red;
                              curlist<<s;
             }
//             delete[]block;

         }
         Image4DSimple * new4DImage = new Image4DSimple();
         new4DImage->setData((unsigned char *)datald_mip, nx, ny, 1, p4DImage->getCDim(), p4DImage->getDatatype());
         v3dhandle newwin = callback.newImageWindow();
         callback.setImage(newwin, new4DImage);
         callback.updateImageWindow(newwin);
         callback.setLandmark(newwin, curlist);
         return 1;

}

int rotate_3D_image(V3DPluginCallback2 &callback, QWidget *parent)
{

    ClusterAnalysis mycluster;
    Branch_Detection_Model pp;
    pp.Get_Image_Date(callback);
    pp.Gui_input_Rorate_3D_Method();

    // set the parameter
    int thres_2d=pp.thres_2d;
    int window_size=pp.window_size;
    int ray_numbers_2d=pp.ray_numbers_2d;
    int based_distance=pp.based_distance;
    int num_layer=pp.num_layer;

    V3DLONG nx=pp.sz[0];
    V3DLONG ny=pp.sz[1];
    V3DLONG nz=pp.sz[2];

    Image4DSimple *p4DImage = pp.p4DImage;
    V3DLONG size_image=pp.size_2D_image;

    unsigned char* datald=0;
    datald=pp.datald;

            unsigned char *image_mip;
            unsigned char *image_binary;
            unsigned char *old_image_binary;

            vector<V3DLONG> X_coor;
            vector<V3DLONG> Y_coor;
            vector<V3DLONG> Z_coor;
            vector<float> soma_radiu;


            /* create the ratate matrix */
            vector<vector<float> > ray_x(ray_numbers_2d,vector<float>(100)), ray_y(ray_numbers_2d,vector<float>(100));
            float ang = 2*PI/ray_numbers_2d;
            float x_dis, y_dis;
            int max_length=90;
            for(int i = 0; i < ray_numbers_2d; i++)
            {
               x_dis = cos(ang*(i+1));
               y_dis = sin(ang*(i+1));
               for(int j = 0; j<max_length ; j++)
                   {
                       ray_x[i][j] = x_dis*(j+1);
                       ray_y[i][j] = y_dis*(j+1);
                   }
            }
            /* difine the color */
            try{image_mip=new unsigned char [size_image];}
            catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

            try{image_binary=new unsigned char [size_image];}
            catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

            try{old_image_binary=new unsigned char [size_image];}
            catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

            cout<<"allocate the memory is successful"<<endl;

            LandmarkList curlist;
            LocationSimple s;
            for(int size_z=num_layer;size_z<nz-num_layer;size_z++)
            {
                /* detect the 2D branch points in each MIP */
                if((size_z+num_layer)<nz)
                {
                   mip(nx,ny,size_z,datald,image_mip,num_layer);
                   thres_segment(size_image,image_mip,image_binary,thres_2d);
                   for(V3DLONG a=0;a<nx*ny;a++)
                   {
                       old_image_binary[a]=image_binary[a];
                   }
                   skeletonization(nx,ny,image_binary);
                   vector<V3DLONG> X_candidate;  // save the adjusted x coordinate;
                   vector<V3DLONG> Y_candidate;  // save the adjusted y coordinate;
                   vector<float> Max_value;      // save the radiu;
                   int count=0; // the number of candidate
                   vector<V3DLONG> X_loc; //save the non-adjusted x coordinate;
                   vector<V3DLONG> Y_loc; //save the non-adjusted y coordinate;
                  // skeletonization(nx,ny,image_binary);
                   seek_2D_candidate_points(nx,ny,image_binary,X_loc,Y_loc,count);
                   if(count>0)
                   {
                       for(int i=0;i<count;i++)
                       {
                           if(((X_loc.at(i)-window_size)<=0)||((Y_loc.at(i)-window_size)<=0)||((Y_loc.at(i)+window_size)>=ny)||((X_loc.at(i)+window_size)>=nx))
                           {
                               continue;
                           }
                           /* using Spherical growth method designed by Chaowang*/
                           V3DLONG adjusted_x, adjusted_y;
                           double radiu;
                           find_neighborhood_maximum_radius(X_loc.at(i),Y_loc.at(i),old_image_binary,window_size,adjusted_x,adjusted_y,radiu,nx,ny,thres_2d);
                           X_candidate.push_back(adjusted_x);  // all adjusted x coordinate of candidate points
                           Y_candidate.push_back(adjusted_y);  // all adjusted y coordinate of candidate points
                           Max_value.push_back(radiu);    // all radiu coordinate of candidate points
                           //cout<<"x is :"<<adjusted_x<<" "<<"y is :"<<adjusted_y<<" "<<"the radiu  is :"<<radiu<<endl;

                       }
                   }
                   else {

                       cout<<"no candidate points in this MIP"<<endl;
                   }
                   std::vector<float>::iterator biggest=std::max_element(std::begin(Max_value),std::end(Max_value));
                   int location=std::distance(std::begin(Max_value),biggest);
                   X_coor.push_back(X_candidate.at(location));
                   Y_coor.push_back(Y_candidate.at(location));
                   soma_radiu.push_back(*biggest);
                   cout<<"the all candidate points in each MIP have detected"<<endl;

                   for(V3DLONG k = 0; k < X_candidate.size(); k++)
                   {
                       vector<float> x_loc;
                       vector<float> y_loc;
                       //create a new mip
                       int block_radiu=3*Max_value.at(k);
                       int block_length=block_radiu*2+1;
                       unsigned char *block=0;
                       try{block=new unsigned char [block_length*block_length*block_radiu];}

                       catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}
                       //cout<<"x is :"<<X_candidate.at(k)<<" "<<" y is :"<<Y_candidate.at(k)<<" "<<"the length is "<<block_length<<endl;
                       if(((X_candidate.at(k)+block_radiu)>nx)||((X_candidate.at(k)-block_radiu)<0)||((Y_candidate.at(k)-block_radiu)<0)||((Y_candidate.at(k)+block_radiu)>ny))
                       {
                           cout<<"this points have beyong the image"<<endl;
                           continue;
                       }
                       int num_block=0;
                       for(V3DLONG b=Y_candidate.at(k)-block_radiu;b<=Y_candidate.at(k)+block_radiu;b++)
                       {
                            for(V3DLONG c=X_candidate.at(k)-block_radiu;c<=X_candidate.at(k)+block_radiu;c++)
                            {
                                unsigned char block_pixe=old_image_binary[b*nx+c];
                                block[num_block]=block_pixe;
                                num_block++;
                            }
                        }
                        delete_small_area(block_length,block_length,block);// connected domain denoising

                        for(int i = 0; i <ray_numbers_2d; i++)   //m is the numble of the ray
                        {
                            for(int j = Max_value.at(k)+based_distance; j < Max_value.at(k)+based_distance+3; j++)    // n is the numble of the points of the each ray
                            {
                                double pixe = project_interp_2d(block_radiu+ray_y[i][j], block_radiu+ray_x[i][j], block, block_length, block_length , block_radiu,  block_radiu);
                                if(pixe>=100)
                                {
                                    x_loc.push_back(block_radiu+ray_x[i][j]);
                                    y_loc.push_back(block_radiu+ray_y[i][j]);

                                }
                            }
                        }

                       /* calculate the DBscan radiu */
                       float change_x1=cos(ang*3)*(Max_value.at(k)+based_distance+3)-cos(ang*2)*(Max_value.at(k)+based_distance+3);
                       float change_x=pow(change_x1,2);
                       float change_y1=sin(ang*3)*(Max_value.at(k)+based_distance+3)-sin(ang*2)*(Max_value.at(k)+based_distance+3);
                       float change_y=pow(change_y1,2);
                       float DB_radius=sqrt(change_x+change_y)+1;
                       mycluster.Read_from_coordiante(x_loc,y_loc,DB_radius,3);
                       int flag=mycluster.DoDBSCANRecursive();

                       /* difine the color */
                       RGBA8 red; red.r=255; red.g=0; red.b=0;
                       RGBA8 green; green.r=0; green.g=255; green.b=0;
                       RGBA8 blue; blue.r=0; blue.g=0; blue.b=255;

                       if(flag==3)
                       {
                                        s.x=  X_candidate[k]+1;
                                        s.y = Y_candidate[k]+1;
                                        unsigned char max_z=0;
                                        int loc_z;
                                           for(int kk=size_z;kk<size_z+num_layer;kk++)
                                           {
                                               unsigned char sum_pixe;
                                               sum_pixe=datald[kk*nx*ny+Y_candidate[k]*nx+X_candidate[k]]+datald[kk*nx*ny+Y_candidate[k]*nx+X_candidate[k]+1]+datald[kk*nx*ny+Y_candidate[k]*nx+X_candidate[k]-1]+
                                                       datald[kk*nx*ny+(Y_candidate[k]+1)*nx+X_candidate[k]]+datald[kk*nx*ny+(Y_candidate[k]-1)*nx+X_candidate[k]]+
                                                       datald[kk*nx*ny+(Y_candidate[k]+1)*nx+X_candidate[k]-1]+datald[kk*nx*ny+(Y_candidate[k]+1)*nx+X_candidate[k]+1]+
                                                       datald[kk*nx*ny+(Y_candidate[k]-1)*nx+X_candidate[k]-1]+datald[kk*nx*ny+(Y_candidate[k]-1)*nx+X_candidate[k]+1];
                                               if(sum_pixe>max_z)
                                               {
                                                   max_z=sum_pixe;
                                                   loc_z=kk;
                                               }
                                           }
                                        s.z = size_z;
                                        s.radius = 1;
                                        s.color = blue;
                                        curlist<<s;
                       }

                   }
                }
            }

            std::vector<float>::iterator biggest=std::max_element(std::begin(soma_radiu),std::end(soma_radiu));
            int new_location=std::distance(std::begin(soma_radiu),biggest);
            int max_radiu=*biggest;
            V3DLONG soma_x=X_coor.at(new_location);
            V3DLONG soma_y=Y_coor.at(new_location);
            cout<<"the soma x is :"<<soma_x<<" "<<"the soma y is :"<<soma_y<<" "<<"the radiu is :"<<max_radiu<<endl;

            for(V3DLONG i=0;i<curlist.size();i++)
            {
                if(((curlist.at(i).x-soma_x)<max_radiu)&&((curlist.at(i).y-soma_y)<max_radiu))
                {
                    curlist.removeAt(i);
                }
            }
            for(V3DLONG i=0;i<curlist.size();i++)
            {
                unsigned char max_pixe=0;
                for(V3DLONG j=1;j<nz;j++)
                {
                    unsigned char z_pixe=p4DImage->getValueUINT8(curlist[i].x,curlist[i].y,j,0);
                    if(z_pixe>max_pixe)
                    {
                        max_pixe=z_pixe;
                    }

                }
                unsigned char new_pixe=p4DImage->getValueUINT8(curlist[i].x,curlist[i].y,curlist[i].z,0);
                if(new_pixe<max_pixe)
                {
                   curlist.removeAt(i);
                }

            }


            for(double i=0;i<curlist.size();i++)
            {
                for(double j=i+1;j<curlist.size();j++)
                {
                    if(((square(curlist[j].x-curlist[i].x)+square(curlist[j].y-curlist[i].y))<50)&&(square(curlist[j].z-curlist[i].z)<500))
                    {
                        curlist[i].x=(curlist[j].x+curlist[i].x)/2;
                        curlist[i].y=(curlist[j].y+curlist[i].y)/2;
                        curlist[i].z=(curlist[j].z+curlist[i].z)/2;
                        curlist.removeAt(j);
                        j=j-1;
                    }
                }
            }


            Image4DSimple * new4DImage = new Image4DSimple();
            new4DImage->setData((unsigned char *)datald, nx, ny, nz, p4DImage->getCDim(), p4DImage->getDatatype());
            v3dhandle newwin = callback.newImageWindow();
            callback.setImage(newwin, new4DImage);
            callback.updateImageWindow(newwin);
            callback.setLandmark(newwin, curlist);
            delete [] image_mip;
            delete []image_binary;
            delete []old_image_binary;
            return 1;



}


int rotate_3D_display_point(V3DPluginCallback2 &callback,QWidget *parent)
{
    ClusterAnalysis mycluster;
//    Branch_Detection_Model pp;
//    pp.Get_Image_Date(callback);
//    pp.Gui_input_Rorate_display_Method();

//    int x_location=pp.x_coordinate;
//    int y_location=pp.y_coordinate;
//    int z_location=pp.z_coordinate;
//    int thres_2d=pp.thres_2d;
//    int ray_numbers_2d=pp.ray_numbers_2d;
//    int based_distance=pp.based_distance;

//    V3DLONG nx=pp.sz[0];
//    V3DLONG ny=pp.sz[1];
//    V3DLONG nz=pp.sz[2];
//    Image4DSimple *p4DImage = pp.p4DImage;
//    V3DLONG size_image=pp.size_2D_image;
//    unsigned char* datald=0;
//    datald=pp.datald;

    v3dhandle curwin = callback.currentImageWindow();
    if(!curwin)
    {
            v3d_msg("No image is open.");
            return -1;
    }
    Image4DSimple *p4DImage = callback.getImage(curwin);
    cout<<"set update the dialog"<<endl;


    //set update the dialog
    int ray_numbers_2d=128;
    int thres_2d=50;
    int x_coordinate=100;
    int y_coordinate=100;
    int z_coordinate=10;
    int based_distance=5;

    QDialog * dialog = new QDialog();


    if(p4DImage->getZDim() > 1)
            dialog->setWindowTitle("3D neuron image branch point detection Based on Ray-shooting algorithm");
    else
            dialog->setWindowTitle("2D neuron image branch point detection Based on Ray-shooting algorithm");
        QGridLayout * layout = new QGridLayout();

        QSpinBox * ray_numbers_2d_spinbox = new QSpinBox();
        ray_numbers_2d_spinbox->setRange(1,1000);
        ray_numbers_2d_spinbox->setValue(ray_numbers_2d);

        QSpinBox * thres_2d_spinbox = new QSpinBox();
        thres_2d_spinbox->setRange(-1, 255);
        thres_2d_spinbox->setValue(thres_2d);

        QSpinBox * x_location_spinbox = new QSpinBox();
        x_location_spinbox->setRange(1,10000);
        x_location_spinbox->setValue(x_coordinate);

        QSpinBox * y_location_spinbox = new QSpinBox();
        y_location_spinbox->setRange(1,10000);
        y_location_spinbox->setValue(y_coordinate);

        QSpinBox * z_location_spinbox = new QSpinBox();
        z_location_spinbox->setRange(1,10000);
        z_location_spinbox->setValue(z_coordinate);

        QSpinBox * based_distance_spinbox = new QSpinBox();
        based_distance_spinbox->setRange(1,100);
        based_distance_spinbox->setValue(based_distance);


        layout->addWidget(new QLabel("ray numbers"),0,0);
        layout->addWidget(ray_numbers_2d_spinbox, 0,1,1,5);

        layout->addWidget(new QLabel("intensity threshold"),1,0);
        layout->addWidget(thres_2d_spinbox, 1,1,1,5);

        layout->addWidget(new QLabel("x location"),2,0);
        layout->addWidget(x_location_spinbox, 2,1,1,5);

        layout->addWidget(new QLabel("y location"),3,0);
        layout->addWidget(y_location_spinbox, 3,1,1,5);

        layout->addWidget(new QLabel("z location"),4,0);
        layout->addWidget(z_location_spinbox, 4,1,1,5);


        layout->addWidget(new QLabel("based distance "),5,0);
        layout->addWidget(based_distance_spinbox, 5,1,1,5);


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

            }

        //get the dialog return values
        ray_numbers_2d = ray_numbers_2d_spinbox->value();
        thres_2d = thres_2d_spinbox->value();
        y_coordinate=y_location_spinbox->value();
        x_coordinate=x_location_spinbox->value();
        z_coordinate=z_location_spinbox->value();
        based_distance=based_distance_spinbox->value();


        int x_location=x_coordinate;
        int y_location =y_coordinate;
        int z_location =z_coordinate;

        V3DLONG sz[3];
        sz[0] = p4DImage->getXDim();
        sz[1] = p4DImage->getYDim();
        sz[2] = p4DImage->getZDim();
        V3DLONG nx=sz[0];
        V3DLONG ny=sz[1];
        V3DLONG nz=sz[2];
        unsigned char* datald=0;
        datald = p4DImage->getRawData();
        V3DLONG size_image=sz[0]*sz[1];

         unsigned char *datald_mip;
         try{datald_mip=new unsigned char [size_image];}
         catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}


         unsigned char *seg_datald_mip;
         try{seg_datald_mip=new unsigned char [size_image];}
         catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

         cout<<"allocate memory  success"<<endl;

         Z_mip(nx,ny,nz,datald,datald_mip);
         thres_segment(nx*ny,datald_mip,seg_datald_mip,thres_2d);

         LandmarkList curlist;
         LocationSimple s;
         int window_size=3;
         vector<float> Radiu_value;

         /* using Spherical growth method designed by PHC*/
         for(int column=-window_size;column<=window_size;column++)
         {
             for(int row=-window_size;row<=window_size;row++)
             {
                 double radius;
                 radius=markerRadius_hanchuan_XY(seg_datald_mip,nx,ny,x_location+row,y_location+column,thres_2d);
                 Radiu_value.push_back(radius);
                 //cout<<"the x location is :"<<i+row<<" "<<"the y location is :"<<j+column<<" "<<"the radiu is :"<<radius<<endl;
             }
         }
         double max_value=*max_element(Radiu_value.begin(),Radiu_value.end());
         cout<<"the max radiu is :"<<max_value<<endl;
         int count=0;
         int add_x=0;
         int add_y=0;
         for(int column=-window_size;column<=window_size;column++)
         {
             for(int row=-window_size;row<=window_size;row++)
             {
                 if(Radiu_value.at(count)==max_value)
                 {
                     add_x=row;
                     add_y=column;
                 }
                 count++;
             }
         }



         float ang = 2*PI/ray_numbers_2d;

         vector<vector<float> > x_dis(32*64,vector<float>(100)), y_dis(32*64,vector<float>(100)),z_dis(32*64,vector<float>(100));
         //float x_dis[1024][128], y_dis[1024][128] , z_dis[1024][128];

         for(int len = 0; len < 64; len++)
         {
             int mm = 0;
             for(int n = 1; n <= 32; n++ )
                 for(int k = 0; k <=63; k++ )
                     {
                         {
                             x_dis[mm][len] = len * sin(PI * n/32) * cos(PI * k / 32);
                             y_dis[mm][len] = len * sin(PI * n/32) * sin(PI * k / 32);
                             z_dis[mm][len] = len * cos(PI * n/32);
                             mm++;
                         }
                     }
                 }


         cout<<"create 3D_ray success"<<endl;



         vector<float> x_loc;
         vector<float> y_loc;
         vector<float> z_loc;
         vector<float> Pixe;
         for(int  i =0;i<32*64;i++)
         {
             for(int j=max_value+based_distance;j<max_value+based_distance+3;j++)
             {
                 double pixe=p4DImage->getValueUINT8(x_location+x_dis[i][j],y_location+y_dis[i][j],z_location+z_dis[i][j],0);
                     x_loc.push_back(x_location-1+x_dis[i][j]);
                     y_loc.push_back(y_location-1+y_dis[i][j]);
                     z_loc.push_back(z_location-1+z_dis[i][j]);
                    // cout<<"the x location is "<<x_location-1+ray_x[i][j]<<" "<<"the y location is "<<y_location-1+ray_y[i][j]<<endl;
                 Pixe.push_back(pixe);
             }
         }


         v3d_msg(QString("x is %1, y is %2, The pixe of source point is %3").arg(x_location-1).arg(y_location-1).arg(datald_mip[(y_location-1)*nx+x_location-1]));




         float change_x1=cos(ang*3)*(max_value+based_distance+5)-cos(ang*2)*(max_value+based_distance+2);
         float change_x=pow(change_x1,2);
         float change_y1=sin(ang*3)*(max_value+based_distance+5)-sin(ang*2)*(max_value+based_distance+2);
         float change_y=pow(change_y1,2);

         float DB_radius=sqrt(change_x+change_y)+1;
         cout<<"the radius is:"<<DB_radius<<endl;
         cout<<"the size of x location is :"<<x_loc.size()<<endl;
         mycluster.Read_from_coordiante(x_loc,y_loc,DB_radius,3);
         int flag=mycluster.DoDBSCANRecursive();
         v3d_msg(QString("the cluster is %1").arg(flag));



         RGBA8 red; red.r=255; red.g=0; red.b=0;
         RGBA8 green; green.r=0; green.g=255; green.b=0;
         RGBA8 blue; blue.r=0; blue.g=0; blue.b=255;

         v3d_msg(QString("the x_loc size is %1").arg(x_loc.size()));
         for(V3DLONG i = 0; i < x_loc.size(); i++)
         {
             s.x=  x_loc[i];
             s.y = y_loc[i];
             s.z = z_loc[i];
             s.radius = 1;
             if(Pixe.at(i)<100)
             {
                 s.color=blue;
             }
             else {
                 s.color = red;
             }
             //s.color = red;
             curlist<<s;
         }

         callback.setImage(curwin, p4DImage);
         callback.updateImageWindow(curwin);
         callback.setLandmark(curwin, curlist);
         return 1;

}


int rotate_3D_slice_image(V3DPluginCallback2 &callback,QWidget *parent)
{
    ClusterAnalysis mycluster;
    Branch_Detection_Model pp;
    pp.Get_Image_Date(callback);
    pp.Gui_input_Rorate_3D_Method();

    // set the parameter
    int thres_2d=pp.thres_2d;
    int window_size=pp.window_size;
    int ray_numbers_2d=pp.ray_numbers_2d;
    int based_distance=pp.based_distance;
    int num_layer=pp.num_layer;

    V3DLONG nx=pp.sz[0];
    V3DLONG ny=pp.sz[1];
    V3DLONG nz=pp.sz[2];

    Image4DSimple *p4DImage = pp.p4DImage;
    V3DLONG size_image=pp.size_2D_image;

    unsigned char* datald=0;
    datald=pp.datald;

            unsigned char *image_mip;
            unsigned char *image_binary;
            unsigned char *old_image_binary;

            vector<V3DLONG> X_coor;
            vector<V3DLONG> Y_coor;
            vector<float> soma_radiu;


            /* create the ratate matrix */
            vector<vector<float> > ray_x(ray_numbers_2d,vector<float>(100)), ray_y(ray_numbers_2d,vector<float>(100));
            float ang = 2*PI/ray_numbers_2d;
            float x_dis, y_dis;
            int max_length=90;
            for(int i = 0; i < ray_numbers_2d; i++)
            {
               x_dis = cos(ang*(i+1));
               y_dis = sin(ang*(i+1));
               for(int j = 0; j<max_length ; j++)
                   {
                       ray_x[i][j] = x_dis*(j+1);
                       ray_y[i][j] = y_dis*(j+1);
                   }
            }
            /* difine the color */
            try{image_mip=new unsigned char [size_image];}
            catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

            try{image_binary=new unsigned char [size_image];}
            catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

            try{old_image_binary=new unsigned char [size_image];}
            catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

            cout<<"allocate the memory is successful"<<endl;

            LandmarkList curlist;
            LocationSimple s;
            for(int size_z=0;size_z<nz;size_z=size_z+num_layer)
            {
                /* detect the 2D branch points in each MIP */
                if((size_z+num_layer)<nz)
                {
                   z_slice_mip(nx,ny,size_z,datald,image_mip,num_layer);
                   thres_segment(size_image,image_mip,image_binary,thres_2d);
                   for(V3DLONG a=0;a<nx*ny;a++)
                   {
                       old_image_binary[a]=image_binary[a];
                   }
                   skeletonization(nx,ny,image_binary);
                   vector<V3DLONG> X_candidate;  // save the adjusted x coordinate;
                   vector<V3DLONG> Y_candidate;  // save the adjusted y coordinate;
                   vector<float> Max_value;      // save the radiu;
                   int count=0; // the number of candidate
                   vector<V3DLONG> X_loc; //save the non-adjusted x coordinate;
                   vector<V3DLONG> Y_loc; //save the non-adjusted y coordinate;
                  // skeletonization(nx,ny,image_binary);
                   seek_2D_candidate_points(nx,ny,image_binary,X_loc,Y_loc,count);
                   if(count>0)
                   {
                       for(int i=0;i<count;i++)
                       {
                           if(((X_loc.at(i)-window_size)<=0)||((Y_loc.at(i)-window_size)<=0)||((Y_loc.at(i)+window_size)>=ny)||((X_loc.at(i)+window_size)>=nx))
                           {
                               continue;
                           }
                           /* using Spherical growth method designed by Chaowang*/
                           V3DLONG adjusted_x, adjusted_y;
                           double radiu;
                           find_neighborhood_maximum_radius(X_loc.at(i),Y_loc.at(i),old_image_binary,window_size,adjusted_x,adjusted_y,radiu,nx,ny,thres_2d);
                           X_candidate.push_back(adjusted_x);  // all adjusted x coordinate of candidate points
                           Y_candidate.push_back(adjusted_y);  // all adjusted y coordinate of candidate points
                           Max_value.push_back(radiu);    // all radiu coordinate of candidate points
                           //cout<<"x is :"<<adjusted_x<<" "<<"y is :"<<adjusted_y<<" "<<"the radiu  is :"<<radiu<<endl;

                       }
                   }
                   else {

                       cout<<"no candidate points in this MIP"<<endl;
                   }
                   std::vector<float>::iterator biggest=std::max_element(std::begin(Max_value),std::end(Max_value));
                   int location=std::distance(std::begin(Max_value),biggest);
                   X_coor.push_back(X_candidate.at(location));
                   Y_coor.push_back(Y_candidate.at(location));
                   soma_radiu.push_back(*biggest);
                   cout<<"the all candidate points in each MIP have detected"<<endl;

                   for(V3DLONG k = 0; k < X_candidate.size(); k++)
                   {
                       vector<float> x_loc;
                       vector<float> y_loc;
                       //create a new mip
                       int block_radiu=3*Max_value.at(k);
                       int block_length=block_radiu*2+1;
                       unsigned char *block=0;
                       try{block=new unsigned char [block_length*block_length*block_radiu];}

                       catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}
                       //cout<<"x is :"<<X_candidate.at(k)<<" "<<" y is :"<<Y_candidate.at(k)<<" "<<"the length is "<<block_length<<endl;
                       if(((X_candidate.at(k)+block_radiu)>nx)||((X_candidate.at(k)-block_radiu)<0)||((Y_candidate.at(k)-block_radiu)<0)||((Y_candidate.at(k)+block_radiu)>ny))
                       {
                           cout<<"this points have beyong the image"<<endl;
                           continue;
                       }
                       int num_block=0;
                       for(V3DLONG b=Y_candidate.at(k)-block_radiu;b<=Y_candidate.at(k)+block_radiu;b++)
                       {
                            for(V3DLONG c=X_candidate.at(k)-block_radiu;c<=X_candidate.at(k)+block_radiu;c++)
                            {
                                unsigned char block_pixe=old_image_binary[b*nx+c];
                                block[num_block]=block_pixe;
                                num_block++;
                            }
                        }
                        delete_small_area(block_length,block_length,block);// connected domain denoising

                        for(int i = 0; i <ray_numbers_2d; i++)   //m is the numble of the ray
                        {
                            for(int j = Max_value.at(k)+based_distance; j < Max_value.at(k)+based_distance+3; j++)    // n is the numble of the points of the each ray
                            {
                                double pixe = project_interp_2d(block_radiu+ray_y[i][j], block_radiu+ray_x[i][j], block, block_length, block_length , block_radiu,  block_radiu);
                                if(pixe>=100)
                                {
                                    x_loc.push_back(block_radiu+ray_x[i][j]);
                                    y_loc.push_back(block_radiu+ray_y[i][j]);

                                }
                            }
                        }

                       /* calculate the DBscan radiu */
                       float change_x1=cos(ang*3)*(Max_value.at(k)+based_distance+3)-cos(ang*2)*(Max_value.at(k)+based_distance+3);
                       float change_x=pow(change_x1,2);
                       float change_y1=sin(ang*3)*(Max_value.at(k)+based_distance+3)-sin(ang*2)*(Max_value.at(k)+based_distance+3);
                       float change_y=pow(change_y1,2);
                       float DB_radius=sqrt(change_x+change_y)+1;
                       mycluster.Read_from_coordiante(x_loc,y_loc,DB_radius,3);
                       int flag=mycluster.DoDBSCANRecursive();

                       /* difine the color */
                       RGBA8 red; red.r=255; red.g=0; red.b=0;
                       RGBA8 green; green.r=0; green.g=255; green.b=0;
                       RGBA8 blue; blue.r=0; blue.g=0; blue.b=255;

                       if(flag==3)
                       {
                                        s.x=  X_candidate[k]+1;
                                        s.y = Y_candidate[k]+1;
                                        unsigned char max_z=0;
                                        int loc_z;
                                           for(int kk=size_z;kk<size_z+num_layer;kk++)
                                           {
                                               unsigned char sum_pixe;
                                               sum_pixe=datald[kk*nx*ny+Y_candidate[k]*nx+X_candidate[k]]+datald[kk*nx*ny+Y_candidate[k]*nx+X_candidate[k]+1]+datald[kk*nx*ny+Y_candidate[k]*nx+X_candidate[k]-1]+
                                                       datald[kk*nx*ny+(Y_candidate[k]+1)*nx+X_candidate[k]]+datald[kk*nx*ny+(Y_candidate[k]-1)*nx+X_candidate[k]]+
                                                       datald[kk*nx*ny+(Y_candidate[k]+1)*nx+X_candidate[k]-1]+datald[kk*nx*ny+(Y_candidate[k]+1)*nx+X_candidate[k]+1]+
                                                       datald[kk*nx*ny+(Y_candidate[k]-1)*nx+X_candidate[k]-1]+datald[kk*nx*ny+(Y_candidate[k]-1)*nx+X_candidate[k]+1];
                                               if(sum_pixe>max_z)
                                               {
                                                   max_z=sum_pixe;
                                                   loc_z=kk;
                                               }
                                           }
                                        s.z = size_z;
                                        s.radius = 1;
                                        s.color = blue;
                                        curlist<<s;
                       }

                   }
                }
            }

            std::vector<float>::iterator biggest=std::max_element(std::begin(soma_radiu),std::end(soma_radiu));
            int new_location=std::distance(std::begin(soma_radiu),biggest);
            int max_radiu=*biggest;
            V3DLONG soma_x=X_coor.at(new_location);
            V3DLONG soma_y=Y_coor.at(new_location);
            cout<<"the soma x is :"<<soma_x<<" "<<"the soma y is :"<<soma_y<<" "<<"the radiu is :"<<max_radiu<<endl;

            for(V3DLONG i=0;i<curlist.size();i++)
            {
                if(((curlist.at(i).x-soma_x)<max_radiu)&&((curlist.at(i).y-soma_y)<max_radiu))
                {
                    curlist.removeAt(i);
                }
            }
            for(V3DLONG i=0;i<curlist.size();i++)
            {
                unsigned char max_pixe=0;
                for(V3DLONG j=1;j<nz;j++)
                {
                    unsigned char z_pixe=p4DImage->getValueUINT8(curlist[i].x,curlist[i].y,j,0);
                    if(z_pixe>max_pixe)
                    {
                        max_pixe=z_pixe;
                    }

                }
                unsigned char new_pixe=p4DImage->getValueUINT8(curlist[i].x,curlist[i].y,curlist[i].z,0);
                if(new_pixe<max_pixe)
                {
                   curlist.removeAt(i);
                }

            }


            for(double i=0;i<curlist.size();i++)
            {
                for(double j=i+1;j<curlist.size();j++)
                {
                    if(((square(curlist[j].x-curlist[i].x)+square(curlist[j].y-curlist[i].y))<50)&&(square(curlist[j].z-curlist[i].z)<500))
                    {
                        curlist[i].x=(curlist[j].x+curlist[i].x)/2;
                        curlist[i].y=(curlist[j].y+curlist[i].y)/2;
                        curlist[i].z=(curlist[j].z+curlist[i].z)/2;
                        curlist.removeAt(j);
                        j=j-1;
                    }
                }
            }


            Image4DSimple * new4DImage = new Image4DSimple();
            new4DImage->setData((unsigned char *)datald, nx, ny, nz, p4DImage->getCDim(), p4DImage->getDatatype());
            v3dhandle newwin = callback.newImageWindow();
            callback.setImage(newwin, new4DImage);
            callback.updateImageWindow(newwin);
            callback.setLandmark(newwin, curlist);
            delete [] image_mip;
            delete []image_binary;
            delete []old_image_binary;
            return 1;



}
