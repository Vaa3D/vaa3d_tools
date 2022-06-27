#ifndef CONNECTIVITY_FUNC_H
#define CONNECTIVITY_FUNC_H

#include "v3d_interface.h"
#include "kmeans.h"
#include "volimg_proc.h"
#include <basic_memory.cpp>
#include "v3dneuron_gd_tracing.h"
#include "vn_imgpreprocess.h"
#include "basic_surf_objs.h"

#include <map>
#include <set>



//#define GI(ind) givals[(int)((inimg1d[ind] - min_int)/max_int*255)]


//re-sampling data volume with different scaling factors of different axes
template <class T> bool resample3dimg(T * & img, V3DLONG * sz, double dfactor_x, double dfactor_y, double dfactor_z, int interp_method)
{
    if (!img || !sz)
    {
        fprintf(stderr, "The input to resample3dimg_interp() are invalid [%s][%d].\n", __FILE__, __LINE__);
        return false;
    }

//    if (dfactor_x<1 || dfactor_y<1 || dfactor_z<1)
//    {
//        fprintf(stderr, "The resampling factor must be >1 in resample3dimg_linear_interp(), because now only DOWN-sampling is supported [%s][%d].\n", __FILE__, __LINE__);
//        return false;
//    }

    if (sz[0]<1 || sz[1]<1 || sz[2]<1 || sz[3]<1)
    {
        fprintf(stderr, "Input image size is not valid in resample3dimg_interp() [%s][%d].\n", __FILE__, __LINE__);
        return false;
    }

//    if (interp_method!=1) //0 for nearest neighbor interp and 1 for linear
//    {
//        fprintf(stderr,"Invalid interpolation code. Now only linear interpolation is supported in  resample3dimg_linear_interp() [you pass a code %d].\n", interp_method);
//        return false;
//    }

    V3DLONG cur_sz0 = (V3DLONG)(floor(double(sz[0]) / double(dfactor_x)));
    V3DLONG cur_sz1 = (V3DLONG)(floor(double(sz[1]) / double(dfactor_y)));
    V3DLONG cur_sz2 = (V3DLONG)(floor(double(sz[2]) / double(dfactor_z)));
    V3DLONG cur_sz3 = sz[3];

    if (cur_sz0 <= 0 || cur_sz1 <=0 || cur_sz2<=0 || cur_sz3<=0)
    {
        fprintf(stderr, "The dfactors are not properly set, -- the resulted resampled size is too small. Do nothing. [%s][%d].\n", __FILE__, __LINE__);
        return false;
    }

    T * outimg = new T [cur_sz0 * cur_sz1 * cur_sz2 * cur_sz3];
    if (!outimg)
    {
        fprintf(stderr, "Fail to allocate memory. [%s][%d].\n", __FILE__, __LINE__);
        return false;
    }

    T **** out_tmp4d = 0;
    T **** in_tmp4d = 0;

    new4dpointer(out_tmp4d, cur_sz0, cur_sz1, cur_sz2, cur_sz3, outimg);
    new4dpointer(in_tmp4d, sz[0], sz[1], sz[2], sz[3], img);

    for (V3DLONG c=0;c<cur_sz3;c++)
    {
        for (V3DLONG k=0;k<cur_sz2;k++)
        {
            V3DLONG k2low=(V3DLONG)(floor(k*dfactor_z)), k2high=(V3DLONG)(floor((k+1)*dfactor_z-1));
            if (k2high>sz[2]-1) k2high = sz[2]-1;
            V3DLONG kw = k2high - k2low + 1;

            for (V3DLONG j=0;j<cur_sz1;j++)
            {
                V3DLONG j2low=(V3DLONG)(floor(j*dfactor_y)), j2high=(V3DLONG)(floor((j+1)*dfactor_y-1));
                if (j2high>sz[1]-1) j2high = sz[1]-1;
                V3DLONG jw = j2high - j2low + 1;

                for (V3DLONG i=0;i<cur_sz0;i++)
                {
                    V3DLONG i2low=(V3DLONG)(floor(i*dfactor_x)), i2high=(V3DLONG)(floor((i+1)*dfactor_x-1));
                    if (i2high>sz[0]-1) i2high = sz[0]-1;
                    V3DLONG iw = i2high - i2low + 1;

                    double cubevolume = double(kw) * jw * iw;
                    //cout<<cubevolume <<" ";

                    if (interp_method == 1) {
                        double s=0.0;
                        for (V3DLONG k1=k2low;k1<=k2high;k1++)
                        {
                            for (V3DLONG j1=j2low;j1<=j2high;j1++)
                            {
                                for (V3DLONG i1=i2low;i1<=i2high;i1++)
                                {
                                    s += in_tmp4d[c][k1][j1][i1];
                                }
                            }
                        }

                        out_tmp4d[c][k][j][i] = (T)(s/cubevolume);
                    } else {
                        int index_k = round(k2low + (k2high - k2low) / 2);
                        int index_j = round(j2low + (j2high - j2low) / 2);
                        int index_i = round(i2low + (i2high - i2low) / 2);
                        out_tmp4d[c][k][j][i] = (T)(in_tmp4d[c][index_k][index_j][index_i]);
                    }
                }
            }
        }
    }

    //delete temprary 4d pointers

    delete4dpointer(out_tmp4d, cur_sz0, cur_sz1, cur_sz2, cur_sz3);
    delete4dpointer(in_tmp4d, sz[0], sz[1], sz[2], sz[3]);

    //copy to output data

    delete []img;
    img = outimg;

    sz[0] = cur_sz0;
    sz[1] = cur_sz1;
    sz[2] = cur_sz2;
    sz[3] = cur_sz3;

    return true;
}

bool getBrainRegions(QString imagePath, int flag, float ratio, int thres, V3DPluginCallback2 & callback);

bool getIndexImageMarkers(QString indexImagePath, QString imagePath, int flag, float ratio, int thres, V3DPluginCallback2 & callback);

bool runGDForBrainRegion(int brainRegionIndex, map<int, ImageMarker>& markerMap, map<int, set<int> >& neighborIndexMap, unsigned char* dataIndex, int datatypeIndex, Image4DSimple *p4dImage, V3DLONG* sz, QString imageDir);

bool resample3DImage(QString inImagePath, QString outImagePath, int sz_x, int sz_y, int sz_z, int flag, V3DPluginCallback2 & callback);

#endif // CONNECTIVITY_FUNC_H
