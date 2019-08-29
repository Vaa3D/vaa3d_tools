#include "new_ray-shooting.h"
#include "v3d_message.h"
#include "stackutil.h"
#include "Branch_detection_plugin.h"
#include <iostream>
#include <cmath>
#include <vector>
#include "v3d_basicdatatype.h"
#include <stdio.h>
#include "new_ray-shooting.h"
#include "fstream"
#include <set>
#include <map>
#include "basic_surf_objs.h"
#include <algorithm>
#include <stack>
#include <numeric>
//#include"ClusterAnalysis.h"
using namespace std;
#include"../../../../vaa3d_tools/released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"
#define PI 3.1415926


float square(float x){return x*x;}
bool measure_one_point_radius_by_iterative_spherical_growth(unsigned char * img2D_binary, V3DLONG nx, V3DLONG ny, int thres_2d, V3DLONG X_loc, V3DLONG Y_loc, V3DLONG &adjusted_x, V3DLONG &adjusted_y, double &radius)
{
    V3DLONG adjusted_x0, adjusted_y0;
    double radius0=0.0;
    find_neighborhood_maximum_radius(X_loc,Y_loc,img2D_binary,1,adjusted_x0,adjusted_y0,radius0,nx,ny,thres_2d);
    double err;
    int err_flag = 0;
    while (err_flag<=1)
    {
       find_neighborhood_maximum_radius(adjusted_x0,adjusted_y0,img2D_binary,1,adjusted_x,adjusted_y,radius,nx,ny,thres_2d);
       err = abs(radius-radius0);
       adjusted_x0 = adjusted_x;
       adjusted_y0 = adjusted_y;
       radius0 = radius;
       if(err<=0.01)err_flag++;// for robustness, allowing one move's stopping
    }
    return true;
}
void delete_small_area(V3DLONG nx,V3DLONG ny, unsigned char * &input_image)
{

    unsigned char *original_image=new unsigned char[nx*ny];
    for(V3DLONG i=0;i<nx*ny;i++)
    {
        original_image[i]=input_image[i];
    }

    for(V3DLONG i=0;i<nx*ny;i++)
    {
        input_image[i]=0;
    }

    for(V3DLONG j=1;j<ny-1;j++)
    {
        for(V3DLONG i=1;i<nx-1;i++)
        {
            input_image[j*nx+i]=original_image[j*nx+i];

        }

    }

    V3DLONG label=1;
    for (int j = 1; j < ny - 1; j++)
    {
        for (int i = 1; i < nx - 1; i++)
        {
            if (input_image[j*nx+i] == 255)   //像素不为0
            {
                //v3d_msg(QString("creat a new stack"));
                stack <V3DLONG> neighborPixels; //新建一个栈
                neighborPixels.push(j*nx+i);     // 像素坐标: <i,j> ，以该像素为起点，寻找连通域
                label=label+1;  // 开始一个新标签，各连通域区别的标志
                while (!neighborPixels.empty())
                {

                    V3DLONG label_coordinate=neighborPixels.top();// 获取堆栈中的顶部像素并使用相同的标签对其进行标记
                    V3DLONG new_x=label_coordinate%nx;
                    V3DLONG new_y=label_coordinate/nx;
                    //cout<<"the x coordinate is"<<new_x<<" "<<"the y coordinate is"<<new_y<<"the lable is "<<label<<endl;
                    //v3d_msg(QString("the x coordinate is %1, the y coordinate is %2").arg(new_x).arg(new_y));
                    input_image[label_coordinate]=label; //对图像对应位置的点进行标记
                    neighborPixels.pop();// 弹出顶部像素   （顶部像素出栈）

                        // 加入8邻域点
                        if (input_image[new_y*nx+new_x-1] == 255)
                        {// 左点
                            neighborPixels.push(new_y*nx+new_x-1); //左边点入栈
                        }

                        if (input_image[new_y*nx+new_x+1] == 255)
                        {// 右点
                            neighborPixels.push(new_y*nx+new_x+1); //右边点入栈
                        }

                        if (input_image[(new_y-1)*nx+new_x] == 255)
                        {// 上点
                            neighborPixels.push((new_y-1)*nx+new_x); //上边点入栈
                        }

                        if (input_image[(new_y+1)*nx+new_x] == 255)
                        {// 下点
                            neighborPixels.push((new_y+1)*nx+new_x); //下边点入栈
                        }

                        if (input_image[(new_y+1)*nx+new_x-1] == 255)
                        {// 左上点
                            neighborPixels.push((new_y+1)*nx+new_x-1); //左上点入栈
                        }

                        if (input_image[(new_y+1)*nx+new_x+1] == 255)
                        {// 右上点
                            neighborPixels.push((new_y+1)*nx+new_x+1); //右上点入栈
                        }

                        if (input_image[(new_y-1)*nx+new_x+1] == 255)
                        {// 左下点
                            neighborPixels.push((new_y-1)*nx+new_x+1); //左下点入栈
                        }

                        if (input_image[(new_y-1)*nx+new_x-1] == 255)
                        {// 右下点
                            neighborPixels.push((new_y-1)*nx+new_x-1); //右下点入栈
                        }
                }
            }
        }
    }


    vector<V3DLONG> label_num;
    for(int i=2;i<=label;i++)
    {
        V3DLONG numm=0;
        for(V3DLONG num=0;num<nx*ny;num++)
        {
            if(input_image[num]==i)
            {
                 numm=numm+1;
            }

        }
        label_num.push_back(numm);
    }

    std::vector<V3DLONG>::iterator biggest=max_element(begin(label_num),end(label_num));  //find the maximum element's pointer
    int location=distance(begin(label_num),biggest);

    int derserve_lable=location+2;
    for(V3DLONG k=0;k<nx*ny;k++)
    {
        if(input_image[k]==derserve_lable)
        {
            input_image[k]=255;
        }
        else
        {
            input_image[k]=0;
        }
    }

}

