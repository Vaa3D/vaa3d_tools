/* Cross_detection_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-5-15 : by Changhao Guo
 */
 
#include "v3d_message.h"
#include <vector>
#include "Cross_detection_plugin.h"
#include "new_ray-shooting.h"
//#include "ClusterAnalysis.h"
#include "ClusterAnalysis_20190409.h"
#include "branch_detection_model.h"


using namespace std;
Q_EXPORT_PLUGIN2(Cross_detection, Cross_detection_Plugin);
 
QStringList Cross_detection_Plugin::menulist() const
{
	return QStringList() 
        <<tr("check_One_crossPoint_2D")
        <<tr("detect_crossPoints_2D")
        <<tr("detect_crossPoints_2_5D")
        <<tr("show_find_radius")
        <<tr("test")
		<<tr("about");
}

QStringList Cross_detection_Plugin::funclist() const
{
	return QStringList()
        <<tr("detect_crossPoints_2D")
        <<tr("detect_crossPoints_3D")
		<<tr("help");
}

void Cross_detection_Plugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("check_One_crossPoint_2D"))
	{
        int flag = check_One_crossPoint_2D(callback,parent);
        if(flag)
        {
            cout<<endl;
            cout<<"***********************************"<<endl;
            cout<<"*check_One_crossPoint_2D finished!*"<<endl;
            cout<<"***********************************"<<endl;
        }
        else
            return;
	}
    else if (menu_name == tr("detect_crossPoints_2D"))
    {
        int flag = detect_crossPoints_2D(callback,parent);
        if(flag)
        {
            cout<<endl;
            cout<<"*********************************"<<endl;
            cout<<"*detect_crossPoints_2D finished!*"<<endl;
            cout<<"*********************************"<<endl;
        }
        else
            return;
    }
    else if (menu_name == tr("detect_crossPoints_2_5D"))
	{
        int flag = detect_crossPoints_2_5D(callback,parent);
        if(flag)
        {
            cout<<endl;
            cout<<"*********************************"<<endl;
            cout<<"*detect_crossPoints_2_5D finished!*"<<endl;
            cout<<"*********************************"<<endl;
        }
        else
            return;
	}
    else if (menu_name == tr("show_find_radius"))
    {
        int flag = show_find_radius(callback,parent);
        if(flag)
        {
            cout<<endl;
            cout<<"****************************"<<endl;
            cout<<"*show_find_radius finished!*"<<endl;
            cout<<"****************************"<<endl;
        }
        else
            return;
    }
    else if (menu_name == tr("test"))
    {
        int flag = test(callback,parent);
        if(flag)
        {
            cout<<endl;
            cout<<"****************"<<endl;
            cout<<"*test finished!*"<<endl;
            cout<<"****************"<<endl;
        }
        else
            return;
    }
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Changhao Guo, 2019-5-15"));
	}
}

bool Cross_detection_Plugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if(func_name==tr("detect_crossPoints_2D"))
    {
        return detect_crossPoints_2D(callback,input,output,parent);
    }
    else if(func_name==tr("detect_crossPoints_3D"))
    {
        return detect_crossPoints_3D(callback,input,output,parent);
    }
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

bool writeMarker_file(const QString & filename, LandmarkList & listMarker)
{
    FILE * fp = fopen(filename.toLatin1(), "wt");
    if (!fp)
    {
        return false;
    }

    fprintf(fp, "##x,y,z,radius,shape,name,comment, color_r,color_g,color_b\n");

    for (int i=0;i<listMarker.size(); i++)
    {
        LocationSimple  p_pt;
        p_pt = listMarker[i];
        fprintf(fp, "%5.3f, %5.3f, %5.3f, %d, %d, %s, %s, %d,%d,%d\n",
                // 090617 RZC: marker file is 1-based
                p_pt.x,
                p_pt.y,
                p_pt.z,
                int(p_pt.radius), p_pt.shape,
                p_pt.name, p_pt.comments,
                p_pt.color.r,p_pt.color.g,p_pt.color.b);
    }

    fclose(fp);
    return true;
}

void displayMarker(V3DPluginCallback2 &callback, v3dhandle windows, LandmarkList calcuMarker)
{
    if(calcuMarker.size())
    {
        LandmarkList MarkerforDisplaying;
        LocationSimple temMarker;
        for(V3DLONG i = 0; i < calcuMarker.size(); i++)
        {
            temMarker.x = calcuMarker.at(i).x+1;
            temMarker.y = calcuMarker.at(i).y+1;
            temMarker.z = calcuMarker.at(i).z+1;
            temMarker.radius = calcuMarker.at(i).radius;
            temMarker.shape = calcuMarker.at(i).shape;
            temMarker.name = calcuMarker.at(i).name;
            temMarker.comments = calcuMarker.at(i).comments;
            temMarker.category = calcuMarker.at(i).category;
            temMarker.color = calcuMarker.at(i).color;
            temMarker.ave = calcuMarker.at(i).ave;
            temMarker.sdev = calcuMarker.at(i).sdev;
            temMarker.skew = calcuMarker.at(i).skew;
            temMarker.curt = calcuMarker.at(i).curt;
            temMarker.on = calcuMarker.at(i).on;
            MarkerforDisplaying.push_back(temMarker);
        }

        callback.setLandmark(windows, MarkerforDisplaying);
    }
    else
    {
        v3d_msg(QString("the marker.size is 0. No marker can be displayed."));
    }

}

