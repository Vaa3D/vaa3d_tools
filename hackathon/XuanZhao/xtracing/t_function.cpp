#include "t_function.h"
#include <iostream>
#include <math.h>
#define VOID 1000000000
#define PI 3.14159265359
#define dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))
#define anglez(a,b) (acos(((b).z-(a).z)/dist(a,b))*180.0/PI)
#define anglex(a,b) (acos(((b).x-(a).x)/sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)))*180.0/PI)
#define angley(a,b) (acos(((b).y-(a).y)/sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)))*180.0/PI)
#define trans(a) ((a/180)*PI)


V3DLONG threeToOne(V3DLONG x,V3DLONG y,V3DLONG z,V3DLONG sz0,V3DLONG sz1,V3DLONG sz2)
{
    V3DLONG sz01 = sz0 * sz1;
    //cout<<x<<" "<<y<<" "<<z<<endl;
    if(x>=sz0||y>=sz1||z>=sz2||x<0||y<0||z<0)
    {
        //cout<<"0"<<endl;
        return 0;
    }
    if(z<=sz2)
    {
        V3DLONG result=floor(z)*sz01+floor(y)*sz0+floor(x);
        //cout<<result<<endl;
        return result;
    }
}


V3DLONG nextPoint(T_Point p,vector<T_Point> &points,double angle0,double angle1,V3DLONG d,V3DLONG sz0,V3DLONG sz1,V3DLONG sz2)
{
    double z_angle=p.z_angle;
    double x_angle=p.x_angle;
    const int span=1;
    int num=(int)d/span;

    cout<<"in nextpoint"<<endl;

    vector<vector<vector<V3DLONG>>> next;
    next=vector<vector<vector<V3DLONG>>>(num,vector<vector<V3DLONG>>());
    for(int i=0;i<num;++i)
    {
        next[i]=vector<vector<V3DLONG>>(8,vector<V3DLONG>());
    }

    cout<<"00000"<<endl;

    for(int i=0;i<num;++i)
    {
        double dd=d-i*span;
        T_Point point;
        point.z=dd*cos(trans(z_angle));
        point.y=dd*sin(trans(z_angle))*sin(trans(x_angle));
        point.x=dd*sin(trans(z_angle))*cos(trans(x_angle));

        cout<<"0"<<endl;

        V3DLONG result=threeToOne(point.x+p.x,point.y+p.y,point.z+p.z,sz0,sz1,sz2);
        cout<<result<<endl;
        if(points[result].state==T_Point::ALIVE&&points[result].is_used==false)
        {
            cout<<"bingo"<<endl;
            return result;
        }

        cout<<"1"<<endl;

        double z_angle0=z_angle+angle0;
        double z_angle1=z_angle-angle0;
        double x_angle0=x_angle+angle1;
        double x_angle1=x_angle-angle1;

        T_Point point0,point1,point2,point3;
        T_Point point4,point5,point6,point7;

        point0.z=dd*cos(trans(z_angle0));
        point0.y=dd*sin(trans(z_angle0))*sin(trans(x_angle));
        point0.x=dd*sin(trans(z_angle0))*cos(trans(x_angle));

        point1.z=dd*cos(trans(z_angle1));
        point1.y=dd*sin(trans(z_angle1))*sin(trans(x_angle));
        point1.x=dd*sin(trans(z_angle1))*cos(trans(x_angle));

        point2.z=dd*cos(trans(z_angle));
        point2.y=dd*sin(trans(z_angle))*sin(trans(x_angle0));
        point2.x=dd*sin(trans(z_angle))*cos(trans(x_angle0));

        point3.z=dd*cos(trans(z_angle));
        point3.y=dd*sin(trans(z_angle))*sin(trans(x_angle1));
        point3.x=dd*sin(trans(z_angle))*cos(trans(x_angle1));

        point4.z=dd*cos(trans(z_angle0));
        point4.y=dd*sin(trans(z_angle0))*sin(trans(x_angle0));
        point4.x=dd*sin(trans(z_angle0))*cos(trans(x_angle0));

        point5.z=dd*cos(trans(z_angle1));
        point5.y=dd*sin(trans(z_angle1))*sin(trans(x_angle0));
        point5.x=dd*sin(trans(z_angle1))*cos(trans(x_angle0));

        point6.z=dd*cos(trans(z_angle0));
        point6.y=dd*sin(trans(z_angle0))*sin(trans(x_angle1));
        point6.x=dd*sin(trans(z_angle0))*cos(trans(x_angle1));

        point7.z=dd*cos(trans(z_angle1));
        point7.y=dd*sin(trans(z_angle1))*sin(trans(x_angle1));
        point7.x=dd*sin(trans(z_angle1))*cos(trans(x_angle1));

        double dist0=dist(point,point0);
        double dist1=dist(point,point1);
        double dist2=dist(point,point2);
        double dist3=dist(point,point3);

        double dist4=dist(point,point0);
        double dist5=dist(point,point1);
        double dist6=dist(point,point2);
        double dist7=dist(point,point3);

        int num0=(int)dist0/span;
        int num1=(int)dist1/span;
        int num2=(int)dist2/span;
        int num3=(int)dist3/span;

        int num4=(int)dist4/span;
        int num5=(int)dist5/span;
        int num6=(int)dist6/span;
        int num7=(int)dist7/span;

        double anglez0=anglez(point,point0);
        double anglex0=anglex(point,point0);
        double anglez1=anglez(point,point1);
        double anglex1=anglex(point,point1);
        double anglez2=anglez(point,point2);
        double anglex2=anglex(point,point2);
        double anglez3=anglez(point,point3);
        double anglex3=anglex(point,point3);

        double anglez4=anglez(point,point4);
        double anglex4=anglex(point,point4);
        double anglez5=anglez(point,point5);
        double anglex5=anglex(point,point5);
        double anglez6=anglez(point,point6);
        double anglex6=anglex(point,point6);
        double anglez7=anglez(point,point7);
        double anglex7=anglex(point,point7);

        cout<<"2"<<endl;

        for(int j=0;j<num0;++j)
        {
            double dd0=dist0-j*span;
            T_Point p0;
            p0.z=dd0*cos(trans(anglez0));
            p0.y=dd0*sin(trans(anglez0))*sin(trans(anglex0));
            p0.x=dd0*sin(trans(anglez0))*cos(trans(anglex0));

            V3DLONG dz=point.z+p.z+p0.z;
            V3DLONG dy=point.y+p.y+p0.y;
            V3DLONG dx=point.x+p.x+p0.x;

            V3DLONG result=threeToOne(dx,dy,dz,sz0,sz1,sz2);
            next[i].at(0).push_back(result);
        }

        cout<<"3"<<endl;

        for(int j=0;j<num1;++j)
        {
            double dd0=dist1-j*span;
            T_Point p0;
            p0.z=dd0*cos(trans(anglez1));
            p0.y=dd0*sin(trans(anglez1))*sin(trans(anglex1));
            p0.x=dd0*sin(trans(anglez1))*cos(trans(anglex1));

            V3DLONG dz=point.z+p.z+p0.z;
            V3DLONG dy=point.y+p.y+p0.y;
            V3DLONG dx=point.x+p.x+p0.x;

            V3DLONG result=threeToOne(dx,dy,dz,sz0,sz1,sz2);
            next[i].at(1).push_back(result);
        }

        cout<<"4"<<endl;

        for(int j=0;j<num2;++j)
        {
            double dd0=dist2-j*span;
            T_Point p0;
            p0.z=dd0*cos(trans(anglez2));
            p0.y=dd0*sin(trans(anglez2))*sin(trans(anglex2));
            p0.x=dd0*sin(trans(anglez2))*cos(trans(anglex2));

            V3DLONG dz=point.z+p.z+p0.z;
            V3DLONG dy=point.y+p.y+p0.y;
            V3DLONG dx=point.x+p.x+p0.x;

            V3DLONG result=threeToOne(dx,dy,dz,sz0,sz1,sz2);
            next[i].at(2).push_back(result);
        }

        cout<<"5"<<endl;

        for(int j=0;j<num3;++j)
        {
            double dd0=dist3-j*span;
            T_Point p0;
            p0.z=dd0*cos(trans(anglez3));
            p0.y=dd0*sin(trans(anglez3))*sin(trans(anglex3));
            p0.x=dd0*sin(trans(anglez3))*cos(trans(anglex3));

            V3DLONG dz=point.z+p.z+p0.z;
            V3DLONG dy=point.y+p.y+p0.y;
            V3DLONG dx=point.x+p.x+p0.x;

            V3DLONG result=threeToOne(dx,dy,dz,sz0,sz1,sz2);
            next[i].at(3).push_back(result);
        }

        cout<<"6"<<endl;

        for(int j=0;j<num4;++j)
        {
            double dd0=dist4-j*span;
            T_Point p0;
            p0.z=dd0*cos(trans(anglez4));
            p0.y=dd0*sin(trans(anglez4))*sin(trans(anglex4));
            p0.x=dd0*sin(trans(anglez4))*cos(trans(anglex4));

            V3DLONG dz=point.z+p.z+p0.z;
            V3DLONG dy=point.y+p.y+p0.y;
            V3DLONG dx=point.x+p.x+p0.x;

            V3DLONG result=threeToOne(dx,dy,dz,sz0,sz1,sz2);
            next[i].at(4).push_back(result);
        }

        for(int j=0;j<num5;++j)
        {
            double dd0=dist5-j*span;
            T_Point p0;
            p0.z=dd0*cos(trans(anglez5));
            p0.y=dd0*sin(trans(anglez5))*sin(trans(anglex5));
            p0.x=dd0*sin(trans(anglez5))*cos(trans(anglex5));

            V3DLONG dz=point.z+p.z+p0.z;
            V3DLONG dy=point.y+p.y+p0.y;
            V3DLONG dx=point.x+p.x+p0.x;

            V3DLONG result=threeToOne(dx,dy,dz,sz0,sz1,sz2);
            next[i].at(5).push_back(result);
        }

        for(int j=0;j<num6;++j)
        {
            double dd0=dist6-j*span;
            T_Point p0;
            p0.z=dd0*cos(trans(anglez6));
            p0.y=dd0*sin(trans(anglez6))*sin(trans(anglex6));
            p0.x=dd0*sin(trans(anglez6))*cos(trans(anglex6));

            V3DLONG dz=point.z+p.z+p0.z;
            V3DLONG dy=point.y+p.y+p0.y;
            V3DLONG dx=point.x+p.x+p0.x;

            V3DLONG result=threeToOne(dx,dy,dz,sz0,sz1,sz2);
            next[i].at(6).push_back(result);
        }

        for(int j=0;j<num7;++j)
        {
            double dd0=dist7-j*span;
            T_Point p0;
            p0.z=dd0*cos(trans(anglez7));
            p0.y=dd0*sin(trans(anglez7))*sin(trans(anglex7));
            p0.x=dd0*sin(trans(anglez7))*cos(trans(anglex7));

            V3DLONG dz=point.z+p.z+p0.z;
            V3DLONG dy=point.y+p.y+p0.y;
            V3DLONG dx=point.x+p.x+p0.x;

            V3DLONG result=threeToOne(dx,dy,dz,sz0,sz1,sz2);
            next[i].at(7).push_back(result);
        }

    }

    cout<<"11111"<<endl;

    for(int i=0;i<num;++i)
    {
        for(int j=0;j<8;++j)
        {
            for(int k=0;k<next[i].at(j).size();++k)
            {
                V3DLONG result=next[i].at(j).at(k);
                if(points[result].state==T_Point::ALIVE&&points[result].is_used==false)
                {
                    return result;
                }
            }
        }
    }

    cout<<"22222"<<endl;

    return -1;

}

