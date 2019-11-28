/* Branch_detection_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-5-20 : by Chao Wang
 */
#include"Branch_detection_plugin.h"
#include <iostream>
#include <fstream>
#include "basic_surf_objs.h"
#include "v3d_message.h"
#include <vector>
#include "new_ray-shooting.h"
#include "../../../../vaa3d_tools/released_plugins/v3d_plugins/bigneuron_siqi_rivuletv3d/fastmarching/fastmarching_dt.h"
#include"../../../../vaa3d_tools/released_plugins/v3d_plugins/neurontracing_vn2/neuron_radius/marker_radius.h"
#define PI 3.1415926
#define based_num 1;
#include <algorithm>
#include"ClusterAnalysis.h"
#include"branch_detection_model.h"

template <class T> void gaussian_filter(T* data1d,
                                        V3DLONG nx, V3DLONG ny,V3DLONG nz,
                                        unsigned int Wx,
                                        unsigned int Wy,
                                        unsigned int Wz,
                                        unsigned int c,
                                        double sigma,
                                        int thres, T* &out_data1d);

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
        <<tr("display ray shooting model")
        <<tr("2D junction detection based on ray-shoooting model ")
        <<tr("display_length")
        <<tr("create groundtruth")
        <<tr("2D rotate algorithm display")
        <<tr("3D rotate algorithm display")
        <<tr("2D juntion detection")
        <<tr("rotate 3D image")
        <<tr("rotate virtual finger by 3 projection")
        <<tr("local radius estimation")
        <<tr("conner detection")
        <<tr("Base_distance for ray_scan")
        <<tr("classification_for_ray_shooting")
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

         if(menu_name==tr("display ray shooting model"))
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
        else if(menu_name==tr("2D junction detection based on ray-shoooting model "))
         {
             int flag=ray_shooting_model(callback,parent);
             if(flag==1)
                 v3d_msg("2D junction point have been detected ");
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
        else if(menu_name==tr("2D juntion detection"))
        {
            int p;
            if(QMessageBox::Yes == QMessageBox::question (0, "", QString("yes: neuron; no : vessel blood"), QMessageBox::Yes, QMessageBox::No))    p = 1;
            if(p==1)
            {
                int flag=ray_scan_model(callback,parent);
                if(flag==1)
                    v3d_msg("2D branch points completed ");
            }
            else{
                int flag=ray_scan_model_vessel(callback,parent);
                if(flag==1)
                    v3d_msg("2D branch points completed ");
            }
        }
        else if(menu_name==tr("rotate 3D image"))
        {
            int flag=rotate_3D_image(callback,parent);
            if(flag==1)
                v3d_msg("3D branch points completed ");
        }
        else if(menu_name==tr("rotate virtual finger by 3 projection"))
        {
            int flag=rotate_vf_3projection(callback,parent);
            if(flag==1)
                v3d_msg("3D branch points completed ");
        }
        else if(menu_name==tr("local radius estimation"))
        {
            int flag=radius_estimation(callback,parent);
            if(flag==1)
                v3d_msg("OK ");
        }
        else if(menu_name==tr("Base_distance for ray_scan"))
        {
         int flag=Base_distance_for_ray_scan(callback,parent);
         if(flag==1)
             v3d_msg("OK ");
        }
         else if(menu_name==tr("classification_for_ray_shooting"))
         {
          int flag=classification_for_ray_shooting(callback,parent);
          if(flag==1)
              v3d_msg("OK ");
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

void printHelp()
{
    cout<<"\nThis is a demo plugin to branch point in an image. by Chao Wang  2018-04"<<endl;
    cout<<"if you have any question please contact hunchaowang@hnu.edu.cn"<<endl;
    return;
}

int display_rayshooting_model_in_xyproject(V3DPluginCallback2 &callback, QWidget *parent)
{

    v3dhandle curwin = callback.currentImageWindow();
    if(!curwin)
    {
            v3d_msg("No image is open.");
            return -1;
    }
    Image4DSimple *p4DImage = callback.getImage(curwin);

    int x_coordinate=100, y_coordinate=100,thres_2d=45,ray_numbers_2d=128;
    int ray_length=8, block_size=10;
    int delete_flag=1;

    V3DLONG sz[3];
    sz[0] = p4DImage->getXDim();
    sz[1] = p4DImage->getYDim();
    sz[2] = p4DImage->getZDim();
    sz[3] = p4DImage->getCDim();
    V3DLONG size_image=sz[0]*sz[1];

    V3DLONG nx=sz[0],ny=sz[1],nz=sz[2];
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

        QSpinBox * x_location_spinbox = new QSpinBox();
        x_location_spinbox->setRange(1,10000);
        x_location_spinbox->setValue(x_coordinate);

        QSpinBox * y_location_spinbox = new QSpinBox();
        y_location_spinbox->setRange(1,10000);
        y_location_spinbox->setValue(y_coordinate);


        QSpinBox * based_distance_spinbox = new QSpinBox();
        based_distance_spinbox->setRange(1,100);
        based_distance_spinbox->setValue(ray_length);

        QSpinBox * block_size_spinbox = new QSpinBox();
        block_size_spinbox->setRange(1,100);
        block_size_spinbox->setValue(block_size);

        QSpinBox * delete_flag_spinbox = new QSpinBox();
        delete_flag_spinbox->setRange(0,1);
        delete_flag_spinbox->setValue(delete_flag);

        layout->addWidget(new QLabel("ray numbers"),0,0);
        layout->addWidget(ray_numbers_2d_spinbox, 0,1,1,5);

        layout->addWidget(new QLabel("intensity threshold"),1,0);
        layout->addWidget(thres_2d_spinbox, 1,1,1,5);

        layout->addWidget(new QLabel("x location"),2,0);
        layout->addWidget(x_location_spinbox, 2,1,1,5);

        layout->addWidget(new QLabel("y location"),3,0);
        layout->addWidget(y_location_spinbox, 3,1,1,5);

        layout->addWidget(new QLabel("ray length "),4,0);
        layout->addWidget(based_distance_spinbox, 4,1,1,4);

        layout->addWidget(new QLabel("block size "),5,0);
        layout->addWidget(block_size_spinbox, 5,1,1,5);

        layout->addWidget(new QLabel("want to delete the small area "),6,0);
        layout->addWidget(delete_flag_spinbox, 6,1,1,6);

        QHBoxLayout * hbox2 = new QHBoxLayout();
        QPushButton * ok = new QPushButton(" ok ");
        ok->setDefault(true);
        QPushButton * cancel = new QPushButton("cancel");
        hbox2->addWidget(cancel);
        hbox2->addWidget(ok);

        layout->addLayout(hbox2,8,0,1,8);
        dialog->setLayout(layout);
        QObject::connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
        QObject::connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));

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
        ray_length=based_distance_spinbox->value();
        block_size=block_size_spinbox->value();
        delete_flag=delete_flag_spinbox->value();

        if (dialog)
            {
                    delete dialog;
                    dialog=0;
                    cout<<"delete dialog"<<endl;
            }

        unsigned char* datald=0;
        datald=p4DImage->getRawData();

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
         find_neighborhood_maximum_radius(x_coordinate,y_coordinate,seg_datald_mip,2,adjusted_x,adjusted_y,radiu,nx,ny,thres_2d);
         v3d_msg(QString("the estmated radius is %1").arg(radiu));
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
         int block_radiu=radiu+ray_length+block_size;
         int block_length=block_radiu*2+1;
         int num_block=0;
          unsigned char *block=new unsigned char[block_length*block_length];
          for(V3DLONG b=adjusted_y-block_radiu;b<=adjusted_y+block_radiu;b++)
          {
              for(V3DLONG c=adjusted_x-block_radiu;c<=adjusted_x+block_radiu;c++)
              {
                 // unsigned char block_pixe=seg_datald_mip[b*nx+c];
                  unsigned char block_pixe=seg_datald_mip[b*nx+c];
                    block[num_block]=block_pixe;
                    num_block++;
              }
          }
          if(delete_flag)
          {
              delete_small_area(block_length,block_length,block);// connected domain denoising
          }



          double ray_pxie_sum;
          int max_value;
                    for(int i = 0; i <ray_numbers_2d; i++)   //i is the numble of the ray
                    {
                        ray_pxie_sum=0;
                        max_value=0;
                        for(int j = 0; j < ray_length; j++)    // j is the numble of circle
                        {
                            double pixe = project_interp_2d(block_radiu+ray_y[i][j], block_radiu+ray_x[i][j], block, block_length,block_length , (block_radiu), (block_radiu));
                            ray_pxie_sum=ray_pxie_sum+pixe;
                            if(pixe>max_value)
                            {
                                max_value=pixe;
                            }
                        }
                        if((ray_pxie_sum/ray_length)>(max_value*0.5))
                        {
                            for(int j = 0; j < ray_length; j++)
                            {
                                double new_pixe=200;
                               x_loc.push_back(block_radiu+ray_x[i][j]);
                               y_loc.push_back(block_radiu+ray_y[i][j]);
                               Pixe.push_back(new_pixe);

                            }
                        }
                        else
                        {
                            for(int j =0; j < ray_length; j++)
                            {
                                double new_pixe=100;
                                x_loc.push_back(block_radiu+ray_x[i][j]);
                                y_loc.push_back(block_radiu+ray_y[i][j]);
                                Pixe.push_back(new_pixe);
                            }
                        }

                    }

          RGBA8 red; red.r=255; red.g=0; red.b=0;
          RGBA8 green; green.r=0; green.g=255; green.b=0;
          RGBA8 blue; blue.r=0; blue.g=0; blue.b=255;

          LandmarkList curlist;
          LocationSimple s;

          for(V3DLONG i = 0; i <x_loc.size(); i++)
          {
              s.x=  x_loc[i]+1;
              s.y = y_loc[i]+1;
              s.z = 1;
              s.radius = 1;
//              s.color=red;
              cout<<Pixe.at(i)<<endl;
              if(Pixe.at(i)<150)
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

int ray_shooting_model(V3DPluginCallback2 &callback, QWidget *parent)
{
    ClusterAnalysis mycluster;
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
    sz[3] = p4DImage->getCDim();
    V3DLONG size_image=sz[0]*sz[1];


    // set the parameter
    int thres_2d=30;
    int ray_numbers_2d=128;
    int cluster_threshold=100;
    V3DLONG nx=sz[0];
    V3DLONG ny=sz[1];
    V3DLONG nz=sz[2];

    if(p4DImage==NULL)
    {
       v3d_msg("No image ");
    }

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

          QSpinBox * cluster_threshold_spinbox = new QSpinBox();
          cluster_threshold_spinbox->setRange(0,255);
          cluster_threshold_spinbox->setValue(cluster_threshold);


          layout->addWidget(new QLabel("ray angle"),0,0);
          layout->addWidget(ray_numbers_2d_spinbox, 0,1,1,5);

          layout->addWidget(new QLabel("intensity threshold"),1,0);
          layout->addWidget(thres_2d_spinbox, 1,1,1,5);

          layout->addWidget(new QLabel("cluster threshold"),4,0);
          layout->addWidget(cluster_threshold_spinbox, 4,1,1,5);


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
          cout<<sz[0]<<" "<<sz[1]<<" "<<sz[2]<<endl;

          //get the dialog return values
          ray_numbers_2d = ray_numbers_2d_spinbox->value();
          thres_2d = thres_2d_spinbox->value();
          cluster_threshold=cluster_threshold_spinbox->value();

          if (dialog)
          {
                  delete dialog;
                  dialog=0;
                  cout<<"delete dialog"<<endl;
          }



        unsigned char* datald=0;
        datald=p4DImage->getRawData();

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
        qint64 etime1;
        QElapsedTimer timer1;
        timer1.start();
        skeletonization(nx,ny,image_binary);
        seek_2D_candidate_points(nx,ny,image_binary,X_loc,Y_loc,count);
        int window_size=1;
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
            }
        }
        else {
            cout<<"no candidate points in this MIP"<<endl;
        }
        v3d_msg(QString("the all candidate points have detected"));
        /* create the ratate matrix */
        vector<vector<float> > ray_x(ray_numbers_2d,vector<float>(1000)), ray_y(ray_numbers_2d,vector<float>(1000));
        float ang = 2*PI/ray_numbers_2d;
        float x_dis, y_dis;
        int max_length=200; // if the rorate crash , please check the max_length;
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
            /* using the ray sacn to detect 2D junction points designed by chaowang 2019/1/15 */

                        double ray_pxie_sum;
                        int max_value;
                        vector<double> ind_sum;
                        vector<double> ind_max;
                        vector<int> ind_foregroud_ray;
                        int num=0;
                        int branch_flag=1;
                        for(int i = 0; i <ray_numbers_2d; i++)   //i is the numble of the ray
                        {
                            ray_pxie_sum=0;
                            max_value=0;
                            for(int j = 0; j < 5*int(Max_value.at(k)); j++)    // j is the numble of circle
                            {
                                if(((X_candidate.at(k)+ 5*int(Max_value.at(k)))>nx)||((Y_candidate.at(k)+ 5*int(Max_value.at(k)))>ny))
                                {
                                    continue;
                                }
                                double pixe = project_interp_2d(Y_candidate.at(k)+ray_y[i][j], X_candidate.at(k)+ray_x[i][j], old_image_binary, nx, ny , Y_candidate.at(k),  X_candidate.at(k));
                                ray_pxie_sum=ray_pxie_sum+pixe;
                                if(pixe>max_value)
                                {
                                    max_value=pixe;
                                }
                            }
                            ind_max.push_back(max_value);
                            ind_sum.push_back(ray_pxie_sum);
                        }

                        for(int s=0;s<ray_numbers_2d;s++)
                        {
                            if(ind_sum.at(s)/(5*int(Max_value.at(k)))>ind_max.at(s)*0.5)
                            {
                                num++;
                                ind_foregroud_ray.push_back(s);
                                //cout<<"the index of foreground is: "<<s<<endl;
                            }
                        }
                        vector<float> x_forgroundnode;
                        vector<float> y_forgroundnode;

                        if(num>1)
                        {
                            for(int i=0;i<num;i++)
                            {
                                float x_loc=X_candidate.at(k)+ray_x[ind_foregroud_ray.at(i)][5*int(Max_value.at(k))-1];
                                float y_loc=Y_candidate.at(k)+ray_y[ind_foregroud_ray.at(i)][5*int(Max_value.at(k))-1];
                                x_forgroundnode.push_back(x_loc);
                                y_forgroundnode.push_back(y_loc);
                            }
                        }
                                   /* calculate the DBscan radiu */
                                    float change_x1= abs(ray_x[2][5*int(Max_value.at(k))-1]-ray_x[3][5*int(Max_value.at(k))-1]);
                                    float change_x=pow(change_x1,2);
                                    float change_y1=abs(Y_candidate.at(k)+ray_y[2][5*int(Max_value.at(k))-1]-Y_candidate.at(k)+ray_y[3][5*int(Max_value.at(k))-1]);;
                                    float change_y=pow(change_y1,2);
                                    float DB_radius=sqrt(change_x+change_y)+1;
                                    mycluster.Read_from_coordiante(x_forgroundnode,y_forgroundnode,DB_radius,1);
                                    int flag=mycluster.DoDBSCANRecursive();



//                        if (num > 1)
//                        {
//                            for (int i = 0;i < num - 2;i++)
//                            {
//                               int ray_distance=abs(ind_foregroud_ray[i]-ind_foregroud_ray[i+1]);
//                               if(ray_distance>=2)
//                               {
//                                   branch_flag=branch_flag+1;
//                               }
//                            }

//                            long dis=ind_foregroud_ray[0]-ind_foregroud_ray[num];

//                            if(abs(dis)==(127))
//                            {
//                              branch_flag=branch_flag-1;
//                            }

//                        }




            if(flag==3)
            {
                             s.x=  X_candidate[k]+1;
                             s.y = Y_candidate[k]+1;
                             s.z = 1;
                             s.radius = 1;
                             s.color = red;
                             curlist<<s;
            }
//             if(flag==4)
//             {
//                              s.x=  X_candidate[k]+1;
//                              s.y = Y_candidate[k]+1;
//                              s.z = 1;
//                              s.radius = 1;
//                              s.color = blue;
//                              curlist<<s;
//             }

        }