void ray_shooting(int m, int n,vector<vector<float> > ray_x,vector<vector<float> > ray_y)
{

    float ang = 2*PI/m;
    float x_dis, y_dis;

    for(int i = 0; i < m; i++)
        {
            x_dis = cos(ang*(i+1));
            y_dis = sin(ang*(i+1));
        for(int j = 0; j<n; j++)
            {
                ray_x[i][j] = x_dis*(j+1);
                ray_y[i][j] = y_dis*(j+1);
            }
        }
    return;
}

int rayshooting_modle_length(int point_y,int point_x,int m,int n, vector<vector<float> > ray_x,vector<vector<float> > ray_y, unsigned char * P,V3DLONG sz0,V3DLONG sz1,int thres,double max_radiu )
{
    vector<int> adaptive_ray_length;
    float pixe = 0.0;  //initialize the pxie
    float sum=0;
    vector<float> sum_value;
    int flag=0; // if the flag =3, the candidate point is ture branch point;
    int gap_pixe=0;    // if the number of the gap more than threee,the ray should suspend;

    for(int i = 0; i <m; i++) //m is the numble of the ray should suspend
     {
        sum=0;
        gap_pixe=0;
        for(int j = 0; j < n; j++)    // n is the numble of the points of the each ray
        {
            pixe = project_interp_2d(point_y+ray_y[i][j], point_x+ray_x[i][j],P,sz0,sz1,point_x,point_y);
            sum=sum+pixe;
            if(pixe<100)
            {
                gap_pixe=gap_pixe+1;
            }
            if(gap_pixe>=3)
            //if(gap_pixe>=10)
            {
                sum_value.push_back(j+1);//cout<<j+1<<endl;//the length of each ray
                adaptive_ray_length.push_back(sum);
                break;
            }
            if(j==(n-1))
            {
                sum_value.push_back(j+gap_pixe);
                adaptive_ray_length.push_back(sum);
                break;
            }

        }
    }

    int max_value=*max_element(adaptive_ray_length.begin(),adaptive_ray_length.end());
    float max_length=*max_element(sum_value.begin(),sum_value.end());

vector<V3DLONG> detected_branch;
int raylength_threshold =0.5*max_length;
float value_threshold=0.25*max_value;
        //the length threshold of each ray

for(V3DLONG k=2;k<adaptive_ray_length.size()-2;k++)  //for 2 to 61
{
    if(sum_value[k]>(max_radiu+3))
    {
        if((adaptive_ray_length.at(k)>adaptive_ray_length.at(k-1))
                &&(adaptive_ray_length.at(k)>adaptive_ray_length.at(k+1))
                &&(adaptive_ray_length.at(k)>adaptive_ray_length.at(k+2))
                &&(adaptive_ray_length.at(k)>adaptive_ray_length.at(k-2)))
        {
            flag=flag+1;detected_branch.push_back(k);
            cout<<"The branch ind is :"<<k<<" "<<"the length is :"<<adaptive_ray_length.at(k)<<endl;
        }
    }
}

if(sum_value[62]>(max_radiu+3))
{
    if((adaptive_ray_length.at(62)>adaptive_ray_length.at(61))
            &&(adaptive_ray_length.at(62)>adaptive_ray_length.at(60))
            &&(adaptive_ray_length.at(62)>adaptive_ray_length.at(63))
            &&(adaptive_ray_length.at(62)>adaptive_ray_length.at(0)))
    {
        flag=flag+1;detected_branch.push_back(62);
        cout<<"The branch ind is :"<<62<<" "<<"the length is :"<<adaptive_ray_length.at(62)<<endl;
    }
}
if(sum_value[63]>(max_radiu+3))
{
    if((adaptive_ray_length.at(63)>adaptive_ray_length.at(62))
            &&(adaptive_ray_length.at(63)>adaptive_ray_length.at(0))
            &&(adaptive_ray_length.at(63)>adaptive_ray_length.at(1))
            &&(adaptive_ray_length.at(63)>adaptive_ray_length.at(61)))
    {
        flag=flag+1;detected_branch.push_back(63);
        cout<<"The branch ind is :"<<63<<" "<<"the length is :"<<adaptive_ray_length.at(63)<<endl;
    }
}
if(sum_value[0]>(max_radiu+3))
{
    if((adaptive_ray_length.at(0)>adaptive_ray_length.at(63))
            &&(adaptive_ray_length.at(0)>adaptive_ray_length.at(1))
            &&(adaptive_ray_length.at(0)>adaptive_ray_length.at(2))
            &&(adaptive_ray_length.at(0)>adaptive_ray_length.at(62)))
    {
        flag=flag+1;detected_branch.push_back(0);
        cout<<"The branch ind is :"<<0<<" "<<"the length is :"<<adaptive_ray_length.at(0)<<endl;
    }
}
if(sum_value[1]>(max_radiu+3))
{
    if((adaptive_ray_length.at(1)>adaptive_ray_length.at(2))
            &&(adaptive_ray_length.at(1)>adaptive_ray_length.at(3))
            &&(adaptive_ray_length.at(1)>adaptive_ray_length.at(0))
            &&(adaptive_ray_length.at(1)>adaptive_ray_length.at(63)))
    {
        flag=flag+1;detected_branch.push_back(1);
        cout<<"The branch ind is :"<<1<<" "<<"the length is :"<<adaptive_ray_length.at(1)<<endl;
    }
}


    //v3d_msg(QString("x is %1, y is %2, flag is %3").arg(point_x).arg(point_y).arg(branch_flag));

    if(flag==3)
    {
        return 1;
    }
    else
    {
        return 0;
    }

}

