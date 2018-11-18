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
//#include "../../zhi/APP2_large_scale/my_surf_objs.h"
#define PI 3.1415926
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
        else if(menu_name==tr("thin_branch_points_detection"))
        {
            int flag=thin(callback,parent);
            if(flag==1)
            v3d_msg("2D branch points completed ");
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
            int ray_length_2d = 8;
            int num_layer=4;


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
       unsigned char *image_mip;
       unsigned char *image_binary;
       unsigned char *old_image_binary;

       try{image_mip=new unsigned char [spage];}
       catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

       try{image_binary=new unsigned char [size_image];}
       catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

       try{old_image_binary=new unsigned char [size_image];}
       catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

       vector<delete_piont> delete_list;
       int neighbor[9];
       int sum_points;
       int new_neighbor[9];
       LandmarkList curlist;
       LocationSimple s;
       for(int size_z=0;size_z<nz;size_z=size_z+num_layer)
       {
           if((size_z+num_layer)<nz)
           {
               mip(nx,ny,size_z,datald,image_mip,num_layer);
               thres_segment(size_image,image_mip,image_binary,thres_2d);
               cout<<"get the 2D mip iamge"<<endl;

               while(true)
               {
                   for(V3DLONG j=1;j<ny;j++)
                   {
                       for(V3DLONG i=1;i<nx;i++)
                       {
                           if(image_binary[j*nx+i]>0)
                           {
                               if (image_binary[(j-1)*nx+i] == 255) neighbor[1] = 1;
                               else  neighbor[1] = 0;
                               if (image_binary[(j-1)*nx+i+1] == 255) neighbor[2] = 1;
                               else  neighbor[2] = 0;
                               if (image_binary[j*nx+i+1] == 255) neighbor[3] = 1;
                               else  neighbor[3] = 0;
                               if (image_binary[(j+1)*nx+i+1] == 255) neighbor[4] = 1;
                               else  neighbor[4] = 0;
                               if (image_binary[(j+1)*nx+i] == 255) neighbor[5] = 1;
                               else  neighbor[5] = 0;
                               if (image_binary[(j+1)*nx+i-1] == 255) neighbor[6] = 1;
                               else  neighbor[6] = 0;
                               if (image_binary[j*nx+i-1] == 255) neighbor[7] = 1;
                               else  neighbor[7] = 0;
                               if (image_binary[(j-1)*nx+i-1] == 255) neighbor[8] = 1;
                               else  neighbor[8] = 0;
                              // v3d_msg(QString(" x is %1,y is %2,p2 is %3,p4 is %4,p6 is %5").arg(i).arg(j).arg(neighbor[1]).arg(neighbor[3]).arg(neighbor[5]));
                               sum_points=0;
                               for (int k = 1; k < 9; k++)
                                  {
                                      sum_points = sum_points + neighbor[k];
                                  }
                             //  v3d_msg(QString(" x is %1,y is %2,sum_points is %3").arg(i).arg(j).arg(sum_points));
                               if ((sum_points >= 2) && (sum_points <= 6))
                               {
                                   int ap = 0;
                                   if ((neighbor[1] == 0) && (neighbor[2] == 1)) ap++;
                                   if ((neighbor[2] == 0) && (neighbor[3] == 1)) ap++;
                                   if ((neighbor[3] == 0) && (neighbor[4] == 1)) ap++;
                                   if ((neighbor[4] == 0) && (neighbor[5] == 1)) ap++;
                                   if ((neighbor[5] == 0) && (neighbor[6] == 1)) ap++;
                                   if ((neighbor[6] == 0) && (neighbor[7] == 1)) ap++;
                                   if ((neighbor[7] == 0) && (neighbor[8] == 1)) ap++;
                                   if ((neighbor[8] == 0) && (neighbor[1] == 1)) ap++;
                                   if (ap == 1)
                                   {
                                       if (((neighbor[1] * neighbor[3] * neighbor[5]) == 0) && ((neighbor[3] * neighbor[5] * neighbor[7]) == 0))
                                       {
                                           delete_piont pp;
                                           pp.xx=i;
                                           pp.yy=j;
                                           delete_list.push_back(pp);
                                       }
                                   }
                               }
                            }

                       }
                   }
                   if (delete_list.size() == 0) break;
                   for (V3DLONG num = 0; num < delete_list.size(); num++)
                   {
                     image_binary[delete_list[num].xx+delete_list[num].yy*nx]=0;
                   }
                   delete_list.clear();

                   cout<<"thin the image seccessful 1"<<endl;


                   for(V3DLONG j=1;j<ny;j++)
                   {
                       for(V3DLONG i=1;i<nx;i++)
                       {
                           if(image_binary[j*nx+i]>0)
                           {
                               if (image_binary[(j-1)*nx+i] == 255) neighbor[1] = 1;
                               else  neighbor[1] = 0;
                               if (image_binary[(j-1)*nx+i + 1] == 255) neighbor[2] = 1;
                               else  neighbor[2] = 0;
                               if (image_binary[j*nx+i + 1] == 255) neighbor[3] = 1;
                               else  neighbor[3] = 0;
                               if (image_binary[(j+1)*nx+i + 1] == 255) neighbor[4] = 1;
                               else  neighbor[4] = 0;
                               if (image_binary[(j+1)*nx+i] == 255) neighbor[5] = 1;
                               else  neighbor[5] = 0;
                               if (image_binary[(j+1)*nx+i - 1] == 255) neighbor[6] = 1;
                               else  neighbor[6] = 0;
                               if (image_binary[j*nx+i - 1] == 255) neighbor[7] = 1;
                               else  neighbor[7] = 0;
                               if (image_binary[(j-1)*nx+i - 1] == 255) neighbor[8] = 1;
                               else  neighbor[8] = 0;

                               sum_points=0;
                               for (int k = 1; k < 9; k++)
                                  {
                                      sum_points = sum_points + neighbor[k];
                                  }

                               if ((sum_points >= 2) && (sum_points <= 6))
                               {
                                   int ap = 0;
                                   if ((neighbor[1] == 0) && (neighbor[2] == 1)) ap++;
                                   if ((neighbor[2] == 0) && (neighbor[3] == 1)) ap++;
                                   if ((neighbor[3] == 0) && (neighbor[4] == 1)) ap++;
                                   if ((neighbor[4] == 0) && (neighbor[5] == 1)) ap++;
                                   if ((neighbor[5] == 0) && (neighbor[6] == 1)) ap++;
                                   if ((neighbor[6] == 0) && (neighbor[7] == 1)) ap++;
                                   if ((neighbor[7] == 0) && (neighbor[8] == 1)) ap++;
                                   if ((neighbor[8] == 0) && (neighbor[1] == 1)) ap++;
                                   if (ap == 1)
                                   {
                                       if (((neighbor[1] * neighbor[5] * neighbor[7]) == 0) && ((neighbor[1] * neighbor[3] * neighbor[7]) == 0))
                                       {
                                           delete_piont pp;
                                           pp.xx=i;
                                           pp.yy=j;
                                           delete_list.push_back(pp);
                                       }
                                   }
                               }
                            }

                       }
                   }
                   if (delete_list.size() == 0) break;
                   for (V3DLONG num = 0; num < delete_list.size(); num++)
                   {
                     image_binary[delete_list[num].xx+delete_list[num].yy*nx]=0;
                   }
                   delete_list.clear();
               }


               cout<<"thin the image seccessful"<<endl;
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
                               int flag8=rayinten_2D(j,i,ray_numbers_2d ,ray_length_2d,ray_x, ray_y, image_binary,nx,ny);
                               int flag10=rayinten_2D(j,i,ray_numbers_2d ,ray_length_2d,ray_x, ray_y, image_binary,nx,ny);
                               int flag12=rayinten_2D(j,i,ray_numbers_2d ,ray_length_2d,ray_x, ray_y, image_binary,nx,ny);
                               int flag14=rayinten_2D(j,i,ray_numbers_2d ,ray_length_2d,ray_x, ray_y, image_binary,nx,ny);
                               int flag16=rayinten_2D(j,i,ray_numbers_2d ,ray_length_2d,ray_x, ray_y, image_binary,nx,ny);
                               int sum_flag=flag8+flag10+flag12+flag14+flag16;
                               if(sum_flag>=1)
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
    cout<<"\nThis is a demo plugin to detect tip point in an image. by Keran Lin 2017-04"<<endl;
    cout<<"\nUsage: v3d -x <example_plugin_name> -f tip_detection -i <input_image_file> -o <output_image_file> -p <subject_color_channel> <threshold>"<<endl;
    cout<<"\t -i <input_image_file>                      input 3D image (tif, raw or lsm)"<<endl;
    cout<<"\t -o <output_image_file>                     output image of the thresholded subject channel"<<endl;
    cout<<"\t                                            the  paras must come in this order"<<endl;
    cout<<"\nDemo: v3d -x libexample_debug.dylib -f image_thresholding -i input.tif -o output.tif -p 0 100\n"<<endl;
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
      int ray_length_2d = 8;


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
      V3DLONG nx=sz[0],ny=sz[1],nz=sz[2];
      V3DLONG size_image=nx*ny;

      unsigned char* datald=0;
      datald = p4DImage->getRawData();

      vector<vector<float> > ray_x(ray_numbers_2d,vector<float>(ray_length_2d)), ray_y(ray_numbers_2d,vector<float>(ray_length_2d));

//      cout<<"create 2D_ray"<<endl;

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


      unsigned char *image_mip=0;
      try{image_mip=new unsigned char [size_image];}
      catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}
      Z_mip(nx,ny,nz,datald,image_mip);
      v3d_msg("mip have complete");

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

      for(V3DLONG i=0;i<size_image;i++)
      {
          old_image_binary[i]=image_binary[i];
      }


      LandmarkList curlist;
      LocationSimple s;      

      vector<V3DLONG> step1_delete_piont;
      vector<V3DLONG> step2_delete_piont;
      int neighbor[10];
      int sum_points;
      while(true)
      {
          for(V3DLONG j=1;j<ny-1;j++)
          {
              for(V3DLONG i=1;i<nx-1;i++)
              {
                  if(image_binary[j*nx+i]>0)
                  {
                      //v3d_msg(QString(" x is %1,y is %2").arg(i).arg(j));
                      //v3d_msg(QString(" location is %1,location is %2,location is %3").arg(j*nx+i).arg((j+1)*nx+i).arg((j-1)*nx+i));
                      if (image_binary[(j-1)*nx+i] == 255) neighbor[1] = 1;
                      else  neighbor[1] = 0;
                      if (image_binary[(j-1)*nx+i+1] == 255) neighbor[2] = 1;
                      else  neighbor[2] = 0;
                      if (image_binary[j*nx+i+1] == 255) neighbor[3] = 1;
                      else  neighbor[3] = 0;
                      if (image_binary[(j+1)*nx+i+1] == 255) neighbor[4] = 1;
                      else  neighbor[4] = 0;
                      if (image_binary[(j+1)*nx+i] == 255) neighbor[5] = 1;
                      else  neighbor[5] = 0;
                      if (image_binary[(j+1)*nx+i-1] == 255) neighbor[6] = 1;
                      else  neighbor[6] = 0;
                      if (image_binary[j*nx+i-1] == 255) neighbor[7] = 1;
                      else  neighbor[7] = 0;
                      if (image_binary[(j-1)*nx+i-1] == 255) neighbor[8] = 1;
                      else  neighbor[8] = 0;
                      //v3d_msg(QString(" x is %1,y is %2,p2 is %3,p4 is %4,p6 is %5").arg(i).arg(j).arg(neighbor[1]).arg(neighbor[3]).arg(neighbor[5]));
                      sum_points=0;
                      for (int k = 1; k < 9; k++)
                         {
                             sum_points = sum_points + neighbor[k];
                         }
                      //v3d_msg(QString(" x is %1,y is %2,sum_points is %3").arg(i).arg(j).arg(sum_points));
                      if ((sum_points >= 2) && (sum_points <= 6))
                      {
                          //v3d_msg(QString(" x is %1,y is %2").arg(i).arg(j));
                          int ap = 0;
                          if ((neighbor[1] == 0) && (neighbor[2] == 1)) ap++;
                          if ((neighbor[2] == 0) && (neighbor[3] == 1)) ap++;
                          if ((neighbor[3] == 0) && (neighbor[4] == 1)) ap++;
                          if ((neighbor[4] == 0) && (neighbor[5] == 1)) ap++;
                          if ((neighbor[5] == 0) && (neighbor[6] == 1)) ap++;
                          if ((neighbor[6] == 0) && (neighbor[7] == 1)) ap++;
                          if ((neighbor[7] == 0) && (neighbor[8] == 1)) ap++;
                          if ((neighbor[8] == 0) && (neighbor[1] == 1)) ap++;
                          if (ap == 1)
                          {
                              if (((neighbor[1] * neighbor[3] * neighbor[5]) == 0) && ((neighbor[3] * neighbor[5] * neighbor[7]) == 0))
                              {
                                  step1_delete_piont.push_back(j*nx+i);

                              }
                          }
                      }
                   }

              }
          }
          for(V3DLONG num=0;num<step1_delete_piont.size();num++)
          {
              image_binary[step1_delete_piont[num]]=0;
          }
          if (step1_delete_piont.size() == 0)
          {
              break;
          }
          else
          {
              step1_delete_piont.clear();
          }

          cout<<"step 1 is complete"<<endl;

          for(V3DLONG jj=1;jj<ny-1;jj++)
          {
              for(V3DLONG ii=1;ii<nx-1;ii++)
              {
                  if(image_binary[jj*nx+ii]>0)
                  {
                      if (image_binary[(jj-1)*nx+ii] == 255) neighbor[1] = 1;
                      else  neighbor[1] = 0;
                      if (image_binary[(jj-1)*nx+ii + 1] == 255) neighbor[2] = 1;
                      else  neighbor[2] = 0;
                      if (image_binary[jj*nx+ii + 1] == 255) neighbor[3] = 1;
                      else  neighbor[3] = 0;
                      if (image_binary[(jj+1)*nx+ii + 1] == 255) neighbor[4] = 1;
                      else  neighbor[4] = 0;
                      if (image_binary[(jj+1)*nx+ii] == 255) neighbor[5] = 1;
                      else  neighbor[5] = 0;
                      if (image_binary[(jj+1)*nx+ii - 1] == 255) neighbor[6] = 1;
                      else  neighbor[6] = 0;
                      if (image_binary[jj*nx+ii - 1] == 255) neighbor[7] = 1;
                      else  neighbor[7] = 0;
                      if (image_binary[(jj-1)*nx+ii - 1] == 255) neighbor[8] = 1;
                      else  neighbor[8] = 0;
                      sum_points=0;
                      for (int k = 1; k < 9; k++)
                         {
                             sum_points = sum_points + neighbor[k];
                         }
                      if ((sum_points >= 2) && (sum_points <= 6))
                      {
                          int ap = 0;
                          if ((neighbor[1] == 0) && (neighbor[2] == 1)) ap++;
                          if ((neighbor[2] == 0) && (neighbor[3] == 1)) ap++;
                          if ((neighbor[3] == 0) && (neighbor[4] == 1)) ap++;
                          if ((neighbor[4] == 0) && (neighbor[5] == 1)) ap++;
                          if ((neighbor[5] == 0) && (neighbor[6] == 1)) ap++;
                          if ((neighbor[6] == 0) && (neighbor[7] == 1)) ap++;
                          if ((neighbor[7] == 0) && (neighbor[8] == 1)) ap++;
                          if ((neighbor[8] == 0) && (neighbor[1] == 1)) ap++;
                          if (ap == 1)
                          {
                              if (((neighbor[1] * neighbor[5] * neighbor[7]) == 0) && ((neighbor[1] * neighbor[3] * neighbor[7]) == 0))
                              {
                                  //cout<<"come step2"<<endl;
                                  step2_delete_piont.push_back(jj*nx+ii);
                              }
                          }
                      }
                   }

              }
          }
          for(V3DLONG num=0;num<step2_delete_piont.size();num++)
          {
              image_binary[step2_delete_piont[num]]=0;
          }
          if (step2_delete_piont.size() == 0)
          {
              break;
          }
          else
          {
              step2_delete_piont.clear();
          }

          cout<<"step 2 is complete"<<endl;
      }

      v3d_msg("skeletonization have complete");
      int new_neighbor[9];
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
                      int flag=rayinten_2D(j,i,ray_numbers_2d ,ray_length_2d,ray_x, ray_y, old_image_binary,nx,ny);
//                      int flag2=rayinten_2D(j,i,ray_numbers_2d ,(ray_length_2d+4),ray_x, ray_y, image_binary,nx,ny);
//                      int flag3=rayinten_2D(j,i,ray_numbers_2d ,(ray_length_2d+6),ray_x, ray_y, image_binary,nx,ny);
//                      int flag4=rayinten_2D(j,i,ray_numbers_2d ,(ray_length_2d+8),ray_x, ray_y, image_binary,nx,ny);
                      //v3d_msg(QString("x is %1, y is %2").arg(i).arg(j));
                     // int sum_flag=flag;
                      if(flag==1)
                      {
                          s.x=i;
                          s.y=j;
                          s.z=1;
                          s.radius=1;
                          s.color = random_rgba8(255);
                          curlist << s;
                          num++;

                      }
                   }
            }

          }
      }
      v3d_msg(QString("do you need show the iamge"));

      if(image_binary) {delete []image_binary; image_binary = 0;}
