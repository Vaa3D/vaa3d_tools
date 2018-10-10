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
struct delete_piont
{
    V3DLONG xx;
    V3DLONG yy;
};

Q_EXPORT_PLUGIN2(Branch_detection, TestPlugin)
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
        <<tr("3D_Branch_points_detection")
        <<tr("3D_thin_branch_points_detection")
        <<tr("3D_Branch_points_detection_single_plane")
        <<tr("3D_thin_branch_points_detection_single_plane");
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
       int flag=Branch_Point_Detection_new(callback,parent);
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
    else if(menu_name==tr("3D_thin_branch_points_detection"))
    {
        int flag=Thin_branch_points_detection(callback,parent);
        if (flag==1)
        {
           v3d_msg(tr("branch points detection completed!"));
        }
    }
    else if(menu_name==tr("3D_thin_branch_points_detection_single_plane"))
    {
        int flag=Thin_branch_points_detection_single(callback,parent);
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
int Branch_Point_Detection_new(V3DPluginCallback2 &callback, QWidget *parent)
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
            int thres_2d = 25;
            int ray_length_2d = 8;
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
       int neighbor[8];
       int sum_points;
       int new_neighbor[8];
       vector<delete_piont> delete_list;
       int sum_flag=0;

       unsigned char *xy_mip_datald;
       try{xy_mip_datald=new unsigned char [nx*ny];}
       catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}
       unsigned char *xy_binar_datald;
       try{xy_binar_datald=new unsigned char [nx*ny];}
       catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}
       XY_mip(nx,ny,nz,datald,xy_mip_datald);
       thres_segment(nx*ny,xy_mip_datald,xy_binar_datald,thres_2d);



       unsigned char *yz_mip_datald;
       try{yz_mip_datald=new unsigned char [ny*nz];}
       catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}
       unsigned char *yz_binar_datald;
       try{yz_binar_datald=new unsigned char [ny*nz];}
       catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}
       YZ_mip(nx,ny,nz,datald,yz_mip_datald);
       thres_segment(nz*ny,yz_mip_datald,yz_binar_datald,thres_2d);

       unsigned char *xz_mip_datald;
       try{xz_mip_datald=new unsigned char [nx*nz];}
       catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}
       unsigned char *xz_binar_datald;
       try{xz_binar_datald=new unsigned char [nx*nz];}
       catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}
       XZ_mip(nx,ny,nz,datald,xz_mip_datald);
       thres_segment(nx*nz,xz_mip_datald,xz_binar_datald,thres_2d);



       unsigned char *block;
       try{block=new unsigned char [size_block];}
       catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

       unsigned char *project;
       try{project=new unsigned char [size_plane];}
       catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

       unsigned char *seg_project;
       try{seg_project=new unsigned char [size_plane];}
       catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}
       for(int swith=0;swith<=2;swith++)
       {
           for(V3DLONG j=radiu_block;j<nz-radiu_block;j++)
           {
                for(V3DLONG i=radiu_block;i<ny-radiu_block;i++)
                {
                    for(V3DLONG k=radiu_block;k<nx-radiu_block;k++)
                    {
                        sum_flag=0;
                        V3DLONG num_block=0;
                        unsigned char pixe=p4DImage->getValueUINT8(k,i,j,0);
                        if(pixe>thres_2d)
                        {
                            for(V3DLONG a=j-radiu_block;a<=j+radiu_block;a++)
                            {
                                 V3DLONG z_location=a*nx*ny;
                                 for(V3DLONG b=i-radiu_block;b<=i+radiu_block;b++)
                                 {
                                     V3DLONG y_location=b*nx;
                                     for(V3DLONG c=k-radiu_block;c<=k+radiu_block;c++)
                                     {
                                           //v3d_msg(QString("pixe is %1").arg(datald[z_location+y_location+c]));
                                           block[num_block]=datald[z_location+y_location+c];
                                           num_block++;

                                     }
                                 }
                            }
//                            v3d_msg(QString("create block success"));
                            if(swith==0) //select xy_plane
                            {
                                XY_mip(length_block,length_block,length_block,block,project);
                                thres_segment(size_plane,project,seg_project,thres_2d);
                                while(true)
                                {
                                    for(int j2=1;j2<length_block;j2++)
                                    {
                                        for(int i2=1;i2<length_block;i2++)
                                        {
                                            if(seg_project[j2*length_block+i2]>0)
                                            {
                                                if (seg_project[(j2-1)*length_block+i2] == 255) neighbor[1] = 1;
                                                else  neighbor[1] = 0;
                                                if (seg_project[(j2-1)*length_block+i2+1] == 255) neighbor[2] = 1;
                                                else  neighbor[2] = 0;
                                                if (seg_project[j2*length_block+i2+1] == 255) neighbor[3] = 1;
                                                else  neighbor[3] = 0;
                                                if (seg_project[(j2+1)*length_block+i2+1] == 255) neighbor[4] = 1;
                                                else  neighbor[4] = 0;
                                                if (seg_project[(j2+1)*length_block+i2] == 255) neighbor[5] = 1;
                                                else  neighbor[5] = 0;
                                                if (seg_project[(j2+1)*length_block+i2-1] == 255) neighbor[6] = 1;
                                                else  neighbor[6] = 0;
                                                if (seg_project[j2*length_block+i2-1] == 255) neighbor[7] = 1;
                                                else  neighbor[7] = 0;
                                                if (seg_project[(j2-1)*length_block+i2-1] == 255) neighbor[8] = 1;
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
                                                        if (((neighbor[1] * neighbor[3] * neighbor[5]) == 0) && ((neighbor[3] * neighbor[5] * neighbor[7]) == 0))
                                                        {
                                                            delete_piont pp;
                                                            pp.xx=i2;
                                                            pp.yy=j2;
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
                                      seg_project[delete_list[num].xx+delete_list[num].yy*length_block]=0;
                                    }
                                    delete_list.clear();


                                    for(V3DLONG j2=1;j2<length_block;j2++)
                                    {
                                        for(V3DLONG i2=1;i2<length_block;i2++)
                                        {
                                            if(seg_project[j2*length_block+i2]>0)
                                            {
                                                if (seg_project[(j2-1)*length_block+i2] == 255) neighbor[1] = 1;
                                                else  neighbor[1] = 0;
                                                if (seg_project[(j2-1)*length_block+i2 + 1] == 255) neighbor[2] = 1;
                                                else  neighbor[2] = 0;
                                                if (seg_project[j2*length_block+i2 + 1] == 255) neighbor[3] = 1;
                                                else  neighbor[3] = 0;
                                                if (seg_project[(j2+1)*length_block+i2 + 1] == 255) neighbor[4] = 1;
                                                else  neighbor[4] = 0;
                                                if (seg_project[(j2+1)*length_block+i2] == 255) neighbor[5] = 1;
                                                else  neighbor[5] = 0;
                                                if (seg_project[(j2+1)*length_block+i2 - 1] == 255) neighbor[6] = 1;
                                                else  neighbor[6] = 0;
                                                if (seg_project[j2*length_block+i2 - 1] == 255) neighbor[7] = 1;
                                                else  neighbor[7] = 0;
                                                if (seg_project[(j2-1)*length_block+i2 - 1] == 255) neighbor[8] = 1;
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
                                                            pp.xx=i2;
                                                            pp.yy=j2;
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
                                      seg_project[delete_list[num].xx+delete_list[num].yy*length_block]=0;
                                    }
                                    delete_list.clear();
                                }

                                for(int j1=1;j1<length_block-1;j1++)
                                {
                                    for(int i1=1;i1<length_block-1;i1++)
                                    {
                                        if(seg_project[j1*length_block+i1]>0)
                                        {
                                            if (seg_project[(j1-1)*length_block+i1-1] == 255) new_neighbor[1] = 1;
                                            else  new_neighbor[1] = 0;
                                            if (seg_project[(j1-1)*length_block+i1] == 255) new_neighbor[2] = 1;
                                            else  new_neighbor[2] = 0;
                                            if (seg_project[(j1-1)*length_block+i1+1] == 255) new_neighbor[3] = 1;
                                            else  new_neighbor[3] = 0;
                                            if (seg_project[(j1)*length_block+i1 + 1] == 255) new_neighbor[4] = 1;
                                            else  new_neighbor[4] = 0;
                                            if (seg_project[(j1+1)*length_block+i1+1] == 255) new_neighbor[5] = 1;
                                            else  new_neighbor[5] = 0;
                                            if (seg_project[(j1+1)*length_block+i1] == 255) new_neighbor[6] = 1;
                                            else  new_neighbor[6] = 0;
                                            if (seg_project[(j1+1)*length_block+i1-1] == 255) new_neighbor[7] = 1;
                                            else  new_neighbor[7] = 0;
                                            if (seg_project[(j1)*length_block+i1-1] == 255) new_neighbor[8] = 1;
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
                                                if(((i1==(radiu_block+1))||(i1==(radiu_block-1))||(i1==(radiu_block)))&&((j1==(radiu_block+1))||(j1==(radiu_block-1))||(j1==(radiu_block))))
                                                {
                                                    int xy_flag8=rayinten_2D(i,k,ray_numbers_2d ,ray_length_2d,ray_x, ray_y, xy_binar_datald,nx,ny);
//                                                    v3d_msg(QString("release this plug_in"));
//                                                    int xy_flag10=rayinten_2D((radiu_block+1),(radiu_block+1),ray_numbers_2d ,(ray_length_2d+2),ray_x, ray_y, seg_project,length_block,length_block);
//                                                    int xy_flag12=rayinten_2D((radiu_block+1),(radiu_block+1),ray_numbers_2d ,(ray_length_2d+4),ray_x, ray_y, seg_project,length_block,length_block);
//                                                    int xy_flag14=rayinten_2D((radiu_block+1),(radiu_block+1),ray_numbers_2d ,(ray_length_2d+6),ray_x, ray_y, seg_project,length_block,length_block);
//                                                    int xy_flag16=rayinten_2D((radiu_block+1),(radiu_block+1),ray_numbers_2d ,(ray_length_2d+8),ray_x, ray_y, seg_project,length_block,length_block);
                                                    int ray_flag=xy_flag8;
                                                    if(ray_flag>=1)
                                                    {
                                                        sum_flag=sum_flag+1;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }

                            }
                            if(swith==1) //select xz_plane
                            {
                                XZ_mip(length_block,length_block,length_block,block,project);
                                thres_segment(size_plane,project,seg_project,thres_2d);
                                while(true)
                                {
                                    for(int j2=1;j2<length_block;j2++)
                                    {
                                        for(int i2=1;i2<length_block;i2++)
                                        {
                                            if(seg_project[j2*length_block+i2]>0)
                                            {
                                                if (seg_project[(j2-1)*length_block+i2] == 255) neighbor[1] = 1;
                                                else  neighbor[1] = 0;
                                                if (seg_project[(j2-1)*length_block+i2+1] == 255) neighbor[2] = 1;
                                                else  neighbor[2] = 0;
                                                if (seg_project[j2*length_block+i2+1] == 255) neighbor[3] = 1;
                                                else  neighbor[3] = 0;
                                                if (seg_project[(j2+1)*length_block+i2+1] == 255) neighbor[4] = 1;
                                                else  neighbor[4] = 0;
                                                if (seg_project[(j2+1)*length_block+i2] == 255) neighbor[5] = 1;
                                                else  neighbor[5] = 0;
                                                if (seg_project[(j2+1)*length_block+i2-1] == 255) neighbor[6] = 1;
                                                else  neighbor[6] = 0;
                                                if (seg_project[j2*length_block+i2-1] == 255) neighbor[7] = 1;
                                                else  neighbor[7] = 0;
                                                if (seg_project[(j2-1)*length_block+i2-1] == 255) neighbor[8] = 1;
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
                                                        if (((neighbor[1] * neighbor[3] * neighbor[5]) == 0) && ((neighbor[3] * neighbor[5] * neighbor[7]) == 0))
                                                        {
                                                            delete_piont pp;
                                                            pp.xx=i2;
                                                            pp.yy=j2;
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
                                      seg_project[delete_list[num].xx+delete_list[num].yy*length_block]=0;
                                    }
                                    delete_list.clear();


                                    for(V3DLONG j2=1;j2<length_block;j2++)
                                    {
                                        for(V3DLONG i2=1;i2<length_block;i2++)
                                        {
                                            if(seg_project[j2*length_block+i2]>0)
                                            {
                                                if (seg_project[(j2-1)*length_block+i2] == 255) neighbor[1] = 1;
                                                else  neighbor[1] = 0;
                                                if (seg_project[(j2-1)*length_block+i2 + 1] == 255) neighbor[2] = 1;
                                                else  neighbor[2] = 0;
                                                if (seg_project[j2*length_block+i2 + 1] == 255) neighbor[3] = 1;
                                                else  neighbor[3] = 0;
                                                if (seg_project[(j2+1)*length_block+i2 + 1] == 255) neighbor[4] = 1;
                                                else  neighbor[4] = 0;
                                                if (seg_project[(j2+1)*length_block+i2] == 255) neighbor[5] = 1;
                                                else  neighbor[5] = 0;
                                                if (seg_project[(j2+1)*length_block+i2 - 1] == 255) neighbor[6] = 1;
                                                else  neighbor[6] = 0;
                                                if (seg_project[j2*length_block+i2 - 1] == 255) neighbor[7] = 1;
                                                else  neighbor[7] = 0;
                                                if (seg_project[(j2-1)*length_block+i2 - 1] == 255) neighbor[8] = 1;
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
                                                            pp.xx=i2;
                                                            pp.yy=j2;
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
                                      seg_project[delete_list[num].xx+delete_list[num].yy*length_block]=0;
                                    }
                                    delete_list.clear();
                                }

                                // find the candidate points
                                for(int j1=1;j1<length_block-1;j1++)
                                {
                                    for(int i1=1;i1<length_block-1;i1++)
                                    {
                                        if(seg_project[j1*length_block+i1]>0)
                                        {
                                            if (seg_project[(j1-1)*length_block+i1-1] == 255) new_neighbor[1] = 1;
                                            else  new_neighbor[1] = 0;
                                            if (seg_project[(j1-1)*length_block+i1] == 255) new_neighbor[2] = 1;
                                            else  new_neighbor[2] = 0;
                                            if (seg_project[(j1-1)*length_block+i1+1] == 255) new_neighbor[3] = 1;
                                            else  new_neighbor[3] = 0;
                                            if (seg_project[(j1)*length_block+i1 + 1] == 255) new_neighbor[4] = 1;
                                            else  new_neighbor[4] = 0;
                                            if (seg_project[(j1+1)*length_block+i1+1] == 255) new_neighbor[5] = 1;
                                            else  new_neighbor[5] = 0;
                                            if (seg_project[(j1+1)*length_block+i1] == 255) new_neighbor[6] = 1;
                                            else  new_neighbor[6] = 0;
                                            if (seg_project[(j1+1)*length_block+i1-1] == 255) new_neighbor[7] = 1;
                                            else  new_neighbor[7] = 0;
                                            if (seg_project[(j1)*length_block+i1-1] == 255) new_neighbor[8] = 1;
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
                                                if(((i1==(radiu_block+1))||(i1==(radiu_block-1))||(i1==(radiu_block)))&&((j1==(radiu_block+1))||(j1==(radiu_block-1))||(j1==(radiu_block))))
                                                {
//                                                    v3d_msg(QString("release this plug_in"));
                                                    int xy_flag8=rayinten_2D(j,k,ray_numbers_2d ,ray_length_2d,ray_x, ray_y, xz_binar_datald,nx,nz);
//                                                    int xy_flag10=rayinten_2D((radiu_block+1),(radiu_block+1),ray_numbers_2d ,(ray_length_2d+2),ray_x, ray_y, seg_project,2*radiu_block,2*radiu_block);
//                                                    int xy_flag12=rayinten_2D((radiu_block+1),(radiu_block+1),ray_numbers_2d ,(ray_length_2d+4),ray_x, ray_y, seg_project,2*radiu_block,2*radiu_block);
//                                                    int xy_flag14=rayinten_2D((radiu_block+1),(radiu_block+1),ray_numbers_2d ,(ray_length_2d+6),ray_x, ray_y, seg_project,2*radiu_block,2*radiu_block);
//                                                    int xy_flag16=rayinten_2D((radiu_block+1),(radiu_block+1),ray_numbers_2d ,(ray_length_2d+8),ray_x, ray_y, seg_project,2*radiu_block,2*radiu_block);
                                                    int ray_flag=xy_flag8;
                                                    if(ray_flag>=1)
                                                    {
                                                        sum_flag=sum_flag+1;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            if(swith==2) //select yz_plane
                            {
                                YZ_mip(length_block,length_block,length_block,block,project);
                                thres_segment(size_plane,project,seg_project,thres_2d);
                                while(true)
                                {
                                    for(int j2=1;j2<length_block;j2++)
                                    {
                                        for(int i2=1;i2<length_block;i2++)
                                        {
                                            if(seg_project[j2*length_block+i2]>0)
                                            {
                                                if (seg_project[(j2-1)*length_block+i2] == 255) neighbor[1] = 1;
                                                else  neighbor[1] = 0;
                                                if (seg_project[(j2-1)*length_block+i2+1] == 255) neighbor[2] = 1;
                                                else  neighbor[2] = 0;
                                                if (seg_project[j2*length_block+i2+1] == 255) neighbor[3] = 1;
                                                else  neighbor[3] = 0;
                                                if (seg_project[(j2+1)*length_block+i2+1] == 255) neighbor[4] = 1;
                                                else  neighbor[4] = 0;
                                                if (seg_project[(j2+1)*length_block+i2] == 255) neighbor[5] = 1;
                                                else  neighbor[5] = 0;
                                                if (seg_project[(j2+1)*length_block+i2-1] == 255) neighbor[6] = 1;
                                                else  neighbor[6] = 0;
                                                if (seg_project[j2*length_block+i2-1] == 255) neighbor[7] = 1;
                                                else  neighbor[7] = 0;
                                                if (seg_project[(j2-1)*length_block+i2-1] == 255) neighbor[8] = 1;
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
                                                        if (((neighbor[1] * neighbor[3] * neighbor[5]) == 0) && ((neighbor[3] * neighbor[5] * neighbor[7]) == 0))
                                                        {
                                                            delete_piont pp;
                                                            pp.xx=i2;
                                                            pp.yy=j2;
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
                                      seg_project[delete_list[num].xx+delete_list[num].yy*length_block]=0;
                                    }
                                    delete_list.clear();


                                    for(V3DLONG j2=1;j2<length_block;j2++)
                                    {
                                        for(V3DLONG i2=1;i2<length_block;i2++)
                                        {
                                            if(seg_project[j2*length_block+i2]>0)
                                            {
                                                if (seg_project[(j2-1)*length_block+i2] == 255) neighbor[1] = 1;
                                                else  neighbor[1] = 0;
                                                if (seg_project[(j2-1)*length_block+i2 + 1] == 255) neighbor[2] = 1;
                                                else  neighbor[2] = 0;
                                                if (seg_project[j2*length_block+i2 + 1] == 255) neighbor[3] = 1;
                                                else  neighbor[3] = 0;
                                                if (seg_project[(j2+1)*length_block+i2 + 1] == 255) neighbor[4] = 1;
                                                else  neighbor[4] = 0;
                                                if (seg_project[(j2+1)*length_block+i2] == 255) neighbor[5] = 1;
                                                else  neighbor[5] = 0;
                                                if (seg_project[(j2+1)*length_block+i2 - 1] == 255) neighbor[6] = 1;
                                                else  neighbor[6] = 0;
                                                if (seg_project[j2*length_block+i2 - 1] == 255) neighbor[7] = 1;
                                                else  neighbor[7] = 0;
                                                if (seg_project[(j2-1)*length_block+i2 - 1] == 255) neighbor[8] = 1;
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
                                                            pp.xx=i2;
                                                            pp.yy=j2;
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
                                      seg_project[delete_list[num].xx+delete_list[num].yy*length_block]=0;
                                    }
                                    delete_list.clear();
                                }

                                for(int j1=1;j1<length_block-1;j1++)
                                {
                                    for(int i1=1;i1<length_block-1;i1++)
                                    {
                                        if(seg_project[j1*length_block+i1]>0)
                                        {
                                            if (seg_project[(j1-1)*length_block+i1-1] == 255) new_neighbor[1] = 1;
                                            else  new_neighbor[1] = 0;
                                            if (seg_project[(j1-1)*length_block+i1] == 255) new_neighbor[2] = 1;
                                            else  new_neighbor[2] = 0;
                                            if (seg_project[(j1-1)*length_block+i1+1] == 255) new_neighbor[3] = 1;
                                            else  new_neighbor[3] = 0;
                                            if (seg_project[(j1)*length_block+i1 + 1] == 255) new_neighbor[4] = 1;
                                            else  new_neighbor[4] = 0;
                                            if (seg_project[(j1+1)*length_block+i1+1] == 255) new_neighbor[5] = 1;
                                            else  new_neighbor[5] = 0;
                                            if (seg_project[(j1+1)*length_block+i1] == 255) new_neighbor[6] = 1;
                                            else  new_neighbor[6] = 0;
                                            if (seg_project[(j1+1)*length_block+i1-1] == 255) new_neighbor[7] = 1;
                                            else  new_neighbor[7] = 0;
                                            if (seg_project[(j1)*length_block+i1-1] == 255) new_neighbor[8] = 1;
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
                                                if(((i1==(radiu_block+1))||(i1==(radiu_block-1))||(i1==(radiu_block)))&&((j1==(radiu_block+1))||(j1==(radiu_block-1))||(j1==(radiu_block))))
                                                {
                                                    // add largr multiscale ray-shooting model
                                                   int xy_flag8=rayinten_2D(j,i,ray_numbers_2d ,ray_length_2d,ray_x, ray_y, yz_binar_datald,ny,nz);
//                                                   int xy_flag10=rayinten_2D((radiu_block+1),(radiu_block+1),ray_numbers_2d ,(ray_length_2d+2),ray_x, ray_y, seg_project,2*radiu_block,2*radiu_block);
//                                                   int xy_flag12=rayinten_2D((radiu_block+1),(radiu_block+1),ray_numbers_2d ,(ray_length_2d+4),ray_x, ray_y, seg_project,2*radiu_block,2*radiu_block);
//                                                   int xy_flag14=rayinten_2D((radiu_block+1),(radiu_block+1),ray_numbers_2d ,(ray_length_2d+6),ray_x, ray_y, seg_project,2*radiu_block,2*radiu_block);
//                                                   int xy_flag16=rayinten_2D((radiu_block+1),(radiu_block+1),ray_numbers_2d ,(ray_length_2d+8),ray_x, ray_y, seg_project,2*radiu_block,2*radiu_block);
                                                   int ray_flag=xy_flag8;
                                                   if(ray_flag>=1)
                                                   {
                                                       sum_flag=sum_flag+1;
                                                   }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            if(sum_flag>=1)
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
        if(project) {delete []project; project = 0;}
        if(seg_project) {delete []seg_project; seg_project = 0;}
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
            int ray_numbers_2d = 64;
            int thres_2d =25;
            int ray_length_2d = 8;
            int radiu_block=8;


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
        int neighbor[8];
        int sum_points;
        int new_neighbor[8];
        vector<delete_piont> delete_list;
        V3DLONG spage=nx*ny*nz;

        unsigned char *mip_datald;
        try{mip_datald=new unsigned char [spage];}
        catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

        unsigned char *binar_datald;
        try{binar_datald=new unsigned char [spage];}
        catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

        XY_mip(nx,ny,nz,datald,mip_datald);
        thres_segment(nx*ny,mip_datald,binar_datald,thres_2d);

        unsigned char *block;
        try{block=new unsigned char [size_block];}
        catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

        unsigned char *xy_project;
        try{xy_project=new unsigned char [size_block];}
        catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

        unsigned char *seg_xy_project;
        try{seg_xy_project=new unsigned char [size_block];}
        catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

        for(V3DLONG j=radiu_block;j<nz-radiu_block;j++)
        {
           for(V3DLONG i=length_block;i<ny-length_block;i++)
           {
               for(V3DLONG k=length_block;k<nx-length_block;k++)
               {
                   unsigned char pixe=p4DImage->getValueUINT8(k,i,j,0);
                   V3DLONG num_block=0;
                   if(pixe>thres_2d)
                   {
//                    v3d_msg(QString("pixe is %1").arg(pixe));
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

                       XY_mip(length_block,length_block,length_block,block,xy_project);
                       thres_segment(size_plane,xy_project,seg_xy_project,thres_2d);


                       //thined the block
                       while(true)
                       {
                           for(int j2=1;j2<length_block;j2++)
                           {
                               for(int i2=1;i2<length_block;i2++)
                               {
                                   if(seg_xy_project[j2*length_block+i2]>0)
                                   {
                                       if (seg_xy_project[(j2-1)*length_block+i2] == 255) neighbor[1] = 1;
                                       else  neighbor[1] = 0;
                                       if (seg_xy_project[(j2-1)*length_block+i2+1] == 255) neighbor[2] = 1;
                                       else  neighbor[2] = 0;
                                       if (seg_xy_project[j2*length_block+i2+1] == 255) neighbor[3] = 1;
                                       else  neighbor[3] = 0;
                                       if (seg_xy_project[(j2+1)*length_block+i2+1] == 255) neighbor[4] = 1;
                                       else  neighbor[4] = 0;
                                       if (seg_xy_project[(j2+1)*length_block+i2] == 255) neighbor[5] = 1;
                                       else  neighbor[5] = 0;
                                       if (seg_xy_project[(j2+1)*length_block+i2-1] == 255) neighbor[6] = 1;
                                       else  neighbor[6] = 0;
                                       if (seg_xy_project[j2*length_block+i2-1] == 255) neighbor[7] = 1;
                                       else  neighbor[7] = 0;
                                       if (seg_xy_project[(j2-1)*length_block+i2-1] == 255) neighbor[8] = 1;
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
                                               if (((neighbor[1] * neighbor[3] * neighbor[5]) == 0) && ((neighbor[3] * neighbor[5] * neighbor[7]) == 0))
                                               {
                                                   delete_piont pp;
                                                   pp.xx=i2;
                                                   pp.yy=j2;
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
                             seg_xy_project[delete_list[num].xx+delete_list[num].yy*length_block]=0;
                           }
                           delete_list.clear();


                           for(V3DLONG j2=1;j2<length_block;j2++)
                           {
                               for(V3DLONG i2=1;i2<length_block;i2++)
                               {
                                   if(seg_xy_project[j2*length_block+i2]>0)
                                   {
                                       if (seg_xy_project[(j2-1)*length_block+i2] == 255) neighbor[1] = 1;
                                       else  neighbor[1] = 0;
                                       if (seg_xy_project[(j2-1)*length_block+i2 + 1] == 255) neighbor[2] = 1;
                                       else  neighbor[2] = 0;
                                       if (seg_xy_project[j2*length_block+i2 + 1] == 255) neighbor[3] = 1;
                                       else  neighbor[3] = 0;
                                       if (seg_xy_project[(j2+1)*length_block+i2 + 1] == 255) neighbor[4] = 1;
                                       else  neighbor[4] = 0;
                                       if (seg_xy_project[(j2+1)*length_block+i2] == 255) neighbor[5] = 1;
                                       else  neighbor[5] = 0;
                                       if (seg_xy_project[(j2+1)*length_block+i2 - 1] == 255) neighbor[6] = 1;
                                       else  neighbor[6] = 0;
                                       if (seg_xy_project[j2*length_block+i2 - 1] == 255) neighbor[7] = 1;
                                       else  neighbor[7] = 0;
                                       if (seg_xy_project[(j2-1)*length_block+i2 - 1] == 255) neighbor[8] = 1;
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
                                                   pp.xx=i2;
                                                   pp.yy=j2;
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
                             seg_xy_project[delete_list[num].xx+delete_list[num].yy*length_block]=0;
                           }
                           delete_list.clear();
                       }

                       // find the candidate
                       for(int j1=1;j1<length_block-1;j1++)
                       {
                           for(int i1=1;i1<length_block-1;i1++)
                           {
                               if(seg_xy_project[j1*length_block+i1]>0)
                               {
                                   if (seg_xy_project[(j1-1)*length_block+i1-1] == 255) new_neighbor[1] = 1;
                                   else  new_neighbor[1] = 0;
                                   if (seg_xy_project[(j1-1)*length_block+i1] == 255) new_neighbor[2] = 1;
                                   else  new_neighbor[2] = 0;
                                   if (seg_xy_project[(j1-1)*length_block+i1+1] == 255) new_neighbor[3] = 1;
                                   else  new_neighbor[3] = 0;
                                   if (seg_xy_project[(j1)*length_block+i1 + 1] == 255) new_neighbor[4] = 1;
                                   else  new_neighbor[4] = 0;
                                   if (seg_xy_project[(j1+1)*length_block+i1+1] == 255) new_neighbor[5] = 1;
                                   else  new_neighbor[5] = 0;
                                   if (seg_xy_project[(j1+1)*length_block+i1] == 255) new_neighbor[6] = 1;
                                   else  new_neighbor[6] = 0;
                                   if (seg_xy_project[(j1+1)*length_block+i1-1] == 255) new_neighbor[7] = 1;
                                   else  new_neighbor[7] = 0;
                                   if (seg_xy_project[(j1)*length_block+i1-1] == 255) new_neighbor[8] = 1;
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
                                       if(((i1==(radiu_block+1))||(i1==(radiu_block-1))||(i1==(radiu_block)))&&((j1==(radiu_block+1))||(j1==(radiu_block-1))||(j1==(radiu_block))))
                                       {
                                           if((i<17)&&(j<17))
                                           {
                                               s.x=k;
                                               s.y=i;
                                               s.z=j;
                                               s.radius=1;
                                               s.color=random_rgba8(255);
                                               curlist<<s;

                                           }
                                           else
                                           {
                                               // add largr multiscale ray-shooting model
                                              int xy_flag8=rayinten_2D(i,k,ray_numbers_2d,ray_length_2d,ray_x,ray_y,binar_datald,nx,ny);
    //                                          v3d_msg(QString(" x is %1, y is %2").arg(i1).arg(j1));
                                              int xy_flag10=rayinten_2D(i,k,ray_numbers_2d,(ray_length_2d+2),ray_x,ray_y,binar_datald,nx,ny);
                                              int xy_flag12=rayinten_2D(i,k,ray_numbers_2d,(ray_length_2d+4),ray_x,ray_y,binar_datald,nx,ny);
                                              int xy_flag14=rayinten_2D(i,k,ray_numbers_2d,(ray_length_2d+6),ray_x,ray_y,binar_datald,nx,ny);
                                              int xy_flag16=rayinten_2D(i,k,ray_numbers_2d,(ray_length_2d+8),ray_x,ray_y,binar_datald,nx,ny);
                                              int sum_flag=xy_flag8+xy_flag10+xy_flag12+xy_flag14+xy_flag16;
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
        if(seg_xy_project) {delete []seg_xy_project; seg_xy_project = 0;}
        if(mip_datald){delete []mip_datald;mip_datald=0;}
        if(binar_datald){delete []binar_datald;binar_datald=0;}
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

int Thin_branch_points_detection(V3DPluginCallback2&callback,QWidget *parent)
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
            int thres_2d = 25;
            int ray_length_2d = 8;
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
        int neighbor[8];
        int sum_points;
        int new_neighbor[8];
        vector<delete_piont> delete_list;
        int sum_flag=0;


        unsigned char *block;
        try{block=new unsigned char [size_block];}
        catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

        unsigned char *project;
        try{project=new unsigned char [size_plane];}
        catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

        unsigned char *seg_project;
        try{seg_project=new unsigned char [size_plane];}
        catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}
        for(int swith=0;swith<=2;swith++)
        {
           for(V3DLONG j=radiu_block;j<nz-radiu_block;j++)
           {
                for(V3DLONG i=radiu_block;i<ny-radiu_block;i++)
                {
                    for(V3DLONG k=radiu_block;k<nx-radiu_block;k++)
                    {
                        sum_flag=0;
                        V3DLONG num_block=0;
                        unsigned char pixe=p4DImage->getValueUINT8(k,i,j,0);
                        if(pixe>thres_2d)
                        {
                            for(V3DLONG a=j-radiu_block;a<=j+radiu_block;a++)
                            {
                                 V3DLONG z_location=a*nx*ny;
                                 for(V3DLONG b=i-radiu_block;b<=i+radiu_block;b++)
                                 {
                                     V3DLONG y_location=b*nx;
                                     for(V3DLONG c=k-radiu_block;c<=k+radiu_block;c++)
                                     {
                                           //v3d_msg(QString("pixe is %1").arg(datald[z_location+y_location+c]));
                                           block[num_block]=datald[z_location+y_location+c];
                                           num_block++;

                                     }
                                 }
                            }
        //                            v3d_msg(QString("create block success"));
                            if(swith==0) //select xy_plane
                            {
                                XY_mip(length_block,length_block,length_block,block,project);
                                thres_segment(size_plane,project,seg_project,thres_2d);
                                while(true)
                                {
                                    for(int j2=1;j2<length_block;j2++)
                                    {
                                        for(int i2=1;i2<length_block;i2++)
                                        {
                                            if(seg_project[j2*length_block+i2]>0)
                                            {
                                                if (seg_project[(j2-1)*length_block+i2] == 255) neighbor[1] = 1;
                                                else  neighbor[1] = 0;
                                                if (seg_project[(j2-1)*length_block+i2+1] == 255) neighbor[2] = 1;
                                                else  neighbor[2] = 0;
                                                if (seg_project[j2*length_block+i2+1] == 255) neighbor[3] = 1;
                                                else  neighbor[3] = 0;
                                                if (seg_project[(j2+1)*length_block+i2+1] == 255) neighbor[4] = 1;
                                                else  neighbor[4] = 0;
                                                if (seg_project[(j2+1)*length_block+i2] == 255) neighbor[5] = 1;
                                                else  neighbor[5] = 0;
                                                if (seg_project[(j2+1)*length_block+i2-1] == 255) neighbor[6] = 1;
                                                else  neighbor[6] = 0;
                                                if (seg_project[j2*length_block+i2-1] == 255) neighbor[7] = 1;
                                                else  neighbor[7] = 0;
                                                if (seg_project[(j2-1)*length_block+i2-1] == 255) neighbor[8] = 1;
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
                                                        if (((neighbor[1] * neighbor[3] * neighbor[5]) == 0) && ((neighbor[3] * neighbor[5] * neighbor[7]) == 0))
                                                        {
                                                            delete_piont pp;
                                                            pp.xx=i2;
                                                            pp.yy=j2;
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
                                      seg_project[delete_list[num].xx+delete_list[num].yy*length_block]=0;
                                    }
                                    delete_list.clear();


                                    for(V3DLONG j2=1;j2<length_block;j2++)
                                    {
                                        for(V3DLONG i2=1;i2<length_block;i2++)
                                        {
                                            if(seg_project[j2*length_block+i2]>0)
                                            {
                                                if (seg_project[(j2-1)*length_block+i2] == 255) neighbor[1] = 1;
                                                else  neighbor[1] = 0;
                                                if (seg_project[(j2-1)*length_block+i2 + 1] == 255) neighbor[2] = 1;
                                                else  neighbor[2] = 0;
                                                if (seg_project[j2*length_block+i2 + 1] == 255) neighbor[3] = 1;
                                                else  neighbor[3] = 0;
                                                if (seg_project[(j2+1)*length_block+i2 + 1] == 255) neighbor[4] = 1;
                                                else  neighbor[4] = 0;
                                                if (seg_project[(j2+1)*length_block+i2] == 255) neighbor[5] = 1;
                                                else  neighbor[5] = 0;
                                                if (seg_project[(j2+1)*length_block+i2 - 1] == 255) neighbor[6] = 1;
                                                else  neighbor[6] = 0;
                                                if (seg_project[j2*length_block+i2 - 1] == 255) neighbor[7] = 1;
                                                else  neighbor[7] = 0;
                                                if (seg_project[(j2-1)*length_block+i2 - 1] == 255) neighbor[8] = 1;
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
                                                            pp.xx=i2;
                                                            pp.yy=j2;
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
                                      seg_project[delete_list[num].xx+delete_list[num].yy*length_block]=0;
                                    }
                                    delete_list.clear();
                                }

                                for(int j1=1;j1<length_block-1;j1++)
                                {
                                    for(int i1=1;i1<length_block-1;i1++)
                                    {
                                        if(seg_project[j1*length_block+i1]>0)
                                        {
                                            if (seg_project[(j1-1)*length_block+i1-1] == 255) new_neighbor[1] = 1;
                                            else  new_neighbor[1] = 0;
                                            if (seg_project[(j1-1)*length_block+i1] == 255) new_neighbor[2] = 1;
                                            else  new_neighbor[2] = 0;
                                            if (seg_project[(j1-1)*length_block+i1+1] == 255) new_neighbor[3] = 1;
                                            else  new_neighbor[3] = 0;
                                            if (seg_project[(j1)*length_block+i1 + 1] == 255) new_neighbor[4] = 1;
                                            else  new_neighbor[4] = 0;
                                            if (seg_project[(j1+1)*length_block+i1+1] == 255) new_neighbor[5] = 1;
                                            else  new_neighbor[5] = 0;
                                            if (seg_project[(j1+1)*length_block+i1] == 255) new_neighbor[6] = 1;
                                            else  new_neighbor[6] = 0;
                                            if (seg_project[(j1+1)*length_block+i1-1] == 255) new_neighbor[7] = 1;
                                            else  new_neighbor[7] = 0;
                                            if (seg_project[(j1)*length_block+i1-1] == 255) new_neighbor[8] = 1;
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
                                                if(((i1==(radiu_block+1))||(i1==(radiu_block-1))||(i1==(radiu_block)))&&((j1==(radiu_block+1))||(j1==(radiu_block-1))||(j1==(radiu_block))))
                                                {
                                                    sum_flag++;
                                                }
                                            }
                                        }
                                    }
                                }

                            }
                            if(swith==1) //select xz_plane
                            {
                                XZ_mip(length_block,length_block,length_block,block,project);
                                thres_segment(size_plane,project,seg_project,thres_2d);
                                while(true)
                                {
                                    for(int j2=1;j2<length_block;j2++)
                                    {
                                        for(int i2=1;i2<length_block;i2++)
                                        {
                                            if(seg_project[j2*length_block+i2]>0)
                                            {
                                                if (seg_project[(j2-1)*length_block+i2] == 255) neighbor[1] = 1;
                                                else  neighbor[1] = 0;
                                                if (seg_project[(j2-1)*length_block+i2+1] == 255) neighbor[2] = 1;
                                                else  neighbor[2] = 0;
                                                if (seg_project[j2*length_block+i2+1] == 255) neighbor[3] = 1;
                                                else  neighbor[3] = 0;
                                                if (seg_project[(j2+1)*length_block+i2+1] == 255) neighbor[4] = 1;
                                                else  neighbor[4] = 0;
                                                if (seg_project[(j2+1)*length_block+i2] == 255) neighbor[5] = 1;
                                                else  neighbor[5] = 0;
                                                if (seg_project[(j2+1)*length_block+i2-1] == 255) neighbor[6] = 1;
                                                else  neighbor[6] = 0;
                                                if (seg_project[j2*length_block+i2-1] == 255) neighbor[7] = 1;
                                                else  neighbor[7] = 0;
                                                if (seg_project[(j2-1)*length_block+i2-1] == 255) neighbor[8] = 1;
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
                                                        if (((neighbor[1] * neighbor[3] * neighbor[5]) == 0) && ((neighbor[3] * neighbor[5] * neighbor[7]) == 0))
                                                        {
                                                            delete_piont pp;
                                                            pp.xx=i2;
                                                            pp.yy=j2;
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
                                      seg_project[delete_list[num].xx+delete_list[num].yy*length_block]=0;
                                    }
                                    delete_list.clear();


                                    for(V3DLONG j2=1;j2<length_block;j2++)
                                    {
                                        for(V3DLONG i2=1;i2<length_block;i2++)
                                        {
                                            if(seg_project[j2*length_block+i2]>0)
                                            {
                                                if (seg_project[(j2-1)*length_block+i2] == 255) neighbor[1] = 1;
                                                else  neighbor[1] = 0;
                                                if (seg_project[(j2-1)*length_block+i2 + 1] == 255) neighbor[2] = 1;
                                                else  neighbor[2] = 0;
                                                if (seg_project[j2*length_block+i2 + 1] == 255) neighbor[3] = 1;
                                                else  neighbor[3] = 0;
                                                if (seg_project[(j2+1)*length_block+i2 + 1] == 255) neighbor[4] = 1;
                                                else  neighbor[4] = 0;
                                                if (seg_project[(j2+1)*length_block+i2] == 255) neighbor[5] = 1;
                                                else  neighbor[5] = 0;
                                                if (seg_project[(j2+1)*length_block+i2 - 1] == 255) neighbor[6] = 1;
                                                else  neighbor[6] = 0;
                                                if (seg_project[j2*length_block+i2 - 1] == 255) neighbor[7] = 1;
                                                else  neighbor[7] = 0;
                                                if (seg_project[(j2-1)*length_block+i2 - 1] == 255) neighbor[8] = 1;
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
                                                            pp.xx=i2;
                                                            pp.yy=j2;
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
                                      seg_project[delete_list[num].xx+delete_list[num].yy*length_block]=0;
                                    }
                                    delete_list.clear();
                                }

                                for(int j1=1;j1<length_block-1;j1++)
                                {
                                    for(int i1=1;i1<length_block-1;i1++)
                                    {
                                        if(seg_project[j1*length_block+i1]>0)
                                        {
                                            if (seg_project[(j1-1)*length_block+i1-1] == 255) new_neighbor[1] = 1;
                                            else  new_neighbor[1] = 0;
                                            if (seg_project[(j1-1)*length_block+i1] == 255) new_neighbor[2] = 1;
                                            else  new_neighbor[2] = 0;
                                            if (seg_project[(j1-1)*length_block+i1+1] == 255) new_neighbor[3] = 1;
                                            else  new_neighbor[3] = 0;
                                            if (seg_project[(j1)*length_block+i1 + 1] == 255) new_neighbor[4] = 1;
                                            else  new_neighbor[4] = 0;
                                            if (seg_project[(j1+1)*length_block+i1+1] == 255) new_neighbor[5] = 1;
                                            else  new_neighbor[5] = 0;
                                            if (seg_project[(j1+1)*length_block+i1] == 255) new_neighbor[6] = 1;
                                            else  new_neighbor[6] = 0;
                                            if (seg_project[(j1+1)*length_block+i1-1] == 255) new_neighbor[7] = 1;
                                            else  new_neighbor[7] = 0;
                                            if (seg_project[(j1)*length_block+i1-1] == 255) new_neighbor[8] = 1;
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
                                                if(((i1==(radiu_block+1))||(i1==(radiu_block-1))||(i1==(radiu_block)))&&((j1==(radiu_block+1))||(j1==(radiu_block-1))||(j1==(radiu_block))))
                                                {
                                                    sum_flag++;
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            if(swith==2) //select yz_plane
                            {
                                YZ_mip(length_block,length_block,length_block,block,project);
                                thres_segment(size_plane,project,seg_project,thres_2d);
                                while(true)
                                {
                                    for(int j2=1;j2<length_block;j2++)
                                    {
                                        for(int i2=1;i2<length_block;i2++)
                                        {
                                            if(seg_project[j2*length_block+i2]>0)
                                            {
                                                if (seg_project[(j2-1)*length_block+i2] == 255) neighbor[1] = 1;
                                                else  neighbor[1] = 0;
                                                if (seg_project[(j2-1)*length_block+i2+1] == 255) neighbor[2] = 1;
                                                else  neighbor[2] = 0;
                                                if (seg_project[j2*length_block+i2+1] == 255) neighbor[3] = 1;
                                                else  neighbor[3] = 0;
                                                if (seg_project[(j2+1)*length_block+i2+1] == 255) neighbor[4] = 1;
                                                else  neighbor[4] = 0;
                                                if (seg_project[(j2+1)*length_block+i2] == 255) neighbor[5] = 1;
                                                else  neighbor[5] = 0;
                                                if (seg_project[(j2+1)*length_block+i2-1] == 255) neighbor[6] = 1;
                                                else  neighbor[6] = 0;
                                                if (seg_project[j2*length_block+i2-1] == 255) neighbor[7] = 1;
                                                else  neighbor[7] = 0;
                                                if (seg_project[(j2-1)*length_block+i2-1] == 255) neighbor[8] = 1;
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
                                                        if (((neighbor[1] * neighbor[3] * neighbor[5]) == 0) && ((neighbor[3] * neighbor[5] * neighbor[7]) == 0))
                                                        {
                                                            delete_piont pp;
                                                            pp.xx=i2;
                                                            pp.yy=j2;
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
                                      seg_project[delete_list[num].xx+delete_list[num].yy*length_block]=0;
                                    }
                                    delete_list.clear();


                                    for(V3DLONG j2=1;j2<length_block;j2++)
                                    {
                                        for(V3DLONG i2=1;i2<length_block;i2++)
                                        {
                                            if(seg_project[j2*length_block+i2]>0)
                                            {
                                                if (seg_project[(j2-1)*length_block+i2] == 255) neighbor[1] = 1;
                                                else  neighbor[1] = 0;
                                                if (seg_project[(j2-1)*length_block+i2 + 1] == 255) neighbor[2] = 1;
                                                else  neighbor[2] = 0;
                                                if (seg_project[j2*length_block+i2 + 1] == 255) neighbor[3] = 1;
                                                else  neighbor[3] = 0;
                                                if (seg_project[(j2+1)*length_block+i2 + 1] == 255) neighbor[4] = 1;
                                                else  neighbor[4] = 0;
                                                if (seg_project[(j2+1)*length_block+i2] == 255) neighbor[5] = 1;
                                                else  neighbor[5] = 0;
                                                if (seg_project[(j2+1)*length_block+i2 - 1] == 255) neighbor[6] = 1;
                                                else  neighbor[6] = 0;
                                                if (seg_project[j2*length_block+i2 - 1] == 255) neighbor[7] = 1;
                                                else  neighbor[7] = 0;
                                                if (seg_project[(j2-1)*length_block+i2 - 1] == 255) neighbor[8] = 1;
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
                                                            pp.xx=i2;
                                                            pp.yy=j2;
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
                                      seg_project[delete_list[num].xx+delete_list[num].yy*length_block]=0;
                                    }
                                    delete_list.clear();
                                }

                                for(int j1=1;j1<length_block-1;j1++)
                                {
                                    for(int i1=1;i1<length_block-1;i1++)
                                    {
                                        if(seg_project[j1*length_block+i1]>0)
                                        {
                                            if (seg_project[(j1-1)*length_block+i1-1] == 255) new_neighbor[1] = 1;
                                            else  new_neighbor[1] = 0;
                                            if (seg_project[(j1-1)*length_block+i1] == 255) new_neighbor[2] = 1;
                                            else  new_neighbor[2] = 0;
                                            if (seg_project[(j1-1)*length_block+i1+1] == 255) new_neighbor[3] = 1;
                                            else  new_neighbor[3] = 0;
                                            if (seg_project[(j1)*length_block+i1 + 1] == 255) new_neighbor[4] = 1;
                                            else  new_neighbor[4] = 0;
                                            if (seg_project[(j1+1)*length_block+i1+1] == 255) new_neighbor[5] = 1;
                                            else  new_neighbor[5] = 0;
                                            if (seg_project[(j1+1)*length_block+i1] == 255) new_neighbor[6] = 1;
                                            else  new_neighbor[6] = 0;
                                            if (seg_project[(j1+1)*length_block+i1-1] == 255) new_neighbor[7] = 1;
                                            else  new_neighbor[7] = 0;
                                            if (seg_project[(j1)*length_block+i1-1] == 255) new_neighbor[8] = 1;
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
                                                if(((i1==(radiu_block+1))||(i1==(radiu_block-1))||(i1==(radiu_block)))&&((j1==(radiu_block+1))||(j1==(radiu_block-1))||(j1==(radiu_block))))
                                                {
                                                    sum_flag++;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            if(sum_flag>=1)
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

        Image4DSimple * new4DImage = new Image4DSimple();
        new4DImage->setData((unsigned char *)datald, p4DImage->getXDim(), p4DImage->getYDim(), p4DImage->getZDim(), p4DImage->getCDim(), p4DImage->getDatatype());
        v3dhandle newwin = callback.newImageWindow();
        callback.setImage(newwin, new4DImage);
        callback.setImageName(newwin, "maximum intensity projection image");
        callback.updateImageWindow(newwin);
        v3d_msg(QString("numble of marker is %1").arg(curlist.size()));
        callback.setLandmark(newwin, curlist);
        if(block) {delete []block; block = 0;}
        if(project) {delete []project; project = 0;}
        if(seg_project) {delete []seg_project; seg_project = 0;}
        return 1;
}

int Thin_branch_points_detection_single(V3DPluginCallback2 &callback, QWidget *parent)
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
            int thres_2d =25;
            int ray_length_2d = 8;
            int radiu_block=8;


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
        int neighbor[8];
        int sum_points;
        int new_neighbor[8];
        vector<delete_piont> delete_list;


        unsigned char *block;
        try{block=new unsigned char [size_block];}
        catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

        unsigned char *xy_project;
        try{xy_project=new unsigned char [size_block];}
        catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

        unsigned char *seg_xy_project;
        try{seg_xy_project=new unsigned char [size_block];}
        catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

        for(V3DLONG j=radiu_block;j<nz-radiu_block;j++)
        {
           for(V3DLONG i=radiu_block;i<ny-radiu_block;i++)
           {
               for(V3DLONG k=radiu_block;k<nx-radiu_block;k++)
               {
                   unsigned char pixe=p4DImage->getValueUINT8(k,i,j,0);
                   V3DLONG num_block=0;
                   if(pixe>thres_2d)
                   {
        //                    v3d_msg(QString("pixe is %1").arg(pixe));
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

                       XY_mip(length_block,length_block,length_block,block,xy_project);
                       thres_segment(size_plane,xy_project,seg_xy_project,thres_2d);


                       //thined the block
                       while(true)
                       {
                           for(int j2=1;j2<length_block;j2++)
                           {
                               for(int i2=1;i2<length_block;i2++)
                               {
                                   if(seg_xy_project[j2*length_block+i2]>0)
                                   {
                                       if (seg_xy_project[(j2-1)*length_block+i2] == 255) neighbor[1] = 1;
                                       else  neighbor[1] = 0;
                                       if (seg_xy_project[(j2-1)*length_block+i2+1] == 255) neighbor[2] = 1;
                                       else  neighbor[2] = 0;
                                       if (seg_xy_project[j2*length_block+i2+1] == 255) neighbor[3] = 1;
                                       else  neighbor[3] = 0;
                                       if (seg_xy_project[(j2+1)*length_block+i2+1] == 255) neighbor[4] = 1;
                                       else  neighbor[4] = 0;
                                       if (seg_xy_project[(j2+1)*length_block+i2] == 255) neighbor[5] = 1;
                                       else  neighbor[5] = 0;
                                       if (seg_xy_project[(j2+1)*length_block+i2-1] == 255) neighbor[6] = 1;
                                       else  neighbor[6] = 0;
                                       if (seg_xy_project[j2*length_block+i2-1] == 255) neighbor[7] = 1;
                                       else  neighbor[7] = 0;
                                       if (seg_xy_project[(j2-1)*length_block+i2-1] == 255) neighbor[8] = 1;
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
                                               if (((neighbor[1] * neighbor[3] * neighbor[5]) == 0) && ((neighbor[3] * neighbor[5] * neighbor[7]) == 0))
                                               {
                                                   delete_piont pp;
                                                   pp.xx=i2;
                                                   pp.yy=j2;
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
                             seg_xy_project[delete_list[num].xx+delete_list[num].yy*length_block]=0;
                           }
                           delete_list.clear();


                           for(V3DLONG j2=1;j2<length_block;j2++)
                           {
                               for(V3DLONG i2=1;i2<length_block;i2++)
                               {
                                   if(seg_xy_project[j2*length_block+i2]>0)
                                   {
                                       if (seg_xy_project[(j2-1)*length_block+i2] == 255) neighbor[1] = 1;
                                       else  neighbor[1] = 0;
                                       if (seg_xy_project[(j2-1)*length_block+i2 + 1] == 255) neighbor[2] = 1;
                                       else  neighbor[2] = 0;
                                       if (seg_xy_project[j2*length_block+i2 + 1] == 255) neighbor[3] = 1;
                                       else  neighbor[3] = 0;
                                       if (seg_xy_project[(j2+1)*length_block+i2 + 1] == 255) neighbor[4] = 1;
                                       else  neighbor[4] = 0;
                                       if (seg_xy_project[(j2+1)*length_block+i2] == 255) neighbor[5] = 1;
                                       else  neighbor[5] = 0;
                                       if (seg_xy_project[(j2+1)*length_block+i2 - 1] == 255) neighbor[6] = 1;
                                       else  neighbor[6] = 0;
                                       if (seg_xy_project[j2*length_block+i2 - 1] == 255) neighbor[7] = 1;
                                       else  neighbor[7] = 0;
                                       if (seg_xy_project[(j2-1)*length_block+i2 - 1] == 255) neighbor[8] = 1;
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
                                                   pp.xx=i2;
                                                   pp.yy=j2;
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
                             seg_xy_project[delete_list[num].xx+delete_list[num].yy*length_block]=0;
                           }
                           delete_list.clear();
                       }

                       // find the candidate
                       for(int j1=1;j1<length_block-1;j1++)
                       {
                           for(int i1=1;i1<length_block-1;i1++)
                           {
                               if(seg_xy_project[j1*length_block+i1]>0)
                               {
                                   if (seg_xy_project[(j1-1)*length_block+i1-1] == 255) new_neighbor[1] = 1;
                                   else  new_neighbor[1] = 0;
                                   if (seg_xy_project[(j1-1)*length_block+i1] == 255) new_neighbor[2] = 1;
                                   else  new_neighbor[2] = 0;
                                   if (seg_xy_project[(j1-1)*length_block+i1+1] == 255) new_neighbor[3] = 1;
                                   else  new_neighbor[3] = 0;
                                   if (seg_xy_project[(j1)*length_block+i1 + 1] == 255) new_neighbor[4] = 1;
                                   else  new_neighbor[4] = 0;
                                   if (seg_xy_project[(j1+1)*length_block+i1+1] == 255) new_neighbor[5] = 1;
                                   else  new_neighbor[5] = 0;
                                   if (seg_xy_project[(j1+1)*length_block+i1] == 255) new_neighbor[6] = 1;
                                   else  new_neighbor[6] = 0;
                                   if (seg_xy_project[(j1+1)*length_block+i1-1] == 255) new_neighbor[7] = 1;
                                   else  new_neighbor[7] = 0;
                                   if (seg_xy_project[(j1)*length_block+i1-1] == 255) new_neighbor[8] = 1;
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
                                       if(((i1==(radiu_block+1))||(i1==(radiu_block-1))||(i1==(radiu_block)))&&((j1==(radiu_block+1))||(j1==(radiu_block-1))||(j1==(radiu_block))))
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
        if(seg_xy_project) {delete []seg_xy_project; seg_xy_project = 0;}
        return 1;
}