float project_interp_2d(float point_y,float point_x,unsigned char * PP,V3DLONG sz0,V3DLONG sz1,int old_x,int old_y)
{
    float result;
    if(point_y < 0||point_x < 0||point_y > sz1-1||point_x > sz0-1)
    {
        return 0.0;

    }
    else if(point_x < 1||point_y < 1||point_x > sz0-2||point_y > sz1-2)
    {
        result = get_2D_ValueUINT8(point_y,point_x,PP, sz0, sz1);
        return result;
    }
    else if(((int(point_x)%(int(point_x)))==0)||((int(point_y)%(int(point_y)))==0)) // fixed this bug by chaowang 2019/6/10
    {
        result = get_2D_ValueUINT8(point_y,point_x,PP, sz0, sz1);
        return result;
    }

    else
    {
        int d = get_2D_ValueUINT8(ceil(point_y), ceil(point_x),PP,sz0,sz1);  // for example ceil(1.2)=2;
        int b = get_2D_ValueUINT8(floor(point_y), ceil(point_x),PP, sz0,sz1); // for examaple floor(1.2)=1;
        int c = get_2D_ValueUINT8(ceil(point_y), floor(point_x),PP, sz0,sz1);
        int a = get_2D_ValueUINT8(floor(point_y), floor(point_x),PP, sz0,sz1);
        result=a*(ceil(point_x)-point_x)*(ceil(point_y)-point_y)+b*(ceil(point_y)-point_y)*(point_x-floor(point_x))+c*(ceil(point_x)-point_x)*(point_y-floor(point_y))+d*(point_x-floor(point_x))*(point_y-floor(point_y));
        return result;
    }
}