int check_One_crossPoint_2D(V3DPluginCallback2 &callback, QWidget *parent)
{
    ClusterAnalysis mycluster;
    Branch_Detection_Model pp;
    pp.Get_Image_Date(callback);
    pp.Gui_input_Rorate_display_Method();

    int x_location=pp.x_coordinate, y_location=pp.y_coordinate,thres_2d=pp.thres_2d,ray_numbers_2d=pp.ray_numbers_2d;
    int based_distance=pp.based_distance,window_size=pp.window_size;

    V3DLONG nx=pp.sz[0],ny=pp.sz[1],nz=pp.sz[2];

    Image4DSimple *p4DImage = pp.p4DImage;
    V3DLONG size_image=pp.size_2D_image;

    unsigned char* data1d=0;
    data1d=pp.data1d;

     unsigned char *data1d_mip;
     try{data1d_mip=new unsigned char [size_image];}
     catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

     unsigned char *seg_data1d_mip;
     try{seg_data1d_mip=new unsigned char [size_image];}
     catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

     Z_mip(nx,ny,nz,data1d,data1d_mip);
     thres_segment(nx*ny,data1d_mip,seg_data1d_mip,thres_2d);

     RGBA8 red; red.r=255; red.g=0; red.b=0;
     RGBA8 green; green.r=0; green.g=255; green.b=0;
     RGBA8 blue; blue.r=0; blue.g=0; blue.b=255;

     LandmarkList curlist; curlist.clear();
     LocationSimple s;

     /* using Spherical growth method designed by PHC*/
     double radius;
     V3DLONG adjusted_x, adjusted_y;

     find_neighborhood_maximum_radius(x_location,y_location,seg_data1d_mip,window_size,adjusted_x,adjusted_y,radius,nx,ny,thres_2d);
     cout<<"x is :"<<adjusted_x<<" "<<"y is :"<<adjusted_y<<" "<<"the radius  is :"<<radius<<endl;


     vector<vector<float> > ray_x(ray_numbers_2d,vector<float>(100)), ray_y(ray_numbers_2d,vector<float>(100));
     float ang = 2*PI/ray_numbers_2d;
     float x_dis, y_dis;

     for(int i = 0; i < ray_numbers_2d; i++)
     {
        x_dis = cos(ang*(i+1));
        y_dis = sin(ang*(i+1));
        for(int j = 0; j<50 ; j++)
            {
                ray_x[i][j] = x_dis*(j+1);
                ray_y[i][j] = y_dis*(j+1);
            }
     }
     vector<float> x_loc;
     vector<float> y_loc;
     vector<float> dis_x_loc;
     vector<float> dis_y_loc;
     vector<float> Pixe;
    double soma_radius=20;
    if(radius<soma_radius)
    {
        v3d_msg(QString("x is %1, y is %2, The pixe of source point is %3").arg(x_location).arg(y_location).arg(data1d_mip[(y_location)*nx+x_location]));
        for(int i = 0; i <ray_numbers_2d; i++)   //m is the numble of the ray
        {
            for(int j = radius+based_distance; j < radius+based_distance+5; j++)    // n is the numble of the points of the each ray
            {
                double pixe = project_interp_2d((adjusted_y)+ray_y[i][j], (adjusted_x)+ray_x[i][j], seg_data1d_mip, nx,ny , (adjusted_x), (adjusted_y));
                if(pixe>=100)
                {
                    x_loc.push_back(adjusted_x+ray_x[i][j]);
                    y_loc.push_back(adjusted_y+ray_y[i][j]);
                }
                dis_x_loc.push_back(adjusted_x+ray_x[i][j]);
                dis_y_loc.push_back(adjusted_y+ray_y[i][j]);
                Pixe.push_back(pixe);
            }
        }
        float change_x1=cos(ang*3)*(radius+based_distance+5)-cos(ang*2)*(radius+based_distance+5);
        float change_x=pow(change_x1,2);
        float change_y1=sin(ang*3)*(radius+based_distance+5)-sin(ang*2)*(radius+based_distance+5);
        float change_y=pow(change_y1,2);
        float DB_radius=sqrt(change_x+change_y)+1;
        cout<<"the DB radius is:"<<DB_radius<<endl;
        mycluster.Read_from_coordiante(x_loc,y_loc,DB_radius,3);
        int flag=mycluster.DoDBSCANRecursive();
        v3d_msg(QString("the cluster is %1").arg(flag),0);
        if(flag>3)
        {
            v3d_msg(QString("Cluster flag is %1. This point may be a crossPoint.").arg(flag));
        }

        for(V3DLONG i = 0; i < dis_x_loc.size(); i++)
        {
            s.x=  dis_x_loc[i];
            s.y = dis_y_loc[i];
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

    }
    else
    {
        v3d_msg(QString("radius>=soma_radius. The point may be in the soma."));
    }





     Image4DSimple * new4DImage = new Image4DSimple();
     new4DImage->setData((unsigned char *)seg_data1d_mip, nx, ny, 1, p4DImage->getCDim(), p4DImage->getDatatype());
     v3dhandle newwin = callback.newImageWindow();
     callback.setImage(newwin, new4DImage);
     callback.updateImageWindow(newwin);
     displayMarker(callback, newwin, curlist);
     return 1;

}


int detect_crossPoints_2D(V3DPluginCallback2 &callback, QWidget *parent)
{
    /* define the color */
    RGBA8 red; red.r=255; red.g=0; red.b=0;
    RGBA8 green; green.r=0; green.g=51; green.b=0;
    RGBA8 blue; blue.r=0; blue.g=0; blue.b=255;
    RGBA8 white; white.r=255; white.g=255; white.b=255;
    RGBA8 pink; pink.r=255; pink.g=105; pink.b=180;
    RGBA8 yellow; yellow.r=255; yellow.g=255; yellow.b=0;

    ClusterAnalysis mycluster;
    Branch_Detection_Model pp;
    pp.Get_Image_Date(callback);
    pp.Gui_input_Rorate_Method();

    // set the parameter
    int thres_2d=pp.thres_2d;
    int window_size=pp.window_size;
    int ray_numbers_2d=pp.ray_numbers_2d;
    int based_distance=pp.based_distance;
    double soma_radius=20;

    V3DLONG nx=pp.sz[0];
    V3DLONG ny=pp.sz[1];
    V3DLONG nz=pp.sz[2];

    Image4DSimple *p4DImage = pp.p4DImage;
    V3DLONG size_image=pp.size_2D_image;

    unsigned char* data1d=0;
    data1d=pp.data1d;

    unsigned char *data1d_mip;
    try{data1d_mip=new unsigned char [size_image];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}


    unsigned char *image_binary;
    try{image_binary=new unsigned char [size_image];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

    unsigned char *old_image_binary=0;
    try{old_image_binary=new unsigned char [size_image];}
    catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

    Z_mip(nx,ny,nz,data1d,data1d_mip);
    thres_segment(nx*ny,data1d_mip,image_binary,thres_2d);

    for(V3DLONG i=0;i<nx*ny;i++)
    {
        old_image_binary[i]=image_binary[i];
    }

    int new_neighbor[10];
    LandmarkList curlist;
    LocationSimple s;
    vector<V3DLONG> X_candidate;
    vector<V3DLONG> Y_candidate;
    vector<float> Max_value;

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
            double radius;
            find_neighborhood_maximum_radius(X_loc.at(i),Y_loc.at(i),old_image_binary,window_size,adjusted_x,adjusted_y,radius,nx,ny,thres_2d);

            if(radius<soma_radius)
            {
                X_candidate.push_back(adjusted_x);  // all adjusted x coordinate of candidate points
                Y_candidate.push_back(adjusted_y);  // all adjusted y coordinate of candidate points
                Max_value.push_back(radius);    // all radius coordinate of candidate points
//            cout<<"x is :"<<adjusted_x<<" "<<"y is :"<<adjusted_y<<" "<<"the radius  is :"<<radius<<endl;
            }
            else
            {
                v3d_msg(QString("radius>=soma_radius. The point may be in the soma."),0);
            }
        }
    }
    else {
        cout<<"no candidate points in this MIP"<<endl;
    }


    cout<<"the all candidate points have detected"<<endl;

    if(!X_candidate.size()){cout<<"no cross candidate points."<<endl;}

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


    for(V3DLONG k = 0; k < X_candidate.size(); k++)
    {
        vector<float> x_loc_outer ;
        vector<float> y_loc_outer ;
//        for(int i = 0; i <ray_numbers_2d; i++)   //m is the numble of the ray
//        {
//            for(int j = Max_value.at(k)+based_distance; j < Max_value.at(k)+based_distance+3; j++)    // n is the numble of the points of the each ray
//            {
//                double pixe = project_interp_2d(Y_candidate.at(k)+ray_y[i][j], X_candidate.at(k)+ray_x[i][j], old_image_binary, nx,ny ,  X_candidate.at(k),  Y_candidate.at(k));
//                if(pixe>=100)
//                {
//                    x_loc_outer.push_back(X_candidate.at(k)+ray_x[i][j]);
//                    y_loc_outer.push_back(Y_candidate.at(k)+ray_y[i][j]);
//                }
//            }
//        }

        vector<float> x_loc_entire ;
        vector<float> y_loc_entire ;
        for(int i = 0; i <ray_numbers_2d; i++)   //m is the numble of the ray
        {
            for(int j = 0; j < Max_value.at(k)+based_distance+5; j++)    // n is the numble of the points of the each ray
            {
                double pixe = project_interp_2d(Y_candidate.at(k)+ray_y[i][j], X_candidate.at(k)+ray_x[i][j], old_image_binary, nx,ny ,  X_candidate.at(k),  Y_candidate.at(k));
                if(pixe>=100)
                {
                    x_loc_entire.push_back(X_candidate.at(k)+ray_x[i][j]);
                    y_loc_entire.push_back(Y_candidate.at(k)+ray_y[i][j]);
                    if((j>=Max_value.at(k)+based_distance)&&(j<Max_value.at(k)+based_distance+5))
                    {
                        x_loc_outer.push_back(X_candidate.at(k)+ray_x[i][j]);
                        y_loc_outer.push_back(Y_candidate.at(k)+ray_y[i][j]);
                    }
                }
            }
        }

        /* calculate the DBscan radius */
        float change_x1=cos(ang*3)*(Max_value.at(k)+based_distance+5)-cos(ang*2)*(Max_value.at(k)+based_distance+5);
        float change_x=pow(change_x1,2);
        float change_y1=sin(ang*3)*(Max_value.at(k)+based_distance+5)-sin(ang*2)*(Max_value.at(k)+based_distance+5);
        float change_y=pow(change_y1,2);
        float DB_radius=sqrt(change_x+change_y)+1;
        mycluster.Read_from_coordiante(x_loc_outer ,y_loc_outer ,DB_radius,3);
        int flag_outer=mycluster.DoDBSCANRecursive();
//        mycluster.Read_from_coordiante(x_loc_entire ,y_loc_entire ,DB_radius,3);//after use very slow, and will reduce some true positives
//        int flag_entire=mycluster.DoDBSCANRecursive();
//        cout<<"flag_outer:"<<flag_outer<<"  flag_entire:"<<flag_entire<<endl;

        if(flag_outer>=4)//flag_entire==3)
        {
             s.x=  X_candidate[k];
             s.y = Y_candidate[k];
             s.z = 1;
             s.radius = 1;
             s.color = red;
             curlist<<s;
        }

    }
    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *)old_image_binary, nx, ny, 1, p4DImage->getCDim(), p4DImage->getDatatype());
    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, new4DImage);
    callback.updateImageWindow(newwin);
    displayMarker(callback, newwin, curlist);
    return 1;
}

