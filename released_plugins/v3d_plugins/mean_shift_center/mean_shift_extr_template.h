#ifndef MEAN_SHIFT_EXTR_TEMPLATE_H
#define MEAN_SHIFT_EXTR_TEMPLATE_H

#include <v3d_interface.h>

template <class T>
void findMaxMinVal(T *data1D, V3DLONG len, V3DLONG & max_ind, T & max_val,  V3DLONG & min_ind, T & min_val)
{
    max_val=data1D[0];
    min_val=data1D[0];
    max_ind=0;
    min_ind=0;
    for(V3DLONG vid=0; vid<len; vid++){
        if(data1D[vid]<min_val){
            min_val=data1D[vid];
            min_ind=vid;
        }
        if(data1D[vid]>max_val){
            max_val=data1D[vid];
            max_ind=vid;
        }
    }

}

template <class T>
void normalizeEachChannelTo255(T * data1Dc, V3DLONG sz_img[4])
{
    V3DLONG page_size = sz_img[0]*sz_img[1]*sz_img[2];
    for(V3DLONG cid=0; cid<sz_img[3]; cid++){
        V3DLONG page = page_size*cid;
        T max_val, min_val;
        V3DLONG max_ind, min_ind;
        findMaxMinVal<T>(data1Dc+page, page_size, max_ind, max_val, min_ind, min_val);

        float scale=(float)(max_val-min_val);
        if(scale>1e-16){
            scale/=255;
            for(V3DLONG vid=page; vid<page+page_size; vid++){
                data1Dc[vid]-=min_val;
                data1Dc[vid]/=scale;
            }
        }else{
            memset(data1Dc+page,0,page_size*sizeof(T));
        }
    }
}
#endif // MEAN_SHIFT_EXTR_TEMPLATE_H
