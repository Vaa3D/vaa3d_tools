#include "some_function.h"
#include <v3d_interface.h>
#include "Hackathon_demo_plugin.h"
#include <iostream>
#include <QtGui>
#include <vector>
#include <QList>



void cut_image(V3DPluginCallback2 &callback,NeuronTree nt,point p,QString& qstr)
{
    V3DLONG dx=64;
    V3DLONG dy=64;
    V3DLONG dz=32;
    //QString suffix0=".eswc";
    QString path0="D://shucai//swc_manual//";
    QString suffix1=".tif";
    QString dir0(qstr);
    const std::string dir=dir0.toStdString();
    if(p.t==point::up)
    {


        size_t min_x,max_x,min_y,max_y,min_z,max_z;
        //V3DLONG o_x,o_y,o_z;
        point o_xyz;
        vector<point> o_up,o_right,o_left,o_front,o_back;
        o_xyz.nswc.x=p.nswc.x;
        o_xyz.nswc.y=p.nswc.y;
        o_xyz.nswc.z=p.nswc.z;
        min_x=o_xyz.nswc.x-dx/2;
        max_x=o_xyz.nswc.x+dx/2;
        min_y=o_xyz.nswc.y-dy/2;
        max_y=o_xyz.nswc.y+dy/2;
        min_z=o_xyz.nswc.z;
        max_z=o_xyz.nswc.z+dz;

        unsigned char* imgblock;
        imgblock=callback.getSubVolumeTeraFly(dir,min_x,max_x,min_y,max_y,min_z,max_z);


        QString si0=path0+"x_"+QString::number((qlonglong)o_xyz.nswc.x,10)+"_y_"+QString::number((qlonglong)o_xyz.nswc.y,10)+"_z_"+QString::number((qlonglong)o_xyz.nswc.z,10)+suffix1;
        const char* si=si0.toStdString().c_str();

        V3DLONG sz0[4]={dx,dy,dz,1};
        int datatype=1;
        simple_saveimage_wrapper(callback,si,imgblock,sz0,datatype);
        std::cout<<"111"<<endl;

        delete imgblock;

        o_up=get_up(nt,min_x,max_x,min_y,max_y,max_z);
        o_right=get_right(nt,min_x,max_x,min_z,max_z,max_y);
        o_left=get_left(nt,min_x,max_x,min_z,max_z,min_y);
        o_front=get_front(nt,min_y,max_y,min_z,max_z,max_x);
        o_back=get_back(nt,min_y,max_y,min_z,max_z,min_x);

        for(int i=0;i<o_up.size();++i)
        {
            cut_image(callback,nt,o_up[i],dir0);
        }
        for(int i=0;i<o_right.size();++i)
        {
            cut_image(callback,nt,o_right[i],dir0);
        }
        for(int i=0;i<o_left.size();++i)
        {
            cut_image(callback,nt,o_left[i],dir0);
        }
        for(int i=0;i<o_front.size();++i)
        {
            cut_image(callback,nt,o_front[i],dir0);
        }
        for(int i=0;i<o_back.size();++i)
        {
            cut_image(callback,nt,o_back[i],dir0);
        }
    }

    if(p.t==point::down)
    {
        size_t min_x,max_x,min_y,max_y,min_z,max_z;
        //V3DLONG o_x,o_y,o_z;
        point o_xyz;
        vector<point> o_down,o_right,o_left,o_front,o_back;
        o_xyz.nswc.x=p.nswc.x;
        o_xyz.nswc.y=p.nswc.y;
        o_xyz.nswc.z=p.nswc.z;
        min_x=o_xyz.nswc.x-dx/2;
        max_x=o_xyz.nswc.x+dx/2;
        min_y=o_xyz.nswc.y-dy/2;
        max_y=o_xyz.nswc.y+dy/2;
        min_z=o_xyz.nswc.z-dz;
        max_z=o_xyz.nswc.z;

        unsigned char* imgblock;
        imgblock=callback.getSubVolumeTeraFly(dir,min_x,max_x,min_y,max_y,min_z,max_z);

        QString si0=path0+QString("x_")+QString::number((qlonglong)o_xyz.nswc.x,10)+QString("_y_")+QString::number((qlonglong)o_xyz.nswc.y,10)+QString("_z_")+QString::number((qlonglong)o_xyz.nswc.z,10)+suffix1;
        const char* si=si0.toStdString().c_str();

        V3DLONG sz0[4]={dx,dy,dz,1};
        int datatype=1;
        simple_saveimage_wrapper(callback,si,imgblock,sz0,datatype);
        std::cout<<"111"<<endl;

        delete imgblock;

        o_down=get_down(nt,min_x,max_x,min_y,max_y,min_z);
        o_right=get_right(nt,min_x,max_x,min_z,max_z,max_y);
        o_left=get_left(nt,min_x,max_x,min_z,max_z,min_y);
        o_front=get_front(nt,min_y,max_y,min_z,max_z,max_x);
        o_back=get_back(nt,min_y,max_y,min_z,max_z,min_x);

        for(int i=0;i<o_down.size();++i)
        {
            cut_image(callback,nt,o_down[i],dir0);
        }
        for(int i=0;i<o_right.size();++i)
        {
            cut_image(callback,nt,o_right[i],dir0);
        }
        for(int i=0;i<o_left.size();++i)
        {
            cut_image(callback,nt,o_left[i],dir0);
        }
        for(int i=0;i<o_front.size();++i)
        {
            cut_image(callback,nt,o_front[i],dir0);
        }
        for(int i=0;i<o_back.size();++i)
        {
            cut_image(callback,nt,o_back[i],dir0);
        }
    }

    if(p.t==point::left)
    {
        size_t min_x,max_x,min_y,max_y,min_z,max_z;
        //V3DLONG o_x,o_y,o_z;
        point o_xyz;
        vector<point> o_up,o_front,o_left,o_down,o_back;
        o_xyz.nswc.x=p.nswc.x;
        o_xyz.nswc.y=p.nswc.y;
        o_xyz.nswc.z=p.nswc.z;
        min_x=o_xyz.nswc.x-dx/2;
        max_x=o_xyz.nswc.x+dx/2;
        min_y=o_xyz.nswc.y-dy;
        max_y=o_xyz.nswc.y;
        min_z=o_xyz.nswc.z-dz/2;
        max_z=o_xyz.nswc.z+dz/2;

        unsigned char* imgblock;
        imgblock=callback.getSubVolumeTeraFly(dir,min_x,max_x,min_y,max_y,min_z,max_z);

        QString si0=path0+"x_"+QString::number((qlonglong)o_xyz.nswc.x,10)+"_y_"+QString::number((qlonglong)o_xyz.nswc.y,10)+"_z_"+QString::number((qlonglong)o_xyz.nswc.z,10)+suffix1;
        const char* si=si0.toStdString().c_str();

        V3DLONG sz0[4]={dx,dy,dz,1};
        int datatype=1;
        simple_saveimage_wrapper(callback,si,imgblock,sz0,datatype);
        std::cout<<"111"<<endl;

        delete imgblock;

        o_up=get_up(nt,min_x,max_x,min_y,max_y,max_z);
        o_down=get_down(nt,min_x,max_x,min_y,max_y,min_z);
        o_left=get_left(nt,min_x,max_x,min_z,max_z,min_y);
        o_front=get_front(nt,min_y,max_y,min_z,max_z,max_x);
        o_back=get_back(nt,min_y,max_y,min_z,max_z,min_x);

        for(int i=0;i<o_up.size();++i)
        {
            cut_image(callback,nt,o_up[i],dir0);
        }
        for(int i=0;i<o_down.size();++i)
        {
            cut_image(callback,nt,o_down[i],dir0);
        }
        for(int i=0;i<o_left.size();++i)
        {
            cut_image(callback,nt,o_left[i],dir0);
        }
        for(int i=0;i<o_front.size();++i)
        {
            cut_image(callback,nt,o_front[i],dir0);
        }
        for(int i=0;i<o_back.size();++i)
        {
            cut_image(callback,nt,o_back[i],dir0);
        }
    }

    if(p.t==point::right)
    {
        size_t min_x,max_x,min_y,max_y,min_z,max_z;
        //V3DLONG o_x,o_y,o_z;
        point o_xyz;
        vector<point> o_up,o_right,o_down,o_front,o_back;
        o_xyz.nswc.x=p.nswc.x;
        o_xyz.nswc.y=p.nswc.y;
        o_xyz.nswc.z=p.nswc.z;
        min_x=o_xyz.nswc.x-dx/2;
        max_x=o_xyz.nswc.x+dx/2;
        min_y=o_xyz.nswc.y;
        max_y=o_xyz.nswc.y+dy;
        min_z=o_xyz.nswc.z-dz/2;
        max_z=o_xyz.nswc.z+dz/2;

        unsigned char* imgblock;
        imgblock=callback.getSubVolumeTeraFly(dir,min_x,max_x,min_y,max_y,min_z,max_z);

        QString si0=path0+"x_"+QString::number((qlonglong)o_xyz.nswc.x,10)+"_y_"+QString::number((qlonglong)o_xyz.nswc.y,10)+"_z_"+QString::number((qlonglong)o_xyz.nswc.z,10)+suffix1;
        const char* si=si0.toStdString().c_str();

        V3DLONG sz0[4]={dx,dy,dz,1};
        int datatype=1;
        simple_saveimage_wrapper(callback,si,imgblock,sz0,datatype);
        std::cout<<"111"<<endl;

        delete imgblock;

        o_up=get_up(nt,min_x,max_x,min_y,max_y,max_z);
        o_right=get_right(nt,min_x,max_x,min_z,max_z,max_y);
        o_down=get_down(nt,min_x,max_x,min_y,max_y,min_z);
        o_front=get_front(nt,min_y,max_y,min_z,max_z,max_x);
        o_back=get_back(nt,min_y,max_y,min_z,max_z,min_x);

        for(int i=0;i<o_up.size();++i)
        {
            cut_image(callback,nt,o_up[i],dir0);
        }
        for(int i=0;i<o_right.size();++i)
        {
            cut_image(callback,nt,o_right[i],dir0);
        }
        for(int i=0;i<o_down.size();++i)
        {
            cut_image(callback,nt,o_down[i],dir0);
        }
        for(int i=0;i<o_front.size();++i)
        {
            cut_image(callback,nt,o_front[i],dir0);
        }
        for(int i=0;i<o_back.size();++i)
        {
            cut_image(callback,nt,o_back[i],dir0);
        }
    }

    if(p.t==point::front)
    {
        size_t min_x,max_x,min_y,max_y,min_z,max_z;
        //V3DLONG o_x,o_y,o_z;
        point o_xyz;
        vector<point> o_up,o_right,o_left,o_front,o_down;
        o_xyz.nswc.x=p.nswc.x;
        o_xyz.nswc.y=p.nswc.y;
        o_xyz.nswc.z=p.nswc.z;
        min_x=o_xyz.nswc.x;
        max_x=o_xyz.nswc.x+dx;
        min_y=o_xyz.nswc.y-dy/2;
        max_y=o_xyz.nswc.y+dy/2;
        min_z=o_xyz.nswc.z-dz/2;
        max_z=o_xyz.nswc.z+dz/2;

        unsigned char* imgblock;
        imgblock=callback.getSubVolumeTeraFly(dir,min_x,max_x,min_y,max_y,min_z,max_z);

        QString si0=path0+"x_"+QString::number((qlonglong)o_xyz.nswc.x,10)+"_y_"+QString::number((qlonglong)o_xyz.nswc.y,10)+"_z_"+QString::number((qlonglong)o_xyz.nswc.z,10)+suffix1;
        const char* si=si0.toStdString().c_str();

        V3DLONG sz0[4]={dx,dy,dz,1};
        int datatype=1;
        simple_saveimage_wrapper(callback,si,imgblock,sz0,datatype);
        std::cout<<"111"<<endl;

        delete imgblock;

        o_up=get_up(nt,min_x,max_x,min_y,max_y,max_z);
        o_right=get_right(nt,min_x,max_x,min_z,max_z,max_y);
        o_left=get_left(nt,min_x,max_x,min_z,max_z,min_y);
        o_front=get_front(nt,min_y,max_y,min_z,max_z,max_x);
        o_down=get_down(nt,min_x,max_x,min_y,max_y,min_z);

        for(int i=0;i<o_up.size();++i)
        {
            cut_image(callback,nt,o_up[i],dir0);
        }
        for(int i=0;i<o_right.size();++i)
        {
            cut_image(callback,nt,o_right[i],dir0);
        }
        for(int i=0;i<o_left.size();++i)
        {
            cut_image(callback,nt,o_left[i],dir0);
        }
        for(int i=0;i<o_front.size();++i)
        {
            cut_image(callback,nt,o_front[i],dir0);
        }
        for(int i=0;i<o_down.size();++i)
        {
            cut_image(callback,nt,o_down[i],dir0);
        }
    }

    if(p.t==point::back)
    {
        size_t min_x,max_x,min_y,max_y,min_z,max_z;
        //V3DLONG o_x,o_y,o_z;
        point o_xyz;
        vector<point> o_up,o_right,o_left,o_down,o_back;
        o_xyz.nswc.x=p.nswc.x;
        o_xyz.nswc.y=p.nswc.y;
        o_xyz.nswc.z=p.nswc.z;
        min_x=o_xyz.nswc.x-dx;
        max_x=o_xyz.nswc.x;
        min_y=o_xyz.nswc.y-dy/2;
        max_y=o_xyz.nswc.y+dy/2;
        min_z=o_xyz.nswc.z-dz/2;
        max_z=o_xyz.nswc.z+dz/2;

        unsigned char* imgblock;
        imgblock=callback.getSubVolumeTeraFly(dir,min_x,max_x,min_y,max_y,min_z,max_z);

        QString si0=path0+"x_"+QString::number((qlonglong)o_xyz.nswc.x,10)+"_y_"+QString::number((qlonglong)o_xyz.nswc.y,10)+"_z_"+QString::number((qlonglong)o_xyz.nswc.z,10)+suffix1;
        const char* si=si0.toStdString().c_str();

        V3DLONG sz0[4]={dx,dy,dz,1};
        int datatype=1;
        simple_saveimage_wrapper(callback,si,imgblock,sz0,datatype);
        std::cout<<"111"<<endl;

        delete imgblock;

        o_up=get_up(nt,min_x,max_x,min_y,max_y,max_z);
        o_right=get_right(nt,min_x,max_x,min_z,max_z,max_y);
        o_left=get_left(nt,min_x,max_x,min_z,max_z,min_y);
        o_down=get_down(nt,min_x,max_x,min_y,max_y,min_z);
        o_back=get_back(nt,min_y,max_y,min_z,max_z,min_x);

        for(int i=0;i<o_up.size();++i)
        {
            cut_image(callback,nt,o_up[i],dir0);
        }
        for(int i=0;i<o_right.size();++i)
        {
            cut_image(callback,nt,o_right[i],dir0);
        }
        for(int i=0;i<o_left.size();++i)
        {
            cut_image(callback,nt,o_left[i],dir0);
        }
        for(int i=0;i<o_down.size();++i)
        {
            cut_image(callback,nt,o_down[i],dir0);
        }
        for(int i=0;i<o_back.size();++i)
        {
            cut_image(callback,nt,o_back[i],dir0);
        }
    }


}
V3DLONG getBranch(NeuronSWC swct,vector<point> pts)
{

    while(true)
    {

        if(swct.parent==-1)
        {
            return 1;
        }
        if(pts[swct.parent-1].children.size()==2)
        {
            return swct.parent;
        }
        std::cout<<swct.parent<<endl;

        swct=pts[swct.parent-1].nswc;

    }
}
vector<point> trans(NeuronTree nt)
{
    vector<point> pts;
    int size=nt.listNeuron.size();
    for(int i=0;i<size;++i)
    {
        point pt;
        pt.nswc=nt.listNeuron[i];
        for(int j=0;j<size;++j)
        {
            if(pt.nswc.n==nt.listNeuron[j].parent)
                pt.children.push_back(nt.listNeuron[j].n);
        }
        pts.push_back(pt);
        pt.children.clear();
    }
    //pts[0].branch=1;
    for(int i=0;i<pts.size();++i)
    {
        NeuronSWC swct=pts[i].nswc;
        pts[i].branch=getBranch(swct,pts);
    }
    return pts;
}