vector<V3DLONG> nextPoints(T_Point &p, vector<T_Point> &points, double angle0_min, double angle1_min, double angle0_max, double angle1_max, V3DLONG d_min, V3DLONG d_max, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2)
{
    cout<<"in nextpoints"<<endl;
    vector<V3DLONG> results;

    const int span=1;
    double z_angle=p.z_angle;
    double x_angle=p.x_angle;

    cout<<"before nextpoint of nextpoints"<<endl;

    V3DLONG result1=nextPoint(p,points,angle0_min,angle1_min,d_max,sz0,sz1,sz2);

    cout<<"after nextpoint of nextpoints"<<endl;
    if(result1!=-1)
    {
        results.push_back(result1);
    }

    int num=(int)d_min/span;

    cout<<"0000"<<endl;

    vector<vector<vector<V3DLONG>>> next;
    next=vector<vector<vector<V3DLONG>>>(num,vector<vector<V3DLONG>>());
    for(int i=0;i<num;++i)
    {
        next[i]=vector<vector<V3DLONG>>(8,vector<V3DLONG>());
    }

    for(int i=0;i<num;++i)
    {

        double dd=d_min-i*span;
        T_Point point;
        point.z=dd*cos(trans(z_angle));
        point.y=dd*sin(trans(z_angle))*sin(trans(x_angle));
        point.x=dd*sin(trans(z_angle))*cos(trans(x_angle));

        double z_angle0=z_angle+angle0_min;
        double z_angle1=z_angle-angle0_min;
        double x_angle0=x_angle+angle1_min;
        double x_angle1=x_angle-angle1_min;
        double z_angle00=z_angle+angle0_max;
        double z_angle11=z_angle-angle0_max;
        double x_angle00=x_angle+angle1_max;
        double x_angle11=x_angle-angle1_max;

        T_Point point0,point1,point2,point3,point00,point11,point22,point33;

        T_Point point4,point5,point6,point7,point44,point55,point66,point77;

        point0.z=dd*cos(trans(z_angle0));
        point0.y=dd*sin(trans(z_angle0))*sin(trans(x_angle));
        point0.x=dd*sin(trans(z_angle0))*cos(trans(x_angle));

        point1.z=dd*cos(trans(z_angle1));
        point1.y=dd*sin(trans(z_angle1))*sin(trans(x_angle));
        point1.x=dd*sin(trans(z_angle1))*cos(trans(x_angle));

        point2.z=dd*cos(trans(z_angle));
        point2.y=dd*sin(trans(z_angle))*sin(trans(x_angle0));
        point2.x=dd*sin(trans(z_angle))*cos(trans(x_angle0));

        point3.z=dd*cos(trans(z_angle));
        point3.y=dd*sin(trans(z_angle))*sin(trans(x_angle1));
        point3.x=dd*sin(trans(z_angle))*cos(trans(x_angle1));

        point00.z=dd*cos(trans(z_angle00));
        point00.y=dd*sin(trans(z_angle00))*sin(trans(x_angle));
        point00.x=dd*sin(trans(z_angle00))*cos(trans(x_angle));

        point11.z=dd*cos(trans(z_angle11));
        point11.y=dd*sin(trans(z_angle11))*sin(trans(x_angle));
        point11.x=dd*sin(trans(z_angle11))*cos(trans(x_angle));

        point22.z=dd*cos(trans(z_angle));
        point22.y=dd*sin(trans(z_angle))*sin(trans(x_angle00));
        point22.x=dd*sin(trans(z_angle))*cos(trans(x_angle00));

        point33.z=dd*cos(trans(z_angle));
        point33.y=dd*sin(trans(z_angle))*sin(trans(x_angle11));
        point33.x=dd*sin(trans(z_angle))*cos(trans(x_angle11));

        point4.z=dd*cos(trans(z_angle0));
        point4.y=dd*sin(trans(z_angle0))*sin(trans(x_angle0));
        point4.x=dd*sin(trans(z_angle0))*cos(trans(x_angle0));

        point5.z=dd*cos(trans(z_angle1));
        point5.y=dd*sin(trans(z_angle1))*sin(trans(x_angle0));
        point5.x=dd*sin(trans(z_angle1))*cos(trans(x_angle0));

        point6.z=dd*cos(trans(z_angle0));
        point6.y=dd*sin(trans(z_angle0))*sin(trans(x_angle1));
        point6.x=dd*sin(trans(z_angle0))*cos(trans(x_angle1));

        point7.z=dd*cos(trans(z_angle1));
        point7.y=dd*sin(trans(z_angle1))*sin(trans(x_angle1));
        point7.x=dd*sin(trans(z_angle1))*cos(trans(x_angle1));

        point44.z=dd*cos(trans(z_angle00));
        point44.y=dd*sin(trans(z_angle00))*sin(trans(x_angle00));
        point44.x=dd*sin(trans(z_angle00))*cos(trans(x_angle00));

        point55.z=dd*cos(trans(z_angle11));
        point55.y=dd*sin(trans(z_angle11))*sin(trans(x_angle00));
        point55.x=dd*sin(trans(z_angle11))*cos(trans(x_angle00));

        point66.z=dd*cos(trans(z_angle00));
        point66.y=dd*sin(trans(z_angle00))*sin(trans(x_angle11));
        point66.x=dd*sin(trans(z_angle00))*cos(trans(x_angle11));

        point77.z=dd*cos(trans(z_angle11));
        point77.y=dd*sin(trans(z_angle11))*sin(trans(x_angle11));
        point77.x=dd*sin(trans(z_angle11))*cos(trans(x_angle11));

        double dist0=dist(point0,point00);
        double dist1=dist(point1,point11);
        double dist2=dist(point2,point22);
        double dist3=dist(point3,point33);

        double dist4=dist(point4,point44);
        double dist5=dist(point5,point55);
        double dist6=dist(point6,point66);
        double dist7=dist(point7,point77);

        int num0=(int)dist0/span;
        int num1=(int)dist1/span;
        int num2=(int)dist2/span;
        int num3=(int)dist3/span;

        int num4=(int)dist4/span;
        int num5=(int)dist5/span;
        int num6=(int)dist6/span;
        int num7=(int)dist7/span;

        double anglez0=anglez(point0,point00);
        double anglex0=anglex(point0,point00);
        double anglez1=anglez(point1,point11);
        double anglex1=anglex(point1,point11);
        double anglez2=anglez(point2,point22);
        double anglex2=anglex(point2,point22);
        double anglez3=anglez(point3,point33);
        double anglex3=anglex(point3,point33);

        double anglez4=anglez(point4,point44);
        double anglex4=anglex(point4,point44);
        double anglez5=anglez(point5,point55);
        double anglex5=anglex(point5,point55);
        double anglez6=anglez(point6,point66);
        double anglex6=anglex(point6,point66);
        double anglez7=anglez(point7,point77);
        double anglex7=anglex(point7,point77);

        for(int j=0;j<num0;++j)
        {
            double dd0=dist0-j*span;
            T_Point p0;
            p0.z=dd0*cos(trans(anglez0));
            p0.y=dd0*sin(trans(anglez0))*sin(trans(anglex0));
            p0.x=dd0*sin(trans(anglez0))*cos(trans(anglex0));

            V3DLONG dz=point0.z+p.z+p0.z;
            V3DLONG dy=point0.y+p.y+p0.y;
            V3DLONG dx=point0.x+p.x+p0.x;

            V3DLONG result=threeToOne(dx,dy,dz,sz0,sz1,sz2);
            next[i].at(0).push_back(result);
        }

        for(int j=0;j<num1;++j)
        {
            double dd0=dist1-j*span;
            T_Point p0;
            p0.z=dd0*cos(trans(anglez1));
            p0.y=dd0*sin(trans(anglez1))*sin(trans(anglex1));
            p0.x=dd0*sin(trans(anglez1))*cos(trans(anglex1));

            V3DLONG dz=point1.z+p.z+p0.z;
            V3DLONG dy=point1.y+p.y+p0.y;
            V3DLONG dx=point1.x+p.x+p0.x;

            V3DLONG result=threeToOne(dx,dy,dz,sz0,sz1,sz2);
            next[i].at(1).push_back(result);
        }

        for(int j=0;j<num2;++j)
        {
            double dd0=dist2-j*span;
            T_Point p0;
            p0.z=dd0*cos(trans(anglez2));
            p0.y=dd0*sin(trans(anglez2))*sin(trans(anglex2));
            p0.x=dd0*sin(trans(anglez2))*cos(trans(anglex2));

            V3DLONG dz=point2.z+p.z+p0.z;
            V3DLONG dy=point2.y+p.y+p0.y;
            V3DLONG dx=point2.x+p.x+p0.x;

            V3DLONG result=threeToOne(dx,dy,dz,sz0,sz1,sz2);
            next[i].at(2).push_back(result);
        }

        for(int j=0;j<num3;++j)
        {
            double dd0=dist3-j*span;
            T_Point p0;
            p0.z=dd0*cos(trans(anglez3));
            p0.y=dd0*sin(trans(anglez3))*sin(trans(anglex3));
            p0.x=dd0*sin(trans(anglez3))*cos(trans(anglex3));

            V3DLONG dz=point3.z+p.z+p0.z;
            V3DLONG dy=point3.y+p.y+p0.y;
            V3DLONG dx=point3.x+p.x+p0.x;

            V3DLONG result=threeToOne(dx,dy,dz,sz0,sz1,sz2);
            next[i].at(3).push_back(result);
        }

        for(int j=0;j<num4;++j)
        {
            double dd0=dist4-j*span;
            T_Point p0;
            p0.z=dd0*cos(trans(anglez4));
            p0.y=dd0*sin(trans(anglez4))*sin(trans(anglex4));
            p0.x=dd0*sin(trans(anglez4))*cos(trans(anglex4));

            V3DLONG dz=point4.z+p.z+p0.z;
            V3DLONG dy=point4.y+p.y+p0.y;
            V3DLONG dx=point4.x+p.x+p0.x;

            V3DLONG result=threeToOne(dx,dy,dz,sz0,sz1,sz2);
            next[i].at(4).push_back(result);
        }

        for(int j=0;j<num5;++j)
        {
            double dd0=dist5-j*span;
            T_Point p0;
            p0.z=dd0*cos(trans(anglez5));
            p0.y=dd0*sin(trans(anglez5))*sin(trans(anglex5));
            p0.x=dd0*sin(trans(anglez5))*cos(trans(anglex5));

            V3DLONG dz=point5.z+p.z+p0.z;
            V3DLONG dy=point5.y+p.y+p0.y;
            V3DLONG dx=point5.x+p.x+p0.x;

            V3DLONG result=threeToOne(dx,dy,dz,sz0,sz1,sz2);
            next[i].at(5).push_back(result);
        }

        for(int j=0;j<num6;++j)
        {
            double dd0=dist6-j*span;
            T_Point p0;
            p0.z=dd0*cos(trans(anglez6));
            p0.y=dd0*sin(trans(anglez6))*sin(trans(anglex6));
            p0.x=dd0*sin(trans(anglez6))*cos(trans(anglex6));

            V3DLONG dz=point6.z+p.z+p0.z;
            V3DLONG dy=point6.y+p.y+p0.y;
            V3DLONG dx=point6.x+p.x+p0.x;

            V3DLONG result=threeToOne(dx,dy,dz,sz0,sz1,sz2);
            next[i].at(6).push_back(result);
        }

        for(int j=0;j<num7;++j)
        {
            double dd0=dist7-j*span;
            T_Point p0;
            p0.z=dd0*cos(trans(anglez7));
            p0.y=dd0*sin(trans(anglez7))*sin(trans(anglex7));
            p0.x=dd0*sin(trans(anglez7))*cos(trans(anglex7));

            V3DLONG dz=point7.z+p.z+p0.z;
            V3DLONG dy=point7.y+p.y+p0.y;
            V3DLONG dx=point7.x+p.x+p0.x;

            V3DLONG result=threeToOne(dx,dy,dz,sz0,sz1,sz2);
            next[i].at(7).push_back(result);
        }

    }

    cout<<"1111"<<endl;

    if(results.size()==1)
    {
        cout<<"size=1"<<endl;
        for(int i=0;i<num;++i)
        {
            for(int j=0;j<7;++j)
            {
                for(int k=0;k<next[i].at(j).size();++k)
                {
                    V3DLONG result=next[i].at(j).at(k);
                    if(points[result].state==T_Point::ALIVE&&points[result].is_used==false)
                    {
                        results.push_back(result);
                        p.mode=3;
                        //cout<<"mode is 3"<<endl;
                        cout<<"mode:"<<p.mode<<" "<<results[0]<<" "<<results[1]<<endl;
                        return results;
                    }
                }
            }
        }
        if(results.size()==1)
        {
            p.mode=1;
            //cout<<"mode is 1"<<endl;
            cout<<"mode:"<<p.mode<<" "<<results[0]<<endl;
            return results;
        }
    }

    if(results.empty())
    {
        cout<<"size=2"<<endl;
        V3DLONG result0[8]={-1,-1,-1,-1,-1,-1,-1,-1};

        for(int i=0;i<num;++i)
        {
            for(int k=0;k<next[i].at(0).size();++k)
            {
                V3DLONG result=next[i].at(0).at(k);
                if(points[result].state==T_Point::ALIVE&&points[result].is_used==false)
                {
                    result0[0]=result;
                }

//                if(result[0]!=-1)
//                {
//                    break;
//                }
            }
        }

        for(int i=0;i<num;++i)
        {
            for(int k=0;k<next[i].at(1).size();++k)
            {
                V3DLONG result=next[i].at(1).at(k);
                if(points[result].state==T_Point::ALIVE&&points[result].is_used==false)
                {
                    result0[1]=result;
                }
//                if(result[1]!=-1)
//                {
//                    break;
//                }
            }
        }

        for(int i=0;i<num;++i)
        {
            for(int k=0;k<next[i].at(2).size();++k)
            {
                V3DLONG result=next[i].at(2).at(k);
                if(points[result].state==T_Point::ALIVE&&points[result].is_used==false)
                {
                    result0[2]=result;
                }
//                if(result[2]!=-1)
//                {
//                    break;
//                }
            }
        }

        for(int i=0;i<num;++i)
        {
            for(int k=0;k<next[i].at(3).size();++k)
            {
                V3DLONG result=next[i].at(3).at(k);
                if(points[result].state==T_Point::ALIVE&&points[result].is_used==false)
                {
                    result0[3]=result;
                }
//                if(result[3]!=-1)
//                {
//                    break;
//                }
            }
        }

        for(int i=0;i<num;++i)
        {
            for(int k=0;k<next[i].at(4).size();++k)
            {
                V3DLONG result=next[i].at(4).at(k);
                if(points[result].state==T_Point::ALIVE&&points[result].is_used==false)
                {
                    result0[4]=result;
                }

//                if(result[0]!=-1)
//                {
//                    break;
//                }
            }
        }

        for(int i=0;i<num;++i)
        {
            for(int k=0;k<next[i].at(5).size();++k)
            {
                V3DLONG result=next[i].at(5).at(k);
                if(points[result].state==T_Point::ALIVE&&points[result].is_used==false)
                {
                    result0[5]=result;
                }
//                if(result[1]!=-1)
//                {
//                    break;
//                }
            }
        }

        for(int i=0;i<num;++i)
        {
            for(int k=0;k<next[i].at(6).size();++k)
            {
                V3DLONG result=next[i].at(6).at(k);
                if(points[result].state==T_Point::ALIVE&&points[result].is_used==false)
                {
                    result0[6]=result;
                }
//                if(result[2]!=-1)
//                {
//                    break;
//                }
            }
        }

        for(int i=0;i<num;++i)
        {
            for(int k=0;k<next[i].at(7).size();++k)
            {
                V3DLONG result=next[i].at(7).at(k);
                if(points[result].state==T_Point::ALIVE&&points[result].is_used==false)
                {
                    result0[7]=result;
                }
//                if(result[3]!=-1)
//                {
//                    break;
//                }
            }
        }

        for(int i=0;i<8;++i)
        {
            if(result0[i]!=-1&&results.size()<2)
            {
                results.push_back(result0[i]);
            }
        }

        cout<<"result0: "<<result0[0]<<" "<<result0[1]<<result0[2]<<" "<<result0[3]<<result0[4]<<" "<<result0[5]<<result0[6]<<" "<<result0[7]<<endl;

        if(results.empty())
        {
            p.mode=0;
            //cout<<"mode is 0"<<endl;
            cout<<"mode:"<<p.mode<<endl;
            return results;
        }else if(results.size()==1)
        {
            p.mode=2;
            //cout<<"mode is 2"<<endl;
            cout<<"mode:"<<p.mode<<" "<<results[0]<<endl;
            return results;

        }else
        {
            p.mode=4;
            //cout<<"mode is 4"<<endl;
            cout<<"mode:"<<p.mode<<" "<<results[0]<<" "<<results[1]<<endl;
            return results;

        }

    }

    cout<<"2222"<<endl;

}





