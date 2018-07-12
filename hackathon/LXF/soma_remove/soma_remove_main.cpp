#include "soma_remove_main.h"

#include <iostream>
 using namespace std;


bool soma_remove_main(unsigned char* data1d,V3DLONG in_sz[4],V3DPluginCallback2 &callback)
{
    V3DLONG M = in_sz[0];
    V3DLONG N = in_sz[1];
    V3DLONG P = in_sz[2];
    V3DLONG C = in_sz[3];
    int thresh = 30;                            //need to be change
    double signal_thresh = 4;                        //need to be change
    double r = 5;                                  //need to be change
    V3DLONG im_cropped_sz[4];
    unsigned char *im_cropped = 0;


    double d = 2*r;
    int y=0;
    im_cropped_sz[0] = in_sz[0];
    im_cropped_sz[1] = in_sz[1];
    im_cropped_sz[2] = in_sz[2];
    im_cropped_sz[3] = in_sz[3];
    V3DLONG pagesz;
    //cout<<im_cropped_sz[0]<<"   "<<im_cropped_sz[1]<<"    "<<im_cropped_sz[2]<<"    "<<im_cropped_sz[3]<<endl;
    pagesz = im_cropped_sz[0]* im_cropped_sz[1]* im_cropped_sz[2]*im_cropped_sz[3];
     cout<<"pagesz = "<<pagesz<<endl;
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
                 if(tmp==0)y++;
                 coodinate1D.push_back(tmp);
             }
             coodinate2D.push_back(coodinate1D);
             coodinate1D.clear();
         }
         coodinate3D.push_back(coodinate2D);
         coodinate2D.clear();
     }
     for(V3DLONG iz = 1; iz < P-1; iz++)
     {
         for(V3DLONG iy = 1; iy < N-1; iy++)
         {
             for(V3DLONG ix = 1; ix < M-1; ix++)
             {
                 int bri = coodinate3D[iz][iy][ix];

                 //cout<<"bri = "<<bri<<endl;
                 if(bri>thresh)
                 {

                     //NI = coodinate3D[iz][iy][ix-1]
                     int SI = coodinate3D[iz][iy][ix+1];
                     int EI = coodinate3D[iz][iy-1][ix];
                     int WI = coodinate3D[iz][iy+1][ix];
                     //DI = coodinate3D[iz-1][iy][ix]
                     //JI = coodinate3D[iz+1][iy][ix]
                     if((SI - bri)*(SI - bri)< signal_thresh)
                     {
                         double count_1=0;
                         for(int ixi=0;ixi<r;ixi++)
                         {
                             for(int iyi=0;iyi<r;iyi++)
                             {
                                 if(ix+ixi>M-1)continue;
                                 if(iy+iyi>N-1)continue;
                                 if(coodinate3D[iz][iy+iyi][ix+ixi]>thresh)
                                 {
                                     count_1++;
                                 }
                             }
                         }

                         double per_1 = (r*r-count_1)/(r*r);
                         //cout<<"per_1 = "<<per_1<<endl;
                         if(per_1>0.7)
                         {
                             for(int k=0;k<d;k++)
                             {
                                 for(int l=0;l<d;l++)
                                 {
                                     cout<<"33333333333333"<<endl;
                                     if(ix+l>M-1)continue;
                                     if(iy+k>N-1)continue;
                                     coodinate3D[iz][iy+k][ix+l]=0;
                                 }
                             }
                         }


                     }
                     if((WI - bri)*(WI - bri)< signal_thresh)
                     {
                         double count_2=0;
                         for(int ixi=0;ixi<r;ixi++)
                         {
                             for(int iyi=0;iyi<r;iyi++)
                             {
                                 if(ix-r/2+ixi>M-1)continue;
                                 if(iy+iyi>N-1)continue;
                                 if(coodinate3D[iz][iy+iyi][ix-r/2+ixi]>thresh)
                                 {
                                     count_2++;
                                 }
                             }
                         }
                         double per_2 = (r*r-count_2)/(r*r);
                         //cout<<"per_2 = "<<per_2<<endl;
                         if(per_2>0.7)
                         {
                             for(int k=0;k<d;k++)
                             {
                                 for(int l=0;l<d;l++)
                                 {
                                     if(ix-r/2+l>M-1)continue;
                                     if(iy+k>N-1)continue;
                                     cout<<"22222222222222"<<endl;
                                     coodinate3D[iz][iy+k][ix-r/2+l]=0;
                                 }
                             }
                         }
                     }
                     //                         if((EI - bri)*(EI - bri)< signal_thresh)
                     //                         {
                     //                             double count_3=0;
                     //                             for(int ixi=0;ixi<r;ixi++)
                     //                             {
                     //                                 for(int iyi=0;iyi<r;iyi++)
                     //                                 {
                     //                                     if(coodinate3D[iz][iy+iyi][ix-r/2+ixi]>thresh)
                     //                                     {
                     //                                         count_3++;
                     //                                     }
                     //                                 }
                     //                             }
                     //                             double per_3 = (r*r-count_2)/(r*r);
                     //                             if(per_3>0.7)
                     //                             {

                     //                             }
                     //                         }

                 }
             }
         }
     }
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
                 im_cropped[offsetk + offsetj + ix] = coodinate3D[iz][iy][ix];
                 //u++;
             }
         }
     }
     int u=0;
     for(V3DLONG iz = 0; iz < P; iz++)
     {
         for(V3DLONG iy = 0; iy < N; iy++)
         {
             for(V3DLONG ix = 0; ix < M; ix++)
             {
                 int o = coodinate3D[iz][iy][ix];
                 if(0==0)
                 {
                     u++;
                 }
             }
         }
     }
     cout<<u<<"  "<<y<<endl;
     v3d_msg("check");
         simple_saveimage_wrapper(callback,QString("tmpimg.v3draw").toStdString().c_str(),im_cropped,in_sz,1);




}