block getBlockOfOPoint(point p,int dx,int dy,int dz)
{
    block btmp;
    btmp.o=p;
    int dd;
    if(p.t==point::ori)
    {
        btmp.min_x=p.nswc.x-dx/2;
        btmp.max_x=p.nswc.x+dx/2;
        btmp.min_y=p.nswc.y-dy/2;
        btmp.max_y=p.nswc.y+dy/2;
        btmp.min_z=p.nswc.z-dz/2;
        btmp.max_z=p.nswc.z+dz/2;
    }
    if(p.t==point::up)
    {
        dd=5;
        btmp.min_x=p.nswc.x-dx/2;
        btmp.max_x=p.nswc.x+dx/2;
        btmp.min_y=p.nswc.y-dy/2;
        btmp.max_y=p.nswc.y+dy/2;
        btmp.min_z=p.nswc.z-dd;
        btmp.max_z=p.nswc.z+dz-dd;
    }
    if(p.t==point::down)
    {
        dd=5;
        btmp.min_x=p.nswc.x-dx/2;
        btmp.max_x=p.nswc.x+dx/2;
        btmp.min_y=p.nswc.y-dy/2;
        btmp.max_y=p.nswc.y+dy/2;
        btmp.min_z=p.nswc.z-dz+dd;
        btmp.max_z=p.nswc.z+dd;
    }
    if(p.t==point::left)
    {
        dd=5;
        btmp.min_x=p.nswc.x-dx/2;
        btmp.max_x=p.nswc.x+dx/2;
        btmp.min_y=p.nswc.y-dy+dd;
        btmp.max_y=p.nswc.y+dd;
        btmp.min_z=p.nswc.z-dz/2;
        btmp.max_z=p.nswc.z+dz/2;
    }
    if(p.t==point::right)
    {
        dd=5;
        btmp.min_x=p.nswc.x-dx/2;
        btmp.max_x=p.nswc.x+dx/2;
        btmp.min_y=p.nswc.y-dd;
        btmp.max_y=p.nswc.y+dy-dd;
        btmp.min_z=p.nswc.z-dz/2;
        btmp.max_z=p.nswc.z+dz/2;
    }
    if(p.t==point::front)
    {
        dd=5;
        btmp.min_x=p.nswc.x-dd;
        btmp.max_x=p.nswc.x+dx-dd;
        btmp.min_y=p.nswc.y-dy/2;
        btmp.max_y=p.nswc.y+dy/2;
        btmp.min_z=p.nswc.z-dz/2;
        btmp.max_z=p.nswc.z+dz/2;
    }
    if(p.t==point::back)
    {
        dd=5;
        btmp.min_x=p.nswc.x-dx+dd;
        btmp.max_x=p.nswc.x+dd;
        btmp.min_y=p.nswc.y-dy/2;
        btmp.max_y=p.nswc.y+dy/2;
        btmp.min_z=p.nswc.z-dz/2;
        btmp.max_z=p.nswc.z+dz/2;
    }
    return btmp;
}

