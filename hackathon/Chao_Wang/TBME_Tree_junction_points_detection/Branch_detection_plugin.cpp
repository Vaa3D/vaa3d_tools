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
#include"../../../../vaa3d_tools/released_plugins/v3d_plugins/neurontracing_vn2/neuron_radius/marker_radius.h"
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
        <<tr("2D juntion detection")
        <<tr("3D junction points detection")
        <<tr("2D junction detection based on ray-shoooting model ")
        <<tr("2D rotate algorithm display")
        <<tr("local radius estimation")
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
    if(menu_name==tr("3D junction points detection"))
    {
        int flag=junction_points_detection_3D(callback,parent);
        if(flag==1)
            v3d_msg("OK ");
    }
    else if(menu_name==tr("2D juntion detection"))
        {
             int cs=0;
             if(QMessageBox::Yes == QMessageBox::question (0, "", QString(" yes: neuron junction detection; no : blood vessel junction detection"), QMessageBox::Yes, QMessageBox::No))    cs = 1;
             if(cs==1)
             {
                 int flag=ray_scan_model(callback,parent);
                 if(flag==1)
                 {
                   v3d_msg("2D junction points have been detected ");
                 }
             }

             else
                 {
                     int flag=ray_scan_model_vessel(callback,parent);
                     if(flag==1)
                     {
                         v3d_msg("2D junction points have been detected  ");
                     }

                  }
          }


        else if(menu_name==tr("local radius estimation"))
        {
            int flag=radius_estimation(callback,parent);
            if(flag==1)
                v3d_msg("OK ");
        }
    else if(menu_name==tr("2D rotate algorithm display"))
    {
        int flag=rotate_2D_display_point(callback,parent);
        if(flag==1)
            v3d_msg("2D branch points completed ");
    }
    else if(menu_name==tr("2D junction detection based on ray-shoooting model "))
        {
            int flag=ray_shooting_model(callback,parent);
            if(flag==1)
                v3d_msg("2D junction point have been detected ");
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
    int cluster_threshold=20;
    int T0=3;
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

          QSpinBox * T0_spinbox = new QSpinBox();
          T0_spinbox->setRange(0,255);
          T0_spinbox->setValue(cluster_threshold);


          layout->addWidget(new QLabel("ray angle"),0,0);
          layout->addWidget(ray_numbers_2d_spinbox, 0,1,1,5);

          layout->addWidget(new QLabel("intensity threshold"),1,0);
          layout->addWidget(thres_2d_spinbox, 1,1,1,5);

          layout->addWidget(new QLabel("cluster threshold"),2,0);
          layout->addWidget(cluster_threshold_spinbox, 2,1,1,5);

          layout->addWidget(new QLabel("T0"),3,0);
          layout->addWidget(T0_spinbox, 3,1,1,5);


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
                        for(int i = 0; i <ray_numbers_2d; i++)   //i is the numble of the ray
                        {
                            ray_pxie_sum=0;
                            max_value=0;
                            for(int j = 0; j < T0*int(Max_value.at(k)); j++)    // j is the numble of circle
                            {
                                if(((X_candidate.at(k)+ T0*int(Max_value.at(k)))>nx)||((Y_candidate.at(k)+ T0*int(Max_value.at(k)))>ny))
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

            if((flag==3)||(flag==4))
            {
                             s.x=  X_candidate[k]+1;
                             s.y = Y_candidate[k]+1;
                             s.z = 1;
                             s.radius = 1;
                             s.color = red;
                             curlist<<s;
            }
        }
        etime1 = timer1.elapsed();
        v3d_msg(QString("the detecting take %1 milliseconds").arg(etime1));

//        std::vector<double>::iterator biggest=std::max_element(std::begin(Max_value),std::end(Max_value));
//        int new_location=std::distance(std::begin(Max_value),biggest);
//        double new_max_radiu=*biggest;
//        double soma_x=X_loc.at(new_location);
//        double soma_y=Y_loc.at(new_location);
//        double max_radiu=new_max_radiu+20;
//        cout<<"the soma x is :"<<soma_x<<" "<<"the soma y is :"<<soma_y<<" "<<"the radiu is :"<<max_radiu<<endl;

//        // delete the soma neiborhood branch points
//        for(int i=0;i<curlist.size();i++)
//        {
//            if((pow(curlist.at(i).x-soma_x,2)+pow(curlist.at(i).y-soma_y,2))<pow(max_radiu,2))
//            {
//                curlist.removeAt(i);
//                i=i-1;
//            }
//        }




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
        based_distance_spinbox->setValue(T0);

        QSpinBox * block_size_spinbox = new QSpinBox();
        block_size_spinbox->setRange(1,100);
        block_size_spinbox->setValue(block_size);

        QSpinBox * ray_length_spinbox = new QSpinBox();
        ray_length_spinbox->setRange(1,20);
        ray_length_spinbox->setValue(ray_length);


        layout->addWidget(new QLabel("ray numbers"),0,0);
        layout->addWidget(ray_numbers_2d_spinbox, 0,1,1,5);

        layout->addWidget(new QLabel("intensity threshold"),1,0);
        layout->addWidget(thres_2d_spinbox, 1,1,1,5);

        layout->addWidget(new QLabel("x location"),2,0);
        layout->addWidget(x_location_spinbox, 2,1,1,5);

        layout->addWidget(new QLabel("y location"),3,0);
        layout->addWidget(y_location_spinbox, 3,1,1,5);

        layout->addWidget(new QLabel(" T0 "),5,0);
        layout->addWidget(based_distance_spinbox, 5,1,1,5);

        layout->addWidget(new QLabel("block size "),4,0);
        layout->addWidget(block_size_spinbox, 4,1,1,5);

        layout->addWidget(new QLabel("ray length "),6,0);
        layout->addWidget(ray_length_spinbox, 6,1,1,5);




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
         find_neighborhood_maximum_radius(x_coordinate,y_coordinate,seg_datald_mip,window_size,adjusted_x,adjusted_y,radiu,nx,ny,thres_2d);
         v3d_msg(QString("the estmated radius is %1").arg(radiu));
         vector<vector<double> > ray_x(ray_numbers_2d,vector<double>(100)), ray_y(ray_numbers_2d,vector<double>(100));
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
         int block_radiu=radiu+double(T0/10)*radiu+block_size;
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
          double based_distance= float(T0*0.1)*radiu;
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

          mycluster.Read_from_coordiante(x_loc,y_loc,DB_radius*1.1,3);
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
    int T0=2;
    int cluster_threshold=100;
    int Db_thres=2;
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

          QSpinBox * based_distance_spinbox = new QSpinBox();
          based_distance_spinbox->setRange(1,100);
          based_distance_spinbox->setValue(T0);

          QSpinBox * cluster_threshold_spinbox = new QSpinBox();
          cluster_threshold_spinbox->setRange(0,255);
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

          layout->addWidget(new QLabel("T0"),3,0);
          layout->addWidget(based_distance_spinbox, 3,1,1,5);

          layout->addWidget(new QLabel("cluster threshold"),4,0);
          layout->addWidget(cluster_threshold_spinbox, 4,1,1,5);

          layout->addWidget(new QLabel("Db_thres"),5,0);
          layout->addWidget(Db_thres_spinbox, 5,1,1,5);

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
        vector<double> Max_value;

        int count=0; // the number of candidate
        vector<V3DLONG> X_loc; //save the non-adjusted x coordinate;
        vector<V3DLONG> Y_loc; //save the non-adjusted y coordinate;
        qint64 etime1;
        QElapsedTimer timer1;
        timer1.start();
        skeletonization(nx,ny,image_binary);
        int cs=0;
        if(QMessageBox::Yes == QMessageBox::question (0, "", QString(" yes: neuron junction detection; no : blood vessel junction detection"), QMessageBox::Yes, QMessageBox::No))    cs = 1;
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
            vector<float> x_loc;
            vector<float> y_loc;
            /* using the ray sacn to detect 2D junction points designed by chaowang 2019/1/15 */
            double ray_sum;
            double max_value;
            for(int i = 0; i <ray_numbers_2d; i++)   //m is the numble of the ray
            {
               ray_sum=0;
               max_value=0;
                for(int j = Max_value.at(k)+T0*Max_value.at(k); j < Max_value.at(k)+T0*Max_value.at(k)+ray_length; j++)    // n is the numble of the points of the each ray
                {
                    if((Max_value.at(k)+T0*Max_value.at(k)+ray_length)>max_length)
                    {
                        v3d_msg(QString("the max_length is too small to include the model, please check the max_length"));
                    }
                    double pixe = project_interp_2d(Y_candidate.at(k)+ray_y[i][j], X_candidate.at(k)+ray_x[i][j], old_image_binary, nx, ny , Y_candidate.at(k),  X_candidate.at(k));
                    ray_sum=ray_sum+pixe;
                    if(pixe>max_value)
                    {
                        max_value=pixe;
                    }
                }
                if((ray_sum/(ray_length+1))>(0.5*max_value))
                {
                    for(int j = Max_value.at(k)+T0*Max_value.at(k); j < Max_value.at(k)+T0*Max_value.at(k)+ray_length; j++)
                    {
                        x_loc.push_back(X_candidate.at(k)+ray_x[i][j]);
                        y_loc.push_back(Y_candidate.at(k)+ray_y[i][j]);
                    }
                }
            }

//           /* calculate the DBscan radiu */
            float change_x1=cos(ang*3)*(Max_value.at(k)+T0*Max_value.at(k)+3)-cos(ang*2)*(Max_value.at(k)+T0*Max_value.at(k)+3);
            float change_x=pow(change_x1,2);
            float change_y1=sin(ang*3)*(Max_value.at(k)+T0*Max_value.at(k)+3)-sin(ang*2)*(Max_value.at(k)+T0*Max_value.at(k)+3);
            float change_y=pow(change_y1,2);
            float DB_radius=sqrt(change_x+change_y)+1;
            mycluster.Read_from_coordiante(x_loc,y_loc,Db_thres*DB_radius,Db_thres*2+1);
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

        callback.setLandmark(curwin, curlist);
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
        skeletonization(nx,ny,datald);
        seek_2D_candidate_points(nx,ny,datald,X_loc,Y_loc,count);
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
            double based_distance=T0;
            //create a new block_mip to detect the 2D branch points
            int block_radiu=Max_value.at(k)+based_distance+ray_length+5;
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
            if(1)
            {
                delete_small_area(block_length,block_length,block);// connected domain denoising
            }
             for(int i = 0; i <ray_numbers_2d; i++)   //m is the numble of the ray
             {
                 for(int j = Max_value.at(k)+based_distance; j < Max_value.at(k)+based_distance+ray_length; j++)    // n is the numble of the points of the each ray
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
            mycluster.Read_from_coordiante(x_loc,y_loc,(1.1*DB_radius),1);
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
            else if(flag==4)
            {
                              s.x=  X_candidate[k]+1;
                              s.y = Y_candidate[k]+1;
                              s.z = 1;
                              s.radius = 1;
                              s.color = blue;
                              curlist<<s;
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
//                        curlist[i].color.b+=51;
                        curlist.removeAt(j);
                        j=j-1;
                        endwhile = false;
                    }
                }
            }
            if(endwhile){flag_while_xy = false;}
        }
        callback.setLandmark(curwin, curlist);
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
int junction_points_detection_3D(V3DPluginCallback2 &callback, QWidget *parent)
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

        // set the parameter
        int thres_2d=30;
        int ray_numbers_2d=128;
        int based_distance=5;
        int num_layer=3;
        int clu_thres=100;
        int ray_length=2;
        V3DLONG nx=sz[0];
        V3DLONG ny=sz[1];
        V3DLONG nz=sz[2];
        V3DLONG image_size_xy=nx*ny;
        V3DLONG image_size_yz=nz*ny;
        V3DLONG image_size_xz=nx*nz;


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

              QSpinBox * based_distance_spinbox = new QSpinBox();
              based_distance_spinbox->setRange(1,100);
              based_distance_spinbox->setValue(based_distance);

              QSpinBox * ray_length_spinbox = new QSpinBox();
              ray_length_spinbox->setRange(1,100);
              ray_length_spinbox->setValue(ray_length);

              QSpinBox * layer_size_spinbox = new QSpinBox();
              layer_size_spinbox->setRange(1,100);
              layer_size_spinbox->setValue(num_layer);

              QSpinBox * clu_thres_spinbox = new QSpinBox();
              clu_thres_spinbox->setRange(1,255);
              clu_thres_spinbox->setValue(clu_thres);


              layout->addWidget(new QLabel("number of points"),0,0);
              layout->addWidget(ray_numbers_2d_spinbox, 0,1,1,5);

              layout->addWidget(new QLabel("vision threshold"),1,0);
              layout->addWidget(thres_2d_spinbox, 1,1,1,5);

              layout->addWidget(new QLabel("based distance "),2,0);
              layout->addWidget(based_distance_spinbox, 2,1,1,5);

              layout->addWidget(new QLabel("number of layer"),3,0);
              layout->addWidget(layer_size_spinbox, 3,1,1,5);

              layout->addWidget(new QLabel("cluster threshold"),4,0);
              layout->addWidget(clu_thres_spinbox, 4,1,1,5);

              layout->addWidget(new QLabel("ray length"),5,0);
              layout->addWidget(ray_length_spinbox, 5,1,1,5);

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
              based_distance=based_distance_spinbox->value();
              ray_length=ray_length_spinbox->value();
              num_layer=layer_size_spinbox->value();
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

                unsigned char *image_mip_yz;
                unsigned char *image_binary_yz;
                unsigned char *old_image_binary_yz;

                unsigned char *image_mip_xz;
                unsigned char *image_binary_xz;
                unsigned char *old_image_binary_xz;



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
                try{image_mip=new unsigned char [image_size_xy];}
                catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

                try{image_binary=new unsigned char [image_size_xy];}
                catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

                try{old_image_binary=new unsigned char [image_size_xy];}
                catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

                cout<<"allocate the memory of xy plane is successful"<<endl;


                try{image_mip_yz=new unsigned char [image_size_yz];}
                catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

                try{image_binary_yz=new unsigned char [image_size_yz];}
                catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

                try{old_image_binary_yz=new unsigned char [image_size_yz];}
                catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

                cout<<"allocate the memory of yz plane is successful"<<endl;

                try{image_mip_xz=new unsigned char [image_size_xz];}
                catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

                try{image_binary_xz=new unsigned char [image_size_xz];}
                catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

                try{old_image_binary_xz=new unsigned char [image_size_xz];}
                catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

                cout<<"allocate the memory of xz plane is successful"<<endl;

                LandmarkList curlist, curlist_yz, curlist_xz;
                LocationSimple s , s_yz, s_xz;

                int para=1;
                int case1=1, case2=0,case3=0;
                if(QMessageBox::Yes == QMessageBox::question (0, "", QString(" yes: 3D junction from xy yz xy project) ; no : 3D junction only xy project(less time for much image from Bigneuron data)"), QMessageBox::Yes, QMessageBox::No))    para = 0;

                if(para==0)
                {
                    case2=1;
                    case3=1;
                }



              if(case2)
              {
                cout<<"begin to detect juntion point of each yz project"<<endl;
                for(int size_x=num_layer;size_x<nx-num_layer;size_x+=MAX(1,num_layer/2))
                {

                    /* detect the 2D branch points in each MIP */
                    if((size_x+num_layer)<nx)
                    {
                       mip_layer_yz(nx,ny,nz,size_x,datald,image_mip_yz,num_layer);
                       thres_segment(ny*nz,image_mip_yz,image_binary_yz,thres_2d);
                       for(V3DLONG a=0;a<nz*ny;a++)
                       {
                           old_image_binary_yz[a]=image_binary_yz[a]; // save the original image;
                       }
                       skeletonization(ny,nz,image_binary_yz);
                       vector<V3DLONG> Y_candidate;  // save the adjusted y coordinate;
                       vector<V3DLONG> Z_candidate;  // save the adjusted z coordinate;
                       vector<float> Max_value;      // save the radiu;
                       int count=0; // the number of candidate
                       vector<V3DLONG> Y_loc; //save the non-adjusted y coordinate;
                       vector<V3DLONG> Z_loc; //save the non-adjusted z coordinate;
                       seek_2D_candidate_points(ny,nz,image_binary_yz,Y_loc,Z_loc,count);
                       int window_size=2;
                       if(count>0)
                       {
                           for(int i=0;i<count;i++)
                           {
                               int edge_size =2;
                               if(((Y_loc.at(i)-window_size)<=edge_size)||((Z_loc.at(i)-window_size)<=edge_size)||((Y_loc.at(i)+window_size)>=(ny-edge_size))||((Z_loc.at(i)+window_size)>=(nz-edge_size)))
                               {
                                   continue;
                               }
                               /* using Spherical growth method designed by Chaowang*/
                               V3DLONG adjusted_y, adjusted_z;
                               double radiu;
                               //measure_one_point_radius_by_iterative_spherical_growth(old_image_binary,nx,ny,thres_2d,X_loc.at(i),Y_loc.at(i),adjusted_x,adjusted_y,radiu);
                               find_neighborhood_maximum_radius(Y_loc.at(i),Z_loc.at(i),old_image_binary_yz,window_size,adjusted_y,adjusted_z,radiu,ny,nz,thres_2d);
                               Y_candidate.push_back(adjusted_y);  // all adjusted y coordinate of candidate points
                               Z_candidate.push_back(adjusted_z);  // all adjusted z coordinate of candidate points
                               Max_value.push_back(radiu);    // all radiu coordinate of candidate points
                               //cout<<"x is :"<<adjusted_x<<" "<<"y is :"<<adjusted_y<<" "<<"the radiu  is :"<<radiu<<endl;

                           }
                       }
                       for(V3DLONG k = 0; k < Y_candidate.size(); k++)
                       {
                           vector<float> y_loc;
                           vector<float> z_loc;
                           //create a new block_mip to detect the 2D branch points
                           int block_radiu=Max_value.at(k)+based_distance+ray_length+2;
                           int block_length=block_radiu*2+1;
                           unsigned char *block=0;
                           try{block=new unsigned char [block_length*block_length*block_length];}
                           catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}
                           //cout<<"x is :"<<X_candidate.at(k)<<" "<<" y is :"<<Y_candidate.at(k)<<" "<<"the length is "<<block_length<<endl;
                           if(((Y_candidate.at(k)+block_radiu)>(ny-1))||((Y_candidate.at(k)-block_radiu)<1)||((Z_candidate.at(k)-block_radiu)<1)||((Z_candidate.at(k)+block_radiu)>(nz-1)))
                           {
                               //enlagre the original
                               V3DLONG new_ny=ny+block_radiu;
                               V3DLONG new_nz=nz+block_radiu;
                               unsigned char *enlarged_image=0;
                               enlarged_image=new unsigned char[new_nz*new_ny];
                               V3DLONG num_datald=0;
                               for(V3DLONG iz=block_radiu;iz<new_nz-block_radiu;iz++)
                               {
                                 for(V3DLONG iy=block_radiu;iy<new_ny-block_radiu;iy++)
                                 {
                                        enlarged_image[iz*new_ny+iy]=old_image_binary_yz[num_datald];
                                        num_datald++;

                                 }
                               }

                           }
                           int num_block=0;
                           for(V3DLONG b=Z_candidate.at(k)-block_radiu;b<=Z_candidate.at(k)+block_radiu;b++)
                           {
                                for(V3DLONG c=Y_candidate.at(k)-block_radiu;c<=Y_candidate.at(k)+block_radiu;c++)
                                {
                                    unsigned char block_pixe=old_image_binary_yz[b*ny+c];
                                    block[num_block]=block_pixe;
                                    num_block++;
                                }
                            }
                           if(0)
                           {
                               delete_small_area(block_length,block_length,block);// connected domain denoising
                           }
                          // based_distance=float(based_distance*0.1)*Max_value.at(k);
                            for(int i = 0; i <ray_numbers_2d; i++)   //m is the numble of the ray
                            {
                                for(int j = Max_value.at(k)+based_distance; j < Max_value.at(k)+based_distance+ray_length; j++)    // n is the numble of the points of the each ray
                                {
                                    if((Max_value.at(k)+based_distance+2)>max_length)
                                        {
                                        v3d_msg(QString("the max_length is too small to can not adapt the model, please check the max_length"));
                                    }
                                    double pixe = project_interp_2d(block_radiu+ray_y[i][j], block_radiu+ray_x[i][j], block, block_length, block_length , block_radiu,  block_radiu);
                                    if(pixe>=100)
                                    {
                                        y_loc.push_back(block_radiu+ray_x[i][j]);
                                        z_loc.push_back(block_radiu+ray_y[i][j]);

                                    }
                                }
                            }

                           /* calculate the DBscan radiu */
                            double a=block_radiu+ray_x[0][Max_value.at(k)+based_distance+ray_length]; //x1
                            double b=block_radiu+ray_x[1][Max_value.at(k)+based_distance+ray_length]; //x2
                            double c=block_radiu+ray_y[0][Max_value.at(k)+based_distance+ray_length]; //y1
                            double d=block_radiu+ray_y[1][Max_value.at(k)+based_distance+ray_length]; //y2
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
                           mycluster.Read_from_coordiante(y_loc,z_loc,DB_radius*1.1,5);
                           int flag=mycluster.DoDBSCANRecursive();

                           if(flag==3)
                           {
                                            s_yz.y=  Y_candidate[k]+1;
                                            s_yz.z = Z_candidate[k]+1;
                                            unsigned char max_x=0;
                                            int loc_x;
                                               for(int kk=size_x;kk<size_x+num_layer;kk++)
                                               {
                                                   unsigned char sum_pixe;
                                                   sum_pixe=datald[Z_candidate[k]*nx*ny+Y_candidate[k]*nx+kk]+datald[(Z_candidate[k]+1)*nx*ny+Y_candidate[k]*nx+kk]+datald[(Z_candidate[k]-1)*nx*ny+Y_candidate[k]*nx+kk]+
                                                           datald[(Z_candidate[k])*nx*ny+(Y_candidate[k]-1)*nx+kk]+datald[(Z_candidate[k]+1)*nx*ny+(Y_candidate[k]-1)*nx+kk]+datald[(Z_candidate[k]-1)*nx*ny+(Y_candidate[k]-1)*nx+kk]+
                                                           datald[(Z_candidate[k])*nx*ny+(Y_candidate[k]+1)*nx+kk]+datald[(Z_candidate[k]+1)*nx*ny+(Y_candidate[k]+1)*nx+kk]+datald[(Z_candidate[k]-1)*nx*ny+(Y_candidate[k]+1)*nx+kk];
                                                   if(sum_pixe>max_x)
                                                   {
                                                       max_x=sum_pixe;
                                                       loc_x=kk;
                                                   }
                                               }
                                            s_yz.x = loc_x;
                                            s_yz.radius = 1;
                                            s_yz.color = blue;
                                            curlist_yz<<s_yz;
                           }

                       }
                    }
                }
              }

              if(case3)
              {
                cout<<"begin to detect juntion point of each yz project"<<endl;

                for(int size_y=num_layer;size_y<ny-num_layer;size_y+=MAX(1,num_layer/2))
                {
                    if((size_y+num_layer)<ny)
                    {
                       mip_layer_xz(nx,ny,nz,size_y,datald,image_mip_xz,num_layer);
                       thres_segment(nx*nz,image_mip_xz,image_binary_xz,thres_2d);
                       for(V3DLONG a=0;a<nx*nz;a++)
                       {
                           old_image_binary_xz[a]=image_binary_xz[a]; // save the original image;
                       }
                       skeletonization(nx,nz,image_binary_xz);
                       vector<V3DLONG> X_candidate;  // save the adjusted x coordinate;
                       vector<V3DLONG> Z_candidate;  // save the adjusted z coordinate;
                       vector<float> Max_value;      // save the radiu;
                       int count=0; // the number of candidate
                       vector<V3DLONG> X_loc; //save the non-adjusted x coordinate;
                       vector<V3DLONG> Z_loc; //save the non-adjusted z coordinate;
                       seek_2D_candidate_points(nx,nz,image_binary_xz,X_loc,Z_loc,count);
                       int window_size=2;
                       if(count>0)
                       {
                           for(int i=0;i<count;i++)
                           {
                               int edge_size =10;
                               if(((X_loc.at(i)-window_size)<=edge_size)||((Z_loc.at(i)-window_size)<=edge_size)||((X_loc.at(i)+window_size)>=(nx-edge_size))||((Z_loc.at(i)+window_size)>=(nz-edge_size)))
                               {
                                   continue;
                               }
                               /* using Spherical growth method designed by Chaowang*/
                               V3DLONG adjusted_x, adjusted_z;
                               double radiu;
                               //measure_one_point_radius_by_iterative_spherical_growth(old_image_binary,nx,ny,thres_2d,X_loc.at(i),Y_loc.at(i),adjusted_x,adjusted_y,radiu);
                               find_neighborhood_maximum_radius(X_loc.at(i),Z_loc.at(i),old_image_binary_xz,window_size,adjusted_x,adjusted_z,radiu,nx,nz,thres_2d);
                               X_candidate.push_back(adjusted_x);  // all adjusted x coordinate of candidate points
                               Z_candidate.push_back(adjusted_z);  // all adjusted z coordinate of candidate points
                               Max_value.push_back(radiu);    // all radiu coordinate of candidate points
                               //cout<<"x is :"<<adjusted_x<<" "<<"y is :"<<adjusted_y<<" "<<"the radiu  is :"<<radiu<<endl;

                           }
                       }

                       for(V3DLONG k = 0; k < X_candidate.size(); k++)
                       {
                           vector<float> x_loc;
                           vector<float> z_loc;
                           //create a new block_mip to detect the 2D branch points
                           int block_radiu=Max_value.at(k)+based_distance+ray_length+2;
                           int block_length=block_radiu*2+1;
                           unsigned char *block=0;
                           try{block=new unsigned char [block_length*block_length*block_length];}
                           catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}
                           //cout<<"x is :"<<X_candidate.at(k)<<" "<<" y is :"<<Y_candidate.at(k)<<" "<<"the length is "<<block_length<<endl;
                           if(((X_candidate.at(k)+block_radiu)>(nx-1))||((X_candidate.at(k)-block_radiu)<1)||((Z_candidate.at(k)-block_radiu)<1)||((Z_candidate.at(k)+block_radiu)>(nz-1)))
                           {
                               //enlagre the original
                               V3DLONG new_nx=nx+block_radiu;
                               V3DLONG new_nz=nz+block_radiu;
                               unsigned char *enlarged_image=0;
                               enlarged_image=new unsigned char[new_nz*new_nx];
                               V3DLONG num_datald=0;
                               for(V3DLONG iz=block_radiu;iz<new_nz-block_radiu;iz++)
                               {
                                 for(V3DLONG ix=block_radiu;ix<new_nx-block_radiu;ix++)
                                 {
                                        enlarged_image[iz*new_nx+ix]=old_image_binary_yz[num_datald];
                                        num_datald++;

                                 }
                               }
                           }
                           int num_block=0;
                           for(V3DLONG b=Z_candidate.at(k)-block_radiu;b<=Z_candidate.at(k)+block_radiu;b++)
                           {
                                for(V3DLONG c=X_candidate.at(k)-block_radiu;c<=X_candidate.at(k)+block_radiu;c++)
                                {
                                    unsigned char block_pixe=old_image_binary_xz[b*nx+c];
                                    block[num_block]=block_pixe;
                                    num_block++;
                                }
                            }
                           if(0)
                           {
                               delete_small_area(block_length,block_length,block);// connected domain denoising
                           }
                          // based_distance=float(based_distance*0.1)*Max_value.at(k);
                            for(int i = 0; i <ray_numbers_2d; i++)   //m is the numble of the ray
                            {
                                for(int j = Max_value.at(k)+based_distance; j < Max_value.at(k)+based_distance+ray_length; j++)    // n is the numble of the points of the each ray
                                {
                                    if((Max_value.at(k)+based_distance+2)>max_length)
                                        {
                                        v3d_msg(QString("the max_length is too small to can not adapt the model, please check the max_length"));
                                    }
                                    double pixe = project_interp_2d(block_radiu+ray_y[i][j], block_radiu+ray_x[i][j], block, block_length, block_length , block_radiu,  block_radiu);
                                    if(pixe>=100)
                                    {
                                        x_loc.push_back(block_radiu+ray_x[i][j]);
                                        z_loc.push_back(block_radiu+ray_y[i][j]);

                                    }
                                }
                            }

                           /* calculate the DBscan radiu */
                            double a=block_radiu+ray_x[0][Max_value.at(k)+based_distance+ray_length]; //x1
                            double b=block_radiu+ray_x[1][Max_value.at(k)+based_distance+ray_length]; //x2
                            double c=block_radiu+ray_y[0][Max_value.at(k)+based_distance+ray_length]; //y1
                            double d=block_radiu+ray_y[1][Max_value.at(k)+based_distance+ray_length]; //y2
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
                           mycluster.Read_from_coordiante(x_loc,z_loc,DB_radius*1.1,5);
                           int flag=mycluster.DoDBSCANRecursive();

                           if(flag==3)
                           {
                                            s_xz.x=  X_candidate[k]+1;
                                            s_xz.z = Z_candidate[k]+1;
                                            unsigned char max_x=0;
                                            int loc_y;
                                               for(int kk=size_y;kk<size_y+num_layer;kk++)
                                               {
                                                   unsigned char sum_pixe;
                                                   sum_pixe=datald[Z_candidate[k]*nx*ny+kk*nx+X_candidate[k]]+datald[Z_candidate[k]*nx*ny+kk*nx+X_candidate[k]+1]+datald[Z_candidate[k]*nx*ny+kk*nx+X_candidate[k]-1]+
                                                           datald[(Z_candidate[k]+1)*nx*ny+kk*nx+X_candidate[k]]+datald[(Z_candidate[k]+1)*nx*ny+kk*nx+X_candidate[k]+1]+datald[(Z_candidate[k]+1)*nx*ny+kk*nx+X_candidate[k]-1]+
                                                           datald[(Z_candidate[k]-1)*nx*ny+kk*nx+X_candidate[k]]+datald[(Z_candidate[k]-1)*nx*ny+kk*nx+X_candidate[k]+1]+datald[(Z_candidate[k]-1)*nx*ny+kk*nx+X_candidate[k]-1];
                                                   if(sum_pixe>max_x)
                                                   {
                                                       max_x=sum_pixe;
                                                       loc_y=kk;
                                                   }
                                               }
                                            s_xz.y = loc_y;
                                            s_xz.radius = 1;
                                            s_xz.color = green;
                                            curlist_xz<<s_xz;
                           }

                       }
                    }
                }
              }

              if(case1)
               {
                cout<<"begin to detect juntion point of each xy project"<<endl;
                for(int size_z=num_layer;size_z<nz-num_layer;size_z+=MAX(1,num_layer/2))
                {

                    /* detect the 2D branch points in each MIP */
                    if((size_z+num_layer)<nz)
                    {
                       mip(nx,ny,size_z,datald,image_mip,num_layer);
                       thres_segment(nx*ny,image_mip,image_binary,thres_2d);
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
                       int window_size=2;
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
                               //measure_one_point_radius_by_iterative_spherical_growth(old_image_binary,nx,ny,thres_2d,X_loc.at(i),Y_loc.at(i),adjusted_x,adjusted_y,radiu);
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
                           int block_radiu=Max_value.at(k)+based_distance+ray_length+2;
                           int block_length=block_radiu*2+1;
                           unsigned char *block=0;
                           try{block=new unsigned char [block_length*block_length*block_length];}
                           catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}
                           //cout<<"x is :"<<X_candidate.at(k)<<" "<<" y is :"<<Y_candidate.at(k)<<" "<<"the length is "<<block_length<<endl;
                           if(((X_candidate.at(k)+block_radiu)>(nx-1))||((X_candidate.at(k)-block_radiu)<1)||((Y_candidate.at(k)-block_radiu)<1)||((Y_candidate.at(k)+block_radiu)>(ny-1)))
                           {
                               //enlagre the original
                               V3DLONG new_nx=nx+block_radiu;
                               V3DLONG new_ny=ny+block_radiu;
                               unsigned char *enlarged_image=0;
                               enlarged_image=new unsigned char[new_nx*new_ny];
                               V3DLONG num_datald=0;
                               for(V3DLONG iy=block_radiu;iy<new_ny-block_radiu;iy++)
                               {
                                 for(V3DLONG ix=block_radiu;ix<new_ny-block_radiu;ix++)
                                 {
                                        enlarged_image[iy*new_nx+ix]=old_image_binary[num_datald];
                                        num_datald++;

                                 }
                               }

                              // continue;
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
                           if(0)
                           {
                               delete_small_area(block_length,block_length,block);// connected domain denoising
                           }
                          // based_distance=float(based_distance*0.1)*Max_value.at(k);
                            for(int i = 0; i <ray_numbers_2d; i++)   //m is the numble of the ray
                            {
                                for(int j = Max_value.at(k)+based_distance; j < Max_value.at(k)+based_distance+ray_length; j++)    // n is the numble of the points of the each ray
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
                            double a=block_radiu+ray_x[0][Max_value.at(k)+based_distance+ray_length]; //x1
                            double b=block_radiu+ray_x[1][Max_value.at(k)+based_distance+ray_length]; //x2
                            double c=block_radiu+ray_y[0][Max_value.at(k)+based_distance+ray_length]; //y1
                            double d=block_radiu+ray_y[1][Max_value.at(k)+based_distance+ray_length]; //y2
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
                           mycluster.Read_from_coordiante(x_loc,y_loc,DB_radius*1.1,5);
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
                                            s.z = loc_z;
                                            s.radius = 1;
                                            s.color = red;
                                            curlist<<s;
                           }

                       }
                    }
                }

                int p;
                if(QMessageBox::Yes == QMessageBox::question (0, "", QString(" yes: delete the soma neiborhood junction; no : NULL"), QMessageBox::Yes, QMessageBox::No))    p = 1;
                if(p==1)
                {
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

                }
               }
              for(V3DLONG i = 0; i < curlist_yz.size(); i++)
              {
                  curlist.append(curlist_yz[i]);
              }
              for(V3DLONG i = 0; i < curlist_xz.size(); i++)
              {
                  curlist.append(curlist_xz[i]);
              }
              cout<<"new curlist_xy.size:"<<curlist.size()<<endl;

              bool flag_while = true;
              while(flag_while)
              {
                  bool endwhile = true;
                  for(V3DLONG i=0;i<curlist.size();i++)
                  {
                      for(V3DLONG j=i+1;j<curlist.size();j++)
                      {
                          if(square(curlist[j].x-curlist[i].x)+square(curlist[j].y-curlist[i].y)+square(curlist[j].z-curlist[i].z)<clu_thres)
                          {
                              curlist[i].x=(curlist[j].x+curlist[i].x)/2;
                              curlist[i].y=(curlist[j].y+curlist[i].y)/2;
                              curlist[i].z=(curlist[j].z+curlist[i].z)/2;
                              curlist[i].color=blue;
                              curlist.removeAt(j);
                              j=j-1;
                              endwhile = false;
                          }
                      }
                  }
                  if(endwhile){flag_while = false;}
              }
              callback.setLandmark(curwin, curlist);
                delete []image_mip;
                delete []image_binary;
                delete []old_image_binary;
                delete []image_mip_yz;
                delete []image_binary_yz;
                delete []old_image_binary_yz;
                delete []image_mip_xz;
                delete []image_binary_xz;
                delete []old_image_binary_xz;
                return 1;

}