v3d_uint8  get_2D_ValueUINT8(V3DLONG  y,  V3DLONG  x, unsigned char * T, V3DLONG  sz0, V3DLONG  sz1)
{

    V3DLONG im_total_sz = sz0*sz1;
    V3DLONG idx = y*sz0 + x;
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


void Z_mip(V3DLONG nx,V3DLONG ny,V3DLONG nz,unsigned char * datald,unsigned char * &image_mip)
{
    for(V3DLONG iy = 0; iy < ny; iy++)
    {
        V3DLONG offsetj = iy*nx;
        for(V3DLONG ix = 0; ix < nx; ix++)
        {
            int max_mip = 0;
            for(V3DLONG iz = 0; iz < nz; iz++)
            {
                V3DLONG offsetk = iz*nx*ny;
                if(datald[offsetk + offsetj + ix] >= max_mip)
                {
                    image_mip[iy*nx + ix] = datald[offsetk + offsetj + ix];
                    max_mip = datald[offsetk + offsetj + ix];
                //    v3d_msg(QString("max_mip is %1").arg(max_mip));
                }
            }
        }
    }

}

void mip(V3DLONG nx,V3DLONG ny,V3DLONG nz,unsigned char * datald,unsigned char * &image_mip,int layer)
{
    for(V3DLONG iy = 0; iy < ny; iy++)
    {
        V3DLONG offsetj = iy*nx;
        for(V3DLONG ix = 0; ix < nx; ix++)
        {
            int max_mip = 0;
            for(V3DLONG iz = nz-layer; iz < nz+2; iz++)
            {
                V3DLONG offsetk = iz*nx*ny;
                if(datald[offsetk + offsetj + ix] >= max_mip)
                {
                    image_mip[iy*nx + ix] = datald[offsetk + offsetj + ix];
                    max_mip = datald[offsetk + offsetj + ix];
                }
            }
        }
    }

}

void mip_layer_xy(V3DLONG nx,V3DLONG ny,V3DLONG nz,unsigned char * datald,unsigned char * &image_mip,int layer)
{
    for(V3DLONG iy = 0; iy < ny; iy++)
    {
        V3DLONG offsetj = iy*nx;
        for(V3DLONG ix = 0; ix < nx; ix++)
        {
            int max_mip = 0;
            for(V3DLONG iz = nz-layer; iz <= nz+layer; iz++)
            {
                V3DLONG offsetk = iz*nx*ny;
                if(datald[offsetk + offsetj + ix] >= max_mip)
                {
                    image_mip[iy*nx + ix] = datald[offsetk + offsetj + ix];
                    max_mip = datald[offsetk + offsetj + ix];
                }
            }
        }
    }
}

void mip_layer_yz(V3DLONG nx,V3DLONG ny,V3DLONG nz,V3DLONG x_sliceloc,unsigned char * datald,unsigned char * &image_mip,int layer)
{
//    for(V3DLONG iy = 0; iy < ny; iy++)
//    {
//        V3DLONG offsetj = iy*nz;
//        for(V3DLONG iz = 0; iz < nz; iz++)
//        {
//            int max_mip = 0;
//            for(V3DLONG ix = nx-layer; ix < nx+layer; ix++)
//            {
//                V3DLONG offsetk = ix*nz*ny;
//                if(datald[offsetk + offsetj + iz] >= max_mip)
//                {
//                    image_mip[iy*nz + iz] = datald[offsetk + offsetj + iz];
//                    max_mip = datald[offsetk + offsetj + iz];
//                }
//            }
//        }
//    }
    for(V3DLONG iz = 0; iz < nz; iz++)
    {
        V3DLONG offsetk = iz*nx*ny;
        for(V3DLONG iy = 0; iy < ny; iy++)
        {
            V3DLONG offsetj = iy*nx;
            int max_mip = 0;
            for(V3DLONG ix = x_sliceloc-layer; ix <= x_sliceloc+2; ix++)
            {
                if(datald[offsetk + offsetj + ix] >= max_mip)
                {
                    image_mip[iz*ny + iy] = datald[offsetk + offsetj + ix];
                    max_mip = datald[offsetk + offsetj + ix];
                }
            }
        }
    }
}

void mip_layer_xz(V3DLONG nx,V3DLONG ny,V3DLONG nz,V3DLONG y_sliceloc,unsigned char * datald,unsigned char * &image_mip,int layer)
{
    for(V3DLONG iz = 0; iz < nz; iz++)
        {
            V3DLONG offsetk = iz*nx*ny;
            for(V3DLONG ix = 0; ix < nx; ix++)
            {
                int max_mip = 0;
                for(V3DLONG iy = y_sliceloc-layer; iy <= y_sliceloc+layer; iy++)
                {
                    V3DLONG offsetj = iy*nx;
                    if(datald[offsetk + offsetj + ix] >= max_mip)
                    {
                        image_mip[iz*nx + ix] = datald[offsetk + offsetj + ix];
                        max_mip = datald[offsetk + offsetj + ix];
                    }
                }
            }
        }
}


void mip_yz(V3DLONG nx,V3DLONG ny,V3DLONG nz,unsigned char * datald,unsigned char * &image_mip)
{
//    for(V3DLONG iz = 0; iy < nz; iz++)
//    {
//        V3DLONG offsetj = iz*nx;
//        for(V3DLONG iz = 0; iz < nz; iz++)
//        {
//            int max_mip = 0;
//            for(V3DLONG ix = nx; ix > 0; ix--)
//            {
//                V3DLONG offsetk = iz*nx*ny;
//                if(datald[offsetk + offsetj + ix] >= max_mip)
//                {
//                    image_mip[iy*nz + iz] = datald[offsetk + offsetj + ix];
//                    max_mip = datald[offsetk + offsetj + ix];
//                //    v3d_msg(QString("max_mip is %1").arg(max_mip));
//                }
//            }
//        }
//    }
    for(V3DLONG iz = 0; iz < nz; iz++)
    {
        V3DLONG offsetk = iz*nx*ny;
        for(V3DLONG iy = 0; iy < ny; iy++)
        {
            V3DLONG offsetj = iy*nx;
            int max_mip = 0;
            for(V3DLONG ix = 0; ix <nx; ix++)
            {
                if(datald[offsetk + offsetj + ix] >= max_mip)
                {
                    image_mip[iz*ny + iy] = datald[offsetk + offsetj + ix];
                    max_mip = datald[offsetk + offsetj + ix];
                }
            }
        }
    }
}

void mip_xz(V3DLONG nx,V3DLONG ny,V3DLONG nz,unsigned char * datald,unsigned char * &image_mip)
{
    for(V3DLONG iz = 0; iz < nz; iz++)
        {
            V3DLONG offsetk = iz*nx*ny;
            for(V3DLONG ix = 0; ix < nx; ix++)
            {
                int max_mip = 0;
                for(V3DLONG iy = 0; iy < ny; iy++)
                {
                    V3DLONG offsetj = iy*nx;
                    if(datald[offsetk + offsetj + ix] >= max_mip)
                    {
                        image_mip[iz*nx + ix] = datald[offsetk + offsetj + ix];
                        max_mip = datald[offsetk + offsetj + ix];
                    }
                }
            }
        }
}
void thres_segment(V3DLONG size_image, unsigned char * old_image, unsigned char * &binary_image,unsigned char thres)
{
    for(V3DLONG i = 0; i < size_image; i++)
    {
        if(old_image[i] > thres)
            binary_image[i] = 255;
        else
            binary_image[i] = 0;
    }
}
int rayinten_2D(int point_y,int point_x,int m,int n, vector<vector<float> > ray_x,vector<vector<float> > ray_y,
                vector<vector<float> > ray_x_left, vector<vector<float> > ray_y_left, vector<vector<float> > ray_x_right,
                vector<vector<float> > ray_y_right, unsigned char * P,V3DLONG sz0,V3DLONG sz1 )
{
    int result;
    float pixel = 0.0;
    float pixel_left = 0.0;
    float pixel_right = 0.0;
    float sum=0;
    float max_indd=0;
    int branch_flag=0;
    int gap_num=0;
    vector<int> ind1;//ind1 is the all piex of each ray

    for(int i = 0; i < m; i++)   //m is the numble of the ray
    {
        sum=0;
        gap_num=0;
        for(int j = 0; j < n; j++)    // n is the numble of the points of the each ray
        {
            pixel = project_interp_2d(point_y+ray_y[i][j], point_x+ray_x[i][j], P, sz0,sz1 , point_x, point_y);
            //v3d_msg(QString("x is %1, y is %2, pixe is %3").arg(point_x+ray_x[i][j]).arg(point_y+ray_y[i][j]).arg(pixel));
            pixel_left = project_interp_2d(point_y+ray_y_left[i][j], point_x+ray_x_left[i][j], P, sz0,sz1 , point_x, point_y);
            pixel_right = project_interp_2d(point_y+ray_y_right[i][j], point_x+ray_x_right[i][j], P, sz0,sz1 , point_x, point_y);
            pixel=0.80*pixel;
            pixel_left=0.10*pixel_left;
            pixel_right=0.10*pixel_right;
            sum=sum+pixel+pixel_left+pixel_right;
            if(pixel<100)
            {
                gap_num=gap_num+1;
            }
            if(gap_num>=3)
            {
                break;
            }

        }
        ind1.push_back(sum);
        cout<<"sum of each ray is :"<<sum<<endl;
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
        if((ind1.at(k)>0.33*max_indd)&&(ind1.at(k)>10*255))
        {
            if(ind1.at(k)>ind1.at(k-1)&&ind1.at(k)>ind1.at(k+1)&&ind1.at(k)>ind1.at(k+2)&&ind1.at(k)>ind1.at(k-2))
            {
                cout<<"max ray  is "<<k<<endl;
                branch_flag=branch_flag+1;
            }
        }
    }
    int size=ind1.size();
    if(ind1.at(size-1)>10*255)  // 64
    {
        if(ind1.at(size-1)>ind1.at(size-2)&&ind1.at(size-1)>ind1.at(0)&&ind1.at(size-1)>ind1.at(1)&&ind1.at(size-1)>ind1.at(size-2))
        {
            cout<<"end ray is the max pxie "<<size-1<<endl;
            branch_flag=branch_flag+1;
        }
    }

   if(ind1.at(0)>10*255)   // 1
   {
       if(ind1.at(0)>ind1.at(size-1)&&ind1.at(0)>ind1.at(1)&&ind1.at(0)>ind1.at(2)&&ind1.at(0)>ind1.at(size-2))
       {
           cout<<"start ray  is the max pixe "<<size-1<<endl;
           branch_flag=branch_flag+1;
       }

   }

   if(ind1.at(size-2)>10*255) //63
   {
       if(ind1.at(size-2)>ind1.at(size-3)&&ind1.at(size-2)>ind1.at(size-3)&&ind1.at(size-2)>ind1.at(size-1)&&ind1.at(size-2)>ind1.at(0))
       {
           cout<<"63 is the max pxie "<<size-2<<endl;
           branch_flag=branch_flag+1;
       }
   }

   if(ind1.at(1)>10*255)    // 2
   {
       if(ind1.at(1)>ind1.at(2)&&ind1.at(1)>ind1.at(3)&&ind1.at(1)>ind1.at(0)&&ind1.at(1)>ind1.at(size-1))
       {
           cout<<"2 is the max pxie "<<1<<endl;
           branch_flag=branch_flag+1;
       }
   }

    if(branch_flag==3)
    {
        result=1;
    }
    else
    {
        result=0;
    }
    return result;

}


void skeletonization(V3DLONG nx, V3DLONG ny, unsigned char * &image_binary )
{
    int neighbor[10];
    int sum_points=0;
    vector<V3DLONG> step1_delete_piont;
    vector<V3DLONG> step2_delete_piont;


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
   }
}