/*
vector<point> getTPointOfBlock(NeuronTree nt, block b)
{
    /*
    vector<point> pv;
    //block btmp=b;
    NeuronSWC ptmp0,ptmp1;
    point p,po=b.o;
    int size=nt.listNeuron.size();

    if(po.t==point::ori)
    {
        for(int i=0;i<size-1;++i)
        {
            ptmp0=nt.listNeuron[i];
            ptmp1=nt.listNeuron[i+1];
            if(ptmp1.parent==ptmp0.n)
            {
                //std::cout<<"111"<<endl;
                if(ptmp0.x>b.min_x&&ptmp0.x<=b.max_x
                        &&ptmp0.y>b.min_y&&ptmp0.y<=b.max_y
                        &&ptmp0.z>b.min_z&&ptmp0.z<=b.max_z
                        &&ptmp1.z>b.max_z)
                {
                    p.nswc.x=ptmp0.x;
                    p.nswc.y=ptmp0.y;
                    p.nswc.z=ptmp0.z;
                    p.t=point::up;
                    pv.push_back(p);
                    std::cout<<"111"<<endl;
                }
                if(ptmp0.x>b.min_x&&ptmp0.x<=b.max_x
                        &&ptmp0.y>b.min_y&&ptmp0.y<=b.max_y
                        &&ptmp0.z>b.min_z&&ptmp0.z<=b.max_z
                        &&ptmp1.z<b.min_z)
                {
                    p.nswc.x=ptmp0.x;
                    p.nswc.y=ptmp0.y;
                    p.nswc.z=ptmp0.z;
                    p.t=point::down;
                    pv.push_back(p);
                    std::cout<<"222"<<endl;
                }
                if(ptmp0.x>b.min_x&&ptmp0.x<=b.max_x
                        &&ptmp0.y>b.min_y&&ptmp0.y<=b.max_y
                        &&ptmp0.z>b.min_z&&ptmp0.z<=b.max_z
                        &&ptmp1.y<b.min_y)
                {
                    p.nswc.x=ptmp0.x;
                    p.nswc.y=ptmp0.y;
                    p.nswc.z=ptmp0.z;
                    p.t=point::left;
                    pv.push_back(p);
                    std::cout<<"333"<<endl;
                }
                if(ptmp0.x>b.min_x&&ptmp0.x<=b.max_x
                        &&ptmp0.y>b.min_y&&ptmp0.y<=b.max_y
                        &&ptmp0.z>b.min_z&&ptmp0.z<=b.max_z
                        &&ptmp1.y>b.max_y)
                {
                    p.nswc.x=ptmp0.x;
                    p.nswc.y=ptmp0.y;
                    p.nswc.z=ptmp0.z;
                    p.t=point::right;
                    pv.push_back(p);
                    std::cout<<"444"<<endl;
                }
                if(ptmp0.x>b.min_x&&ptmp0.x<=b.max_x
                        &&ptmp0.y>b.min_y&&ptmp0.y<=b.max_y
                        &&ptmp0.z>b.min_z&&ptmp0.z<=b.max_z
                        &&ptmp1.x>b.max_x)
                {
                    p.nswc.x=ptmp0.x;
                    p.nswc.y=ptmp0.y;
                    p.nswc.z=ptmp0.z;
                    p.t=point::front;
                    pv.push_back(p);
                    std::cout<<"555"<<endl;
                }
                if(ptmp0.x>b.min_x&&ptmp0.x<=b.max_x
                        &&ptmp0.y>b.min_y&&ptmp0.y<=b.max_y
                        &&ptmp0.z>b.min_z&&ptmp0.z<=b.max_z
                        &&ptmp1.x<b.min_x)
                {
                    p.nswc.x=ptmp0.x;
                    p.nswc.y=ptmp0.y;
                    p.nswc.z=ptmp0.z;
                    p.t=point::back;
                    pv.push_back(p);
                    std::cout<<"666"<<endl;
                }
            }
        }

    }

    if(po.t==point::up)
    {
        for(int i=0;i<size-1;++i)
        {
            ptmp0=nt.listNeuron[i];
            ptmp1=nt.listNeuron[i+1];
            if(ptmp1.parent==ptmp0.n)
            {
                if(ptmp0.x>b.min_x&&ptmp0.x<=b.max_x
                        &&ptmp0.y>b.min_y&&ptmp0.y<=b.max_y
                        &&ptmp0.z>b.min_z&&ptmp0.z<=b.max_z
                        &&ptmp1.z>b.max_z)
                {
                    p.nswc.x=ptmp0.x;
                    p.nswc.y=ptmp0.y;
                    p.nswc.z=ptmp0.z;
                    p.t=point::up;
                    pv.push_back(p);
                }
                if(ptmp0.x>b.min_x&&ptmp0.x<=b.max_x
                        &&ptmp0.y>b.min_y&&ptmp0.y<=b.max_y
                        &&ptmp0.z>b.min_z&&ptmp0.z<=b.max_z
                        &&ptmp1.y<b.min_y)
                {
                    p.nswc.x=ptmp0.x;
                    p.nswc.y=ptmp0.y;
                    p.nswc.z=ptmp0.z;
                    p.t=point::left;
                    pv.push_back(p);
                }
                if(ptmp0.x>b.min_x&&ptmp0.x<=b.max_x
                        &&ptmp0.y>b.min_y&&ptmp0.y<=b.max_y
                        &&ptmp0.z>b.min_z&&ptmp0.z<=b.max_z
                        &&ptmp1.y>b.max_y)
                {
                    p.nswc.x=ptmp0.x;
                    p.nswc.y=ptmp0.y;
                    p.nswc.z=ptmp0.z;
                    p.t=point::right;
                    pv.push_back(p);
                }
                if(ptmp0.x>b.min_x&&ptmp0.x<=b.max_x
                        &&ptmp0.y>b.min_y&&ptmp0.y<=b.max_y
                        &&ptmp0.z>b.min_z&&ptmp0.z<=b.max_z
                        &&ptmp1.x>b.max_x)
                {
                    p.nswc.x=ptmp0.x;
                    p.nswc.y=ptmp0.y;
                    p.nswc.z=ptmp0.z;
                    p.t=point::front;
                    pv.push_back(p);
                }
                if(ptmp0.x>b.min_x&&ptmp0.x<=b.max_x
                        &&ptmp0.y>b.min_y&&ptmp0.y<=b.max_y
                        &&ptmp0.z>b.min_z&&ptmp0.z<=b.max_z
                        &&ptmp1.x<b.min_x)
                {
                    p.nswc.x=ptmp0.x;
                    p.nswc.y=ptmp0.y;
                    p.nswc.z=ptmp0.z;
                    p.t=point::back;
                    pv.push_back(p);
                }
            }
        }

    }

    if(po.t==point::left)
    {
        for(int i=0;i<size-1;++i)
        {
            ptmp0=nt.listNeuron[i];
            ptmp1=nt.listNeuron[i+1];
            if(ptmp1.parent==ptmp0.n)
            {
                if(ptmp0.x>b.min_x&&ptmp0.x<=b.max_x
                        &&ptmp0.y>b.min_y&&ptmp0.y<=b.max_y
                        &&ptmp0.z>b.min_z&&ptmp0.z<=b.max_z
                        &&ptmp1.z>b.max_z)
                {
                    p.nswc.x=ptmp0.x;
                    p.nswc.y=ptmp0.y;
                    p.nswc.z=ptmp0.z;
                    p.t=point::up;
                    pv.push_back(p);
                }
                if(ptmp0.x>b.min_x&&ptmp0.x<=b.max_x
                        &&ptmp0.y>b.min_y&&ptmp0.y<=b.max_y
                        &&ptmp0.z>b.min_z&&ptmp0.z<=b.max_z
                        &&ptmp1.z<b.min_z)
                {
                    p.nswc.x=ptmp0.x;
                    p.nswc.y=ptmp0.y;
                    p.nswc.z=ptmp0.z;
                    p.t=point::down;
                    pv.push_back(p);
                }
                if(ptmp0.x>b.min_x&&ptmp0.x<=b.max_x
                        &&ptmp0.y>b.min_y&&ptmp0.y<=b.max_y
                        &&ptmp0.z>b.min_z&&ptmp0.z<=b.max_z
                        &&ptmp1.y<b.min_y)
                {
                    p.nswc.x=ptmp0.x;
                    p.nswc.y=ptmp0.y;
                    p.nswc.z=ptmp0.z;
                    p.t=point::left;
                    pv.push_back(p);
                }
                if(ptmp0.x>b.min_x&&ptmp0.x<=b.max_x
                        &&ptmp0.y>b.min_y&&ptmp0.y<=b.max_y
                        &&ptmp0.z>b.min_z&&ptmp0.z<=b.max_z
                        &&ptmp1.x>b.max_x)
                {
                    p.nswc.x=ptmp0.x;
                    p.nswc.y=ptmp0.y;
                    p.nswc.z=ptmp0.z;
                    p.t=point::front;
                    pv.push_back(p);
                }
                if(ptmp0.x>b.min_x&&ptmp0.x<=b.max_x
                        &&ptmp0.y>b.min_y&&ptmp0.y<=b.max_y
                        &&ptmp0.z>b.min_z&&ptmp0.z<=b.max_z
                        &&ptmp1.x<b.min_x)
                {
                    p.nswc.x=ptmp0.x;
                    p.nswc.y=ptmp0.y;
                    p.nswc.z=ptmp0.z;
                    p.t=point::back;
                    pv.push_back(p);
                }
            }
        }

    }

    if(po.t==point::right)
    {
        for(int i=0;i<size-1;++i)
        {
            ptmp0=nt.listNeuron[i];
            ptmp1=nt.listNeuron[i+1];
            if(ptmp1.parent==ptmp0.n)
            {
                if(ptmp0.x>b.min_x&&ptmp0.x<=b.max_x
                        &&ptmp0.y>b.min_y&&ptmp0.y<=b.max_y
                        &&ptmp0.z>b.min_z&&ptmp0.z<=b.max_z
                        &&ptmp1.z>b.max_z)
                {
                    p.nswc.x=ptmp0.x;
                    p.nswc.y=ptmp0.y;
                    p.nswc.z=ptmp0.z;
                    p.t=point::up;
                    pv.push_back(p);
                }
                if(ptmp0.x>b.min_x&&ptmp0.x<=b.max_x
                        &&ptmp0.y>b.min_y&&ptmp0.y<=b.max_y
                        &&ptmp0.z>b.min_z&&ptmp0.z<=b.max_z
                        &&ptmp1.z<b.min_z)
                {
                    p.nswc.x=ptmp0.x;
                    p.nswc.y=ptmp0.y;
                    p.nswc.z=ptmp0.z;
                    p.t=point::down;
                    pv.push_back(p);
                }
                if(ptmp0.x>b.min_x&&ptmp0.x<=b.max_x
                        &&ptmp0.y>b.min_y&&ptmp0.y<=b.max_y
                        &&ptmp0.z>b.min_z&&ptmp0.z<=b.max_z
                        &&ptmp1.y>b.max_y)
                {
                    p.nswc.x=ptmp0.x;
                    p.nswc.y=ptmp0.y;
                    p.nswc.z=ptmp0.z;
                    p.t=point::right;
                    pv.push_back(p);
                }
                if(ptmp0.x>b.min_x&&ptmp0.x<=b.max_x
                        &&ptmp0.y>b.min_y&&ptmp0.y<=b.max_y
                        &&ptmp0.z>b.min_z&&ptmp0.z<=b.max_z
                        &&ptmp1.x>b.max_x)
                {
                    p.nswc.x=ptmp0.x;
                    p.nswc.y=ptmp0.y;
                    p.nswc.z=ptmp0.z;
                    p.t=point::front;
                    pv.push_back(p);
                }
                if(ptmp0.x>b.min_x&&ptmp0.x<=b.max_x
                        &&ptmp0.y>b.min_y&&ptmp0.y<=b.max_y
                        &&ptmp0.z>b.min_z&&ptmp0.z<=b.max_z
                        &&ptmp1.x<b.min_x)
                {
                    p.nswc.x=ptmp0.x;
                    p.nswc.y=ptmp0.y;
                    p.nswc.z=ptmp0.z;
                    p.t=point::back;
                    pv.push_back(p);
                }
            }
        }

    }

    if(po.t==point::front)
    {
        for(int i=0;i<size-1;++i)
        {
            ptmp0=nt.listNeuron[i];
            ptmp1=nt.listNeuron[i+1];
            if(ptmp1.parent==ptmp0.n)
            {
                if(ptmp0.x>b.min_x&&ptmp0.x<=b.max_x
                        &&ptmp0.y>b.min_y&&ptmp0.y<=b.max_y
                        &&ptmp0.z>b.min_z&&ptmp0.z<=b.max_z
                        &&ptmp1.z>b.max_z)
                {
                    p.nswc.x=ptmp0.x;
                    p.nswc.y=ptmp0.y;
                    p.nswc.z=ptmp0.z;
                    p.t=point::up;
                    pv.push_back(p);
                }
                if(ptmp0.x>b.min_x&&ptmp0.x<=b.max_x
                        &&ptmp0.y>b.min_y&&ptmp0.y<=b.max_y
                        &&ptmp0.z>b.min_z&&ptmp0.z<=b.max_z
                        &&ptmp1.z<b.min_z)
                {
                    p.nswc.x=ptmp0.x;
                    p.nswc.y=ptmp0.y;
                    p.nswc.z=ptmp0.z;
                    p.t=point::down;
                    pv.push_back(p);
                }
                if(ptmp0.x>b.min_x&&ptmp0.x<=b.max_x
                        &&ptmp0.y>b.min_y&&ptmp0.y<=b.max_y
                        &&ptmp0.z>b.min_z&&ptmp0.z<=b.max_z
                        &&ptmp1.y<b.min_y)
                {
                    p.nswc.x=ptmp0.x;
                    p.nswc.y=ptmp0.y;
                    p.nswc.z=ptmp0.z;
                    p.t=point::left;
                    pv.push_back(p);
                }
                if(ptmp0.x>b.min_x&&ptmp0.x<=b.max_x
                        &&ptmp0.y>b.min_y&&ptmp0.y<=b.max_y
                        &&ptmp0.z>b.min_z&&ptmp0.z<=b.max_z
                        &&ptmp1.y>b.max_y)
                {
                    p.nswc.x=ptmp0.x;
                    p.nswc.y=ptmp0.y;
                    p.nswc.z=ptmp0.z;
                    p.t=point::right;
                    pv.push_back(p);
                }
                if(ptmp0.x>b.min_x&&ptmp0.x<=b.max_x
                        &&ptmp0.y>b.min_y&&ptmp0.y<=b.max_y
                        &&ptmp0.z>b.min_z&&ptmp0.z<=b.max_z
                        &&ptmp1.x>b.max_x)
                {
                    p.nswc.x=ptmp0.x;
                    p.nswc.y=ptmp0.y;
                    p.nswc.z=ptmp0.z;
                    p.t=point::front;
                    pv.push_back(p);
                }
            }
        }

    }

    if(po.t==point::back)
    {
        for(int i=0;i<size-1;++i)
        {
            ptmp0=nt.listNeuron[i];
            ptmp1=nt.listNeuron[i+1];
            if(ptmp1.parent==ptmp0.n)
            {
                if(ptmp0.x>b.min_x&&ptmp0.x<=b.max_x
                        &&ptmp0.y>b.min_y&&ptmp0.y<=b.max_y
                        &&ptmp0.z>b.min_z&&ptmp0.z<=b.max_z
                        &&ptmp1.z>b.max_z)
                {
                    p.nswc.x=ptmp0.x;
                    p.nswc.y=ptmp0.y;
                    p.nswc.z=ptmp0.z;
                    p.t=point::up;
                    pv.push_back(p);
                }
                if(ptmp0.x>b.min_x&&ptmp0.x<=b.max_x
                        &&ptmp0.y>b.min_y&&ptmp0.y<=b.max_y
                        &&ptmp0.z>b.min_z&&ptmp0.z<=b.max_z
                        &&ptmp1.z<b.min_z)
                {
                    p.nswc.x=ptmp0.x;
                    p.nswc.y=ptmp0.y;
                    p.nswc.z=ptmp0.z;
                    p.t=point::down;
                    pv.push_back(p);
                }
                if(ptmp0.x>b.min_x&&ptmp0.x<=b.max_x
                        &&ptmp0.y>b.min_y&&ptmp0.y<=b.max_y
                        &&ptmp0.z>b.min_z&&ptmp0.z<=b.max_z
                        &&ptmp1.y<b.min_y)
                {
                    p.nswc.x=ptmp0.x;
                    p.nswc.y=ptmp0.y;
                    p.nswc.z=ptmp0.z;
                    p.t=point::left;
                    pv.push_back(p);
                }
                if(ptmp0.x>b.min_x&&ptmp0.x<=b.max_x
                        &&ptmp0.y>b.min_y&&ptmp0.y<=b.max_y
                        &&ptmp0.z>b.min_z&&ptmp0.z<=b.max_z
                        &&ptmp1.y>b.max_y)
                {
                    p.nswc.x=ptmp0.x;
                    p.nswc.y=ptmp0.y;
                    p.nswc.z=ptmp0.z;
                    p.t=point::right;
                    pv.push_back(p);
                }
                if(ptmp0.x>b.min_x&&ptmp0.x<=b.max_x
                        &&ptmp0.y>b.min_y&&ptmp0.y<=b.max_y
                        &&ptmp0.z>b.min_z&&ptmp0.z<=b.max_z
                        &&ptmp1.x<b.min_x)
                {
                    p.nswc.x=ptmp0.x;
                    p.nswc.y=ptmp0.y;
                    p.nswc.z=ptmp0.z;
                    p.t=point::back;
                    pv.push_back(p);
                }
            }
        }

    }

    return pv;

    vector<point> pv;
    NeuronSWC ptmp0,ptmp1;
    point p;
    int size=nt.listNeuron.size();
    for(int i=0;i<size-1;++i)
    {
        ptmp0=nt.listNeuron[i];
        ptmp1=nt.listNeuron[i+1];
        if(ptmp1.parent==ptmp0.n
                &&ptmp0.x>b.min_x&&ptmp0.x<=b.max_x
                &&ptmp0.y>b.min_y&&ptmp0.y<=b.max_y
                &&ptmp0.z>b.min_z&&ptmp0.z<=b.max_z)
        {
            if(ptmp1.z>b.max_z
                    &&ptmp1.x>b.min_x&&ptmp1.x<=b.max_x
                    &&ptmp1.y>b.min_y&&ptmp1.y<=b.max_y
                    &&b.o.t!=point::down)
            {
                p.nswc.x=ptmp0.x;
                p.nswc.y=ptmp0.y;
                p.nswc.z=ptmp0.z;
                p.t=point::up;
                pv.push_back(p);
            }
            if(ptmp1.z<b.min_z
                    &&ptmp1.y>b.min_x&&ptmp1.x<=b.max_x
                    &&ptmp1.y>b.min_y&&ptmp1.y<=b.max_y
                    &&b.o.t!=point::up)
            {
                p.nswc.x=ptmp0.x;
                p.nswc.y=ptmp0.y;
                p.nswc.z=ptmp0.z;
                p.t=point::down;
                pv.push_back(p);
            }
            if(ptmp1.y<b.min_y
                    &&ptmp1.x>b.min_x&&ptmp1.x<=b.max_x
                    &&ptmp1.z>b.min_z&&ptmp1.z<=b.max_z
                    &&b.o.t!=point::right)
            {
                p.nswc.x=ptmp0.x;
                p.nswc.y=ptmp0.y;
                p.nswc.z=ptmp0.z;
                p.t=point::left;
                pv.push_back(p);
            }
            if(ptmp1.y>b.max_y
                    &&ptmp1.x>b.min_x&&ptmp1.x<=b.max_x
                    &&ptmp1.z>b.min_z&&ptmp1.z<=b.max_z
                    &&b.o.t!=point::left)
            {
                p.nswc.x=ptmp0.x;
                p.nswc.y=ptmp0.y;
                p.nswc.z=ptmp0.z;
                p.t=point::right;
                pv.push_back(p);
            }
            if(ptmp1.x<b.min_x
                    &&ptmp1.y>b.min_y&&ptmp1.y<=b.max_y
                    &&ptmp1.z>b.min_z&&ptmp1.z<=b.max_z
                    &&b.o.t!=point::front)
            {
                p.nswc.x=ptmp0.x;
                p.nswc.y=ptmp0.y;
                p.nswc.z=ptmp0.z;
                p.t=point::back;
                pv.push_back(p);
            }
            if(ptmp1.x>b.max_x
                    &&ptmp1.y>b.min_y&&ptmp1.y<=b.max_y
                    &&ptmp1.z>b.min_z&&ptmp1.z<=b.max_z
                    &&b.o.t!=point::back)
            {
                p.nswc.x=ptmp0.x;
                p.nswc.y=ptmp0.y;
                p.nswc.z=ptmp0.z;
                p.t=point::front;
                pv.push_back(p);
            }
        }
    }
    return pv;
}*/

