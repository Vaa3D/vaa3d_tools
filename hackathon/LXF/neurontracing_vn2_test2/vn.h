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



        /**********************************change here by LXF************************************************/
        //v.1

        //        V3DLONG M = in_sz0[0];
        //        V3DLONG N = in_sz0[1];
        //        V3DLONG P = in_sz0[2];
        //        V3DLONG C = in_sz0[3];
        //       // cout<<"MNP = "<<M<<"  "<<N<<"  "<<P<<"  "<<endl;

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
        //                    im_cropped[j] = data1d[offsetk + offsetj + ix];
        //                    //cout<<"before = "<<int(im_cropped[j])<<endl;
        //                    int count = 0;
        //                    if(int(im_cropped[j])>18&&int(im_cropped[j])<20)
        //                    {
        //                        int tmpdata1d=0;
        //                        for(V3DLONG izi = iz-1 ; izi < iz+2; izi++)
        //                        {
        //                            if(izi<0)izi=0;
        //                            if(izi>P)izi=P;
        //                            V3DLONG offsetki = izi*M*N;
        //                            for(V3DLONG iyi = iy-1; iyi < iy+2; iyi++)
        //                            {
        //                                if(iyi<0)iyi=0;
        //                                if(iyi>N)iyi=N;

        //                                V3DLONG offsetjj = iyi*N;
        //                                for(V3DLONG ixi = ix-1; ixi < ix+2; ixi++)
        //                                {
        //                                    if(ixi<0)ixi=0;
        //                                    if(ixi>M)ixi=M;
        //                                    if(int(data1d[offsetki + offsetjj + ixi])>20)
        //                                    {
        //                                        count++;
        //                                        tmpdata1d = tmpdata1d + int(data1d[offsetki + offsetjj + ixi]);
        //                                    }
        //                                }
        //                            }
        //                        }
        //                        if(count!=0)
        //                        {
        //                            im_cropped[j] = tmpdata1d/count;
        //                        }
        //                    }
        //                    j++;
        //                }
        //            }
        //        }
        //        p4dImage->setData(im_cropped,M,N,P,C,V3D_UINT8);


//        //v.2

//        V3DLONG M = in_sz0[0];
//        V3DLONG N = in_sz0[1];
//        V3DLONG P = in_sz0[2];
//        V3DLONG C = in_sz0[3];
//        int NI,SI,EI,WI,DI,JI;
//        double cN,cS,cE,cW,cD,cJ;

//        cout<<"MNP = "<<M<<"  "<<N<<"  "<<P<<"  "<<endl;

//        unsigned char* data1d = p4dImage->getRawData();
//      //  unsigned char* im_cropped = 0;
//        V3DLONG pagesz;
//        pagesz = in_sz0[0]*in_sz0[1]*in_sz0[2]*in_sz0[3];
//      //  try {im_cropped = new unsigned char [pagesz];}
//      //  catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return false;}
//        //V3DLONG j = 0;
//        V3DLONG NUM=4;
//        int k=5;
//        double lambda = 0.05;
//        for(V3DLONG num = 0;num<NUM;num++)
//        {
//            cout<<"NUM"<<endl;
//            for(V3DLONG iz = 2; iz < P-2; iz++)
//            {
//              //  cout<<"M"<<endl;
//                V3DLONG offsetk = iz*M*N;
//                for(V3DLONG iy = 2; iy < N-2; iy++)
//                {
//                   // cout<<"N"<<endl;
//                    V3DLONG offsetj = iy*N;
//                    for(V3DLONG ix = 2; ix < M-2; ix++)
//                    {
//                        //  cout<<"P"<<endl;
//                        int bri = data1d[offsetk + offsetj + ix];
//                        // cout<<"lllllllllllllllllllllllllllllllllllllllllll offsetk + offsetj + ix = "<<offsetk + offsetj + ix<<endl;
//                        //  cout<<"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%bri = "<<bri<<endl;
//                        // cout<<"55555555555555555555555555555555555555555555555data1d[offsetk + offsetj + ix-1] = "<<int(data1d[offsetk + offsetj + ix-1])<<endl;
//                        if(bri<23&&bri>15)
//                        {
//                            cout<<"<<<23"<<endl;
//                            NI = data1d[offsetk + offsetj + ix-1] - data1d[offsetk + offsetj + ix];
//                            SI = data1d[offsetk + offsetj + ix+1] - data1d[offsetk + offsetj + ix];
//                            // cout<<"oooo"<<endl;
//                            EI = data1d[(iz-1)*M*N + offsetj + ix] - data1d[offsetk + offsetj + ix];
//                            WI = data1d[(iz+1)*M*N + offsetj + ix] - data1d[offsetk + offsetj + ix];
//                            //  cout<<"pppp"<<endl;
//                            DI = data1d[offsetk + (iy-1)*N + ix] - data1d[offsetk + offsetj + ix];
//                            JI = data1d[offsetk + (iy+1)*N + ix] - data1d[offsetk + offsetj + ix];
//                            //  cout<<"U"<<endl;

