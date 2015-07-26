#include "learning.h"

learning::learning(V3DPluginCallback2 *cb, const char * inname_img, const char *marker_inname, const char* inname_out)
{
    callback=cb;
    fname_img = inname_img;
    fname_out = inname_out;
    fname_marker = marker_inname;
    p_img1D = 0;
    ppp_img3D = 0;
    tmp_ppp_window=0;
    //get label_groups
    //get image

}

int learning::loadData()
{
    qDebug()<<"~~~~~learning: Loading datas";
    int type_img=0;
    if(!simple_loadimage_wrapper(*callback, fname_img.toStdString().c_str(), p_img1D, img_sz, type_img)){
        qDebug()<<"load wrong";
        return 11;
    }
    if(type_img!=1){
        return 12;
    }
    if(img_sz[3]>1){
        //qDebug()<<"WARNING: plugin can only take 1 channel, the first channel will be used.";
        img_sz[3]=1;
    }

    //arrange the image into 3D
    ppp_img3D=new unsigned char ** [img_sz[2]];
    for(V3DLONG z=0; z<img_sz[2]; z++){
        ppp_img3D[z]=new unsigned char * [img_sz[1]];
        for(V3DLONG y=0; y<img_sz[1]; y++){
            ppp_img3D[z][y]=p_img1D+y*img_sz[0]+z*img_sz[0]*img_sz[1];
        }
    }

    return 0;
}

bool learning::loadmarker()
{
    marker_list=readMarker_file(fname_marker);
    return true;
}

void learning::wavelet_start()
{
    qDebug()<<"wavelet starting here...."<<img_sz[0]<<":"<<img_sz[1];
    for (int i=0;i<marker_list.size();i++)
    {
        V3DLONG x,y,z;
        vector<float> tmp_wave;
        x=(V3DLONG)marker_list.at(i).x+0.5;
        y=(V3DLONG)marker_list.at(i).y+0.5;
        z=(V3DLONG)marker_list.at(i).z+0.5;
        qDebug()<<"i:"<<"xyz:"<<x<<":"<<y<<":"<<z;
        getWindowWavelet(x,y,z,tmp_wave);
    }
}

bool learning::getWindowWavelet(V3DLONG x, V3DLONG y, V3DLONG z, vector<float> & wave)
{
    bool b_firstRun = false;
    if(x<0||x>=img_sz[0]||y<0||y>=img_sz[1]||z<0||z>=img_sz[2]){
        qDebug()<<"warning: targeting point "<<x<<":"<<y<<":"<<z<<":"<<" is out of image boundary.";
//        if(x<0) x=0;
//        if(x>=sz_img[0]) x=sz_img[0]-1;
//        if(y<0) y=0;
//        if(y>=sz_img[1]) y=sz_img[1]-1;
//        if(z<0) z=0;
//        if(z>=sz_img[2]) z=sz_img[2]-1;
    }
    int level=3; //param.wavelet_level;


    //init reusable storage memory if it is the first time to run
    if(tmp_pppp_outWave.size()!=level || tmp_ppp_window==0){
        qDebug()<<"first time initiation";
        b_firstRun = true;
        featureInfo.clear();
        winSize=1;
        tmp_pppp_outWave.resize(level);
        for(int i=level-1; i>=0; i--){
            winSize*=2;
            tmp_pppp_outWave[i]=new float *** [8];
            for(int j=0; j<8; j++){
                tmp_pppp_outWave[i][j]=new_3D_memory(winSize);
            }
        }
        tmp_ppp_window=new_3D_memory(winSize*2);
    }
    qDebug()<<"initiating image"<<winSize;
    //init image
    for(int dx=-winSize; dx<winSize; dx++){
        V3DLONG cx=x+dx; //mirror for out bound points
        while(cx<0 || cx>=img_sz[0]){
            if(cx<0) cx=-(cx+1);
            if(cx>=img_sz[0]) cx=img_sz[0]*2-cx-1;
        }
        for(int dy=-winSize; dy<winSize; dy++){
            V3DLONG cy=y+dy; //mirror for out bound points
            while(cy<0 || cy>=img_sz[1]){
                if(cy<0) cy=-(cy+1);
                if(cy>=img_sz[1]) cy=img_sz[1]*2-cy-1;
            }
            for(int dz=-winSize; dz<winSize; dz++){
                V3DLONG cz=z+dz; //mirror for out bound points
                while(cz<0 || cz>=img_sz[2]){
                    if(cz<0) cz=-(cz+1);
                    if(cz>=img_sz[2]) cz=img_sz[2]*2-cz-1;
                }
//                qDebug()<<"before tmp_ppp_window"<<dz+winSize<<":"<<dy+winSize<<":"<<dx+winSize
//                    <<"cx,cy,cz:"<<cx<<":"<<cy<<":"<<cz;
                tmp_ppp_window[dz+winSize][dy+winSize][dx+winSize]=ppp_img3D[cz][cy][cx];
            }
        }
    }
    qDebug()<<"computing wavelet";
    //compute wavelet
    int feaNumber=0;
    int tmp=getWavelet(tmp_ppp_window,tmp_pppp_outWave[0],winSize*2);
    if(tmp==0){
        qDebug()<<"error: failed to compute wavelet, level:0";
        return 0;
    }else{
        feaNumber+=tmp;
    }
    for(int i=1; i<level; i++){
        tmp=getWavelet(tmp_pppp_outWave[i-1][0],tmp_pppp_outWave[i],winSize/i);
        if(tmp==0){
            qDebug()<<"error: failed to compute wavelet, level:"<<i;
            return 0;
        }else{
            feaNumber+=tmp;
        }
    }

    //push to output
    feaNumber=feaNumber/8*7+8;
    wave.resize(feaNumber);
    if(b_firstRun){
        featureInfo.resize(feaNumber);
    }
    long feaInd=0;
    for(int z=0; z<2; z++){
        for(int y=0; y<2; y++){
            for(int x=0; x<2; x++){
                wave[feaInd]=tmp_pppp_outWave.at(level-1)[0][z][y][x];
                if(b_firstRun){ //record the wavelet info of feature for future analysis
                    featureInfo[feaInd].resize(5);
                    featureInfo[feaInd][0]=level;
                    featureInfo[feaInd][1]=0;
                    featureInfo[feaInd][2]=x;
                    featureInfo[feaInd][3]=y;
                    featureInfo[feaInd][4]=z;
                }

                feaInd++;
            }
        }
    }
    int cur_winSize=1;
    for(int i=level-1; i>=0; i--){
        cur_winSize*=2;
        for(int w=1; w<8; w++){
            for(int z=0; z<cur_winSize; z++){
                for(int y=0; y<cur_winSize; y++){
                    for(int x=0; x<cur_winSize; x++){
                        wave[feaInd]=tmp_pppp_outWave.at(i)[w][z][y][x];
                        if(b_firstRun){ //record the wavelet info of feature for future analysis
                            featureInfo[feaInd].resize(5);
                            featureInfo[feaInd][0]=i+1;
                            featureInfo[feaInd][1]=w;
                            featureInfo[feaInd][2]=x;
                            featureInfo[feaInd][3]=y;
                            featureInfo[feaInd][4]=z;
                        }

                        feaInd++;
                    }
                }
            }
        }
    }

    //for test
//    qDebug()<<"saving image";
//    cur_winSize=1;
//    V3DLONG tmp_sz[4]={1,1,1,1};
//    char tmp_filename[1000];
//    for(int i=level-1; i>=0; i--){
//        cur_winSize*=2;
//        tmp_sz[0]=tmp_sz[1]=tmp_sz[2]=cur_winSize;
//        for(int w=0; w<8; w++){
//            sprintf(tmp_filename,"test_wave_l%d_w%d.raw",i,w);
//            saveImage(tmp_filename, (unsigned char *) tmp_pppp_outWave[i][w][0][0], tmp_sz, 4);
//        }
//    }

    return true;
}