//         LandmarkList new_curlist;
//         ClusterAnalysis cluster;
//         cluster.Read_from_curlist(curlist,5,2);
//         cluster.DoDBSCANRecursive();
//         new_curlist = cluster.get_clustered_curlist(curlist);
        etime1 = timer1.elapsed();
        v3d_msg(QString("the detecting take %1 milliseconds").arg(etime1));

        std::vector<double>::iterator biggest=std::max_element(std::begin(Max_value),std::end(Max_value));
        int new_location=std::distance(std::begin(Max_value),biggest);
        double new_max_radiu=*biggest;
        double soma_x=X_loc.at(new_location);
        double soma_y=Y_loc.at(new_location);
        double max_radiu=new_max_radiu+20;
        cout<<"the soma x is :"<<soma_x<<" "<<"the soma y is :"<<soma_y<<" "<<"the radiu is :"<<max_radiu<<endl;

        // delete the soma neiborhood branch points
        for(int i=0;i<curlist.size();i++)
        {
            if((pow(curlist.at(i).x-soma_x,2)+pow(curlist.at(i).y-soma_y,2))<pow(max_radiu,2))
            {
                curlist.removeAt(i);
                i=i-1;
            }
        }




        bool flag_while_xy = true;
        while(flag_while_xy)
        {
            bool endwhile = true;
            for(V3DLONG i=0;i<curlist.size();i++)
            {
                for(V3DLONG j=i+1;j<curlist.size();j++)
                {
                    if((square(curlist[j].x-curlist[i].x)+square(curlist[j].y-curlist[i].y))<cluster_threshold)
                    {
                        curlist[i].x=(curlist[j].x+curlist[i].x)/2;
                        curlist[i].y=(curlist[j].y+curlist[i].y)/2;
                        curlist[i].z=(curlist[j].z+curlist[i].z)/2;
                        curlist[i].color.b+=51;
                        curlist.removeAt(j);
                        j=j-1;
                        endwhile = false;
                    }
                }
            }
            if(endwhile){flag_while_xy = false;}
        }
        //if(image_binary) {delete []image_binary; image_binary = 0;}
        //if(datald_mip) {delete []datald_mip; datald_mip = 0;}

        Image4DSimple * new4DImage = new Image4DSimple();
        new4DImage->setData((unsigned char *)old_image_binary, nx, ny, 1, p4DImage->getCDim(), p4DImage->getDatatype());
        v3dhandle newwin = callback.newImageWindow();
        callback.setImage(newwin, new4DImage);
        callback.updateImageWindow(newwin);
        callback.setLandmark(newwin, curlist);
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
         vector<int> indd,ind1,temp_gap;//ind1 is the all piex of each ray
         float max_indd=0.0;
         int flag=0;
         int count=0;
         vector<float> x_loc,temp_pixel;
         vector<float> y_loc;
         int gap_pixe;
         v3d_msg(QString("x is %1, y is %2, The pixe of source point is %3").arg(x_location-1).arg(y_location-1).arg(datald_mip[(y_location-1)*nx+x_location-1]));
         vector<V3DLONG> adaptive_ray_length;
         int gap_length;
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

                 if(gap_pixe==3)
                 //if(gap_pixe>=10)
                 {
                     adaptive_ray_length.push_back(j+1);//cout<<j+1<<endl;//the length of each ray
//                     break;
                 }

             }
             temp_gap.push_back(gap_pixe);
             if(i==0)
             {
                  gap_length=gap_pixe;
             }
             else
             {
                 if(temp_gap.at(i-1)>=gap_pixe)
                 {
                     gap_length=gap_pixe;
                 }
                 else
                 {
                     gap_length=temp_gap.at(i-1);
                 }

             }

             ind1.push_back(sum);
             //v3d_msg(QString("sum of pixe is %1").arg(sum));
         }
         cout<<"gap_length:" <<gap_length<<endl;
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
    v3dhandle curwin = callback.currentImageWindow();
    if(!curwin)
    {
            v3d_msg("No image is open.");
            return -1;
    }
    Image4DSimple *p4DImage = callback.getImage(curwin);

    int x_coordinate=100, y_coordinate=100,thres_2d=45,ray_numbers_2d=128, ray_length=3;
    int T0=3,window_size=1, block_size=10;
    int minpts=3;


    V3DLONG sz[3];
    sz[0] = p4DImage->getXDim();
    sz[1] = p4DImage->getYDim();
    sz[2] = p4DImage->getZDim();
    sz[3] = p4DImage->getCDim();
    V3DLONG size_image=sz[0]*sz[1];


    V3DLONG nx=sz[0],ny=sz[1],nz=sz[2];


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

        QSpinBox * x_location_spinbox = new QSpinBox();
        x_location_spinbox->setRange(1,10000);
        x_location_spinbox->setValue(x_coordinate);

        QSpinBox * y_location_spinbox = new QSpinBox();
        y_location_spinbox->setRange(1,10000);
        y_location_spinbox->setValue(y_coordinate);


        QSpinBox * based_distance_spinbox = new QSpinBox();
        based_distance_spinbox->setRange(0,100);
        based_distance_spinbox->setValue(T0);

        QSpinBox * block_size_spinbox = new QSpinBox();
        block_size_spinbox->setRange(1,100);
        block_size_spinbox->setValue(block_size);

        QSpinBox * ray_length_spinbox = new QSpinBox();
        ray_length_spinbox->setRange(1,20);
        ray_length_spinbox->setValue(ray_length);

        QSpinBox * minpts_spinbox = new QSpinBox();
        minpts_spinbox->setRange(1,20);
        minpts_spinbox->setValue(minpts);


        layout->addWidget(new QLabel("ray numbers"),0,0);
        layout->addWidget(ray_numbers_2d_spinbox, 0,1,1,5);

        layout->addWidget(new QLabel("intensity threshold"),1,0);
        layout->addWidget(thres_2d_spinbox, 1,1,1,5);

        layout->addWidget(new QLabel("x location"),2,0);
        layout->addWidget(x_location_spinbox, 2,1,1,5);

        layout->addWidget(new QLabel("y location"),3,0);
        layout->addWidget(y_location_spinbox, 3,1,1,5);

        layout->addWidget(new QLabel(" T0 "),4,0);
        layout->addWidget(based_distance_spinbox, 4,1,1,5);

        layout->addWidget(new QLabel("block size "),5,0);
        layout->addWidget(block_size_spinbox, 5,1,1,5);

        layout->addWidget(new QLabel("ray length "),6,0);
        layout->addWidget(ray_length_spinbox, 6,1,1,5);

        layout->addWidget(new QLabel(" minpts "),7,0);
        layout->addWidget(minpts_spinbox, 7,1,1,5);




        QHBoxLayout * hbox2 = new QHBoxLayout();
        QPushButton * ok = new QPushButton(" ok ");
        ok->setDefault(true);
        QPushButton * cancel = new QPushButton("cancel");
        hbox2->addWidget(cancel);
        hbox2->addWidget(ok);

        layout->addLayout(hbox2,8,0,1,8);
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
        T0=based_distance_spinbox->value();
        block_size=block_size_spinbox->value();
        ray_length=ray_length_spinbox->value();
        minpts=minpts_spinbox->value();


        if (dialog)
            {
                    delete dialog;
                    dialog=0;
                    cout<<"delete dialog"<<endl;
            }
        unsigned char* datald=0;
        datald=p4DImage->getRawData();

         unsigned char *datald_mip;
         try{datald_mip=new unsigned char [size_image];}
         catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

         unsigned char *seg_datald_mip;
         try{seg_datald_mip=new unsigned char [size_image];}
         catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

         Z_mip(nx,ny,nz,datald,datald_mip);
         thres_segment(nx*ny,datald_mip,seg_datald_mip,thres_2d);

         /* using Spherical growth method designed by PHC*/

         V3DLONG adjusted_x, adjusted_y;
         double radiu;