int candidate_points(V3DLONG nx, unsigned char * image_binary,V3DLONG point_y,V3DLONG point_x)
{
    int new_neighbor[9];
    int sum;
    int cnp;
    int result;
            if(image_binary[point_y*nx+point_x]>0)
            {
                if (image_binary[(point_y-1)*nx+point_x-1] == 255) new_neighbor[1] = 1;
                else  new_neighbor[1] = 0;
                if (image_binary[(point_y-1)*nx+point_x] == 255) new_neighbor[2] = 1;
                else  new_neighbor[2] = 0;
                if (image_binary[(point_y-1)*nx+point_x+1] == 255) new_neighbor[3] = 1;
                else  new_neighbor[3] = 0;
                if (image_binary[(point_y)*nx+point_x + 1] == 255) new_neighbor[4] = 1;
                else  new_neighbor[4] = 0;
                if (image_binary[(point_y+1)*nx+point_x+1] == 255) new_neighbor[5] = 1;
                else  new_neighbor[5] = 0;
                if (image_binary[(point_y+1)*nx+point_x] == 255) new_neighbor[6] = 1;
                else  new_neighbor[6] = 0;
                if (image_binary[(point_y+1)*nx+point_x-1] == 255) new_neighbor[7] = 1;
                else  new_neighbor[7] = 0;
                if (image_binary[(point_y)*nx+point_x-1] == 255) new_neighbor[8] = 1;
                else  new_neighbor[8] = 0;

                sum=0;
                cnp;
                for(int k=1;k<=7;k++)
                {
                    sum=sum+abs(new_neighbor[k]-new_neighbor[k+1]);
                }
                int num=abs(new_neighbor[1]-new_neighbor[8]);
                cnp=0.5*(sum+num);
                if(cnp==3)
                {
                    result=1;
                }
                else {
                    result=0;
                }
          }
            return result;



}