int learning::getWavelet(float *** ppp_inImage, float **** pppp_outWave, int dim)
{
    qDebug()<<"in getting wavelet";
    float L[2];
    float H[2];
    float LL[2];
    float LH[2];
    float HL[2];
    float HH[2];
    for(int z=0; z<dim/2; z++){
        for(int y=0; y<dim/2; y++){
            for(int x=0; x<dim/2; x++){
                for(int dz=0; dz<2; dz++){
                    for(int dy=0; dy<2; dy++){
                        L[dy]=(ppp_inImage[z*2+dz][y*2+dy][x*2]+ppp_inImage[z*2+dz][y*2+dy][x*2+1])/2;
                        H[dy]=(ppp_inImage[z*2+dz][y*2+dy][x*2]-ppp_inImage[z*2+dz][y*2+dy][x*2+1]);
                    }
                    LL[dz]=(L[0]+L[1])/2;
                    LH[dz]=L[0]-L[1];
                    HL[dz]=(H[0]+H[1])/2;
                    HH[dz]=H[0]-H[1];
                }
                pppp_outWave[0][z][y][x]=(LL[0]+LL[1])/2;
                pppp_outWave[1][z][y][x]=(HL[0]+HL[1])/2;
                pppp_outWave[2][z][y][x]=(LH[0]+LH[1])/2;
                pppp_outWave[3][z][y][x]=(LL[0]-LL[1]);
                pppp_outWave[4][z][y][x]=(LH[0]-LH[1]);
                pppp_outWave[5][z][y][x]=(HL[0]-HL[1]);
                pppp_outWave[6][z][y][x]=(HH[0]+HH[1])/2;
                pppp_outWave[7][z][y][x]=(HH[0]-HH[1]);
            }
        }
    }

    return dim*dim*dim;
}

float *** new_3D_memory(int dim){
    float * data_1p=new float [dim*dim*dim];
    float *** data = new float ** [dim];
    for(int i=0; i<dim; i++){
        data[i]=new float * [dim];
        for(int j=0; j<dim; j++){
            data[i][j]=data_1p+i*dim*dim+j*dim;
        }
    }
    return data;
}

void delete_3D_memory(float ***& data, int dim){
    if(data==0){
        return;
    }
    float * data_1p=data[0][0];
    for(int i=0; i<dim; i++){
        if(data[i]){
            delete[] (data[i]);
            data[i]=0;
        }
    }
    delete [] data_1p;
    data_1p=0;
    delete [] data;
    data=0;

    return;
}
