#include "tipdetector.h"
#include<iostream>
#include "basic_surf_objs.h"
#include "my_surf_objs.h"
#include"ClusterAnalysis.h"
#include<QFileInfo>
#include<QDir>
#define PI 3.1415926
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))

using namespace std;

TipDetector::TipDetector()
{
    isSavingTime=false;
    //init all para
     datald=0;
     tip_img_datald=0;
     number_of_rays = 64;
     background_threshold = 10;
     magnitude =5.0;
     minimum_scale=4;
     number_of_scales=4;
     step_size=2;
     intensity_threshold=115;
     angle_threshold=68;
//     mTreeList=NULL;

//initial it for
     block = 0;
     xy_mip_datald = 0;
     xz_mip_datald = 0;
     yz_mip_datald=0;
     enlarged_datald=0;
     enlarged_datald=0;

}
TipDetector::~TipDetector()
{
    if(block) {delete []block; block = 0;}
    if(xy_mip_datald) {delete []xy_mip_datald; xy_mip_datald = 0;}
    if(xz_mip_datald) {delete []xz_mip_datald; xz_mip_datald = 0;}
    if(yz_mip_datald){delete []yz_mip_datald;yz_mip_datald=0;}
    if(enlarged_datald){delete []enlarged_datald;enlarged_datald=0;}
    if(binar_datald){delete []enlarged_datald;enlarged_datald=0;}
}

void TipDetector::GetNeuronTreeFrom3dview(V3DPluginCallback2 &callback)
{

    QList <V3dR_MainWindow *> list_3dviewer = callback.getListAll3DViewers();
    if (list_3dviewer.size() < 1)
    {
        v3d_msg("Please open  a SWC file from the main menu first!");
        return;
    }
    V3dR_MainWindow *surface_win = list_3dviewer[0];
    if (!surface_win){
        v3d_msg("Please open up a SWC file from the main menu first!");
        return;
    }
//    cout<<"read SWC from 3d Viewer  "<<endl;
//    mTreeList = callback.getHandleNeuronTrees_Any3DViewer(surface_win);
}

LocationSimple TipDetector::Nearest_tip(LocationSimple point_a)
{
    //input a point output the nearest tip point
    double small_dist=1000000;
    LocationSimple final_tip_point;

    if(curlist.size()==0)
    {
        final_tip_point.x=-1;
        final_tip_point.y=-1;
        final_tip_point.z=-1;
        v3d_msg("error ,can't find tip points");
    }
    for(V3DLONG i=0;i<curlist.size();i++)
    {
        LocationSimple cur_tip=curlist.at(i);
        double temp_dist=sqrt((cur_tip.x - point_a.x)*(cur_tip.x - point_a.x) + (cur_tip.y - point_a.y)*(cur_tip.y - point_a.y) + (cur_tip.z - point_a.z)*(cur_tip.z - point_a.z));
        if(temp_dist<small_dist)
        {
            small_dist=temp_dist;
            final_tip_point=cur_tip;
        }
    }
    return final_tip_point;

}

bool TipDetector::TipDetect_onePoint_neighbor_no_z(LocationSimple CandiPoint,LocationSimple &can,unsigned int radius)
{
    if(radius<0) return false;
    for(int temp_radius=0;temp_radius<radius;temp_radius++)
    {
        for(float k=-temp_radius*5;k<=temp_radius*5;k++)
        {
            for(float j=-temp_radius;j<=temp_radius;j++)
            {
                for(float i=-temp_radius;i<=temp_radius;i++)
                {
                   can.x=CandiPoint.x+i;
                   can.y=CandiPoint.y+j;
                   can.z=CandiPoint.z+k;

                   bool isTip=TipDetect_onePoint(can);
                   if(isTip)
                       return true;
                }
            }
        }
    }
    return false;
}