bool save_groundtruth_marker(string marker_file, vector<vector<int>> & out_marker, list<string> & infostring )
{
    cout<<"save"<<out_marker.size()<<"markers to file"<<marker_file<<endl;
    ofstream ofs(marker_file.c_str());
    if(ofs.fail())
    {
        cout<<"file error QAQ"<<endl;
        return false;
    }
//    list<string>::iterator it;
//    for(it=infostring.begin();it!=infostring.end();it++)
//        ofs<<*it<<endl;
//    ofs<<"#x,y,z"<<endl;
    int count=0;
    for(int i=0;i<out_marker.size();i++)
    {
       count=0;
      for(int j=0;j<out_marker[0].size();j++)
      {
          ofs<<out_marker[i][j]<<",";
          count=count+1;
          if(count==out_marker[0].size())
          {
              ofs<<endl;
              break;
          }
      }
    }
//    for(int i=0;i<out_marker.size();i++)
//    {
//        ofs<<out_marker[i].radius<<","<<endl;
//    }
    ofs.close();
    return true;
}

vector<MyMarker> readMarker_file(string marker_file)
{
    vector<MyMarker> markers; markers.reserve(1000);
    //ifstream ifs(marker_file.c_str());
#ifdef __WIN32__
#else

#endif
    ifstream ifs;
    ifs.open(marker_file);
    if(ifs.fail())
    {cout<<" unable to open marker file "<<marker_file<<endl; return markers;}
    set<MyMarker> marker_set;
    int count = 0;
    while(ifs.good())
    {
        if(ifs.peek() == '#' || ifs.eof())
        {
            ifs.ignore(1000,'\n'); continue;
        }
        MyMarker marker;
        ifs>>marker.x;
        ifs.ignore(10,',');
        ifs>>marker.y;
        ifs.ignore(10,',');
        ifs.ignore(1000,'\n');

        if(0 && marker_set.find(marker) != marker_set.end())
        {
            cout<<"omit duplicated marker"<<markers.size()<<" : x = "<<marker.x<<" y = "<<marker.y<<" z = "<<marker.z<<" r = "<<marker.radius<<endl;
        }
        else
        {
            markers.push_back(marker);
            marker_set.insert(marker);
            if(0) cout<<"marker"<<markers.size()<<" : x = "<<marker.x<<" y = "<<marker.y<<" z = "<<marker.z<<" r = "<<marker.radius<<endl;
        }
        count++;
    }
    cout<<count<<" markers loaded"<<endl;
    ifs.close();
    return markers;
}

