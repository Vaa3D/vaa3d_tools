#include "v3d_message.h"
#include "stackutil.h"
#include "Branch_c_plugin.h"
#include <iostream>
#include <math.h>
#include <vector>
#include "v3d_basicdatatype.h"
#include <stdio.h>
#include "rayshooting_func.h"   



using namespace std;
#define PI 3.1415926
#define EXP 2.71828


float square(float x){return x*x;}

int rayinten_2D_multiscale(int point_x,int point_y,int m,int scale, int T1, vector<vector<float> > ray_x,vector<vector<float> > ray_y, unsigned char * P,int sz0,int sz1)
{

    int point[2] = {point_x,point_y};
    int branch_flag1=0;
    int branch_flag;
    //int  point_coordinate[3];
    float pixe = 0.0;
    int n;
    int iz_distance;
    int count;
    int cflag;
    int size_zflag;
    int ray_distance;
    float sum,sum1,px,py,px1,py1,dist;
    float ang = 2*PI/64;
    float Ta=0.039;
    //vector<int> indd[m],ind1[m];
    vector<int> indd,ind1,zflag,branchpoint;
    for(int k=scale-6; k<=scale; k=k+2)
    {
        indd.clear();
        ind1.clear();
        zflag.clear();
        iz_distance=0;
        count=0;
    for(int i = 0; i < m; i++)   //m is the numble of the ray
        {
         sum = 0;
         sum1 = 0;
         n=k;
         size_zflag=0;
         cflag=0;
         for(int j = 0; j<n; j++)   // n is the numble of the points of the each ray
            {
             float offsety;
             float offsetx;
             offsetx=ray_x[i][j];
             offsety=ray_y[i][j];
             pixe = 0.0;
                 if(abs(ray_x[i][j])<0.0001)
                 {
                     int offsetx1=0;
                     int offsety1=ray_y[i][j];
//                     pixe = interp_2d(point[0]+offsetx1, point[1]+offsety1, P, sz0,sz1 );
                       pixe=get_2D_ValueUINT8(point[0]+offsetx1,point[1]+offsety1,P, sz0, sz1);
//                     cout<<point[0]+offsetx1<<","<<point[1]+offsety1<<pixe<<endl;
//                     v3d_msg("chech1");
                 }
                 else if(abs(ray_y[i][j])<0.0001)
                 {
                     int offsety1=0;
                     int offsetx1=ray_x[i][j];
                     pixe=get_2D_ValueUINT8(point[0]+offsetx1,point[1]+offsety1,P, sz0, sz1);
                 }
                 else
                 {
                     pixe = interp_2d(point[0]+offsetx, point[1]+offsety, P, sz0,sz1 );
                 }
                 //v3d_msg(QString("indli is %1").arg(pixe));
                 if(pixe<50)
                 {
                     zflag.push_back(j);
                     size_zflag=size_zflag+1;
                 }
                 if(size_zflag>1)
                 {
                     iz_distance=zflag[size_zflag-1]-zflag[size_zflag-2];
                     if(iz_distance==1)
                         cflag=1;
                 }
                 //v3d_msg(QString("pixe is %1").arg(pixe));
                 if(!cflag)
                 {
                     pixe = pixe*pow(EXP,(0.05*(j-n)));
                 }
//                     cout<<pixe<<","<<ray_x[i][j]<<","<<ray_y[i][j]<<"\t";
//                     cout<<pixe<<","<<"\t";
                 else {break;}
                  sum = sum+pixe;
            }
//     cout<<endl;
           sum1 = sum/n;
           ind1.push_back(sum1);
          //ind1[i]=sum; //i is the numble of the ray
           //v3d_msg(QString("indli is %1").arg(ind1[i]));
        }
//    v3d_msg("chech1");
    for (int i = 0;i < m;i++)
    {
        if (ind1[i] >= T1)
            {
                //indd[count] = i;
                 indd.push_back(i);
                 count++;
            }
    }

    // v3d_msg(QString("count is %1, scale is %2").arg(count).arg(n));
    // float dis=0,max_dis=0;
    ray_distance=0;
    branch_flag=0;
    branchpoint.clear();
    int c_flag=2;
if(count>2)
{
    for(int i=0;i<count-1;i++)
    {
        px=cos(indd[i]*ang);
        py=sin(indd[i]*ang);
        px1=cos(indd[i+1]*ang);
        py1=sin(indd[i+1]*ang);

        dist=square(px-px1)+square(py-py1);
        c_flag--;
        if(dist>Ta&&c_flag<=0)
        {
            branch_flag=branch_flag+1;
//            cout<<branch_flag<<","<<i+1<<endl;
            branchpoint.push_back(i+1);
            c_flag=2;
        }
//        cout<<dist<<","<<square(2.1*sin(ang/2))<<endl;
//cout<<dist<<" ,  "<<Ta<<endl;
    }
//    cout<<branchpoint.size()<<endl;
    px=cos(indd[0]*ang);
    py=sin(indd[0]*ang);
    px1=cos(indd[count-1]*ang);
    py1=sin(indd[count-1]*ang);
    dist=square(px-px1)+square(py-py1);
    if(dist>Ta&&branchpoint.size()>0)
    {
        int indb=branchpoint.back();
        int bfcount=abs(indd[indb]-indd[count-1])+1;
        if(bfcount>=2)
        {branch_flag=branch_flag+1;}
    }
    else if(branchpoint.size()>0)
    {
        int indb=branchpoint.back();
            int bfcount=abs(indd[indb]-indd[count-1])+1;
            px=cos(indd[0]*ang);
            py=sin(indd[0]*ang);
            px1=cos(indd[1]*ang);
            py1=sin(indd[1]*ang);
            dist=square(px-px1)+square(py-py1);
            if(bfcount>=2&&dist>Ta)
            {
                branch_flag=branch_flag+1;
            }
    }
}

//    if (count > 2)
//        {
//            for (int i = 0;i < count-1 ;i++)
//            {
//              ray_distance=abs(indd[i]-indd[i+1]);
//                   if(ray_distance>=2)
//                   {
//                      branch_flag=branch_flag+1;
//                   }
//            }
//            long dis=indd[1]-indd[count];
//            if(abs(dis)<=(m-2))
//                branch_flag=branch_flag-1;
//         }
for (int i = 0;i < count;i++)
{
//    cout<<i<<","<<indd[i]<<endl;
}
    if (branch_flag==3)
    {
        branch_flag1=branch_flag1+1;
    }
//    cout<<branch_flag<<","<<branch_flag1<<endl;
//    v3d_msg("chech2");
    //v3d_msg(QString("branch flag is %1").arg(branch_flag));
    }  
    //cout<<branch_flag1<<endl;
    if(branch_flag1>1)
        return 1;
    else
        return 0;

}

