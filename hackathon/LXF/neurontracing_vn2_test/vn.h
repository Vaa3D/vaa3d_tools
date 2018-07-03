//last change: by Hanchuan Peng. 2012-12-30

#ifndef __VAA3DNEURON_PARA_H__
#define __VAA3DNEURON_PARA_H__

#include <QtGui>
#include <v3d_interface.h>
#include <iostream>

#include "vn_imgpreprocess.h"
using namespace std;
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

//        V3DLONG M = in_sz0[0];
//        V3DLONG N = in_sz0[1];
//        V3DLONG P = in_sz0[2];
//        V3DLONG C = in_sz0[3];
//        cout<<"MNP = "<<M<<"  "<<N<<"  "<<P<<"  "<<endl;

//        unsigned char* data1d = p4dImage->getRawData();
//        unsigned char* im_cropped = 0;
//        V3DLONG pagesz;
//        pagesz = in_sz0[0]*in_sz0[1]*in_sz0[2]*in_sz0[3];
//        try {im_cropped = new unsigned char [pagesz];}
//        catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return false;}
//        V3DLONG j = 0;

//        for(V3DLONG iz = 0; iz < in_sz0[2]; iz++)
//        {
//            V3DLONG offsetk = iz*M*N;
//            for(V3DLONG iy = 0; iy < in_sz0[1]; iy++)
//            {
//                V3DLONG offsetj = iy*N;
//                for(V3DLONG ix = 0; ix < in_sz0[0]; ix++)
//                {
//                     im_cropped[j] = data1d[offsetk + offsetj + ix];
//                     //cout<<"before = "<<int(im_cropped[j])<<endl;
//                     int count = 0;
//                     if(int(im_cropped[j])>18&&int(im_cropped[j])<20)
//                     {
//                         int tmpdata1d=0;
//                         for(V3DLONG izi = iz-1 ; izi < iz+2; izi++)
//                         {
//                            if(izi<0)izi=0;
//                            if(izi>P)izi=P;
//                             V3DLONG offsetki = izi*M*N;
//                             for(V3DLONG iyi = iy-1; iyi < iy+2; iyi++)
//                             {
//                                 if(iyi<0)iyi=0;
//                                 if(iyi>N)iyi=N;

//                                 V3DLONG offsetjj = iyi*N;
//                                 for(V3DLONG ixi = ix-1; ixi < ix+2; ixi++)
//                                 {
//                                     if(ixi<0)ixi=0;
//                                     if(ixi>M)ixi=M;
//                                     if(int(data1d[offsetki + offsetjj + ixi])>20)
//                                     {
//                                         count++;
//                                         tmpdata1d = tmpdata1d + int(data1d[offsetki + offsetjj + ixi]);
//                                     }
//                                 }
//                             }
//                         }
//                         if(count!=0)
//                         {
//                            im_cropped[j] = tmpdata1d/count;
//                         }
//                     }
//                     j++;
//                }
//            }
//        }
//        p4dImage->setData(im_cropped,M,N,P,C,V3D_UINT8);

        
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
                t.x = landmarks[i].x;
                t.y = landmarks[i].y;
                t.z = landmarks[i].z;

                if(t.x<xc0+1 || t.x>xc1+1 || t.y<yc0+1 || t.y>yc1+1 || t.z<zc0+1 || t.z>zc1+1)
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