bool find_neighborhood_maximum(V3DLONG source_point_x,V3DLONG source_point_y, float * phi, int window_size, int &changed_x_location, int &changed_y_location,V3DLONG nx,V3DLONG ny)
{

   vector<float> distance_set;
   int location;
   int window_length=2*window_size+1;
   int center_length=window_size;
   int changed_x;
   int changed_y;
   changed_x_location=0;
   changed_y_location=0;
    for(int column=-window_size;column<=window_size;column++)
    {
        for(int row=-window_size;row<=window_size;row++)
        {
            float distance_value=phi[(source_point_y+column)*nx+source_point_x+row];
            distance_set.push_back(distance_value);

        }
    }
    std::vector<float>::iterator biggest=std::max_element(std::begin(distance_set),std::end(distance_set));
    location=std::distance(std::begin(distance_set),biggest);
    changed_x=location%window_length-center_length;
    changed_y=location/window_length-center_length;
    changed_x_location=source_point_x+changed_x;
    changed_y_location=source_point_y+changed_y;
    distance_set.clear();
    return true;
}
template<class T> double markerRadius_hanchuan_XY(T* &inimg1d, V3DLONG nx, V3DLONG ny, V3DLONG x,V3DLONG y, double thresh)
{
    //printf("markerRadius_hanchuan   XY 2D\n");
    int radius_prec=1; // the precision of the estimated radius

    long sz0 = nx;
    double max_r = nx/2;
    if (max_r > ny/2) max_r = ny/2;

    double total_num, background_num;
    double ir;
    for (ir=1; ir<=max_r; ir=ir+radius_prec)
    {
        total_num = background_num = 0;

        //double dz, dy, dx;
        double dx,dy;
        //double zlower = 0, zupper = 0;
        //for (dz= zlower; dz <= zupper; ++dz)
            for (dy= -ir; dy <= +ir; dy=dy+radius_prec)
                for (dx= -ir; dx <= +ir; dx=dx+radius_prec)
                {
                    total_num++;
                    //double r = sqrt(dx*dx + dy*dy + dz*dz);
                    double r = sqrt(dx*dx + dy*dy);
                    if (r>ir-1 && r<=ir)
                    {
                        V3DLONG i = x+dx;   if (i<0 || i>=nx) goto end1;
                        V3DLONG j = y+dy;   if (j<0 || j>=ny) goto end1;
                        //V3DLONG k = marker.z+dz;   if (k<0 || k>=sz[2]) goto end1;
                        if (inimg1d[j * sz0 + i] <= thresh)
                        {
                            background_num++;

                            if ((background_num/total_num) > 0.001) goto end1; //change 0.01 to 0.001 on 100104
                        }
                    }
                }
    }
end1:
    return ir;

}

