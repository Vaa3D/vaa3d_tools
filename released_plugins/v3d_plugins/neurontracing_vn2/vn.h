//last change: by Hanchuan Peng. 2012-12-30

#ifndef __VAA3DNEURON_PARA_H__
#define __VAA3DNEURON_PARA_H__

#include <QtGui>
#include <v3d_interface.h>

#include "vn_imgpreprocess.h"

struct PARA_VN //VN - V3DNeuron
{
    Image4DSimple * p4dImage;
    int xc0, xc1, yc0, yc1, zc0, zc1; //the six bounding box boundaries
    LandmarkList landmarks;
    PARA_VN() 
    {
        p4dImage = NULL;
        xc0 = xc1 = yc0 = yc1 = zc0 = zc1 = 0;
        landmarks.clear();
    }
    bool initialize(V3DPluginCallback2 &callback)
    {
        v3dhandle curwin = callback.currentImageWindow();
        p4dImage = callback.getImage(curwin);
        if (!p4dImage)
        {
            v3d_msg("The input image is NULL. Do nothing.");
            return false;
        }
        landmarks = callback.getLandmark(curwin);
        V3DLONG in_sz0[4] = {p4dImage->getXDim(), p4dImage->getYDim(), p4dImage->getZDim(), p4dImage->getCDim()};
        
        V3DLONG view3d_datasz0, view3d_datasz1, view3d_datasz2;
        View3DControl * view3d = callback.getView3DControl(curwin);
        if (!view3d)
        {
            v3d_msg("The view3d pointer is NULL which indicates there is no 3D viewer window open, thus set the cut-box as max.",0);
            
            xc0 = 0;
            xc1 = in_sz0[0]-1;
            
            yc0 = 0;
            yc1 = in_sz0[1]-1;
            
            zc0 = 0;
            zc1 = in_sz0[2]-1;
        }
        else
        {
            view3d_datasz0 = view3d->dataDim1();
            view3d_datasz1 = view3d->dataDim2();
            view3d_datasz2 = view3d->dataDim3();
            
            xc0 = int(double(view3d->xCut0()) * in_sz0[0] / view3d_datasz0 + 0.5);
            xc1 = int(double(view3d->xCut1()) * in_sz0[0] / view3d_datasz0 + 0.5);
            if (xc1>in_sz0[0]-1) xc1 = in_sz0[0]-1;
            //xc1 = in_sz0[0]-1;//for debug purpose. 130102
            
            yc0 = int(double(view3d->yCut0()) * in_sz0[1] / view3d_datasz1 + 0.5);
            yc1 = int(double(view3d->yCut1()) * in_sz0[1] / view3d_datasz1 + 0.5);
            if (yc1>in_sz0[1]-1) yc1 = in_sz0[1]-1;
            
            zc0 = int(double(view3d->zCut0()) * in_sz0[2] / view3d_datasz2 + 0.5);
            zc1 = int(double(view3d->zCut1()) * in_sz0[2] / view3d_datasz2 + 0.5);
            if (zc1>in_sz0[2]-1) zc1 = in_sz0[2]-1;
            
            //printf("%5.3f, %5.3f, %5.3f\n", float(view3d->xCut1()), float(in_sz0[0]), float(view3d_datasz0));
        }

        if(landmarks.size()>0)
        {
            LocationSimple t;
            for(int i = 0; i < landmarks.size(); i++)
            {
                t.x = landmarks[i].x-1;
                t.y = landmarks[i].y-1;
                t.z = landmarks[i].z-1;

                if(t.x<xc0 || t.x>xc1 || t.y<yc0 || t.y>yc1 || t.z<zc0 || t.z>zc1)
                {


                    if(i==0)
                    {
                        v3d_msg("The first marker is invalid.");
                        return false;
                    }
                    else
                        landmarks.removeAt(i);
                }
            }


        }
        return true;
    }
    
};

#endif
