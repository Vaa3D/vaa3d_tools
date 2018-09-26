/* Branch_detection_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-9-25 : by Chao Wang
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

Q_EXPORT_PLUGIN2(Branch_detection, TestPlugin)
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
        <<tr("3D_Branch_points_detection")
        <<tr("3D_Branch_points_detection_single_plane");
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
    if (menu_name == tr("3D_Branch_points_detection"))
    {
       int flag=Branch_Point_Detection(callback,parent);
       if (flag==1)
       {
          v3d_msg(tr("branch points detection completed!"));
       }
    }
    else if(menu_name==tr("3D_Branch_points_detection_single_plane"))
    {
        int flag=Branch_Point_Detection_single_plane(callback,parent);
        if (flag==1)
        {
           v3d_msg(tr("branch points detection completed!"));
        }
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
int Branch_Point_Detection(V3DPluginCallback2 &callback, QWidget *parent)
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
            int radiu_block=16;


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


        QSpinBox * radiu_block_spinbox = new QSpinBox();
        radiu_block_spinbox->setRange(1,255);
        radiu_block_spinbox->setValue(radiu_block);

        layout->addWidget(new QLabel("ray numbers"),0,0);
        layout->addWidget(ray_numbers_2d_spinbox, 0,1,1,5);

        layout->addWidget(new QLabel("intensity threshold"),1,0);
        layout->addWidget(thres_2d_spinbox, 1,1,1,5);

        layout->addWidget(new QLabel("ray length"),2,0);
        layout->addWidget(ray_length_2d_spinbox, 2,1,1,5);

        layout->addWidget(new QLabel("size of block"),3,0);
        layout->addWidget(radiu_block_spinbox, 3,1,1,5);


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
        radiu_block=radiu_block_spinbox->value();

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

       LandmarkList curlist;
       LocationSimple s;
       int length_block=2*radiu_block+1;
       int size_plane=length_block*length_block;
       int size_block=size_plane*length_block;


       unsigned char *block;
       try{block=new unsigned char [size_block];}
       catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

       unsigned char *xy_project;
       try{xy_project=new unsigned char [size_plane];}
       catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

//       unsigned char *xz_project=0;
//       try{xy_project=new unsigned char [size_plane];}
//       catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

//       unsigned char *yz_project;
//       try{yz_project=new unsigned char [size_plane];}
//       catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

       unsigned char *seg_xy_project;
       try{seg_xy_project=new unsigned char [size_plane];}
       catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

//       unsigned char *seg_xz_project;
//       try{seg_xz_project=new unsigned char [size_plane];}
//       catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

//       unsigned char *seg_yz_project;
//       try{seg_yz_project=new unsigned char [size_plane];}
//       catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

      int num_block=0;
      for(V3DLONG j=radiu_block;j<nz-radiu_block;j++)
      {
           for(V3DLONG i=radiu_block;i<ny-radiu_block;i++)
           {
               for(V3DLONG k=radiu_block;k<nx-radiu_block;k++)
               {
                   unsigned char pixe=p4DImage->getValueUINT8(k,i,j,0);
                   if(pixe>thres_2d)
                   {
                       v3d_msg(QString("pixe is %1").arg(pixe));
                       for(V3DLONG a=j-radiu_block;a<=j+radiu_block;a++)
                       {
                            V3DLONG z_location=a*nx*ny;
                            for(V3DLONG b=i-radiu_block;b<=i+radiu_block;b++)
                            {
                                V3DLONG y_location=b*nx;
                                for(V3DLONG c=k-radiu_block;c<=k+radiu_block;c++)
                                {

                                      block[num_block]=datald[z_location+y_location+c];
                                      num_block++;

                                }
                            }
                       }
                       v3d_msg(QString("create block success"));

//                       XZ_mip(length_block,length_block,length_block,block,xz_project);
//                       v3d_msg(QString("create xz_mip success"));

                       XY_mip(length_block,length_block,length_block,block,xy_project);
                       v3d_msg(QString("create xy_mip success"));

//                       YZ_mip(length_block,length_block,length_block,block,yz_project);
//                       v3d_msg(QString("create yz_mip success"));

                       thres_segment(size_plane,xy_project,seg_xy_project,thres_2d);
//                       thres_segment(size_plane,xz_project,seg_xz_project,thres_2d);
//                       thres_segment(size_plane,yz_project,seg_yz_project,thres_2d);
                       v3d_msg(QString("segment mip success"));

                       int flag1=rayinten_2D(j,i,ray_numbers_2d ,ray_length_2d,ray_x, ray_y, seg_xy_project,2*radiu_block,2*radiu_block);
//                       int flag2=rayinten_2D(j,i,ray_numbers_2d ,ray_length_2d,ray_x, ray_y, seg_xz_project,2*radiu_block,2*radiu_block);
//                       int flag3=rayinten_2D(j,i,ray_numbers_2d ,ray_length_2d,ray_x, ray_y, seg_yz_project,2*radiu_block,2*radiu_block);

//                       int sum_flag=flag1+flag2+flag3;
                       int sum_flag=flag1;
                       v3d_msg(QString("segment mip success").arg(sum_flag));
                       if(sum_flag>=1)
                       {
                           s.x=k;
                           s.y=i;
                           s.z=j;
                           s.radius=1;
                           s.color=random_rgba8(255);
                           curlist<<s;
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
        if(block) {delete []block; block = 0;}
        if(xy_project) {delete []xy_project; xy_project = 0;}
//        if(xz_project) {delete []xz_project; xz_project = 0;}
//        if(yz_project) {delete []yz_project; yz_project = 0;}
        if(seg_xy_project) {delete []seg_xy_project; seg_xy_project = 0;}
//        if(seg_xz_project) {delete []seg_xz_project; seg_xz_project = 0;}
//        if(seg_yz_project) {delete []seg_yz_project; seg_yz_project = 0;}
        return 1;
}

int Branch_Point_Detection_single_plane(V3DPluginCallback2 &callback, QWidget *parent)
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
            int radiu_block=16;


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


        QSpinBox * radiu_block_spinbox = new QSpinBox();
        radiu_block_spinbox->setRange(1,255);
        radiu_block_spinbox->setValue(radiu_block);

        layout->addWidget(new QLabel("ray numbers"),0,0);
        layout->addWidget(ray_numbers_2d_spinbox, 0,1,1,5);

        layout->addWidget(new QLabel("intensity threshold"),1,0);
        layout->addWidget(thres_2d_spinbox, 1,1,1,5);

        layout->addWidget(new QLabel("ray length"),2,0);
        layout->addWidget(ray_length_2d_spinbox, 2,1,1,5);

        layout->addWidget(new QLabel("size of block"),3,0);
        layout->addWidget(radiu_block_spinbox, 3,1,1,5);


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
        radiu_block=radiu_block_spinbox->value();

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

        LandmarkList curlist;
        LocationSimple s;
        int length_block=2*radiu_block+1;
        int size_plane=length_block*length_block;
        int size_block=size_plane*length_block;


//        unsigned char *block;
//        try{block=new unsigned char [size_block];}
//        catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

//        unsigned char *xy_project;
//        try{xy_project=new unsigned char [size_block];}
//        catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

//        unsigned char *seg_xy_project;
//        try{seg_xy_project=new unsigned char [size_block];}
//        catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

        V3DLONG num_block=0;
        for(V3DLONG j=radiu_block;j<nz-radiu_block;j++)
        {
           for(V3DLONG i=radiu_block;i<ny-radiu_block;i++)
           {
               for(V3DLONG k=radiu_block;k<nx-radiu_block;k++)
               {
                   unsigned char pixe=p4DImage->getValueUINT8(k,i,j,0);
                   if(pixe>thres_2d)
                   {
                       v3d_msg(QString("location is %1").arg(j*nx*ny+i*nx+k));
                       v3d_msg(QString("pixe is %1").arg(pixe));
                       unsigned char *block;
                       try{block=new unsigned char [size_block];}
                       catch(...) {v3d_msg("cannot allocate memory for image_mip7."); return 0;}

                       unsigned char *xy_project;
                       try{xy_project=new unsigned char [size_block];}
                       catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

                       unsigned char *seg_xy_project;
                       try{seg_xy_project=new unsigned char [size_block];}
                       catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

                       for(V3DLONG a=j-radiu_block;a<=j+radiu_block;a++)
                       {
                            V3DLONG z_location=a*nx*ny;
                            for(V3DLONG b=i-radiu_block;b<=i+radiu_block;b++)
                            {
                                V3DLONG y_location=b*nx;
                                for(V3DLONG c=k-radiu_block;c<=k+radiu_block;c++)
                                {

                                      block[num_block]=datald[z_location+y_location+c];
                                      num_block++;
                                      v3d_msg(QString("location1 is %1").arg(z_location+y_location+c));


                                }
                            }
                       }
                       v3d_msg(QString("create block success"));


                       XY_mip(length_block,length_block,length_block,block,xy_project);
                       v3d_msg(QString("create xy_mip success"));

                       thres_segment(size_plane,xy_project,seg_xy_project,thres_2d);
                       v3d_msg(QString("segment mip success"));

                       int flag1=rayinten_2D(j,i,ray_numbers_2d ,ray_length_2d,ray_x, ray_y, seg_xy_project,2*radiu_block,2*radiu_block);
                       int sum_flag=flag1;
                       v3d_msg(QString("flag is %1").arg(sum_flag));
                       if(sum_flag>=1)
                       {
                           s.x=k;
                           s.y=i;
                           s.z=j;
                           s.radius=1;
                           s.color=random_rgba8(255);
                           curlist<<s;
                       }
//                       if(block) {delete []block; block = 0;}
//                       if(xy_project) {delete []xy_project; xy_project = 0;}
//                       if(seg_xy_project) {delete []seg_xy_project; seg_xy_project = 0;}
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
//        if(block) {delete []block; block = 0;}
//        if(xy_project) {delete []xy_project; xy_project = 0;}
//        if(seg_xy_project) {delete []seg_xy_project; seg_xy_project = 0;}
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