bool find_neighborhood_maximum_radius(V3DLONG source_point_x,V3DLONG source_point_y, unsigned char * datald_2D, int window_size, V3DLONG &changed_x_location, V3DLONG &changed_y_location,double &max_radius,V3DLONG nx,V3DLONG ny ,int thres)
{
    vector<double> Radiu_value;
    double radius;
    int location;
    int window_length=2*window_size+1;
    int center_length=window_size;
    int changed_x;
    int changed_y;
    changed_x_location=0;
    changed_y_location=0;
    for(int column=-window_size;column<=window_size;column++)
    {
        for(int row=-window_size;row<=window_size;row++)
        {
            radius=markerRadius_hanchuan_XY(datald_2D,nx,ny,source_point_x+row,source_point_y+column,thres);
            Radiu_value.push_back(radius);
        }
    }
    std::vector<double>::iterator biggest=max_element(begin(Radiu_value),end(Radiu_value));  //find the maximum element's pointer
    max_radius=*biggest;
    location=distance(begin(Radiu_value),biggest);
    changed_x=location%window_length-center_length;
    changed_y=location/window_length-center_length;
    changed_x_location=source_point_x+changed_x;   //adjusted x coordinate;
    changed_y_location=source_point_y+changed_y;   //adjusted y coordiante;
    Radiu_value.clear();
    return true;
}
void rorate_method(int point_y,int point_x,int model_size,double modle_length,  vector<vector<float> > ray_x,vector<vector<float> > ray_y, unsigned char * P,V3DLONG nx,V3DLONG ny,vector<float> & x_loc, vector<float> & y_loc)
{
    int ray_numbers_2d=model_size;
    double max_radiu=modle_length;

    for(int i = 0; i <ray_numbers_2d; i++)   //m is the numble of the ray
    {
        for(int j = max_radiu+3; j < max_radiu+6; j++)    // n is the numble of the points of the each ray
        {
            double pixe = project_interp_2d((point_y)+ray_y[i][j], (point_x)+ray_x[i][j], P, nx,ny , point_x, point_y);
            if(pixe>200)
            {
                x_loc.push_back(point_x+ray_x[i][j]);
                y_loc.push_back(point_y+ray_y[i][j]);
            }

        }

    }
}

void seek_2D_candidate_points(V3DLONG nx,V3DLONG ny, unsigned char *image_binary,vector<V3DLONG> &x_loc,vector<V3DLONG> &y_loc,int &count)
{
    count=0;
    x_loc.clear();
    y_loc.clear();
    int new_neighbor[10];
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
                    if((cnp==3))     //  if cnp==3 the (i,j) point is the 2D candidate point.
                    {
                        x_loc.push_back(i);
                        y_loc.push_back(j);
                        count=count+1;
                    }

            }
        }
   }
}

void seek_2D_candidate_points_vessel(V3DLONG nx,V3DLONG ny, unsigned char *image_binary,vector<V3DLONG> &x_loc,vector<V3DLONG> &y_loc,int &count)
{
    count=0;
    x_loc.clear();
    y_loc.clear();
    int new_neighbor[10];
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
                    if((cnp==3)||(cnp==4))     //  if cnp==3 the (i,j) point is the 2D candidate point.
                    {
                        x_loc.push_back(i);
                        y_loc.push_back(j);
                        count=count+1;
                    }

            }
        }
   }
}