//                            cN=1/(NI^2/(k*k)+1);
//                            cS=1/(SI^2/(k*k)+1);
//                            cE=1/(EI^2/(k*k)+1);
//                            cW=1/(WI^2/(k*k)+1);
//                            cD=1/(DI^2/(k*k)+1);
//                            cJ=1/(JI^2/(k*k)+1);
//                            //  cout<<"hahahaha"<<endl;
//                            int all = lambda*(cN*NI+cS*SI+cE*EI+cW*WI);
//                            //                        cout<<"NI = "<<NI<<"  "<<int(data1d[offsetk + offsetj + ix-1])<<"  "<<int(data1d[offsetk + offsetj + ix])<<endl;
//                            //                        cout<<"SI = "<<SI<<"  "<<int(data1d[offsetk + offsetj + ix+1])<<"  "<<int(data1d[offsetk + offsetj + ix])<<endl;
//                            //                        cout<<"EI = "<<EI<<endl;
//                            //                        cout<<"WI = "<<WI<<endl;
//                            //                        cout<<"DI = "<<DI<<endl;
//                            //                        cout<<"JI = "<<JI<<endl;

//                            //                        cout<<"cN = "<<cN<<endl;
//                            //                        cout<<"cS = "<<cS<<endl;
//                            //                        cout<<"cE = "<<cE<<endl;
//                            //                        cout<<"cW = "<<cW<<endl;
//                            //                        cout<<"cD = "<<cD<<endl;
//                            //                        cout<<"cJ = "<<cJ<<endl;
//                            //                        cout<<"all = "<<all<<endl;
//                            int tmp = bri + all;
//                            if(tmp<0)
//                                tmp=0;
//                            data1d[offsetk + offsetj + ix] = tmp;
//                        }


//                    }
//                }
//            }
//        }