int detect_crossPoints_2_5D(V3DPluginCallback2 &callback, QWidget *parent)
{
    /* define the color */
    RGBA8 red; red.r=255; red.g=0; red.b=0;
    RGBA8 green; green.r=0; green.g=51; green.b=0;
    RGBA8 blue; blue.r=0; blue.g=0; blue.b=255;
    RGBA8 white; white.r=255; white.g=255; white.b=255;
    RGBA8 pink; pink.r=255; pink.g=105; pink.b=180;
    RGBA8 yellow; yellow.r=255; yellow.g=255; yellow.b=0;

    ClusterAnalysis mycluster;

    Branch_Detection_Model pp;
    pp.Get_Image_Date(callback);
    pp.Gui_input_CrossPoint_3D_Method();

    // set the parameter
    int thres_2d=pp.thres_2d;
    int window_size=pp.window_size;
    int ray_numbers_2d=pp.ray_numbers_2d;
    int based_distance=pp.based_distance;
    int num_layer=pp.num_layer;
    double soma_radius=pp.soma_radius;

    V3DLONG nx=pp.sz[0];
    V3DLONG ny=pp.sz[1];
    V3DLONG nz=pp.sz[2];

    Image4DSimple *p4DImage = pp.p4DImage;
    V3DLONG size_image=pp.size_2D_image;
    V3DLONG size_3D_image=pp.size_3D_image;
    unsigned char* data1d=0;
    data1d=pp.data1d;

    unsigned char* data1d_copy;
    unsigned char *image_mip;
    unsigned char *image_binary;
    unsigned char *old_image_binary;


    try{data1d_copy=new unsigned char [size_3D_image];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}
    for(V3DLONG i = 0; i < size_3D_image; i++)
    {data1d_copy[i]=data1d[i];}

    try{image_mip=new unsigned char [size_image];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

    try{image_binary=new unsigned char [size_image];}
    catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

    try{old_image_binary=new unsigned char [size_image];}
    catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

    cout<<"allocate the memory is successful"<<endl;

    /* create the ratate matrix */
    vector<vector<float> > ray_x(ray_numbers_2d,vector<float>(150)), ray_y(ray_numbers_2d,vector<float>(150));
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

    LandmarkList curlist;
    LocationSimple s;
    vector<int> each_layer_PointNum;
    for(int size_z=num_layer;size_z<nz-num_layer;size_z+=MAX(1,num_layer/2))
    {      
        /* detect the 2D branch points in each MIP */
        if((size_z+num_layer)<nz)
        {
           int count_crossPoint=0;
           mip(nx,ny,size_z,data1d,image_mip,num_layer);
           thres_segment(size_image,image_mip,image_binary,thres_2d);
           for(V3DLONG a=0;a<nx*ny;a++)
           {
               old_image_binary[a]=image_binary[a];
           }
           skeletonization(nx,ny,image_binary);
           vector<V3DLONG> X_candidate;  // save the adjusted x coordinate;
           vector<V3DLONG> Y_candidate;  // save the adjusted y coordinate;
           vector<float> Max_value;      // save the radius;
           int count=0; // the number of candidate
           vector<V3DLONG> X_loc; //save the non-adjusted x coordinate;
           vector<V3DLONG> Y_loc; //save the non-adjusted y coordinate;
           seek_2D_candidate_points(nx,ny,image_binary,X_loc,Y_loc,count);
           if(count>0)
           {
               for(int i=0;i<count;i++)
               {
                   if(((X_loc.at(i)-window_size)<=0)||((Y_loc.at(i)-window_size)<=0)||((Y_loc.at(i)+window_size)>=ny)||((X_loc.at(i)+window_size)>=nx))
                   {
                       continue;
                   }
                   /* using Spherical growth method designed by Chaowang*/  //loop until finding the real max radius by guochanghao in 20190520
                   V3DLONG adjusted_x0, adjusted_y0;
                   double radius0;
                   find_neighborhood_maximum_radius(X_loc.at(i),Y_loc.at(i),old_image_binary,window_size,adjusted_x0,adjusted_y0,radius0,nx,ny,thres_2d);

                   V3DLONG adjusted_x, adjusted_y;
                   double radius;
                   double err = 1000;
                   int err_flag = 0;
                   while (err_flag<=1)
                   {
                       find_neighborhood_maximum_radius(adjusted_x0,adjusted_y0,old_image_binary,window_size,adjusted_x,adjusted_y,radius,nx,ny,thres_2d);
                       err = abs(radius-radius0);
                       adjusted_x0 = adjusted_x;
                       adjusted_y0 = adjusted_y;
                       radius0 = radius;
                       if(err<=0.01)err_flag++;// for robustness, allowing one move's stopping
                   }

                   if(radius<soma_radius)
                    {
                        X_candidate.push_back(adjusted_x);  // all adjusted x coordinate of candidate points
                        Y_candidate.push_back(adjusted_y);  // all adjusted y coordinate of candidate points
                        Max_value.push_back(radius);    // all radius coordinate of candidate points
//                   cout<<"x is :"<<adjusted_x<<" "<<"y is :"<<adjusted_y<<" "<<"the radius  is :"<<radius<<endl;
                    }
                    else
                    {
                        v3d_msg(QString("radius>=soma_radius. The point in laye%1 may be in the soma.").arg(size_z),0);
                    }
               }
           }
           else {
               cout<<"no candidate points in this MIP"<<endl;
           }
//                   cout<<"the all candidate points in each MIP have detected"<<endl;

           for(V3DLONG k = 0; k < X_candidate.size(); k++)
           {
               vector<float> x_loc_entire, x_loc_outer;
               vector<float> y_loc_entire, y_loc_outer;

               for(int i = 0; i <ray_numbers_2d; i++)   //m is the numble of the ray
               {
                   for(int j = 0; j < Max_value.at(k)+based_distance+3; j++)    // n is the numble of the points of the each ray
                   {
                       double pixe = project_interp_2d(Y_candidate.at(k)+ray_y[i][j], X_candidate.at(k)+ray_x[i][j], old_image_binary, nx,ny ,  X_candidate.at(k),  Y_candidate.at(k));
                       if(pixe>=100)
                       {
                           x_loc_entire.push_back(X_candidate.at(k)+ray_x[i][j]);
                           y_loc_entire.push_back(Y_candidate.at(k)+ray_y[i][j]);
                           if((j>=Max_value.at(k)+based_distance)&&(j<Max_value.at(k)+based_distance+3))
                           {
                               x_loc_outer.push_back(X_candidate.at(k)+ray_x[i][j]);
                               y_loc_outer.push_back(Y_candidate.at(k)+ray_y[i][j]);
                           }
                       }
                   }
               }

               /* calculate the DBscan radius */
               float change_x1=cos(ang*3)*(Max_value.at(k)+based_distance+3)-cos(ang*2)*(Max_value.at(k)+based_distance+3);
               float change_x=pow(change_x1,2);
               float change_y1=sin(ang*3)*(Max_value.at(k)+based_distance+3)-sin(ang*2)*(Max_value.at(k)+based_distance+3);
               float change_y=pow(change_y1,2);
               float DB_radius=sqrt(change_x+change_y)+1;
               mycluster.Read_from_coordiante(x_loc_outer,y_loc_outer,DB_radius,3);
               int flag_outer=mycluster.DoDBSCANRecursive();
//               mycluster.Read_from_coordiante(x_loc_entire,y_loc_entire,DB_radius,3);
//               int flag_entire=mycluster.DoDBSCANRecursive();


               if(flag_outer>=4)//-flag_entire>=3)
               {
                    s.x=  X_candidate[k];
                    s.y = Y_candidate[k];
                    s.z = size_z;
                    s.radius = 1;
                    s.color=red;
                    curlist<<s;
                    count_crossPoint++;
               }
           }
           if(count_crossPoint){each_layer_PointNum.push_back(count_crossPoint);}
        }
    }

    /** show markers in different color in different layer **/
    int color_increase = 255/(each_layer_PointNum.size());
    int curr_color = 0;
    int count_layer = 0;
    for(V3DLONG t=0; t<each_layer_PointNum.size(); t++)
    {
        for(V3DLONG i=count_layer; i<count_layer+each_layer_PointNum.at(t); i++)
        {
            curlist[i].color.g+=curr_color;
        }
        curr_color+=color_increase;
        count_layer += each_layer_PointNum.at(t);
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


//    for(double i=0;i<curlist.size();i++)
//    {
//        for(double j=i+1;j<curlist.size();j++)
//        {
//            if(square(curlist[j].x-curlist[i].x)+square(curlist[j].y-curlist[i].y)+square(curlist[j].z-curlist[i].z)<200)
//            {
//                curlist[i].x=(curlist[j].x+curlist[i].x)/2;
//                curlist[i].y=(curlist[j].y+curlist[i].y)/2;
//                curlist[i].z=(curlist[j].z+curlist[i].z)/2;
//                curlist.removeAt(j);
//                j=j-1;
//            }
//        }
//    }

    double plane_thresold=10;
    double axis_threshold=2*num_layer;
    bool flag_while_xy = true;
    while(flag_while_xy)
    {
        bool endwhile = true;
        for(V3DLONG i=0;i<curlist.size();i++)
        {
            for(V3DLONG j=i+1;j<curlist.size();j++)
            {
                if((abs(curlist[j].x-curlist[i].x)+abs(curlist[j].y-curlist[i].y))<plane_thresold&&(abs(curlist[j].z-curlist[i].z))<axis_threshold)
                {
                    curlist[i].x=(curlist[j].x+curlist[i].x)/2;
                    curlist[i].y=(curlist[j].y+curlist[i].y)/2;
                    curlist[i].z=(curlist[j].z+curlist[i].z)/2;
                    curlist.removeAt(j);
                    j=j-1;
                    endwhile = false;
                }
            }
        }
        if(endwhile){flag_while_xy = false;}
    }

    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *)data1d_copy, nx, ny, nz, p4DImage->getCDim(), p4DImage->getDatatype());
    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, new4DImage);
    callback.updateImageWindow(newwin);
    displayMarker(callback, newwin, curlist);


    return 1;
}