//         measure_one_point_radius_by_iterative_spherical_growth(seg_datald_mip,nx,ny,thres_2d,x_coordinate,y_coordinate,adjusted_x,adjusted_y,radiu);
         radiu=markerRadius_hanchuan_XY(seg_datald_mip,nx,ny,x_coordinate,y_coordinate,thres_2d);
         adjusted_x=x_coordinate;
         adjusted_y=y_coordinate;
//         double radiu;
//         V3DLONG adjusted_x, adjusted_y;
//         find_neighborhood_maximum_radius(x_coordinate,y_coordinate,seg_datald_mip,window_size,adjusted_x,adjusted_y,radiu,nx,ny,thres_2d);
         v3d_msg(QString("the estmated radius is %1").arg(radiu));
//         v3d_msg(QString(" the original x is %1, the original y is %2, the adjusted x is %3, the adjusted y is %4 ").arg(x_coordinate).arg(y_coordinate).arg(adjusted_x).arg(adjusted_y));
         vector<vector<double> > ray_x(ray_numbers_2d,vector<double>(400)), ray_y(ray_numbers_2d,vector<double>(400));
         float ang = 2*PI/ray_numbers_2d;
         float x_dis, y_dis;

         for(int i = 0; i < ray_numbers_2d; i++)
         {
            x_dis = cos(ang*(i+1));
            y_dis = sin(ang*(i+1));
            for(int j = 0; j<400 ; j++)
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
         int block_radiu=radiu+T0+block_size;
         int block_length=block_radiu*2+1;
         int num_block=0;
          unsigned char *block=new unsigned char[block_length*block_length];
          for(V3DLONG b=adjusted_y-block_radiu;b<=adjusted_y+block_radiu;b++)
          {
              for(V3DLONG c=adjusted_x-block_radiu;c<=adjusted_x+block_radiu;c++)
              {
                 // unsigned char block_pixe=seg_datald_mip[b*nx+c];
                    unsigned char block_pixe=seg_datald_mip[b*nx+c];
                    block[num_block]=block_pixe;
                    num_block++;
              }
          }
          if(0)
          {
              delete_small_area(block_length,block_length,block);// connected domain denoising
          }
          //double based_distance= float(T0*0.1)*radiu;
          double based_distance=T0;
          cout<<"the dased distance is "<< based_distance<<endl;

          for(int i = 0; i <ray_numbers_2d; i++)   //i is the numble of the ray
          {
              for(int j = radiu+based_distance; j < radiu+based_distance+ray_length; j++)    // j is the numble of circle
              {
                  cout<<"x is "<< block_radiu+ray_x[i][j]<<"y is :"<<block_radiu+ray_y[i][j]<<endl;
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

          double a=block_radiu+ray_x[0][radiu+based_distance+ray_length]; //x1
          double b=block_radiu+ray_x[1][radiu+based_distance+ray_length]; //x2
          double c=block_radiu+ray_y[0][radiu+based_distance+ray_length]; //y1
          double d=block_radiu+ray_y[1][radiu+based_distance+ray_length]; //y2
          double change_x=abs(a-b);
          double change_x1=pow(change_x,2);
          double change_y=abs(c-d);
          double change_x2=pow(change_y,2);
          double det_x=sqrt(change_x1+change_x2);
          int det_y=1;
          double DB_radius;
          if(det_x>det_y)
          {
              DB_radius=det_x;
          }
          else {
              DB_radius=det_y;
          }


          v3d_msg(QString("the calculated eps is %1").arg(DB_radius));

          mycluster.Read_from_coordiante(x_loc,y_loc,DB_radius*1.1,minpts);
          int flag=mycluster.DoDBSCANRecursive();
          v3d_msg(QString("the cluster is %1").arg(flag));

          RGBA8 orange; orange.r=255; orange.g=0; orange.b=0;
          RGBA8 blue; blue.r=65; blue.g=113; blue.b=156;

          RGBA8 green; green.r=0; green.g=255; green.b=0;


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
                  s.color=green;
              }
              else {
                  s.color =orange ;
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

int ray_scan_model(V3DPluginCallback2 &callback,QWidget *parent)
{

    ClusterAnalysis mycluster;
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
    sz[3] = p4DImage->getCDim();
    V3DLONG size_image=sz[0]*sz[1];


    // set the parameter
    int thres_2d=30;
    int ray_length=2;
    int ray_numbers_2d=128;
    int t0=3;
    int t1=5;
    int cluster_threshold=100;
    int Db_thres=3;
    V3DLONG nx=sz[0];
    V3DLONG ny=sz[1];
    V3DLONG nz=sz[2];

    if(p4DImage==NULL)
    {
       v3d_msg("No image ");
    }

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

          QSpinBox * ray_length_spinbox = new QSpinBox();
          ray_length_spinbox->setRange(-1, 255);
          ray_length_spinbox->setValue(ray_length);

//          QSpinBox * based_distance_spinbox = new QSpinBox();
//          based_distance_spinbox->setRange(1,100);
//          based_distance_spinbox->setValue(T0);

//          QSpinBox * add_based_distance_spinbox = new QSpinBox();
//          add_based_distance_spinbox->setRange(1,100);
//          add_based_distance_spinbox->setValue(T1);

          QSpinBox * cluster_threshold_spinbox = new QSpinBox();
          cluster_threshold_spinbox->setRange(0,nx);
          cluster_threshold_spinbox->setValue(cluster_threshold);

          QSpinBox * Db_thres_spinbox = new QSpinBox();
          Db_thres_spinbox->setRange(0,255);
          Db_thres_spinbox->setValue(Db_thres);

          layout->addWidget(new QLabel("ray angle"),0,0);
          layout->addWidget(ray_numbers_2d_spinbox, 0,1,1,5);

          layout->addWidget(new QLabel("intensity threshold"),1,0);
          layout->addWidget(thres_2d_spinbox, 1,1,1,5);

          layout->addWidget(new QLabel("ray length"),2,0);
          layout->addWidget(ray_length_spinbox, 2,1,1,5);

//          layout->addWidget(new QLabel("based distance for junctions"),3,0);
//          layout->addWidget(based_distance_spinbox, 3,1,1,5);

//          layout->addWidget(new QLabel("based distance for tip"),4,0);
//          layout->addWidget(add_based_distance_spinbox, 4,1,1,5);

          layout->addWidget(new QLabel("cluster threshold"),5,0);
          layout->addWidget(cluster_threshold_spinbox, 5,1,1,5);

          layout->addWidget(new QLabel("Db_thres"),6,0);
          layout->addWidget(Db_thres_spinbox, 6,1,1,5);

          QHBoxLayout * hbox2 = new QHBoxLayout();
          QPushButton * ok = new QPushButton(" ok ");
          ok->setDefault(true);
          QPushButton * cancel = new QPushButton("cancel");
          hbox2->addWidget(cancel);
          hbox2->addWidget(ok);


          layout->addLayout(hbox2,7,0,1,7);
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
          cout<<sz[0]<<" "<<sz[1]<<" "<<sz[2]<<endl;

          //get the dialog return values
          ray_numbers_2d = ray_numbers_2d_spinbox->value();
          thres_2d = thres_2d_spinbox->value();
          ray_length=ray_length_spinbox->value();
          cluster_threshold=cluster_threshold_spinbox->value();
          Db_thres=Db_thres_spinbox->value();

          if (dialog)
          {
                  delete dialog;
                  dialog=0;
                  cout<<"delete dialog"<<endl;
          }



        unsigned char* datald=0;
        datald=p4DImage->getRawData();

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
        vector<int> T0;
        vector<int> T1;
        vector<double> Max_value;

        int count=0; // the number of candidate
        vector<V3DLONG> X_loc; //save the non-adjusted x coordinate;
        vector<V3DLONG> Y_loc; //save the non-adjusted y coordinate;

//        vector<V3DLONG> cross_X_loc; //save the non-adjusted x coordinate;
//        vector<V3DLONG> cross_Y_loc; //save the non-adjusted y coordinate;
        vector<V3DLONG> Tip_flag;

                /* difine the color */
        RGBA8 red; red.r=255; red.g=0; red.b=0;
        RGBA8 green; green.r=0; green.g=255; green.b=0;
        RGBA8 blue; blue.r=0; blue.g=0; blue.b=255;
        RGBA8 purple;purple.r=112;purple.g=48;purple.b=160;

        qint64 etime1;
        QElapsedTimer timer1;
        timer1.start();//caculating the time of testing;
        skeletonization(nx,ny,image_binary);
        cross_seek_2D_candidate_points(nx,ny,image_binary,X_loc,Y_loc,Tip_flag,count);




        /* save candidate points to tex.file*/
//        ofstream out_files;
//        out_files.open("D:\\manual1.txt");

//        if(out_files.fail())
//        {
//            cout<<" cnadidate points file error"<<endl;
//            return false;
//        }
//        int candidate_num=0;
//        for(int i=0;i<X_loc.size();i++)
//        {
//           candidate_num=0;
//           out_files<<X_loc.at(i)<<","<<Y_loc.at(i)<<","<<endl;
//        }
//        out_files.close();

        int window_size=2;
        if(count>0)
        {
            for(int i=0;i<count;i++)
            {
//                    if(((X_loc.at(i)-window_size)<=0)||((Y_loc.at(i)-window_size)<=0)||((Y_loc.at(i)+window_size)>=ny)||((X_loc.at(i)+window_size)>=nx))
//                    {
//                        count=count-1;
//                        continue;
//                    }
                    /* using Spherical growth method designed by Chaowang*/
                    double radius=markerRadius_hanchuan_XY(old_image_binary,nx,ny,X_loc.at(i),Y_loc.at(i),thres_2d);
                     X_candidate.push_back(X_loc.at(i));  // X_loc=X_candidate
                     Y_candidate.push_back(Y_loc.at(i));  // Y_loc=Y_candidate
//                    V3DLONG adjusted_x, adjusted_y;
//                    double radiu;
                     get_the_based_distance_rayscan(X_loc.at(i),Y_loc.at(i),radius,old_image_binary,nx,ny,t0,t1);
                     T0.push_back(t0);  // all adjusted x coordinate of candidate points
                     T1.push_back(t1);
//                    find_neighborhood_maximum_radius(X_loc.at(i),Y_loc.at(i),old_image_binary,window_size,adjusted_x,adjusted_y,radiu,nx,ny,thres_2d);
//                    //measure_one_point_radius_by_iterative_spherical_growth(old_image_binary,nx,ny,thres_2d,X_loc.at(i),Y_loc.at(i),adjusted_x,adjusted_y,radiu);
//                    X_candidate.push_back(X_loc.at(i));  // all adjusted x coordinate of candidate points
//                    Y_candidate.push_back(Y_loc.at(i));  // all adjusted y coordinate of candidate points
                     Max_value.push_back(radius);    // all radiu coordinate of candidate points

            }
        }
        else {
            cout<<"no candidate points in this MIP"<<endl;
        }
        cout<<"the all candidate points have detected"<<endl;

        /* create the rotate matrix */
        vector<vector<float> > ray_x(ray_numbers_2d,vector<float>(1000)), ray_y(ray_numbers_2d,vector<float>(1000));
        float ang = 2*PI/ray_numbers_2d;
        float x_dis, y_dis;
        int max_length=400; // if the rorate crash , please check the max_length;
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


        for(V3DLONG k = 0; k < X_candidate.size(); k++)
        {
//            cout<< "flag is :"<<Tip_flag.at(k)<<"  "<<"x cooridnate is :"<< X_candidate.at(k)<<"  "<<" y coordiante is :"<<Y_candidate.at(k)<<endl;
            vector<float> x_loc;
            vector<float> y_loc;
            int block_size=10;
            //create a new block_mip to detect the 2D branch points
            int block_radiu=Max_value.at(k)+ray_length+block_size;
            int block_length=block_radiu*2+1;
            unsigned char *block=0;
            try{block=new unsigned char [block_length*block_length*block_length];}
            catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}
            //cout<<"x is :"<<X_candidate.at(k)<<" "<<" y is :"<<Y_candidate.at(k)<<" "<<"the length is "<<block_length<<endl;
            if(((X_candidate.at(k)+block_radiu)>(nx-1))||((X_candidate.at(k)-block_radiu)<1)||((Y_candidate.at(k)-block_radiu)<1)||((Y_candidate.at(k)+block_radiu)>(ny-1)))
            {
                cout<<"this points beyong the image"<<endl;
                continue;
            }
            int num_block=0;
            for(V3DLONG b=Y_candidate.at(k)-block_radiu;b<=Y_candidate.at(k)+block_radiu;b++)
            {
                 for(V3DLONG c=X_candidate.at(k)-block_radiu;c<=X_candidate.at(k)+block_radiu;c++)
                 {
                     unsigned char block_pixe=old_image_binary[b*nx+c];  //each candidate point have 4block_radiu^2 blocks.
                     block[num_block]=block_pixe;
                     num_block++;                 //
                 }
             }

            /* connected domain denoising */
            if(1)
            {
                delete_small_area(block_length,block_length,block);// connected domain denoising
            }

            /* ray shooting model for junctions and cross points */
            if((Tip_flag.at(k)==3)||(Tip_flag.at(k)==2))
            {
                for(int i = 0; i <ray_numbers_2d; i++)   //m is the numble of the ray
                {
                    for(int j = Max_value.at(k)+T0.at(k); j < Max_value.at(k)+T0.at(k)+ray_length; j++)    // n is the numble of the points of the each ray
                    {
                        if((Max_value.at(k)+T0.at(k)+ray_length)>max_length)
                            {
                            v3d_msg(QString("the max_length is too small to can not adapt the model, please check the max_length"));
                        }
                        double pixe = project_interp_2d(block_radiu+ray_y[i][j], block_radiu+ray_x[i][j], block, block_length, block_length , block_radiu,  block_radiu);
                        if(pixe>=100)
                        {
                            x_loc.push_back(block_radiu+ray_x[i][j]);
                            y_loc.push_back(block_radiu+ray_y[i][j]);

                        }
                    }
                }
            }
            /* ray shooting model for tip */
            else
            {
                for(int i = 0; i <ray_numbers_2d; i++)   //m is the numble of the ray
                {
                    for(int j = Max_value.at(k)+T1.at(k); j < Max_value.at(k)+T1.at(k)+ray_length; j++)    // n is the numble of the points of the each ray
                    {
                        if((Max_value.at(k)+T1.at(k)+ray_length)>max_length)
                            {
                            v3d_msg(QString("the max_length is too small to can not adapt the model, please check the max_length"));
                        }
                        double pixe = project_interp_2d(block_radiu+ray_y[i][j], block_radiu+ray_x[i][j], block, block_length, block_length , block_radiu,  block_radiu);
                        if(pixe>=100)
                        {
                            x_loc.push_back(block_radiu+ray_x[i][j]);
                            y_loc.push_back(block_radiu+ray_y[i][j]);

                        }
                    }
                }
            }

            double a=block_radiu+ray_x[0][Max_value.at(k)+T0.at(k)+ray_length]; //x1
            double b=block_radiu+ray_x[1][Max_value.at(k)+T0.at(k)+ray_length]; //x2
            double c=block_radiu+ray_y[0][Max_value.at(k)+T0.at(k)+ray_length]; //y1
            double d=block_radiu+ray_y[1][Max_value.at(k)+T0.at(k)+ray_length]; //y2
            double change_x=abs(a-b);
            double change_x1=pow(change_x,2);
            double change_y=abs(c-d);
            double change_x2=pow(change_y,2);
            double det_x=sqrt(change_x1+change_x2);
            int det_y=1;
            double DB_radius;
            if(det_x>det_y)
            {
                DB_radius=det_x;
            }
            else {
                DB_radius=det_y;
            }
           mycluster.Read_from_coordiante(x_loc,y_loc,1.1*DB_radius,Db_thres);
           int flag;
//           cout<<" branch number is :"<<flag<<endl;
           flag=mycluster.DoDBSCANRecursive();
//           cout<<" x coordinate is："<<X_candidate.at(k)<<""<< "y coordinate is :"<<Y_candidate.at(k)<<"value of cluster is :"<<flag<<endl;

//           s.x= X_candidate[k]+1;
//           s.y =Y_candidate[k]+1;
//           s.z = 1;
//           s.radius = 1;
//           s.color = purple;
//           curlist<<s;



           if((flag==3)&&(Tip_flag.at(k)==3))
            {
                             s.x=  X_candidate[k]+1;
                             s.y = Y_candidate[k]+1;
                             s.z = 1;
                             s.radius = 3;
                             s.color = red;
                             curlist<<s;
            }
            else if((flag==1)&&(Tip_flag.at(k)==1))
            {
                s.x=  X_candidate[k]+1;
                s.y = Y_candidate[k]+1;
                s.z = 1;
                s.radius = 1;
                s.color =blue;
                curlist<<s;
            }
            else if(flag==4/*(Tip_flag.at(k)==4)*/)
            {
                s.x=  X_candidate[k]+1;
                s.y =  Y_candidate[k]+1;
                s.z = 1;
                s.radius = 4;
                s.color =green;
                curlist<<s;
            }
            else
            {
                   continue;
            }


        }
        etime1 = timer1.elapsed();
        v3d_msg(QString("the detecting take %1 milliseconds").arg(etime1));

        /* deleting the junction points around the cross-points*/
        for(int i=0;i<curlist.size();i++)
        {
            if ((curlist.at(i).radius)==float(4.0))
            {
                for(int j=0;j<curlist.size();j++)
                {
                    if((curlist.at(j).radius)==float(3.0))
                    {
                        if(((square(curlist[j].x-curlist[i].x)+square(curlist[j].y-curlist[i].y))<cluster_threshold))
                        {
                            curlist.removeAt(j);
                        }
                    }
                }
            }
        }

        std::vector<double>::iterator biggest=std::max_element(std::begin(Max_value),std::end(Max_value));
        int new_location=std::distance(std::begin(Max_value),biggest);
        double new_max_radiu=*biggest;
        double soma_x=X_loc.at(new_location);
        double soma_y=Y_loc.at(new_location);
        double max_radiu=new_max_radiu+20;
//        cout<<"the soma x is :"<<soma_x<<" "<<"the soma y is :"<<soma_y<<" "<<"the radiu is :"<<max_radiu<<endl;

        // delete the soma neiborhood branch points
        for(int i=0;i<curlist.size();i++)
        {
            if((pow(curlist.at(i).x-soma_x,2)+pow(curlist.at(i).y-soma_y,2))<pow(max_radiu,2))
            {
                curlist.removeAt(i);
                i=i-1;
            }
        }




        // using a seletced threshold to clusete the close points
        bool flag_while_xy = true;
        while(flag_while_xy)
        {
            bool endwhile = true;
            for(V3DLONG i=0;i<curlist.size();i++)
            {
                for(V3DLONG j=i+1;j<curlist.size();j++)
                {
                    if((square(curlist[j].x-curlist[i].x)+square(curlist[j].y-curlist[i].y))<cluster_threshold)
                    {
                        curlist[i].x=(curlist[j].x+curlist[i].x)/2;
                        curlist[i].y=(curlist[j].y+curlist[i].y)/2;
                        curlist[i].z=(curlist[j].z+curlist[i].z)/2;
                        //curlist[i].color.b+=51;
                        curlist.removeAt(j);
                        j=j-1;
                        endwhile = false;
                    }
                }
            }
            if(endwhile){flag_while_xy = false;}
        }
        //if(image_binary) {delete []image_binary; image_binary = 0;}
        //if(datald_mip) {delete []datald_mip; datald_mip = 0;}

        Image4DSimple * new4DImage = new Image4DSimple();
        new4DImage->setData((unsigned char *)old_image_binary, nx, ny, 1, p4DImage->getCDim(), p4DImage->getDatatype());
        v3dhandle newwin = callback.newImageWindow();
        callback.setImage(newwin, new4DImage);
        callback.updateImageWindow(newwin);
        callback.setLandmark(newwin, curlist);
        return 1;

}
int ray_scan_model_vessel(V3DPluginCallback2 &callback,QWidget *parent)
{
    ClusterAnalysis mycluster;
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
    sz[3] = p4DImage->getCDim();
    V3DLONG size_image=sz[0]*sz[1];

    // set the parameter
    int thres_2d=30;
    int ray_length=2;
    int ray_numbers_2d=128;
    int T0=3;
    int cluster_threshold=10;
    V3DLONG nx=sz[0];
    V3DLONG ny=sz[1];
    V3DLONG nz=sz[2];

    if(p4DImage==NULL)
    {
       v3d_msg("No image ");
    }

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

          QSpinBox * ray_length_spinbox = new QSpinBox();
          ray_length_spinbox->setRange(1,10000);
          ray_length_spinbox->setValue(ray_length);

          QSpinBox * based_distance_spinbox = new QSpinBox();
          based_distance_spinbox->setRange(1,100);
          based_distance_spinbox->setValue(T0);

          QSpinBox * cluster_threshold_spinbox = new QSpinBox();
          cluster_threshold_spinbox->setRange(0,255);
          cluster_threshold_spinbox->setValue(cluster_threshold);

          layout->addWidget(new QLabel("ray angle"),0,0);
          layout->addWidget(ray_numbers_2d_spinbox, 0,1,1,5);

          layout->addWidget(new QLabel("intensity threshold"),1,0);
          layout->addWidget(thres_2d_spinbox, 1,1,1,5);

          layout->addWidget(new QLabel("ray length"),2,0);
          layout->addWidget(ray_length_spinbox, 2,1,1,5);

          layout->addWidget(new QLabel("T0"),3,0);
          layout->addWidget(based_distance_spinbox, 3,1,1,5);

          layout->addWidget(new QLabel("cluster threshold"),4,0);
          layout->addWidget(cluster_threshold_spinbox, 4,1,1,5);

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
          cout<<sz[0]<<" "<<sz[1]<<" "<<sz[2]<<endl;

          //get the dialog return values
          ray_numbers_2d = ray_numbers_2d_spinbox->value();
          thres_2d = thres_2d_spinbox->value();
          ray_length=ray_length_spinbox->value();
          T0=based_distance_spinbox->value();
          cluster_threshold=cluster_threshold_spinbox->value();

          if (dialog)
          {
                  delete dialog;
                  dialog=0;
                  cout<<"delete dialog"<<endl;
          }



        unsigned char* datald=0;
        datald=p4DImage->getRawData();

        unsigned char *datald_mip;
        try{datald_mip=new unsigned char [size_image];}
        catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

        unsigned char *image_binary;
        try{image_binary=new unsigned char [size_image];}
        catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

        unsigned char *old_image_binary=0;
        try{old_image_binary=new unsigned char [size_image];}
        catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

//        Z_mip(nx,ny,nz,datald,datald_mip);
//        thres_segment(nx*ny,datald_mip,image_binary,thres_2d);

        for(V3DLONG i=0;i<nx*ny;i++)
        {
            old_image_binary[i]=datald[i];
        }

        LandmarkList curlist;
        LocationSimple s;
        vector<V3DLONG> X_candidate;
        vector<V3DLONG> Y_candidate;
        vector<double> Max_value;

        int count=0; // the number of candidate
        vector<V3DLONG> X_loc; //save the non-adjusted x coordinate;
        vector<V3DLONG> Y_loc; //save the non-adjusted y coordinate;
        qint64 etime1;
        QElapsedTimer timer1;
        timer1.start();
        skeletonization(nx,ny,datald);// thinning the segmented images;
        seek_2D_candidate_points(nx,ny,datald,X_loc,Y_loc,count); // 2D  candidate points detection;
        int window_size=1;
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
            }
        }
        else {
            cout<<"no candidate points in this MIP"<<endl;
        }
        cout<<"the all candidate points have detected"<<endl;
        /* create the ray-shooting matrix */
        vector<vector<float> > ray_x(ray_numbers_2d,vector<float>(1000)), ray_y(ray_numbers_2d,vector<float>(1000));
        float ang = 2*PI/ray_numbers_2d;
        float x_dis, y_dis;
        int max_length=200; // if the rorate crash , please check the max_length;
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

            double based_distance=float(T0*0.1)*Max_value.at(k);
            /* using the ray sacn to detect 2D junction points designed by chaowang 2019/1/15 */
            for(int i = 0; i <ray_numbers_2d; i++)   //m is the numble of the ray
            {
                for(int j = Max_value.at(k)+based_distance; j < Max_value.at(k)+based_distance+ray_length; j++)    // n is the numble of the points of the each ray
                {

                    if((Max_value.at(k)+based_distance+ray_length)>max_length)
                    {
                        v3d_msg(QString("the max_length is too small to include the model, please check the max_length"));
                    }
                    double pixe = project_interp_2d(Y_candidate.at(k)+ray_y[i][j],X_candidate.at(k)+ray_x[i][j], old_image_binary, nx, ny , X_candidate.at(k),  Y_candidate.at(k));
                    //cout<<"the x is"<<block_radiu+ray_x[i][j]<<"the y is :"<<block_radiu+ray_y[i][j]<<" "<<"the pixe is :"<<pixe<<endl;
                    if(pixe>=100)
                    {
                        x_loc.push_back(X_candidate.at(k)+ray_x[i][j]);
                        y_loc.push_back(Y_candidate.at(k)+ray_y[i][j]);

                    }
                }
            }

//           /* calculate the DBscan radiu */
//            v3d_msg(QString("the x is %1, the y is %2").arg(X_candidate.at(k)).arg(Y_candidate.at(k)));
            double a=X_candidate.at(k)+ray_x[0][Max_value.at(k)+based_distance+ray_length]; //x1
            double b=X_candidate.at(k)+ray_x[1][Max_value.at(k)+based_distance+ray_length]; //x2
            double c=Y_candidate.at(k)+ray_y[0][Max_value.at(k)+based_distance+ray_length]; //y1
            double d=Y_candidate.at(k)+ray_y[1][Max_value.at(k)+based_distance+ray_length]; //y2
            double change_x=abs(a-b);
            double change_x1=pow(change_x,2);
            double change_y=abs(c-d);
            double change_x2=pow(change_y,2);
            double det_x=sqrt(change_x1+change_x2);

            int det_y=1;
            double DB_radius;
            if(det_x>det_y)
            {
                DB_radius=det_x;
            }
            else {
                DB_radius=det_y;
            }
//            v3d_msg(QString("the x1 is %1, the x2 is %2, the y1 is %3, the y2 is %4").arg(a).arg(b).arg(c).arg(d));
//            v3d_msg(QString("the DB_radius is %1").arg(DB_radius));
            mycluster.Read_from_coordiante(x_loc,y_loc,(1.1*DB_radius),3);
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
            else if(flag==1)
            {
                s.x=  X_candidate[k]+1;
                s.y = Y_candidate[k]+1;
                s.z = 1;
                s.radius = 1;
                s.color = blue;
                curlist<<s;


            }
            else if(flag==4)
            {
                              s.x=  X_candidate[k]+1;
                              s.y = Y_candidate[k]+1;
                              s.z = 1;
                              s.radius = 1;
                              s.color = green;
                              curlist<<s;
            }
            else
            {
                continue;
            }

        }
        etime1 = timer1.elapsed();
        v3d_msg(QString("the detecting take %1 milliseconds").arg(etime1));

        bool flag_while_xy = true;
        while(flag_while_xy)
        {
            bool endwhile = true;
            for(V3DLONG i=0;i<curlist.size();i++)
            {
                for(V3DLONG j=i+1;j<curlist.size();j++)
                {
                    if((square(curlist[j].x-curlist[i].x)+square(curlist[j].y-curlist[i].y))<cluster_threshold)
                    {
                        curlist[i].x=(curlist[j].x+curlist[i].x)/2;
                        curlist[i].y=(curlist[j].y+curlist[i].y)/2;
                        curlist[i].z=(curlist[j].z+curlist[i].z)/2;
                        curlist[i].color.b+=51;
                        curlist.removeAt(j);
                        j=j-1;
                        endwhile = false;
                    }
                }
            }
            if(endwhile){flag_while_xy = false;}
        }
        Image4DSimple * new4DImage = new Image4DSimple();
        new4DImage->setData((unsigned char *)old_image_binary, nx, ny, 1, p4DImage->getCDim(), p4DImage->getDatatype());
        v3dhandle newwin = callback.newImageWindow();
        callback.setImage(newwin, new4DImage);
        callback.updateImageWindow(newwin);
        callback.setLandmark(newwin, curlist);
        return 1;

}
int rotate_3D_image(V3DPluginCallback2 &callback, QWidget *parent)
{
    ClusterAnalysis mycluster;
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
    sz[3] = p4DImage->getCDim();
    V3DLONG size_image=sz[0]*sz[1];


    // set the parameter
    int thres_2d=30;
    int window_size=2;
    int ray_numbers_2d=128;
    int based_distance=10;
    int block_size=5;
    int num_layer=2;
    int delete_flag=1;
    int Db_thres=2;
    int clu_thres=100;
    V3DLONG nx=sz[0];
    V3DLONG ny=sz[1];
    V3DLONG nz=sz[2];

    if(p4DImage==NULL)
    {
       v3d_msg("No image ");
    }

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

          QSpinBox * window_size_spinbox = new QSpinBox();
          window_size_spinbox->setRange(1,10000);
          window_size_spinbox->setValue(window_size);

          QSpinBox * based_distance_spinbox = new QSpinBox();
          based_distance_spinbox->setRange(1,100);
          based_distance_spinbox->setValue(based_distance);

          QSpinBox * block_size_spinbox = new QSpinBox();
          block_size_spinbox->setRange(1,100);
          block_size_spinbox->setValue(block_size);

          QSpinBox * layer_size_spinbox = new QSpinBox();
          layer_size_spinbox->setRange(1,100);
          layer_size_spinbox->setValue(num_layer);

          QSpinBox * delete_flag_spinbox = new QSpinBox();
          delete_flag_spinbox->setRange(0,1);
          delete_flag_spinbox->setValue(delete_flag);

          QSpinBox * Db_thres_spinbox = new QSpinBox();
          Db_thres_spinbox->setRange(0,255);
          Db_thres_spinbox->setValue(Db_thres);

          QSpinBox * clu_thres_spinbox = new QSpinBox();
          clu_thres_spinbox->setRange(1,255);
          clu_thres_spinbox->setValue(clu_thres);


          layout->addWidget(new QLabel("number of points"),0,0);
          layout->addWidget(ray_numbers_2d_spinbox, 0,1,1,5);

          layout->addWidget(new QLabel("vision threshold"),1,0);
          layout->addWidget(thres_2d_spinbox, 1,1,1,5);

          layout->addWidget(new QLabel("window size of detected center point"),2,0);
          layout->addWidget(window_size_spinbox, 2,1,1,5);

          layout->addWidget(new QLabel("based distance "),3,0);
          layout->addWidget(based_distance_spinbox, 3,1,1,5);

          layout->addWidget(new QLabel("size of block"),4,0);
          layout->addWidget(block_size_spinbox, 4,1,1,5);

          layout->addWidget(new QLabel("number of layer"),5,0);
          layout->addWidget(layer_size_spinbox, 5,1,1,5);

          layout->addWidget(new QLabel("want to delete the small area"),6,0);
          layout->addWidget(delete_flag_spinbox, 6,1,1,5);

          layout->addWidget(new QLabel("Db_thres"),7,0);
          layout->addWidget(Db_thres_spinbox, 7,1,1,5);

          layout->addWidget(new QLabel("cluster threshold"),8,0);
          layout->addWidget(clu_thres_spinbox, 8,1,1,5);

          QHBoxLayout * hbox2 = new QHBoxLayout();
          QPushButton * ok = new QPushButton(" ok ");
          ok->setDefault(true);
          QPushButton * cancel = new QPushButton("cancel");
          hbox2->addWidget(cancel);
          hbox2->addWidget(ok);


          layout->addLayout(hbox2,9,0,1,9);
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
          cout<<sz[0]<<" "<<sz[1]<<" "<<sz[2]<<endl;

          //get the dialog return values
          ray_numbers_2d = ray_numbers_2d_spinbox->value();
          thres_2d = thres_2d_spinbox->value();
          window_size=window_size_spinbox->value();
          based_distance=based_distance_spinbox->value();
          block_size=block_size_spinbox->value();
          num_layer=layer_size_spinbox->value();
          Db_thres=Db_thres_spinbox->value();
          clu_thres=clu_thres_spinbox->value();

          if (dialog)
          {
                  delete dialog;
                  dialog=0;
                  cout<<"delete dialog"<<endl;
          }

            unsigned char* datald=0;
            datald=p4DImage->getRawData();

            unsigned char *image_mip;
            unsigned char *image_binary;
            unsigned char *old_image_binary;

            vector<V3DLONG> X_coor;
            vector<V3DLONG> Y_coor;
            vector<float> soma_radiu;
            /* create the ratate matrix */
            vector<vector<float> > ray_x(ray_numbers_2d,vector<float>(500)), ray_y(ray_numbers_2d,vector<float>(500));
            float ang = 2*PI/ray_numbers_2d;
            float x_dis, y_dis;
            int max_length=200; // if the rorate crash , please check the max_length;
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
            cout<<"create the model successful"<<endl;

            /* difine the color */
            RGBA8 red; red.r=255; red.g=0; red.b=0;
            RGBA8 green; green.r=0; green.g=255; green.b=0;
            RGBA8 blue; blue.r=0; blue.g=0; blue.b=255;
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
                       old_image_binary[a]=image_binary[a]; // save the original image;
                   }
                   skeletonization(nx,ny,image_binary);
                   vector<V3DLONG> X_candidate;  // save the adjusted x coordinate;
                   vector<V3DLONG> Y_candidate;  // save the adjusted y coordinate;
                   vector<float> Max_value;      // save the radiu;
                   int count=0; // the number of candidate
                   vector<V3DLONG> X_loc; //save the non-adjusted x coordinate;
                   vector<V3DLONG> Y_loc; //save the non-adjusted y coordinate;
                   seek_2D_candidate_points(nx,ny,image_binary,X_loc,Y_loc,count);
                   if(count>0)
                   {
                       for(int i=0;i<count;i++)
                       {
                           int edge_size =10;
                           if(((X_loc.at(i)-window_size)<=edge_size)||((Y_loc.at(i)-window_size)<=edge_size)||((Y_loc.at(i)+window_size)>=(ny-edge_size))||((X_loc.at(i)+window_size)>=(nx-edge_size)))
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
                   if(!(Max_value.size()==0))
                   {
                       std::vector<float>::iterator biggest=std::max_element(std::begin(Max_value),std::end(Max_value));
                       int location=std::distance(std::begin(Max_value),biggest);
                       X_coor.push_back(X_candidate.at(location));
                       Y_coor.push_back(Y_candidate.at(location));
                       soma_radiu.push_back(*biggest);
                   }
                   cout<<"the all candidate points in each MIP have detected"<<endl;

                   for(V3DLONG k = 0; k < X_candidate.size(); k++)
                   {
                       vector<float> x_loc;
                       vector<float> y_loc;
                       //create a new block_mip to detect the 2D branch points
                       int block_radiu=Max_value.at(k)+based_distance+block_size;
                       int block_length=block_radiu*2+1;
                       unsigned char *block=0;
                       try{block=new unsigned char [block_length*block_length*block_length];}
                       catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}
                       //cout<<"x is :"<<X_candidate.at(k)<<" "<<" y is :"<<Y_candidate.at(k)<<" "<<"the length is "<<block_length<<endl;
                       if(((X_candidate.at(k)+block_radiu)>(nx-1))||((X_candidate.at(k)-block_radiu)<1)||((Y_candidate.at(k)-block_radiu)<1)||((Y_candidate.at(k)+block_radiu)>(ny-1)))
                       {
                           cout<<"this points beyong the image"<<endl;
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
                       if(delete_flag)
                       {
                           delete_small_area(block_length,block_length,block);// connected domain denoising
                       }
                        for(int i = 0; i <ray_numbers_2d; i++)   //m is the numble of the ray
                        {
                            for(int j = Max_value.at(k)+based_distance; j < Max_value.at(k)+based_distance+2; j++)    // n is the numble of the points of the each ray
                            {
                                if((Max_value.at(k)+based_distance+2)>max_length)
                                    {
                                    v3d_msg(QString("the max_length is too small to can not adapt the model, please check the max_length"));
                                }
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
                       mycluster.Read_from_coordiante(x_loc,y_loc,DB_radius*Db_thres,2*Db_thres+1);
                       int flag=mycluster.DoDBSCANRecursive();

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
                                        s.color = red;
                                        curlist<<s;
                       }

                   }
                }
            }

            std::vector<float>::iterator biggest=std::max_element(std::begin(soma_radiu),std::end(soma_radiu));
            int new_location=std::distance(std::begin(soma_radiu),biggest);
            double new_max_radiu=*biggest;
            double soma_x=X_coor.at(new_location);
            double soma_y=Y_coor.at(new_location);
            double max_radiu=new_max_radiu+20;
            cout<<"the soma x is :"<<soma_x<<" "<<"the soma y is :"<<soma_y<<" "<<"the radiu is :"<<max_radiu<<endl;

            // delete the soma neiborhood branch points
            for(int i=0;i<curlist.size();i++)
            {
                if((pow(curlist.at(i).x-soma_x,2)+pow(curlist.at(i).y-soma_y,2))<pow(max_radiu,2))
                {
                    curlist.removeAt(i);
                    i=i-1;
                }
            }


            // cluster z slice branch points
//            for(V3DLONG i=0;i<curlist.size();i++)
//            {
//                unsigned char max_pixe=0;
//                for(V3DLONG j=1;j<nz;j++)
//                {
//                    unsigned char z_pixe=p4DImage->getValueUINT8(curlist[i].x,curlist[i].y,j,0);
//                    if(z_pixe>max_pixe)
//                    {
//                        max_pixe=z_pixe;
//                    }
//                }
//                unsigned char new_pixe=p4DImage->getValueUINT8(curlist[i].x,curlist[i].y,curlist[i].z,0);
//                if(new_pixe<max_pixe)
//                {
//                   curlist.removeAt(i);
//                   i=i-1;
//                }

//            }

//            bool flag_while_xy = true;
//            while(flag_while_xy)
//            {
//                bool endwhile = true;
//                for(V3DLONG i=0;i<curlist.size();i++)
//                {
//                    for(V3DLONG j=i+1;j<curlist.size();j++)
//                    {
//                        if((square(curlist[j].x-curlist[i].x)+square(curlist[j].y-curlist[i].y))<100&&(square(curlist[j].z-curlist[i].z))<500)
//                        {
//                            curlist[i].x=(curlist[j].x+curlist[i].x)/2;
//                            curlist[i].y=(curlist[j].y+curlist[i].y)/2;
//                            curlist[i].z=(curlist[j].z+curlist[i].z)/2;
//                            curlist[i].color.b+=51;
//                            curlist.removeAt(j);
//                            j=j-1;
//                            endwhile = false;
//                        }
//                    }
//                }
//                if(endwhile){flag_while_xy = false;}
//            }

            for(double i=0;i<curlist.size();i++)
            {
                for(double j=i+1;j<curlist.size();j++)
                {
                    if(((square(curlist[j].x-curlist[i].x)+square(curlist[j].y-curlist[i].y))<clu_thres)&&(square(curlist[j].z-curlist[i].z)<100))
                    {
                        curlist[i].x=(curlist[j].x+curlist[i].x)/2;
                        curlist[i].y=(curlist[j].y+curlist[i].y)/2;
                        curlist[i].z=(curlist[j].z+curlist[i].z)/2;
                        curlist.removeAt(j);
                        j=j-1;
                    }
                }
            }

            callback.setLandmark(curwin, curlist);
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
int rotate_vf_3projection(V3DPluginCallback2 &callback, QWidget *parent)
{

    ClusterAnalysis mycluster;
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
    sz[3] = p4DImage->getCDim();
    V3DLONG size_image=sz[0]*sz[1];


    // set the parameter
    int thres_2d=30;
    int ray_length=2;
    int ray_numbers_2d=128;
    int T0=10;
    int cluster_threshold=10;
    V3DLONG nx=sz[0];
    V3DLONG ny=sz[1];
    V3DLONG nz=sz[2];

    if(p4DImage==NULL)
    {
       v3d_msg("No image ");
    }

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

          QSpinBox * ray_length_spinbox = new QSpinBox();
          ray_length_spinbox->setRange(1,10000);
          ray_length_spinbox->setValue(ray_length);

          QSpinBox * based_distance_spinbox = new QSpinBox();
          based_distance_spinbox->setRange(1,100);
          based_distance_spinbox->setValue(T0);

          QSpinBox * cluster_threshold_spinbox = new QSpinBox();
          cluster_threshold_spinbox->setRange(0,255);
          cluster_threshold_spinbox->setValue(cluster_threshold);

          layout->addWidget(new QLabel("ray angle"),0,0);
          layout->addWidget(ray_numbers_2d_spinbox, 0,1,1,5);

          layout->addWidget(new QLabel("intensity threshold"),1,0);
          layout->addWidget(thres_2d_spinbox, 1,1,1,5);

          layout->addWidget(new QLabel("ray length"),2,0);
          layout->addWidget(ray_length_spinbox, 2,1,1,5);

          layout->addWidget(new QLabel("T0"),3,0);
          layout->addWidget(based_distance_spinbox, 3,1,1,5);

          layout->addWidget(new QLabel("cluster threshold"),4,0);
          layout->addWidget(cluster_threshold_spinbox, 4,1,1,5);

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
          cout<<sz[0]<<" "<<sz[1]<<" "<<sz[2]<<endl;

          //get the dialog return values
          ray_numbers_2d = ray_numbers_2d_spinbox->value();
          thres_2d = thres_2d_spinbox->value();
          ray_length=ray_length_spinbox->value();
          T0=based_distance_spinbox->value();
          cluster_threshold=cluster_threshold_spinbox->value();

          if (dialog)
          {
                  delete dialog;
                  dialog=0;
                  cout<<"delete dialog"<<endl;
          }



        unsigned char* datald=0;
        datald=p4DImage->getRawData();

        unsigned char *datald_mip;
        try{datald_mip=new unsigned char [size_image];}
        catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

        unsigned char *image_binary;
        try{image_binary=new unsigned char [size_image];}
        catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

        unsigned char *old_image_binary=0;
        try{old_image_binary=new unsigned char [size_image];}
        catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

//        Z_mip(nx,ny,nz,datald,datald_mip);
//        thres_segment(nx*ny,datald_mip,image_binary,thres_2d);

        gaussian_filter(datald_mip,nx,ny,nz,7,7,7,3,5,thres_2d,image_binary);
//        for(V3DLONG i=0;i<nx*ny;i++)
//        {
//            old_image_binary[i]=datald[i];
//        }

//        LandmarkList curlist;
//        LocationSimple s;
//        vector<V3DLONG> X_candidate;
//        vector<V3DLONG> Y_candidate;
//        vector<double> Max_value;

//        int count=0; // the number of candidate
//        vector<V3DLONG> X_loc; //save the non-adjusted x coordinate;
//        vector<V3DLONG> Y_loc; //save the non-adjusted y coordinate;
//        qint64 etime1;
//        QElapsedTimer timer1;
//        timer1.start();
//        skeletonization(nx,ny,datald);
//        seek_2D_candidate_points(nx,ny,datald,X_loc,Y_loc,count);
//        int window_size=1;
//        if(count>0)
//        {
//            for(int i=0;i<count;i++)
//            {
//                if(((X_loc.at(i)-window_size)<=0)||((Y_loc.at(i)-window_size)<=0)||((Y_loc.at(i)+window_size)>=ny)||((X_loc.at(i)+window_size)>=nx))
//                {
//                    continue;
//                }
//                /* using Spherical growth method designed by Chaowang*/
//                V3DLONG adjusted_x, adjusted_y;
//                double radiu;
//                find_neighborhood_maximum_radius(X_loc.at(i),Y_loc.at(i),old_image_binary,window_size,adjusted_x,adjusted_y,radiu,nx,ny,thres_2d);
//                X_candidate.push_back(adjusted_x);  // all adjusted x coordinate of candidate points
//                Y_candidate.push_back(adjusted_y);  // all adjusted y coordinate of candidate points
//                Max_value.push_back(radiu);    // all radiu coordinate of candidate points
//            }
//        }

//        else {
//            cout<<"no candidate points in this MIP"<<endl;
//        }
//        cout<<"the all candidate points have detected"<<endl;
//        /* create the ray-shooting matrix */
//        int max_length;
//        max_length=*max_element(Max_value.begin(),Max_value.end());
//        cout<<"the max length is :"<<max_length;
//        vector<vector<float> > ray_x(ray_numbers_2d,vector<float>(max_length+T0+5)), ray_y(ray_numbers_2d,vector<float>(max_length+T0+1));
//        float ang = 2*PI/ray_numbers_2d;
//        float x_dis, y_dis;
//        //int max_length=200; // if the rorate crash , please check the max_length;
//        for(int i = 0; i < ray_numbers_2d; i++)
//        {
//           x_dis = cos(ang*(i+1));
//           y_dis = sin(ang*(i+1));
//           for(int j = 0; j<max_length ; j++)
//               {
//                   ray_x[i][j] = x_dis*(j+1);
//                   ray_y[i][j] = y_dis*(j+1);
//               }
//        }

//        /* difine the color */
//        RGBA8 red; red.r=255; red.g=0; red.b=0;
//        RGBA8 green; green.r=0; green.g=255; green.b=0;
//        RGBA8 blue; blue.r=0; blue.g=0; blue.b=255;

//        for(V3DLONG k = 0; k < X_candidate.size(); k++)
//        {
//            vector<float> x_loc;
//            vector<float> y_loc;
//            double based_distance=T0;
//            //create a new block_mip to detect the 2D branch points
//            int block_radiu=Max_value.at(k)+based_distance+ray_length+1;
//            int block_length=block_radiu*2+1;
//            unsigned char *block=0;
//            try{block=new unsigned char [block_length*block_length*block_length];}
//            catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}
//            //cout<<"x is :"<<X_candidate.at(k)<<" "<<" y is :"<<Y_candidate.at(k)<<" "<<"the length is "<<block_length<<endl;
//            if(((X_candidate.at(k)+block_radiu)>(nx-1))||((X_candidate.at(k)-block_radiu)<1)||((Y_candidate.at(k)-block_radiu)<1)||((Y_candidate.at(k)+block_radiu)>(ny-1)))
//            {
//                cout<<"this points beyong the image"<<endl;
//                continue;
//            }
//            int num_block=0;
//            for(V3DLONG b=Y_candidate.at(k)-block_radiu;b<=Y_candidate.at(k)+block_radiu;b++)
//            {
//                 for(V3DLONG c=X_candidate.at(k)-block_radiu;c<=X_candidate.at(k)+block_radiu;c++)
//                 {
//                     unsigned char block_pixe=old_image_binary[b*nx+c];
//                     block[num_block]=block_pixe;
//                     num_block++;
//                 }
//             }
//            if(1)
//            {
//                delete_small_area(block_length,block_length,block);// connected domain denoising
//            }
//             for(int i = 0; i <ray_numbers_2d; i++)   //m is the numble of the ray
//             {
//                 for(int j = Max_value.at(k)+based_distance; j < Max_value.at(k)+based_distance+ray_length; j++)    // n is the numble of the points of the each ray
//                 {
//                     if((Max_value.at(k)+based_distance+2)>max_length)
//                         {
//                         v3d_msg(QString("the max_length is too small to can not adapt the model, please check the max_length"));
//                     }
//                     double pixe = project_interp_2d(block_radiu+ray_y[i][j], block_radiu+ray_x[i][j], block, block_length, block_length , block_radiu,  block_radiu);
//                     if(pixe>=100)
//                     {
//                         x_loc.push_back(block_radiu+ray_x[i][j]);
//                         y_loc.push_back(block_radiu+ray_y[i][j]);

//                     }
//                 }
//             }
//            double a=X_candidate.at(k)+ray_x[0][Max_value.at(k)+based_distance+ray_length]; //x1
//            double b=X_candidate.at(k)+ray_x[1][Max_value.at(k)+based_distance+ray_length]; //x2
//            double c=Y_candidate.at(k)+ray_y[0][Max_value.at(k)+based_distance+ray_length]; //y1
//            double d=Y_candidate.at(k)+ray_y[1][Max_value.at(k)+based_distance+ray_length]; //y2
//            double change_x=abs(a-b);
//            double change_x1=pow(change_x,2);
//            double change_y=abs(c-d);
//            double change_x2=pow(change_y,2);
//            double det_x=sqrt(change_x1+change_x2);

//            int det_y=1;
//            double DB_radius;
//            if(det_x>det_y)
//            {
//                DB_radius=det_x;
//            }
//            else {
//                DB_radius=det_y;
//            }
//            mycluster.Read_from_coordiante(x_loc,y_loc,(1.1*DB_radius),1);
//            int flag=mycluster.DoDBSCANRecursive();
//            if(flag==3)
//            {
//                             s.x=  X_candidate[k]+1;
//                             s.y = Y_candidate[k]+1;
//                             s.z = 1;
//                             s.radius = 1;
//                             s.color = red;
//                             curlist<<s;
//            }
//            else if(flag==4)
//            {
//                              s.x=  X_candidate[k]+1;
//                              s.y = Y_candidate[k]+1;
//                              s.z = 1;
//                              s.radius = 1;
//                              s.color = blue;
//                              curlist<<s;
//            }
//        }
//        etime1 = timer1.elapsed();
//        v3d_msg(QString("the detecting take %1 milliseconds").arg(etime1));

//        bool flag_while_xy = true;
//        while(flag_while_xy)
//        {
//            bool endwhile = true;
//            for(V3DLONG i=0;i<curlist.size();i++)
//            {
//                for(V3DLONG j=i+1;j<curlist.size();j++)
//                {
//                    if((square(curlist[j].x-curlist[i].x)+square(curlist[j].y-curlist[i].y))<cluster_threshold)
//                    {
//                        curlist[i].x=(curlist[j].x+curlist[i].x)/2;
//                        curlist[i].y=(curlist[j].y+curlist[i].y)/2;
//                        curlist[i].z=(curlist[j].z+curlist[i].z)/2;
////                        curlist[i].color.b+=51;
//                        curlist.removeAt(j);
//                        j=j-1;
//                        endwhile = false;
//                    }
//                }
//            }
//            if(endwhile){flag_while_xy = false;}
//        }
        Image4DSimple * new4DImage = new Image4DSimple();
        new4DImage->setData((unsigned char *)image_binary, nx, ny, 1, p4DImage->getCDim(), p4DImage->getDatatype());
        v3dhandle newwin = callback.newImageWindow();
        callback.setImage(newwin, new4DImage);
        callback.updateImageWindow(newwin);
//        callback.setLandmark(newwin, curlist);
        return 1;
}

int radius_estimation(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandle curwin = callback.currentImageWindow();
    if(!curwin)
    {
            v3d_msg("No image is open.");
            return -1;
    }
    Image4DSimple *p4DImage = callback.getImage(curwin);

    int x_coordinate=100, y_coordinate=100,thres_2d=45;

    V3DLONG sz[3];
    sz[0] = p4DImage->getXDim();
    sz[1] = p4DImage->getYDim();
    sz[2] = p4DImage->getZDim();
    sz[3] = p4DImage->getCDim();
    V3DLONG size_image=sz[0]*sz[1];
    V3DLONG nx=sz[0],ny=sz[1],nz=sz[2];


    //set update the dialog
    QDialog * dialog = new QDialog();


    if(p4DImage->getZDim() > 1)
            dialog->setWindowTitle("3D neuron image branch point detection Based on Ray-shooting algorithm");
    else
            dialog->setWindowTitle("2D neuron image branch point detection Based on Ray-shooting algorithm");
        QGridLayout * layout = new QGridLayout();

        QSpinBox * thres_2d_spinbox = new QSpinBox();
        thres_2d_spinbox->setRange(-1, 255);
        thres_2d_spinbox->setValue(thres_2d);

        QSpinBox * x_location_spinbox = new QSpinBox();
        x_location_spinbox->setRange(1,10000);
        x_location_spinbox->setValue(x_coordinate);

        QSpinBox * y_location_spinbox = new QSpinBox();
        y_location_spinbox->setRange(1,10000);
        y_location_spinbox->setValue(y_coordinate);






        layout->addWidget(new QLabel("intensity threshold"),1,0);
        layout->addWidget(thres_2d_spinbox, 1,1,1,5);

        layout->addWidget(new QLabel("x location"),2,0);
        layout->addWidget(x_location_spinbox, 2,1,1,5);

        layout->addWidget(new QLabel("y location"),3,0);
        layout->addWidget(y_location_spinbox, 3,1,1,5);



        QHBoxLayout * hbox2 = new QHBoxLayout();
        QPushButton * ok = new QPushButton(" ok ");
        ok->setDefault(true);
        QPushButton * cancel = new QPushButton("cancel");
        hbox2->addWidget(cancel);
        hbox2->addWidget(ok);

        layout->addLayout(hbox2,8,0,1,8);
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
        thres_2d = thres_2d_spinbox->value();
        y_coordinate=y_location_spinbox->value();
        x_coordinate=x_location_spinbox->value();


        if (dialog)
            {
                    delete dialog;
                    dialog=0;
                    cout<<"delete dialog"<<endl;
            }
        unsigned char* datald=0;
        datald=p4DImage->getRawData();

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
         int window_size=2;
         find_neighborhood_maximum_radius(x_coordinate,y_coordinate,seg_datald_mip,window_size,adjusted_x,adjusted_y,radiu,nx,ny,thres_2d);
         v3d_msg(QString("x location is %1, y location is %2, radius is %3").arg(x_coordinate).arg(y_coordinate).arg(radiu));
         //cout<<"x is :"<<adjusted_x<<" "<<"y is :"<<adjusted_y<<" "<<"the radiu  is :"<<radiu<<endl;
          return 1;

}

int classification_for_ray_shooting(V3DPluginCallback2 &callback, QWidget *parent)
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

         unsigned char *datald_mip;
         try{datald_mip=new unsigned char [nx*ny];}
         catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

         unsigned char *seg_datald_mip;
         try{seg_datald_mip=new unsigned char [nx*ny];}
         catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

         cout<<"allocate memory  success"<<endl;
        // v3d_msg(QString("x is %1, y is %2, z is %3").arg(x_location).arg(y_location).arg(z_location));

         Z_mip(nx,ny,nz,datald,datald_mip);
         thres_segment(nx*ny,datald,seg_datald_mip,thres_2d);

         LandmarkList curlist;
         LocationSimple s;


         float pixe=0,sum;
//         float source_pixe;
         vector<int> temp_gap;//ind1 is the all piex of each ray
         vector<float> x_loc,x_loc1;
         vector<float> y_loc,y_loc1;
         int gap_pixe,T0,T1,n=2;
         RGBA8 red; red.r=255; red.g=170; red.b=255;
         RGBA8 blue; blue.r=65; blue.g=113; blue.b=156;
         int gap_length,flag;
         for(int i = 0; i <ray_numbers_2d; i++)   //m is the numble of the ray
         {
             vector<float> temp_pixel;
             gap_pixe=0;
             flag=0;
             for(int j = 0; j < ray_length_2d; j++)    // n is the numble of the points of the each ray
             {
                 pixe = project_interp_2d((y_location)+ray_y[i][j], (x_location)+ray_x[i][j], seg_datald_mip, nx,ny , (x_location), (y_location));
//                 cout<<"pixe:" <<pixe<<"i:" <<i<<"j:" <<j<<endl;
                 if(pixe>100)
                 {
                     x_loc.push_back(x_location+ray_x[i][j]);
                     y_loc.push_back(y_location+ray_y[i][j]);
                 }

                 gap_pixe=gap_pixe+pixe;

             }
//             if(gap_pixe>1100)
//             {
//                 for(int j = 0; j < ray_length_2d; j++)
//                 {
//                     x_loc.push_back(x_location+ray_x[i][j]);
//                     y_loc.push_back(y_location+ray_y[i][j]);
//                 }
//             }
//             else
//             {
//                 for(int j = 0; j < ray_length_2d; j++)
//                 {
//                     x_loc1.push_back(x_location+ray_x[i][j]);
//                     y_loc1.push_back(y_location+ray_y[i][j]);
//                 }
//             }
         }
         for(V3DLONG i = 0; i <x_loc.size(); i++)
         {
             s.x=  x_loc[i]+1;
             s.y = y_loc[i]+1;
             s.z = 1;
             s.radius = 1;
             s.color=red;
             curlist<<s;
         }
         for(V3DLONG i = 0; i <x_loc1.size(); i++)
         {
             s.x=  x_loc1[i]+1;
             s.y = y_loc1[i]+1;
             s.z = 1;
             s.radius = 1;
             s.color=blue;
             curlist<<s;
         }

         callback.setLandmark(curwin, curlist);
         return 1;

}