bool TipDetector::TipDetect_onePoint_neighbor(LocationSimple CandiPoint,LocationSimple &can,unsigned int radius)
{
    //something wrong ?
    if(radius<0) return false;
    CandiPoint.x=V3DLONG(CandiPoint.x+0.5);
    CandiPoint.y=V3DLONG(CandiPoint.y+0.5);
    CandiPoint.z=V3DLONG(CandiPoint.z+0.5);
    for(int temp_radius=0;temp_radius<radius;temp_radius++)
    {
        for(float k=-temp_radius;k<=temp_radius;k++)
        {
            for(float j=-temp_radius;j<=temp_radius;j++)
            {
                for(float i=-temp_radius;i<=temp_radius;i++)
                {
                   can.x=CandiPoint.x+i;
                   can.y=CandiPoint.y+j;
                   can.z=CandiPoint.z+k;

                   bool isTip=TipDetect_onePoint(can);
                   if(isTip)
                       return true;
                }
            }
        }
    }
    return false;
}



bool TipDetector::TipDetect_onePoint_27neighbor(LocationSimple CandiPoint,LocationSimple &can)
{
    for(float i=-1;i<=1;i++)
    {
        for(float j=-1;j<=1;j++)
        {
            for(float k=-1;k<=1;k++)
            {
               can.x=CandiPoint.x+i;
               can.y=CandiPoint.y+j;
               can.z=CandiPoint.z+k;

               bool isTip=TipDetect_onePoint(can);
               if(isTip)
                   return true;
            }
        }
    }
    return false;
}