float interp_2d(float point_x,float point_y,unsigned char * PP,int sz0,int sz1)
{
    float result;

    if(point_x < 0||point_y < 0||point_x > sz0-1||point_y > sz1-1)
        return 0.0;
    else if(point_x < 1||point_y < 1||point_x > sz0-2||point_y > sz1-2)
    {
        result = get_2D_ValueUINT8(point_x,point_y,PP, sz0, sz1);
        return result;
    }
    else
    {
        int y1 = get_2D_ValueUINT8(ceil(point_x), ceil(point_y),PP,sz0,sz1);
        int y0 = get_2D_ValueUINT8(floor(point_x), ceil(point_y),PP, sz0,sz1);
        int x1 = get_2D_ValueUINT8(ceil(point_x), floor(point_y),PP, sz0,sz1);
        int x0 = get_2D_ValueUINT8(floor(point_x), floor(point_y),PP, sz0,sz1);

        result = x0*(point_x-floor(point_x))*(point_y-floor(point_y))+x1*(ceil(point_x)-point_x)*(point_y-floor(point_y))
            +y0*(point_x-floor(point_x))*(ceil(point_y)-point_y)+y1*(ceil(point_x)-point_x)*(ceil(point_y)-point_y);
        return result;
    }
}

v3d_uint8  get_2D_ValueUINT8(int  x,  int  y, unsigned char * T, int  sz0, int  sz1)
{

    int im_total_sz = sz0*sz1;
    int idx = y*sz0 + x;
    //cout<<im_total_sz<<","<<idx<<endl;
    if (idx < im_total_sz)
    {
        return T[idx];//是否要用this指针

    }
    else
    {
        printf("Image4DSimple::getIntensity() error: index exceeds the image size");
        return 0;
    }
}