int Base_distance_for_ray_scan(V3DPluginCallback2 &callback, QWidget *parent)
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
         thres_segment(nx*ny,datald,seg_datald_mip,thres_2d);

         LandmarkList curlist;
         LocationSimple s;


         float pixe=0,sum;
//         float source_pixe;
         vector<int> temp_gap;//ind1 is the all piex of each ray
         vector<float> x_loc,x_loc1;
         vector<float> y_loc,y_loc1;
         int gap_pixe,T0,T1,n=2;
         RGBA8 red; red.r=255; red.g=170; red.b=255;
         int gap_length,flag;
         for(int i = 0; i <ray_numbers_2d; i++)   //m is the numble of the ray
         {
             vector<float> temp_pixel;
             gap_pixe=0;
             flag=0;
             for(int j = 0; j < ray_length_2d; j++)    // n is the numble of the points of the each ray
             {
                 pixe = project_interp_2d((y_location)+ray_y[i][j], (x_location)+ray_x[i][j], seg_datald_mip, nx,ny , (x_location), (y_location));
                 cout<<"pixe:" <<pixe<<"i:" <<i<<"j:" <<j<<endl;
                 temp_pixel.push_back(pixe);
                 if((j>=2))
                 {
                     x_loc.push_back(x_location+ray_x[i][j-2]);
                     y_loc.push_back(y_location+ray_y[i][j-2]);
                 }


                 if((j<6))
                 {
                     gap_pixe=gap_pixe;
                 }
                 else if (j>=6)
                 {
                     sum=((temp_pixel.at(j-2)-temp_pixel.at(j))+(temp_pixel.at(j-3)-temp_pixel.at(j-1))
                           +(temp_pixel.at(j-4)-temp_pixel.at(j-2)));
                     if ((sum<(-320))&&(sum>(-550)))
                     {
                         if((temp_pixel.at(j-4)<20)&&(temp_pixel.at(j-5)<20)&&(temp_pixel.at(j-6)<20))
                         {
                             flag=1;
                             break;
                         }

                     }
                 }
                 gap_pixe=gap_pixe+1;

             }
             if((flag==0)/*||(gap_pixe==0)*//*||(gap_pixe==1)*/) //Judge no gap ray
             {
                 gap_pixe=ray_numbers_2d-3;
             }

//             v3d_msg(QString("temp_pixel is %1").arg(temp_pixel),0);
             temp_gap.push_back(gap_pixe);
             cout<<"gap_pixe:" <<gap_pixe<<endl;
         }
         gap_length=*min_element(temp_gap.begin(),temp_gap.end());
