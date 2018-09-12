#ifndef MULTI_CHANNEL_SWC_TEMPLATE_H
#define MULTI_CHANNEL_SWC_TEMPLATE_H

#ifndef MAX_LONG_LONG
#define MAX_LONG_LONG 9223372036854775807
#endif

#ifndef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif

#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif

#include <v3d_interface.h>
#include <vector>

using namespace std;

template<class T>
void calculate_info(T* p_img, V3DLONG sz_img[4], NeuronTree* nt,
int ch1, float thr1, int ch2, float thr2, vector<float>& N, vector<float>& ratio, vector<float>& mean, vector<float>& stdev, vector<float>& N_P, vector<float>& ratio_P, vector<float>& mean_P, vector<float>& stdev_P)
{
    //initial outputs
    N.clear();
    N.resize(nt->listNeuron.size(),0);
    ratio.clear();
    ratio.resize(nt->listNeuron.size(),0);
    mean.clear();
    mean.resize(nt->listNeuron.size(),0);
    stdev.clear();
    stdev.resize(nt->listNeuron.size(),0);
    N_P.clear();
    N_P.resize(nt->listNeuron.size(),0);
    ratio_P.clear();
    ratio_P.resize(nt->listNeuron.size(),0);
    mean_P.clear();
    mean_P.resize(nt->listNeuron.size(),0);
    stdev_P.clear();
    stdev_P.resize(nt->listNeuron.size(),0);


    //calculate for each point
    for(V3DLONG nid=0; nid<nt->listNeuron.size(); nid++){
        if(nt->listNeuron.at(nid).parent>=0){//if the node has parent
            //get the parent node id
            V3DLONG pid=nt->hashNeuron.value(nt->listNeuron.at(nid).pn);

            V3DLONG win_1[3];
            V3DLONG win_2[3];

            //find the window for calculate
            win_1[0]=MIN(nt->listNeuron.at(pid).x-nt->listNeuron.at(pid).radius,nt->listNeuron.at(nid).x-nt->listNeuron.at(nid).radius);
            win_1[1]=MIN(nt->listNeuron.at(pid).y-nt->listNeuron.at(pid).radius,nt->listNeuron.at(nid).y-nt->listNeuron.at(nid).radius);
            win_1[2]=MIN(nt->listNeuron.at(pid).z-nt->listNeuron.at(pid).radius,nt->listNeuron.at(nid).z-nt->listNeuron.at(nid).radius);
            win_2[0]=MAX(nt->listNeuron.at(pid).x+nt->listNeuron.at(pid).radius,nt->listNeuron.at(nid).x+nt->listNeuron.at(nid).radius);
            win_2[1]=MAX(nt->listNeuron.at(pid).y+nt->listNeuron.at(pid).radius,nt->listNeuron.at(nid).y+nt->listNeuron.at(nid).radius);
            win_2[2]=MAX(nt->listNeuron.at(pid).z+nt->listNeuron.at(pid).radius,nt->listNeuron.at(nid).z+nt->listNeuron.at(nid).radius);

            //prevent going outside the image
            win_1[0]=MAX(0,win_1[0]);
            win_1[1]=MAX(0,win_1[1]);
            win_1[2]=MAX(0,win_1[2]);
            win_2[0]=MIN(sz_img[0],win_2[0]);
            win_2[1]=MIN(sz_img[1],win_2[1]);
            win_2[2]=MIN(sz_img[2],win_2[2]);

            float num_vol_ch1=0;
            float num_vol_ch2=0;
            float intensity_ch1=0;
            float intensity_ch2=0;
            //do the calculation
            for(V3DLONG x=win_1[0]; x<win_2[0]; x++)
                for(V3DLONG y=win_1[1]; y<win_2[1]; y++)
                    for(V3DLONG z=win_1[2]; z<win_2[2]; z++){
                        V3DLONG pos1=x+y*sz_img[0]+z*sz_img[1]*sz_img[0]+ch1*sz_img[1]*sz_img[0]*sz_img[2];
                        V3DLONG pos2=x+y*sz_img[0]+z*sz_img[1]*sz_img[0]+ch2*sz_img[1]*sz_img[0]*sz_img[2];
                        if(p_img[pos1]>thr1){
                            num_vol_ch1++;
                            if(p_img[pos2]>thr2){
                                num_vol_ch2++;
                                intensity_ch2+=p_img[pos2];
                                intensity_ch1+=p_img[pos1];
                            }
                        }
                    }
            float std_ch2=0;
            float std_ch1=0;
            if(num_vol_ch2>0.1){
                intensity_ch2/=num_vol_ch2;
               //std
                for(V3DLONG x=win_1[0]; x<win_2[0]; x++)
                    for(V3DLONG y=win_1[1]; y<win_2[1]; y++)
                        for(V3DLONG z=win_1[2]; z<win_2[2]; z++){
                            V3DLONG pos1=x+y*sz_img[0]+z*sz_img[1]*sz_img[0]+ch1*sz_img[1]*sz_img[0]*sz_img[2];
                            V3DLONG pos2=x+y*sz_img[0]+z*sz_img[1]*sz_img[0]+ch2*sz_img[1]*sz_img[0]*sz_img[2];
                            if(p_img[pos1]>thr1){
                                if(p_img[pos2]>thr2){
                                    float tmp=p_img[pos2]-intensity_ch2;
                                    std_ch2+=tmp*tmp;

                                }
                            }
                        }
                std_ch2=sqrt(std_ch2/num_vol_ch2);

            }
            if(num_vol_ch1>0.1){
                intensity_ch1/=num_vol_ch1;
               //std
                for(V3DLONG x=win_1[0]; x<win_2[0]; x++)
                    for(V3DLONG y=win_1[1]; y<win_2[1]; y++)
                        for(V3DLONG z=win_1[2]; z<win_2[2]; z++){
                            V3DLONG pos1=x+y*sz_img[0]+z*sz_img[1]*sz_img[0]+ch1*sz_img[1]*sz_img[0]*sz_img[2];
                            V3DLONG pos2=x+y*sz_img[0]+z*sz_img[1]*sz_img[0]+ch2*sz_img[1]*sz_img[0]*sz_img[2];
                            if(p_img[pos1]>thr1){
                                    float tmp1=p_img[pos1]-intensity_ch1;
                                    std_ch1+=tmp1*tmp1;
                                }
                            }
                std_ch1=sqrt(std_ch1/num_vol_ch1);

            }

            if(num_vol_ch1>0.1){
                ratio[nid]=num_vol_ch2/num_vol_ch1;
                ratio_P[nid]=num_vol_ch1/num_vol_ch1;
            }
            mean[nid]=intensity_ch2;
            stdev[nid]=std_ch2;
            N[nid]= num_vol_ch2;
            N_P[nid]=num_vol_ch1;
            mean_P[nid]=intensity_ch1;
            stdev_P[nid]=std_ch1;

        }
    }
    }

#endif // MULTI_CHANNEL_SWC_TEMPLATE_H
