#ifndef LEARNING_H
#define LEARNING_H

#include <QtGui>
#include "v3d_interface.h"
#include <map>
#include <algorithm>
#include <iostream>
#include <set>
#include "v3d_basicdatatype.h"
#include <vector>
#include "stackutil.h"

using namespace std;

class learning
{
public:
    learning(V3DPluginCallback2 *cb,const char * inname_img,const char *marker_inname,
             const char* inname_out);
    void wavelet_start();
    int loadData();
    bool loadmarker();

private:
    int getWavelet(float *** ppp_inImage, float **** pppp_outWave, int dim);
    bool getWindowWavelet(V3DLONG x, V3DLONG y, V3DLONG z, vector<float>& wave);
    //ppp_inImage is dim*dim*dim size 3D image, dim%2==0, ppp_inImage[z][y][x]
    //pppp_outWave is (dim/2)*(dim/2)*(dim/2)*8 4D image, pppp_outWave[z][y][x][w]
    //w from 0~7 are: LLL, HLL, LHL, LLH, LHH, HLH, HHL, HHH

private:
    V3DPluginCallback * callback;
    unsigned char * p_img1D;
    unsigned char *** ppp_img3D; //ppp_img3d[z][y][x]
    V3DLONG img_sz[4];
    QString fname_img,fname_marker;
    QString fname_out;
    vector<vector<int> > featureInfo;
    vector<float ****> tmp_pppp_outWave;
    float *** tmp_ppp_window;
    int winSize;
    QList<ImageMarker> marker_list;

public:
    bool pushImageData(unsigned char * data1Dc_in, V3DLONG new_sz_img[4])
    {
        //qDebug()<<"in push imageData";
        if(ppp_img3D!=0){
            delete [] ppp_img3D; ppp_img3D=0;
        }
        img_sz[0]=new_sz_img[0];
        img_sz[1]=new_sz_img[1];
        img_sz[2]=new_sz_img[2];

        //we know which channel to select
        //need to check this channel whether there are voxels

        int max=0;
        V3DLONG page_size=img_sz[0]*img_sz[1]*img_sz[2];
        p_img1D = new unsigned char [page_size];
        for(V3DLONG i=0; i<page_size; i++){
            p_img1D[i] = data1Dc_in[i];
            if (p_img1D[i]>max)
                max=p_img1D[i];
        }
        if (max==0)
        {
            v3d_msg("Please check the provided channel. No voxels found");
            delete[] p_img1D;
            p_img1D=0;
            return false;
        }
        //arrange the image into 3D
        ppp_img3D=new unsigned char ** [img_sz[2]];
        for(V3DLONG z=0; z<img_sz[2]; z++){
            ppp_img3D[z]=new unsigned char * [img_sz[1]];
            for(V3DLONG y=0; y<img_sz[1]; y++){
                ppp_img3D[z][y]=p_img1D+y*img_sz[0]+z*img_sz[0]*img_sz[1];
            }
        }
        qDebug()<<"in pushnewdata:"<<img_sz[0]<<":"<<img_sz[1]<<":"<<img_sz[2];
        return true;
    }
};


float *** new_3D_memory(int dim);
void delete_3D_memory(float ***, int dim);
#endif // LEARNING_H
