#include "learning.h"

learning::learning()
{
    //get center
    //get label_groups

}

//bool learning::getWindowWavelet(V3DLONG x, V3DLONG y, V3DLONG z, vector<float>& wave)
//{
//    bool b_firstRun = false;
//    if(x<0||x>=sz_img[0]||y<0||y>=sz_img[1]||z<0||z>=sz_img[2]){
//        qDebug()<<"warning: targeting point "<<x<<":"<<y<<":"<<z<<":"<<" is out of image boundary.";
////        if(x<0) x=0;
////        if(x>=sz_img[0]) x=sz_img[0]-1;
////        if(y<0) y=0;
////        if(y>=sz_img[1]) y=sz_img[1]-1;
////        if(z<0) z=0;
////        if(z>=sz_img[2]) z=sz_img[2]-1;
//    }
//    int level=param.wavelet_level;

//    //init reusable storage memory if it is the first time to run
//    if(tmp_pppp_outWave.size()!=level || tmp_ppp_window==0){
//        b_firstRun = true;
//        featureInfo.clear();
//        winSize=1;
//        tmp_pppp_outWave.resize(level);
//        for(int i=level-1; i>=0; i--){
//            winSize*=2;
//            tmp_pppp_outWave[i]=new float *** [8];
//            for(int j=0; j<8; j++){
//                tmp_pppp_outWave[i][j]=new_3D_memory(winSize);
//            }
//        }
//        tmp_ppp_window=new_3D_memory(winSize*2);
//    }
//    //init image
//    for(int dx=-winSize; dx<winSize; dx++){
//        V3DLONG cx=x+dx; //mirror for out bound points
//        while(cx<0 || cx>=sz_img[0]){
//            if(cx<0) cx=-(cx+1);
//            if(cx>=sz_img[0]) cx=sz_img[0]*2-cx-1;
//        }
//        for(int dy=-winSize; dy<winSize; dy++){
//            V3DLONG cy=y+dy; //mirror for out bound points
//            while(cy<0 || cy>=sz_img[1]){
//                if(cy<0) cy=-(cy+1);
//                if(cy>=sz_img[1]) cy=sz_img[1]*2-cy-1;
//            }
//            for(int dz=-winSize; dz<winSize; dz++){
//                V3DLONG cz=z+dz; //mirror for out bound points
//                while(cz<0 || cz>=sz_img[2]){
//                    if(cz<0) cz=-(cz+1);
//                    if(cz>=sz_img[2]) cz=sz_img[2]*2-cz-1;
//                }
//                tmp_ppp_window[dz+winSize][dy+winSize][dx+winSize]=ppp_img3D[cz][cy][cx];
//            }
//        }
//    }

//    //compute wavelet
//    int feaNumber=0;
//    int tmp=getWavelet(tmp_ppp_window,tmp_pppp_outWave[0],winSize*2);
//    if(tmp==0){
//        qDebug()<<"error: failed to compute wavelet, level:0";
//        return 0;
//    }else{
//        feaNumber+=tmp;
//    }
//    for(int i=1; i<level; i++){
//        tmp=getWavelet(tmp_pppp_outWave[i-1][0],tmp_pppp_outWave[i],winSize/i);
//        if(tmp==0){
//            qDebug()<<"error: failed to compute wavelet, level:"<<i;
//            return 0;
//        }else{
//            feaNumber+=tmp;
//        }
//    }

//    //push to output
//    feaNumber=feaNumber/8*7+8;
//    wave.resize(feaNumber);
//    if(b_firstRun){
//        featureInfo.resize(feaNumber);
//    }
//    long feaInd=0;
//    for(int z=0; z<2; z++){
//        for(int y=0; y<2; y++){
//            for(int x=0; x<2; x++){
//                wave[feaInd]=tmp_pppp_outWave.at(level-1)[0][z][y][x];
//                if(b_firstRun){ //record the wavelet info of feature for future analysis
//                    featureInfo[feaInd].resize(5);
//                    featureInfo[feaInd][0]=level;
//                    featureInfo[feaInd][1]=0;
//                    featureInfo[feaInd][2]=x;
//                    featureInfo[feaInd][3]=y;
//                    featureInfo[feaInd][4]=z;
//                }

//                feaInd++;
//            }
//        }
//    }
//    int cur_winSize=1;
//    for(int i=level-1; i>=0; i--){
//        cur_winSize*=2;
//        for(int w=1; w<8; w++){
//            for(int z=0; z<cur_winSize; z++){
//                for(int y=0; y<cur_winSize; y++){
//                    for(int x=0; x<cur_winSize; x++){
//                        wave[feaInd]=tmp_pppp_outWave.at(i)[w][z][y][x];
//                        if(b_firstRun){ //record the wavelet info of feature for future analysis
//                            featureInfo[feaInd].resize(5);
//                            featureInfo[feaInd][0]=i+1;
//                            featureInfo[feaInd][1]=w;
//                            featureInfo[feaInd][2]=x;
//                            featureInfo[feaInd][3]=y;
//                            featureInfo[feaInd][4]=z;
//                        }

//                        feaInd++;
//                    }
//                }
//            }
//        }
//    }

////    //for test
////    cur_winSize=1;
////    V3DLONG tmp_sz[4]={1,1,1,1};
////    char tmp_filename[1000];
////    for(int i=level-1; i>=0; i--){
////        cur_winSize*=2;
////        tmp_sz[0]=tmp_sz[1]=tmp_sz[2]=cur_winSize;
////        for(int w=0; w<8; w++){
////            sprintf(tmp_filename,"test_wave_l%d_w%d.raw",i,w);
////            saveImage(tmp_filename, (unsigned char *) tmp_pppp_outWave[i][w][0][0], tmp_sz, 4);
////        }
////    }

//    return true;
//}

int learning::getWavelet(float *** ppp_inImage, float **** pppp_outWave, int dim)
{
//    float L[2];
//    float H[2];
//    float LL[2];
//    float LH[2];
//    float HL[2];
//    float HH[2];
//    for(int z=0; z<dim/2; z++){
//        for(int y=0; y<dim/2; y++){
//            for(int x=0; x<dim/2; x++){
//                for(int dz=0; dz<2; dz++){
//                    for(int dy=0; dy<2; dy++){
//                        L[dy]=(ppp_inImage[z*2+dz][y*2+dy][x*2]+ppp_inImage[z*2+dz][y*2+dy][x*2+1])/2;
//                        H[dy]=(ppp_inImage[z*2+dz][y*2+dy][x*2]-ppp_inImage[z*2+dz][y*2+dy][x*2+1]);
//                    }
//                    LL[dz]=(L[0]+L[1])/2;
//                    LH[dz]=L[0]-L[1];
//                    HL[dz]=(H[0]+H[1])/2;
//                    HH[dz]=H[0]-H[1];
//                }
//                pppp_outWave[0][z][y][x]=(LL[0]+LL[1])/2;
//                pppp_outWave[1][z][y][x]=(HL[0]+HL[1])/2;
//                pppp_outWave[2][z][y][x]=(LH[0]+LH[1])/2;
//                pppp_outWave[3][z][y][x]=(LL[0]-LL[1]);
//                pppp_outWave[4][z][y][x]=(LH[0]-LH[1]);
//                pppp_outWave[5][z][y][x]=(HL[0]-HL[1]);
//                pppp_outWave[6][z][y][x]=(HH[0]+HH[1])/2;
//                pppp_outWave[7][z][y][x]=(HH[0]-HH[1]);
//            }
//        }
//    }

//    return dim*dim*dim;
    return 1;
}