bool TipDetector::isForeground(LocationSimple CandiPoint)
{
    V3DLONG true_i=V3DLONG(CandiPoint.x+0.5);
    V3DLONG true_j=V3DLONG(CandiPoint.y+0.5);
    V3DLONG true_k=V3DLONG(CandiPoint.z+0.5);
    if(true_i>=sz[0]||true_j>=sz[1]||true_k>=sz[2])
    {
        return false;
    }
    else if(true_i<0||true_j<0||true_k<0)
    {
        return false;
    }
    unsigned char pixe=datald[true_k*sz[1]*sz[0]+true_j*sz[0]+true_i];
    if(pixe>background_threshold)
    {
        return true;
    }
    else
    { return false;}


}
bool TipDetector::refer_TipDetect_onePoint_(LocationSimple CandiPoint)
{
    int xy_flag=0;
    int xz_flag=0;
    int yz_flag=0;
    int sum_flag=0;
    V3DLONG k=V3DLONG(CandiPoint.x+radiu_block+0.5);
    V3DLONG i=V3DLONG(CandiPoint.y+radiu_block+0.5);
    V3DLONG j=V3DLONG(CandiPoint.z+radiu_block+0.5);

    V3DLONG true_i=V3DLONG(CandiPoint.x+0.5);
    V3DLONG true_j=V3DLONG(CandiPoint.y+0.5);
    V3DLONG true_k=V3DLONG(CandiPoint.z+0.5);
    if(true_i>=sz[0]||true_j>=sz[1]||true_k>=sz[2])
    {
        return false;
    }
    else if(true_i<0||true_j<0||true_k<0)
    {
        return false;
    }

    if(tip_img_datald[true_k*sz[0]*sz[1]+true_j*sz[0]+true_i]==255)
    {
//            cout<<"to find true tip";
        return true;
    }
    else if(tip_img_datald[true_k*sz[0]*sz[1]+true_j*sz[0]+true_i]==1||tip_img_datald[true_k*sz[0]*sz[1]+true_j*sz[0]+true_i]==99)
    {
        return false;
    }
    else if(tip_img_datald[true_k*sz[0]*sz[1]+true_j*sz[0]+true_i]==0)
    {cout<<"please run whole img before"<<endl;}
}
bool TipDetector::TipDetect_onePoint(LocationSimple CandiPoint)
{
    int xy_flag=0;
    int xz_flag=0;
    int yz_flag=0;
    int sum_flag=0;
    V3DLONG k=V3DLONG(CandiPoint.x+radiu_block+0.5);
    V3DLONG i=V3DLONG(CandiPoint.y+radiu_block+0.5);
    V3DLONG j=V3DLONG(CandiPoint.z+radiu_block+0.5);

    V3DLONG true_i=V3DLONG(CandiPoint.x+0.5);
    V3DLONG true_j=V3DLONG(CandiPoint.y+0.5);
    V3DLONG true_k=V3DLONG(CandiPoint.z+0.5);

    if(1)
    {
//        if(true_i==102&&true_j==427)
//        {
//            cout<<"true_i==102&&true_j==427 true_k:"<<true_k<<endl;
//        }
        if(true_i>=sz[0]||true_j>=sz[1]||true_k>=sz[2])
        {
            return false;
        }
        else if(true_i<0||true_j<0||true_k<0)
        {
            return false;
        }

        if(tip_img_datald[true_k*sz[0]*sz[1]+true_j*sz[0]+true_i]==255)
        {
//            cout<<"to find true tip";
            return true;
        }
        else if(tip_img_datald[true_k*sz[0]*sz[1]+true_j*sz[0]+true_i]==1||tip_img_datald[true_k*sz[0]*sz[1]+true_j*sz[0]+true_i]==99)
        {
            return false;
        }
//        if(true_i==102&&true_j==427)
//        {
//            cout<<"true_i==102&&true_j==427 true_k not over:"<<true_k<<endl;
//        }
    }
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
//        if(sz[2]==1)
//        {
//            //special
//            XY_mip(length_block,length_block,length_block,block,xy_mip_datald);
//            xy_flag=rayinten_2D(radiu_block,radiu_block,number_of_rays,minimum_scale,ray_x,ray_y,length_block,length_block,xy_mip_datald,intensity_threshold,angle_threshold);
//            if(xy_flag>=1)
//            {
//                tip_img_datald[true_k*sz[0]*sz[1]+true_j*sz[0]+true_i]=255;

//                return true;
//            }
//            else
//                return false;

//        }
        XY_mip(length_block,length_block,length_block,block,xy_mip_datald);
        XZ_mip(length_block,length_block,length_block,block,xz_mip_datald);
        YZ_mip(length_block,length_block,length_block,block,yz_mip_datald);

        xy_flag=rayinten_2D(radiu_block,radiu_block,number_of_rays,minimum_scale,ray_x,ray_y,length_block,length_block,xy_mip_datald,intensity_threshold,angle_threshold);
        xz_flag=rayinten_2D(radiu_block,radiu_block,number_of_rays,minimum_scale,ray_x,ray_y,length_block,length_block,xz_mip_datald,intensity_threshold,angle_threshold);
        yz_flag=rayinten_2D(radiu_block,radiu_block,number_of_rays,minimum_scale,ray_x,ray_y,length_block,length_block,yz_mip_datald,intensity_threshold,angle_threshold);

        sum_flag=xy_flag+xz_flag+yz_flag;
        if(sum_flag>=2)
        {
           tip_img_datald[true_k*sz[0]*sz[1]+true_j*sz[0]+true_i]=255;
            return true;
        }
        else
        {
            tip_img_datald[true_k*sz[0]*sz[1]+true_j*sz[0]+true_i]=99;
        }
    }
    else{tip_img_datald[true_k*sz[0]*sz[1]+true_j*sz[0]+true_i]=1;}


    return false;

}