bool construct_tree(T_Point root,vector<T_Point> &points,vector<T_Point> &outtree,double angle0_min,double angle1_min,double angle0_max,double angle1_max,V3DLONG d_min,V3DLONG d_max,V3DLONG sz0,V3DLONG sz1,V3DLONG sz2)
{

    V3DLONG tol_sz = sz0 * sz1 * sz2;
    V3DLONG sz01 = sz0 * sz1;

    const int d=5;


    if(points[root.n].state!=T_Point::ALIVE)
    {
        cout<<"Please select the correct marker"<<endl;
        return false;
    }
    points[root.n].parent=-1;
    points[root.n].is_used=true;
    points[root.n].z_angle=root.z_angle;
    points[root.n].x_angle=root.x_angle;
    points[root.n].turn_d=0;
    points[root.n].bifurcation_d=0;
    points[root.n].adjoin_d=0;
    points[root.n].bifparent=points[root.n].n;
    points[root.n].turnparent=points[root.n].n;
    vector<vector<V3DLONG>> chilrden;
    chilrden=vector<vector<V3DLONG>>(tol_sz,vector<V3DLONG>());
    //outtree.push_back(points[root].n]);

    vector<T_Point> queue;
    queue.push_back(points[root.n]);

    /*


    vector<T_Point> queue,queue1;
    queue.push_back(*points[root].n]);
    queue1.push_back(*points[root].n]);
    while(!queue.empty())
    {
        T_Point p=queue.front();
        queue.erase(queue.begin());
        if(p.turn_d>d)
        {
            int mode=-1;double angle_min,angle_max;
            vector<V3DLONG> branch=nextPoints(p,angle_min,angle_max,d,mode);
            if(mode==0)
            {
                outtree.insert(outtree.end(),*queue1.begin(),*queue1.end());
                queue1.clear();
            }
            if(mode==1)
            {
                //points[branch[0]]...
                queue1.push_back(points[branch[0]]);
            }
            if(mode==2)
            {
                //points[branch[0]]...
                outtree.insert(outtree.end(),*queue1.begin(),*queue1.end());
                queue1.clear();
                queue1.push_back(points[branch[0]]);
            }
            if(mode==3)
            {

            }

        }

    }

    */

    cout<<222<<endl;
    //simple version
    while(!queue.empty())
    {
        T_Point p=queue.front();
        queue.erase(queue.begin());
        cout<<"333"<<endl;
        if(p.turn_d>d)
        {
            //int mode=-1;
            cout<<"before 444"<<endl;
            vector<V3DLONG> branch=nextPoints(p,points,angle0_min,angle1_min,angle0_max,angle1_max,d_min,d_max,sz0,sz1,sz2);
            cout<<"444"<<endl;
            if(p.mode==0)
            {
                outtree.push_back(p);
            }
            if(p.mode==1)
            {
                points[branch[0]].adjoin_d=dist(p,points[branch[0]]);
                points[branch[0]].is_used=true;
                points[branch[0]].parent=p.n;
                points[branch[0]].z_angle=anglez(p,points[branch[0]]);
                points[branch[0]].x_angle=anglex(p,points[branch[0]]);
                points[branch[0]].bifparent=p.bifparent;
                points[branch[0]].bifurcation_d=p.bifurcation_d+points[branch[0]].adjoin_d;
                points[branch[0]].turn_d=p.turn_d+points[branch[0]].adjoin_d;
                points[branch[0]].turnparent=p.turnparent;
                queue.push_back(points[branch[0]]);
                outtree.push_back(p);
            }
            if(p.mode==2)
            {
                points[branch[0]].adjoin_d=dist(p,points[branch[0]]);
                points[branch[0]].is_used=true;
                p.is_turn=true;
                points[branch[0]].parent=p.n;
                points[branch[0]].z_angle=anglez(p,points[branch[0]]);
                points[branch[0]].x_angle=anglex(p,points[branch[0]]);
                points[branch[0]].bifparent=p.bifparent;
                points[branch[0]].bifurcation_d=p.bifurcation_d+points[branch[0]].adjoin_d;
                points[branch[0]].turn_d=points[branch[0]].adjoin_d;
                points[branch[0]].turnparent=p.n;
                queue.push_back(points[branch[0]]);
                outtree.push_back(p);
            }
            if(p.mode==3)
            {
                points[branch[0]].adjoin_d=dist(p,points[branch[0]]);
                points[branch[0]].is_used=true;
                p.bifurcation=true;
                p.is_turn=true;
                points[branch[0]].parent=p.n;
                points[branch[0]].z_angle=anglez(p,points[branch[0]]);
                points[branch[0]].x_angle=anglex(p,points[branch[0]]);
                points[branch[0]].bifparent=p.n;
                points[branch[0]].bifurcation_d=points[branch[0]].adjoin_d;
                points[branch[0]].turn_d=points[branch[0]].adjoin_d;
                points[branch[0]].turnparent=p.n;
                queue.push_back(points[branch[0]]);

                points[branch[1]].adjoin_d=dist(p,points[branch[1]]);
                points[branch[1]].is_used=true;
                points[branch[1]].parent=p.n;
                points[branch[1]].z_angle=anglez(p,points[branch[1]]);
                points[branch[1]].x_angle=anglex(p,points[branch[1]]);
                points[branch[1]].bifparent=p.n;
                points[branch[1]].bifurcation_d=points[branch[1]].adjoin_d;
                points[branch[1]].turn_d=points[branch[1]].adjoin_d;
                points[branch[1]].turnparent=p.n;
                queue.push_back(points[branch[1]]);

                outtree.push_back(p);

            }
            if(p.mode==4)
            {
                points[branch[0]].adjoin_d=dist(p,points[branch[0]]);
                points[branch[0]].is_used=true;
                p.bifurcation=true;
                p.is_turn=true;
                points[branch[0]].parent=p.n;
                points[branch[0]].z_angle=anglez(p,points[branch[0]]);
                points[branch[0]].x_angle=anglex(p,points[branch[0]]);
                points[branch[0]].bifparent=p.n;
                points[branch[0]].bifurcation_d=points[branch[0]].adjoin_d;
                points[branch[0]].turn_d=points[branch[0]].adjoin_d;
                points[branch[0]].turnparent=p.n;
                queue.push_back(points[branch[0]]);

                points[branch[1]].adjoin_d=dist(p,points[branch[1]]);
                points[branch[1]].is_used=true;
                points[branch[1]].parent=p.n;
                points[branch[1]].z_angle=anglez(p,points[branch[1]]);
                points[branch[1]].x_angle=anglex(p,points[branch[1]]);
                points[branch[1]].bifparent=p.n;
                points[branch[1]].bifurcation_d=points[branch[1]].adjoin_d;
                points[branch[1]].turn_d=points[branch[1]].adjoin_d;
                points[branch[1]].turnparent=p.n;
                queue.push_back(points[branch[1]]);

                outtree.push_back(p);
            }

        }

        if(p.turn_d<=d)
        {
            cout<<"before 555"<<endl;
            V3DLONG n=nextPoint(p,points,angle0_min,angle1_min,d_max,sz0,sz1,sz2);
            cout<<"555"<<endl;
            if(n==-1)
            {
                outtree.push_back(p);
            }else
            {
                points[n].adjoin_d=dist(p,points[n]);
                points[n].is_used=true;
                points[n].parent=p.n;
                points[n].z_angle=anglez(p,points[n]);
                points[n].x_angle=anglex(p,points[n]);
                points[n].bifparent=p.bifparent;
                points[n].bifurcation_d=p.bifurcation_d+points[n].adjoin_d;
                points[n].turn_d=p.turn_d+points[n].adjoin_d;
                points[n].turnparent=p.turnparent;
                queue.push_back(points[n]);
                outtree.push_back(p);
            }
        }



    }

    return TRUE;


}