vector<point> getTPointOfBlock(vector<point> pts, block b)
{
    vector<point> tips;
    point tmp0,tmp1;
    tmp0=b.o;
    bool flag=false;
    bool flag1=false;

    while(tmp0.nswc.x>b.min_x&&tmp0.nswc.x<=b.max_x
          &&tmp0.nswc.y>b.min_y&&tmp0.nswc.y<=b.max_y
          &&tmp0.nswc.z>b.min_z&&tmp0.nswc.z<=b.max_z)
    {
        if(tmp0.children.size()==0)
        {
            if(flag==false)
            {
               tmp1=pts[pts[tmp0.branch-1].children[1]-1];
               flag=true;
            }else
            {
               tmp1=pts[pts[pts[tmp0.branch-1].branch-1].children[1]-1];
            }
        }else
        {
            tmp1=pts[tmp0.children[0]-1];
        }
        if(tmp1.nswc.x==pts[tmp0.nswc.n].nswc.x
                &&tmp1.nswc.y==pts[tmp0.nswc.n].nswc.y
                &&tmp1.nswc.z==pts[tmp0.nswc.n].nswc.z
                &&tmp1.nswc.x>b.max_x)
        {
            tmp0.t=point::up;
            tips.push_back(tmp0);
            if(flag==false)
            {
                tmp0=pts[pts[tmp0.branch-1].children[1]-1];
                flag=true;
                flag1=true;
            }else
            {
                tmp0=pts[pts[pts[tmp0.branch-1].branch-1].children[1]-1];
                flag1=true;
            }

        }
        if(tmp1.nswc.x==pts[tmp0.nswc.n].nswc.x
                &&tmp1.nswc.y==pts[tmp0.nswc.n].nswc.y
                &&tmp1.nswc.z==pts[tmp0.nswc.n].nswc.z
                &&tmp1.nswc.x<b.min_x)
        {
            tmp0.t=point::down;
            tips.push_back(tmp0);
            if(flag==false)
            {
                tmp0=pts[pts[tmp0.branch-1].children[1]-1];
                flag=true;
                flag1=true;
            }else
            {
                tmp0=pts[pts[pts[tmp0.branch-1].branch-1].children[1]-1];
                flag1=true;
            }

        }
        if(tmp1.nswc.x==pts[tmp0.nswc.n].nswc.x
                &&tmp1.nswc.y==pts[tmp0.nswc.n].nswc.y
                &&tmp1.nswc.z==pts[tmp0.nswc.n].nswc.z
                &&tmp1.nswc.y>b.max_y)
        {
            tmp0.t=point::right;
            tips.push_back(tmp0);
            if(flag==false)
            {
                tmp0=pts[pts[tmp0.branch-1].children[1]-1];
                flag=true;
                flag1=true;
            }else
            {
                tmp0=pts[pts[pts[tmp0.branch-1].branch-1].children[1]-1];
                flag1=true;
            }

        }
        if(tmp1.nswc.x==pts[tmp0.nswc.n].nswc.x
                &&tmp1.nswc.y==pts[tmp0.nswc.n].nswc.y
                &&tmp1.nswc.z==pts[tmp0.nswc.n].nswc.z
                &&tmp1.nswc.y<b.min_y)
        {
            tmp0.t=point::left;
            tips.push_back(tmp0);
            if(flag==false)
            {
                tmp0=pts[pts[tmp0.branch-1].children[1]-1];
                flag=true;
                flag1=true;
            }else
            {
                tmp0=pts[pts[pts[tmp0.branch-1].branch-1].children[1]-1];
                flag1=true;
            }

        }
        if(tmp1.nswc.x==pts[tmp0.nswc.n].nswc.x
                &&tmp1.nswc.y==pts[tmp0.nswc.n].nswc.y
                &&tmp1.nswc.z==pts[tmp0.nswc.n].nswc.z
                &&tmp1.nswc.x>b.max_x)
        {
            tmp0.t=point::front;
            tips.push_back(tmp0);
            if(flag==false)
            {
                tmp0=pts[pts[tmp0.branch-1].children[1]-1];
                flag=true;
                flag1=true;
            }else
            {
                tmp0=pts[pts[pts[tmp0.branch-1].branch-1].children[1]-1];
                flag1=true;
            }

        }
        if(tmp1.nswc.x==pts[tmp0.nswc.n].nswc.x
                &&tmp1.nswc.y==pts[tmp0.nswc.n].nswc.y
                &&tmp1.nswc.z==pts[tmp0.nswc.n].nswc.z
                &&tmp1.nswc.x<b.min_x)
        {
            tmp0.t=point::back;
            tips.push_back(tmp0);
            if(flag==false)
            {
                tmp0=pts[pts[tmp0.branch-1].children[1]-1];
                flag=true;
                flag1=true;
            }else
            {
                tmp0=pts[pts[pts[tmp0.branch-1].branch-1].children[1]-1];
                flag1=true;
            }

        }
        if(flag1==false)
        {
            tmp0=tmp1;
        }
        flag1=false;
    }

    return tips;
}