bool TipDetector::save_tp_as_temp(V3DPluginCallback2& callback)
{
    // save tp.out
    v3dhandle curwin = callback.currentImageWindow();
    QString img_name=callback.getImageName(curwin);
    QFileInfo fileinfo(img_name);
    QString path_tp=fileinfo.path();
//    cout<<"path_tp before "<<path_tp.toUtf8().data()<<endl;
    path_tp.append("//temp_tp_");
    path_tp.append(fileinfo.baseName());

    QDir dir;
    if(!dir.exists(path_tp))
    {
        dir.mkdir(path_tp);
    }
//    cout<<"path_tp after append "<<path_tp.toUtf8().data()<<endl;

    if(1)
    {
        //save tip list
        LandmarkList tip_true_list=this->OutputTipPoint();
        QString marker_filename=path_tp;
        marker_filename.append("//");
        marker_filename.append(fileinfo.baseName());
        marker_filename.append("_tiplist_temp.marker");
//        cout<<"marker_filename"<<marker_filename.toUtf8().data()<<endl;
        writeMarker_file(marker_filename,LandmarkListQList_ImageMarker(tip_true_list));


        //save  tip_img_datald  sz[4]
        QString tip_img_filename=path_tp;
        tip_img_filename.append("//");
        tip_img_filename.append(fileinfo.baseName());
//        tip_img_filename.append(QString::number(rand()%500));
        tip_img_filename.append("_tp_img_temp.tif");
//        cout<<"tip_img_filename"<<tip_img_filename.toUtf8().data()<<endl;
        simple_saveimage_wrapper(callback,tip_img_filename.toUtf8().data(),(unsigned char *)tip_img_datald,sz,p4DImage->getDatatype());

//        temp_tip_img_datald
    }

//
    return true;


}
bool TipDetector::load_tp_temp(V3DPluginCallback2& callback)
{
    // save tp.out
    v3dhandle curwin = callback.currentImageWindow();
    QString img_name=callback.getImageName(curwin);
    QFileInfo fileinfo(img_name);
    QString path_tp=fileinfo.path();
//    cout<<"path_tp before "<<path_tp.toUtf8().data()<<endl;
    path_tp.append("//temp_tp_");
    path_tp.append(fileinfo.baseName());

    QDir dir;
    if(!dir.exists(path_tp))
    {
        return false;
    }
//    cout<<"path_tp after append "<<path_tp.toUtf8().data()<<endl;

    if(1)
    {
        //save tip list
//        LandmarkList tip_true_list=this->OutputTipPoint();
        QString marker_filename=path_tp;
        marker_filename.append("//");
        marker_filename.append(fileinfo.baseName());
        marker_filename.append("_tiplist_temp.marker");
//        cout<<"marker_filename"<<marker_filename.toUtf8().data()<<endl;
//        writeMarker_file(marker_filename,LandmarkListQList_ImageMarker(tip_true_list));
        curlist=QList_ImageMarker2LandmarkList(readMarker_file(marker_filename));
        RGBA8 white;
        white.r=255;white.g=255;white.b=255;
        for(int i=0;i<curlist.size();i++)
        {
            curlist[i].color.r=255;
            curlist[i].color.g=0;
            curlist[i].color.b=0;
        }
        //save  tip_img_datald  sz[4]
        QString tip_img_filename=path_tp;
        tip_img_filename.append("//");
        tip_img_filename.append(fileinfo.baseName());
        tip_img_filename.append("_tp_img_temp.tif");
//        cout<<"tip_img_filename"<<tip_img_filename.toUtf8().data()<<endl;
//        simple_saveimage_wrapper(callback,tip_img_filename.toUtf8().data(),(unsigned char *)tip_img_datald,sz,p4DImage->getDatatype());
        int temp_Datatype;
        simple_loadimage_wrapper(callback,tip_img_filename.toUtf8().data(),(unsigned char *)tip_img_datald,sz,temp_Datatype);
        return true;
    }

//
    return false;




}
void TipDetector::show_tipdetetor_img(V3DPluginCallback2& callback)
{
    cout<<"begin to show";
    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *)tip_img_datald, sz[0], sz[1], sz[2], p4DImage->getCDim(), p4DImage->getDatatype());
    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, new4DImage);