//         cout<<"gap_length_final:" <<gap_length<<endl;
         for(V3DLONG i = 0; i <x_loc.size(); i++)
         {
             s.x=  x_loc[i]+1;
             s.y = y_loc[i]+1;
             s.z = 1;
             s.radius = 1;
             s.color=red;
             curlist<<s;
         }

         double redius=markerRadius_hanchuan_XY(seg_datald_mip,nx,ny,(x_location),(y_location),thres_2d);
//         cout<<"radius:" <<redius<<endl;
         if((gap_length-redius-5)<=0)
         {
             T0=2;
             T1=2;
         }
         else if ((gap_length-redius-5)>0)
         {
             T0=gap_length-redius-4;
             T1=gap_length-redius-4;
         }
         cout<<"1 T0:" <<T0<<"1 T1:" <<T1<<endl;
         get_the_based_distance_rayscan((x_location),(y_location),redius,seg_datald_mip, nx,ny ,T0, T1);
         cout<<"2 T0:" <<T0<<"2 T1:" <<T1<<endl;
         callback.setLandmark(curwin, curlist);
         return 1;

}
template <class T> void gaussian_filter(T* data1d,
                                        V3DLONG nx, V3DLONG ny,V3DLONG nz,
                                        unsigned int Wx,
                                        unsigned int Wy,
                                        unsigned int Wz,
                                        unsigned int c,
                                        double sigma,
                                        int thres, T* &out_data1d)
{
    float *outimg=0;
    if (!data1d  || nx<=0 || ny<=0 || nz<=0 )
    {
        v3d_msg("Invalid parameters to gaussian_filter().", 0);
        return;
    }

//    if (outimg)
//    {
//        v3d_msg("Warning: you have supplied an non-empty output image pointer. This program will force to free it now. But you may want to double check.");
//        delete []outimg;
//        outimg = 0;
//    }

     // for filter kernel
     double sigma_s2 = 0.5/(sigma*sigma); // 1/(2*sigma*sigma)
     double pi_sigma = 1.0/(sqrt(2*3.1415926)*sigma); // 1.0/(sqrt(2*pi)*sigma)

     float min_val = INF, max_val = 0;

     V3DLONG N = nx;
     V3DLONG M = ny;
     V3DLONG P = nz;
     V3DLONG pagesz = N*M*P;

     //filtering
     V3DLONG offset_init = (c-1)*pagesz;

     //declare temporary pointer
     float *pImage = new float [pagesz];
     if (!pImage)
     {
          printf("Fail to allocate memory.\n");
          return;
     }
     else
     {
          for(V3DLONG i=0; i<pagesz; i++)
               pImage[i] = data1d[i + offset_init];  //first channel data (red in V3D, green in ImageJ)
     }
       //Filtering
     //
     //   Filtering along x
     if(N<2)
     {
          //do nothing
     }
     else
     {
          //create Gaussian kernel
          float  *WeightsX = 0;
          WeightsX = new float [Wx];
          if (!WeightsX)
               return;

          float Half = (float)(Wx-1)/2.0;

          // Gaussian filter equation:
          // http://en.wikipedia.org/wiki/Gaussian_blur
       //   for (unsigned int Weight = 0; Weight < Half; ++Weight)
       //   {
       //        const float  x = Half* float (Weight) / float (Half);
      //         WeightsX[(int)Half - Weight] = WeightsX[(int)Half + Weight] = pi_sigma * exp(-x * x *sigma_s2); // Corresponding symmetric WeightsX
      //    }

          for (unsigned int Weight = 0; Weight <= Half; ++Weight)
          {
              const float  x = float(Weight)-Half;
              WeightsX[Weight] = WeightsX[Wx-Weight-1] = pi_sigma * exp(-(x * x *sigma_s2)); // Corresponding symmetric WeightsX
          }


          double k = 0.;
          for (unsigned int Weight = 0; Weight < Wx; ++Weight)
               k += WeightsX[Weight];

          for (unsigned int Weight = 0; Weight < Wx; ++Weight)
               WeightsX[Weight] /= k;

         printf("\n x dierction");

         for (unsigned int Weight = 0; Weight < Wx; ++Weight)
             printf("/n%f",WeightsX[Weight]);

          //   Allocate 1-D extension array
          float  *extension_bufferX = 0;
          extension_bufferX = new float [N + (Wx<<1)];

          unsigned int offset = Wx>>1;

          //	along x
          const float  *extStop = extension_bufferX + N + offset;

          for(V3DLONG iz = 0; iz < P; iz++)
          {
               for(V3DLONG iy = 0; iy < M; iy++)
               {
                    float  *extIter = extension_bufferX + Wx;
                    for(V3DLONG ix = 0; ix < N; ix++)
                    {
                         *(extIter++) = pImage[iz*M*N + iy*N + ix];
                    }

                    //   Extend image
                    const float  *const stop_line = extension_bufferX - 1;
                    float  *extLeft = extension_bufferX + Wx - 1;
                    const float  *arrLeft = extLeft + 2;
                    float  *extRight = extLeft + N + 1;
                    const float  *arrRight = extRight - 2;

                    while (extLeft > stop_line)
                    {
                         *(extLeft--) = *(arrLeft++);
                         *(extRight++) = *(arrRight--);

                    }

                    //	Filtering
                    extIter = extension_bufferX + offset;

                    float  *resIter = &(pImage[iz*M*N + iy*N]);

                    while (extIter < extStop)
                    {
                         double sum = 0.;
                         const float  *weightIter = WeightsX;
                         const float  *const End = WeightsX + Wx;
                         const float * arrIter = extIter;
                         while (weightIter < End)
                              sum += *(weightIter++) * float (*(arrIter++));
                         extIter++;
                         *(resIter++) = sum;

                         //for rescale
                         if(max_val<*arrIter) max_val = *arrIter;
                         if(min_val>*arrIter) min_val = *arrIter;


                    }

               }
          }
          //de-alloc
           if (WeightsX) {delete []WeightsX; WeightsX=0;}
           if (extension_bufferX) {delete []extension_bufferX; extension_bufferX=0;}

     }

     //   Filtering along y
     if(M<2)
     {
          //do nothing
     }
     else
     {
          //create Gaussian kernel
          float  *WeightsY = 0;
          WeightsY = new float [Wy];
          if (!WeightsY)
               return;

          float Half = (float)(Wy-1)/2.0;

          // Gaussian filter equation:
          // http://en.wikipedia.org/wiki/Gaussian_blur
         /* for (unsigned int Weight = 0; Weight < Half; ++Weight)
          {
               const float  y = Half* float (Weight) / float (Half);
               WeightsY[(int)Half - Weight] = WeightsY[(int)Half + Weight] = pi_sigma * exp(-y * y *sigma_s2); // Corresponding symmetric WeightsY
          }*/

          for (unsigned int Weight = 0; Weight <= Half; ++Weight)
          {
              const float  y = float(Weight)-Half;
              WeightsY[Weight] = WeightsY[Wy-Weight-1] = pi_sigma * exp(-(y * y *sigma_s2)); // Corresponding symmetric WeightsY
          }


          double k = 0.;
          for (unsigned int Weight = 0; Weight < Wy; ++Weight)
               k += WeightsY[Weight];

          for (unsigned int Weight = 0; Weight < Wy; ++Weight)
               WeightsY[Weight] /= k;

          //	along y
          float  *extension_bufferY = 0;
          extension_bufferY = new float [M + (Wy<<1)];

          unsigned int offset = Wy>>1;
          const float *extStop = extension_bufferY + M + offset;

          for(V3DLONG iz = 0; iz < P; iz++)
          {
               for(V3DLONG ix = 0; ix < N; ix++)
               {
                    float  *extIter = extension_bufferY + Wy;
                    for(V3DLONG iy = 0; iy < M; iy++)
                    {
                         *(extIter++) = pImage[iz*M*N + iy*N + ix];
                    }

                    //   Extend image
                    const float  *const stop_line = extension_bufferY - 1;
                    float  *extLeft = extension_bufferY + Wy - 1;
                    const float  *arrLeft = extLeft + 2;
                    float  *extRight = extLeft + M + 1;
                    const float  *arrRight = extRight - 2;

                    while (extLeft > stop_line)
                    {
                         *(extLeft--) = *(arrLeft++);
                         *(extRight++) = *(arrRight--);
                    }

                    //	Filtering
                    extIter = extension_bufferY + offset;

                    float  *resIter = &(pImage[iz*M*N + ix]);

                    while (extIter < extStop)
                    {
                         double sum = 0.;
                         const float  *weightIter = WeightsY;
                         const float  *const End = WeightsY + Wy;
                         const float * arrIter = extIter;
                         while (weightIter < End)
                              sum += *(weightIter++) * float (*(arrIter++));
                         extIter++;
                         *resIter = sum;
                         resIter += N;

                         //for rescale
                         if(max_val<*arrIter) max_val = *arrIter;
                         if(min_val>*arrIter) min_val = *arrIter;


                    }

               }
          }

          //de-alloc
          if (WeightsY) {delete []WeightsY; WeightsY=0;}
          if (extension_bufferY) {delete []extension_bufferY; extension_bufferY=0;}


     }

     //  Filtering  along z
     if(P<2)
     {
          //do nothing
     }
     else
     {
          //create Gaussian kernel
          float  *WeightsZ = 0;
          WeightsZ = new float [Wz];
          if (!WeightsZ)
               return;

          float Half = (float)(Wz-1)/2.0;

         /* for (unsigned int Weight = 1; Weight < Half; ++Weight)
          {
               const float  z = Half * float (Weight) / Half;
               WeightsZ[(int)Half - Weight] = WeightsZ[(int)Half + Weight] = pi_sigma * exp(-z * z * sigma_s2) ; // Corresponding symmetric WeightsZ
          }*/

          for (unsigned int Weight = 0; Weight <= Half; ++Weight)
          {
              const float  z = float(Weight)-Half;
              WeightsZ[Weight] = WeightsZ[Wz-Weight-1] = pi_sigma * exp(-(z * z *sigma_s2)); // Corresponding symmetric WeightsZ
          }


          double k = 0.;
          for (unsigned int Weight = 0; Weight < Wz; ++Weight)
               k += WeightsZ[Weight];

          for (unsigned int Weight = 0; Weight < Wz; ++Weight)
               WeightsZ[Weight] /= k;

          //	along z
          float  *extension_bufferZ = 0;
          extension_bufferZ = new float [P + (Wz<<1)];

          unsigned int offset = Wz>>1;
          const float *extStop = extension_bufferZ + P + offset;

          for(V3DLONG iy = 0; iy < M; iy++)
          {
               for(V3DLONG ix = 0; ix < N; ix++)
               {

                    float  *extIter = extension_bufferZ + Wz;
                    for(V3DLONG iz = 0; iz < P; iz++)
                    {
                         *(extIter++) = pImage[iz*M*N + iy*N + ix];
                    }

                    //   Extend image
                    const float  *const stop_line = extension_bufferZ - 1;
                    float  *extLeft = extension_bufferZ + Wz - 1;
                    const float  *arrLeft = extLeft + 2;
                    float  *extRight = extLeft + P + 1;
                    const float  *arrRight = extRight - 2;

                    while (extLeft > stop_line)
                    {
                         *(extLeft--) = *(arrLeft++);
                         *(extRight++) = *(arrRight--);
                    }

                    //	Filtering
                    extIter = extension_bufferZ + offset;

                    float  *resIter = &(pImage[iy*N + ix]);

                    while (extIter < extStop)
                    {
                         double sum = 0.;
                         const float  *weightIter = WeightsZ;
                         const float  *const End = WeightsZ + Wz;
                         const float * arrIter = extIter;
                         while (weightIter < End)
                              sum += *(weightIter++) * float (*(arrIter++));
                         extIter++;
                         *resIter = sum;
                         resIter += M*N;

                         //for rescale
                         if(max_val<*arrIter) max_val = *arrIter;
                         if(min_val>*arrIter) min_val = *arrIter;

                    }

               }
          }

          //de-alloc
          if (WeightsZ) {delete []WeightsZ; WeightsZ=0;}
          if (extension_bufferZ) {delete []extension_bufferZ; extension_bufferZ=0;}


     }

    outimg = pImage;
    outimg = pImage;
    for(V3DLONG i=0;i++;i<N*M)
    {
        if(outimg[i]<thres)
        {
            out_data1d[i]=0;
        }
        else {
            out_data1d[i]=255;
        }
    }


    return;
}