//            if(image_mip) {delete []image_mip; image_mip = 0;}
 //           if(datald){delete []datald;datald=0;}
      for(double i=0;i<curlist.size();i++)
      {
          for(double j=i+1;j<curlist.size();j++)
          {
              if(square(curlist[j].x-curlist[i].x)+square(curlist[j].y-curlist[i].y)+square(curlist[j].z-curlist[i].z)<50)
              {
                  curlist[i].x=(curlist[j].x+curlist[i].x)/2;
                  curlist[i].y=(curlist[j].y+curlist[i].y)/2;
                  curlist[i].z=(curlist[j].z+curlist[i].z)/2;
                  curlist.removeAt(j);
                  j=j-1;
              }
          }
      }

//      callback.setImage(curwin, p4DImage);
//      callback.setImageName(curwin, "maximum intensity projection image");
//      callback.updateImageWindow(curwin);
//      callback.setLandmark(curwin, curlist);
        v3d_msg(QString("single numble of marker is %1").arg(curlist.size()));
//        Image4DSimple * new4DImage = new Image4DSimple();
//        new4DImage->setData((unsigned char *)image_mip, nx, ny, 1, p4DImage->getCDim(), p4DImage->getDatatype());
//        v3dhandle newwin = callback.newImageWindow();
//        callback.setImage(newwin, new4DImage);
//        callback.updateImageWindow(newwin);
//        callback.setLandmark(newwin, curlist);

        Image4DSimple * new4DImage = new Image4DSimple();
        new4DImage->setData((unsigned char *)old_image_binary, nx, ny, 1, p4DImage->getCDim(), p4DImage->getDatatype());
        v3dhandle newwin = callback.newImageWindow();
        callback.setImage(newwin, new4DImage);
        //callback.setImageName(newwin, title);
        callback.updateImageWindow(newwin);
         callback.setLandmark(newwin, curlist);
      return 1;
}