//    callback.updateImageWindow(newwin);
    callback.setLandmark(newwin, curlist);

}
void TipDetector::whole_img(V3DPluginCallback2& callback)
{
    if(isSavingTime)
    {
//        if(this->load_tp_temp(callback))
//        {
//            cout<<"read data from local file ,finish deal"<<endl;
//            return;
//        }
    }

    cout<<"this is a "<<sz[0]<<"*"<<sz[1]<<"*"<<sz[2]<<" size img"<<endl;
    RGBA8 p_color;
    p_color.r=255;
    p_color.g=255;
    p_color.b=255;
    for(V3DLONG j=0;j<sz[2];j++)
    {
         for(V3DLONG i=0;i<sz[1];i++)
         {
             for(V3DLONG k=0;k<sz[0];k++)
             {
                 LocationSimple candypoint;
                 candypoint.x=k;
                 candypoint.y=i;
                 candypoint.z=j;
                 bool isTip;
                 isTip=this->TipDetect_onePoint(candypoint);
                  //test TipDetect_onePoint function
                 if(isTip)
                 {
                     s.x=k;
                     s.y=i;
                     s.z=j;
                     s.color=p_color;
                     s.size=1;
                     curlist<<s;
                 }
             }
         }
    }

    this->cluster(curlist);
    if(0)  //adjust point to near tip
    {
        cout<<"begin to adjust "<<endl;
        for(V3DLONG i=0;i<curlist.size();i++)
        {
            //adjust tip point to true tip point

            if(!this->TipDetect_onePoint(curlist.at(i)))
            {
                 //如果聚类到了非末梢点  那么需要回到末梢点
                cout<<" need to adjust point x:"<<curlist.at(i).x<<" y "<<curlist.at(i).y<<" z "<<curlist.at(i).z<<endl;
                LocationSimple true_point;
                if(TipDetect_onePoint_neighbor(curlist.at(i),true_point,10))
                {
                    cout<<"before adjust:x "<<curlist.at(i).x<<" y "<<curlist.at(i).y<<" z "<<curlist.at(i).z<<endl;
                    curlist[i]=true_point;
                    cout<<"after adjust:x "<<curlist.at(i).x<<" y "<<curlist.at(i).y<<" z "<<curlist.at(i).z<<endl;

                    cout<<"datald: "<<int(tip_img_datald[V3DLONG(true_point.z*sz[0]*sz[1]+true_point.y*sz[0]+true_point.x)])<<endl;

                }
                else
                {
                    cout<<"there must be something wrong in 5*sqrt(3) x "<<curlist.at(i).x<<" y "<<curlist.at(i).y<<" z "<<curlist.at(i).z<<endl;

                }



            }
            else
            {
                cout<<"no need for adjust x "<<curlist.at(i).x<<" y "<<curlist.at(i).y<<" z "<<curlist.at(i).z<<endl;
            }

        }
        cout<<"end  adjust "<<endl;
    }




// // display output
//    if(datald){delete []datald;datald=0;}
//    Image4DSimple * new4DImage = new Image4DSimple();
//    new4DImage->setData((unsigned char *)binar_datald, new_nx, new_ny, new_nz, p4DImage->getCDim(), p4DImage->getDatatype());
//    v3dhandle newwin = callback->newImageWindow();
//    callback->setImage(newwin, new4DImage);
//    callback->updateImageWindow(newwin);
//    callback->setLandmark(newwin, curlist);
    cout<<"tip detector finished"<<endl;
    if(isSavingTime)
    {
        this->save_tp_as_temp(callback);
    }

    return ;
}