bool detect_crossPoints_2D(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent)
{
    /* define the color */
    RGBA8 red; red.r=255; red.g=0; red.b=0;
    RGBA8 green; green.r=0; green.g=51; green.b=0;
    RGBA8 blue; blue.r=0; blue.g=0; blue.b=255;
    RGBA8 white; white.r=255; white.g=255; white.b=255;
    RGBA8 pink; pink.r=255; pink.g=105; pink.b=180;
    RGBA8 yellow; yellow.r=255; yellow.g=255; yellow.b=0;

    ClusterAnalysis mycluster;

    // set the parameter
    int thres_2d =30;
    int window_size=3;
    int ray_numbers_2d=64;
    int based_distance=5;

    vector<char*> *pinfiles=(input.size()>=1)?(vector<char*> *) input[0].p : 0;
    vector<char*> * poutfiles = (output.size() >= 1) ? (vector<char*> *) output[0].p : 0;
    vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
    vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
    vector<char*> outfiles = (poutfiles != 0) ? * poutfiles : vector<char*>();
    vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

    cout << "infiles: " << infiles[0] << endl;
    cout << "outfiles[0]" << outfiles[0] << endl;
    cout << "paras[0]" << paras[0] << endl;

    thres_2d = atoi(paras.at(0));
    char * inimg_file =  infiles[0];
    char * out_marker =  outfiles[0];
    Image4DSimple *p4DImage = callback.loadImage(inimg_file);
    unsigned char* data1d = p4DImage->getRawData();  
    double soma_radius=20;

    V3DLONG sz[4];
    sz[0]=p4DImage->getXDim();
    sz[1]=p4DImage->getYDim();
    sz[2]=p4DImage->getZDim();
    sz[3]=p4DImage->getCDim();

    V3DLONG nx=sz[0];
    V3DLONG ny=sz[1];
    V3DLONG nz=sz[2];

    V3DLONG size_image = nx*ny;

    unsigned char *data1d_mip;
    try{data1d_mip=new unsigned char [size_image];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

    unsigned char *image_binary;
    try{image_binary=new unsigned char [size_image];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

    unsigned char *old_image_binary=0;
    try{old_image_binary=new unsigned char [size_image];}
    catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

    cout<<"allocate the memory is successful"<<endl;

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

    Z_mip(nx,ny,nz,data1d,data1d_mip);
    thres_segment(nx*ny,data1d_mip,image_binary,thres_2d);

    for(V3DLONG i=0;i<nx*ny;i++)
    {
        old_image_binary[i]=image_binary[i];
    }



    int new_neighbor[10];
    LandmarkList curlist;
    LocationSimple s;
    vector<V3DLONG> X_candidate;
    vector<V3DLONG> Y_candidate;
    vector<float> Max_value;

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
            double radius;
            find_neighborhood_maximum_radius(X_loc.at(i),Y_loc.at(i),old_image_binary,window_size,adjusted_x,adjusted_y,radius,nx,ny,thres_2d);
            if(radius<soma_radius)
            {
                X_candidate.push_back(adjusted_x);  // all adjusted x coordinate of candidate points
                Y_candidate.push_back(adjusted_y);  // all adjusted y coordinate of candidate points
                Max_value.push_back(radius);    // all radius coordinate of candidate points
//            cout<<"x is :"<<adjusted_x<<" "<<"y is :"<<adjusted_y<<" "<<"the radius  is :"<<radius<<endl;

            }
            else
            {
                v3d_msg(QString("radius>=soma_radius. The point may be in the soma."),0);
            }
        }
    }
    else {
        cout<<"no candidate points in this MIP"<<endl;
    }

    cout<<"the all candidate points have detected"<<endl;

    if(!X_candidate.size()){cout<<"no cross candidate points."<<endl;}

    for(V3DLONG k = 0; k < X_candidate.size(); k++)
    {
        vector<float> x_loc_outer ;
        vector<float> y_loc_outer ;
//        for(int i = 0; i <ray_numbers_2d; i++)   //m is the numble of the ray
//        {
//            for(int j = Max_value.at(k)+based_distance; j < Max_value.at(k)+based_distance+3; j++)    // n is the numble of the points of the each ray
//            {
//                double pixe = project_interp_2d(Y_candidate.at(k)+ray_y[i][j], X_candidate.at(k)+ray_x[i][j], old_image_binary, nx,ny ,  X_candidate.at(k),  Y_candidate.at(k));
//                if(pixe>=100)
//                {
//                    x_loc_outer.push_back(X_candidate.at(k)+ray_x[i][j]);
//                    y_loc_outer.push_back(Y_candidate.at(k)+ray_y[i][j]);
//                }
//            }
//        }



        vector<float> x_loc_entire ;
        vector<float> y_loc_entire ;
        for(int i = 0; i <ray_numbers_2d; i++)   //m is the numble of the ray
        {
            for(int j = 0; j < Max_value.at(k)+based_distance+5; j++)    // n is the numble of the points of the each ray
            {
                double pixe = project_interp_2d(Y_candidate.at(k)+ray_y[i][j], X_candidate.at(k)+ray_x[i][j], old_image_binary, nx,ny ,  X_candidate.at(k),  Y_candidate.at(k));
                if(pixe>=100)
                {
                    x_loc_entire.push_back(X_candidate.at(k)+ray_x[i][j]);
                    y_loc_entire.push_back(Y_candidate.at(k)+ray_y[i][j]);
                    if((j>=Max_value.at(k)+based_distance)&&(j<Max_value.at(k)+based_distance+5))
                    {
                        x_loc_outer.push_back(X_candidate.at(k)+ray_x[i][j]);
                        y_loc_outer.push_back(Y_candidate.at(k)+ray_y[i][j]);
                    }
                }
            }
        }

        /* calculate the DBscan radius */
        float change_x1=cos(ang*3)*(Max_value.at(k)+based_distance+5)-cos(ang*2)*(Max_value.at(k)+based_distance+5);
        float change_x=pow(change_x1,2);
        float change_y1=sin(ang*3)*(Max_value.at(k)+based_distance+5)-sin(ang*2)*(Max_value.at(k)+based_distance+5);
        float change_y=pow(change_y1,2);
        float DB_radius=sqrt(change_x+change_y)+1;
        mycluster.Read_from_coordiante(x_loc_outer ,y_loc_outer ,DB_radius,3);
        int flag_outer=mycluster.DoDBSCANRecursive();
//        mycluster.Read_from_coordiante(x_loc_entire ,y_loc_entire ,DB_radius,3);//after use very slow, and will reduce some true positives
//        int flag_entire=mycluster.DoDBSCANRecursive();
//        cout<<"flag_outer:"<<flag_outer<<"  flag_entire:"<<flag_entire<<endl;

        if(flag_outer>=4)//flag_entire==3)
        {
             s.x=  X_candidate[k];
             s.y = Y_candidate[k];
             s.z = 1;
             s.radius = 1;
             s.color = red;
             curlist<<s;
        }

    }
    vector<MyMarker> outmarkers;
    for(V3DLONG i = 0; i < curlist.size(); i++)
    {
        MyMarker tmp;
        tmp.x = curlist[i].x;
        tmp.y = curlist[i].y;
        tmp.z = curlist[i].z;
        tmp.radius = curlist[i].radius;
        outmarkers.push_back(tmp);
    }
//    saveMarker_file(out_marker, outmarkers);
    writeMarker_file(out_marker, curlist);
    return true;
}