blockTree getBlockTree(vector<point> nt, int dx, int dy, int dz)
{
    point o;
    o=nt[0];
    o.t=point::ori;
    point ptmp;
    blockTree bt;
    block btmp;
    vector<point> ptmps,pzs;
    btmp=getBlockOfOPoint(o,dx,dy,dz);
    btmp.n=0;
    btmp.parent=-1;
    ptmps=getTPointOfBlock(nt,btmp);
    std::cout<<ptmps.size()<<endl;
    btmp.tips.assign(ptmps.begin(),ptmps.end());
    pzs.insert(pzs.end(),ptmps.begin(),ptmps.end());


    bt.blocklist.push_back(btmp);
    int count=1;
    while(pzs.size()!=0)
    {
        ptmps.clear();
        ptmp=*(pzs.end()-1);
        pzs.pop_back();
        btmp=getBlockOfOPoint(ptmp,dx,dy,dz);
        btmp.n=bt.blocklist.size()-1;
        for(int i=0;i<bt.blocklist.size();++i)
        {
            for(int j=0;j<bt.blocklist[i].tips.size();++j)
            {
                if(btmp.o.nswc.x==bt.blocklist[i].tips[j].nswc.x
                        &&btmp.o.nswc.y==bt.blocklist[i].tips[j].nswc.y
                        &&btmp.o.nswc.z==bt.blocklist[i].tips[j].nswc.z)
                    btmp.parent=bt.blocklist[i].n;
            }

        }
        ptmps=getTPointOfBlock(nt,btmp);
        btmp.tips.assign(ptmps.begin(),ptmps.end());
        if(ptmps.size()!=0)
        {
            pzs.insert(pzs.end(),ptmps.begin(),ptmps.end());
        }
        bt.blocklist.push_back(btmp);
        std::cout<<count<<endl;
        count++;
    }

    return bt;


}






