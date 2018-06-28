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
        <<tr("mip_corner_detection")
        <<tr("creat_ray-shooting_model")
        <<tr("corner_detection")
        <<tr(" Gaussian Filtering ")
        <<tr("calculate the MIP of a subset of Z slices")
        <<tr("about")
        <<tr("skeletonization");

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

            int flag= corner_detection(callback,parent);

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

        else if(menu_name==tr("mip_corner_detection"))
        {
            int flag=mip_corner_detection(callback,parent);
            if(flag==1)
            v3d_msg("MIP was ok ");

        }

        else if(menu_name==tr("skeletonization"))
        {
            int flag=skeletonization(callback,parent);
            if(flag==1)
            v3d_msg("skeletonization was ok ");

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

            int ray_numbers_2d = 128;
            int thres_2d = 45;
            int ray_length_2d = 16;
            int num_layer=2;


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
       V3DLONG size_image=sz[0]*sz[1]*sz[2];

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

       struct delete_piont
       {
           V3DLONG xx;
           V3DLONG yy;
       };
       vector<delete_piont> delete_list;
       int neighbor[8];
       int sum_points;
       int new_neighbor[8];
       int size_point;
       LandmarkList curlist;
       LocationSimple s;
       for(int size_z=0;size_z<nz;size_z=size_z+num_layer)
       {
           if((size_z+num_layer)<nz)
           {
               mip(nx,ny,size_z,datald,image_mip,num_layer);
               thres_segment(size_image,image_mip,image_binary,thres_2d);
               old_image_binary=image_binary;

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
                               //v3d_msg(QString(" x is %1,y is %2,p2 is %3,p4 is %4,p6 is %5").arg(i).arg(j).arg(neighbor[1]).arg(neighbor[3]).arg(neighbor[5]));
                               sum_points=0;
                               for (int k = 1; k < 9; k++)
                                  {
                                      sum_points = sum_points + neighbor[k];
                                  }
                               //v3d_msg(QString(" x is %1,y is %2,sum_points is %3").arg(i).arg(j).arg(sum_points));
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
                           //v3d_msg(QString("cnp is %1").arg(cnp));
                           if(cnp==3)
                           {
                               int flag=rayinten_2D(j,i,ray_numbers_2d ,ray_length_2d,ray_x, ray_y, old_image_binary,nx,ny);
                               //v3d_msg(QString("flag is %1").arg(flag));
                               if(flag==1)
                               {
                                   s.x=i;
                                   s.y=j;
                                   unsigned char max_z=0;
                                   int loc_z;
                                   for(int k=size_z;k<size_z+num_layer;k++)
                                   {

                                       if(datald[k*nx*ny+j*nx+i]>max_z)
                                       {
                                           max_z=datald[k*nx*ny+j*nx+i];
                                           //v3d_msg(QString("max_z is %1").arg(max_z));
                                           loc_z=k;
                                           s.z=k;
                                           //v3d_msg(QString("z_size is %1").arg(s.z));
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
        Image4DSimple * new4DImage = new Image4DSimple();
        new4DImage->setData((unsigned char *)datald, p4DImage->getXDim(), p4DImage->getYDim(), p4DImage->getZDim(), p4DImage->getCDim(), p4DImage->getDatatype());
        v3dhandle newwin = callback.newImageWindow();
        callback.setImage(newwin, new4DImage);
        callback.setImageName(newwin, "maximum intensity projection image");
        callback.updateImageWindow(newwin);
        v3d_msg(QString("numble of marker is %1").arg(curlist.size()));
        callback.setLandmark(newwin, curlist);
//       if(image_binary) {delete []image_binary; image_binary = 0;}
//        if(image_mip) {delete []image_mip; image_mip = 0;}
//        if(datald){delete []datald;datald=0;}
       if(old_image_binary){delete []old_image_binary;old_image_binary=0;}

        return 1;
}


int mip_corner_detection(V3DPluginCallback2 &callback, QWidget *parent)
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


    V3DLONG nx=sz[0];
    V3DLONG ny=sz[1];
    V3DLONG nz=sz[2];
    V3DLONG spage=nx*ny*nz;


    v3d_msg("mip");
    unsigned char *image_mip=0;
    try{image_mip=new unsigned char [spage];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}
    Z_mip(nx,ny,nz,datald,image_mip);
    v3d_msg("mip have complete");

    v3d_msg("2D segment");
    unsigned char *mip_binary=0;
    try{mip_binary=new unsigned char [size_image];}
    catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}
    thres_segment(size_image,image_mip,mip_binary,thres_2d);

    LandmarkList curlist;
    LocationSimple s;

    unsigned char *imagesobelx_mip=0;
    try{imagesobelx_mip=new unsigned char [spage];}
    catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

    unsigned char *imagesobely_mip=0;
    try{imagesobely_mip=new unsigned char [spage];}
    catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

    float *xx_mip=0;
    try{xx_mip=new float [spage];}
    catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

    float *yy_mip=0;
    try{yy_mip=new float [spage];}
    catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

    float *xy_mip=0;
    try{xy_mip=new float [spage];}
    catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

    float *Gxx_mip=0;
    try{Gxx_mip=new float [spage];}
    catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

    float *Gyy_mip=0;
    try{Gyy_mip=new float [spage];}
    catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

    float *Gxy_mip=0;
    try{Gxy_mip=new float [spage];}
    catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

    float *Hresult_mip=0;
    try{Hresult_mip=new float [spage];}
    catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}
    v3d_msg("allocate the memory");

     for (V3DLONG i = 1; i < ny - 1; i++)
     {
          for (V3DLONG j = 1; j < nx - 1; j++)
           {
                   //通过指针遍历图像上每一个像素
                   double gradY = mip_binary[(i + 1)*nx + j - 1] + mip_binary[(i + 1)*nx + j] * 2 + mip_binary[(i + 1)*nx + j + 1] - mip_binary[(i - 1)*nx + j - 1] - mip_binary[(i - 1)*nx + j] * 2 - mip_binary[(i - 1)*nx + j + 1];
                   imagesobelx_mip[i*nx + j] = abs(gradY);

                   double gradX = mip_binary[(i - 1)*nx + j + 1] + mip_binary[i*nx + j + 1] * 2 + mip_binary[(i + 1)*nx + j + 1] - mip_binary[(i - 1)*nx + j - 1] - mip_binary[i*nx + j - 1] * 2 - mip_binary[(i + 1)*nx + j - 1];
                   imagesobely_mip[i*nx + j] = abs(gradX);
           }
     }

    mul(imagesobelx_mip,xx_mip,nx,ny);  //x方向梯度的平方
    mul(imagesobely_mip,yy_mip,nx,ny);  //y方向梯度的平方
    mul_xy(imagesobelx_mip,imagesobely_mip,xy_mip,nx,ny);  //xy方向梯度的平方

    //gaussion filter
    float max_result=0.0;
    MyGaussianBlur(xx_mip,Gxx_mip,size_gaussion,sz[0],sz[1]);
    MyGaussianBlur(yy_mip,Gyy_mip,size_gaussion,sz[0],sz[1]);
    MyGaussianBlur(xy_mip,Gxy_mip,size_gaussion,sz[0],sz[1]);
    harrisResponse(Gxx_mip,Gyy_mip,Gxy_mip,Hresult_mip,0.05,sz[0],sz[1],max_result);
    v3d_msg(QString("max_result is %1").arg(max_result));

    int r = size_gaussion / 2;

    for (int i = r; i < ny-r; i++)
    {
        for (int j = r; j < nx - r; j++)
        {
            if (
                Hresult_mip[i*nx+j] > Hresult_mip[(i-1)*nx+j] &&
                Hresult_mip[i*nx+j] > Hresult_mip[(i-1)*nx+j-1] &&
                Hresult_mip[i*nx+j] > Hresult_mip[(i-1)*nx+j+1] &&
                Hresult_mip[i*nx+j] > Hresult_mip[i*nx+j-1] &&
                Hresult_mip[i*nx+j] > Hresult_mip[i*nx+j+1] &&
                Hresult_mip[i*nx+j] > Hresult_mip[(i+1)*nx+j-1] &&
                Hresult_mip[i*nx+j] > Hresult_mip[(i+1)*nx+j] &&
                Hresult_mip[i*nx+j] > Hresult_mip[(i+1)*nx+j+1])
            {

                //v3d_msg(QString("Hresult[i*nx+j] is %1").arg(Hresult[i*nx+j]));
                if (Hresult_mip[i*nx+j] > 8000)
                {
                        s.x=j+1;
                        s.y=i+1;
                        s.z=1;
                        s.radius=1;
                        s.color = random_rgba8(255);
                        curlist << s;

                }

          }

        }
    }

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
    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *)mip_binary, p4DImage->getXDim(), p4DImage->getYDim(), 1, p4DImage->getCDim(), p4DImage->getDatatype());
    //v3dhandle newwin = callback.newImageWindow();
    //v3dhandle newwin = callback.currentImageWindow();
    callback.setImage(curwin, new4DImage);
    callback.setImageName(curwin, "maximum intensity projection image");
    callback.updateImageWindow(curwin);
    v3d_msg(QString("numble of marker is %1").arg(curlist.size()));
    callback.setLandmark(curwin, curlist);

    //       if(image_binary) {delete []image_binary; image_binary = 0;}
    //       if(image_mip) {delete []image_mip; image_mip = 0;}
    //       if(datald){delete []datald;datald=0;}

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


    int x_point=0;
    int y_point=0;
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
    Z_mip(nx,ny,nz,datald,image_mip);

    v3d_msg("segment");
    unsigned char *image_binary=0;
    try{image_binary=new unsigned char [size_image];}
    catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}
    thres_segment(size_image,image_mip,image_binary,thres);


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
        float tt=interp_2d( y_lac[i], x_lac[i],image_binary, sz[0], sz[1],x_point,y_point);
        pixel.push_back(tt);
        //v3d_msg(QString("pixel is %1").arg(pixel[i]));
    }

    for(int i=0;i<num_points;i++)
    {
        v3d_msg(QString("pixel is %1,y is %2,pixel is %3").arg(x_lac[i]).arg(y_lac[i]).arg(pixel[i]));
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

int corner_detection(V3DPluginCallback2 &callback, QWidget *parent)
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


    int window_size = 3;
    int thres_2d = 45;
    int K_harris = 1;


    //set update the dialog
    QDialog * dialog = new QDialog();


    if(p4DImage->getZDim() > 1)
            dialog->setWindowTitle("3D neuron image tip point detection Based on Ray-shooting algorithm");
    else
            dialog->setWindowTitle("2D neuron image tip point detection Based on Ray-shooting algorithm");

QGridLayout * layout = new QGridLayout();

QSpinBox * window_size_spinbox = new QSpinBox();
window_size_spinbox->setRange(1,1000);
window_size_spinbox->setValue(window_size);

QSpinBox * thres_2d_spinbox = new QSpinBox();
thres_2d_spinbox->setRange(-1, 255);
thres_2d_spinbox->setValue(thres_2d);

QSpinBox * K_harris_spinbox = new QSpinBox();
K_harris_spinbox->setRange(1,10);
K_harris_spinbox->setValue(K_harris);


layout->addWidget(new QLabel("guassion size"),0,0);
layout->addWidget(window_size_spinbox, 0,1,1,5);

layout->addWidget(new QLabel("intensity threshold"),1,0);
layout->addWidget(thres_2d_spinbox, 1,1,1,5);

layout->addWidget(new QLabel("k thres"),2,0);
layout->addWidget(K_harris_spinbox, 2,1,1,5);


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
window_size = window_size_spinbox->value();
thres_2d = thres_2d_spinbox->value();
K_harris = K_harris_spinbox->value();
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
              float *imagesobelx=new float [nx*ny];
              float *imagesobely=new float [nx*ny];
              float *xx=new float[nx*ny];
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
                  new_datald[i*nx+j]=datald[z_size*nx*ny+i*nx+j];


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
        float max_hresult=0.0;
        MyGaussianBlur(xx,Gxx,window_size,sz[0],sz[1]);
        MyGaussianBlur(yy,Gyy,window_size,sz[0],sz[1]);
        MyGaussianBlur(xy,Gxy,window_size,sz[0],sz[1]);
        harrisResponse(Gxx,Gyy,Gxy,Hresult,0.06,sz[0],sz[1],max_hresult);
        v3d_msg(QString("max_hr is %1").arg(max_hresult));


        int r = window_size / 2;

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
                   // if (Hresult[i*nx+j] > (18000))
                    //{
                        s.x=j+1;
                        s.y=i+1;
                        s.z=z_size;
                        s.radius=1;
                        s.color = random_rgba8(255);
                        curlist << s;
                   // }
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

int skeletonization(V3DPluginCallback2 &callback, QWidget *parent)
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
      V3DLONG size_image=nx*ny*nz;

      unsigned char* datald=0;
      datald = p4DImage->getRawData();
      bool ok1;
      int thres = QInputDialog::getInteger(parent, "threshold  of this image ",
                                    "Enter threshold ",
                                    1, 1, 1000, 1, &ok1);
      struct delete_piont
      {
          V3DLONG xx;
          V3DLONG yy;
      };
      vector<delete_piont> delete_list;
      int neighbor[8];
      int sum_points;

      vector<vector<float> > ray_x(128,vector<float>(8)), ray_y(128,vector<float>(8));

      cout<<"create 2D_ray"<<endl;

      float ang = 2*PI/128;
      float x_dis, y_dis;

      for(int i = 0; i < 128; i++)
      {
          x_dis = cos(ang*(i+1));
          y_dis = sin(ang*(i+1));
          for(int j = 0; j<8; j++)
              {
                  ray_x[i][j] = x_dis*(j+1);
                  ray_y[i][j] = y_dis*(j+1);
              }
      }

      cout<<"create 2D_ray success"<<endl;

      v3d_msg("mip");
      unsigned char *image_mip=0;
      try{image_mip=new unsigned char [size_image];}
      catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}
      Z_mip(nx,ny,nz,datald,image_mip);
      v3d_msg("mip have complete");

      v3d_msg("segment");
      unsigned char *image_binary=0;
      try{image_binary=new unsigned char [size_image];}
      catch(...) {v3d_msg("cannot allocate memory for imag"
                          "e_binary."); return 0;}
      thres_segment(nx*ny*nz,image_mip,image_binary,thres);
      v3d_msg("segment have complete");

      unsigned char *old_binary_image=0;
      try{old_binary_image=new unsigned char [size_image];}
      catch(...) {v3d_msg("cannot allocate memory for imag"
                          "e_binary."); return 0;}
      for(V3DLONG i=0;i<size_image;i++)
      {
          old_binary_image[i]=image_binary[i];
      }



      //int flag=1;
      //V3DLONG num_point=0;
      //V3DLONG num=0;

      LandmarkList curlist;
      LocationSimple s;
      while(true)
      {
          for(V3DLONG j=1;j<ny;j++)
          {
              for(V3DLONG i=1;i<nx;i++)
              {
                  if(image_binary[j*nx+i]>0)
                  {
                      //num=num+1;
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


          for(V3DLONG j=1;j<ny;j++)
          {
              for(V3DLONG i=1;i<nx;i++)
              {
                  if(image_binary[j*nx+i]>0)
                  {
                      //num=num+1;
                      //v3d_msg(QString(" x is %1,y is %2").arg(i).arg(j));
                      //v3d_msg(QString(" location is %1,location is %2,location is %3").arg(j*nx+i).arg((j+1)*nx+i).arg((j-1)*nx+i));
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

      v3d_msg("skeletonization have complete");
      int new_neighbor[8];
      int size_point;
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
                  //v3d_msg(QString("cnp is %1").arg(cnp));
                  if(cnp==3)
                  {
                    //int flag=rayinten_2D(j,i,128 ,8,ray_x, ray_y, image_mip,nx,ny);
                    //if(flag==1)
                    //{
                        s.x=i;
                        s.y=j;
                        s.z=1;
                        s.radius=1;
                        s.color = random_rgba8(255);
                        curlist << s;
                        size_point++;

                    //}



                   }
            }

          }
      }


      Image4DSimple * new4DImage = new Image4DSimple();
      new4DImage->setData((unsigned char *)old_binary_image, p4DImage->getXDim(), p4DImage->getYDim(), 1, p4DImage->getCDim(), p4DImage->getDatatype());
      callback.setImage(curwin, new4DImage);
      callback.setImageName(curwin, "maximum intensity projection image");
      callback.updateImageWindow(curwin);
      callback.setLandmark(curwin, curlist);
      return 1;


}