bool detect_crossPoints_3D(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent)
{
    /* define the color */
    RGBA8 red; red.r=255; red.g=0; red.b=0;
    RGBA8 green; green.r=0; green.g=51; green.b=0;
    RGBA8 blue; blue.r=0; blue.g=0; blue.b=255;
    RGBA8 white; white.r=255; white.g=255; white.b=255;
    RGBA8 pink; pink.r=255; pink.g=105; pink.b=180;
    RGBA8 yellow; yellow.r=255; yellow.g=255; yellow.b=0;

    ClusterAnalysis mycluster;

    // set the parameter
    int thres_2d =30;
    int window_size=3;
    int ray_numbers_2d=64;
    int based_distance=5;
    int num_layer = 6;
    double soma_radius=10;

    vector<char*> *pinfiles=(input.size()>=1)?(vector<char*> *) input[0].p : 0;
    vector<char*> * poutfiles = (output.size() >= 1) ? (vector<char*> *) output[0].p : 0;
    vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
    vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
    vector<char*> outfiles = (poutfiles != 0) ? * poutfiles : vector<char*>();
    vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

    cout << "infiles: " << infiles[0] << endl;
    cout << "outfiles[0]" << outfiles[0] << endl;
    cout << "paras[0]" << paras[0] << endl;

    thres_2d = atoi(paras.at(0));
    char * inimg_file =  infiles[0];
    char * out_marker =  outfiles[0];
    Image4DSimple *p4DImage = callback.loadImage(inimg_file);
    unsigned char* data1d = p4DImage->getRawData();

    V3DLONG sz[4];
    sz[0]=p4DImage->getXDim();
    sz[1]=p4DImage->getYDim();
    sz[2]=p4DImage->getZDim();
    sz[3]=p4DImage->getCDim();

    V3DLONG nx=sz[0];
    V3DLONG ny=sz[1];
    V3DLONG nz=sz[2];

    V3DLONG size_image = nx*ny;
    V3DLONG size_3D_image=nx*ny*nz;

    unsigned char* data1d_copy;
    unsigned char *image_mip;
    unsigned char *image_binary;
    unsigned char *old_image_binary;


    try{data1d_copy=new unsigned char [size_3D_image];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}
    for(V3DLONG i = 0; i < size_3D_image; i++)
    {data1d_copy[i]=data1d[i];}

    try{image_mip=new unsigned char [size_image];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

    try{image_binary=new unsigned char [size_image];}
    catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

    try{old_image_binary=new unsigned char [size_image];}
    catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

    cout<<"allocate the memory is successful"<<endl;

    /* create the ratate matrix */
    vector<vector<float> > ray_x(ray_numbers_2d,vector<float>(150)), ray_y(ray_numbers_2d,vector<float>(150));
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

    LandmarkList curlist;
    LocationSimple s;
    vector<int> each_layer_PointNum;
    for(int size_z=num_layer;size_z<nz-num_layer;size_z+=num_layer/2)
    {
        /* detect the 2D branch points in each MIP */
        if((size_z+num_layer)<nz)
        {
           int count_crossPoint=0;
           mip(nx,ny,size_z,data1d,image_mip,num_layer);
           thres_segment(size_image,image_mip,image_binary,thres_2d);
           for(V3DLONG a=0;a<nx*ny;a++)
           {
               old_image_binary[a]=image_binary[a];
           }
           skeletonization(nx,ny,image_binary);
           vector<V3DLONG> X_candidate;  // save the adjusted x coordinate;
           vector<V3DLONG> Y_candidate;  // save the adjusted y coordinate;
           vector<float> Max_value;      // save the radius;
           int count=0; // the number of candidate
           vector<V3DLONG> X_loc; //save the non-adjusted x coordinate;
           vector<V3DLONG> Y_loc; //save the non-adjusted y coordinate;
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
                   double radius;
                   find_neighborhood_maximum_radius(X_loc.at(i),Y_loc.at(i),old_image_binary,window_size,adjusted_x,adjusted_y,radius,nx,ny,thres_2d);
                   if(radius<soma_radius)
                   {
                       X_candidate.push_back(adjusted_x);  // all adjusted x coordinate of candidate points
                       Y_candidate.push_back(adjusted_y);  // all adjusted y coordinate of candidate points
                       Max_value.push_back(radius);    // all radius coordinate of candidate points
//                   cout<<"x is :"<<adjusted_x<<" "<<"y is :"<<adjusted_y<<" "<<"the radius  is :"<<radius<<endl;
                   }
                   else
                   {
//                       v3d_msg(QString("radius>=soma_radius. The point in laye%1 may be in the soma.").arg(size_z),0);
                   }
               }
           }
           else {
               cout<<"no candidate points in this MIP"<<endl;
           }
//                   cout<<"the all candidate points in each MIP have detected"<<endl;
           for(V3DLONG k = 0; k < X_candidate.size(); k++)
           {
               vector<float> x_loc_entire, x_loc_outer;
               vector<float> y_loc_entire, y_loc_outer;

               for(int i = 0; i <ray_numbers_2d; i++)   //m is the numble of the ray
               {
                   for(int j = 0; j < Max_value.at(k)+based_distance+5; j++)    // n is the numble of the points of the each ray
                   {
                       double pixe = project_interp_2d(Y_candidate.at(k)+ray_y[i][j], X_candidate.at(k)+ray_x[i][j], old_image_binary, nx,ny ,  X_candidate.at(k),  Y_candidate.at(k));
                       if(pixe>=100)
                       {
                           x_loc_entire.push_back(X_candidate.at(k)+ray_x[i][j]);
                           y_loc_entire.push_back(Y_candidate.at(k)+ray_y[i][j]);
                           if((j>=Max_value.at(k)+based_distance)&&(j<Max_value.at(k)+based_distance+5))
                           {
                               x_loc_outer.push_back(X_candidate.at(k)+ray_x[i][j]);
                               y_loc_outer.push_back(Y_candidate.at(k)+ray_y[i][j]);
                           }
                       }
                   }
               }

               /* calculate the DBscan radius */
               float change_x1=cos(ang*3)*(Max_value.at(k)+based_distance+5)-cos(ang*2)*(Max_value.at(k)+based_distance+5);
               float change_x=pow(change_x1,2);
               float change_y1=sin(ang*3)*(Max_value.at(k)+based_distance+5)-sin(ang*2)*(Max_value.at(k)+based_distance+5);
               float change_y=pow(change_y1,2);
               float DB_radius=sqrt(change_x+change_y)+1;
               mycluster.Read_from_coordiante(x_loc_outer,y_loc_outer,DB_radius,3);
               int flag_outer=mycluster.DoDBSCANRecursive();
//               mycluster.Read_from_coordiante(x_loc_entire,y_loc_entire,DB_radius,3);
//               int flag_entire=mycluster.DoDBSCANRecursive();


               if(flag_outer>=4)//-flag_entire>=3)
               {
                    s.x=  X_candidate[k];
                    s.y = Y_candidate[k];
                    s.z = size_z;
                    s.radius = 1;
                    s.color = red;
                    curlist<<s;
                    count_crossPoint++;
               }
           }
           if(count_crossPoint){each_layer_PointNum.push_back(count_crossPoint);}
        }
    }

    /** show markers in different color in different layer **/
    int color_increase = 255/(each_layer_PointNum.size());
    int curr_color = 0;
    int count_layer = 0;
    for(V3DLONG t=0; t<each_layer_PointNum.size(); t++)
    {
        for(V3DLONG i=count_layer; i<count_layer+each_layer_PointNum.at(t); i++)
        {
            curlist[i].color.g+=curr_color;
        }
        curr_color+=color_increase;
        count_layer += each_layer_PointNum.at(t);
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

    vector<MyMarker> outmarkers;
    for(V3DLONG i = 0; i < curlist.size(); i++)
    {
        MyMarker tmp;
        tmp.x = curlist[i].x;
        tmp.y = curlist[i].y;
        tmp.z = curlist[i].z;
        tmp.radius = curlist[i].radius;
        outmarkers.push_back(tmp);
    }
//    saveMarker_file(out_marker, outmarkers);
    writeMarker_file(out_marker, curlist);
    return true;
}

