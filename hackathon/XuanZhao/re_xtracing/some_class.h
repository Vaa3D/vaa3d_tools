#ifndef SOME_CLASS_H
#define SOME_CLASS_H

#include <QtGui>
#include <v3d_interface.h>
#include <vector>
using namespace std;

struct unit_p
{
    double n,parent;
    double type,x,y,z,r,
    nchild,segid,nodeinsegid,
    level,creatmode;
    unit_p() {n=parent=type=x=y=z=r=nchild=segid=nodeinsegid=level=creatmode=0;}
};

struct unit_seg
{
    double seg_id;
    vector<unit_p> seg_tree;
    vector<unit_p> interface_p;
    unit_seg() {seg_id=0;seg_tree.clear();interface_p.clear();}
};

struct PARA_T
{
    Image4DSimple* p4dImage;
    int xc0,xc1,yc0,yc1,zc0,zc1;
    LandmarkList landmarks;

    bool is_gsdt;
    double visible_thres;
    int channel;


    PARA_T()
    {
        p4dImage = NULL;
        xc0 = xc1 = yc0 = yc1 = zc0 = zc1 = 0;
        landmarks.clear();

        is_gsdt = true;
        visible_thres = 25;
        channel = 0;
    }

    bool initialize(V3DPluginCallback2 &callback)
    {
        v3dhandle curwin = callback.currentImageWindow();
        p4dImage = callback.getImage(curwin);
        if(!p4dImage)
        {
            v3d_msg("The input image is NULL. Do nothing.");
            return false;
        }
        landmarks = callback.getLandmark(curwin);
        V3DLONG in_sz0[4] = {p4dImage->getXDim(),p4dImage->getYDim(),p4dImage->getZDim(),p4dImage->getCDim()};
        V3DLONG view3d_datasz0,view3d_datasz1,view3d_datasz2;
        View3DControl* view3d = callback.getView3DControl(curwin);
        if(!view3d)
        {
            v3d_msg("The view3d pointer is NULL which indicates there is no 3D viewer window open, thus set the cut-box as max.",0);
            xc0 = 0;
            xc1 = in_sz0[0] - 1;

            yc0 = 0;
            yc1 = in_sz0[1] - 1;

            zc0 = 0;
            zc1 = in_sz0[2] - 1;
        }
        else
        {
            view3d_datasz0 = view3d->dataDim1();
            view3d_datasz1 = view3d->dataDim2();
            view3d_datasz2 = view3d->dataDim3();

            xc0 = int(double(view3d->xCut0()) * in_sz0[0] / view3d_datasz0 + 0.5);
            xc1 = int(double(view3d->xCut1()) * in_sz0[0] / view3d_datasz0 + 0.5);
            if(xc1 > in_sz0[0] - 1) xc1 = in_sz0[0] - 1;

            yc0 = int(double(view3d->yCut0()) * in_sz0[1] / view3d_datasz1 + 0.5);
            yc1 = int(double(view3d->yCut1()) * in_sz0[1] / view3d_datasz1 + 0.5);
            if (yc1 > in_sz0[1] - 1) yc1 = in_sz0[1] - 1;

            zc0 = int(double(view3d->zCut0()) * in_sz0[2] / view3d_datasz2 + 0.5);
            zc1 = int(double(view3d->zCut1()) * in_sz0[2] / view3d_datasz2 + 0.5);
            if (zc1 > in_sz0[2] - 1) zc1 = in_sz0[2] - 1;

        }

        if(landmarks.size() > 0)
        {
            LocationSimple t;
            for(int i = 0; i < landmarks.size(); ++i)
            {
                t.x = landmarks[i].x;
                t.y = landmarks[i].y;
                t.z = landmarks[i].z;

                if(t.x < xc0 + 1 || t.x > xc1 + 1 || t.y < yc0 + 1 || t.y > yc1 + 1 || t.z < zc0 + 1 || t.z > zc1 + 1)
                {


                    if(i == 0)
                    {
                        v3d_msg("The first marker is invalid.");
                        return false;
                    }
                    else
                    {
                        landmarks.removeAt(i);
                        i--;
                    }

                }

            }
        }

        return true;
    }

    bool t_dialog()
    {

    }

};


template<class T>
class ImageCtrl{
private:
    T* data;
    V3DLONG sz[4];
public:
    ImageCtrl(){
        data=0;
        sz=0;
    }
    ImageCtrl(T* _data,V3DLONG* _sz)
    {
        for(int i=0;i<4;++i)
        {
            sz[i]=_sz[i];
        }
        V3DLONG num_sz=sz[0]*sz[1]*sz[2];
        data=new T[num_sz];
        if(sz[3]==1)
        {
            for(V3DLONG i=0;i<num_sz;++i)
            {
                data[i]=_data[i];
            }
        }
    }
    ~ImageCtrl()
    {
        if(data) delete[] data;
        Image4DSimple
    }
}









































#endif // SOME_CLASS_H