int thin(V3DPluginCallback2 &callback, QWidget *parent)
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
      int ray_length_2d = 8;


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
      V3DLONG nx=sz[0],ny=sz[1],nz=sz[2];
      V3DLONG size_image=nx*ny;

      unsigned char* datald=0;
      datald = p4DImage->getRawData();

      vector<vector<float> > ray_x(ray_numbers_2d,vector<float>(ray_length_2d)), ray_y(ray_numbers_2d,vector<float>(ray_length_2d));

//      cout<<"create 2D_ray"<<endl;

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


      unsigned char *image_mip=0;
      try{image_mip=new unsigned char [size_image];}
      catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}
      Z_mip(nx,ny,nz,datald,image_mip);
      v3d_msg("mip have complete");

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

      for(V3DLONG i=0;i<size_image;i++)
      {
          old_image_binary[i]=image_binary[i];
      }


      LandmarkList curlist;
      LocationSimple s;

      vector<V3DLONG> step1_delete_piont;
      vector<V3DLONG> step2_delete_piont;
      int neighbor[10];
      int sum_points;
      while(true)
      {
          for(V3DLONG j=1;j<ny-1;j++)
          {
              for(V3DLONG i=1;i<nx-1;i++)
              {
                  if(image_binary[j*nx+i]>0)
                  {
                      //v3d_msg(QString(" x is %1,y is %2").arg(i).arg(j));
                      //v3d_msg(QString(" location is %1,location is %2,location is %3").arg(j*nx+i).arg((j+1)*nx+i).arg((j-1)*nx+i));
                      if (image_binary[(j-1)*nx+i] == 255) neighbor[1] = 1;
                      else  neighbor[1] = 0;
                      if (image_binary[(j-1)*nx+i+1] == 255) neighbor[2] = 1;
                      else  neighbor[2] = 0;
                      if (image_binary[j*nx+i+1] == 255) neighbor[3] = 1;
                      else  neighbor[3] = 0;
                      if (image_binary[(j+1)*nx+i+1] == 255) neighbor[4] = 1;
                      else  neighbor[4] = 0;
                      if (image_binary[(j+1)*nx+i] == 255) neighbor[5] = 1;
                      else  neighbor[5] = 0;
                      if (image_binary[(j+1)*nx+i-1] == 255) neighbor[6] = 1;
                      else  neighbor[6] = 0;
                      if (image_binary[j*nx+i-1] == 255) neighbor[7] = 1;
                      else  neighbor[7] = 0;
                      if (image_binary[(j-1)*nx+i-1] == 255) neighbor[8] = 1;
                      else  neighbor[8] = 0;
                      //v3d_msg(QString(" x is %1,y is %2,p2 is %3,p4 is %4,p6 is %5").arg(i).arg(j).arg(neighbor[1]).arg(neighbor[3]).arg(neighbor[5]));
                      sum_points=0;
                      for (int k = 1; k < 9; k++)
                         {
                             sum_points = sum_points + neighbor[k];
                         }
                      //v3d_msg(QString(" x is %1,y is %2,sum_points is %3").arg(i).arg(j).arg(sum_points));
                      if ((sum_points >= 2) && (sum_points <= 6))
                      {
                          //v3d_msg(QString(" x is %1,y is %2").arg(i).arg(j));
                          int ap = 0;
                          if ((neighbor[1] == 0) && (neighbor[2] == 1)) ap++;
                          if ((neighbor[2] == 0) && (neighbor[3] == 1)) ap++;
                          if ((neighbor[3] == 0) && (neighbor[4] == 1)) ap++;
                          if ((neighbor[4] == 0) && (neighbor[5] == 1)) ap++;
                          if ((neighbor[5] == 0) && (neighbor[6] == 1)) ap++;
                          if ((neighbor[6] == 0) && (neighbor[7] == 1)) ap++;
                          if ((neighbor[7] == 0) && (neighbor[8] == 1)) ap++;
                          if ((neighbor[8] == 0) && (neighbor[1] == 1)) ap++;
                          if (ap == 1)
                          {
                              if (((neighbor[1] * neighbor[3] * neighbor[5]) == 0) && ((neighbor[3] * neighbor[5] * neighbor[7]) == 0))
                              {
                                  step1_delete_piont.push_back(j*nx+i);

                              }
                          }
                      }
                   }

              }
          }
          for(V3DLONG num=0;num<step1_delete_piont.size();num++)
          {
              image_binary[step1_delete_piont[num]]=0;
          }
          if (step1_delete_piont.size() == 0)
          {
              break;
          }
          else
          {
              step1_delete_piont.clear();
          }

          cout<<"step 1 is complete"<<endl;

          for(V3DLONG jj=1;jj<ny-1;jj++)
          {
              for(V3DLONG ii=1;ii<nx-1;ii++)
              {
                  if(image_binary[jj*nx+ii]>0)
                  {
                      if (image_binary[(jj-1)*nx+ii] == 255) neighbor[1] = 1;
                      else  neighbor[1] = 0;
                      if (image_binary[(jj-1)*nx+ii + 1] == 255) neighbor[2] = 1;
                      else  neighbor[2] = 0;
                      if (image_binary[jj*nx+ii + 1] == 255) neighbor[3] = 1;
                      else  neighbor[3] = 0;
                      if (image_binary[(jj+1)*nx+ii + 1] == 255) neighbor[4] = 1;
                      else  neighbor[4] = 0;
                      if (image_binary[(jj+1)*nx+ii] == 255) neighbor[5] = 1;
                      else  neighbor[5] = 0;
                      if (image_binary[(jj+1)*nx+ii - 1] == 255) neighbor[6] = 1;
                      else  neighbor[6] = 0;
                      if (image_binary[jj*nx+ii - 1] == 255) neighbor[7] = 1;
                      else  neighbor[7] = 0;
                      if (image_binary[(jj-1)*nx+ii - 1] == 255) neighbor[8] = 1;
                      else  neighbor[8] = 0;
                      sum_points=0;
                      for (int k = 1; k < 9; k++)
                         {
                             sum_points = sum_points + neighbor[k];
                         }
                      if ((sum_points >= 2) && (sum_points <= 6))
                      {
                          int ap = 0;
                          if ((neighbor[1] == 0) && (neighbor[2] == 1)) ap++;
                          if ((neighbor[2] == 0) && (neighbor[3] == 1)) ap++;
                          if ((neighbor[3] == 0) && (neighbor[4] == 1)) ap++;
                          if ((neighbor[4] == 0) && (neighbor[5] == 1)) ap++;
                          if ((neighbor[5] == 0) && (neighbor[6] == 1)) ap++;
                          if ((neighbor[6] == 0) && (neighbor[7] == 1)) ap++;
                          if ((neighbor[7] == 0) && (neighbor[8] == 1)) ap++;
                          if ((neighbor[8] == 0) && (neighbor[1] == 1)) ap++;
                          if (ap == 1)
                          {
                              if (((neighbor[1] * neighbor[5] * neighbor[7]) == 0) && ((neighbor[1] * neighbor[3] * neighbor[7]) == 0))
                              {
                                  //cout<<"come step2"<<endl;
                                  step2_delete_piont.push_back(jj*nx+ii);
                              }
                          }
                      }
                   }

              }
          }
          for(V3DLONG num=0;num<step2_delete_piont.size();num++)
          {
              image_binary[step2_delete_piont[num]]=0;
          }
          if (step2_delete_piont.size() == 0)
          {
              break;
          }
          else
          {
              step2_delete_piont.clear();
          }

          cout<<"step 2 is complete"<<endl;
      }

      v3d_msg("skeletonization have complete");
      int new_neighbor[9];
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
                      v3d_msg(QString(" x is %1,y is %2").arg(i).arg(j));
                          s.x=i;
                          s.y=j;
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
//            if(image_mip) {delete []image_mip; image_mip = 0;}
 //           if(datald){delete []datald;datald=0;}
      for(double i=0;i<curlist.size();i++)
      {
          for(double j=i+1;j<curlist.size();j++)
          {
              if(square(curlist[j].x-curlist[i].x)+square(curlist[j].y-curlist[i].y)+square(curlist[j].z-curlist[i].z)<250)
              {
                  curlist[i].x=(curlist[j].x+curlist[i].x)/2;
                  curlist[i].y=(curlist[j].y+curlist[i].y)/2;
                  curlist[i].z=(curlist[j].z+curlist[i].z)/2;
                  curlist.removeAt(j);
                  j=j-1;
              }
          }
      }

//      callback.setImage(curwin, p4DImage);
//      callback.setImageName(curwin, "maximum intensity projection image");
//      callback.updateImageWindow(curwin);
//      callback.setLandmark(curwin, curlist);
        v3d_msg(QString("single numble of marker is %1").arg(curlist.size()));
//        Image4DSimple * new4DImage = new Image4DSimple();
//        new4DImage->setData((unsigned char *)image_mip, nx, ny, 1, p4DImage->getCDim(), p4DImage->getDatatype());
//        v3dhandle newwin = callback.newImageWindow();
//        callback.setImage(newwin, new4DImage);
//        callback.updateImageWindow(newwin);
//        callback.setLandmark(newwin, curlist);

        Image4DSimple * new4DImage = new Image4DSimple();
        new4DImage->setData((unsigned char *)old_image_binary, nx, ny, 1, p4DImage->getCDim(), p4DImage->getDatatype());
        v3dhandle newwin = callback.newImageWindow();
        callback.setImage(newwin, new4DImage);
        //callback.setImageName(newwin, title);
        callback.updateImageWindow(newwin);
         callback.setLandmark(newwin, curlist);
      return 1;
}