int show_find_radius(V3DPluginCallback2 &callback, QWidget *parent)
{
    /* define the color */
    RGBA8 red; red.r=255; red.g=0; red.b=0;
    RGBA8 green; green.r=0; green.g=51; green.b=0;
    RGBA8 blue; blue.r=0; blue.g=0; blue.b=255;
    RGBA8 white; white.r=255; white.g=255; white.b=255;
    RGBA8 pink; pink.r=255; pink.g=105; pink.b=180;
    RGBA8 yellow; yellow.r=255; yellow.g=255; yellow.b=0;

    ClusterAnalysis mycluster;

    Branch_Detection_Model pp;
    pp.Get_Image_Date(callback);
    pp.Gui_input_Rorate_3D_Method();

    // set the parameter
    int thres_2d=pp.thres_2d;
    int window_size=pp.window_size;
    int ray_numbers_2d=pp.ray_numbers_2d;
    int num_layer=pp.num_layer;
    double soma_radius=20;

    V3DLONG nx=pp.sz[0];
    V3DLONG ny=pp.sz[1];
    V3DLONG nz=pp.sz[2];

    Image4DSimple *p4DImage = pp.p4DImage;
    V3DLONG size_image=pp.size_2D_image;
    V3DLONG size_3D_image=pp.size_3D_image;
    unsigned char* data1d=0;
    data1d=pp.data1d;

    unsigned char* data1d_copy;
    unsigned char *image_mip;
    unsigned char *image_binary;


    try{data1d_copy=new unsigned char [size_3D_image];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}
    for(V3DLONG i = 0; i < size_3D_image; i++)
    {data1d_copy[i]=data1d[i];}

    try{image_mip=new unsigned char [size_image];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

    try{image_binary=new unsigned char [size_image];}
    catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

    cout<<"allocate the memory is successful"<<endl;

    v3dhandle curwin = callback.currentImageWindow();
    LandmarkList curlist = callback.getLandmark(curwin);

    mip(nx,ny,curlist.at(0).z,data1d,image_mip,num_layer);
    thres_segment(size_image,image_mip,image_binary,thres_2d);

    LocationSimple s;
    LandmarkList nextP;



    V3DLONG adjusted_x0 = curlist.at(0).x;
    V3DLONG adjusted_y0 = curlist.at(0).y;
    V3DLONG adjusted_z0 = curlist.at(0).z;
    double radius0;
    s.x = adjusted_x0; s.y = adjusted_y0; s.z =1; s.color = red;
    nextP.push_back(s);

    V3DLONG adjusted_x, adjusted_y;
    double radius;
    find_neighborhood_maximum_radius(adjusted_x0,adjusted_y0,image_binary,window_size,adjusted_x,adjusted_y,radius0,nx,ny,thres_2d);

    s.x = adjusted_x; s.y = adjusted_y; s.z =1; s.color.g+=70;
    nextP.push_back(s);
    double err = radius0; //cout<<"radius0:"<<radius0<<"  err:"<<err<<endl;
    int err_flag = 0;
    while (err_flag<=1)
    {
        adjusted_x0 = adjusted_x;
        adjusted_y0 = adjusted_y;
        find_neighborhood_maximum_radius(adjusted_x0,adjusted_y0,image_binary,window_size,adjusted_x,adjusted_y,radius,nx,ny,thres_2d);
        err = abs(radius-radius0); //cout<<"*radius0:"<<radius0<<"  radius:"<<radius<<"  err:"<<err<<endl;
        s.x = adjusted_x; s.y = adjusted_y; s.z =1;s.color.g+=70;
        nextP.push_back(s);
        radius0=radius;
        if(err<=0.01)err_flag++;
    }



    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *)image_binary, nx, ny, 1, p4DImage->getCDim(), p4DImage->getDatatype());
    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, new4DImage);
    callback.updateImageWindow(newwin);
    displayMarker(callback, newwin, nextP);


    return 1;
}