//v.3
                V3DLONG M = in_sz0[0];
                V3DLONG N = in_sz0[1];
                V3DLONG P = in_sz0[2];
                V3DLONG C = in_sz0[3];
                V3DLONG NUM=1;
                int k=14;
                double lambda = 0.15;
                int NI,SI,EI,WI,DI,JI;
                double cN,cS,cE,cW,cD,cJ;

                unsigned char* data1d = p4dImage->getRawData();
                unsigned char* im_cropped = 0;
                V3DLONG pagesz;
                pagesz = in_sz0[0]*in_sz0[1]*in_sz0[2]*in_sz0[3];
                try {im_cropped = new unsigned char [pagesz];}
                catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return false;}


                vector<vector<vector<V3DLONG> > > coodinate3D;
                vector<vector<V3DLONG> > coodinate2D;
                vector<V3DLONG> coodinate1D;


                    for(V3DLONG iz = 0; iz < P; iz++)
                    {
                        V3DLONG offsetk = iz*M*N;
                        for(V3DLONG iy = 0; iy < N; iy++)
                        {
                            V3DLONG offsetj = iy*N;
                            for(V3DLONG ix = 0; ix < M; ix++)
                            {
                                V3DLONG tmp = data1d[offsetk + offsetj + ix];
                                coodinate1D.push_back(tmp);
                            }
                            coodinate2D.push_back(coodinate1D);
                            coodinate1D.clear();
                        }
                        coodinate3D.push_back(coodinate2D);
                        coodinate2D.clear();
                    }
                    for(V3DLONG num = 0;num < NUM; num++)
                    {
                        cout<<"NUM = "<<num<<endl;
                        for(V3DLONG iz = 1; iz < P-1; iz++)
                        {
                            //V3DLONG offsetk = iz*M*N;
                            for(V3DLONG iy = 1; iy < N-1; iy++)
                            {
                                //V3DLONG offsetj = iy*N;
                                for(V3DLONG ix = 1; ix < M-1; ix++)
                                {
                                    int bri = coodinate3D[iz][iy][ix];
                                    if(bri>10&&bri<20)
                                    {
                                        NI = coodinate3D[iz][iy][ix-1] - coodinate3D[iz][iy][ix];
                                        SI = coodinate3D[iz][iy][ix+1] - coodinate3D[iz][iy][ix];
                                        EI = coodinate3D[iz][iy-1][ix] - coodinate3D[iz][iy][ix];
                                        WI = coodinate3D[iz][iy+1][ix] - coodinate3D[iz][iy][ix];
                                        DI = coodinate3D[iz-1][iy][ix] - coodinate3D[iz][iy][ix];
                                        JI = coodinate3D[iz+1][iy][ix] - coodinate3D[iz][iy][ix];
                                        cN=1/(NI^2/(k*k)+1);
                                        cS=1/(SI^2/(k*k)+1);
                                        cE=1/(EI^2/(k*k)+1);
                                        cW=1/(WI^2/(k*k)+1);
                                        cD=1/(DI^2/(k*k)+1);
                                        cJ=1/(JI^2/(k*k)+1);
                                        int all = lambda*(cN*NI+cS*SI+cE*EI+cW*WI);
                                        //                                    cout<<"EI = "<<EI<<endl;
                                        //                                    cout<<"WI = "<<WI<<endl;
                                        //                                    cout<<"DI = "<<DI<<endl;
                                        //                                    cout<<"JI = "<<JI<<endl;

                                        //                                    cout<<"cN = "<<cN<<endl;
                                        //                                    cout<<"cS = "<<cS<<endl;
                                        //                                    cout<<"cE = "<<cE<<endl;
                                        //                                    cout<<"cW = "<<cW<<endl;
                                        //                                    cout<<"cD = "<<cD<<endl;
                                        //                                    cout<<"cJ = "<<cJ<<endl;
                                        //                                    cout<<"all = "<<all<<endl;
                                        int tmp = bri + all;
                                        if(tmp<0)
                                            tmp=0;
                                        coodinate3D[iz][iy][ix] = tmp;
                                    }
                                }

                            }

                        }
                    }
                    //if(data1d) {delete []data1d; data1d = 0;}
                    cout<<"hahhahaa"<<endl;
                    //int u=0;
                    for(V3DLONG iz = 0; iz < P; iz++)
                    {
                        V3DLONG offsetk = iz*M*N;
                        for(V3DLONG iy = 0; iy < N; iy++)
                        {
                            V3DLONG offsetj = iy*N;
                            for(V3DLONG ix = 0; ix < M; ix++)
                            {
                                data1d[offsetk + offsetj + ix] = coodinate3D[iz][iy][ix];
                                //u++;
                            }
                        }
                    }
                    simple_saveimage_wrapper(callback,QString("tmpimg.v3draw").toStdString().c_str(),data1d,in_sz0,1);









    /**********************************change here by LXF************************************************/
        
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