void TipDetector::cluster(LandmarkList &curlistToCluster)
{
    //    // change the clustering algorithm by chaowang 2018.11.16
//        for(double i=0;i<curlistToCluster.size();i++)
//        {
//            for(double j=i+1;j<curlistToCluster.size();j++)
//            {
//                if(square(curlistToCluster[j].x-curlistToCluster[i].x)+square(curlistToCluster[j].y-curlistToCluster[i].y)+square(curlistToCluster[j].z-curlistToCluster[i].z)<60)
//                {
//                    curlistToCluster[i].x=(curlistToCluster[j].x+curlistToCluster[i].x)/2;
//                    curlistToCluster[i].y=(curlistToCluster[j].y+curlistToCluster[i].y)/2;
//                    curlistToCluster[i].z=(curlistToCluster[j].z+curlistToCluster[i].z)/2;
//                    curlistToCluster.removeAt(j);
//                    j=j-1;
//                }
//            }
//        }
        ClusterAnalysis cluster;
        cluster.Read_from_curlist(curlistToCluster,10,2);
        cluster.DoDBSCANRecursive();
        curlistToCluster = cluster.get_clustered_curlist(curlistToCluster);
        return;

}
float TipDetector::getPluginVersion()
{
    return 1.2f;
}
void TipDetector::help()
{
//    cout<<"\nThis is a demo plugin to detect tip point in an image. by Keran Lin 2017-04"<<endl;
//    cout<<"\nUsage: v3d -x <example_plugin_name> -f tip_detection -i <input_image_file> -o <output_image_file> -p <subject_color_channel> <threshold>"<<endl;
//    cout<<"\t -i <input_image_file>                      input 3D image (tif, raw or lsm)"<<endl;
//    cout<<"\t -o <output_image_file>                     output image of the thresholded subject channel"<<endl;
//    cout<<"\t                                            the  paras must come in this order"<<endl;
//    cout<<"\nDemo: v3d -x libexample_debug.dylib -f image_thresholding -i input.tif -o output.tif -p 0 100\n"<<endl;
    return;
}
bool TipDetector::GUI_input_argu()
{
    if(p4DImage==NULL)
    {
        v3d_msg("please input img data first");
        return false;
    }
    //set update the dialog
    QDialog * dialog = new QDialog();

    if(p4DImage->getZDim() > 1 )
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
        return false;
    }

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
    thres_segment(new_nx*new_ny*new_nz,enlarged_datald,binar_datald,background_threshold);

    if (dialog)
    {
        delete dialog;
        dialog=0;
        cout<<"delete dialog"<<endl;
    }
    return true;
}
void TipDetector::getImgData(V3DPluginCallback2& callback2)
{
    // it can be  rewrite for input Image4DSimple
    v3dhandle curwin = callback2.currentImageWindow();
    if(!curwin)
    {
        v3d_msg("No image is open.");
        return ;
    }
    p4DImage = callback2.getImage(curwin);
    datald = p4DImage->getRawData();

    sz[0] = p4DImage->getXDim();
    sz[1] = p4DImage->getYDim();
    sz[2] = p4DImage->getZDim();
    sz[3] = p4DImage->getCDim();

    V3DLONG nx=sz[0],ny=sz[1],nz=sz[2];
    radiu_block=number_of_scales*step_size+minimum_scale; //the size of the block
    length_block=2*radiu_block+1;
    int size_block=length_block*length_block*length_block;


    float ang = 2*PI/number_of_rays;
    float x_dis, y_dis;
    new_nx=nx+2*radiu_block;new_ny=ny+2*radiu_block;new_nz=nz+2*radiu_block;

    try{binar_datald=new unsigned char [new_nx*new_ny*new_nz];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return ;}

    try{enlarged_datald=new unsigned char [new_nx*new_ny*new_nz];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return ;}

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

    V3DLONG num_datald=0;
    for(V3DLONG i=radiu_block;i<new_nz-radiu_block;i++)
    {
      for(V3DLONG j=radiu_block;j<new_ny-radiu_block;j++)
      {
          for(V3DLONG k=radiu_block;k<new_nx-radiu_block;k++)
          {
             enlarged_datald[i*new_nx*new_ny+j*new_nx+k]=datald[num_datald];
             num_datald++;
          }
      }
    }
    cout<<"Expand original iamge successfully"<<endl;



    try{block=new unsigned char [size_block];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return ;}

    try{xy_mip_datald=new unsigned char [nx*ny];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return ;}

    try{yz_mip_datald=new unsigned char [ny*nz];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return ;}

    try{xz_mip_datald=new unsigned char [nx*nz];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return ;}

    ray_x.resize(number_of_rays);
    ray_y.resize(number_of_rays);
    for(int i=0;i<number_of_rays;i++)
    {
        ray_x[i].resize(radiu_block);
        ray_y[i].resize(radiu_block);
    }
//    v3d_msg(QString("cereat ray_shooting model"));
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
    // in this step what we need is image

    if(1)
    {
        //ste img for tip
        cout<<"init tip_img_datald"<<endl;
        if(tip_img_datald){cout<<"error?sure tip_img_datald already exist."<<endl;delete []tip_img_datald;tip_img_datald=0;}
        try{tip_img_datald=new unsigned char [nx*ny*nz];}
        catch(...) {v3d_msg("cannot allocate memory for tip_img_datald."); return ;}
        for(V3DLONG i=0;i<nz;i++)
        {
            for(V3DLONG j=0;j<ny;j++)
            {
                for(V3DLONG k=0;k<nx;k++)
                {
                    tip_img_datald[i*nx*ny+j*nx+k]=0;
                }
            }
        }

    }


}