//vector<int> valid_layer;
//find_valid_layer(data1d_copy, nx, ny, nz, thres_2d, valid_layer);
//v3d_msg(QString("find valid layer."),0);
//if(valid_layer.at(size_z)==0)
//{  continue;}
//else
//{v3d_msg(QString("valid layer:%1").arg(size_z),0);}

int test(V3DPluginCallback2 &callback, QWidget *parent)
{

    /* define the color */
    RGBA8 red; red.r=255; red.g=0; red.b=0;
    RGBA8 green; green.r=0; green.g=51; green.b=0;
    RGBA8 blue; blue.r=0; blue.g=0; blue.b=255;
    RGBA8 white; white.r=255; white.g=255; white.b=255;
    RGBA8 pink; pink.r=255; pink.g=105; pink.b=180;
    RGBA8 yellow; yellow.r=255; yellow.g=255; yellow.b=0;

    ClusterAnalysis mycluster;

    Branch_Detection_Model pp;
    pp.Get_Image_Date(callback);
    pp.Gui_input_CrossPoint_3D_Method();

    // set the parameter
    int thres_2d=pp.thres_2d;
    int window_size=pp.window_size;
    int ray_numbers_2d=pp.ray_numbers_2d;
    int based_distance=pp.based_distance;
    int num_layer=pp.num_layer;
    double soma_radius=pp.soma_radius;

    V3DLONG nx=pp.sz[0];
    V3DLONG ny=pp.sz[1];
    V3DLONG nz=pp.sz[2];

    Image4DSimple *p4DImage = pp.p4DImage;
    V3DLONG size_image=pp.size_2D_image;
    V3DLONG size_3D_image=pp.size_3D_image;
    unsigned char* data1d=0;
    data1d=pp.data1d;

    unsigned char* data1d_copy;
    unsigned char *image_mip;
    unsigned char *image_binary;
    unsigned char *old_image_binary;


    try{data1d_copy=new unsigned char [size_3D_image];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}
    for(V3DLONG i = 0; i < size_3D_image; i++)
    {data1d_copy[i]=data1d[i];}

    try{image_mip=new unsigned char [size_image];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}

    try{image_binary=new unsigned char [size_image];}
    catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

    try{old_image_binary=new unsigned char [size_image];}
    catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}

    cout<<"allocate the memory is successful"<<endl;

    /* create the ratate matrix */
    vector<vector<float> > ray_x(ray_numbers_2d,vector<float>(150)), ray_y(ray_numbers_2d,vector<float>(150));
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

    LandmarkList curlist;
    LocationSimple s;
    vector<int> each_layer_PointNum;
    for(int size_z=num_layer;size_z<nz-num_layer;size_z+=MAX(1,num_layer/2))
    {
        /* detect the 2D branch points in each MIP */
        if((size_z+num_layer)<nz)
        {
           int count_crossPoint=0;
           mip(nx,ny,size_z,data1d,image_mip,num_layer);
           thres_segment(size_image,image_mip,image_binary,thres_2d);cout<<"test0"<<endl;
           for(V3DLONG a=0;a<nx*ny;a++)
           {
               old_image_binary[a]=image_binary[a];
           }
           skeletonization(nx,ny,image_binary);
           vector<V3DLONG> X_candidate;  // save the adjusted x coordinate;
           vector<V3DLONG> Y_candidate;  // save the adjusted y coordinate;
           vector<float> Max_value;      // save the radius;
           int count=0; // the number of candidate
           vector<V3DLONG> X_loc; //save the non-adjusted x coordinate;
           vector<V3DLONG> Y_loc; //save the non-adjusted y coordinate;
           seek_2D_candidate_points(nx,ny,image_binary,X_loc,Y_loc,count);cout<<"test1"<<endl;
           if(count>0)
           {
               for(int i=0;i<count;i++)
               {
                   if(((X_loc.at(i)-window_size)<0)||((Y_loc.at(i)-window_size)<0)||((Y_loc.at(i)+window_size)>=ny)||((X_loc.at(i)+window_size)>=nx))
                   {
                       continue;
                   }
                   /* using Spherical growth method designed by Chaowang*/  //loop until finding the real max radius by guochanghao in 20190520
                   V3DLONG adjusted_x0, adjusted_y0;
                   double radius0;
                   find_neighborhood_maximum_radius(X_loc.at(i),Y_loc.at(i),old_image_binary,window_size,adjusted_x0,adjusted_y0,radius0,nx,ny,thres_2d);

                   V3DLONG adjusted_x, adjusted_y;cout<<"test2"<<endl;
                   double radius;
                   double err = 1000;
                   int err_flag = 0;
                   while (err_flag<=1)
                   {
                       find_neighborhood_maximum_radius(adjusted_x0,adjusted_y0,old_image_binary,window_size,adjusted_x,adjusted_y,radius,nx,ny,thres_2d);
                       err = abs(radius-radius0);
                       adjusted_x0 = adjusted_x;
                       adjusted_y0 = adjusted_y;
                       radius0 = radius;
                       if(err<=0.01)err_flag++;// for robustness, allowing one move's stopping
                   }

                   if(radius<soma_radius)
                    {
                        X_candidate.push_back(adjusted_x);  // all adjusted x coordinate of candidate points
                        Y_candidate.push_back(adjusted_y);  // all adjusted y coordinate of candidate points
                        Max_value.push_back(radius);    // all radius coordinate of candidate points
//                   cout<<"x is :"<<adjusted_x<<" "<<"y is :"<<adjusted_y<<" "<<"the radius  is :"<<radius<<endl;
                    }
                    else
                    {
                        v3d_msg(QString("radius>=soma_radius. The point in laye%1 may be in the soma.").arg(size_z),0);
                    }
               }
           }
           else {
               cout<<"no candidate points in this MIP"<<endl;
           }
//                   cout<<"the all candidate points in each MIP have detected"<<endl;

           for(V3DLONG k = 0; k < X_candidate.size(); k++)
           {
               vector<float> x_loc_entire, x_loc_outer;
               vector<float> y_loc_entire, y_loc_outer;

               for(int i = 0; i <ray_numbers_2d; i++)   //m is the numble of the ray
               {
                   for(int j = 0; j < Max_value.at(k)+based_distance+3; j++)    // n is the numble of the points of the each ray
                   {
                       int py=Y_candidate.at(k)+ray_y[i][j]; int px=X_candidate.at(k)+ray_x[i][j];
                       if(px<0||px>=nx||py<0||py>=ny)
                           continue;cout<<"test3"<<endl;
                       double pixe = project_interp_2d(Y_candidate.at(k)+ray_y[i][j], X_candidate.at(k)+ray_x[i][j], old_image_binary, nx,ny ,  X_candidate.at(k),  Y_candidate.at(k));
                       cout<<"test4"<<endl;
                       if(pixe>=100)
                       {
                           x_loc_entire.push_back(X_candidate.at(k)+ray_x[i][j]);
                           y_loc_entire.push_back(Y_candidate.at(k)+ray_y[i][j]);
                           if((j>=Max_value.at(k)+based_distance)&&(j<Max_value.at(k)+based_distance+3))
                           {
                               x_loc_outer.push_back(X_candidate.at(k)+ray_x[i][j]);
                               y_loc_outer.push_back(Y_candidate.at(k)+ray_y[i][j]);
                           }
                       }
                   }
               }

               /* calculate the DBscan radius */
               float change_x1=cos(ang*3)*(Max_value.at(k)+based_distance+3)-cos(ang*2)*(Max_value.at(k)+based_distance+3);
               float change_x=pow(change_x1,2);
               float change_y1=sin(ang*3)*(Max_value.at(k)+based_distance+3)-sin(ang*2)*(Max_value.at(k)+based_distance+3);
               float change_y=pow(change_y1,2);
               float DB_radius=sqrt(change_x+change_y)+1;
               mycluster.Read_from_coordiante(x_loc_outer,y_loc_outer,DB_radius,3);cout<<"test5"<<endl;
               int flag_outer=mycluster.DoDBSCANRecursive();
//               mycluster.Read_from_coordiante(x_loc_entire,y_loc_entire,DB_radius,3);
//               int flag_entire=mycluster.DoDBSCANRecursive();


               if(flag_outer>=4)//-flag_entire>=3)
               {
                    s.x=  X_candidate[k];
                    s.y = Y_candidate[k];
                    s.z = size_z;
                    s.radius = 1;
                    s.color=red;
                    curlist<<s;
                    count_crossPoint++;
               }
           }
           if(count_crossPoint){each_layer_PointNum.push_back(count_crossPoint);}
        }
    }

    /** show markers in different color in different layer **/
    int color_increase = 255/(each_layer_PointNum.size());
    int curr_color = 0;
    int count_layer = 0;
    for(V3DLONG t=0; t<each_layer_PointNum.size(); t++)
    {
        for(V3DLONG i=count_layer; i<count_layer+each_layer_PointNum.at(t); i++)
        {
            curlist[i].color.g+=curr_color;
        }
        curr_color+=color_increase;
        count_layer += each_layer_PointNum.at(t);
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


//    for(double i=0;i<curlist.size();i++)
//    {
//        for(double j=i+1;j<curlist.size();j++)
//        {
//            if(square(curlist[j].x-curlist[i].x)+square(curlist[j].y-curlist[i].y)+square(curlist[j].z-curlist[i].z)<200)
//            {
//                curlist[i].x=(curlist[j].x+curlist[i].x)/2;
//                curlist[i].y=(curlist[j].y+curlist[i].y)/2;
//                curlist[i].z=(curlist[j].z+curlist[i].z)/2;
//                curlist.removeAt(j);
//                j=j-1;
//            }
//        }
//    }

    double plane_thresold=10;
    double axis_threshold=2*num_layer;
    bool flag_while_xy = true;
    while(flag_while_xy)
    {
        bool endwhile = true;
        for(V3DLONG i=0;i<curlist.size();i++)
        {
            for(V3DLONG j=i+1;j<curlist.size();j++)
            {
                if((abs(curlist[j].x-curlist[i].x)+abs(curlist[j].y-curlist[i].y))<plane_thresold&&(abs(curlist[j].z-curlist[i].z))<axis_threshold)
                {
                    curlist[i].x=(curlist[j].x+curlist[i].x)/2;
                    curlist[i].y=(curlist[j].y+curlist[i].y)/2;
                    curlist[i].z=(curlist[j].z+curlist[i].z)/2;
                    curlist.removeAt(j);
                    j=j-1;
                    endwhile = false;
                }
            }
        }
        if(endwhile){flag_while_xy = false;}
    }

    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *)data1d_copy, nx, ny, nz, p4DImage->getCDim(), p4DImage->getDatatype());
    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, new4DImage);
    callback.updateImageWindow(newwin);
    displayMarker(callback, newwin, curlist);


    return 1;
}
