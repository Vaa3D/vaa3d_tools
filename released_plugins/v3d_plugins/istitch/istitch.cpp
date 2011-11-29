/* istitch.cpp
 * 2010-02-08: the program is created by Yang Yu
 * updated 2010-07-15 by Yang Yu (version 1.0)
 * added command line by Yang Yu updated 2010-11-29
 * added batch_group_stitching function by Jinzhu Yang 2011-4-15
 * added batch_generate_rawdata from tc file function by Jinzhu Yang 2011-4-19
 * added subpixel registration module by Yang Yu 2011-10-22
 * added completely new algorithm to obtain global consistency by Yang Yu 2011-11-11 (version 1.1)
 */

/*
 * The reference paper for version 1.0
 *
 * "Automated high speed stitching of large 3D microscopic images"
 * Yang Yu and Hanchuan Peng
 * Proc. of IEEE 2011 International Symposium on Biomedical Imaging: From Nano to Macro (ISBI'2011), pp.238-241, 2011.
 *
 */

// 

#ifndef __ISTITCH_SRC_CPP__
#define __ISTITCH_SRC_CPP__

#include <QtGui>

#include <cmath>
#include <stdlib.h>
#include <ctime>

#include "istitch.h"
#include "istitch_gui.h"

#include "y_imglib.h"

#include "basic_surf_objs.h"
#include "stackutil.h"
#include "volimg_proc.h"
#include "img_definition.h"
#include "basic_landmark.h"

#include "mg_utilities.h"
#include "mg_image_lib.h"

#include "basic_landmark.h"
#include "basic_4dimage.h"

// extracrt roi image
#include "../basic_c_fun/customary_structs/v3d_imaging_para.h"

// 
class PL 
{
public:
    PL(){}
    ~PL(){}

public:
    V3DLONG pln;
    V3DLONG count;

};

typedef std::vector<PL> PLLIST;

enum StitchDataType {ISUINT8=1, ISUINT16=2, ISREAL=4};

// Open a series of inputs
QStringList importSeriesFileList_addnumbersort(const QString & curFilePath)
{
    QStringList myList;
    myList.clear();

    // get the image files namelist in the directory
    QStringList imgSuffix;
    imgSuffix<<"*.tif"<<"*.raw"<<"*.v3draw"<<"*.lsm"
            <<"*.TIF"<<"*.RAW"<<"*.V3DRAW"<<"*.LSM";

    QDir dir(curFilePath);
    if (!dir.exists())
    {
        qWarning("Cannot find the directory");
        return myList;
    }

    foreach (QString file, dir.entryList(imgSuffix, QDir::Files, QDir::Name))
    {
        myList += QFileInfo(dir, file).absoluteFilePath();
    }

    // print filenames
    foreach (QString qs, myList)  qDebug() << qs;

    return myList;
}

#ifdef COMPILE_TO_COMMANDLINE

// compute normalized cross correlation between a pair of images
template <class SDATATYPE, class Y_IMG_DATATYPE>
int cmpt_ncc_imagepair(SDATATYPE *subject1d, V3DLONG *sz_subject1d, SDATATYPE *target1d, V3DLONG *sz_target1d, PEAKS *&pos)
{
    //
    V3DLONG sx = sz_subject1d[0];
    V3DLONG sy = sz_subject1d[1];
    V3DLONG sz = sz_subject1d[2]; 
    V3DLONG sc = sz_subject1d[3];

    V3DLONG pagesz_sub = sx*sy*sz;

    V3DLONG tx = sz_target1d[0];
    V3DLONG ty = sz_target1d[1];
    V3DLONG tz = sz_target1d[2];
    V3DLONG tc = sz_target1d[3];

    V3DLONG pagesz_tar = tx*ty*tz;

    // anchor position
    REAL pos_score = 0;
    V3DLONG pos_x=0, pos_y=0, pos_z=0;

    V3DLONG *szPad = new V3DLONG [3];
    V3DLONG *szTar = new V3DLONG [3];
    V3DLONG *szSub = new V3DLONG [3];

    V3DLONG sx_pad = sx+tx-1, sy_pad = sy+ty-1, sz_pad = sz+tz-1;

    V3DLONG even_odd = sx_pad%2; // 0 for even 1 for odd

    bool fftw_in_place = true;

    if(fftw_in_place)
        sx_pad += (2-even_odd); //2*(sx_pad/2 + 1); // fftw_in_place

    V3DLONG len_pad = sx_pad*sy_pad*sz_pad;

    //
    szPad[0] = sx_pad;
    szPad[1] = sy_pad;
    szPad[2] = sz_pad;

    //
    szTar[0] = tx;
    szTar[1] = ty;
    szTar[2] = tz;

    //
    szSub[0] = sx;
    szSub[1] = sy;
    szSub[2] = sz;

    // padding target with zeros
    REAL* pTar = new REAL [len_pad];
    if (!pTar)
    {
        printf("Fail to allocate memory.\n");
        return -1;
    }
    else
    {
        for(V3DLONG i=0; i<len_pad; i++)
            pTar[i]=0;

        //padding zeros for target imag
        Y_IMG_REAL pOut(pTar, szPad);
        Y_IMG_DATATYPE pIn(target1d, szTar);

        YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
        tmp.padding(pOut, pIn, false, fftw_in_place, even_odd, 3);

    }

    // padding subject with zeros
    REAL* pSub = new REAL [len_pad];
    if (!pSub)
    {
        printf("Fail to allocate memory.\n");
        return -1;
    }
    else
    {
        for(V3DLONG i=0; i<len_pad; i++)
            pSub[i]=0;

        //padding zeros for subject image
        Y_IMG_REAL pOut(pSub, szPad);
        Y_IMG_DATATYPE pIn(subject1d, szSub);

        YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
        tmp.padding(pOut, pIn, true, fftw_in_place, even_odd, 3);
    }

    //ncc
    Y_IMG_REAL pOut(pSub, szPad);
    Y_IMG_REAL pIn(pTar, szPad);

    YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_REAL> tmp;
    tmp.cmpt_ncc3D(pOut, pIn, szSub, szTar, even_odd, fftw_in_place, pos);

    pos_x = pos->x; pos_y = pos->y; pos_z = pos->z; pos_score = pos->value;

    if(pos_score == -1)
        return -1;

    //de-alloc
    if(pTar) {delete []pTar; pTar=0;}
    if(pSub) {delete []pSub; pSub=0;}

    //
    return true;

}		

// Multiscale pairwise images stitching with thick planes (e.g. 10 pixels width) in boundary bounding box
template <class SDATATYPE, class Y_IMG_DATATYPE>
int stitching_bb_thickplanes(SDATATYPE *subject1d, V3DLONG *sz_subject1d, SDATATYPE *target1d, V3DLONG *sz_target1d, REAL overlap_percent, bool m_similarity, REAL *scale, PEAKS *&pos)
{
    //groupisflag is for group image stitching flag.
    //0: pairwise image stitching
    //1: return the score and shift offsets of downsampled images
    //2: rerurn the final score and shift offsets

    //
    V3DLONG sx = sz_subject1d[0];
    V3DLONG sy = sz_subject1d[1];
    V3DLONG sz = sz_subject1d[2]; 
    V3DLONG sc = sz_subject1d[3];

    V3DLONG pagesz_sub = sx*sy*sz;

    V3DLONG tx = sz_target1d[0];
    V3DLONG ty = sz_target1d[1];
    V3DLONG tz = sz_target1d[2];
    V3DLONG tc = sz_target1d[3];

    V3DLONG pagesz_tar = tx*ty*tz;

    REAL threshold_corr_score = 0; //0.75; //

    // sampling by factors less than 1 (downsample)
    // obtain an approximate translative estimation at a coarse scale
    //--------------------------------------------------------------------------------------
    V3DLONG ssx, ssy, ssz, stx, sty, stz;

    V3DLONG sx_ori = sx;
    V3DLONG sy_ori = sy;
    V3DLONG sz_ori = sz;

    V3DLONG tx_ori = tx;
    V3DLONG ty_ori = ty;
    V3DLONG tz_ori = tz;

    // anchor position
    REAL pos_score = 0;
    V3DLONG pos_x=0, pos_y=0, pos_z=0;

    V3DLONG *szPad = new V3DLONG [3];
    V3DLONG *szTar = new V3DLONG [3];
    V3DLONG *szSub = new V3DLONG [3];

    size_t start_t_2;

    if(true) // groupflag
    {

        ssx = (V3DLONG)(ceil(sx*scale[0]));
        ssy = (V3DLONG)(ceil(sy*scale[1]));
        ssz = (V3DLONG)(ceil(sz*scale[2]));

        stx = (V3DLONG)(ceil(tx*scale[0]));
        sty = (V3DLONG)(ceil(ty*scale[1]));
        stz = (V3DLONG)(ceil(tz*scale[2]));

        SDATATYPE *p_sub, *p_tar;

        if(scale[0]==1 && scale[1]==1 && scale[2]==1)
        {
            p_sub = subject1d;
            p_tar = target1d;
        }
        else
        {
            //
            V3DLONG pagesz_ds_sub = ssx*ssy*ssz;
            V3DLONG pagesz_ds_tar = stx*sty*stz;

            V3DLONG *sz_down_target = new V3DLONG [3];

            sz_down_target[0] = stx;
            sz_down_target[1] = sty;
            sz_down_target[2] = stz;

            V3DLONG *sz_down_subject = new V3DLONG [3];

            sz_down_subject[0] = ssx;
            sz_down_subject[1] = ssy;
            sz_down_subject[2] = ssz;

            p_sub = new SDATATYPE [pagesz_ds_sub];
            if (!p_sub)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }
            else
            {
                // down sampling
                Y_IMG_DATATYPE pIn(subject1d, sz_subject1d);
                Y_IMG_DATATYPE pOut(p_sub, sz_down_subject);

                YImg<REAL, V3DLONG, Y_IMG_DATATYPE, Y_IMG_DATATYPE> tmp;
                tmp.down_sampling(pOut, pIn, scale);
            }

            p_tar = new SDATATYPE [pagesz_ds_tar];
            if (!p_tar)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }
            else
            {
                // down sampling
                Y_IMG_DATATYPE pIn(target1d, sz_target1d);
                Y_IMG_DATATYPE pOut(p_tar, sz_down_target);

                YImg<REAL, V3DLONG, Y_IMG_DATATYPE, Y_IMG_DATATYPE> tmp;
                tmp.down_sampling(pOut, pIn, scale);
            }
        }


        sx = ssx; sy = ssy; sz = ssz;
        tx = stx; ty = sty; tz = stz;

        // 3D images stitching
        //--------------------------------------------------------------------------------------

        // declaration
        V3DLONG sx_pad = sx+tx-1, sy_pad = sy+ty-1, sz_pad = sz+tz-1;

        V3DLONG even_odd = sx_pad%2; // 0 for even 1 for odd

        bool fftw_in_place = true;

        if(fftw_in_place)
            sx_pad += (2-even_odd); //2*(sx_pad/2 + 1); // fftw_in_place

        V3DLONG len_pad = sx_pad*sy_pad*sz_pad;

        //
        szPad[0] = sx_pad;
        szPad[1] = sy_pad;
        szPad[2] = sz_pad;

        //
        szTar[0] = tx;
        szTar[1] = ty;
        szTar[2] = tz;

        //
        szSub[0] = sx;
        szSub[1] = sy;
        szSub[2] = sz;

        // padding target with zeros
        REAL* pTar = new REAL [len_pad];
        if (!pTar)
        {
            printf("Fail to allocate memory.\n");
            return -1;
        }
        else
        {
            for(V3DLONG i=0; i<len_pad; i++)
                pTar[i]=0;

            //padding zeros for target image
            Y_IMG_REAL pOut(pTar, szPad);
            Y_IMG_DATATYPE pIn(p_tar, szTar);

            YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
            tmp.padding(pOut, pIn, false, fftw_in_place, even_odd, 3);
        }

        // padding subject with zeros
        REAL* pSub = new REAL [len_pad];
        if (!pSub)
        {
            printf("Fail to allocate memory.\n");
            return -1;
        }
        else
        {
            for(V3DLONG i=0; i<len_pad; i++)
                pSub[i]=0;

            //padding zeros for subject image
            Y_IMG_REAL pOut(pSub, szPad);
            Y_IMG_DATATYPE pIn(p_sub, szSub);

            YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
            tmp.padding(pOut, pIn, true, fftw_in_place, even_odd, 3);
        }

        //ncc || pc-ncc
        if(m_similarity==true)
        {

            Y_IMG_REAL pOut(pSub, szPad);
            Y_IMG_REAL pIn(pTar, szPad);

            YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_REAL> tmp;
            tmp.fftncc(pOut, pIn, szTar, even_odd, fftw_in_place, overlap_percent, 3); // fftw_in_place

            //find the highest response of FFT CC (pSub)
            V3DLONG sx_pad_ori = sx_pad - (2-even_odd);

            for(V3DLONG k=0; k<sz_pad; k++)
            {
                V3DLONG offset_k = k*sx_pad*sy_pad;
                for(V3DLONG j=0; j<sy_pad; j++)
                {
                    V3DLONG offset_j = offset_k + j*sx_pad;
                    for(V3DLONG i=0; i<sx_pad_ori; i++)	// fftw_in_place
                    {
                        V3DLONG idx = offset_j + i;

                        REAL score = pSub[idx];

                        //obtain the highest response
                        if(score>pos_score)
                        {
                            //qDebug("score %lf", score);
                            pos_score = score;
                            pos_x = i; pos_y = j; pos_z = k;
                        }

                    }
                }
            }

        }
        else
        {
            PEAKS pos_pcncc;
            Y_IMG_REAL pOut(pSub, szPad);
            Y_IMG_REAL pIn(pTar, szPad);

            YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_REAL> tmp;
            tmp.fftpcncc3D(pOut, pIn, szTar, even_odd, fftw_in_place, overlap_percent, &pos_pcncc);

            pos_x = pos_pcncc.x; pos_y = pos_pcncc.y; pos_z = pos_pcncc.z; pos_score = pos_pcncc.value;
        }

        //qDebug() << "score ..." << pos_score;

        if(pos_score == -1)
            return -1;

        // coarse-scale done
        pos_x /= scale[0]; pos_y /= scale[1]; pos_z /= scale[2];

        //de-alloc
        if(pTar) {delete []pTar; pTar=0;}
        if(pSub) {delete []pSub; pSub=0;}

        //
        qDebug() << " ds us offsets ..." << pos_x << pos_y << pos_z << "ds correlation score ..." << pos_score;

        //
        if(pos_score<threshold_corr_score)
            return true;

    }


    // step 2 --------------------------------------------------------------------------------------------
    // choose a thick planes from boundary bounding box from sub image then register it with tar image boundary bounding box
    // at a fine scale
    if(!(scale[0]==1 && scale[1]==1 && scale[2]==1))
    {
        sx = sx_ori; sy = sy_ori; sz = sz_ori;
        tx = tx_ori; ty = ty_ori; tz = tz_ori;

        //
        V3DLONG offset[3];
        offset[0] = pos_x - sx +1;
        offset[1] = pos_y - sy +1;
        offset[2] = pos_z - sz +1;

        qDebug() << "offset ..." << offset[0] << offset[1] << offset[2];

        V3DLONG nbbx, nbby, nbbz;

        nbbx = 1/scale[0] + 2;
        nbby = 1/scale[1] + 2;
        nbbz = 1/scale[2] + 2;

        V3DLONG bsx = (offset[0]>0)?0:-offset[0];
        V3DLONG bsy = (offset[1]>0)?0:-offset[1];
        V3DLONG bsz = (offset[2]>0)?0:-offset[2];

        if(bsx-nbbx>=0)
            bsx -= nbbx;
        else
            bsx = 0;

        if(bsy-nbby>=0)
            bsy -= nbby;
        else
            bsy = 0;

        if(bsz-nbbz>=0)
            bsz -= nbbz;
        else
            bsz = 0;

        V3DLONG esx = (sx-1);
        V3DLONG esy = (sy-1);
        V3DLONG esz = (sz-1);

        if(offset[0])
        {
            V3DLONG tmp = sx-1 + offset[0] - (tx-1);

            if(tmp>0)
                esx = sx-1 -tmp;
        }

        if(offset[1])
        {
            V3DLONG tmp = offset[1]+sy-1 - (ty-1);

            if(tmp>0)
                esy = sy-1 - tmp;
        }

        if(offset[2])
        {
            V3DLONG tmp = offset[2]+sz-1 - (tz-1);

            if(tmp>0)
                esz = sz-1 -tmp;
        }

        if(esx+nbbx<=sx-1)
            esx += nbbx;
        else
            esx = sx-1;

        if(esy+nbby<=sy-1)
            esy += nbby;
        else
            esy = sy-1;

        if(esz+nbbz<=sz-1)
            esz += nbbz;
        else
            esz = sz-1;

        V3DLONG btx = (offset[0]>0)?offset[0]:0;
        V3DLONG bty = (offset[1]>0)?offset[1]:0;
        V3DLONG btz = (offset[2]>0)?offset[2]:0;

        if(btx-nbbx>=0)
            btx -= nbbx;
        else
            btx = 0;

        if(bty-nbby>=0)
            bty -= nbby;
        else
            bty = 0;

        if(btz-nbbz>=0)
            btz -= nbbz;
        else
            btz = 0;

        V3DLONG etx = (offset[0]>0)?(tx-1):pos_x;
        V3DLONG ety = (offset[1]>0)?(ty-1):pos_y;
        V3DLONG etz = (offset[2]>0)?(tz-1):pos_z;

        if(offset[0]>0)
        {
            V3DLONG tmp = (tx-1) - (sx-1 + offset[0]);

            if(tmp>0)
                etx = tx-1 - tmp;
        }

        if(offset[1]>0)
        {
            V3DLONG tmp = (ty-1) - (sy-1 + offset[1]);

            if(tmp>0)
                ety = ty-1 - tmp;
        }

        if(offset[2]>0)
        {
            V3DLONG tmp = (tz-1) - (sz-1 + offset[2]);

            if(tmp>0)
                etz = tz-1 - tmp;
        }


        if(etx+nbbx<=tx-1)
            etx += nbbx;
        else
            etx = tx-1;

        if(ety+nbby<=ty-1)
            ety += nbby;
        else
            ety = ty-1;

        if(etz+nbbz<=tz-1)
            etz += nbbz;
        else
            etz = tz-1;

        V3DLONG bbsx = esx - bsx + 1; if(bbsx<0 || bbsx>sx) return -1;
        V3DLONG bbsy = esy - bsy + 1; if(bbsy<0 || bbsy>sy) return -1;
        V3DLONG bbsz = esz - bsz + 1; if(bbsz<0 || bbsz>sz) return -1;

        V3DLONG bbtx = etx - btx + 1; if(bbtx<0 || bbtx>tx) return -1;
        V3DLONG bbty = ety - bty + 1; if(bbty<0 || bbty>ty) return -1;
        V3DLONG bbtz = etz - btz + 1; if(bbtz<0 || bbtz>tz) return -1;

        qDebug() << " testing ... boundary ... "<< bsx << bsy << bsz << esx << esy << esz << btx << bty << btz << etx << ety << etz;
        qDebug() << " testing ... dims ... "<< bbsx << bbsy << bbsz << bbtx << bbty << bbtz;
        qDebug() << "...";

        float rate_x, rate_y, rate_z;

        rate_x = qMax( (float)bbsx/(float)sx, (float)bbtx/(float)tx);
        rate_y = qMax( (float)bbsy/(float)sy, (float)bbty/(float)ty);
        rate_z = qMax( (float)bbsz/(float)sz, (float)bbtz/(float)tz);

        // crop options
        // planes
        bool plane_yz=false, plane_xz=false, plane_xy=false;

        if(rate_x < rate_y && rate_x < rate_z)
        {
            plane_yz = true;

            bsy = 0; esy = sy-1;
            bbsy = sy;

            bty = 0; ety = ty-1;
            bbty = ty;

            bsz = 0; esz = sz-1;
            bbsz = sz;

            btz = 0; etz = tz-1;
            bbtz = tz;

        }
        if(rate_y < rate_x && rate_y < rate_z)
        {
            plane_xz = true;

            bsx = 0; esx = sx-1;
            bbsx = sx;

            btx = 0; etx = tx-1;
            bbtx = tx;

            bsz = 0; esz = sz-1;
            bbsz = sz;

            btz = 0; etz = tz-1;
            bbtz = tz;

        }
        if(rate_z < rate_x && rate_z < rate_y)
        {
            plane_xy = true;

            bsx = 0; esx = sx-1;
            bbsx = sx;

            btx = 0; etx = tx-1;
            bbtx = tx;

            bsy = 0; esy = sy-1;
            bbsy = sy;

            bty = 0; ety = ty-1;
            bbty = ty;

        }

        //
        qDebug() << "rate ..." << rate_x << rate_y << rate_z << "planes ..." << plane_yz << plane_xz << plane_xy;
        qDebug() << " current bounding box ... boundary ... "<< bsx << bsy << bsz << esx << esy << esz << btx << bty << btz << etx << ety << etz;
        qDebug() << " current bounding box ... dims ... "<< bbsx << bbsy << bbsz << bbtx << bbty << bbtz;

        //
        REAL sum=0;
        for(V3DLONG k=bsz; k<=esz; k++)
        {
            V3DLONG offset_o_k = k*sx*sy;
            for(V3DLONG j=bsy; j<=esy; j++)
            {
                V3DLONG offset_o_j = offset_o_k + j*sx;
                for(V3DLONG i=bsx; i<=esx; i++)
                {
                    V3DLONG idx_o = offset_o_j + i;

                    sum += subject1d[idx_o];
                }
            }
        }
        SDATATYPE meanv = (SDATATYPE) (sum/(bbsx*bbsy*bbsz));

        // extract thick planes boundary bounding box
        //---------------------------------------------------------------------------------------------------------------
        if(plane_yz == true)
        {
            // finding rich information plane from sub aV3DLONG x
            V3DLONG info_count=0, xpln=0, max_info=0;

            // approach 1
            V3DLONG weights = bbsx*0.15;
            weights /= 2;
            V3DLONG start_x=bsx+weights+nbbx, end_x=esx-weights-nbbx;
            for(V3DLONG i=start_x; i<=end_x; i++) //
            {
                info_count = 0;

                for(V3DLONG k=bsz; k<=esz; k++)
                {
                    V3DLONG offset_o_k = k*sx*sy;
                    for(V3DLONG j=bsy; j<=esy; j++)
                    {
                        V3DLONG idx_o = offset_o_k + j*sx + i;

                        if( subject1d[idx_o] > meanv)
                            info_count++;
                    }
                }

                if(info_count > max_info)
                {
                    max_info = info_count;
                    xpln = i;
                }

            }

            //
            if(xpln<start_x) xpln = start_x;

            qDebug() << "xpln ..." << xpln;

            // extraction
            V3DLONG b_bsx = xpln - weights; //

            if(b_bsx>bsx)
                bsx = b_bsx;

            V3DLONG e_esx = xpln + weights;

            if(e_esx<esx)
                esx = e_esx;

            bbsx = esx-bsx+1; // dims

            // crop corresponding thick planes from tar image
            V3DLONG b_btx = bsx + offset[0] - nbbx;

            if(b_btx>btx)
                btx = b_btx;

            V3DLONG e_etx = esx + offset[0] + nbbx;

            if(e_etx<etx)
                etx = e_etx;

            bbtx = etx-btx+1;

            qDebug() << " updated boundary ... "<< bsx << bsy << bsz << esx << esy << esz << btx << bty << btz << etx << ety << etz;
            qDebug() << " updated dims ... "<< bbsx << bbsy << bbsz << bbtx << bbty << bbtz;

            V3DLONG pagesz_bb_sub = bbsx*bbsy*bbsz;
            V3DLONG pagesz_bb_tar = bbtx*bbty*bbtz;

            // extract one plane from sub
            SDATATYPE* p_sub = new SDATATYPE [pagesz_bb_sub];
            if (!p_sub)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }
            else
            {
                for(V3DLONG k=bsz; k<=esz; k++)
                {
                    V3DLONG offset_k = (k-bsz)*bbsx*bbsy;
                    V3DLONG offset_o_k = k*sx*sy;
                    for(V3DLONG j=bsy; j<=esy; j++)
                    {
                        V3DLONG offset_j = offset_k + (j-bsy)*bbsx;
                        V3DLONG offset_o_j = offset_o_k + j*sx;
                        for(V3DLONG i=bsx; i<=esx; i++)
                        {
                            V3DLONG idx = offset_j + (i-bsx);
                            V3DLONG idx_o = offset_o_j + i;

                            p_sub[idx] = subject1d[idx_o];
                        }
                    }
                }
            }

            SDATATYPE* p_tar = new SDATATYPE [pagesz_bb_tar];
            if (!p_tar)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }


            //REAL max_response = 0;
            // search the best match plane from tar
            for(V3DLONG k=btz; k<=etz; k++)
            {
                V3DLONG offset_k = (k-btz)*bbtx*bbty;
                V3DLONG offset_o_k = k*tx*ty;
                for(V3DLONG j=bty; j<=ety; j++)
                {
                    V3DLONG offset_j = offset_k + (j-bty)*bbtx;
                    V3DLONG offset_o_j = offset_o_k + j*tx;
                    for(V3DLONG i=btx; i<=etx; i++)
                    {
                        V3DLONG idx = offset_j + (i-btx);
                        V3DLONG idx_o = offset_o_j + i;

                        p_tar[idx] = target1d[idx_o];
                    }
                }
            }

            //
            sx = bbsx; sy = bbsy; sz = bbsz;
            tx = bbtx; ty = bbty; tz = bbtz;

            //
            V3DLONG sx_pad = sx+tx-1, sy_pad = sy+ty-1, sz_pad = sz+tz-1;

            V3DLONG even_odd = sx_pad%2; // 0 for even 1 for odd

            bool fftw_in_place = true;

            if(fftw_in_place)
                sx_pad += (2-even_odd); //2*(sx_pad/2 + 1); // fftw_in_place

            V3DLONG len_pad = sx_pad*sy_pad*sz_pad;

            szPad[0] = sx_pad;
            szPad[1] = sy_pad;
            szPad[2] = sz_pad;

            szTar[0] = tx;
            szTar[1] = ty;
            szTar[2] = tz;

            szSub[0] = sx;
            szSub[1] = sy;
            szSub[2] = sz;

            REAL* p_f_sub = new REAL [len_pad];
            if (!p_f_sub)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }
            else
            {
                for(V3DLONG i=0; i<len_pad; i++)
                    p_f_sub[i]=0;

                //padding zeros for target image
                Y_IMG_REAL pOut(p_f_sub, szPad);
                Y_IMG_DATATYPE pIn(p_sub, szSub);

                YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
                tmp.padding(pOut, pIn, true, fftw_in_place, even_odd, 3);
            }

            REAL* p_f_tar = new REAL [len_pad];
            if (!p_f_tar)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }
            else
            {
                for(V3DLONG i=0; i<len_pad; i++)
                    p_f_tar[i]=0;

                //padding zeros for target image
                Y_IMG_REAL pOut(p_f_tar, szPad);
                Y_IMG_DATATYPE pIn(p_tar, szTar);

                YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
                tmp.padding(pOut, pIn, false, fftw_in_place, even_odd, 3);
            }

            //de-alloc
            if(p_sub) {delete []p_sub; p_sub=0;}
            if(p_tar) {delete []p_tar; p_tar=0;}


            // fft-ncc
            PEAKS pos_pcncc;

            pos_pcncc.x = (bsx - ((sx_ori-1) - pos_x)) - btx + (bbsx-1);
            pos_pcncc.y = pos_y;
            pos_pcncc.z = pos_z;

            Y_IMG_REAL pOut(p_f_sub, szPad);
            Y_IMG_REAL pIn(p_f_tar, szPad);

            YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_REAL> tmp;
            tmp.fftnccp3D(pOut, pIn, szSub, szTar, even_odd, fftw_in_place, scale, &pos_pcncc);

            //pos_score = pos_pcncc.value;

            qDebug() << " bb score ..." << pos_score << "offset ..." << bsx << btx << pos_pcncc.x << bbsx;

            pos_x = (sx_ori-1) - (bsx - (btx + pos_pcncc.x - (bbsx-1))); pos_y = pos_pcncc.y; pos_z = pos_pcncc.z; //

            //de-alloc
            if(p_f_sub) {delete []p_f_sub; p_f_sub=0;}
            if(p_f_tar) {delete []p_f_tar; p_f_tar=0;}


        }
        else if(plane_xz == true)
        {
            // finding rich information plane from sub aV3DLONG y
            V3DLONG info_count=0, ypln, max_info=0;

            V3DLONG weights = bbsy*0.15;
            weights /= 2;
            V3DLONG start_y=bsy+weights+nbby, end_y=esy-weights-nbby;
            for(V3DLONG j=start_y; j<=end_y; j++) //
            {
                info_count = 0;

                V3DLONG offset_o_j = j*sx;
                for(V3DLONG k=bsz; k<=esz; k++)
                {
                    V3DLONG offset_o_k = k*sx*sy;
                    for(V3DLONG i=bsx; i<=esx; i++)
                    {
                        V3DLONG idx_o = offset_o_k + offset_o_j + i;

                        if( subject1d[idx_o] > meanv)
                            info_count++;
                    }
                }

                if(info_count > max_info)
                {
                    max_info = info_count;
                    ypln = j;
                }

            }

            qDebug() << "plane ..." << ypln;

            // extraction
            V3DLONG b_bsy = ypln - weights;

            if(b_bsy>bsy)
                bsy = b_bsy;

            V3DLONG e_esy = ypln + weights;

            if(e_esy<esy)
                esy = e_esy;

            bbsy = esy-bsy+1; // dims

            // crop corresponding thick planes from tar image
            V3DLONG b_bty = bsy + offset[1] - nbby; //ypln + offset[1] - nbby;

            if(b_bty>bty)
                bty = b_bty;

            V3DLONG e_ety = esy + offset[1] + nbby; //ypln + offset[1] + nbby;

            if(e_ety<ety)
                ety = e_ety;

            bbty = ety-bty+1;

            qDebug() << " updated boundary ... "<< bsx << bsy << bsz << esx << esy << esz << btx << bty << btz << etx << ety << etz;
            qDebug() << " updated dims ... "<< bbsx << bbsy << bbsz << bbtx << bbty << bbtz;

            V3DLONG pagesz_bb_sub = bbsx*bbsy*bbsz;
            V3DLONG pagesz_bb_tar = bbtx*bbty*bbtz;

            // extract one plane from sub
            SDATATYPE* p_sub = new SDATATYPE [pagesz_bb_sub];
            if (!p_sub)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }
            else
            {
                for(V3DLONG k=bsz; k<=esz; k++)
                {
                    V3DLONG offset_k = (k-bsz)*bbsx*bbsy;
                    V3DLONG offset_o_k = k*sx*sy;
                    for(V3DLONG j=bsy; j<=esy; j++)
                    {
                        V3DLONG offset_j = offset_k + (j-bsy)*bbsx;
                        V3DLONG offset_o_j = offset_o_k + j*sx;
                        for(V3DLONG i=bsx; i<=esx; i++)
                        {
                            V3DLONG idx = offset_j + (i-bsx);
                            V3DLONG idx_o = offset_o_j + i;

                            p_sub[idx] = subject1d[idx_o];
                        }
                    }
                }
            }

            SDATATYPE* p_tar = new SDATATYPE [pagesz_bb_tar];
            if (!p_tar)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }


            //REAL max_response = 0;
            // search the best match plane from tar
            for(V3DLONG k=btz; k<=etz; k++)
            {
                V3DLONG offset_k = (k-btz)*bbtx*bbty;
                V3DLONG offset_o_k = k*tx*ty;
                for(V3DLONG j=bty; j<=ety; j++)
                {
                    V3DLONG offset_j = offset_k + (j-bty)*bbtx;
                    V3DLONG offset_o_j = offset_o_k + j*tx;
                    for(V3DLONG i=btx; i<=etx; i++)
                    {
                        V3DLONG idx = offset_j + (i-btx);
                        V3DLONG idx_o = offset_o_j + i;

                        p_tar[idx] = target1d[idx_o];
                    }
                }
            }

            //
            sx = bbsx; sy = bbsy; sz = bbsz;
            tx = bbtx; ty = bbty; tz = bbtz;

            V3DLONG sx_pad = sx+tx-1, sy_pad = sy+ty-1, sz_pad = sz+tz-1;

            V3DLONG even_odd = sx_pad%2; // 0 for even 1 for odd

            bool fftw_in_place = true;

            if(fftw_in_place)
                sx_pad += (2-even_odd); //2*(sx_pad/2 + 1); // fftw_in_place

            V3DLONG len_pad = sx_pad*sy_pad*sz_pad;

            szPad[0] = sx_pad;
            szPad[1] = sy_pad;
            szPad[2] = sz_pad;

            szTar[0] = tx;
            szTar[1] = ty;
            szTar[2] = tz;

            szSub[0] = sx;
            szSub[1] = sy;
            szSub[2] = sz;

            REAL* p_f_sub = new REAL [len_pad];
            if (!p_f_sub)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }
            else
            {
                for(V3DLONG i=0; i<len_pad; i++)
                    p_f_sub[i]=0;

                //padding zeros for target image
                Y_IMG_REAL pOut(p_f_sub, szPad);
                Y_IMG_DATATYPE pIn(p_sub, szSub);

                YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
                tmp.padding(pOut, pIn, true, fftw_in_place, even_odd, 3);
            }

            REAL* p_f_tar = new REAL [len_pad];
            if (!p_f_tar)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }
            else
            {
                for(V3DLONG i=0; i<len_pad; i++)
                    p_f_tar[i]=0;

                //padding zeros for target image
                Y_IMG_REAL pOut(p_f_tar, szPad);
                Y_IMG_DATATYPE pIn(p_tar, szTar);

                YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
                tmp.padding(pOut, pIn, false, fftw_in_place, even_odd, 3);
            }

            //de-alloc
            if(p_sub) {delete []p_sub; p_sub=0;}
            if(p_tar) {delete []p_tar; p_tar=0;}

            // fft-ncc
            PEAKS pos_pcncc;

            pos_pcncc.x = pos_x;
            pos_pcncc.y =  (bsy - ((sy_ori-1) - pos_y)) - bty + (bbsy-1);
            pos_pcncc.z = pos_z;

            Y_IMG_REAL pOut(p_f_sub, szPad);
            Y_IMG_REAL pIn(p_f_tar, szPad);

            YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_REAL> tmp;
            tmp.fftnccp3D(pOut, pIn, szSub, szTar, even_odd, fftw_in_place, scale, &pos_pcncc);

            //pos_score = pos_pcncc.value;

            qDebug() << " bb score ..." << pos_score << "offset ..." << bsy << bty << pos_pcncc.y << bbsy;

            pos_x = pos_pcncc.x; pos_y = (sy_ori-1) - (bsy - (bty + pos_pcncc.y - (bbsy-1))); pos_z = pos_pcncc.z; //

            //de-alloc
            if(p_f_sub) {delete []p_f_sub; p_f_sub=0;}
            if(p_f_tar) {delete []p_f_tar; p_f_tar=0;}

        }
        else if(plane_xy == true)
        {
            // finding rich information plane from sub aV3DLONG z
            V3DLONG info_count=0, zpln, max_info=0;

            V3DLONG weights = bbsz*0.15;
            weights /= 2;
            V3DLONG start_z=bsz+weights+nbbz, end_z=esz-weights-nbbz;
            for(V3DLONG k=start_z; k<=end_z; k++) //
            {
                info_count = 0;

                V3DLONG offset_o_k = k*sx*sy;
                for(V3DLONG j=bsy; j<=esy; j++)
                {
                    V3DLONG offset_o_j = j*sx;
                    for(V3DLONG i=bsx; i<=esx; i++)
                    {
                        V3DLONG idx_o = offset_o_k + offset_o_j + i;

                        if( subject1d[idx_o] > meanv)
                            info_count++;
                    }
                }

                if(info_count > max_info)
                {
                    max_info = info_count;
                    zpln = k;
                }

            }

            // extraction
            V3DLONG b_bsz = zpln - weights/2;

            if(b_bsz>bsz)
                bsz = b_bsz;

            V3DLONG e_esz = zpln + weights/2;

            if(e_esz<esz)
                esz = e_esz;

            bbsz = esz-bsz+1; // dims

            // crop corresponding thick planes from tar image
            V3DLONG b_btz = bsz + offset[2] - nbbz;

            if(b_btz>btz)
                btz = b_btz;

            V3DLONG e_etz = esz + offset[2] + nbbz;

            if(e_etz<etz)
                etz = e_etz;

            bbtz = etz-btz+1;

            qDebug() << " updated boundary ... "<< bsx << bsy << bsz << esx << esy << esz << btx << bty << btz << etx << ety << etz;
            qDebug() << " updated dims ... "<< bbsx << bbsy << bbsz << bbtx << bbty << bbtz;

            V3DLONG pagesz_bb_sub = bbsx*bbsy*bbsz;
            V3DLONG pagesz_bb_tar = bbtx*bbty*bbtz;

            // extract one plane from sub
            SDATATYPE* p_sub = new SDATATYPE [pagesz_bb_sub];
            if (!p_sub)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }
            else
            {
                for(V3DLONG k=bsz; k<=esz; k++)
                {
                    V3DLONG offset_k = (k-bsz)*bbsx*bbsy;
                    V3DLONG offset_o_k = k*sx*sy;
                    for(V3DLONG j=bsy; j<=esy; j++)
                    {
                        V3DLONG offset_j = offset_k + (j-bsy)*bbsx;
                        V3DLONG offset_o_j = offset_o_k + j*sx;
                        for(V3DLONG i=bsx; i<=esx; i++)
                        {
                            V3DLONG idx = offset_j + (i-bsx);
                            V3DLONG idx_o = offset_o_j + i;

                            p_sub[idx] = subject1d[idx_o];
                        }
                    }
                }
            }

            SDATATYPE* p_tar = new SDATATYPE [pagesz_bb_tar];
            if (!p_tar)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }


            //REAL max_response = 0;
            // search the best match plane from tar
            for(V3DLONG k=btz; k<=etz; k++)
            {
                V3DLONG offset_k = (k-btz)*bbtx*bbty;
                V3DLONG offset_o_k = k*tx*ty;
                for(V3DLONG j=bty; j<=ety; j++)
                {
                    V3DLONG offset_j = offset_k + (j-bty)*bbtx;
                    V3DLONG offset_o_j = offset_o_k + j*tx;
                    for(V3DLONG i=btx; i<=etx; i++)
                    {
                        V3DLONG idx = offset_j + (i-btx);
                        V3DLONG idx_o = offset_o_j + i;

                        p_tar[idx] = target1d[idx_o];
                    }
                }
            }

            //
            sx = bbsx; sy = bbsy; sz = bbsz;
            tx = bbtx; ty = bbty; tz = bbtz;

            V3DLONG sx_pad = sx+tx-1, sy_pad = sy+ty-1, sz_pad = sz+tz-1;

            V3DLONG even_odd = sx_pad%2; // 0 for even 1 for odd

            bool fftw_in_place = true;

            if(fftw_in_place)
                sx_pad += (2-even_odd); //2*(sx_pad/2 + 1); // fftw_in_place

            V3DLONG len_pad = sx_pad*sy_pad*sz_pad;

            szPad[0] = sx_pad;
            szPad[1] = sy_pad;
            szPad[2] = sz_pad;

            szTar[0] = tx;
            szTar[1] = ty;
            szTar[2] = tz;

            szSub[0] = sx;
            szSub[1] = sy;
            szSub[2] = sz;

            REAL* p_f_sub = new REAL [len_pad];
            if (!p_f_sub)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }
            else
            {
                for(V3DLONG i=0; i<len_pad; i++)
                    p_f_sub[i]=0;

                //padding zeros for target image
                Y_IMG_REAL pOut(p_f_sub, szPad);
                Y_IMG_DATATYPE pIn(p_sub, szSub);

                YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
                tmp.padding(pOut, pIn, true, fftw_in_place, even_odd, 3);
            }

            REAL* p_f_tar = new REAL [len_pad];
            if (!p_f_tar)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }
            else
            {
                for(V3DLONG i=0; i<len_pad; i++)
                    p_f_tar[i]=0;

                //padding zeros for target imag
                Y_IMG_REAL pOut(p_f_tar, szPad);
                Y_IMG_DATATYPE pIn(p_tar, szTar);

                YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
                tmp.padding(pOut, pIn, false, fftw_in_place, even_odd, 3);
            }

            //de-alloc
            if(p_sub) {delete []p_sub; p_sub=0;}
            if(p_tar) {delete []p_tar; p_tar=0;}


            // fft-ncc
            PEAKS pos_pcncc;

            pos_pcncc.x = pos_x;
            pos_pcncc.y = pos_y;
            pos_pcncc.z = (bsz - ((sz_ori-1) - pos_z)) - btz + (bbsz-1);

            Y_IMG_REAL pOut(p_f_sub, szPad);
            Y_IMG_REAL pIn(p_f_tar, szPad);

            YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_REAL> tmp;
            tmp.fftnccp3D(pOut, pIn, szSub, szTar, even_odd, fftw_in_place, scale, &pos_pcncc);

            //pos_score = pos_pcncc.value;

            qDebug() << " bb score ..." << pos_score << "offset ..." << bsz << btz << pos_pcncc.z << bbsz;

            pos_x = pos_pcncc.x; pos_y = pos_pcncc.y; pos_z = (sz_ori-1) - (bsz - (btz + pos_pcncc.z - (bbsz-1))); //

            //de-alloc
            if(p_f_sub) {delete []p_f_sub; p_f_sub=0;}
            if(p_f_tar) {delete []p_f_tar; p_f_tar=0;}
        }


        //de-alloc
        if(szPad) {delete []szPad; szPad=0;}
        if(szSub) {delete []szSub; szSub=0;}
        if(szTar) {delete []szTar; szTar=0;}

    }

    //Output
    //------------------------------------------------------------------------------------------------
    pos->x = pos_x; pos->y = pos_y; pos->z = pos_z; pos->value = pos_score;

    qDebug() << " finally..." << pos->x << pos->y << pos->z << "correlation score ..." << pos->value << "offset ..." << pos_x - sx_ori +1 << pos_y - sy_ori +1 << pos_z - sz_ori +1;

    return true;

}

// pairwise stitching function
int pairwise_stitching(char *fn_target, V3DLONG ch_tar, char *fn_subject, V3DLONG ch_sub, char *fn_stitched, char *fn_output_cs, bool flag_cmpt_ncc, PEAKS *&pos)
{
    // load images
    unsigned char *subject1d=NULL, *target1d=NULL;
    V3DLONG *sz_subject=NULL, *sz_target=NULL;
    int datatype_subject, datatype_target;

    V3DLONG sx, sy, sz, sc, tx, ty, tz, tc;

    //
    if(QFile::exists(QString(fn_target)))
    {
        if (loadImage(fn_target, target1d, sz_target, datatype_target)!=true)
        {
            cout << "The target image does not exist."<<endl;
            return false;
        }

        tx= sz_target[0], ty = sz_target[1], tz = sz_target[2]; tc = sz_target[3];

    }
    else
    {
        cout << "The target image does not exist."<<endl;
        return false;
    }

    if(QFile::exists(QString(fn_subject)))
    {
        if (loadImage(const_cast<char *>( fn_subject ), subject1d, sz_subject, datatype_subject)!=true)
        {
            cout << "The subject image does not exist."<<endl;
            return false;
        }

        sx= sz_subject[0], sy = sz_subject[1], sz = sz_subject[2]; sc = sz_subject[3];

    }
    else
    {
        cout << "The subject image does not exist."<<endl;
        return false;
    }

    //
    if(datatype_subject != datatype_target)
    {
        cout << "currently v3d_stitch_pair only support stitching images with the same datatype."<<endl;
        return false;
    }

    REAL overlap_percent = 0.05;

    REAL *scale = new REAL [6];

    scale[0] = 0.25;
    scale[1] = 0.25;
    scale[2] = 0.25;
    scale[3] = 1;
    scale[4] = 1;
    scale[5] = 1;

    //
    V3DLONG pagesz_sub = sx*sy*sz;
    V3DLONG pagesz_tar = tx*ty*tz;

    V3DLONG offset_sub = ch_sub*sx*sy*sz;
    V3DLONG offset_tar = ch_tar*tx*ty*tz;

    int start_t = clock();

    //stitching
    //--------------------------------------------------------------------------------------
    //choose the FFT NCC or FFT PC method
    bool m_similarity = false;

    // anchor position
    REAL pos_score = 0;
    V3DLONG pos_x=0, pos_y=0, pos_z=0;

    //PEAKS *pos= new PEAKS;

    V3DLONG *szSub = new V3DLONG [4];
    szSub[0] = sx; szSub[1] = sy; szSub[2] = sz; szSub[3] = sc;

    V3DLONG *szTar = new V3DLONG [4];
    szTar[0] = tx; szTar[1] = ty; szTar[2] = tz; szTar[3] = tc;

    REAL score1, score2; // score1: paper "Automated High Speed Stitching of Large 3D Microscopic Images" score;
    // score2: normalized cross correlation score in overlap region between two tiles

    int success;

    if(datatype_subject == 1)
    {
        success = stitching_bb_thickplanes<unsigned char, Y_IMG_UINT8>(subject1d+offset_sub, szSub, target1d+offset_tar, szTar, overlap_percent, m_similarity, scale, pos);
    }
    else if(datatype_subject == 2)
    {
        success = stitching_bb_thickplanes<unsigned short, Y_IMG_UINT16>((unsigned short *)(subject1d)+offset_sub, szSub, (unsigned short *)(target1d)+offset_tar, szTar, overlap_percent, m_similarity, scale, pos);
    }
    else if(datatype_subject == 4)
    {
        success = stitching_bb_thickplanes<REAL, Y_IMG_REAL>((REAL *)(subject1d)+offset_sub, szSub, (REAL *)(target1d)+offset_tar, szTar, overlap_percent, m_similarity, scale, pos);
    }
    else
    {
        printf("Currently this program only support UINT8, UINT16, and FLOAT32 data type 1.\n");
        return -1;
    }

    if(success!=true) return false;

    pos_x = pos->x; pos_y = pos->y; pos_z = pos->z; pos_score = pos->value;

    cout<< "pairwise pos " << pos->x << " " << pos->y  << " " << pos->z << " " << pos->value << endl;

    score1 = pos->value;

    if(flag_cmpt_ncc)
    {
        if(datatype_subject == 1)
        {
            success = cmpt_ncc_imagepair<unsigned char, Y_IMG_UINT8>(subject1d+offset_sub, szSub, target1d+offset_tar, szTar, pos);
        }
        else if(datatype_subject == 2)
        {
            success = cmpt_ncc_imagepair<unsigned short, Y_IMG_UINT16>((unsigned short *)subject1d+offset_sub, szSub, (unsigned short *)target1d+offset_tar, szTar, pos);
        }
        else if(datatype_subject == 4)
        {
            success = cmpt_ncc_imagepair<REAL, Y_IMG_REAL>((REAL *)subject1d+offset_sub, szSub, (REAL *)target1d+offset_tar, szTar, pos);
        }
        else
        {
            printf("Currently this program only support UINT8, UINT16, and FLOAT32 data type.\n");
            return -1;
        }

        if(success!=true) return false;

        score2 = pos->value;

        cout<< "score ... " << pos->value << endl;
    }

    //
    int end_t = clock();
    qDebug("time eclapse %d seconds for image stitching.", (end_t-start_t));

    //output
    //------------------------------------------------------------------------------------------------

    // save correlation score
    if(fn_output_cs)
    {
        FILE *pFileOut=0;

        pFileOut = fopen(fn_output_cs,"wt");
        if (pFileOut == NULL) perror ("Error opening writing file!");
        else
        {
            fprintf(pFileOut, "%f \n", score1);

            if(flag_cmpt_ncc)
            {
                fprintf(pFileOut, "%f \n", score2);
            }
        }
        fclose(pFileOut);
    }
    else
    {
        cout << "The output correlation score saving file is not specified."<<endl;
        return false;
    }

    // save output stitched image
    if(fn_stitched)
    {
        V3DLONG offset[3];
        offset[0] = pos_x - sx +1;
        offset[1] = pos_y - sy +1;
        offset[2] = pos_z - sz +1;

        V3DLONG new_sz0, new_sz1, new_sz2;

        new_sz0 = (offset[0]<0) ? (tx - offset[0]) : (sx + offset[0]);
        if (new_sz0<tx) new_sz0=tx;
        if (new_sz0<sx) new_sz0=sx;

        new_sz1 = (offset[1]<0) ? (ty - offset[1]) : (sy + offset[1]);
        if (new_sz1<ty) new_sz1=ty;
        if (new_sz1<sy) new_sz1=sy;

        new_sz2 = (offset[2]<0) ? (tz - offset[2]) : (sz + offset[2]);
        if (new_sz2<tz) new_sz2=tz;
        if (new_sz2<sz) new_sz2=sz;

        V3DLONG offset_tx, offset_ty, offset_tz, offset_sx, offset_sy, offset_sz;

        if(offset[0]<0)
        {
            offset_sx = 0; offset_tx = -offset[0];
        }
        else
        {
            offset_sx = offset[0]; offset_tx = 0;
        }
        if(offset[1]<0)
        {
            offset_sy = 0; offset_ty = -offset[1];
        }
        else
        {
            offset_sy = offset[1]; offset_ty = 0;
        }
        if(offset[2]<0)
        {
            offset_sz = 0; offset_tz = -offset[2];
        }
        else
        {
            offset_sz = offset[2]; offset_tz = 0;
        }

        V3DLONG pagesz_overlap = new_sz0*new_sz1*new_sz2;

        V3DLONG totalplxs = sc*new_sz0*new_sz1*new_sz2;
        unsigned char* data1d = new unsigned char[totalplxs];
        if (!data1d)
        {
            printf("Fail to allocate memory.\n");
            return -1;
        }
        for(V3DLONG i=0; i<totalplxs; i++)
            data1d[i] = 0;

        //outputs
        V3DLONG offset_data = new_sz0*new_sz1*new_sz2;
        V3DLONG offset_target = tx*ty*tz;
        V3DLONG offset_subject = sx*sy*sz;

        V3DLONG i_start, j_start, k_start;
        V3DLONG sz_start = offset_sz, sz_end = sz_start + sz; if(sz_start<0) k_start=0; else k_start=sz_start; if(sz_end>new_sz2) sz_end=new_sz2;
        V3DLONG sy_start = offset_sy, sy_end = sy_start + sy; if(sy_start<0) j_start=0; else j_start=sy_start; if(sy_end>new_sz1) sy_end=new_sz1;
        V3DLONG sx_start = offset_sx, sx_end = sx_start + sx; if(sx_start<0) i_start=0; else i_start=sx_start; if(sx_end>new_sz0) sx_end=new_sz0;

        //
        for(V3DLONG c=0; c<sc; c++)
        {
            V3DLONG offset_c = c*offset_data;
            V3DLONG offsets_c = c*offset_subject;
            for(V3DLONG k=k_start; k<sz_end; k++)
            {
                V3DLONG offset_k = offset_c + k*new_sz0*new_sz1;
                V3DLONG offsets_k = offsets_c + (k-k_start)*sx*sy; if (k-k_start >= sz) continue;
                for(V3DLONG j=j_start; j<sy_end; j++)
                {
                    V3DLONG offset_j = offset_k + j*new_sz0;
                    V3DLONG offsets_j = offsets_k + (j-j_start)*sx; if (j-j_start >= sy) continue;
                    for(V3DLONG i=i_start; i<sx_end; i++)
                    {
                        V3DLONG idx = offset_j + i;

                        data1d[idx] = subject1d[offsets_j + i - i_start];
                    }
                }
            }
        }

        V3DLONG tz_start = offset_tz, tz_end = offset_tz + tz; if(tz_start<0) tz_start=0; if(tz_end>new_sz2) tz_end=new_sz2;
        V3DLONG ty_start = offset_ty, ty_end = offset_ty + ty; if(ty_start<0) ty_start=0; if(ty_end>new_sz1) ty_end=new_sz1;
        V3DLONG tx_start = offset_tx, tx_end = offset_tx + tx;	if(tx_start<0) tx_start=0; if(tx_end>new_sz0) tx_end=new_sz0;

        for(V3DLONG c=0; c<sc; c++)
        {
            V3DLONG offset_c = c*offset_data;
            V3DLONG offsets_c = c*offset_target;
            for(V3DLONG k=tz_start; k<tz_end; k++)
            {
                V3DLONG offset_k = offset_c + k*new_sz0*new_sz1;
                V3DLONG offsets_k = offsets_c + (k-tz_start)*tx*ty;
                for(V3DLONG j=ty_start; j<ty_end; j++)
                {
                    V3DLONG offset_j = offset_k + j*new_sz0;
                    V3DLONG offsets_j = offsets_k + (j-ty_start)*tx;
                    for(V3DLONG i=tx_start; i<tx_end; i++)
                    {
                        V3DLONG idx = offset_j + i;

                        //					if(data1d[idx])
                        //					{
                        //						data1d[idx] = fmax(data1d[idx], (unsigned char) target1d[offsets_j + i - tx_start]); // Max. Intensity
                        //					}
                        //					else
                        //					{
                        //						data1d[idx] = (unsigned char) target1d[offsets_j + i - tx_start];
                        //					}

                        if(data1d[idx])
                        {
                            data1d[idx] = 0.5*(data1d[idx] + (unsigned char) target1d[offsets_j + i - tx_start]); // Avg. Intensity
                        }
                        else
                        {
                            data1d[idx] = (unsigned char) target1d[offsets_j + i - tx_start];
                        }

                    }
                }
            }
        }

        //saving
        V3DLONG sz_output[4];
        sz_output[0] = new_sz0; sz_output[1] = new_sz1; sz_output[2] = new_sz2; sz_output[3] = sc;

        saveImage(fn_stitched, (unsigned char *)data1d, sz_output, 1);

        // de-alloc
        if(data1d) {delete []data1d; data1d=NULL;}

    }
    else
    {
        cout << "The output stitched image file is not specified."<<endl;
        return false;
    }

    // de-alloc
    if(subject1d) {delete []subject1d; subject1d=NULL;}
    if(target1d) {delete []target1d; target1d=NULL;}
    if(sz_subject) {delete []sz_subject; sz_subject=NULL;}
    if(sz_target) {delete []sz_target; sz_target=NULL;}
    if(scale) {delete []scale; scale=0;}

    //
    return true;
}

// 
void printHelp_v3d_stitch_pair()
{
    cout<<endl<<"v3d_stitch_pair: a single pair image-stitching program developed by Yang Yu, Peng Lab, Janelia Farm Research Campus."<<endl;
    cout<<endl<<"Usage: v3d_stitch_pair -h/H [ -t <target> -C <channel_target> -s <subject> -c <channel_subject> -o <output_stitched_image> -u <correlation_score>] "<<endl;

    cout<<"    -h/H			help information."<<endl;

    cout<<"    -t <file>			target image."<<endl;
    cout<<"    -C <channel>		choose channel of target image to stitch."<<endl;

    cout<<"    -s <file>			subject image."<<endl;
    cout<<"    -c <channel>		choose channel of subject image to stitch."<<endl;

    cout<<"    -o <file>			save stitched image."<<endl;

    cout<<"    -u <file>			save correlation score to a txt file."<<endl<<endl;
    cout<<"    -n <0/1>			compute normalized cross correlation."<<endl<<endl;

    return;
}


#ifndef __ISTITCH_FUNC_CALL__
// 
int main(int argc, char **argv)
{
    //
    char *fn_subject=NULL, *fn_target=NULL, *fn_stitched=NULL, *fn_output_cs=NULL;
    V3DLONG ch_tar = 0, ch_sub = 0;
    bool flag_cmpt_ncc=false;

    // ------ parsing aguements here ---------------------
    if (argc<=1)
    {
        printHelp_v3d_stitch_pair();
        return true;
    }
    else
    {
        //
        if(argc<=2)
        {
            if(string(argv[1]) == "-h" || string(argv[1]) == "-H")
            {
                printHelp_v3d_stitch_pair();
                return true;
            }
            else
            {
                cout<<"Your module code is illegal. Please follow the instruction of the help page below."<<endl;
                printHelp_v3d_stitch_pair();
                return false;
            }
        }
        else
        {
            // if there is -h/H, V3D only print help info and return
            for(int i=1; i<argc; i++)
            {
                if(string(argv[i]) == "-h" || string(argv[i]) == "-H")
                {
                    printHelp_v3d_stitch_pair();
                    return true;
                }
            }

            // parsing arguments in other cases
            for(int i=1; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    if(string(argv[i]) == "-t")
                    {
                        //
                        fn_target = argv[i+1];
                        i++;
                        cout << "-t "<<fn_target<<endl;
                    }
                    else if(string(argv[i]) == "-C")
                    {
                        //
                        ch_tar = V3DLONG( atoi(argv[i+1]) );
                        i++;
                        cout << "-C "<<ch_tar<<endl;
                    }
                    else if(string(argv[i]) == "-s")
                    {
                        //
                        fn_subject = argv[i+1];
                        i++;
                        cout << "-s "<<fn_subject<<endl;
                    }
                    else if(string(argv[i]) == "-c")
                    {
                        //
                        ch_sub = V3DLONG( atoi(argv[i+1]) );
                        i++;
                        cout << "-c "<<ch_sub<<endl;
                    }
                    else if(string(argv[i]) == "-o")
                    {
                        //
                        fn_stitched = argv[i+1];
                        i++;
                        cout << "-o "<<fn_stitched<<endl;
                    }
                    else if(string(argv[i]) == "-u")
                    {
                        //
                        fn_output_cs = argv[i+1];
                        i++;
                        cout << "-u "<<fn_output_cs<<endl;
                    }
                    else if(string(argv[i]) == "-n")
                    {
                        //
                        flag_cmpt_ncc = bool( atoi(argv[i+1]) );
                        i++;
                        cout << "-u "<<fn_output_cs<<endl;
                    }
                    else
                    {
                        cout<<"Your module code is illegal. Please follow the instruction of the help page below."<<endl;
                        printHelp_v3d_stitch_pair();
                        return false;
                    }
                }

            }


        }
    }

    //
    if(fn_subject == NULL || fn_target == NULL || fn_output_cs == NULL)
    {
        cout<<"Your module code is illegal. Please follow the instruction of the help page below."<<endl;
        printHelp_v3d_stitch_pair();
        return false;
    }

    //
    PEAKS *pos= new PEAKS;
    return pairwise_stitching(fn_target, ch_tar, fn_subject, ch_sub, fn_stitched, fn_output_cs, flag_cmpt_ncc, pos);

}

#endif


#else

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(imageStitch, IStitchPlugin);

// stitching function
template <class SDATATYPE, class Y_IMG_DATATYPE> int stitching_bb_thickplanes(SDATATYPE *subject1d, V3DLONG *sz_subject, SDATATYPE *target1d, V3DLONG *sz_target, REAL overlap_percent, bool m_similarity, REAL *scale, PEAKS *pos, bool ms, unsigned char groupisflag, StitchDataType dt);

// pairwise stitching function
int pairwise_stitching(V3DPluginCallback2 &callback, QWidget *parent);

// group image stitching wrapper
int group_stitching_wrap(V3DPluginCallback2 &callback, QWidget *parent);

// global stitching function without configuration
int group_stitching(V3DPluginCallback2 &callback, QWidget *parent);

//batch global stitching function without configuration
int batch_group_stitching(V3DPluginCallback2 &callback, QWidget *parent);

// global stitching function with configuration
int group_stitching_wc(V3DPluginCallback2 &callback, QWidget *parent);

// point navigating function
int point_navigating(V3DPluginCallback2 &callback, QWidget *parent);

// region navigating function
int region_navigating(V3DPluginCallback2 &callback, QWidget *parent);

//batch region navigating
int batch_region_navigating(V3DPluginCallback2 &callback, QWidget *parent);

// 3d roi navigating function
int roi_navigating(V3DPluginCallback2 &callback, QWidget *parent);

// pairwise image blending function
template <class SDATATYPE> int pwi_fusing(SDATATYPE *p_mask, SDATATYPE *data1d, SDATATYPE *subject1d, V3DLONG *sz_subject, SDATATYPE *target1d, V3DLONG *sz_target, V3DLONG *offset, bool axes_show, QString m_InputFileName, V3DLONG new_sz0, V3DLONG new_sz1, V3DLONG new_sz2, V3DLONG pagesz_overlap, V3DLONG sub_c, V3DLONG tar_c);

// pairwise image blending function
template <class SDATATYPE> int groupi_fusing(SDATATYPE *pVImg, Y_VIM<REAL, V3DLONG, indexed_t<V3DLONG, REAL>, LUT<V3DLONG> > vim, V3DLONG vx, V3DLONG vy, V3DLONG vz, V3DLONG vc, bool axes_show);

// subspace stitching and linear blending tiles
template <class Tdata, class Y_IMG_DATATYPE> bool iSubspaceStitching(Tdata *pVImg, Y_VIM<REAL, V3DLONG, indexed_t<V3DLONG, REAL>, LUT<V3DLONG> > vim, Tdata intensityrange, V3DLONG channel, bool flagFusion);

// open tutorial and download test data
void OpenDownloadPage(QWidget *parent);

//plugin funcs
const QString title = "Image Stitching";
QStringList IStitchPlugin::menulist() const
{
    return QStringList() << tr("Pairwise Image Stitching")
                         << tr("Group Image Stitching")
                         << tr("batch Group Image Stitching")
                         << tr("batch generate rawdata from tc file")
                         << tr("Check voxel intensity at a XYZ location")
                         << tr("Region Navigating")
                         << tr("open test data web page")
                         << tr("HIDDEN_3DROI_Navigation")
                         << tr("About");
}

void IStitchPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("Pairwise Image Stitching"))
    {
    	pairwise_stitching(callback, parent);
    }
    else if (menu_name == tr("Group Image Stitching"))
    {
        group_stitching_wrap(callback, parent);
    }
    else if(menu_name == tr("batch Group Image Stitching"))
    {
        batch_group_stitching(callback, parent);
    }
    else if(menu_name == tr("batch generate rawdata from tc file"))
    {
        batch_region_navigating(callback, parent);
    }
    else if (menu_name == tr("Check voxel intensity at a XYZ location"))
    {
        point_navigating(callback, parent);
    }
    else if (menu_name == tr("Region Navigating"))
    {
        region_navigating(callback, parent);
    }
    else if(menu_name==tr("open test data web page"))
    {
        OpenDownloadPage(parent);
    }
    else if (menu_name == tr("HIDDEN_3DROI_Navigation"))
    {
        roi_navigating(callback, parent);
    }
    else if (menu_name == tr("About"))
    {
        QMessageBox::information(parent, "Version info", QString("ImageStitching Plugin %1 (March 1, 2010) developed by Yang Yu. (Peng Lab, Janelia Research Farm Campus, HHMI)").arg(getPluginVersion()).append("\n"
                                                                                                                                                                                                                 "\n Tiles Configuration File Format (.tc)"
                                                                                                                                                                                                                 "\n # thumbnail file "
                                                                                                                                                                                                                 "\n [name of thumbnail file]"
                                                                                                                                                                                                                 "\n # tiles "
                                                                                                                                                                                                                 "\n [how many tiles]"
                                                                                                                                                                                                                 "\n # dimensions (XYZC) "
                                                                                                                                                                                                                 "\n [dimX dimY dimZ dimC]"
                                                                                                                                                                                                                 "\n # origin (XYZ) "
                                                                                                                                                                                                                 "\n [physics_start_position_x physics_start_position_y physics_start_position_z]"
                                                                                                                                                                                                                 "\n # resolution (XYZ) "
                                                                                                                                                                                                                 "\n [X Y Z]"
                                                                                                                                                                                                                 "\n # image coordinates look up table "
                                                                                                                                                                                                                 "\n [tile_path_name (offset2ref_s_x, offset2ref_s_y, offset2ref_s_z) (offset2ref_e_x, offset2ref_e_y, offset2ref_e_z)]"));
        return;
    }
}

// open tutorial and download test data
void OpenDownloadPage(QWidget *parent)
{
    bool b_openurl_worked;
    b_openurl_worked=QDesktopServices::openUrl(QUrl("http://penglab.janelia.org/proj/stitching/supp/supp_index.htm", QUrl::TolerantMode));
    if (! b_openurl_worked)
        QMessageBox::warning(parent,
                             "Error opening download page", // title
                             "Please browse to\n"
                             "http://penglab.janelia.org/proj/stitching/supp/supp_index.htm\n"
                             "to download the test data for this plugin");

}

// function call
QStringList IStitchPlugin::funclist() const
{
    return QStringList() << tr("v3dstitch")
                         << tr("istitch-subspace")
                         << tr("istitch-gc");
}

// Multiscale pairwise images stitching with thick planes (e.g. 10 pixels width) in boundary bounding box
template <class SDATATYPE, class Y_IMG_DATATYPE>
int stitching_bb_thickplanes(SDATATYPE *subject1d, V3DLONG *sz_subject1d, SDATATYPE *target1d, V3DLONG *sz_target1d, REAL overlap_percent, bool m_similarity, REAL *scale, PEAKS *&pos, unsigned char groupisflag)
{
    //groupisflag is for group image stitching flag.
    //0: pairwise image stitching
    //1: return the score and shift offsets of downsampled images
    //2: rerurn the final score and shift offsets

    //
    V3DLONG sx = sz_subject1d[0];
    V3DLONG sy = sz_subject1d[1];
    V3DLONG sz = sz_subject1d[2]; 
    V3DLONG sc = sz_subject1d[3];

    V3DLONG pagesz_sub = sx*sy*sz;

    V3DLONG tx = sz_target1d[0];
    V3DLONG ty = sz_target1d[1];
    V3DLONG tz = sz_target1d[2];
    V3DLONG tc = sz_target1d[3];

    V3DLONG pagesz_tar = tx*ty*tz;

    size_t start_t = clock();

    REAL threshold_corr_score = 0; //0.75; //

    // sampling by factors less than 1 (downsample)
    // obtain an approximate translative estimation at a coarse scale
    //--------------------------------------------------------------------------------------
    V3DLONG ssx, ssy, ssz, stx, sty, stz;

    V3DLONG sx_ori = sx;
    V3DLONG sy_ori = sy;
    V3DLONG sz_ori = sz;

    V3DLONG tx_ori = tx;
    V3DLONG ty_ori = ty;
    V3DLONG tz_ori = tz;

    // anchor position
    REAL pos_score = 0;
    V3DLONG pos_x=0, pos_y=0, pos_z=0;

    V3DLONG *szPad = new V3DLONG [3];
    V3DLONG *szTar = new V3DLONG [3];
    V3DLONG *szSub = new V3DLONG [3];

    size_t start_t_2;

    if(groupisflag!=2)
    {

        ssx = (V3DLONG)(ceil(sx*scale[0]));
        ssy = (V3DLONG)(ceil(sy*scale[1]));
        ssz = (V3DLONG)(ceil(sz*scale[2]));

        stx = (V3DLONG)(ceil(tx*scale[0]));
        sty = (V3DLONG)(ceil(ty*scale[1]));
        stz = (V3DLONG)(ceil(tz*scale[2]));

        SDATATYPE *p_sub, *p_tar;

        if(scale[0]==1 && scale[1]==1 && scale[2]==1)
        {
            p_sub = subject1d;
            p_tar = target1d;
        }
        else
        {
            //
            V3DLONG pagesz_ds_sub = ssx*ssy*ssz;
            V3DLONG pagesz_ds_tar = stx*sty*stz;

            V3DLONG *sz_down_target = new V3DLONG [3];

            sz_down_target[0] = stx;
            sz_down_target[1] = sty;
            sz_down_target[2] = stz;

            V3DLONG *sz_down_subject = new V3DLONG [3];

            sz_down_subject[0] = ssx;
            sz_down_subject[1] = ssy;
            sz_down_subject[2] = ssz;

            p_sub = new SDATATYPE [pagesz_ds_sub];
            if (!p_sub)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }
            else
            {
                // down sampling
                Y_IMG_DATATYPE pIn(subject1d, sz_subject1d);
                Y_IMG_DATATYPE pOut(p_sub, sz_down_subject);

                YImg<REAL, V3DLONG, Y_IMG_DATATYPE, Y_IMG_DATATYPE> tmp;
                tmp.down_sampling(pOut, pIn, scale);
            }

            p_tar = new SDATATYPE [pagesz_ds_tar];
            if (!p_tar)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }
            else
            {
                // down sampling
                Y_IMG_DATATYPE pIn(target1d, sz_target1d);
                Y_IMG_DATATYPE pOut(p_tar, sz_down_target);

                YImg<REAL, V3DLONG, Y_IMG_DATATYPE, Y_IMG_DATATYPE> tmp;
                tmp.down_sampling(pOut, pIn, scale);
            }
        }


        sx = ssx; sy = ssy; sz = ssz;
        tx = stx; ty = sty; tz = stz;

        // 3D images stitching
        //--------------------------------------------------------------------------------------

        // declaration
        V3DLONG sx_pad = sx+tx-1, sy_pad = sy+ty-1, sz_pad = sz+tz-1;

        V3DLONG even_odd = sx_pad%2; // 0 for even 1 for odd

        bool fftw_in_place = true;

        if(fftw_in_place)
            sx_pad += (2-even_odd); //2*(sx_pad/2 + 1); // fftw_in_place

        V3DLONG len_pad = sx_pad*sy_pad*sz_pad;

        //
        szPad[0] = sx_pad;
        szPad[1] = sy_pad;
        szPad[2] = sz_pad;

        //
        szTar[0] = tx;
        szTar[1] = ty;
        szTar[2] = tz;

        //
        szSub[0] = sx;
        szSub[1] = sy;
        szSub[2] = sz;

        // padding target with zeros
        REAL* pTar = new REAL [len_pad];
        if (!pTar)
        {
            printf("Fail to allocate memory.\n");
            return -1;
        }
        else
        {
            for(V3DLONG i=0; i<len_pad; i++)
                pTar[i]=0;

            //padding zeros for target image
            Y_IMG_REAL pOut(pTar, szPad);
            Y_IMG_DATATYPE pIn(p_tar, szTar);

            YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
            tmp.padding(pOut, pIn, false, fftw_in_place, even_odd, 3);
            //tmp.padding_mirror_3D(pOut, pIn, false, fftw_in_place, even_odd);

        }

        // padding subject with zeros
        REAL* pSub = new REAL [len_pad];
        if (!pSub)
        {
            printf("Fail to allocate memory.\n");
            return -1;
        }
        else
        {
            for(V3DLONG i=0; i<len_pad; i++)
                pSub[i]=0;

            //padding zeros for subject image
            Y_IMG_REAL pOut(pSub, szPad);
            Y_IMG_DATATYPE pIn(p_sub, szSub);

            YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
            tmp.padding(pOut, pIn, true, fftw_in_place, even_odd, 3);
            //tmp.padding_mirror_3D(pOut, pIn, true, fftw_in_place, even_odd);

        }

        //ncc || pc-ncc
        if(m_similarity==true)
        {

            Y_IMG_REAL pOut(pSub, szPad);
            Y_IMG_REAL pIn(pTar, szPad);

            YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_REAL> tmp;
            tmp.fftncc(pOut, pIn, szTar, even_odd, fftw_in_place, overlap_percent, 3); // fftw_in_place

            //find the highest response of FFT CC (pSub)
            V3DLONG sx_pad_ori = sx_pad - (2-even_odd);

            for(V3DLONG k=0; k<sz_pad; k++)
            {
                V3DLONG offset_k = k*sx_pad*sy_pad;
                for(V3DLONG j=0; j<sy_pad; j++)
                {
                    V3DLONG offset_j = offset_k + j*sx_pad;
                    for(V3DLONG i=0; i<sx_pad_ori; i++)	// fftw_in_place
                    {
                        V3DLONG idx = offset_j + i;

                        REAL score = pSub[idx];

                        //obtain the highest response
                        if(score>pos_score)
                        {
                            //qDebug("score %lf", score);
                            pos_score = score;
                            pos_x = i; pos_y = j; pos_z = k;
                        }

                    }
                }
            }

        }
        else
        {
            PEAKS pos_pcncc;
            Y_IMG_REAL pOut(pSub, szPad);
            Y_IMG_REAL pIn(pTar, szPad);

            YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_REAL> tmp;
            tmp.fftpcncc3D(pOut, pIn, szTar, even_odd, fftw_in_place, overlap_percent, &pos_pcncc);

            pos_x = pos_pcncc.x; pos_y = pos_pcncc.y; pos_z = pos_pcncc.z; pos_score = pos_pcncc.value;
        }

        qDebug() << "score ..." << pos_score;

        if(pos_score == -1)
            return -1;

        // coarse-scale done
        pos_x /= scale[0]; pos_y /= scale[1]; pos_z /= scale[2];

        //de-alloc
        if(pTar) {delete []pTar; pTar=0;}
        if(pSub) {delete []pSub; pSub=0;}

        //
        qDebug() << " ds us offsets ..." << pos_x << pos_y << pos_z << "response ..." << pos_score;

        //
        start_t_2 = clock();

        qDebug() << "ds time ..." << start_t_2 - start_t;

        if(pos_score<threshold_corr_score)
            return true;

        // group case
        if(groupisflag==1)
        {
            pos->x = pos_x; pos->y = pos_y; pos->z = pos_z; pos->value = pos_score;

            return true;
        }

    }


    // step 2 --------------------------------------------------------------------------------------------
    // choose a thick planes from boundary bounding box from sub image then register it with tar image boundary bounding box
    // at a fine scale
    if(!(scale[0]==1 && scale[1]==1 && scale[2]==1))
    {
        sx = sx_ori; sy = sy_ori; sz = sz_ori;
        tx = tx_ori; ty = ty_ori; tz = tz_ori;

        //
        if(groupisflag==2)
        {
            pos_x = pos->x; pos_y = pos->y; pos_z = pos->z; pos_score = pos->value;
        }

        //
        V3DLONG offset[3];
        offset[0] = pos_x - sx +1;
        offset[1] = pos_y - sy +1;
        offset[2] = pos_z - sz +1;

        qDebug() << "offset ..." << offset[0] << offset[1] << offset[2];

        V3DLONG nbbx, nbby, nbbz;

        nbbx = 1/scale[0] + 2;
        nbby = 1/scale[1] + 2;
        nbbz = 1/scale[2] + 2;

        V3DLONG bsx = (offset[0]>0)?0:-offset[0];
        V3DLONG bsy = (offset[1]>0)?0:-offset[1];
        V3DLONG bsz = (offset[2]>0)?0:-offset[2];

        if(bsx-nbbx>=0)
            bsx -= nbbx;
        else
            bsx = 0;

        if(bsy-nbby>=0)
            bsy -= nbby;
        else
            bsy = 0;

        if(bsz-nbbz>=0)
            bsz -= nbbz;
        else
            bsz = 0;

        V3DLONG esx = (sx-1);
        V3DLONG esy = (sy-1);
        V3DLONG esz = (sz-1);

        if(offset[0])
        {
            V3DLONG tmp = sx-1 + offset[0] - (tx-1);

            if(tmp>0)
                esx = sx-1 -tmp;
        }

        if(offset[1])
        {
            V3DLONG tmp = offset[1]+sy-1 - (ty-1);

            if(tmp>0)
                esy = sy-1 - tmp;
        }

        if(offset[2])
        {
            V3DLONG tmp = offset[2]+sz-1 - (tz-1);

            if(tmp>0)
                esz = sz-1 -tmp;
        }

        if(esx+nbbx<=sx-1)
            esx += nbbx;
        else
            esx = sx-1;

        if(esy+nbby<=sy-1)
            esy += nbby;
        else
            esy = sy-1;

        if(esz+nbbz<=sz-1)
            esz += nbbz;
        else
            esz = sz-1;

        V3DLONG btx = (offset[0]>0)?offset[0]:0;
        V3DLONG bty = (offset[1]>0)?offset[1]:0;
        V3DLONG btz = (offset[2]>0)?offset[2]:0;

        if(btx-nbbx>=0)
            btx -= nbbx;
        else
            btx = 0;

        if(bty-nbby>=0)
            bty -= nbby;
        else
            bty = 0;

        if(btz-nbbz>=0)
            btz -= nbbz;
        else
            btz = 0;

        V3DLONG etx = (offset[0]>0)?(tx-1):pos_x;
        V3DLONG ety = (offset[1]>0)?(ty-1):pos_y;
        V3DLONG etz = (offset[2]>0)?(tz-1):pos_z;

        if(offset[0]>0)
        {
            V3DLONG tmp = (tx-1) - (sx-1 + offset[0]);

            if(tmp>0)
                etx = tx-1 - tmp;
        }

        if(offset[1]>0)
        {
            V3DLONG tmp = (ty-1) - (sy-1 + offset[1]);

            if(tmp>0)
                ety = ty-1 - tmp;
        }

        if(offset[2]>0)
        {
            V3DLONG tmp = (tz-1) - (sz-1 + offset[2]);

            if(tmp>0)
                etz = tz-1 - tmp;
        }


        if(etx+nbbx<=tx-1)
            etx += nbbx;
        else
            etx = tx-1;

        if(ety+nbby<=ty-1)
            ety += nbby;
        else
            ety = ty-1;

        if(etz+nbbz<=tz-1)
            etz += nbbz;
        else
            etz = tz-1;

        V3DLONG bbsx = esx - bsx + 1; if(bbsx<0 || bbsx>sx) return -1;
        V3DLONG bbsy = esy - bsy + 1; if(bbsy<0 || bbsy>sy) return -1;
        V3DLONG bbsz = esz - bsz + 1; if(bbsz<0 || bbsz>sz) return -1;

        V3DLONG bbtx = etx - btx + 1; if(bbtx<0 || bbtx>tx) return -1;
        V3DLONG bbty = ety - bty + 1; if(bbty<0 || bbty>ty) return -1;
        V3DLONG bbtz = etz - btz + 1; if(bbtz<0 || bbtz>tz) return -1;

        qDebug() << " testing ... boundary ... "<< bsx << bsy << bsz << esx << esy << esz << btx << bty << btz << etx << ety << etz;
        qDebug() << " testing ... dims ... "<< bbsx << bbsy << bbsz << bbtx << bbty << bbtz;
        qDebug() << "...";

        float rate_x, rate_y, rate_z;

        rate_x = qMax( (float)bbsx/(float)sx, (float)bbtx/(float)tx);
        rate_y = qMax( (float)bbsy/(float)sy, (float)bbty/(float)ty);
        rate_z = qMax( (float)bbsz/(float)sz, (float)bbtz/(float)tz);

        // crop options
        // planes
        bool plane_yz=false, plane_xz=false, plane_xy=false;

        if(rate_x < rate_y && rate_x < rate_z)
        {
            plane_yz = true;

            bsy = 0; esy = sy-1;
            bbsy = sy;

            bty = 0; ety = ty-1;
            bbty = ty;

            bsz = 0; esz = sz-1;
            bbsz = sz;

            btz = 0; etz = tz-1;
            bbtz = tz;

        }
        if(rate_y < rate_x && rate_y < rate_z)
        {
            plane_xz = true;

            bsx = 0; esx = sx-1;
            bbsx = sx;

            btx = 0; etx = tx-1;
            bbtx = tx;

            bsz = 0; esz = sz-1;
            bbsz = sz;

            btz = 0; etz = tz-1;
            bbtz = tz;

        }
        if(rate_z < rate_x && rate_z < rate_y)
        {
            plane_xy = true;

            bsx = 0; esx = sx-1;
            bbsx = sx;

            btx = 0; etx = tx-1;
            bbtx = tx;

            bsy = 0; esy = sy-1;
            bbsy = sy;

            bty = 0; ety = ty-1;
            bbty = ty;

        }

        //
        qDebug() << "rate ..." << rate_x << rate_y << rate_z << "planes ..." << plane_yz << plane_xz << plane_xy;
        qDebug() << " current bounding box ... boundary ... "<< bsx << bsy << bsz << esx << esy << esz << btx << bty << btz << etx << ety << etz;
        qDebug() << " current bounding box ... dims ... "<< bbsx << bbsy << bbsz << bbtx << bbty << bbtz;

        //
        REAL sum=0;
        for(V3DLONG k=bsz; k<=esz; k++)
        {
            V3DLONG offset_o_k = k*sx*sy;
            for(V3DLONG j=bsy; j<=esy; j++)
            {
                V3DLONG offset_o_j = offset_o_k + j*sx;
                for(V3DLONG i=bsx; i<=esx; i++)
                {
                    V3DLONG idx_o = offset_o_j + i;

                    sum += subject1d[idx_o];
                }
            }
        }
        SDATATYPE meanv = (SDATATYPE) (sum/(bbsx*bbsy*bbsz));

        // extract thick planes boundary bounding box
        //---------------------------------------------------------------------------------------------------------------
        if(plane_yz == true)
        {
            // finding rich information plane from sub aV3DLONG x
            V3DLONG info_count=0, xpln=0, max_info=0;

            // approach 1
            V3DLONG weights = bbsx*0.15;
            weights /= 2;
            V3DLONG start_x=bsx+weights+nbbx, end_x=esx-weights-nbbx;
            for(V3DLONG i=start_x; i<=end_x; i++) //
            {
                info_count = 0;

                for(V3DLONG k=bsz; k<=esz; k++)
                {
                    V3DLONG offset_o_k = k*sx*sy;
                    for(V3DLONG j=bsy; j<=esy; j++)
                    {
                        V3DLONG idx_o = offset_o_k + j*sx + i;

                        if( subject1d[idx_o] > meanv)
                            info_count++;
                    }
                }

                if(info_count > max_info)
                {
                    max_info = info_count;
                    xpln = i;
                }

            }

            // approach 2
            //			V3DLONG weights = bbsx*0.15;
            //			weights /= 2;
            //			V3DLONG length = 2*weights + 1;
            //
            //			V3DLONG start_x=bsx+weights, end_x=esx-weights;
            //			V3DLONG len_a = end_x-start_x+1;
            //
            //			if(len_a<0) len_a=1;
            //
            //			V3DLONG *a = 0;
            //			try
            //			{
            //				a = new V3DLONG [len_a];
            //			}
            //			catch (...)
            //			{
            //				printf("Fail to allocate memory.\n");
            //				return -1;
            //			}
            //
            //			for(V3DLONG i=start_x; i<=end_x; i++) //
            //			{
            //				info_count = 0;
            //
            //				for(V3DLONG k=bsz; k<=esz; k++)
            //				{
            //					V3DLONG offset_o_k = k*sx*sy;
            //					for(V3DLONG j=bsy; j<=esy; j++)
            //					{
            //						V3DLONG idx_o = offset_o_k + j*sx + i;
            //
            //						if( subject1d[idx_o] > meanv)
            //							info_count++;
            //					}
            //				}
            //
            //				a[i-start_x] = info_count;
            //
            //			}
            //
            //			len_a -= length-1;
            //			PL p;
            //
            //			if(len_a<0) len_a=1;
            //
            //			V3DLONG *b = 0;
            //			try
            //			{
            //				b = new V3DLONG [len_a];
            //			}
            //			catch (...)
            //			{
            //				printf("Fail to allocate memory.\n");
            //				return -1;
            //			}
            //
            //			b[0] = a[0];
            //			for(V3DLONG i=1; i<length; i++)
            //				b[0] += a[i];
            //
            //			p.pln = start_x;
            //			p.count = b[0];
            //			for(V3DLONG i=1; i<len_a; i++)
            //			{
            //				b[i] = b[i-1] - a[i-1] + a[i+length-1];
            //
            //				if(b[i]>p.count)
            //				{
            //					p.pln = start_x + i;
            //					p.count = b[i];
            //				}
            //			}
            //
            //			//de-alloc
            //			if(a) {delete []a; a=0;}
            //			if(b) {delete []b; b=0;}
            //
            //			//
            //			xpln = p.pln;
            //
            if(xpln<start_x) xpln = start_x;

            qDebug() << "xpln ..." << xpln;

            // extraction
            V3DLONG b_bsx = xpln - weights; //

            if(b_bsx>bsx)
                bsx = b_bsx;

            V3DLONG e_esx = xpln + weights;

            if(e_esx<esx)
                esx = e_esx;

            bbsx = esx-bsx+1; // dims

            // crop corresponding thick planes from tar image
            V3DLONG b_btx = bsx + offset[0] - nbbx;

            if(b_btx>btx)
                btx = b_btx;

            V3DLONG e_etx = esx + offset[0] + nbbx;

            if(e_etx<etx)
                etx = e_etx;

            bbtx = etx-btx+1;

            qDebug() << " updated boundary ... "<< bsx << bsy << bsz << esx << esy << esz << btx << bty << btz << etx << ety << etz;
            qDebug() << " updated dims ... "<< bbsx << bbsy << bbsz << bbtx << bbty << bbtz;

            V3DLONG pagesz_bb_sub = bbsx*bbsy*bbsz;
            V3DLONG pagesz_bb_tar = bbtx*bbty*bbtz;

            // extract one plane from sub
            SDATATYPE* p_sub = new SDATATYPE [pagesz_bb_sub];
            if (!p_sub)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }
            else
            {
                for(V3DLONG k=bsz; k<=esz; k++)
                {
                    V3DLONG offset_k = (k-bsz)*bbsx*bbsy;
                    V3DLONG offset_o_k = k*sx*sy;
                    for(V3DLONG j=bsy; j<=esy; j++)
                    {
                        V3DLONG offset_j = offset_k + (j-bsy)*bbsx;
                        V3DLONG offset_o_j = offset_o_k + j*sx;
                        for(V3DLONG i=bsx; i<=esx; i++)
                        {
                            V3DLONG idx = offset_j + (i-bsx);
                            V3DLONG idx_o = offset_o_j + i;

                            p_sub[idx] = subject1d[idx_o];
                        }
                    }
                }
            }

            SDATATYPE* p_tar = new SDATATYPE [pagesz_bb_tar];
            if (!p_tar)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }


            //REAL max_response = 0;
            // search the best match plane from tar
            for(V3DLONG k=btz; k<=etz; k++)
            {
                V3DLONG offset_k = (k-btz)*bbtx*bbty;
                V3DLONG offset_o_k = k*tx*ty;
                for(V3DLONG j=bty; j<=ety; j++)
                {
                    V3DLONG offset_j = offset_k + (j-bty)*bbtx;
                    V3DLONG offset_o_j = offset_o_k + j*tx;
                    for(V3DLONG i=btx; i<=etx; i++)
                    {
                        V3DLONG idx = offset_j + (i-btx);
                        V3DLONG idx_o = offset_o_j + i;

                        p_tar[idx] = target1d[idx_o];
                    }
                }
            }

            //
            sx = bbsx; sy = bbsy; sz = bbsz;
            tx = bbtx; ty = bbty; tz = bbtz;

            V3DLONG sx_pad = sx+tx-1, sy_pad = sy+ty-1, sz_pad = sz+tz-1;

            V3DLONG even_odd = sx_pad%2; // 0 for even 1 for odd

            bool fftw_in_place = true;

            if(fftw_in_place)
                sx_pad += (2-even_odd); //2*(sx_pad/2 + 1); // fftw_in_place

            V3DLONG len_pad = sx_pad*sy_pad*sz_pad;

            szPad[0] = sx_pad;
            szPad[1] = sy_pad;
            szPad[2] = sz_pad;

            szTar[0] = tx;
            szTar[1] = ty;
            szTar[2] = tz;

            szSub[0] = sx;
            szSub[1] = sy;
            szSub[2] = sz;

            REAL* p_f_sub = new REAL [len_pad];
            if (!p_f_sub)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }
            else
            {
                // init
                memset(p_f_sub, 0, sizeof(REAL)*len_pad);

                //padding zeros for target imag
                Y_IMG_REAL pOut(p_f_sub, szPad);
                Y_IMG_DATATYPE pIn(p_sub, szSub);

                YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
                tmp.padding(pOut, pIn, true, fftw_in_place, even_odd, 3);
                //tmp.padding_mirror_3D(pOut, pIn, true, fftw_in_place, even_odd);

            }

            REAL* p_f_tar = new REAL [len_pad];
            if (!p_f_tar)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }
            else
            {
                // init
                memset(p_f_tar, 0, sizeof(REAL)*len_pad);

                //padding zeros for target imag
                Y_IMG_REAL pOut(p_f_tar, szPad);
                Y_IMG_DATATYPE pIn(p_tar, szTar);

                YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
                tmp.padding(pOut, pIn, false, fftw_in_place, even_odd, 3);
                //tmp.padding_mirror_3D(pOut, pIn, false, fftw_in_place, even_odd);

            }

            //de-alloc
            if(p_sub) {delete []p_sub; p_sub=0;}
            if(p_tar) {delete []p_tar; p_tar=0;}


            // fft-ncc
            PEAKS pos_pcncc;

            pos_pcncc.x = (bsx - ((sx_ori-1) - pos_x)) - btx + (bbsx-1);
            pos_pcncc.y = pos_y;
            pos_pcncc.z = pos_z;

            Y_IMG_REAL pOut(p_f_sub, szPad);
            Y_IMG_REAL pIn(p_f_tar, szPad);

            YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_REAL> tmp;
            tmp.fftnccp3D(pOut, pIn, szSub, szTar, even_odd, fftw_in_place, scale, &pos_pcncc);

            //pos_score = pos_pcncc.value;

            qDebug() << " bb score ..." << pos_score << "offset ..." << bsx << btx << pos_pcncc.x << bbsx;

            pos_x = (sx_ori-1) - (bsx - (btx + pos_pcncc.x - (bbsx-1))); pos_y = pos_pcncc.y; pos_z = pos_pcncc.z; //

            //de-alloc
            if(p_f_sub) {delete []p_f_sub; p_f_sub=0;}
            if(p_f_tar) {delete []p_f_tar; p_f_tar=0;}


        }
        else if(plane_xz == true)
        {
            // finding rich information plane from sub aV3DLONG y
            V3DLONG info_count=0, ypln, max_info=0;

            V3DLONG weights = bbsy*0.15;
            weights /= 2;
            V3DLONG start_y=bsy+weights+nbby, end_y=esy-weights-nbby;
            for(V3DLONG j=start_y; j<=end_y; j++) //
            {
                info_count = 0;

                V3DLONG offset_o_j = j*sx;
                for(V3DLONG k=bsz; k<=esz; k++)
                {
                    V3DLONG offset_o_k = k*sx*sy;
                    for(V3DLONG i=bsx; i<=esx; i++)
                    {
                        V3DLONG idx_o = offset_o_k + offset_o_j + i;

                        if( subject1d[idx_o] > meanv)
                            info_count++;
                    }
                }

                if(info_count > max_info)
                {
                    max_info = info_count;
                    ypln = j;
                }

            }

            qDebug() << "plane ..." << ypln;

            // extraction
            V3DLONG b_bsy = ypln - weights;

            if(b_bsy>bsy)
                bsy = b_bsy;

            V3DLONG e_esy = ypln + weights;

            if(e_esy<esy)
                esy = e_esy;

            bbsy = esy-bsy+1; // dims

            // crop corresponding thick planes from tar image
            V3DLONG b_bty = bsy + offset[1] - nbby; //ypln + offset[1] - nbby;

            if(b_bty>bty)
                bty = b_bty;

            V3DLONG e_ety = esy + offset[1] + nbby; //ypln + offset[1] + nbby;

            if(e_ety<ety)
                ety = e_ety;

            bbty = ety-bty+1;

            qDebug() << " updated boundary ... "<< bsx << bsy << bsz << esx << esy << esz << btx << bty << btz << etx << ety << etz;
            qDebug() << " updated dims ... "<< bbsx << bbsy << bbsz << bbtx << bbty << bbtz;

            V3DLONG pagesz_bb_sub = bbsx*bbsy*bbsz;
            V3DLONG pagesz_bb_tar = bbtx*bbty*bbtz;

            // extract one plane from sub
            SDATATYPE* p_sub = new SDATATYPE [pagesz_bb_sub];
            if (!p_sub)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }
            else
            {
                for(V3DLONG k=bsz; k<=esz; k++)
                {
                    V3DLONG offset_k = (k-bsz)*bbsx*bbsy;
                    V3DLONG offset_o_k = k*sx*sy;
                    for(V3DLONG j=bsy; j<=esy; j++)
                    {
                        V3DLONG offset_j = offset_k + (j-bsy)*bbsx;
                        V3DLONG offset_o_j = offset_o_k + j*sx;
                        for(V3DLONG i=bsx; i<=esx; i++)
                        {
                            V3DLONG idx = offset_j + (i-bsx);
                            V3DLONG idx_o = offset_o_j + i;

                            p_sub[idx] = subject1d[idx_o];
                        }
                    }
                }
            }

            SDATATYPE* p_tar = new SDATATYPE [pagesz_bb_tar];
            if (!p_tar)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }


            //REAL max_response = 0;
            // search the best match plane from tar
            for(V3DLONG k=btz; k<=etz; k++)
            {
                V3DLONG offset_k = (k-btz)*bbtx*bbty;
                V3DLONG offset_o_k = k*tx*ty;
                for(V3DLONG j=bty; j<=ety; j++)
                {
                    V3DLONG offset_j = offset_k + (j-bty)*bbtx;
                    V3DLONG offset_o_j = offset_o_k + j*tx;
                    for(V3DLONG i=btx; i<=etx; i++)
                    {
                        V3DLONG idx = offset_j + (i-btx);
                        V3DLONG idx_o = offset_o_j + i;

                        p_tar[idx] = target1d[idx_o];
                    }
                }
            }

            //
            sx = bbsx; sy = bbsy; sz = bbsz;
            tx = bbtx; ty = bbty; tz = bbtz;

            V3DLONG sx_pad = sx+tx-1, sy_pad = sy+ty-1, sz_pad = sz+tz-1;

            V3DLONG even_odd = sx_pad%2; // 0 for even 1 for odd

            bool fftw_in_place = true;

            if(fftw_in_place)
                sx_pad += (2-even_odd); //2*(sx_pad/2 + 1); // fftw_in_place

            V3DLONG len_pad = sx_pad*sy_pad*sz_pad;

            szPad[0] = sx_pad;
            szPad[1] = sy_pad;
            szPad[2] = sz_pad;

            szTar[0] = tx;
            szTar[1] = ty;
            szTar[2] = tz;

            szSub[0] = sx;
            szSub[1] = sy;
            szSub[2] = sz;

            REAL* p_f_sub = new REAL [len_pad];
            if (!p_f_sub)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }
            else
            {
                // init
                memset(p_f_sub, 0, sizeof(REAL)*len_pad);

                //padding zeros for target imag
                Y_IMG_REAL pOut(p_f_sub, szPad);
                Y_IMG_DATATYPE pIn(p_sub, szSub);

                YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
                tmp.padding(pOut, pIn, true, fftw_in_place, even_odd, 3);

            }

            REAL* p_f_tar = new REAL [len_pad];
            if (!p_f_tar)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }
            else
            {
                // init
                memset(p_f_tar, 0, sizeof(REAL)*len_pad);

                //padding zeros for target imag
                Y_IMG_REAL pOut(p_f_tar, szPad);
                Y_IMG_DATATYPE pIn(p_tar, szTar);

                YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
                tmp.padding(pOut, pIn, false, fftw_in_place, even_odd, 3);

            }

            //de-alloc
            if(p_sub) {delete []p_sub; p_sub=0;}
            if(p_tar) {delete []p_tar; p_tar=0;}


            // fft-ncc
            PEAKS pos_pcncc;

            pos_pcncc.x = pos_x;
            pos_pcncc.y =  (bsy - ((sy_ori-1) - pos_y)) - bty + (bbsy-1);
            pos_pcncc.z = pos_z;

            Y_IMG_REAL pOut(p_f_sub, szPad);
            Y_IMG_REAL pIn(p_f_tar, szPad);

            YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_REAL> tmp;
            tmp.fftnccp3D(pOut, pIn, szSub, szTar, even_odd, fftw_in_place, scale, &pos_pcncc);

            //pos_score = pos_pcncc.value;

            qDebug() << " bb score ..." << pos_score << "offset ..." << bsy << bty << pos_pcncc.y << bbsy;

            pos_x = pos_pcncc.x; pos_y = (sy_ori-1) - (bsy - (bty + pos_pcncc.y - (bbsy-1))); pos_z = pos_pcncc.z; //

            //de-alloc
            if(p_f_sub) {delete []p_f_sub; p_f_sub=0;}
            if(p_f_tar) {delete []p_f_tar; p_f_tar=0;}

        }
        else if(plane_xy == true)
        {
            // finding rich information plane from sub aV3DLONG z
            V3DLONG info_count=0, zpln, max_info=0;

            V3DLONG weights = bbsz*0.15;
            weights /= 2;
            V3DLONG start_z=bsz+weights+nbbz, end_z=esz-weights-nbbz;
            for(V3DLONG k=start_z; k<=end_z; k++) //
            {
                info_count = 0;

                V3DLONG offset_o_k = k*sx*sy;
                for(V3DLONG j=bsy; j<=esy; j++)
                {
                    V3DLONG offset_o_j = j*sx;
                    for(V3DLONG i=bsx; i<=esx; i++)
                    {
                        V3DLONG idx_o = offset_o_k + offset_o_j + i;

                        if( subject1d[idx_o] > meanv)
                            info_count++;
                    }
                }

                if(info_count > max_info)
                {
                    max_info = info_count;
                    zpln = k;
                }

            }

            // extraction
            V3DLONG b_bsz = zpln - weights/2;

            if(b_bsz>bsz)
                bsz = b_bsz;

            V3DLONG e_esz = zpln + weights/2;

            if(e_esz<esz)
                esz = e_esz;

            bbsz = esz-bsz+1; // dims

            // crop corresponding thick planes from tar image
            V3DLONG b_btz = bsz + offset[2] - nbbz;

            if(b_btz>btz)
                btz = b_btz;

            V3DLONG e_etz = esz + offset[2] + nbbz;

            if(e_etz<etz)
                etz = e_etz;

            bbtz = etz-btz+1;

            qDebug() << " updated boundary ... "<< bsx << bsy << bsz << esx << esy << esz << btx << bty << btz << etx << ety << etz;
            qDebug() << " updated dims ... "<< bbsx << bbsy << bbsz << bbtx << bbty << bbtz;

            V3DLONG pagesz_bb_sub = bbsx*bbsy*bbsz;
            V3DLONG pagesz_bb_tar = bbtx*bbty*bbtz;

            // extract one plane from sub
            SDATATYPE* p_sub = new SDATATYPE [pagesz_bb_sub];
            if (!p_sub)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }
            else
            {
                for(V3DLONG k=bsz; k<=esz; k++)
                {
                    V3DLONG offset_k = (k-bsz)*bbsx*bbsy;
                    V3DLONG offset_o_k = k*sx*sy;
                    for(V3DLONG j=bsy; j<=esy; j++)
                    {
                        V3DLONG offset_j = offset_k + (j-bsy)*bbsx;
                        V3DLONG offset_o_j = offset_o_k + j*sx;
                        for(V3DLONG i=bsx; i<=esx; i++)
                        {
                            V3DLONG idx = offset_j + (i-bsx);
                            V3DLONG idx_o = offset_o_j + i;

                            p_sub[idx] = subject1d[idx_o];
                        }
                    }
                }
            }

            SDATATYPE* p_tar = new SDATATYPE [pagesz_bb_tar];
            if (!p_tar)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }


            //REAL max_response = 0;
            // search the best match plane from tar
            for(V3DLONG k=btz; k<=etz; k++)
            {
                V3DLONG offset_k = (k-btz)*bbtx*bbty;
                V3DLONG offset_o_k = k*tx*ty;
                for(V3DLONG j=bty; j<=ety; j++)
                {
                    V3DLONG offset_j = offset_k + (j-bty)*bbtx;
                    V3DLONG offset_o_j = offset_o_k + j*tx;
                    for(V3DLONG i=btx; i<=etx; i++)
                    {
                        V3DLONG idx = offset_j + (i-btx);
                        V3DLONG idx_o = offset_o_j + i;

                        p_tar[idx] = target1d[idx_o];
                    }
                }
            }

            //
            sx = bbsx; sy = bbsy; sz = bbsz;
            tx = bbtx; ty = bbty; tz = bbtz;

            V3DLONG sx_pad = sx+tx-1, sy_pad = sy+ty-1, sz_pad = sz+tz-1;

            V3DLONG even_odd = sx_pad%2; // 0 for even 1 for odd

            bool fftw_in_place = true;

            if(fftw_in_place)
                sx_pad += (2-even_odd); //2*(sx_pad/2 + 1); // fftw_in_place

            V3DLONG len_pad = sx_pad*sy_pad*sz_pad;

            szPad[0] = sx_pad;
            szPad[1] = sy_pad;
            szPad[2] = sz_pad;

            szTar[0] = tx;
            szTar[1] = ty;
            szTar[2] = tz;

            szSub[0] = sx;
            szSub[1] = sy;
            szSub[2] = sz;

            REAL* p_f_sub = new REAL [len_pad];
            if (!p_f_sub)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }
            else
            {
                // init
                memset(p_f_sub, 0, sizeof(REAL)*len_pad);

                //padding zeros for target imag
                Y_IMG_REAL pOut(p_f_sub, szPad);
                Y_IMG_DATATYPE pIn(p_sub, szSub);

                YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
                tmp.padding(pOut, pIn, true, fftw_in_place, even_odd, 3);

            }

            REAL* p_f_tar = new REAL [len_pad];
            if (!p_f_tar)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }
            else
            {
                // init
                memset(p_f_tar, 0, sizeof(REAL)*len_pad);

                //padding zeros for target imag
                Y_IMG_REAL pOut(p_f_tar, szPad);
                Y_IMG_DATATYPE pIn(p_tar, szTar);

                YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
                tmp.padding(pOut, pIn, false, fftw_in_place, even_odd, 3);

            }

            //de-alloc
            if(p_sub) {delete []p_sub; p_sub=0;}
            if(p_tar) {delete []p_tar; p_tar=0;}


            // fft-ncc
            PEAKS pos_pcncc;

            pos_pcncc.x = pos_x;
            pos_pcncc.y = pos_y;
            pos_pcncc.z = (bsz - ((sz_ori-1) - pos_z)) - btz + (bbsz-1);

            Y_IMG_REAL pOut(p_f_sub, szPad);
            Y_IMG_REAL pIn(p_f_tar, szPad);

            YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_REAL> tmp;
            tmp.fftnccp3D(pOut, pIn, szSub, szTar, even_odd, fftw_in_place, scale, &pos_pcncc);

            //pos_score = pos_pcncc.value;

            qDebug() << " bb score ..." << pos_score << "offset ..." << bsz << btz << pos_pcncc.z << bbsz;

            pos_x = pos_pcncc.x; pos_y = pos_pcncc.y; pos_z = (sz_ori-1) - (bsz - (btz + pos_pcncc.z - (bbsz-1))); //

            //de-alloc
            if(p_f_sub) {delete []p_f_sub; p_f_sub=0;}
            if(p_f_tar) {delete []p_f_tar; p_f_tar=0;}
        }


        //de-alloc
        if(szPad) {delete []szPad; szPad=0;}
        if(szSub) {delete []szSub; szSub=0;}
        if(szTar) {delete []szTar; szTar=0;}

    }

    size_t end_t = clock();

    qDebug() << "time bb ... " << end_t-start_t_2;

    //Output
    //------------------------------------------------------------------------------------------------
    pos->x = pos_x; pos->y = pos_y; pos->z = pos_z; pos->value = pos_score;

    qDebug() << " finally..." << pos->x << pos->y << pos->z << pos->value << "offset ..." << pos_x - sx_ori +1 << pos_y - sy_ori +1 << pos_z - sz_ori +1;

    return true;

}

// pairwise image blending function
template <class SDATATYPE> 
int pwi_fusing(SDATATYPE *p_mask, SDATATYPE *data1d, SDATATYPE *subject1d, V3DLONG *sz_subject, SDATATYPE *target1d, V3DLONG *sz_target, V3DLONG *offset, bool axes_show, QString m_InputFileName, V3DLONG new_sz0, V3DLONG new_sz1, V3DLONG new_sz2, V3DLONG pagesz_overlap, V3DLONG sub_c, V3DLONG tar_c)
{
    //
    V3DLONG sx = sz_subject[0], sy = sz_subject[1], sz = sz_subject[2], sc = sz_subject[3];
    V3DLONG tx = sz_target[0], ty = sz_target[1], tz = sz_target[2], tc = sz_target[3];

    V3DLONG offset_tx, offset_ty, offset_tz, offset_sx, offset_sy, offset_sz;

    if(offset[0]<0)
    {
        offset_sx = 0; offset_tx = -offset[0];
    }
    else
    {
        offset_sx = offset[0]; offset_tx = 0;
    }
    if(offset[1]<0)
    {
        offset_sy = 0; offset_ty = -offset[1];
    }
    else
    {
        offset_sy = offset[1]; offset_ty = 0;
    }
    if(offset[2]<0)
    {
        offset_sz = 0; offset_tz = -offset[2];
    }
    else
    {
        offset_sz = offset[2]; offset_tz = 0;
    }

    qDebug("new_sz0 %ld new_sz1 %ld offset_tx %ld offset_ty %ld offset_sx %ld offset_sy %ld", new_sz0, new_sz1, offset_tx, offset_ty, offset_sx, offset_sy);

    //outputs
    V3DLONG offset_data = new_sz0*new_sz1*new_sz2;
    V3DLONG offset_target = tx*ty*tz;
    V3DLONG offset_subject = sx*sy*sz;

    V3DLONG i_start, j_start, k_start;
    V3DLONG sz_start = offset_sz, sz_end = sz_start + sz; if(sz_start<0) k_start=0; else k_start=sz_start; if(sz_end>new_sz2) sz_end=new_sz2;
    V3DLONG sy_start = offset_sy, sy_end = sy_start + sy; if(sy_start<0) j_start=0; else j_start=sy_start; if(sy_end>new_sz1) sy_end=new_sz1;
    V3DLONG sx_start = offset_sx, sx_end = sx_start + sx; if(sx_start<0) i_start=0; else i_start=sx_start; if(sx_end>new_sz0) sx_end=new_sz0;

    //cout<< k_start << " " << sz_end  << " " << j_start  << " " << sy_end  << " " << i_start  << " " << sx_end << endl;

    for(V3DLONG c=0; c<sc; c++)
    {
        V3DLONG offset_c = c*offset_data;
        V3DLONG offsets_c = c*offset_subject;
        for(V3DLONG k=k_start; k<sz_end; k++)
        {
            V3DLONG offset_k = offset_c + k*new_sz0*new_sz1;
            V3DLONG offsets_k = offsets_c + (k-k_start)*sx*sy; if (k-k_start >= sz) continue;
            for(V3DLONG j=j_start; j<sy_end; j++)
            {
                V3DLONG offset_j = offset_k + j*new_sz0;
                V3DLONG offsets_j = offsets_k + (j-j_start)*sx; if (j-j_start >= sy) continue;
                for(V3DLONG i=i_start; i<sx_end; i++)
                {
                    V3DLONG idx = offset_j + i;

                    data1d[idx] = subject1d[offsets_j + i - i_start];

                    //show boundary
                    //					if(axes_show)
                    //					{
                    //						if( (i==i_start && j==j_start) || (i==i_start && k==k_start) || (j==j_start && k==k_start) || (i==sx_end-1 && j==j_start) || (i==sx_end-1 && k==k_start) || (j==sy_end-1 && i==i_start) || (j==sy_end-1 && k==k_start) || (k==sz_end-1 && i==i_start) || (k==sz_end-1 && j==j_start) || (i==sx_end-1 && j==sy_end-1) || (i==sx_end-1 && k==sz_end-1) || (j==sy_end-1 && k==sz_end-1) )
                    //						{
                    //							data1d[idx] = 255;
                    //						}
                    //					}

                    if(c==sub_c)
                        p_mask[idx - c*offset_data]=subject1d[offsets_j + i - i_start] + 1; //255; //mask
                }
            }
        }
    }

    V3DLONG tz_start = offset_tz, tz_end = offset_tz + tz; if(tz_start<0) tz_start=0; if(tz_end>new_sz2) tz_end=new_sz2;
    V3DLONG ty_start = offset_ty, ty_end = offset_ty + ty; if(ty_start<0) ty_start=0; if(ty_end>new_sz1) ty_end=new_sz1;
    V3DLONG tx_start = offset_tx, tx_end = offset_tx + tx;	if(tx_start<0) tx_start=0; if(tx_end>new_sz0) tx_end=new_sz0;

    //cout<< tz_start << " " << tz_end  << " " << ty_start  << " " << ty_end  << " " << tx_start  << " " << tx_end << endl;

    for(V3DLONG c=0; c<sc; c++)
    {
        V3DLONG offset_c = c*offset_data;
        V3DLONG offsets_c = c*offset_target;
        for(V3DLONG k=tz_start; k<tz_end; k++)
        {
            V3DLONG offset_k = offset_c + k*new_sz0*new_sz1;
            V3DLONG offsets_k = offsets_c + (k-tz_start)*tx*ty;
            for(V3DLONG j=ty_start; j<ty_end; j++)
            {
                V3DLONG offset_j = offset_k + j*new_sz0;
                V3DLONG offsets_j = offsets_k + (j-ty_start)*tx;
                for(V3DLONG i=tx_start; i<tx_end; i++)
                {
                    V3DLONG idx = offset_j + i;

                    //					if(data1d[idx])
                    //					{
                    //						data1d[idx] = fmax(data1d[idx], (unsigned char) target1d[offsets_j + i - tx_start]); // Max. Intensity
                    //					}
                    //					else
                    //					{
                    //						data1d[idx] = (unsigned char) target1d[offsets_j + i - tx_start];
                    //					}

                    if(data1d[idx]) //p_mask[idx-offset_c]
                    {
                        data1d[idx] = (SDATATYPE) ( (data1d[idx] + target1d[offsets_j + i - tx_start])/2.0 ); // Avg. Intensity
                    }
                    else
                    {
                        data1d[idx] = (SDATATYPE) target1d[offsets_j + i - tx_start];
                    }

                    //show boundary
                    //					if(axes_show)
                    //					{
                    //						if( (i==tx_start && j==ty_start) || (i==tx_start && k==tz_start) || (j==ty_start && k==tz_start) || (i==tx_end-1 && j==ty_start) || (i==tx_end-1 && k==tz_start) || (j==ty_end-1 && i==tx_start) || (j==ty_end-1 && k==tz_start) || (k==tz_end-1 && i==tx_start) || (k==tz_end-1 && j==ty_start) || (i==tx_end-1 && j==ty_end-1) || (i==tx_end-1 && k==tz_end-1) || (j==ty_end-1 && k==tz_end-1) )
                    //						{
                    //							data1d[idx] = 255;
                    //						}
                    //					}

                    if(c==tar_c)
                        p_mask[pagesz_overlap + idx - c*offset_data] = target1d[offsets_j + i - tx_start]; //255; //mask
                }
            }
        }
    }

    // .swc boundary file
    if(axes_show)
    {
        //write a .swc file showing the boundary
        //
        //			2 _______5
        //			/|      /|
        //		 1 /_|6____/3|
        //		   | /_____|_/8
        //       4 |/______|/7
        //
        // points: 1(0) 2(1) 3(1) 4(1) 5(2,3) 6(2,4) 7(3,4) 8(5,6,7)
        // .swc:   n, type, x, y, z, r, pn
        NeuronSWC S;
        QString outputSWC = m_InputFileName + ".swc";
        FILE * fp = fopen(outputSWC.toAscii(), "wt");

        V3DLONG n_swc=1;

        //sub
        fprintf(fp, "%ld %d %ld %ld %ld %d %ld\n", n_swc++, 2, i_start, j_start, k_start, 2L, 0L); //1

        fprintf(fp, "%ld %d %ld %ld %ld %ld %ld\n", n_swc++, 2, i_start, j_start, sz_end-1, 2L, n_swc-1); //2
        fprintf(fp, "%ld %d %ld %ld %ld %ld %ld\n", n_swc++, 2, sx_end-1, j_start, k_start, 2L, n_swc-2); //3
        fprintf(fp, "%ld %d %ld %ld %ld %ld %ld\n", n_swc++, 2, i_start, sy_end-1, k_start, 2L, n_swc-3); //4

        fprintf(fp, "%ld %d %ld %ld %ld %ld %ld\n", n_swc, 2, sx_end-1, j_start, sz_end-1, 2L, n_swc-3); //5
        fprintf(fp, "%ld %d %ld %ld %ld %ld %ld\n", n_swc++, 2, sx_end-1, j_start, sz_end-1, 2L, n_swc-2); //5
        fprintf(fp, "%ld %d %ld %ld %ld %ld %ld\n", n_swc, 2, i_start, sy_end-1, sz_end-1, 2L, n_swc-4); //6
        fprintf(fp, "%ld %d %ld %ld %ld %ld %ld\n", n_swc++, 2, i_start, sy_end-1, sz_end-1, 2L, n_swc-2); //6
        fprintf(fp, "%ld %d %ld %ld %ld %ld %ld\n", n_swc, 2, sx_end-1, sy_end-1, k_start, 2L, n_swc-4); //7
        fprintf(fp, "%ld %d %ld %ld %ld %ld %ld\n", n_swc++, 2, sx_end-1, sy_end-1, k_start, 2L, n_swc-3); //7

        fprintf(fp, "%ld %d %ld %ld %ld %ld %ld\n", n_swc, 2, sx_end-1, sy_end-1, sz_end-1, 2L, n_swc-3); //8
        fprintf(fp, "%ld %d %ld %ld %ld %ld %ld\n", n_swc, 2, sx_end-1, sy_end-1, sz_end-1, 2L, n_swc-2); //8
        fprintf(fp, "%ld %d %ld %ld %ld %ld %ld\n", n_swc++, 2, sx_end-1, sy_end-1, sz_end-1, 2L, n_swc-1); //8

        // tar
        fprintf(fp, "%ld %d %ld %ld %ld %ld %ld\n", n_swc++, 2, tx_start, ty_start, tz_start, 2L, 0L); //1

        fprintf(fp, "%ld %d %ld %ld %ld %ld %ld\n", n_swc++, 2, tx_start, ty_start, tz_end-1, 2L, n_swc-1); //2
        fprintf(fp, "%ld %d %ld %ld %ld %ld %ld\n", n_swc++, 2, tx_end-1, ty_start, tz_start, 2L, n_swc-2); //3
        fprintf(fp, "%ld %d %ld %ld %ld %ld %ld\n", n_swc++, 2, tx_start, ty_end-1, tz_start, 2L, n_swc-3); //4

        fprintf(fp, "%ld %d %ld %ld %ld %ld %ld\n", n_swc, 2, tx_end-1, ty_start, tz_end-1, 2L, n_swc-3); //5
        fprintf(fp, "%ld %d %ld %ld %ld %ld %ld\n", n_swc++, 2, tx_end-1, ty_start, tz_end-1, 2L, n_swc-2); //5
        fprintf(fp, "%ld %d %ld %ld %ld %ld %ld\n", n_swc, 2, tx_start, ty_end-1, tz_end-1, 2L, n_swc-4); //6
        fprintf(fp, "%ld %d %ld %ld %ld %ld %ld\n", n_swc++, 2, tx_start, ty_end-1, tz_end-1, 2L, n_swc-2); //6
        fprintf(fp, "%ld %d %ld %ld %ld %ld %ld\n", n_swc, 2, tx_end-1, ty_end-1, tz_start, 2L, n_swc-4); //7
        fprintf(fp, "%ld %d %ld %ld %ld %ld %ld\n", n_swc++, 2, tx_end-1, ty_end-1, tz_start, 2L, n_swc-3); //7

        fprintf(fp, "%ld %d %ld %ld %ld %ld %ld\n", n_swc, 2, tx_end-1, ty_end-1, tz_end-1, 2L, n_swc-3); //8
        fprintf(fp, "%ld %d %ld %ld %ld %ld %ld\n", n_swc, 2, tx_end-1, ty_end-1, tz_end-1, 2L, n_swc-2); //8
        fprintf(fp, "%ld %d %ld %ld %ld %ld %ld\n", n_swc++, 2, tx_end-1, ty_end-1, tz_end-1, 2L, n_swc-1); //8

        fclose(fp);

    }

    return true;
}

// group images blending function
template <class SDATATYPE> 
int groupi_fusing(SDATATYPE *pVImg, Y_VIM<REAL, V3DLONG, indexed_t<V3DLONG, REAL>, LUT<V3DLONG> > vim, V3DLONG vx, V3DLONG vy, V3DLONG vz, V3DLONG vc, bool axes_show)
{
    // for boundary counting
    V3DLONG n_swc=1;
    QString outputSWC, outputAPO;
    FILE * fp_swc=NULL, *fp_apo=NULL; // .swc showing boundary .apo showing tile's name

    // look up lut
    for(V3DLONG ii=0; ii<vim.tilesList.size(); ii++)
    {
        // loading relative imagg files
        V3DLONG *sz_relative = 0;
        int datatype_relative = 0;
        unsigned char* relative1d = 0;

        if (loadImage(const_cast<char *>(vim.tilesList.at(ii).fn_image.c_str()), relative1d, sz_relative, datatype_relative)!=true)
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",vim.tilesList.at(ii).fn_image.c_str());
            return -1;
        }
        V3DLONG rx=sz_relative[0], ry=sz_relative[1], rz=sz_relative[2], rc=sz_relative[3];

        //
        V3DLONG tile2vi_xs = vim.lut[ii].start_pos[0]-vim.min_vim[0];
        V3DLONG tile2vi_xe = vim.lut[ii].end_pos[0]-vim.min_vim[0];
        V3DLONG tile2vi_ys = vim.lut[ii].start_pos[1]-vim.min_vim[1];
        V3DLONG tile2vi_ye = vim.lut[ii].end_pos[1]-vim.min_vim[1];
        V3DLONG tile2vi_zs = vim.lut[ii].start_pos[2]-vim.min_vim[2];
        V3DLONG tile2vi_ze = vim.lut[ii].end_pos[2]-vim.min_vim[2];

        V3DLONG x_start = (1 > tile2vi_xs) ? 1 : tile2vi_xs;
        V3DLONG x_end = (vx < tile2vi_xe) ? vx : tile2vi_xe;
        V3DLONG y_start = (1 > tile2vi_ys) ? 1 : tile2vi_ys;
        V3DLONG y_end = (vy < tile2vi_ye) ? vy : tile2vi_ye;
        V3DLONG z_start = (1 > tile2vi_zs) ? 1 : tile2vi_zs;
        V3DLONG z_end = (vz < tile2vi_ze) ? vz : tile2vi_ze;

        x_end++;
        y_end++;
        z_end++;

        //
        qDebug() << x_start << " " << x_end << " " << y_start << " " << y_end << " " << z_start << " " << z_end << endl;

        //suppose all tiles with same color dimensions
        if(rc>vc)
            rc = vc;

        //show boundary
        if(axes_show)
        {
            // .swc boundary file
            //write a .swc file showing the boundary
            //
            //			2 _______5
            //			/|      /|
            //		 1 /_|6____/3|
            //		   | /_____|_/8
            //       4 |/______|/7
            //
            // points: 1(0) 2(1) 3(1) 4(1) 5(2,3) 6(2,4) 7(3,4) 8(5,6,7)
            // .swc:   n, type, x, y, z, r, pn

            if(ii==0)
            {
                outputSWC = QString(vim.tilesList.at(0).fn_image.c_str()) + ".swc";
                fp_swc = fopen(outputSWC.toAscii(), "wt");

                outputAPO = QString(vim.tilesList.at(0).fn_image.c_str()) + ".apo";
                fp_apo = fopen(outputAPO.toAscii(), "wt");
            }

            //saving tile's boundary
            fprintf(fp_swc, "%ld %d %ld %ld %ld %d %ld\n", n_swc++, 2, x_start, y_start, z_start, 1L, 0L); //1

            fprintf(fp_swc, "%ld %d %ld %ld %ld %ld %ld\n", n_swc++, 2, x_start, y_start, z_end-1, 1L, n_swc-1); //2
            fprintf(fp_swc, "%ld %d %ld %ld %ld %ld %ld\n", n_swc++, 2, x_end-1, y_start, z_start, 1L, n_swc-2); //3
            fprintf(fp_swc, "%ld %d %ld %ld %ld %ld %ld\n", n_swc++, 2, x_start, y_end-1, z_start, 1L, n_swc-3); //4

            fprintf(fp_swc, "%ld %d %ld %ld %ld %ld %ld\n", n_swc, 2, x_end-1, y_start, z_end-1, 1L, n_swc-3); //5
            fprintf(fp_swc, "%ld %d %ld %ld %ld %ld %ld\n", n_swc++, 2, x_end-1, y_start, z_end-1, 1L, n_swc-2); //5
            fprintf(fp_swc, "%ld %d %ld %ld %ld %ld %ld\n", n_swc, 2, x_start, y_end-1, z_end-1, 1L, n_swc-4); //6
            fprintf(fp_swc, "%ld %d %ld %ld %ld %ld %ld\n", n_swc++, 2, x_start, y_end-1, z_end-1, 1L, n_swc-2); //6
            fprintf(fp_swc, "%ld %d %ld %ld %ld %ld %ld\n", n_swc, 2, x_end-1, y_end-1, z_start, 1L, n_swc-4); //7
            fprintf(fp_swc, "%ld %d %ld %ld %ld %ld %ld\n", n_swc++, 2, x_end-1, y_end-1, z_start, 1L, n_swc-3); //7

            fprintf(fp_swc, "%ld %d %ld %ld %ld %ld %ld\n", n_swc, 2, x_end-1, y_end-1, z_end-1, 1L, n_swc-3); //8
            fprintf(fp_swc, "%ld %d %ld %ld %ld %ld %ld\n", n_swc, 2, x_end-1, y_end-1, z_end-1, 1L, n_swc-2); //8
            fprintf(fp_swc, "%ld %d %ld %ld %ld %ld %ld\n", n_swc++, 2, x_end-1, y_end-1, z_end-1, 1L, n_swc-1); //8

            //saving tile's name
            string fn = QString(vim.tilesList.at(ii).fn_image.c_str()).remove(0, QFileInfo(QString(vim.tilesList.at(ii).fn_image.c_str())).path().length()+1).toStdString();
            fprintf(fp_apo, "%d,%s,%s,%s,%ld,%ld,%ld,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,,,\n",
                    ii+1, //i+1,
                    "",
                    fn.c_str(),
                    "",
                    V3DLONG(z_start + 0.5*vim.tilesList.at(ii).sz_image[2]),
                    V3DLONG(x_start + 0.5*vim.tilesList.at(ii).sz_image[0]),
                    V3DLONG(y_start + 0.5*vim.tilesList.at(ii).sz_image[1]),
                    10.0, //pixmax,
                    10.0, //intensity,
                    10.0, //sdev,
                    1000.0, //volsize,
                    10.0 //mass

                    //					color.r,
                    //					color.g,
                    //					color.b
                    );


            if(ii==vim.tilesList.size()-1)
            {
                fclose(fp_swc);
                fclose(fp_apo);
            }

        }

        //
        SDATATYPE *prelative = (SDATATYPE *)relative1d;

        for(V3DLONG c=0; c<rc; c++)
        {
            V3DLONG o_c = c*vx*vy*vz;
            V3DLONG o_r_c = c*rx*ry*rz;
            for(V3DLONG k=z_start; k<z_end; k++)
            {
                V3DLONG o_k = o_c + k*vx*vy;
                V3DLONG o_r_k = o_r_c + (k-z_start)*rx*ry;

                for(V3DLONG j=y_start; j<y_end; j++)
                {
                    V3DLONG o_j = o_k + j*vx;
                    V3DLONG o_r_j = o_r_k + (j-y_start)*rx;
                    for(V3DLONG i=x_start; i<x_end; i++)
                    {
                        V3DLONG idx = o_j + i;
                        V3DLONG idx_r = o_r_j + (i-x_start);

                        //						if(pVImg[idx]>0)
                        //							pVImg[idx] = (pVImg[idx]>relative1d[idx_r])?pVImg[idx]:relative1d[idx_r]; // Max. Intensity
                        //						else
                        //							pVImg[idx] = relative1d[idx_r];

                        if(pVImg[idx])
                        {
                            pVImg[idx] = 0.5*(pVImg[idx] + prelative[idx_r]); // Avg. Intensity
                        }
                        else
                        {
                            pVImg[idx] = prelative[idx_r];
                        }

                        //show boundary
                        //						if(axes_show)
                        //						{
                        //							if( (i==x_start && j==y_start) || (i==x_start && k==z_start) || (j==y_start && k==z_start) || (i==x_end-1 && j==y_start) || (i==x_end-1 && k==z_start) || (j==y_end-1 && i==x_start) || (j==y_end-1 && k==z_start) || (k==z_end-1 && i==x_start) || (k==z_end-1 && j==y_start) || (i==x_end-1 && j==y_end-1) || (i==x_end-1 && k==z_end-1) || (j==y_end-1 && k==z_end-1) )
                        //							{
                        //								pVImg[idx] = 255;
                        //							}
                        //
                        //						}

                    }
                }
            }
        }

        //de-alloc
        if(relative1d) {delete []relative1d; relative1d=0;}
        if(sz_relative) {delete []sz_relative; sz_relative=0;}
    }

    return true;
}

// group images blending function
template <class SDATATYPE> 
int region_groupfusing(SDATATYPE *pVImg, Y_VIM<REAL, V3DLONG, indexed_t<V3DLONG, REAL>, LUT<V3DLONG> > vim, unsigned char *relative1d,
                       V3DLONG vx, V3DLONG vy, V3DLONG vz, V3DLONG vc, V3DLONG rx, V3DLONG ry, V3DLONG rz, V3DLONG rc,
                       V3DLONG tile2vi_zs, V3DLONG tile2vi_ys, V3DLONG tile2vi_xs,
                       V3DLONG z_start, V3DLONG z_end, V3DLONG y_start, V3DLONG y_end, V3DLONG x_start, V3DLONG x_end, V3DLONG *start)
{

    //
    SDATATYPE *prelative = (SDATATYPE *)relative1d;

    if(x_end<x_start || y_end<y_start || z_end<z_start)
        return false;

    // update virtual image pVImg
    V3DLONG offset_volume_v = vx*vy*vz;
    V3DLONG offset_volume_r = rx*ry*rz;

    V3DLONG offset_pagesz_v = vx*vy;
    V3DLONG offset_pagesz_r = rx*ry;

    for(V3DLONG c=0; c<rc; c++)
    {
        V3DLONG o_c = c*offset_volume_v;
        V3DLONG o_r_c = c*offset_volume_r;
        for(V3DLONG k=z_start; k<z_end; k++)
        {
            V3DLONG o_k = o_c + (k-start[2])*offset_pagesz_v;
            V3DLONG o_r_k = o_r_c + (k-z_start)*offset_pagesz_r;

            for(V3DLONG j=y_start; j<y_end; j++)
            {
                V3DLONG o_j = o_k + (j-start[1])*vx;
                V3DLONG o_r_j = o_r_k + (j-y_start)*rx;

                for(V3DLONG i=x_start; i<x_end; i++)
                {
                    V3DLONG idx = o_j + i-start[0];
                    V3DLONG idx_r = o_r_j + (i-x_start);

                    if(pVImg[idx])
                    {
                        pVImg[idx] = 0.5*(pVImg[idx] + prelative[idx_r]); // Avg. Intensity
                    }
                    else
                    {
                        pVImg[idx] = prelative[idx_r];
                    }
                }
            }
        }
    }

    return true;
}

// subspace stitching and linear blending tiles
template <class Tdata, class Y_IMG_DATATYPE> bool iSubspaceStitching(Tdata *pVImg, Y_VIM<REAL, V3DLONG, indexed_t<V3DLONG, REAL>, LUT<V3DLONG> > vim, Tdata intensityrange, V3DLONG channel, bool flagFusion)
{
    //
    V3DLONG vx = vim.sz[0];
    V3DLONG vy = vim.sz[1];
    V3DLONG vz = vim.sz[2];
    V3DLONG vc = vim.sz[3];
    
    V3DLONG pagesz = vx*vy*vz;
    V3DLONG imgsz = pagesz*vc;
    
    bool fftw_in_place = true;
    
    REAL *pTmp = NULL;
    
    if(flagFusion)
    {
        try {
            pTmp = new REAL [imgsz];
            memset(pTmp, 0.0, sizeof(REAL)*imgsz);
        } catch (...) {
            printf("Fail to allocate memory for pTmp!\n");

            y_del<REAL>(pTmp);
            return false;
        }
    }

    // testing
    for(int i_path=0; i_path<vim.number_tiles; i_path++)
    {
        qDebug()<<"current ..."<<vim.tilesList.at(i_path).n<<"init offsets ..."<<vim.tilesList.at(i_path).offsets_sa[0]<<vim.tilesList.at(i_path).offsets_sa[1]<<vim.tilesList.at(i_path).offsets_sa[2];
    }

    // subspace stitch
    for(V3DLONG ii=0; ii<vim.number_tiles; ii++)
    {

        qDebug()<<"tile g "<<ii<<"\'s predecessor f ..."<<vim.tilesList.at(ii).predecessor;

        if(vim.tilesList.at(ii).predecessor<0) // reference image without parent
        {
            if(flagFusion)
            {
                // loading ref image
                V3DLONG *sz_ref = 0;
                int datatype_ref = 0;
                unsigned char* ref1d = 0;
                
                //
                if (loadImage(const_cast<char *>(vim.tilesList.at(ii).fn_image.c_str()), ref1d, sz_ref, datatype_ref)!=true)
                {
                    fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",vim.tilesList.at(ii).fn_image.c_str());
                    return false;
                }
                V3DLONG rx=sz_ref[0], ry=sz_ref[1], rz=sz_ref[2], rc=sz_ref[3];
                
                //
                V3DLONG tile2vi_xs = vim.lut[ii].start_pos[0]-vim.min_vim[0];
                V3DLONG tile2vi_xe = vim.lut[ii].end_pos[0]-vim.min_vim[0];
                V3DLONG tile2vi_ys = vim.lut[ii].start_pos[1]-vim.min_vim[1];
                V3DLONG tile2vi_ye = vim.lut[ii].end_pos[1]-vim.min_vim[1];
                V3DLONG tile2vi_zs = vim.lut[ii].start_pos[2]-vim.min_vim[2];
                V3DLONG tile2vi_ze = vim.lut[ii].end_pos[2]-vim.min_vim[2];
                
                V3DLONG x_start = (0 > tile2vi_xs) ? 0 : tile2vi_xs;
                V3DLONG x_end = (vx-1 < tile2vi_xe) ? vx-1 : tile2vi_xe;
                V3DLONG y_start = (0 > tile2vi_ys) ? 0 : tile2vi_ys;
                V3DLONG y_end = (vy-1 < tile2vi_ye) ? vy-1 : tile2vi_ye;
                V3DLONG z_start = (0 > tile2vi_zs) ? 0 : tile2vi_zs;
                V3DLONG z_end = (vz-1 < tile2vi_ze) ? vz-1 : tile2vi_ze;
                
                x_end++;
                y_end++;
                z_end++;
                
                //suppose all tiles with same color dimensions
                Tdata *prelative = (Tdata *)ref1d;
                
                for(V3DLONG c=0; c<rc; c++)
                {
                    V3DLONG o_c = c*vx*vy*vz;
                    V3DLONG o_r_c = c*rx*ry*rz;
                    for(V3DLONG k=z_start; k<z_end; k++)
                    {
                        V3DLONG o_k = o_c + k*vx*vy;
                        V3DLONG o_r_k = o_r_c + (k-z_start)*rx*ry;
                        
                        for(V3DLONG j=y_start; j<y_end; j++)
                        {
                            V3DLONG o_j = o_k + j*vx;
                            V3DLONG o_r_j = o_r_k + (j-y_start)*rx;
                            for(V3DLONG i=x_start; i<x_end; i++)
                            {
                                V3DLONG idx = o_j + i;
                                V3DLONG idx_r = o_r_j + (i-x_start);
                                
                                float val = (float)(prelative[idx_r]);
                                
                                //
                                float coef;
                                if(!computeWeights(vim, i, j, k, ii, coef) )
                                {
                                    printf("Fail to call function computeWeights!\n");
                                    return false;
                                }
                                
                                pTmp[idx] += (Tdata)(val*coef); // linear blending
                                
                            }
                        }
                    }
                }
                
                //de-alloc
                if(ref1d) {delete []ref1d; ref1d=0;}
                if(sz_ref) {delete []sz_ref; sz_ref=0;}
            }

            continue;
        }
        
        //
        V3DLONG i = ii; // current tile g
        V3DLONG j = vim.tilesList.at(ii).predecessor; // parent tile f
        
        V3DLONG jx_s, jy_s, jz_s, jx_e, jy_e, jz_e;
        V3DLONG ix_s, iy_s, iz_s, ix_e, iy_e, iz_e;
        
        jx_s = vim.lut[j].start_pos[0] + vim.min_vim[0];
        jy_s = vim.lut[j].start_pos[1] + vim.min_vim[1];
        jz_s = vim.lut[j].start_pos[2] + vim.min_vim[2];
        
        jx_e = vim.lut[j].end_pos[0] + vim.min_vim[0];
        jy_e = vim.lut[j].end_pos[1] + vim.min_vim[1];
        jz_e = vim.lut[j].end_pos[2] + vim.min_vim[2];
        
        ix_s = vim.lut[i].start_pos[0] + vim.min_vim[0];
        iy_s = vim.lut[i].start_pos[1] + vim.min_vim[1];
        iz_s = vim.lut[i].start_pos[2] + vim.min_vim[2];
        
        ix_e = vim.lut[i].end_pos[0] + vim.min_vim[0];
        iy_e = vim.lut[i].end_pos[1] + vim.min_vim[1];
        iz_e = vim.lut[i].end_pos[2] + vim.min_vim[2];
        
        // load image f
        V3DLONG *sz_f = 0;
        int datatype_f = 0;
        unsigned char* f1d = 0;
        
        if (loadImage(const_cast<char *>(vim.tilesList.at(j).fn_image.c_str()), f1d, sz_f, datatype_f)!=true)
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",vim.tilesList.at(j).fn_image.c_str());
            return false;
        }
        V3DLONG fx=sz_f[0], fy=sz_f[1], fz=sz_f[2], fc=sz_f[3];
        
        // load image g
        V3DLONG *sz_g = 0;
        int datatype_g = 0;
        unsigned char* g1d = 0;
        
        //
        if (loadImage(const_cast<char *>(vim.tilesList.at(i).fn_image.c_str()), g1d, sz_g, datatype_g)!=true)
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",vim.tilesList.at(i).fn_image.c_str());
            return false;
        }
        V3DLONG gx=sz_g[0], gy=sz_g[1], gz=sz_g[2], gc=sz_g[3];
        
        // overlap image from f and g
        V3DLONG start_x = qMax(jx_s, ix_s);
        V3DLONG start_y = qMax(jy_s, iy_s);
        V3DLONG start_z = qMax(jz_s, iz_s);
        
        V3DLONG end_x = qMin(jx_e, ix_e);
        V3DLONG end_y = qMin(jy_e, iy_e);
        V3DLONG end_z = qMin(jz_e, iz_e);
        
        V3DLONG dimxol= end_x-start_x+1;
        V3DLONG dimyol= end_y-start_y+1;
        V3DLONG dimzol= end_z-start_z+1;

        V3DLONG even_odd = dimxol%2;
        if(fftw_in_place)
        {
            if(dimxol>2)
                dimxol -= (2-even_odd);
            else
                dimxol += (2-even_odd);
        }

        V3DLONG *szOL = new V3DLONG [3];
        
        szOL[0] = dimxol;
        szOL[1] = dimyol;
        szOL[2] = dimzol;
        
        V3DLONG pagesz_ol = dimxol*dimyol*dimzol;
        
        Tdata *fol1d = NULL;
        Tdata *gol1d = NULL;
        
        try 
        {
            // suppose fc = gc = vc
            fol1d = new Tdata [pagesz_ol];
            gol1d = new Tdata [pagesz_ol];

            memset(fol1d, 0.0, sizeof(Tdata)*pagesz_ol);
            memset(gol1d, 0.0, sizeof(Tdata)*pagesz_ol);
        } 
        catch (...) 
        {
            printf("Fail to allocate memory for fol1d and gol1d.\n");
            
            y_del<Tdata>(fol1d);
            y_del<Tdata>(gol1d);
            
            return false;
        }
        
        V3DLONG offsets_fc = channel*fx*fy*fz; // ref channel
        V3DLONG offsets_gc = channel*gx*gy*gz;
        
        for(V3DLONG z=start_z; z<end_z; z++)
        {
            V3DLONG offsets_fz = offsets_fc + (z-jz_s)*fx*fy;
            V3DLONG offsets_gz = offsets_gc + (z-iz_s)*gx*gy;
            
            V3DLONG offsets_olz = (z-start_z)*dimxol*dimyol;
            
            for(V3DLONG y=start_y; y<end_y; y++)
            {
                V3DLONG offsets_fy = offsets_fz + (y-jy_s)*fx;
                V3DLONG offsets_gy = offsets_gz + (y-iy_s)*gx;
                
                V3DLONG offsets_oly = offsets_olz + (y-start_y)*dimxol;
                
                for(V3DLONG x=start_x; x<end_x; x++)
                {
                    V3DLONG idx = offsets_oly + x - start_x;
                    
                    fol1d[idx] = ((Tdata *)f1d)[offsets_fy + (x-jx_s)];
                    gol1d[idx] = ((Tdata *)g1d)[offsets_gy + (x-ix_s)];
                }
            }
        }
        // de-alloc
        y_del<unsigned char>(f1d);
        y_del<unsigned char>(g1d);

        // zero-padding
        REAL rate_x, rate_y, rate_z;

        rate_x = qMax( (REAL)dimxol/(REAL)gx, (REAL)dimxol/(REAL)fx);
        rate_y = qMax( (REAL)dimyol/(REAL)gy, (REAL)dimyol/(REAL)fy);
        rate_z = qMax( (REAL)dimzol/(REAL)gz, (REAL)dimzol/(REAL)fz);

        bool plane_yz=false, plane_xz=false, plane_xy=false;

        V3DLONG bsx=0, esx=dimxol, bsy=0, esy=dimyol, bsz=0, esz=dimzol;
        V3DLONG bbsx=dimxol, bbsy=dimyol, bbsz=dimzol;

        if(rate_x < rate_y && rate_x < rate_z)
        {
            plane_yz = true;
        }
        if(rate_y < rate_x && rate_y < rate_z)
        {
            plane_xz = true;
        }
        if(rate_z < rate_x && rate_z < rate_y)
        {
            plane_xy = true;
        }

        REAL sum=0;
        for(V3DLONG k=bsz; k<esz; k++)
        {
            V3DLONG offset_o_k = k*dimxol*dimyol;
            for(V3DLONG j=bsy; j<esy; j++)
            {
                V3DLONG offset_o_j = offset_o_k + j*dimxol;
                for(V3DLONG i=bsx; i<esx; i++)
                {
                    V3DLONG idx_o = offset_o_j + i;

                    sum += fol1d[idx_o];
                }
            }
        }
        Tdata meanv = (Tdata) (sum/(REAL)(bbsx*bbsy*bbsz));

        if(plane_yz == true)
        {
            // finding key-plane along x direction
            V3DLONG info_count=0, xpln=0, max_info=0;

            //
            V3DLONG weights = bbsx*0.15; // threshold
            weights /= 2;
            V3DLONG start_x=bsx+weights+1, end_x=esx-weights-1;
            for(V3DLONG i=start_x; i<end_x; i++) //
            {
                info_count = 0;

                for(V3DLONG k=bsz; k<esz; k++)
                {
                    V3DLONG offset_o_k = k*dimxol*dimyol;
                    for(V3DLONG j=bsy; j<esy; j++)
                    {
                        V3DLONG idx_o = offset_o_k + j*dimxol + i;

                        if( fol1d[idx_o] > meanv)
                            info_count++;
                    }
                }

                if(info_count > max_info)
                {
                    max_info = info_count;
                    xpln = i;
                }

            }

            if(xpln<start_x) xpln = start_x;

            qDebug() << "xpln ..." << xpln;

            // extraction
            V3DLONG b_bsx = xpln - weights; //

            if(b_bsx>bsx)
                bsx = b_bsx;

            V3DLONG e_esx = xpln + weights;

            if(e_esx<esx)
                esx = e_esx;

            bbsx = esx-bsx+1; // dims

            V3DLONG pagesz_bb_sub = bbsx*bbsy*bbsz;
            V3DLONG pagesz_bb_tar = pagesz_bb_sub;

            // extract one plane from sub
            Tdata *p_sub = NULL, *p_tar = NULL;
            try
            {
                p_sub = new Tdata [pagesz_bb_sub];
                p_tar = new Tdata [pagesz_bb_sub];

                for(V3DLONG k=bsz; k<esz; k++)
                {
                    V3DLONG offset_k = (k-bsz)*bbsx*bbsy;
                    V3DLONG offset_o_k = k*dimxol*dimyol;
                    for(V3DLONG j=bsy; j<esy; j++)
                    {
                        V3DLONG offset_j = offset_k + (j-bsy)*bbsx;
                        V3DLONG offset_o_j = offset_o_k + j*dimxol;
                        for(V3DLONG i=bsx; i<esx; i++)
                        {
                            V3DLONG idx = offset_j + (i-bsx);
                            V3DLONG idx_o = offset_o_j + i;

                            p_sub[idx] = gol1d[idx_o];
                            p_tar[idx] = fol1d[idx_o];
                        }
                    }
                }
            }
            catch(...)
            {
                printf("Fail to allocate memory.\n");
                return false;
            }
            //
            y_del<Tdata>(fol1d);
            y_del<Tdata>(gol1d);

            //
            V3DLONG sx_pad = 2*bbsx-1, sy_pad = 2*bbsy-1, sz_pad = 2*bbsz-1;

            V3DLONG even_odd = sx_pad%2; // 0 for even 1 for odd

            if(fftw_in_place)
                sx_pad += (2-even_odd); //

            V3DLONG len_pad = sx_pad*sy_pad*sz_pad;

            V3DLONG *szPad = new V3DLONG [3];
            szPad[0] = sx_pad;
            szPad[1] = sy_pad;
            szPad[2] = sz_pad;

            V3DLONG *szSub = new V3DLONG [3];
            szSub[0] = bbsx;
            szSub[1] = bbsy;
            szSub[2] = bbsz;

            REAL* p_f_sub = NULL;
            try
            {
                p_f_sub = new REAL [len_pad];
                memset(p_f_sub, 0, sizeof(REAL)*len_pad);

                //padding zeros
                Y_IMG_REAL pOut(p_f_sub, szPad);
                Y_IMG_DATATYPE pIn(p_sub, szSub);

                YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
                tmp.padding(pOut, pIn, true, fftw_in_place, even_odd, 3);
            }
            catch(...)
            {
                printf("Fail to allocate memory.\n");
                return false;
            }

            REAL* p_f_tar = NULL;
            try
            {
                p_f_tar = new REAL [len_pad];
                memset(p_f_tar, 0, sizeof(REAL)*len_pad);

                //padding zeros
                Y_IMG_REAL pOut(p_f_tar, szPad);
                Y_IMG_DATATYPE pIn(p_tar, szSub);

                YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
                tmp.padding(pOut, pIn, false, fftw_in_place, even_odd, 3);
            }
            catch(...)
            {
                printf("Fail to allocate memory.\n");
                return false;
            }
            //de-alloc
            y_del<Tdata>(p_sub);
            y_del<Tdata>(p_tar);

            // fft-pc subspace translation estimating (Foroosh's method)
            rPEAKS pos;
            Y_IMG_REAL pOut(p_f_sub, szPad);
            Y_IMG_REAL pIn(p_f_tar, szPad);

            YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_REAL> tmp;
            tmp.fftpcsubspace3D(pOut, pIn, even_odd, fftw_in_place, &pos); // subpixel shifts

            // subpixel translation estimation
            (&vim.tilesList.at(i))->offsets_sa[0] += pos.x;
            (&vim.tilesList.at(i))->offsets_sa[1] += pos.y;
            (&vim.tilesList.at(i))->offsets_sa[2] += pos.z;

            //de-alloc
            y_del<REAL>(p_f_sub);
            y_del<REAL>(p_f_tar);
        }

        // subpixel registration using Foroosh's method
        //        rPEAKS pos;
        //        Y_IMG_REAL pOut(gol1d, szOL);
        //        Y_IMG_REAL pIn(fol1d, szOL);
        
        //        YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_REAL> tmp;
        //        tmp.fftpcsubspace3D(pOut, pIn, even_odd, fftw_in_place, &pos); // subpixel shifts
        
        //        // subpixel translation estimation
        //        (&vim.tilesList.at(j))->offsets_sa[0] += pos.x;
        //        (&vim.tilesList.at(j))->offsets_sa[1] += pos.y;
        //        (&vim.tilesList.at(j))->offsets_sa[2] += pos.z;
        
        //        // de-alloc
        //        y_del<REAL>(fol1d);
        //        y_del<REAL>(gol1d);

        else if(plane_xz == true)
        {
            // finding key-plane along y direction
            V3DLONG info_count=0, ypln=0, max_info=0;

            //
            V3DLONG weights = bbsy*0.15; // threshold
            weights /= 2;
            V3DLONG start_y=bsy+weights+1, end_y=esy-weights-1;
            for(V3DLONG j=start_y; j<end_y; j++) //
            {
                info_count = 0;

                V3DLONG offset_o_j = j*dimxol;
                for(V3DLONG k=bsz; k<esz; k++)
                {
                    V3DLONG offset_o_k = k*dimxol*dimyol;
                    for(V3DLONG i=bsx; i<esx; i++)
                    {
                        V3DLONG idx_o = offset_o_k + offset_o_j + i;

                        if( fol1d[idx_o] > meanv)
                            info_count++;
                    }
                }

                if(info_count > max_info)
                {
                    max_info = info_count;
                    ypln = j;
                }

            }

            if(ypln<start_y) ypln = start_y;

            qDebug() << "ypln ..." << ypln;

            // extraction
            V3DLONG b_bsy = ypln - weights;

            if(b_bsy>bsy)
                bsy = b_bsy;

            V3DLONG e_esy = ypln + weights;

            if(e_esy<esy)
                esy = e_esy;

            bbsy = esy-bsy+1; // dims

            V3DLONG pagesz_bb_sub = bbsx*bbsy*bbsz;
            V3DLONG pagesz_bb_tar = pagesz_bb_sub;

            // extract one plane from sub
            Tdata *p_sub = NULL, *p_tar = NULL;
            try
            {
                p_sub = new Tdata [pagesz_bb_sub];
                p_tar = new Tdata [pagesz_bb_sub];

                for(V3DLONG k=bsz; k<esz; k++)
                {
                    V3DLONG offset_k = (k-bsz)*bbsx*bbsy;
                    V3DLONG offset_o_k = k*dimxol*dimyol;
                    for(V3DLONG j=bsy; j<esy; j++)
                    {
                        V3DLONG offset_j = offset_k + (j-bsy)*bbsx;
                        V3DLONG offset_o_j = offset_o_k + j*dimxol;
                        for(V3DLONG i=bsx; i<esx; i++)
                        {
                            V3DLONG idx = offset_j + (i-bsx);
                            V3DLONG idx_o = offset_o_j + i;

                            p_sub[idx] = gol1d[idx_o];
                            p_tar[idx] = fol1d[idx_o];
                        }
                    }
                }
            }
            catch(...)
            {
                printf("Fail to allocate memory.\n");
                return false;
            }
            //
            y_del<Tdata>(fol1d);
            y_del<Tdata>(gol1d);

            //
            V3DLONG sx_pad = 2*bbsx-1, sy_pad = 2*bbsy-1, sz_pad = 2*bbsz-1;

            V3DLONG even_odd = sx_pad%2; // 0 for even 1 for odd

            if(fftw_in_place)
                sx_pad += (2-even_odd); //

            V3DLONG len_pad = sx_pad*sy_pad*sz_pad;

            V3DLONG *szPad = new V3DLONG [3];
            szPad[0] = sx_pad;
            szPad[1] = sy_pad;
            szPad[2] = sz_pad;

            V3DLONG *szSub = new V3DLONG [3];
            szSub[0] = bbsx;
            szSub[1] = bbsy;
            szSub[2] = bbsz;

            REAL* p_f_sub = NULL;
            try
            {
                p_f_sub = new REAL [len_pad];
                memset(p_f_sub, 0, sizeof(REAL)*len_pad);

                //padding zeros
                Y_IMG_REAL pOut(p_f_sub, szPad);
                Y_IMG_DATATYPE pIn(p_sub, szSub);

                YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
                tmp.padding(pOut, pIn, true, fftw_in_place, even_odd, 3);
            }
            catch(...)
            {
                printf("Fail to allocate memory.\n");
                return false;
            }

            REAL* p_f_tar = NULL;
            try
            {
                p_f_tar = new REAL [len_pad];
                memset(p_f_tar, 0, sizeof(REAL)*len_pad);

                //padding zeros
                Y_IMG_REAL pOut(p_f_tar, szPad);
                Y_IMG_DATATYPE pIn(p_tar, szSub);

                YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
                tmp.padding(pOut, pIn, false, fftw_in_place, even_odd, 3);
            }
            catch(...)
            {
                printf("Fail to allocate memory.\n");
                return false;
            }
            //de-alloc
            y_del<Tdata>(p_sub);
            y_del<Tdata>(p_tar);

            // fft-pc subspace translation estimating (Foroosh's method)
            rPEAKS pos;
            Y_IMG_REAL pOut(p_f_sub, szPad);
            Y_IMG_REAL pIn(p_f_tar, szPad);

            YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_REAL> tmp;
            tmp.fftpcsubspace3D(pOut, pIn, even_odd, fftw_in_place, &pos); // subpixel shifts

            // subpixel translation estimation
            (&vim.tilesList.at(i))->offsets_sa[0] += pos.x;
            (&vim.tilesList.at(i))->offsets_sa[1] += pos.y;
            (&vim.tilesList.at(i))->offsets_sa[2] += pos.z;

            //de-alloc
            y_del<REAL>(p_f_sub);
            y_del<REAL>(p_f_tar);
        }
        else if(plane_xy == true)
        {
            // finding key-plane along x direction
            V3DLONG info_count=0, zpln=0, max_info=0;

            //
            V3DLONG weights = bbsz*0.15; // threshold
            weights /= 2;
            V3DLONG start_z=bsz+weights+1, end_z=esz-weights-1;
            for(V3DLONG k=start_z; k<end_z; k++) //
            {
                info_count = 0;

                V3DLONG offset_o_k = k*dimxol*dimyol;
                for(V3DLONG j=bsy; j<esy; j++)
                {
                    V3DLONG offset_o_j = j*dimxol;
                    for(V3DLONG i=bsx; i<esx; i++)
                    {
                        V3DLONG idx_o = offset_o_k + offset_o_j + i;

                        if( fol1d[idx_o] > meanv)
                            info_count++;
                    }
                }

                if(info_count > max_info)
                {
                    max_info = info_count;
                    zpln = k;
                }

            }

            // extraction
            V3DLONG b_bsz = zpln - weights/2;

            if(b_bsz>bsz)
                bsz = b_bsz;

            V3DLONG e_esz = zpln + weights/2;

            if(e_esz<esz)
                esz = e_esz;

            bbsz = esz-bsz+1; // dims

            V3DLONG pagesz_bb_sub = bbsx*bbsy*bbsz;
            V3DLONG pagesz_bb_tar = pagesz_bb_sub;

            // extract one plane from sub
            Tdata *p_sub = NULL, *p_tar = NULL;
            try
            {
                p_sub = new Tdata [pagesz_bb_sub];
                p_tar = new Tdata [pagesz_bb_sub];

                for(V3DLONG k=bsz; k<esz; k++)
                {
                    V3DLONG offset_k = (k-bsz)*bbsx*bbsy;
                    V3DLONG offset_o_k = k*dimxol*dimyol;
                    for(V3DLONG j=bsy; j<esy; j++)
                    {
                        V3DLONG offset_j = offset_k + (j-bsy)*bbsx;
                        V3DLONG offset_o_j = offset_o_k + j*dimxol;
                        for(V3DLONG i=bsx; i<esx; i++)
                        {
                            V3DLONG idx = offset_j + (i-bsx);
                            V3DLONG idx_o = offset_o_j + i;

                            p_sub[idx] = gol1d[idx_o];
                            p_tar[idx] = fol1d[idx_o];
                        }
                    }
                }
            }
            catch(...)
            {
                printf("Fail to allocate memory.\n");
                return false;
            }
            //
            y_del<Tdata>(fol1d);
            y_del<Tdata>(gol1d);

            //
            V3DLONG sx_pad = 2*bbsx-1, sy_pad = 2*bbsy-1, sz_pad = 2*bbsz-1;

            V3DLONG even_odd = sx_pad%2; // 0 for even 1 for odd

            if(fftw_in_place)
                sx_pad += (2-even_odd); //

            V3DLONG len_pad = sx_pad*sy_pad*sz_pad;

            V3DLONG *szPad = new V3DLONG [3];
            szPad[0] = sx_pad;
            szPad[1] = sy_pad;
            szPad[2] = sz_pad;

            V3DLONG *szSub = new V3DLONG [3];
            szSub[0] = bbsx;
            szSub[1] = bbsy;
            szSub[2] = bbsz;

            REAL* p_f_sub = NULL;
            try
            {
                p_f_sub = new REAL [len_pad];
                memset(p_f_sub, 0, sizeof(REAL)*len_pad);

                //padding zeros
                Y_IMG_REAL pOut(p_f_sub, szPad);
                Y_IMG_DATATYPE pIn(p_sub, szSub);

                YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
                tmp.padding(pOut, pIn, true, fftw_in_place, even_odd, 3);
            }
            catch(...)
            {
                printf("Fail to allocate memory.\n");
                return false;
            }

            REAL* p_f_tar = NULL;
            try
            {
                p_f_tar = new REAL [len_pad];
                memset(p_f_tar, 0, sizeof(REAL)*len_pad);

                //padding zeros
                Y_IMG_REAL pOut(p_f_tar, szPad);
                Y_IMG_DATATYPE pIn(p_tar, szSub);

                YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
                tmp.padding(pOut, pIn, false, fftw_in_place, even_odd, 3);
            }
            catch(...)
            {
                printf("Fail to allocate memory.\n");
                return false;
            }
            //de-alloc
            y_del<Tdata>(p_sub);
            y_del<Tdata>(p_tar);

            // fft-pc subspace translation estimating (Foroosh's method)
            rPEAKS pos;
            Y_IMG_REAL pOut(p_f_sub, szPad);
            Y_IMG_REAL pIn(p_f_tar, szPad);

            YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_REAL> tmp;
            tmp.fftpcsubspace3D(pOut, pIn, even_odd, fftw_in_place, &pos); // subpixel shifts

            // subpixel translation estimation
            (&vim.tilesList.at(i))->offsets_sa[0] += pos.x;
            (&vim.tilesList.at(i))->offsets_sa[1] += pos.y;
            (&vim.tilesList.at(i))->offsets_sa[2] += pos.z;

            //de-alloc
            y_del<REAL>(p_f_sub);
            y_del<REAL>(p_f_tar);
        }

    }

    // testing
    for(int i_path=0; i_path<vim.number_tiles; i_path++)
    {
        qDebug()<<"current ..."<<vim.tilesList.at(i_path).n<<"computed offsets ..."<<vim.tilesList.at(i_path).offsets_sa[0]<<vim.tilesList.at(i_path).offsets_sa[1]<<vim.tilesList.at(i_path).offsets_sa[2];
    }

    for(int i=0; i<vim.number_tiles; i++)
    {
        vim.tilesList.at(i).visited = false;
    }

    qDebug()<<"ref ..."<<vim.tilesList.at(0).n<<"offsets ..."<<vim.tilesList.at(0).offsets_sa[0]<<vim.tilesList.at(0).offsets_sa[1]<<vim.tilesList.at(0).offsets_sa[2];

    // init offsets of ref
    vim.tilesList.at(0).offsets_sa[0] = 0;
    vim.tilesList.at(0).offsets_sa[1] = 0;
    vim.tilesList.at(0).offsets_sa[2] = 0;

    for(int i_path=1; i_path<vim.number_tiles; i_path++)
    {
        //
        V3DLONG current = vim.tilesList.at(i_path).n;
        V3DLONG previous = vim.tilesList.at(i_path).predecessor;

        qDebug()<<"current ..."<<vim.tilesList.at(i_path).n<<"offsets ..."<<vim.tilesList.at(i_path).offsets_sa[0]<<vim.tilesList.at(i_path).offsets_sa[1]<<vim.tilesList.at(i_path).offsets_sa[2];

        //
        while(previous!=-1)
        {
            if(vim.tilesList.at(current).visited)
            {
                break;
            }

            (&vim.tilesList.at(i_path))->offsets_sa[0] += vim.tilesList.at(previous).offsets_sa[0];
            (&vim.tilesList.at(i_path))->offsets_sa[1] += vim.tilesList.at(previous).offsets_sa[1];
            (&vim.tilesList.at(i_path))->offsets_sa[2] += vim.tilesList.at(previous).offsets_sa[2];

            //
            current = previous;
            previous = vim.tilesList.at(current).predecessor;
        }

        (&vim.tilesList.at(i_path))->visited = true;
    }

    // testing
    for(int i_path=0; i_path<vim.number_tiles; i_path++)
    {
        qDebug()<<"current ..."<<vim.tilesList.at(i_path).n<<"accumulate offsets ..."<<vim.tilesList.at(i_path).offsets_sa[0]<<vim.tilesList.at(i_path).offsets_sa[1]<<vim.tilesList.at(i_path).offsets_sa[2];
    }

    // blending
    for(V3DLONG ii=1; ii<vim.number_tiles; ii++)
    {
        // loading relative image files
        V3DLONG *sz_relative = 0;
        int datatype_relative = 0;
        unsigned char* relative1d = 0;
        
        //
        if (loadImage(const_cast<char *>(vim.tilesList.at(ii).fn_image.c_str()), relative1d, sz_relative, datatype_relative)!=true)
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",vim.lut[ii].fn_img.c_str());
            return false;
        }
        V3DLONG rx=sz_relative[0], ry=sz_relative[1], rz=sz_relative[2], rc=sz_relative[3];
        
        // subpixel translate
        rPEAKS pos;
        pos.x = vim.tilesList.at(ii).offsets_sa[0] - vim.lut[ii].start_pos[0];
        pos.y = vim.tilesList.at(ii).offsets_sa[1] - vim.lut[ii].start_pos[1];
        pos.z = vim.tilesList.at(ii).offsets_sa[2] - vim.lut[ii].start_pos[2];

        qDebug()<<"current ... "<<ii<<" pre ... "<<vim.tilesList.at(ii).predecessor<<"offsets ..."<<pos.x<<pos.y<<pos.z;

        REAL *prelative = NULL;
        V3DLONG even_odd = rx%2;
        V3DLONG rx_pad = rx;
        //        if(fftw_in_place)
        //            rx_pad += (2-even_odd);

        try
        {
            V3DLONG szRelative = rx_pad*ry*rz*rc;
            
            prelative = new REAL [szRelative];
            
            for(V3DLONG c=0; c<rc; c++)
            {
                V3DLONG offset_c = c*rx*ry*rz;
                V3DLONG offset_cr = c*rx_pad*ry*rz;

                for(V3DLONG k=0; k<rz; k++)
                {
                    V3DLONG offset_k = offset_c + k*rx*ry;
                    V3DLONG offset_kr = offset_cr + k*rx_pad*ry;

                    for(V3DLONG j=0; j<ry; j++)
                    {
                        V3DLONG offset_j = offset_k + j*rx;
                        V3DLONG offset_jr = offset_kr + j*rx_pad;

                        for(V3DLONG i=0; i<rx_pad; i++)
                        {
                            if(i<rx)
                                prelative[offset_jr+i] = ((Tdata*)relative1d)[offset_j+i];
                            else
                                prelative[offset_jr+i] = 0.0;
                        }
                    }
                }
            }
            
        }
        catch (...)
        {
            y_del<REAL>(prelative);
            
            cout<<"Fail to allocate memory!"<<endl;
            return false;
        }

        //        for(V3DLONG c=0; c<rc; c++)
        //        {
        //            V3DLONG offset_c = c*rx_pad*ry*rz;

        //            Y_IMG_REAL pOut(prelative+offset_c, sz_relative);


        //            YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_REAL> tmp;
        //            tmp.subpixeltranslate(pOut, even_odd, fftw_in_place, &pos);
        //        }

        V3DLONG *effectiveEnvelope = new V3DLONG [6];
        if(subpixshift3D<Tdata>(prelative, (Tdata *)relative1d, sz_relative, pos, effectiveEnvelope)!=true)
        {
            cout<<"Fail to subpixel translate image!"<<endl;
            return false;
        }

        //
        V3DLONG tile2vi_xs = vim.lut[ii].start_pos[0]-vim.min_vim[0];
        V3DLONG tile2vi_xe = vim.lut[ii].end_pos[0]-vim.min_vim[0];
        V3DLONG tile2vi_ys = vim.lut[ii].start_pos[1]-vim.min_vim[1];
        V3DLONG tile2vi_ye = vim.lut[ii].end_pos[1]-vim.min_vim[1];
        V3DLONG tile2vi_zs = vim.lut[ii].start_pos[2]-vim.min_vim[2];
        V3DLONG tile2vi_ze = vim.lut[ii].end_pos[2]-vim.min_vim[2];
        
        V3DLONG x_start = (0 > tile2vi_xs) ? 0 : tile2vi_xs;
        V3DLONG x_end = (vx-1 < tile2vi_xe) ? vx-1 : tile2vi_xe;
        V3DLONG y_start = (0 > tile2vi_ys) ? 0 : tile2vi_ys;
        V3DLONG y_end = (vy-1 < tile2vi_ye) ? vy-1 : tile2vi_ye;
        V3DLONG z_start = (0 > tile2vi_zs) ? 0 : tile2vi_zs;
        V3DLONG z_end = (vz-1 < tile2vi_ze) ? vz-1 : tile2vi_ze;
        
        x_end++;
        y_end++;
        z_end++;

        // fit into envelope from subspace translation
        x_start += effectiveEnvelope[0];
        x_end -= rx - effectiveEnvelope[1];

        y_start += effectiveEnvelope[2];
        y_end -= ry - effectiveEnvelope[3];

        z_start += effectiveEnvelope[4];
        z_end -= rz - effectiveEnvelope[5];
        
        //suppose all tiles with same color dimensions
        if(rc>vc)
            rc = vc;
        
        //        
        for(V3DLONG c=0; c<rc; c++)
        {
            V3DLONG o_c = c*vx*vy*vz;
            V3DLONG o_r_c = c*rx_pad*ry*rz;
            for(V3DLONG k=z_start; k<z_end; k++)
            {
                V3DLONG k_idx = k-z_start + effectiveEnvelope[4];
                if(k_idx>=effectiveEnvelope[5]) continue;

                V3DLONG o_k = o_c + k*vx*vy;
                V3DLONG o_r_k = o_r_c + (k_idx)*rx_pad*ry;
                
                for(V3DLONG j=y_start; j<y_end; j++)
                {
                    V3DLONG j_idx = j-y_start + effectiveEnvelope[2];
                    if(j_idx>=effectiveEnvelope[3]) continue;

                    V3DLONG o_j = o_k + j*vx;
                    V3DLONG o_r_j = o_r_k + (j_idx)*rx_pad;
                    for(V3DLONG i=x_start; i<x_end; i++)
                    {
                        V3DLONG i_idx = i-x_start + effectiveEnvelope[0];
                        if(i_idx>=effectiveEnvelope[1]) continue;

                        V3DLONG idx = o_j + i;
                        V3DLONG idx_r = o_r_j + (i_idx);
                        
                        REAL val = (REAL)(prelative[idx_r]);
                        
                        //
                        REAL coef;
                        if(!computeWeights(vim, i, j, k, ii, coef) )
                        {
                            printf("Fail to call function computeWeights!\n");
                            return false;
                        }
                        
                        pTmp[idx] += (Tdata)(val*coef); // linear blending
                        
                    }
                }
            }
        }
        
        //de-alloc
        y_del<REAL>(prelative);
        y_del<unsigned char>(relative1d);
        y_del<V3DLONG>(sz_relative);
    }
    
    float minval, maxval;
    for(V3DLONG c=0; c<vc; c++) 
    {
        V3DLONG offsets = c*pagesz;
        
        minval=1e9;
        maxval=-1e9;
        for (V3DLONG i=0; i<pagesz; i++) 
        {
            V3DLONG idx = offsets+i;
            
            float val=pTmp[idx];
            
            if(minval>val) minval = val;
            if(maxval<val) maxval = val;
        }
        maxval -= minval;
        
        for (V3DLONG i=0; i<pagesz; i++) 
        {
            V3DLONG idx = offsets+i;
            
            pVImg[idx] = (Tdata) ((REAL)intensityrange * (pTmp[idx] - minval)/maxval);
        }
    }
    
    //de-alloc
    y_del<REAL>(pTmp);
    
    return true;
}

// intensity-based multiscale pairwise-image stitching (coarse-to-fine)
template <class SDATATYPE, class Y_IMG_DATATYPE>
int pistitching(SDATATYPE *subject1d, V3DLONG *sz_subject1d, SDATATYPE *target1d, V3DLONG *sz_target1d, REAL overlap_percent, REAL *scale, rPEAKS &pos)
{
    // input:  pairwise images (1d pointers of 3D images)
    // output: subspace shifts

    bool fftw_in_place = true; //
    bool b_SubpixelEst = false; // true to do subspace pair-wise stitching

    //
    V3DLONG sx = sz_subject1d[0];
    V3DLONG sy = sz_subject1d[1];
    V3DLONG sz = sz_subject1d[2];
    //V3DLONG sc = sz_subject1d[3];

    V3DLONG pagesz_sub = sx*sy*sz;

    V3DLONG tx = sz_target1d[0];
    V3DLONG ty = sz_target1d[1];
    V3DLONG tz = sz_target1d[2];
    //V3DLONG tc = sz_target1d[3];

    V3DLONG pagesz_tar = tx*ty*tz;

    //
    size_t start_t = clock();

    // sampling by factors less than 1 (downsample)
    // obtain an approximate translative estimation at a coarse scale
    //--------------------------------------------------------------------------------------
    V3DLONG ssx, ssy, ssz, stx, sty, stz;

    V3DLONG sx_ori = sx;
    V3DLONG sy_ori = sy;
    V3DLONG sz_ori = sz;

    V3DLONG tx_ori = tx;
    V3DLONG ty_ori = ty;
    V3DLONG tz_ori = tz;

    // anchor position
    REAL pos_score = 0.0;
    REAL pos_x=0.0, pos_y=0.0, pos_z=0.0;

    //
    ssx = (V3DLONG)(ceil(sx*scale[0]));
    ssy = (V3DLONG)(ceil(sy*scale[1]));
    ssz = (V3DLONG)(ceil(sz*scale[2]));

    stx = (V3DLONG)(ceil(tx*scale[0]));
    sty = (V3DLONG)(ceil(ty*scale[1]));
    stz = (V3DLONG)(ceil(tz*scale[2]));

    SDATATYPE* p_sub = NULL;
    SDATATYPE* p_tar = NULL;

    if(scale[0]==1 && scale[1]==1 && scale[2]==1)
    {
        p_sub = subject1d;
        p_tar = target1d;
    }
    else
    {
        // downsampling
        V3DLONG pagesz_ds_sub = ssx*ssy*ssz;
        V3DLONG pagesz_ds_tar = stx*sty*stz;

        V3DLONG *sz_down_target = NULL;

        y_new<V3DLONG, V3DLONG>(sz_down_target, 3);

        sz_down_target[0] = stx;
        sz_down_target[1] = sty;
        sz_down_target[2] = stz;

        V3DLONG *sz_down_subject = NULL;

        y_new<V3DLONG, V3DLONG>(sz_down_subject, 3);

        sz_down_subject[0] = ssx;
        sz_down_subject[1] = ssy;
        sz_down_subject[2] = ssz;

        y_new<SDATATYPE, V3DLONG>(p_sub, pagesz_ds_sub);
        if(p_sub)
        {
            //
            Y_IMG_DATATYPE pIn(subject1d, sz_subject1d);
            Y_IMG_DATATYPE pOut(p_sub, sz_down_subject);

            YImg<REAL, V3DLONG, Y_IMG_DATATYPE, Y_IMG_DATATYPE> tmp;
            tmp.down_sampling(pOut, pIn, scale);
        }

        y_new<SDATATYPE, V3DLONG>(p_tar, pagesz_ds_tar);
        if(p_tar)
        {
            //
            Y_IMG_DATATYPE pIn(target1d, sz_target1d);
            Y_IMG_DATATYPE pOut(p_tar, sz_down_target);

            YImg<REAL, V3DLONG, Y_IMG_DATATYPE, Y_IMG_DATATYPE> tmp;
            tmp.down_sampling(pOut, pIn, scale);
        }

    }
    sx = ssx; sy = ssy; sz = ssz;
    tx = stx; ty = sty; tz = stz;

    // zero padding
    V3DLONG sx_pad = sx+tx-1, sy_pad = sy+ty-1, sz_pad = sz+tz-1;

    V3DLONG even_odd = sx_pad%2; // 0 for even 1 for odd

    if(fftw_in_place)
        sx_pad += (2-even_odd); //2*(sx_pad/2 + 1); // fftw_in_place

    V3DLONG len_pad = sx_pad*sy_pad*sz_pad;

    V3DLONG *szPad = NULL;
    V3DLONG *szTar = NULL;
    V3DLONG *szSub = NULL;

    y_new<V3DLONG, V3DLONG>(szPad, 3);
    y_new<V3DLONG, V3DLONG>(szTar, 3);
    y_new<V3DLONG, V3DLONG>(szSub, 3);

    //
    szPad[0] = sx_pad;
    szPad[1] = sy_pad;
    szPad[2] = sz_pad;

    //
    szTar[0] = tx;
    szTar[1] = ty;
    szTar[2] = tz;

    //
    szSub[0] = sx;
    szSub[1] = sy;
    szSub[2] = sz;

    // padding target with zeros
    REAL *pTar = NULL;

    y_new<REAL, V3DLONG>(pTar, len_pad);
    if(pTar)
    {
        memset(pTar, 0.0, sizeof(REAL)*len_pad);

        //
        Y_IMG_REAL pOut(pTar, szPad);
        Y_IMG_DATATYPE pIn(p_tar, szTar);

        YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
        tmp.padding(pOut, pIn, false, fftw_in_place, even_odd, 3);
    }
    y_del<SDATATYPE>(p_tar);

    // padding subject with zeros
    REAL *pSub = NULL;

    y_new<REAL, V3DLONG>(pSub, len_pad);
    if(pSub)
    {
        memset(pSub, 0.0, sizeof(REAL)*len_pad);

        //
        Y_IMG_REAL pOut(pSub, szPad);
        Y_IMG_DATATYPE pIn(p_sub, szSub);

        YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
        tmp.padding(pOut, pIn, true, fftw_in_place, even_odd, 3);
    }
    y_del<SDATATYPE>(p_sub);

    //pc-ncc
    if(pSub && pTar)
    {
        PEAKS pos_pcncc;
        Y_IMG_REAL pOut(pSub, szPad);
        Y_IMG_REAL pIn(pTar, szPad);

        YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_REAL> tmp;
        tmp.fftpcncc3D(pOut, pIn, szTar, even_odd, fftw_in_place, overlap_percent, &pos_pcncc);

        pos_x = pos_pcncc.x; pos_y = pos_pcncc.y; pos_z = pos_pcncc.z; pos_score = pos_pcncc.value;
    }

    // coarse-scale offsets estimated
    pos_x /= scale[0]; pos_y /= scale[1]; pos_z /= scale[2];

    qDebug() << " coarse-scale estimating ..." << pos_x << pos_y << pos_z << "response ..." << pos_score;

    //de-alloc
    y_del<REAL>(pTar);
    y_del<REAL>(pSub);

    // choose a VOI from sub image then register it with VOI extracted from the tar image
    // obtain an pixel-precise translation estimation at a fine scale
    //--------------------------------------------------------------------------------------
    if(!(scale[0]==1 && scale[1]==1 && scale[2]==1))
    {
        sx = sx_ori; sy = sy_ori; sz = sz_ori;
        tx = tx_ori; ty = ty_ori; tz = tz_ori;

        //
        REAL offset[3];
        offset[0] = pos_x - sx+1;
        offset[1] = pos_y - sy+1;
        offset[2] = pos_z - sz+1;

        V3DLONG nbbx, nbby, nbbz;

        nbbx = 1/scale[0] + 2;
        nbby = 1/scale[1] + 2;
        nbbz = 1/scale[2] + 2;

        V3DLONG bsx = (offset[0]>0)?0:-offset[0];
        V3DLONG bsy = (offset[1]>0)?0:-offset[1];
        V3DLONG bsz = (offset[2]>0)?0:-offset[2];

        if(bsx-nbbx>=0)
            bsx -= nbbx;
        else
            bsx = 0;

        if(bsy-nbby>=0)
            bsy -= nbby;
        else
            bsy = 0;

        if(bsz-nbbz>=0)
            bsz -= nbbz;
        else
            bsz = 0;

        V3DLONG esx = (sx-1);
        V3DLONG esy = (sy-1);
        V3DLONG esz = (sz-1);

        if(offset[0])
        {
            V3DLONG tmp = sx-1 + offset[0] - (tx-1);

            if(tmp>0)
                esx = sx-1 -tmp;
        }

        if(offset[1])
        {
            V3DLONG tmp = offset[1]+sy-1 - (ty-1);

            if(tmp>0)
                esy = sy-1 - tmp;
        }

        if(offset[2])
        {
            V3DLONG tmp = offset[2]+sz-1 - (tz-1);

            if(tmp>0)
                esz = sz-1 -tmp;
        }

        if(esx+nbbx<=sx-1)
            esx += nbbx;
        else
            esx = sx-1;

        if(esy+nbby<=sy-1)
            esy += nbby;
        else
            esy = sy-1;

        if(esz+nbbz<=sz-1)
            esz += nbbz;
        else
            esz = sz-1;

        V3DLONG btx = (offset[0]>0)?offset[0]:0;
        V3DLONG bty = (offset[1]>0)?offset[1]:0;
        V3DLONG btz = (offset[2]>0)?offset[2]:0;

        if(btx-nbbx>=0)
            btx -= nbbx;
        else
            btx = 0;

        if(bty-nbby>=0)
            bty -= nbby;
        else
            bty = 0;

        if(btz-nbbz>=0)
            btz -= nbbz;
        else
            btz = 0;

        V3DLONG etx = (offset[0]>0)?(tx-1):pos_x;
        V3DLONG ety = (offset[1]>0)?(ty-1):pos_y;
        V3DLONG etz = (offset[2]>0)?(tz-1):pos_z;

        if(offset[0]>0)
        {
            V3DLONG tmp = (tx-1) - (sx-1 + offset[0]);

            if(tmp>0)
                etx = tx-1 - tmp;
        }

        if(offset[1]>0)
        {
            V3DLONG tmp = (ty-1) - (sy-1 + offset[1]);

            if(tmp>0)
                ety = ty-1 - tmp;
        }

        if(offset[2]>0)
        {
            V3DLONG tmp = (tz-1) - (sz-1 + offset[2]);

            if(tmp>0)
                etz = tz-1 - tmp;
        }

        if(etx+nbbx<=tx-1)
            etx += nbbx;
        else
            etx = tx-1;

        if(ety+nbby<=ty-1)
            ety += nbby;
        else
            ety = ty-1;

        if(etz+nbbz<=tz-1)
            etz += nbbz;
        else
            etz = tz-1;

        V3DLONG bbsx = esx - bsx + 1; if(bbsx<0 || bbsx>sx) return -1;
        V3DLONG bbsy = esy - bsy + 1; if(bbsy<0 || bbsy>sy) return -1;
        V3DLONG bbsz = esz - bsz + 1; if(bbsz<0 || bbsz>sz) return -1;

        V3DLONG bbtx = etx - btx + 1; if(bbtx<0 || bbtx>tx) return -1;
        V3DLONG bbty = ety - bty + 1; if(bbty<0 || bbty>ty) return -1;
        V3DLONG bbtz = etz - btz + 1; if(bbtz<0 || bbtz>tz) return -1;

        REAL rate_x, rate_y, rate_z;

        rate_x = qMax( (REAL)bbsx/(REAL)sx, (REAL)bbtx/(REAL)tx);
        rate_y = qMax( (REAL)bbsy/(REAL)sy, (REAL)bbty/(REAL)ty);
        rate_z = qMax( (REAL)bbsz/(REAL)sz, (REAL)bbtz/(REAL)tz);

        // key planes
        bool plane_yz=false, plane_xz=false, plane_xy=false;

        if(rate_x < rate_y && rate_x < rate_z)
        {
            plane_yz = true;

            bsy = 0; esy = sy-1;
            bbsy = sy;

            bty = 0; ety = ty-1;
            bbty = ty;

            bsz = 0; esz = sz-1;
            bbsz = sz;

            btz = 0; etz = tz-1;
            bbtz = tz;

        }
        if(rate_y < rate_x && rate_y < rate_z)
        {
            plane_xz = true;

            bsx = 0; esx = sx-1;
            bbsx = sx;

            btx = 0; etx = tx-1;
            bbtx = tx;

            bsz = 0; esz = sz-1;
            bbsz = sz;

            btz = 0; etz = tz-1;
            bbtz = tz;

        }
        if(rate_z < rate_x && rate_z < rate_y)
        {
            plane_xy = true;

            bsx = 0; esx = sx-1;
            bbsx = sx;

            btx = 0; etx = tx-1;
            bbtx = tx;

            bsy = 0; esy = sy-1;
            bbsy = sy;

            bty = 0; ety = ty-1;
            bbty = ty;

        }

        // using meanv as a threshold to tell foreground from background
        REAL sum=0;
        for(V3DLONG k=bsz; k<=esz; k++)
        {
            V3DLONG offset_o_k = k*sx*sy;
            for(V3DLONG j=bsy; j<=esy; j++)
            {
                V3DLONG offset_o_j = offset_o_k + j*sx;
                for(V3DLONG i=bsx; i<=esx; i++)
                {
                    V3DLONG idx_o = offset_o_j + i;

                    sum += subject1d[idx_o];
                }
            }
        }
        SDATATYPE meanv = (SDATATYPE) (sum/(REAL)(bbsx*bbsy*bbsz));

        // extract VOI according to key plane
        if(plane_yz == true)
        {
            // finding key plane from the subject along x
            V3DLONG info_count=0, xpln=0, max_info=0;

            //
            V3DLONG weights = bbsx*0.15;
            weights /= 2;
            V3DLONG start_x=bsx+weights+nbbx, end_x=esx-weights-nbbx;
            for(V3DLONG i=start_x; i<=end_x; i++) //
            {
                info_count = 0;

                for(V3DLONG k=bsz; k<=esz; k++)
                {
                    V3DLONG offset_o_k = k*sx*sy;
                    for(V3DLONG j=bsy; j<=esy; j++)
                    {
                        V3DLONG idx_o = offset_o_k + j*sx + i;

                        if( subject1d[idx_o] > meanv)
                            info_count++;
                    }
                }

                if(info_count > max_info)
                {
                    max_info = info_count;
                    xpln = i;
                }

            }

            if(xpln<start_x) xpln = start_x;

            // VOI extraction
            V3DLONG b_bsx = xpln - weights; //

            if(b_bsx>bsx)
                bsx = b_bsx;

            V3DLONG e_esx = xpln + weights;

            if(e_esx<esx)
                esx = e_esx;

            bbsx = esx-bsx+1; // dims

            // crop corresponding thick planes from tar image
            V3DLONG b_btx = bsx + offset[0] - nbbx;

            if(b_btx>btx)
                btx = b_btx;

            V3DLONG e_etx = esx + offset[0] + nbbx;

            if(e_etx<etx)
                etx = e_etx;

            bbtx = etx-btx+1;

            V3DLONG pagesz_bb_sub = bbsx*bbsy*bbsz;
            V3DLONG pagesz_bb_tar = bbtx*bbty*bbtz;

            // extract one plane from sub
            y_new<SDATATYPE, V3DLONG>(p_sub, pagesz_bb_sub);
            if(p_sub)
            {
                for(V3DLONG k=bsz; k<=esz; k++)
                {
                    V3DLONG offset_k = (k-bsz)*bbsx*bbsy;
                    V3DLONG offset_o_k = k*sx*sy;
                    for(V3DLONG j=bsy; j<=esy; j++)
                    {
                        V3DLONG offset_j = offset_k + (j-bsy)*bbsx;
                        V3DLONG offset_o_j = offset_o_k + j*sx;
                        for(V3DLONG i=bsx; i<=esx; i++)
                        {
                            V3DLONG idx = offset_j + (i-bsx);
                            V3DLONG idx_o = offset_o_j + i;

                            p_sub[idx] = subject1d[idx_o];
                        }
                    }
                }
            }

            y_new<SDATATYPE, V3DLONG>(p_tar, pagesz_bb_tar);
            if(p_tar)
            {
                //REAL max_response = 0;
                // search the best match plane from tar
                for(V3DLONG k=btz; k<=etz; k++)
                {
                    V3DLONG offset_k = (k-btz)*bbtx*bbty;
                    V3DLONG offset_o_k = k*tx*ty;
                    for(V3DLONG j=bty; j<=ety; j++)
                    {
                        V3DLONG offset_j = offset_k + (j-bty)*bbtx;
                        V3DLONG offset_o_j = offset_o_k + j*tx;
                        for(V3DLONG i=btx; i<=etx; i++)
                        {
                            V3DLONG idx = offset_j + (i-btx);
                            V3DLONG idx_o = offset_o_j + i;

                            p_tar[idx] = target1d[idx_o];
                        }
                    }
                }
            }

            //
            sx = bbsx; sy = bbsy; sz = bbsz;
            tx = bbtx; ty = bbty; tz = bbtz;

            V3DLONG sx_pad = sx+tx-1, sy_pad = sy+ty-1, sz_pad = sz+tz-1;

            V3DLONG even_odd = sx_pad%2; // 0 for even 1 for odd

            if(fftw_in_place)
                sx_pad += (2-even_odd); //2*(sx_pad/2 + 1); // fftw_in_place

            V3DLONG len_pad = sx_pad*sy_pad*sz_pad;

            szPad[0] = sx_pad;
            szPad[1] = sy_pad;
            szPad[2] = sz_pad;

            szTar[0] = tx;
            szTar[1] = ty;
            szTar[2] = tz;

            szSub[0] = sx;
            szSub[1] = sy;
            szSub[2] = sz;

            REAL *p_f_sub = NULL;
            y_new<REAL, V3DLONG>(p_f_sub, len_pad);
            if(p_f_sub)
            {
                // init
                memset(p_f_sub, 0, sizeof(REAL)*len_pad);

                //padding zeros
                Y_IMG_REAL pOut(p_f_sub, szPad);
                Y_IMG_DATATYPE pIn(p_sub, szSub);

                YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
                tmp.padding(pOut, pIn, true, fftw_in_place, even_odd, 3);
            }

            REAL *p_f_tar = NULL;
            y_new<REAL, V3DLONG>(p_f_tar, len_pad);
            if(p_f_tar)
            {
                // init
                memset(p_f_tar, 0, sizeof(REAL)*len_pad);

                //padding zeros for target imag
                Y_IMG_REAL pOut(p_f_tar, szPad);
                Y_IMG_DATATYPE pIn(p_tar, szTar);

                YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
                tmp.padding(pOut, pIn, false, fftw_in_place, even_odd, 3);
            }

            //de-alloc
            y_del<SDATATYPE>(p_sub);
            y_del<SDATATYPE>(p_tar);

            // fft-ncc
            PEAKS pos_pcncc;

            pos_pcncc.x = (bsx - ((sx_ori-1) - pos_x)) - btx + (bbsx-1);
            pos_pcncc.y = pos_y;
            pos_pcncc.z = pos_z;

            Y_IMG_REAL pOutPix(p_f_sub, szPad);
            Y_IMG_REAL pInPix(p_f_tar, szPad);

            YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_REAL> pixEst;
            pixEst.fftnccp3D(pOutPix, pInPix, szSub, szTar, even_odd, fftw_in_place, scale, &pos_pcncc);

            //pos_score = pos_pcncc.value;
            pos_x = (sx_ori-1) - (bsx - (btx + pos_pcncc.x - (bbsx-1))); pos_y = pos_pcncc.y; pos_z = pos_pcncc.z; //

            //de-alloc
            y_del<REAL>(p_f_sub);
            y_del<REAL>(p_f_tar);

        }
        else if(plane_xz == true)
        {
            // finding key plane from sub along y
            V3DLONG info_count=0, ypln, max_info=0;

            V3DLONG weights = bbsy*0.15;
            weights /= 2;
            V3DLONG start_y=bsy+weights+nbby, end_y=esy-weights-nbby;
            for(V3DLONG j=start_y; j<=end_y; j++) //
            {
                info_count = 0;

                V3DLONG offset_o_j = j*sx;
                for(V3DLONG k=bsz; k<=esz; k++)
                {
                    V3DLONG offset_o_k = k*sx*sy;
                    for(V3DLONG i=bsx; i<=esx; i++)
                    {
                        V3DLONG idx_o = offset_o_k + offset_o_j + i;

                        if( subject1d[idx_o] > meanv)
                            info_count++;
                    }
                }

                if(info_count > max_info)
                {
                    max_info = info_count;
                    ypln = j;
                }

            }

            // extraction
            V3DLONG b_bsy = ypln - weights;

            if(b_bsy>bsy)
                bsy = b_bsy;

            V3DLONG e_esy = ypln + weights;

            if(e_esy<esy)
                esy = e_esy;

            bbsy = esy-bsy+1; // dims

            // extract corresponding VOI from tar image
            V3DLONG b_bty = bsy + offset[1] - nbby; //ypln + offset[1] - nbby;

            if(b_bty>bty)
                bty = b_bty;

            V3DLONG e_ety = esy + offset[1] + nbby; //ypln + offset[1] + nbby;

            if(e_ety<ety)
                ety = e_ety;

            bbty = ety-bty+1;

            V3DLONG pagesz_bb_sub = bbsx*bbsy*bbsz;
            V3DLONG pagesz_bb_tar = bbtx*bbty*bbtz;

            // extract VOI from sub
            y_new<SDATATYPE, V3DLONG>(p_sub,pagesz_bb_sub);
            if(p_sub)
            {
                for(V3DLONG k=bsz; k<=esz; k++)
                {
                    V3DLONG offset_k = (k-bsz)*bbsx*bbsy;
                    V3DLONG offset_o_k = k*sx*sy;
                    for(V3DLONG j=bsy; j<=esy; j++)
                    {
                        V3DLONG offset_j = offset_k + (j-bsy)*bbsx;
                        V3DLONG offset_o_j = offset_o_k + j*sx;
                        for(V3DLONG i=bsx; i<=esx; i++)
                        {
                            V3DLONG idx = offset_j + (i-bsx);
                            V3DLONG idx_o = offset_o_j + i;

                            p_sub[idx] = subject1d[idx_o];
                        }
                    }
                }
            }

            y_new<SDATATYPE, V3DLONG>(p_tar,pagesz_bb_tar);
            if(p_tar)
            {
                //REAL max_response = 0;
                // search the best match plane from tar
                for(V3DLONG k=btz; k<=etz; k++)
                {
                    V3DLONG offset_k = (k-btz)*bbtx*bbty;
                    V3DLONG offset_o_k = k*tx*ty;
                    for(V3DLONG j=bty; j<=ety; j++)
                    {
                        V3DLONG offset_j = offset_k + (j-bty)*bbtx;
                        V3DLONG offset_o_j = offset_o_k + j*tx;
                        for(V3DLONG i=btx; i<=etx; i++)
                        {
                            V3DLONG idx = offset_j + (i-btx);
                            V3DLONG idx_o = offset_o_j + i;

                            p_tar[idx] = target1d[idx_o];
                        }
                    }
                }
            }

            //
            sx = bbsx; sy = bbsy; sz = bbsz;
            tx = bbtx; ty = bbty; tz = bbtz;

            V3DLONG sx_pad = sx+tx-1, sy_pad = sy+ty-1, sz_pad = sz+tz-1;

            V3DLONG even_odd = sx_pad%2; // 0 for even 1 for odd

            if(fftw_in_place)
                sx_pad += (2-even_odd); //2*(sx_pad/2 + 1); // fftw_in_place

            V3DLONG len_pad = sx_pad*sy_pad*sz_pad;

            szPad[0] = sx_pad;
            szPad[1] = sy_pad;
            szPad[2] = sz_pad;

            szTar[0] = tx;
            szTar[1] = ty;
            szTar[2] = tz;

            szSub[0] = sx;
            szSub[1] = sy;
            szSub[2] = sz;

            REAL *p_f_sub = NULL;
            y_new<REAL, V3DLONG>(p_f_sub, len_pad);
            if(p_f_sub)
            {
                // init
                memset(p_f_sub, 0, sizeof(REAL)*len_pad);

                //padding zeros for target imag
                Y_IMG_REAL pOut(p_f_sub, szPad);
                Y_IMG_DATATYPE pIn(p_sub, szSub);

                YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
                tmp.padding(pOut, pIn, true, fftw_in_place, even_odd, 3);
            }

            REAL *p_f_tar = NULL;
            y_new<REAL, V3DLONG>(p_f_tar, len_pad);
            if(p_f_tar)
            {
                // init
                memset(p_f_tar, 0, sizeof(REAL)*len_pad);

                //padding zeros for target imag
                Y_IMG_REAL pOut(p_f_tar, szPad);
                Y_IMG_DATATYPE pIn(p_tar, szTar);

                YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
                tmp.padding(pOut, pIn, false, fftw_in_place, even_odd, 3);
            }

            //de-alloc
            y_del<SDATATYPE>(p_sub);
            y_del<SDATATYPE>(p_tar);

            // fft-ncc
            PEAKS pos_pcncc;

            pos_pcncc.x = pos_x;
            pos_pcncc.y =  (bsy - ((sy_ori-1) - pos_y)) - bty + (bbsy-1);
            pos_pcncc.z = pos_z;

            Y_IMG_REAL pOut(p_f_sub, szPad);
            Y_IMG_REAL pIn(p_f_tar, szPad);

            YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_REAL> tmp;
            tmp.fftnccp3D(pOut, pIn, szSub, szTar, even_odd, fftw_in_place, scale, &pos_pcncc);

            //pos_score = pos_pcncc.value;
            pos_x = pos_pcncc.x; pos_y = (sy_ori-1) - (bsy - (bty + pos_pcncc.y - (bbsy-1))); pos_z = pos_pcncc.z; //

            //de-alloc
            y_del<REAL>(p_f_sub);
            y_del<REAL>(p_f_tar);

        }
        else if(plane_xy == true)
        {
            // finding key plane from subject along z
            V3DLONG info_count=0, zpln, max_info=0;

            V3DLONG weights = bbsz*0.15;
            weights /= 2;
            V3DLONG start_z=bsz+weights+nbbz, end_z=esz-weights-nbbz;
            for(V3DLONG k=start_z; k<=end_z; k++) //
            {
                info_count = 0;

                V3DLONG offset_o_k = k*sx*sy;
                for(V3DLONG j=bsy; j<=esy; j++)
                {
                    V3DLONG offset_o_j = j*sx;
                    for(V3DLONG i=bsx; i<=esx; i++)
                    {
                        V3DLONG idx_o = offset_o_k + offset_o_j + i;

                        if( subject1d[idx_o] > meanv)
                            info_count++;
                    }
                }

                if(info_count > max_info)
                {
                    max_info = info_count;
                    zpln = k;
                }

            }

            // extraction
            V3DLONG b_bsz = zpln - weights/2;

            if(b_bsz>bsz)
                bsz = b_bsz;

            V3DLONG e_esz = zpln + weights/2;

            if(e_esz<esz)
                esz = e_esz;

            bbsz = esz-bsz+1; // dims

            // crop corresponding thick planes from tar image
            V3DLONG b_btz = bsz + offset[2] - nbbz;

            if(b_btz>btz)
                btz = b_btz;

            V3DLONG e_etz = esz + offset[2] + nbbz;

            if(e_etz<etz)
                etz = e_etz;

            bbtz = etz-btz+1;

            V3DLONG pagesz_bb_sub = bbsx*bbsy*bbsz;
            V3DLONG pagesz_bb_tar = bbtx*bbty*bbtz;

            // extract key plane from sub
            y_new<SDATATYPE, V3DLONG>(p_sub, pagesz_bb_sub);
            if(p_sub)
            {
                for(V3DLONG k=bsz; k<=esz; k++)
                {
                    V3DLONG offset_k = (k-bsz)*bbsx*bbsy;
                    V3DLONG offset_o_k = k*sx*sy;
                    for(V3DLONG j=bsy; j<=esy; j++)
                    {
                        V3DLONG offset_j = offset_k + (j-bsy)*bbsx;
                        V3DLONG offset_o_j = offset_o_k + j*sx;
                        for(V3DLONG i=bsx; i<=esx; i++)
                        {
                            V3DLONG idx = offset_j + (i-bsx);
                            V3DLONG idx_o = offset_o_j + i;

                            p_sub[idx] = subject1d[idx_o];
                        }
                    }
                }
            }

            y_new<SDATATYPE, V3DLONG>(p_tar, pagesz_bb_tar);
            if(p_tar)
            {
                //REAL max_response = 0;
                // search the best match plane from tar
                for(V3DLONG k=btz; k<=etz; k++)
                {
                    V3DLONG offset_k = (k-btz)*bbtx*bbty;
                    V3DLONG offset_o_k = k*tx*ty;
                    for(V3DLONG j=bty; j<=ety; j++)
                    {
                        V3DLONG offset_j = offset_k + (j-bty)*bbtx;
                        V3DLONG offset_o_j = offset_o_k + j*tx;
                        for(V3DLONG i=btx; i<=etx; i++)
                        {
                            V3DLONG idx = offset_j + (i-btx);
                            V3DLONG idx_o = offset_o_j + i;

                            p_tar[idx] = target1d[idx_o];
                        }
                    }
                }

            }

            //
            sx = bbsx; sy = bbsy; sz = bbsz;
            tx = bbtx; ty = bbty; tz = bbtz;

            V3DLONG sx_pad = sx+tx-1, sy_pad = sy+ty-1, sz_pad = sz+tz-1;

            V3DLONG even_odd = sx_pad%2; // 0 for even 1 for odd

            if(fftw_in_place)
                sx_pad += (2-even_odd); //2*(sx_pad/2 + 1); // fftw_in_place

            V3DLONG len_pad = sx_pad*sy_pad*sz_pad;

            szPad[0] = sx_pad;
            szPad[1] = sy_pad;
            szPad[2] = sz_pad;

            szTar[0] = tx;
            szTar[1] = ty;
            szTar[2] = tz;

            szSub[0] = sx;
            szSub[1] = sy;
            szSub[2] = sz;

            REAL *p_f_sub = NULL;
            y_new<REAL, V3DLONG>(p_f_sub, len_pad);
            if(p_f_sub)
            {
                // init
                memset(p_f_sub, 0, sizeof(REAL)*len_pad);

                //padding zeros for target imag
                Y_IMG_REAL pOut(p_f_sub, szPad);
                Y_IMG_DATATYPE pIn(p_sub, szSub);

                YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
                tmp.padding(pOut, pIn, true, fftw_in_place, even_odd, 3);
            }

            REAL *p_f_tar = NULL;
            y_new<REAL, V3DLONG>(p_f_tar, len_pad);
            if(p_f_tar)
            {
                // init
                memset(p_f_tar, 0, sizeof(REAL)*len_pad);

                //padding zeros for target imag
                Y_IMG_REAL pOut(p_f_tar, szPad);
                Y_IMG_DATATYPE pIn(p_tar, szTar);

                YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
                tmp.padding(pOut, pIn, false, fftw_in_place, even_odd, 3);
            }

            //de-alloc
            y_del<SDATATYPE>(p_sub);
            y_del<SDATATYPE>(p_tar);

            // fft-ncc
            PEAKS pos_pcncc;

            pos_pcncc.x = pos_x;
            pos_pcncc.y = pos_y;
            pos_pcncc.z = (bsz - ((sz_ori-1) - pos_z)) - btz + (bbsz-1);

            Y_IMG_REAL pOut(p_f_sub, szPad);
            Y_IMG_REAL pIn(p_f_tar, szPad);

            YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_REAL> tmp;
            tmp.fftnccp3D(pOut, pIn, szSub, szTar, even_odd, fftw_in_place, scale, &pos_pcncc);

            //pos_score = pos_pcncc.value;

            pos_x = pos_pcncc.x; pos_y = pos_pcncc.y; pos_z = (sz_ori-1) - (bsz - (btz + pos_pcncc.z - (bbsz-1))); //

            //de-alloc
            y_del<REAL>(p_f_sub);
            y_del<REAL>(p_f_tar);

        }

        qDebug() << " pixel-precise translation ..." << pos_x << pos_y << pos_z << "offset ..." << pos_x - sx_ori +1 << pos_y - sy_ori +1 << pos_z - sz_ori + 1;

    }

    // subspace translation estimating
    if(b_SubpixelEst)
    {
        sx = sx_ori; sy = sy_ori; sz = sz_ori;
        tx = tx_ori; ty = ty_ori; tz = tz_ori;

        //
        V3DLONG offset[3];
        offset[0] = pos_x - sx +1;
        offset[1] = pos_y - sy +1;
        offset[2] = pos_z - sz +1;

        V3DLONG bsx = (offset[0]>0)?0:-offset[0];
        V3DLONG bsy = (offset[1]>0)?0:-offset[1];
        V3DLONG bsz = (offset[2]>0)?0:-offset[2];

        V3DLONG esx = (sx-1);
        V3DLONG esy = (sy-1);
        V3DLONG esz = (sz-1);

        if(offset[0])
        {
            V3DLONG tmp = sx-1 + offset[0] - (tx-1);

            if(tmp>0)
                esx = sx-1 -tmp;
        }

        if(offset[1])
        {
            V3DLONG tmp = offset[1]+sy-1 - (ty-1);

            if(tmp>0)
                esy = sy-1 - tmp;
        }

        if(offset[2])
        {
            V3DLONG tmp = offset[2]+sz-1 - (tz-1);

            if(tmp>0)
                esz = sz-1 -tmp;
        }

        V3DLONG btx = (offset[0]>0)?offset[0]:0;
        V3DLONG bty = (offset[1]>0)?offset[1]:0;
        V3DLONG btz = (offset[2]>0)?offset[2]:0;

        V3DLONG etx = (offset[0]>0)?(tx-1):pos_x;
        V3DLONG ety = (offset[1]>0)?(ty-1):pos_y;
        V3DLONG etz = (offset[2]>0)?(tz-1):pos_z;

        if(offset[0]>0)
        {
            V3DLONG tmp = (tx-1) - (sx-1 + offset[0]);

            if(tmp>0)
                etx = tx-1 - tmp;
        }

        if(offset[1]>0)
        {
            V3DLONG tmp = (ty-1) - (sy-1 + offset[1]);

            if(tmp>0)
                ety = ty-1 - tmp;
        }

        if(offset[2]>0)
        {
            V3DLONG tmp = (tz-1) - (sz-1 + offset[2]);

            if(tmp>0)
                etz = tz-1 - tmp;
        }

        V3DLONG bbsx = esx - bsx + 1; if(bbsx<0 || bbsx>sx) return -1;
        V3DLONG bbsy = esy - bsy + 1; if(bbsy<0 || bbsy>sy) return -1;
        V3DLONG bbsz = esz - bsz + 1; if(bbsz<0 || bbsz>sz) return -1;

        V3DLONG bbtx = etx - btx + 1; if(bbtx<0 || bbtx>tx) return -1;
        V3DLONG bbty = ety - bty + 1; if(bbty<0 || bbty>ty) return -1;
        V3DLONG bbtz = etz - btz + 1; if(bbtz<0 || bbtz>tz) return -1;

        // (bbsx==bbtx && bbsy==bbty && bbsz==bbtz)=true
        V3DLONG pagesz = bbsx*bbsy*bbsz;

        //
        REAL rate_x, rate_y, rate_z;

        rate_x = qMax( (REAL)bbsx/(REAL)sx, (REAL)bbtx/(REAL)tx);
        rate_y = qMax( (REAL)bbsy/(REAL)sy, (REAL)bbty/(REAL)ty);
        rate_z = qMax( (REAL)bbsz/(REAL)sz, (REAL)bbtz/(REAL)tz);

        bool plane_yz=false, plane_xz=false, plane_xy=false;

        if(rate_x < rate_y && rate_x < rate_z)
        {
            plane_yz = true;
        }
        if(rate_y < rate_x && rate_y < rate_z)
        {
            plane_xz = true;
        }
        if(rate_z < rate_x && rate_z < rate_y)
        {
            plane_xy = true;
        }

        REAL sum=0;
        for(V3DLONG k=bsz; k<esz; k++)
        {
            V3DLONG offset_o_k = k*sx*sy;
            for(V3DLONG j=bsy; j<esy; j++)
            {
                V3DLONG offset_o_j = offset_o_k + j*sx;
                for(V3DLONG i=bsx; i<esx; i++)
                {
                    V3DLONG idx_o = offset_o_j + i;

                    sum += subject1d[idx_o];
                }
            }
        }
        SDATATYPE meanv = (SDATATYPE) (sum/(REAL)(pagesz));

        //
        if(plane_yz == true)
        {
            // finding key-plane along x direction
            V3DLONG info_count=0, xpln=0, max_info=0;

            //
            V3DLONG weights = bbsx*0.15; // threshold
            weights /= 2;
            V3DLONG start_x=bsx+weights+1, end_x=esx-weights-1;
            for(V3DLONG i=start_x; i<end_x; i++) //
            {
                info_count = 0;

                for(V3DLONG k=bsz; k<esz; k++)
                {
                    V3DLONG offset_o_k = k*sy*sx;
                    for(V3DLONG j=bsy; j<esy; j++)
                    {
                        V3DLONG idx_o = offset_o_k + j*sx + i;

                        if( subject1d[idx_o] > meanv)
                            info_count++;
                    }
                }

                if(info_count > max_info)
                {
                    max_info = info_count;
                    xpln = i;
                }
            }

            if(xpln<start_x) xpln = start_x;

            // extraction
            V3DLONG b_bsx = xpln - weights; //

            if(b_bsx>bsx)
                bsx = b_bsx;

            V3DLONG e_esx = xpln + weights;

            if(e_esx<esx)
                esx = e_esx;

            bbsx = esx-bsx+1; // dims

            V3DLONG pagesz_bb_sub = bbsx*bbsy*bbsz;
            V3DLONG pagesz_bb_tar = pagesz_bb_sub;

            // extract one plane from sub
            y_new<SDATATYPE, V3DLONG>(p_sub, pagesz_bb_sub);
            if(p_sub)
            {
                for(V3DLONG k=bsz; k<=esz; k++)
                {
                    V3DLONG offset_k = (k-bsz)*bbsx*bbsy;
                    V3DLONG offset_o_k = k*sx*sy;
                    for(V3DLONG j=bsy; j<=esy; j++)
                    {
                        V3DLONG offset_j = offset_k + (j-bsy)*bbsx;
                        V3DLONG offset_o_j = offset_o_k + j*sx;
                        for(V3DLONG i=bsx; i<=esx; i++)
                        {
                            V3DLONG idx = offset_j + (i-bsx);
                            V3DLONG idx_o = offset_o_j + i;

                            p_sub[idx] = subject1d[idx_o];
                        }
                    }
                }
            }

            V3DLONG b_btx = bsx + offset[0];

            if(b_btx>btx)
                btx = b_btx;

            V3DLONG e_etx = esx + offset[0];

            if(e_etx<etx)
                etx = e_etx;

            bbtx = etx-btx+1;

            y_new<SDATATYPE, V3DLONG>(p_tar, pagesz_bb_tar);
            if(p_tar)
            {
                for(V3DLONG k=btz; k<=etz; k++)
                {
                    V3DLONG offset_k = (k-btz)*bbtx*bbty;
                    V3DLONG offset_o_k = k*tx*ty;
                    for(V3DLONG j=bty; j<=ety; j++)
                    {
                        V3DLONG offset_j = offset_k + (j-bty)*bbtx;
                        V3DLONG offset_o_j = offset_o_k + j*tx;
                        for(V3DLONG i=btx; i<=etx; i++)
                        {
                            V3DLONG idx = offset_j + (i-btx);
                            V3DLONG idx_o = offset_o_j + i;

                            p_tar[idx] = target1d[idx_o];
                        }
                    }
                }
            }

            //
            V3DLONG sx_pad = 2*bbsx-1, sy_pad = 2*bbsy-1, sz_pad = 2*bbsz-1;

            V3DLONG even_odd = sx_pad%2; // 0 for even 1 for odd

            if(fftw_in_place)
                sx_pad += (2-even_odd); //

            V3DLONG len_pad = sx_pad*sy_pad*sz_pad;

            //
            szPad[0] = sx_pad;
            szPad[1] = sy_pad;
            szPad[2] = sz_pad;

            szSub[0] = bbsx;
            szSub[1] = bbsy;
            szSub[2] = bbsz;

            szTar[0] = bbtx;
            szTar[1] = bbty;
            szTar[2] = bbtz;

            REAL *p_f_sub = NULL;
            y_new<REAL, V3DLONG>(p_f_sub, len_pad);
            if(p_f_sub)
            {
                memset(p_f_sub, 0, sizeof(REAL)*len_pad);

                //padding zeros
                Y_IMG_REAL pOut(p_f_sub, szPad);
                Y_IMG_DATATYPE pIn(p_sub, szSub);

                YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
                tmp.padding(pOut, pIn, true, fftw_in_place, even_odd, 3);
            }

            REAL* p_f_tar = NULL;
            y_new<REAL, V3DLONG>(p_f_tar, len_pad);
            if(p_f_tar)
            {
                memset(p_f_tar, 0, sizeof(REAL)*len_pad);

                //padding zeros
                Y_IMG_REAL pOut(p_f_tar, szPad);
                Y_IMG_DATATYPE pIn(p_tar, szTar);

                YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
                tmp.padding(pOut, pIn, false, fftw_in_place, even_odd, 3);
            }

            //de-alloc
            y_del<SDATATYPE>(p_sub);
            y_del<SDATATYPE>(p_tar);

            // fft-pc subspace translation estimating (Foroosh's method)
            rPEAKS subpos;
            Y_IMG_REAL pOut(p_f_sub, szPad);
            Y_IMG_REAL pIn(p_f_tar, szPad);

            YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_REAL> tmp;
            tmp.fftpcsubspace3D(pOut, pIn, even_odd, fftw_in_place, &subpos); // subpixel shifts

            // subpixel translation estimation
            pos_x += subpos.x;
            pos_y += subpos.y;
            pos_z += subpos.z;

            //de-alloc
            y_del<REAL>(p_f_sub);
            y_del<REAL>(p_f_tar);
        }
        else if(plane_xz == true)
        {
            // finding key-plane along y direction
            V3DLONG info_count=0, ypln=0, max_info=0;

            //
            V3DLONG weights = bbsy*0.15; // threshold
            weights /= 2;
            V3DLONG start_y=bsy+weights+1, end_y=esy-weights-1;
            for(V3DLONG j=start_y; j<end_y; j++) //
            {
                info_count = 0;

                V3DLONG offset_o_j = j*sx;
                for(V3DLONG k=bsz; k<esz; k++)
                {
                    V3DLONG offset_o_k = k*sy*sx;
                    for(V3DLONG i=bsx; i<esx; i++)
                    {
                        V3DLONG idx_o = offset_o_k + offset_o_j + i;

                        if( subject1d[idx_o] > meanv)
                            info_count++;
                    }
                }

                if(info_count > max_info)
                {
                    max_info = info_count;
                    ypln = j;
                }

            }

            if(ypln<start_y) ypln = start_y;

            // extraction
            V3DLONG b_bsy = ypln - weights;

            if(b_bsy>bsy)
                bsy = b_bsy;

            V3DLONG e_esy = ypln + weights;

            if(e_esy<esy)
                esy = e_esy;

            bbsy = esy-bsy+1; // dims

            V3DLONG b_bty = bsy + offset[1];

            if(b_bty>bty)
                bty = b_bty;

            V3DLONG e_ety = esy + offset[1];

            if(e_ety<ety)
                ety = e_ety;

            bbty = ety-bty+1;

            V3DLONG pagesz_bb_sub = bbsx*bbsy*bbsz;
            V3DLONG pagesz_bb_tar = pagesz_bb_sub; //

            // extract one plane from sub
            y_new<SDATATYPE, V3DLONG>(p_sub, pagesz_bb_sub);
            if(p_sub)
            {
                for(V3DLONG k=bsz; k<=esz; k++)
                {
                    V3DLONG offset_k = (k-bsz)*bbsx*bbsy;
                    V3DLONG offset_o_k = k*sx*sy;
                    for(V3DLONG j=bsy; j<=esy; j++)
                    {
                        V3DLONG offset_j = offset_k + (j-bsy)*bbsx;
                        V3DLONG offset_o_j = offset_o_k + j*sx;
                        for(V3DLONG i=bsx; i<=esx; i++)
                        {
                            V3DLONG idx = offset_j + (i-bsx);
                            V3DLONG idx_o = offset_o_j + i;

                            p_sub[idx] = subject1d[idx_o];
                        }
                    }
                }
            }

            y_new<SDATATYPE, V3DLONG>(p_tar, pagesz_bb_tar);
            if(p_tar)
            {
                for(V3DLONG k=btz; k<=etz; k++)
                {
                    V3DLONG offset_k = (k-btz)*bbtx*bbty;
                    V3DLONG offset_o_k = k*tx*ty;
                    for(V3DLONG j=bty; j<=ety; j++)
                    {
                        V3DLONG offset_j = offset_k + (j-bty)*bbtx;
                        V3DLONG offset_o_j = offset_o_k + j*tx;
                        for(V3DLONG i=btx; i<=etx; i++)
                        {
                            V3DLONG idx = offset_j + (i-btx);
                            V3DLONG idx_o = offset_o_j + i;

                            p_tar[idx] = target1d[idx_o];
                        }
                    }
                }
            }

            //
            V3DLONG sx_pad = 2*bbsx-1, sy_pad = 2*bbsy-1, sz_pad = 2*bbsz-1;

            V3DLONG even_odd = sx_pad%2; // 0 for even 1 for odd

            if(fftw_in_place)
                sx_pad += (2-even_odd); //

            V3DLONG len_pad = sx_pad*sy_pad*sz_pad;

            //
            szPad[0] = sx_pad;
            szPad[1] = sy_pad;
            szPad[2] = sz_pad;

            szSub[0] = bbsx;
            szSub[1] = bbsy;
            szSub[2] = bbsz;

            szTar[0] = bbtx;
            szTar[1] = bbty;
            szTar[2] = bbtz;

            REAL *p_f_sub = NULL;
            y_new<REAL, V3DLONG>(p_f_sub, len_pad);
            if(p_f_sub)
            {
                memset(p_f_sub, 0, sizeof(REAL)*len_pad);

                //padding zeros
                Y_IMG_REAL pOut(p_f_sub, szPad);
                Y_IMG_DATATYPE pIn(p_sub, szSub);

                YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
                tmp.padding(pOut, pIn, true, fftw_in_place, even_odd, 3);
            }

            REAL* p_f_tar = NULL;
            y_new<REAL, V3DLONG>(p_f_tar, len_pad);
            if(p_f_tar)
            {
                memset(p_f_tar, 0, sizeof(REAL)*len_pad);

                //padding zeros
                Y_IMG_REAL pOut(p_f_tar, szPad);
                Y_IMG_DATATYPE pIn(p_tar, szTar);

                YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
                tmp.padding(pOut, pIn, false, fftw_in_place, even_odd, 3);
            }

            //de-alloc
            y_del<SDATATYPE>(p_sub);
            y_del<SDATATYPE>(p_tar);

            // fft-pc subspace translation estimating (Foroosh's method)
            rPEAKS subpos;
            Y_IMG_REAL pOut(p_f_sub, szPad);
            Y_IMG_REAL pIn(p_f_tar, szPad);

            YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_REAL> tmp;
            tmp.fftpcsubspace3D(pOut, pIn, even_odd, fftw_in_place, &subpos); // subpixel shifts

            // subpixel translation estimation
            pos_x += subpos.x;
            pos_y += subpos.y;
            pos_z += subpos.z;

            //de-alloc
            y_del<REAL>(p_f_sub);
            y_del<REAL>(p_f_tar);
        }
        else if(plane_xy == true)
        {
            // finding key-plane along z direction
            V3DLONG info_count=0, zpln=0, max_info=0;

            //
            V3DLONG weights = bbsz*0.15; // threshold
            weights /= 2;
            V3DLONG start_z=bsz+weights+1, end_z=esz-weights-1;
            for(V3DLONG k=start_z; k<end_z; k++) //
            {
                info_count = 0;

                V3DLONG offset_o_k = k*sy*sx;
                for(V3DLONG j=bsy; j<esy; j++)
                {
                    V3DLONG offset_o_j = j*sx;
                    for(V3DLONG i=bsx; i<esx; i++)
                    {
                        V3DLONG idx_o = offset_o_k + offset_o_j + i;

                        if( subject1d[idx_o] > meanv)
                            info_count++;
                    }
                }

                if(info_count > max_info)
                {
                    max_info = info_count;
                    zpln = k;
                }
            }

            // extraction
            V3DLONG b_bsz = zpln - weights/2;

            if(b_bsz>bsz)
                bsz = b_bsz;

            V3DLONG e_esz = zpln + weights/2;

            if(e_esz<esz)
                esz = e_esz;

            bbsz = esz-bsz+1; // dims

            //
            V3DLONG b_btz = bsz + offset[2];

            if(b_btz>btz)
                btz = b_btz;

            V3DLONG e_etz = esz + offset[2];

            if(e_etz<etz)
                etz = e_etz;

            bbtz = etz-btz+1;

            V3DLONG pagesz_bb_sub = bbsx*bbsy*bbsz;
            V3DLONG pagesz_bb_tar = pagesz_bb_sub; //

            // extract one plane from sub
            y_new<SDATATYPE, V3DLONG>(p_sub, pagesz_bb_sub);
            if(p_sub)
            {
                for(V3DLONG k=bsz; k<=esz; k++)
                {
                    V3DLONG offset_k = (k-bsz)*bbsx*bbsy;
                    V3DLONG offset_o_k = k*sx*sy;
                    for(V3DLONG j=bsy; j<=esy; j++)
                    {
                        V3DLONG offset_j = offset_k + (j-bsy)*bbsx;
                        V3DLONG offset_o_j = offset_o_k + j*sx;
                        for(V3DLONG i=bsx; i<=esx; i++)
                        {
                            V3DLONG idx = offset_j + (i-bsx);
                            V3DLONG idx_o = offset_o_j + i;

                            p_sub[idx] = subject1d[idx_o];
                        }
                    }
                }
            }

            y_new<SDATATYPE, V3DLONG>(p_tar, pagesz_bb_tar);
            if(p_tar)
            {
                for(V3DLONG k=btz; k<=etz; k++)
                {
                    V3DLONG offset_k = (k-btz)*bbtx*bbty;
                    V3DLONG offset_o_k = k*tx*ty;
                    for(V3DLONG j=bty; j<=ety; j++)
                    {
                        V3DLONG offset_j = offset_k + (j-bty)*bbtx;
                        V3DLONG offset_o_j = offset_o_k + j*tx;
                        for(V3DLONG i=btx; i<=etx; i++)
                        {
                            V3DLONG idx = offset_j + (i-btx);
                            V3DLONG idx_o = offset_o_j + i;

                            p_tar[idx] = target1d[idx_o];
                        }
                    }
                }
            }

            //
            V3DLONG sx_pad = 2*bbsx-1, sy_pad = 2*bbsy-1, sz_pad = 2*bbsz-1;

            V3DLONG even_odd = sx_pad%2; // 0 for even 1 for odd

            if(fftw_in_place)
                sx_pad += (2-even_odd); //

            V3DLONG len_pad = sx_pad*sy_pad*sz_pad;

            //
            szPad[0] = sx_pad;
            szPad[1] = sy_pad;
            szPad[2] = sz_pad;

            szSub[0] = bbsx;
            szSub[1] = bbsy;
            szSub[2] = bbsz;

            szTar[0] = bbtx;
            szTar[1] = bbty;
            szTar[2] = bbtz;

            REAL *p_f_sub = NULL;
            y_new<REAL, V3DLONG>(p_f_sub, len_pad);
            if(p_f_sub)
            {
                memset(p_f_sub, 0, sizeof(REAL)*len_pad);

                //padding zeros
                Y_IMG_REAL pOut(p_f_sub, szPad);
                Y_IMG_DATATYPE pIn(p_sub, szSub);

                YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
                tmp.padding(pOut, pIn, true, fftw_in_place, even_odd, 3);
            }

            REAL* p_f_tar = NULL;
            y_new<REAL, V3DLONG>(p_f_tar, len_pad);
            if(p_f_tar)
            {
                memset(p_f_tar, 0, sizeof(REAL)*len_pad);

                //padding zeros
                Y_IMG_REAL pOut(p_f_tar, szPad);
                Y_IMG_DATATYPE pIn(p_tar, szTar);

                YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_DATATYPE> tmp;
                tmp.padding(pOut, pIn, false, fftw_in_place, even_odd, 3);
            }

            //de-alloc
            y_del<SDATATYPE>(p_sub);
            y_del<SDATATYPE>(p_tar);

            // fft-pc subspace translation estimating (Foroosh's method)
            rPEAKS subpos;
            Y_IMG_REAL pOut(p_f_sub, szPad);
            Y_IMG_REAL pIn(p_f_tar, szPad);

            YImg<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_REAL> tmp;
            tmp.fftpcsubspace3D(pOut, pIn, even_odd, fftw_in_place, &subpos); // subpixel shifts

            // subpixel translation estimation
            pos_x += subpos.x;
            pos_y += subpos.y;
            pos_z += subpos.z;

            //de-alloc
            y_del<REAL>(p_f_sub);
            y_del<REAL>(p_f_tar);
        }
    }

    //de-alloc
    y_del<V3DLONG>(szPad);
    y_del<V3DLONG>(szSub);
    y_del<V3DLONG>(szTar);

    size_t end_t = clock();

    qDebug() << "time elapse ... " << end_t-start_t;

    //Output
     //--------------------------------------------------------------------------------------
    pos.x = pos_x; pos.y = pos_y; pos.z = pos_z; pos.value = pos_score;

    qDebug() << " finally..." << pos.x << pos.y << pos.z << pos.value << "offset ..." << pos_x - sx_ori +1 << pos_y - sy_ori +1 << pos_z - sz_ori +1;

    return true;
}

// blending func
template <class SDATATYPE, class Y_TLUT>
bool ifusing(SDATATYPE *pVImg, Y_TLUT tlut, SDATATYPE intensityrange)
{
    //
    V3DLONG vx, vy, vz, vc;
    vx = tlut.vx;
    vy = tlut.vy;
    vz = tlut.vz;
    vc = tlut.vc;

    //
    V3DLONG pagesz = vx*vy*vz;
    V3DLONG npxls = vc*pagesz;

    // tmporary REAL pointer
    REAL *pTmp = NULL;
    y_new<REAL, V3DLONG>(pTmp, npxls);
    if(pTmp)
    {
        memset(pTmp, 0.0, sizeof(REAL)*npxls);
    }

    // blending
    for(V3DLONG ii=0; ii<tlut.tcList.size(); ii++)
    {
        // loading relative imagg files
        V3DLONG *sz_relative = 0;
        int datatype_relative = 0;
        unsigned char* relative1d = 0;

        if (loadImage(const_cast<char *>(tlut.tcList.at(ii).fn_image.c_str()), relative1d, sz_relative, datatype_relative)!=true)
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",tlut.tcList.at(ii).fn_image.c_str());
            return -1;
        }
        V3DLONG rx=sz_relative[0], ry=sz_relative[1], rz=sz_relative[2], rc=sz_relative[3];

        // subpixel translate
        rPEAKS pos;
        pos.x = tlut.tcList.at(ii).aBX - (V3DLONG)(tlut.tcList.at(ii).aBX);
        pos.y = tlut.tcList.at(ii).aBY - (V3DLONG)(tlut.tcList.at(ii).aBY);
        pos.z = tlut.tcList.at(ii).aBZ - (V3DLONG)(tlut.tcList.at(ii).aBZ);

        qDebug()<<"pos ... "<<pos.x<<pos.y<<pos.z;

        REAL *prelative = NULL;
        V3DLONG npxls_relative = rx*ry*rz*rc;
        y_new<REAL, V3DLONG>(prelative, npxls_relative);

        V3DLONG *effectiveEnvelope=NULL;
        y_new<V3DLONG, V3DLONG>(effectiveEnvelope,6);

        if(y_abs<REAL>(pos.x)>0.0 || y_abs<REAL>(pos.y)>0.0 || y_abs<REAL>(pos.z)>0.0)
        {
            if(subpixshift3D<SDATATYPE>(prelative, (SDATATYPE *)relative1d, sz_relative, pos, effectiveEnvelope)!=true)
            {
                cout<<"Fail to subpixel translate image!"<<endl;
                return false;
            }
        }
        else
        {
            effectiveEnvelope[0] = 0;
            effectiveEnvelope[1] = rx;
            effectiveEnvelope[2] = 0;
            effectiveEnvelope[3] = ry;
            effectiveEnvelope[4] = 0;
            effectiveEnvelope[5] = rz;

            for(V3DLONG i=0; i<npxls_relative; i++)
            {
                prelative[i] = (REAL)(((SDATATYPE*)relative1d)[i]);
            }
        }

        // fit into envelope from subspace translation
        V3DLONG x_start = tlut.tcList.at(ii).aBX + effectiveEnvelope[0];
        V3DLONG x_end = tlut.tcList.at(ii).aEX - (rx - effectiveEnvelope[1]);

        V3DLONG y_start =tlut.tcList.at(ii).aBY + effectiveEnvelope[2];
        V3DLONG y_end = tlut.tcList.at(ii).aEY - (ry - effectiveEnvelope[3]);

        V3DLONG z_start =tlut.tcList.at(ii).aBZ + effectiveEnvelope[4];
        V3DLONG z_end =tlut.tcList.at(ii).aEZ - (rz - effectiveEnvelope[5]);

        //suppose all tiles with same color dimensions
        if(rc>vc)   rc = vc;

        //
        for(V3DLONG c=0; c<rc; c++)
        {
            V3DLONG o_c = c*vx*vy*vz;
            V3DLONG o_r_c = c*rx*ry*rz;
            for(V3DLONG k=z_start; k<z_end; k++)
            {
                V3DLONG k_idx = k-z_start + effectiveEnvelope[4];
                if(k_idx>=effectiveEnvelope[5]) continue;

                V3DLONG o_k = o_c + k*vx*vy;
                V3DLONG o_r_k = o_r_c + (k_idx)*rx*ry;

                for(V3DLONG j=y_start; j<y_end; j++)
                {
                    V3DLONG j_idx = j-y_start + effectiveEnvelope[2];
                    if(j_idx>=effectiveEnvelope[3]) continue;

                    V3DLONG o_j = o_k + j*vx;
                    V3DLONG o_r_j = o_r_k + (j_idx)*rx;
                    for(V3DLONG i=x_start; i<x_end; i++)
                    {
                        V3DLONG i_idx = i-x_start + effectiveEnvelope[0];
                        if(i_idx>=effectiveEnvelope[1]) continue;

                        V3DLONG idx = o_j + i;
                        V3DLONG idx_r = o_r_j + (i_idx);

                        REAL val = (REAL)(prelative[idx_r]);

                        //
                        REAL coef;
                        if(!computeWeightsTC(tlut, i, j, k, ii, coef) )
                        {
                            printf("Fail to call function computeWeights!\n");
                            return false;
                        }

                        pTmp[idx] += val*coef; // linear blending

                    }
                }
            }
        }
        //de-alloc
        y_del<REAL>(prelative);
        y_del<V3DLONG>(effectiveEnvelope);
        y_del<unsigned char>(relative1d);
        y_del<V3DLONG>(sz_relative);
        y_del<V3DLONG>(effectiveEnvelope);
    }

    // output
    REAL minval, maxval;
    for(V3DLONG c=0; c<vc; c++)
    {
        V3DLONG offsets = c*pagesz;

        minval=INF;
        maxval=-INF;
        for (V3DLONG i=0; i<pagesz; i++)
        {
            V3DLONG idx = offsets+i;

            REAL val=pTmp[idx];

            if(minval>val) minval = val;
            if(maxval<val) maxval = val;
        }
        maxval -= minval;

        for (V3DLONG i=0; i<pagesz; i++)
        {
            V3DLONG idx = offsets+i;

            pVImg[idx] = (SDATATYPE) ((REAL)intensityrange * (pTmp[idx] - minval)/maxval);
        }
    }

    //de-alloc
    y_del<REAL>(pTmp);

    return true;
}

// stitching 2 images and display in V3D
int pairwise_stitching(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandleList win_list = callback.getImageWindowList();

    if(win_list.size()<1)
    {
        QMessageBox::information(0, title, QObject::tr("No image is open."));
        return -1;
    }

    ImageStitchingDialog dialog(callback, parent);
    if (dialog.exec()!=QDialog::Accepted)
        return -1;

    dialog.update();

    int i1 = dialog.combo_subject->currentIndex();
    int i2 = dialog.combo_target->currentIndex();

    V3DLONG sub_c = dialog.sub_c-1;
    V3DLONG tar_c = dialog.tar_c-1;

    REAL overlap_percent = dialog.overlap;

    bool axes_show = dialog.axes_show; // 20100615

    REAL *scale = new REAL [6];

    scale[0] = dialog.scale_x;
    scale[1] = dialog.scale_y;
    scale[2] = dialog.scale_z;
    scale[3] = 1;
    scale[4] = 1;
    scale[5] = 1;

    Image4DSimple* subject = callback.getImage(win_list[i1]);
    Image4DSimple* target = callback.getImage(win_list[i2]);

    QString m_InputFileName[2];
    m_InputFileName[0] = callback.getImageName(win_list[i1]);
    m_InputFileName[1] = callback.getImageName(win_list[i2]);

    QString fn1 = m_InputFileName[0]; /// .tc configuration file
    QString fn2 = m_InputFileName[1];
    fn1.chop(4);
    fn2.chop(4);
    QString tmp_filename = fn1.append("_").append(fn2.remove(0, QFileInfo(fn2).path().length()+1)).append(".tc");

    qDebug()<<tmp_filename;

    Y_VIM<REAL, V3DLONG, indexed_t<V3DLONG, REAL>, LUT<V3DLONG> > vim;

    for(V3DLONG i=0; i<2; i++)
    {
        V3DLONG offset[3];
        offset[0]=0; offset[1]=0; offset[2]=0;

        indexed_t<V3DLONG, REAL> idx_t(offset);

        idx_t.n = i;
        idx_t.ref_n = 0; // init with default values
        idx_t.fn_image = m_InputFileName[i].toStdString();
        idx_t.score = 0;

        vim.tilesList.push_back(idx_t);
    }

    if (!subject || !target)
    {
        QMessageBox::information(0, title, QObject::tr("No image is open."));
        return -1;
    }
    if (subject->getDatatype() != target->getDatatype())
    {
        QMessageBox::information(0, title, QObject::tr("currently v3d_stitch_pair only support stitching images with the same datatype."));
        return -1;
    }

    ImagePixelType datatype_subject = subject->getDatatype();

    unsigned char* subject1d = subject->getRawData();

    V3DLONG sx = subject->getXDim();
    V3DLONG sy = subject->getYDim();
    V3DLONG sz = subject->getZDim();
    V3DLONG sc = subject->getCDim();

    (&vim.tilesList.at(0))->sz_image = new V3DLONG [4];

    (&vim.tilesList.at(0))->sz_image[0] = sx;
    (&vim.tilesList.at(0))->sz_image[1] = sy;
    (&vim.tilesList.at(0))->sz_image[2] = sz;
    (&vim.tilesList.at(0))->sz_image[3] = sc;

    V3DLONG pagesz_sub = sx*sy*sz;

    unsigned char* target1d = target->getRawData();

    V3DLONG tx = target->getXDim();
    V3DLONG ty = target->getYDim();
    V3DLONG tz = target->getZDim();
    V3DLONG tc = target->getCDim();

    (&vim.tilesList.at(1))->sz_image = new V3DLONG [4];

    (&vim.tilesList.at(1))->sz_image[0] = tx;
    (&vim.tilesList.at(1))->sz_image[1] = ty;
    (&vim.tilesList.at(1))->sz_image[2] = tz;
    (&vim.tilesList.at(1))->sz_image[3] = tc;

    V3DLONG pagesz_tar = tx*ty*tz;

    V3DLONG offset_sub = sub_c*sx*sy*sz;
    V3DLONG offset_tar = tar_c*tx*ty*tz;

    int start_t = clock();

    //stitching
    //--------------------------------------------------------------------------------------
    //choose the FFT NCC or FFT PC method
    bool m_similarity = false;

    // anchor position
    REAL pos_score = 0;
    V3DLONG pos_x=0, pos_y=0, pos_z=0;

    PEAKS *pos= new PEAKS;

    V3DLONG *szSub = new V3DLONG [4];
    szSub[0] = sx; szSub[1] = sy; szSub[2] = sz; szSub[3] = sc;

    V3DLONG *szTar = new V3DLONG [4];
    szTar[0] = tx; szTar[1] = ty; szTar[2] = tz; szTar[3] = tc;

    int success;

    if(datatype_subject == V3D_UINT8)
    {
        success = stitching_bb_thickplanes<unsigned char, Y_IMG_UINT8>((unsigned char *)subject1d+offset_sub, szSub, (unsigned char *)target1d+offset_tar, szTar, overlap_percent, m_similarity, scale, pos, 0);
    }
    else if(datatype_subject == V3D_UINT16)
    {
        success = stitching_bb_thickplanes<unsigned short, Y_IMG_UINT16>((unsigned short *)(subject1d)+offset_sub, szSub, (unsigned short *)(target1d)+offset_tar, szTar, overlap_percent, m_similarity, scale, pos, 0);
    }
    else if(datatype_subject == V3D_FLOAT32)
    {
        success = stitching_bb_thickplanes<REAL, Y_IMG_REAL>((REAL *)(subject1d)+offset_sub, szSub, (REAL *)(target1d)+offset_tar, szTar, overlap_percent, m_similarity, scale, pos, 0);
    }
    else
    {
        printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
        return -1;
    }

    if(success!=true) return false;

    pos_x = pos->x; pos_y = pos->y; pos_z = pos->z; pos_score = pos->value;

    cout<< "pairwise pos " << pos->x << " " << pos->y  << " " << pos->z << " " << pos->value << endl;

    //de-alloc
    if(scale) {delete []scale; scale=0;}

    //
    int end_t = clock();
    qDebug("time eclapse %d seconds for image stitching.", (end_t-start_t));

    //showing time consuming statistics info
    QTextEdit *pText=new QTextEdit(QString("<br>time eclapse %1 seconds for image stitching.<br>").arg((end_t-start_t)/1000000));
    pText->resize(800, 200);
    pText->setReadOnly(true);
    pText->setFontPointSize(12);
    pText->show();

    //display in V3D Mainwindow
    //------------------------------------------------------------------------------------------------
    V3DLONG *offset = new V3DLONG [3];
    offset[0] = pos_x - sx +1;
    offset[1] = pos_y - sy +1;
    offset[2] = pos_z - sz +1;

    (&vim.tilesList.at(1))->offsets[0] = -offset[0];
    (&vim.tilesList.at(1))->offsets[1] = -offset[1];
    (&vim.tilesList.at(1))->offsets[2] = -offset[2];

    // construct lookup table
    vim.y_clut(vim.tilesList.size());
    // save lut
    vim.y_save(tmp_filename.toStdString());

    qDebug("offset[0] %ld offset[1] %ld offset[2] %ld pos_x %ld pos_y %ld pos_z %ld", offset[0], offset[1], offset[2], pos_x, pos_y, pos_z);

    V3DLONG new_sz0, new_sz1, new_sz2, pagesz_overlap;

    new_sz0 = (offset[0]<0) ? (tx - offset[0]) : (sx + offset[0]);
    if (new_sz0<tx) new_sz0=tx;
    if (new_sz0<sx) new_sz0=sx;

    new_sz1 = (offset[1]<0) ? (ty - offset[1]) : (sy + offset[1]);
    if (new_sz1<ty) new_sz1=ty;
    if (new_sz1<sy) new_sz1=sy;

    new_sz2 = (offset[2]<0) ? (tz - offset[2]) : (sz + offset[2]);
    if (new_sz2<tz) new_sz2=tz;
    if (new_sz2<sz) new_sz2=sz;

    pagesz_overlap = new_sz0*new_sz1*new_sz2;

    if(datatype_subject == V3D_UINT8)
    {
        //
        unsigned char *p_mask = NULL;
        try
        {
            p_mask = new unsigned char [2*pagesz_overlap];

            memset(p_mask, 0, sizeof(unsigned char)*2*pagesz_overlap);
        }
        catch(...)
        {
            printf("Fail to allocate memory.\n");
            return -1;
        }

        V3DLONG totalplxs = sc*new_sz0*new_sz1*new_sz2;
        unsigned char* data1d = NULL;
        try
        {
            data1d = new unsigned char [totalplxs];

            memset(data1d, 0, sizeof(unsigned char)*totalplxs);
        }
        catch(...)
        {
            printf("Fail to allocate memory.\n");
            return -1;
        }

        //
        success = pwi_fusing<unsigned char>((unsigned char *)p_mask, (unsigned char *)data1d, (unsigned char *)subject1d, szSub, (unsigned char *)target1d, szTar, offset, axes_show, m_InputFileName[0], new_sz0, new_sz1, new_sz2, pagesz_overlap, sub_c, tar_c);

        //display
        Image4DSimple p4DImage;
        p4DImage.setData((unsigned char*)data1d, new_sz0, new_sz1, new_sz2, sc, subject->getDatatype()); //

        v3dhandle newwin = callback.newImageWindow();
        callback.setImage(newwin, &p4DImage);
        callback.setImageName(newwin, "Stitched Image");
        callback.updateImageWindow(newwin);

        //display overlap mask
        Image4DSimple p4Dmask;
        p4Dmask.setData((unsigned char*)p_mask, new_sz0, new_sz1, new_sz2, 2, subject->getDatatype());

        v3dhandle maskwin = callback.newImageWindow();
        callback.setImage(maskwin, &p4Dmask);
        callback.setImageName(maskwin, "Stitched Mask");
        callback.updateImageWindow(maskwin);
    }
    else if(datatype_subject == V3D_UINT16)
    {
        //
        unsigned short *p_mask = NULL;
        try
        {
            p_mask = new unsigned short [2*pagesz_overlap];

            memset(p_mask, 0, sizeof(unsigned short)*2*pagesz_overlap);
        }
        catch(...)
        {
            printf("Fail to allocate memory.\n");
            return -1;
        }

        V3DLONG totalplxs = sc*new_sz0*new_sz1*new_sz2;
        unsigned short* data1d = NULL;
        try
        {
            data1d = new unsigned short [totalplxs];

            memset(data1d, 0, sizeof(unsigned short)*totalplxs);
        }
        catch(...)
        {
            printf("Fail to allocate memory.\n");
            return -1;
        }

        //
        success = pwi_fusing<unsigned short>((unsigned short *)p_mask, (unsigned short *)data1d, (unsigned short *)subject1d, szSub, (unsigned short *)target1d, szTar, offset, axes_show, m_InputFileName[0], new_sz0, new_sz1, new_sz2, pagesz_overlap, sub_c, tar_c);

        //display
        Image4DSimple p4DImage;
        p4DImage.setData((unsigned char*)data1d, new_sz0, new_sz1, new_sz2, sc, subject->getDatatype()); //

        v3dhandle newwin = callback.newImageWindow();
        callback.setImage(newwin, &p4DImage);
        callback.setImageName(newwin, "Stitched Image");
        callback.updateImageWindow(newwin);

        //display overlap mask
        Image4DSimple p4Dmask;
        p4Dmask.setData((unsigned char*)p_mask, new_sz0, new_sz1, new_sz2, 2, subject->getDatatype());

        v3dhandle maskwin = callback.newImageWindow();
        callback.setImage(maskwin, &p4Dmask);
        callback.setImageName(maskwin, "Stitched Mask");
        callback.updateImageWindow(maskwin);
    }
    else if(datatype_subject == V3D_FLOAT32)
    {
        //
        REAL *p_mask = NULL;
        try
        {
            p_mask = new REAL [2*pagesz_overlap];

            memset(p_mask, 0, sizeof(REAL)*2*pagesz_overlap);
        }
        catch(...)
        {
            printf("Fail to allocate memory.\n");
            return -1;
        }

        V3DLONG totalplxs = sc*new_sz0*new_sz1*new_sz2;
        REAL* data1d = NULL;
        try
        {
            data1d = new REAL [totalplxs];

            memset(data1d, 0, sizeof(REAL)*totalplxs);
        }
        catch(...)
        {
            printf("Fail to allocate memory.\n");
            return -1;
        }

        //
        success = pwi_fusing<REAL>((REAL *)p_mask, (REAL *)data1d, (REAL *)subject1d, szSub, (REAL *)target1d, szTar, offset, axes_show, m_InputFileName[0], new_sz0, new_sz1, new_sz2, pagesz_overlap, sub_c, tar_c);

        //display
        Image4DSimple p4DImage;
        p4DImage.setData((unsigned char*)data1d, new_sz0, new_sz1, new_sz2, sc, subject->getDatatype()); //

        v3dhandle newwin = callback.newImageWindow();
        callback.setImage(newwin, &p4DImage);
        callback.setImageName(newwin, "Stitched Image");
        callback.updateImageWindow(newwin);

        //display overlap mask
        Image4DSimple p4Dmask;
        p4Dmask.setData((unsigned char*)p_mask, new_sz0, new_sz1, new_sz2, 2, subject->getDatatype());

        v3dhandle maskwin = callback.newImageWindow();
        callback.setImage(maskwin, &p4Dmask);
        callback.setImageName(maskwin, "Stitched Mask");
        callback.updateImageWindow(maskwin);
    }
    else
    {
        printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
        return -1;
    }

    if(!success) return false;

    return true;

}

// stitching a group images and saving as a virtual image
// without configuration
int group_stitching_wrap(V3DPluginCallback2 &callback, QWidget *parent)
{
    // whether the user with configuration prior knowledge
    if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you have the order information file of tiled images?"), QMessageBox::Yes, QMessageBox::No))
    {
        group_stitching_wc(callback, parent);
    }
    else
    {
        group_stitching(callback, parent);
    }

}

// stitching a group images and saving as a virtual image
// without configuration
int group_stitching(V3DPluginCallback2 &callback, QWidget *parent)
{
    // init
    GroupImageStitchingDialog dialog(callback, parent, NULL);
    if (dialog.exec()!=QDialog::Accepted)
        return -1;

    dialog.update();

    QString m_InputFileName = dialog.fn_img;

    int channel1 = dialog.channel1-1;

    REAL overlap_percent = dialog.overlap;

    bool axes_show = dialog.axes_show; // 20100615
    bool img_show = dialog.img_show; // 20110126

    REAL *scale = new REAL [6];

    scale[0] = dialog.scale_x;
    scale[1] = dialog.scale_y;
    scale[2] = dialog.scale_z;
    scale[3] = 1;
    scale[4] = 1;
    scale[5] = 1;

    bool m_similarity = false;
    bool success = false;

    ImagePixelType imgdatatype;
    V3DLONG cdim;

    int start_t = clock();

    // load tiles and stitch
    //----------------------------------------------------------------------------------------------------------------------------------------------------
    QStringList imgList = importSeriesFileList_addnumbersort(m_InputFileName);
    m_InputFileName = imgList.at(0);

    Y_VIM<REAL, V3DLONG, indexed_t<V3DLONG, REAL>, LUT<V3DLONG> > vim;

    V3DLONG count=0;
    foreach (QString img_str, imgList)
    {
        V3DLONG offset[3];
        offset[0]=0; offset[1]=0; offset[2]=0;

        indexed_t<V3DLONG, REAL> idx_t(offset);

        idx_t.n = count;
        idx_t.ref_n = 0; // init with default values
        idx_t.fn_image = img_str.toStdString();
        idx_t.score = 0;

        vim.tilesList.push_back(idx_t);

        count++;
    }

    // stitching image pairs
    // suppose 0 as a reference
    int NTILES = vim.tilesList.size();
    int NTILES_I = NTILES - 1;
    int NTILES_II = NTILES_I - 1;

    // first step: rough estimation in a coarse scale
    V3DLONG offsets[3];
    for(int i=0; i<NTILES_I; i++) // record all the sz_image information
    {
        //loading target files
        V3DLONG *sz_target = 0;
        int datatype_target = 0;
        unsigned char* target1d = 0;

        if (loadImage(const_cast<char *>(vim.tilesList.at(i).fn_image.c_str()), target1d, sz_target, datatype_target)!=true)
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",vim.tilesList.at(i).fn_image.c_str());
            return -1;
        }
        V3DLONG tx=sz_target[0], ty=sz_target[1], tz=sz_target[2], tc=sz_target[3];
        cdim = tc; // init

        if(i==0)
        {
            imgdatatype = (ImagePixelType)datatype_target;
            if(datatype_target==4) imgdatatype = V3D_FLOAT32;
        }
        else
        {
            if(datatype_target != imgdatatype)
            {
                printf("The program only support all tiled images with the same datatype.\n");
                return -1;
            }
        }

        (&vim.tilesList.at(i))->sz_image = new V3DLONG [4];

        (&vim.tilesList.at(i))->sz_image[0] = tx;
        (&vim.tilesList.at(i))->sz_image[1] = ty;
        (&vim.tilesList.at(i))->sz_image[2] = tz;
        (&vim.tilesList.at(i))->sz_image[3] = tc;

        // channel of target
        V3DLONG offsets_tar = channel1*tx*ty*tz;

        // try rest of tiles
        for(int j=i+1; j<NTILES; j++)
        {
            //loading subject files
            V3DLONG *sz_subject = 0;
            int datatype_subject = 0;
            unsigned char* subject1d = 0;

            if (loadImage(const_cast<char *>(vim.tilesList.at(j).fn_image.c_str()), subject1d, sz_subject, datatype_subject)!=true)
            {
                fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n", vim.tilesList.at(j).fn_image.c_str());
                return -1;
            }

            V3DLONG sx=sz_subject[0], sy=sz_subject[1], sz=sz_subject[2], sc=sz_subject[3];

            // init
            if(i==NTILES_II)
            {
                (&vim.tilesList.at(j))->sz_image = new V3DLONG [4];

                (&vim.tilesList.at(j))->sz_image[0] = sx;
                (&vim.tilesList.at(j))->sz_image[1] = sy;
                (&vim.tilesList.at(j))->sz_image[2] = sz;
                (&vim.tilesList.at(j))->sz_image[3] = sc;
            }

            // channel of subject
            V3DLONG offsets_sub = channel1*sx*sy*sz;

            // try
            PEAKS *pos = new PEAKS;

            if(imgdatatype == V3D_UINT8)
            {
                success = stitching_bb_thickplanes<unsigned char, Y_IMG_UINT8>((unsigned char *)subject1d+offsets_sub, sz_subject, (unsigned char *)target1d+offsets_tar, sz_target, overlap_percent, m_similarity, scale, pos, 1);
            }
            else if(imgdatatype == V3D_UINT16)
            {
                success = stitching_bb_thickplanes<unsigned short, Y_IMG_UINT16>((unsigned short *)(subject1d)+offsets_sub, sz_subject, (unsigned short *)(target1d)+offsets_tar, sz_target, overlap_percent, m_similarity, scale, pos, 1);
            }
            else if(imgdatatype == V3D_FLOAT32)
            {
                success = stitching_bb_thickplanes<REAL, Y_IMG_REAL>((REAL *)(subject1d)+offsets_sub, sz_subject, (REAL *)(target1d)+offsets_tar, sz_target, overlap_percent, m_similarity, scale, pos, 1);
            }
            else
            {
                printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                return -1;
            }
            if(success!=true) return false;

            cout<< "pairwise pos " << pos->x << " " << pos->y  << " " << pos->z << " " << pos->value << endl;

            //record n by n-1
            offsets[0] = pos->x - sx +1;
            offsets[1] = pos->y - sy +1;
            offsets[2] = pos->z - sz +1;

            indexed_t<V3DLONG, REAL> t(offsets);  //

            t.score = pos->value;
            t.n = vim.tilesList.at(i).n;

            (&vim.tilesList.at(j))->record.push_back(t);

            qDebug()<<"t offsets ..."<<t.offsets[0]<<t.offsets[1]<<t.offsets[2];

            cout << vim.tilesList.at(i).fn_image << " over " << vim.tilesList.at(j).fn_image << endl;

            //de-alloc
            if(subject1d) {delete []subject1d; subject1d=0;}
            if(sz_subject) {delete []sz_subject; sz_subject=0;}

        }

        //de-alloc
        if(target1d) {delete []target1d; target1d=0;}
        if(sz_target) {delete []sz_target; sz_target=0;}

    }

    // find mst of whole tiled images
    for(int i=0; i<NTILES; i++)
    {
        vim.tilesList.at(i).visited = false;
    }
    if(mstPrim(vim.tilesList)) // run Prim's algorithm
    {
        cout<<"Fail to call MST approach!"<<endl;
        return false;
    }

    //define threshold of correlation coeffecient score
    //REAL threshold_corr_score = 0.75; //

    //find a ref. image
    //std::vector<indexed_t>::iterator iterStart = vim.tilesList.begin();

    //	bool del_tile = true;
    //
    //	for(int iter = 0; iter < NTILES_I; iter++)
    //	{
    //
    //		for(int j=iter+1; j<NTILES; j++)
    //		{
    //
    //			if( vim.tilesList.at(j).ref_n == vim.tilesList.at(iter).n && vim.tilesList.at(j).score>threshold_corr_score) //
    //			{
    //				del_tile = false;
    //				break;
    //			}
    //
    //		}
    //
    //		if(del_tile)
    //		{
    //			vim.tilesList.erase( vim.tilesList.begin()+iter );
    //
    //			NTILES = vim.tilesList.size();
    //			NTILES_I = NTILES - 1;
    //
    //			iter--;
    //		}
    //		else
    //			break;
    //
    //	}

    //stitch at a fine scale
    NTILES = vim.tilesList.size();
    NTILES_I = NTILES - 1;
    NTILES_II = NTILES_I - 1;

    if(scale[0]==1 && scale[1]==1 && scale[2]==1)
    {
        //continue;
    }
    else
    {
        //
        for(int i=0; i<NTILES; i++)
        {
            vim.tilesList.at(i).visited = false;
        }

        //
        for(int i=1; i<NTILES; i++) // traverse all tiled images
        {
            PEAKS *pos = new PEAKS;

            V3DLONG current = vim.tilesList.at(i).n;
            V3DLONG previous = vim.tilesList.at(i).predecessor;

            if(vim.tilesList.at(current).visited) // computed
            {
                continue;
            }

            //loading subject files
            V3DLONG *sz_subject = 0;
            int datatype_subject = 0;
            unsigned char* subject1d = 0;

            if (loadImage(const_cast<char *>(vim.tilesList.at(current).fn_image.c_str()), subject1d, sz_subject, datatype_subject)!=true)
            {
                fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n", vim.tilesList.at(current).fn_image.c_str());
                return -1;
            }

            V3DLONG sx=sz_subject[0], sy=sz_subject[1], sz=sz_subject[2], sc=sz_subject[3];

            // try rest of tiles
            while(previous!=-1)
            {

                //loading target files
                V3DLONG *sz_target = 0;
                int datatype_target = 0;
                unsigned char* target1d = 0;

                if (loadImage(const_cast<char *>(vim.tilesList.at(previous).fn_image.c_str()), target1d, sz_target, datatype_target)!=true)
                {
                    fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",vim.tilesList.at(previous).fn_image.c_str());
                    return -1;
                }
                V3DLONG tx=sz_target[0], ty=sz_target[1], tz=sz_target[2], tc=sz_target[3];

                //
                if(vim.tilesList.at(current).visited)
                {
                    current = previous;
                    previous = vim.tilesList.at(current).predecessor;

                    //de-alloc
                    if(subject1d) {delete []subject1d; subject1d=0;}
                    if(sz_subject) {delete []sz_subject; sz_subject=0;}

                    //
                    subject1d = target1d;
                    sz_subject = sz_target;
                    sx=sz_subject[0], sy=sz_subject[1], sz=sz_subject[2], sc=sz_subject[3];

                    //
                    continue;
                }

                //
                V3DLONG offsets_tar = channel1*tx*ty*tz;
                V3DLONG offsets_sub = channel1*sx*sy*sz;

                //
                pos->x = vim.tilesList.at(current).offsets[0] + (sx-1);
                pos->y = vim.tilesList.at(current).offsets[1] + (sy-1);
                pos->z = vim.tilesList.at(current).offsets[2] + (sz-1);
                pos->value = vim.tilesList.at(current>previous?current:previous).record.at(current<previous?current:previous).score; //

                //
                if(imgdatatype == V3D_UINT8)
                {
                    success = stitching_bb_thickplanes<unsigned char, Y_IMG_UINT8>((unsigned char *)subject1d+offsets_sub, sz_subject, (unsigned char *)target1d+offsets_tar, sz_target, overlap_percent, m_similarity, scale, pos, 2);
                }
                else if(imgdatatype == V3D_UINT16)
                {
                    success = stitching_bb_thickplanes<unsigned short, Y_IMG_UINT16>((unsigned short *)(subject1d)+offsets_sub, sz_subject, (unsigned short *)(target1d)+offsets_tar, sz_target, overlap_percent, m_similarity, scale, pos, 2);
                }
                else if(imgdatatype == V3D_FLOAT32)
                {
                    success = stitching_bb_thickplanes<REAL, Y_IMG_REAL>((REAL *)(subject1d)+offsets_sub, sz_subject, (REAL *)(target1d)+offsets_tar, sz_target, overlap_percent, m_similarity, scale, pos, 2);
                }
                else
                {
                    printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                    return -1;
                }
                if(success!=true) return false;

                // update offsets of current to previous
                (&vim.tilesList.at(current))->offsets[0] = pos->x - sx +1;
                (&vim.tilesList.at(current))->offsets[1] = pos->y - sy +1;
                (&vim.tilesList.at(current))->offsets[2] = pos->z - sz +1;

                (&vim.tilesList.at(current))->visited = true;

                //
                current = previous;
                previous = vim.tilesList.at(current).predecessor;

                //de-alloc
                if(subject1d) {delete []subject1d; subject1d=0;}
                if(sz_subject) {delete []sz_subject; sz_subject=0;}

                //
                subject1d = target1d;
                sz_subject = sz_target;
                sx=sz_subject[0], sy=sz_subject[1], sz=sz_subject[2], sc=sz_subject[3];

                if(previous==-1)
                {
                    if(target1d) {delete []target1d; target1d=0;}
                    if(sz_target) {delete []sz_target; sz_target=0;}
                }

            }


        }

    }
    //de-alloc
    if(scale) {delete []scale; scale=0;}

    // adjusting offset reference to ref. image
    // compute accumulate offsets from path list
    int ref_image = vim.tilesList.at(0).n;
    (&vim.tilesList.at(0))->offsets[0] = 0;
    (&vim.tilesList.at(0))->offsets[1] = 0;
    (&vim.tilesList.at(0))->offsets[2] = 0;

    for(int i=0; i<NTILES; i++)
    {
        vim.tilesList.at(i).visited = false;
    }

    for(int i_path=1; i_path<NTILES; i_path++)
    {
        // record final shifts
        vim.tilesList.at(i_path).record.at(0).offsets[0] = vim.tilesList.at(i_path).offsets[0];
        vim.tilesList.at(i_path).record.at(0).offsets[1] = vim.tilesList.at(i_path).offsets[1];
        vim.tilesList.at(i_path).record.at(0).offsets[2] = vim.tilesList.at(i_path).offsets[2];
    }

    for(int i_path=1; i_path<NTILES; i_path++)
    {
        // ref
        (&vim.tilesList.at(i_path))->ref_n = ref_image;

        V3DLONG current = vim.tilesList.at(i_path).n;
        V3DLONG previous = vim.tilesList.at(i_path).predecessor;

        //
        while(previous!=-1)
        {
            if(vim.tilesList.at(current).visited)
            {
                break;
            }



            (&vim.tilesList.at(i_path))->offsets[0] += vim.tilesList.at(previous).offsets[0];
            (&vim.tilesList.at(i_path))->offsets[1] += vim.tilesList.at(previous).offsets[1];
            (&vim.tilesList.at(i_path))->offsets[2] += vim.tilesList.at(previous).offsets[2];


            //
            current = previous;
            previous = vim.tilesList.at(current).predecessor;
        }

        (&vim.tilesList.at(i_path))->visited = true;
    }

    // construct lookup table
    vim.y_clut(vim.tilesList.size());

    //------------------------------------------------------------------------------------------------------------------------------------------
    // save lut
    QString tmp_filename = QFileInfo(m_InputFileName).path() + "/" + "stitched_image.tc"; //.tc tile configuration

    vim.y_save(tmp_filename.toStdString());

    //
    int end_t = clock();
    qDebug("time eclapse %d seconds for image stitching.", (end_t-start_t));

    //showing time consuming statistics info
    QTextEdit *pText=new QTextEdit(QString("<br>time eclapse %1 seconds for image stitching.<br>").arg((end_t-start_t)/1000000));
    pText->resize(800, 200);
    pText->setReadOnly(true);
    pText->setFontPointSize(12);
    pText->show();

    //------------------------------------------------------------------------------------------------------------------------------------------
    //visualization
    if(img_show)
    {
        V3DLONG vx, vy, vz, vc;

        vx = vim.max_vim[0] - vim.min_vim[0] + 1; //
        vy = vim.max_vim[1] - vim.min_vim[1] + 1;
        vz = vim.max_vim[2] - vim.min_vim[2] + 1;
        vc = cdim;

        V3DLONG pagesz_vim = vx*vy*vz*vc;

        qDebug()<<"vim dims ..."<< vx << vy << vz << vc;

        if(imgdatatype == V3D_UINT8)
        {
            // init
            unsigned char *pVImg = NULL;

            try
            {
                pVImg = new unsigned char [pagesz_vim];

                memset(pVImg, 0, sizeof(unsigned char)*pagesz_vim);
            }
            catch (...)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }

            //
            success = groupi_fusing<unsigned char>((unsigned char *)pVImg, vim, vx, vy, vz, vc, axes_show);

            //display
            Image4DSimple p4DImage;
            p4DImage.setData((unsigned char *)pVImg, vx, vy, vz, vc, imgdatatype);

            v3dhandle newwin = callback.newImageWindow();
            callback.setImage(newwin, &p4DImage);
            callback.setImageName(newwin, "Stitched_Image");
            callback.updateImageWindow(newwin);
        }
        else if(imgdatatype == V3D_UINT16)
        {
            // init
            unsigned short *pVImg = NULL;

            try
            {
                pVImg = new unsigned short [pagesz_vim];

                memset(pVImg, 0, sizeof(unsigned short)*pagesz_vim);
            }
            catch (...)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }

            //
            success = groupi_fusing<unsigned short>((unsigned short *)pVImg, vim, vx, vy, vz, vc, axes_show);

            //display
            Image4DSimple p4DImage;
            p4DImage.setData((unsigned char *)pVImg, vx, vy, vz, vc, imgdatatype);

            v3dhandle newwin = callback.newImageWindow();
            callback.setImage(newwin, &p4DImage);
            callback.setImageName(newwin, "Stitched_Image");
            callback.updateImageWindow(newwin);
        }
        else if(imgdatatype == V3D_FLOAT32)
        {
            // init
            REAL *pVImg = NULL;

            try
            {
                pVImg = new REAL [pagesz_vim];

                memset(pVImg, 0, sizeof(REAL)*pagesz_vim);
            }
            catch (...)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }

            //
            success = groupi_fusing<REAL>((REAL *)pVImg, vim, vx, vy, vz, vc, axes_show);

            //display
            Image4DSimple p4DImage;
            p4DImage.setData((unsigned char *)pVImg, vx, vy, vz, vc, imgdatatype);

            v3dhandle newwin = callback.newImageWindow();
            callback.setImage(newwin, &p4DImage);
            callback.setImageName(newwin, "Stitched_Image");
            callback.updateImageWindow(newwin);
        }
        else
        {
            printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
            return -1;
        }
        if(success!=true) return false;

        //
        return true;


        //Output
        //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        //	//saving
        //	//temprary image
        //	QString tmp_filename = m_InputFileName + ".tmp.raw";
        //	V3DLONG sz_tmp[4];
        //
        //	sz_tmp[0] = vx; sz_tmp[1] = vy; sz_tmp[2] = vz; sz_tmp[3] = vc;
        //
        //	if (saveImage(tmp_filename.toStdString().c_str(), (const unsigned char *)pVImg, sz_tmp, 1)!=true)
        //	{
        //		fprintf(stderr, "Error happens in file writing. Exit. \n");
        //		return -1;
        //	}

    }

    return true;
}
int batch_group_stitching(V3DPluginCallback2 &callback, QWidget *parent)
{
    // init
    GroupImageStitchingDialog dialog(callback, parent, NULL);
    if (dialog.exec()!=QDialog::Accepted)
        return -1;

    dialog.update();

    QString m_InputFileName;

    //int channel1 = dialog.channel1-1;

    REAL overlap_percent = dialog.overlap;

    bool axes_show = dialog.axes_show; // 20100615
    bool img_show = dialog.img_show; // 20110126

    //	REAL *scale = new REAL [6];
    //
    //	scale[0] = dialog.scale_x;
    //	scale[1] = dialog.scale_y;
    //	scale[2] = dialog.scale_z;
    //	scale[3] = 1;
    //	scale[4] = 1;
    //	scale[5] = 1;

    bool m_similarity = false;
    bool success = false;

    ImagePixelType imgdatatype;
    V3DLONG cdim;

    int start_t = clock();

    ////////////////////////

    QString fn_img = QFileDialog::getExistingDirectory(0, QObject::tr("Choose the directory including all tiled images "),
                                                       QDir::currentPath(),
                                                       QFileDialog::ShowDirsOnly);
    QDir dir(fn_img);

    QStringList rootlist;

    foreach (QString subDir,dir.entryList(QDir::Dirs|QDir::NoDotAndDotDot))
    {
        rootlist += QFileInfo(dir, subDir).absoluteFilePath();
    }

    foreach (QString qs, rootlist) qDebug() << qs;

    for(int i =0; i<rootlist.size();i++)
    {
        int channel1 = dialog.channel1-1;

        REAL *scale = new REAL [6];

        scale[0] = dialog.scale_x;
        scale[1] = dialog.scale_y;
        scale[2] = dialog.scale_z;
        scale[3] = 1;
        scale[4] = 1;
        scale[5] = 1;

        m_InputFileName=rootlist.at(i);

        // load tiles and stitch
        //----------------------------------------------------------------------------------------------------------------------------------------------------
        QStringList imgList = importSeriesFileList_addnumbersort(m_InputFileName);
        m_InputFileName = imgList.at(0);

        Y_VIM<REAL, V3DLONG, indexed_t<V3DLONG, REAL>, LUT<V3DLONG> > vim;

        V3DLONG count=0;
        foreach (QString img_str, imgList)
        {
            V3DLONG offset[3];
            offset[0]=0; offset[1]=0; offset[2]=0;

            indexed_t<V3DLONG, REAL> idx_t(offset);

            idx_t.n = count;
            idx_t.ref_n = 0; // init with default values
            idx_t.fn_image = img_str.toStdString();
            idx_t.score = 0;

            vim.tilesList.push_back(idx_t);

            count++;
        }

        // stitching image pairs
        // suppose 0 as a reference
        int NTILES = vim.tilesList.size();
        int NTILES_I = NTILES - 1;
        int NTILES_II = NTILES_I - 1;

        // first step: rough estimation in a coarse scale
        V3DLONG offsets[3];
        for(int i=0; i<NTILES_I; i++) // record all the sz_image information
        {
            //loading target files
            V3DLONG *sz_target = 0;
            int datatype_target = 0;
            unsigned char* target1d = 0;

            if (loadImage(const_cast<char *>(vim.tilesList.at(i).fn_image.c_str()), target1d, sz_target, datatype_target)!=true)
            {
                fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",vim.tilesList.at(i).fn_image.c_str());
                return -1;
            }
            V3DLONG tx=sz_target[0], ty=sz_target[1], tz=sz_target[2], tc=sz_target[3];
            cdim = tc; // init

            if(i==0)
            {
                imgdatatype = (ImagePixelType)datatype_target;
                if(datatype_target==4) imgdatatype = V3D_FLOAT32;
            }
            else
            {
                if(datatype_target != imgdatatype)
                {
                    printf("The program only support all tiled images with the same datatype.\n");
                    return -1;
                }
            }

            (&vim.tilesList.at(i))->sz_image = new V3DLONG [4];

            (&vim.tilesList.at(i))->sz_image[0] = tx;
            (&vim.tilesList.at(i))->sz_image[1] = ty;
            (&vim.tilesList.at(i))->sz_image[2] = tz;
            (&vim.tilesList.at(i))->sz_image[3] = tc;

            // channel of target
            V3DLONG offsets_tar = channel1*tx*ty*tz;

            // try rest of tiles
            for(int j=i+1; j<NTILES; j++)
            {
                //loading subject files
                V3DLONG *sz_subject = 0;
                int datatype_subject = 0;
                unsigned char* subject1d = 0;

                if (loadImage(const_cast<char *>(vim.tilesList.at(j).fn_image.c_str()), subject1d, sz_subject, datatype_subject)!=true)
                {
                    fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n", vim.tilesList.at(j).fn_image.c_str());
                    return -1;
                }

                V3DLONG sx=sz_subject[0], sy=sz_subject[1], sz=sz_subject[2], sc=sz_subject[3];

                // init
                if(i==NTILES_II)
                {
                    (&vim.tilesList.at(j))->sz_image = new V3DLONG [4];

                    (&vim.tilesList.at(j))->sz_image[0] = sx;
                    (&vim.tilesList.at(j))->sz_image[1] = sy;
                    (&vim.tilesList.at(j))->sz_image[2] = sz;
                    (&vim.tilesList.at(j))->sz_image[3] = sc;
                }

                // channel of subject
                V3DLONG offsets_sub = channel1*sx*sy*sz;

                // try
                PEAKS *pos = new PEAKS;

                if(imgdatatype == V3D_UINT8)
                {
                    success = stitching_bb_thickplanes<unsigned char, Y_IMG_UINT8>((unsigned char *)subject1d+offsets_sub, sz_subject, (unsigned char *)target1d+offsets_tar, sz_target, overlap_percent, m_similarity, scale, pos, 1);
                }
                else if(imgdatatype == V3D_UINT16)
                {
                    success = stitching_bb_thickplanes<unsigned short, Y_IMG_UINT16>((unsigned short *)(subject1d)+offsets_sub, sz_subject, (unsigned short *)(target1d)+offsets_tar, sz_target, overlap_percent, m_similarity, scale, pos, 1);
                }
                else if(imgdatatype == V3D_FLOAT32)
                {
                    success = stitching_bb_thickplanes<REAL, Y_IMG_REAL>((REAL *)(subject1d)+offsets_sub, sz_subject, (REAL *)(target1d)+offsets_tar, sz_target, overlap_percent, m_similarity, scale, pos, 1);
                }
                else
                {
                    printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                    return -1;
                }
                if(success!=true) return false;

                cout<< "pairwise pos " << pos->x << " " << pos->y  << " " << pos->z << " " << pos->value << endl;

                //record n by n-1
                offsets[0] = pos->x - sx +1;
                offsets[1] = pos->y - sy +1;
                offsets[2] = pos->z - sz +1;

                indexed_t<V3DLONG, REAL> t(offsets);  //

                t.score = pos->value;
                t.n = vim.tilesList.at(i).n;

                (&vim.tilesList.at(j))->record.push_back(t);

                qDebug()<<"t offsets ..."<<t.offsets[0]<<t.offsets[1]<<t.offsets[2];

                cout << vim.tilesList.at(i).fn_image << " over " << vim.tilesList.at(j).fn_image << endl;

                //de-alloc
                if(subject1d) {delete []subject1d; subject1d=0;}
                if(sz_subject) {delete []sz_subject; sz_subject=0;}

            }

            //de-alloc
            if(target1d) {delete []target1d; target1d=0;}
            if(sz_target) {delete []sz_target; sz_target=0;}

        }

        // find mst of whole tiled images
        for(int i=0; i<NTILES; i++)
        {
            vim.tilesList.at(i).visited = false;
        }
        mstPrim(vim.tilesList); // run Prim's algorithm
        //stitch at a fine scale
        NTILES = vim.tilesList.size();
        NTILES_I = NTILES - 1;
        NTILES_II = NTILES_I - 1;

        if(scale[0]==1 && scale[1]==1 && scale[2]==1)
        {
            //continue;
        }
        else
        {
            //
            for(int i=0; i<NTILES; i++)
            {
                vim.tilesList.at(i).visited = false;
            }

            //
            for(int i=1; i<NTILES; i++) // traverse all tiled images
            {
                PEAKS *pos = new PEAKS;

                V3DLONG current = vim.tilesList.at(i).n;
                V3DLONG previous = vim.tilesList.at(i).predecessor;

                //loading subject files
                V3DLONG *sz_subject = 0;
                int datatype_subject = 0;
                unsigned char* subject1d = 0;

                if (loadImage(const_cast<char *>(vim.tilesList.at(current).fn_image.c_str()), subject1d, sz_subject, datatype_subject)!=true)
                {
                    fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n", vim.tilesList.at(current).fn_image.c_str());
                    return -1;
                }

                V3DLONG sx=sz_subject[0], sy=sz_subject[1], sz=sz_subject[2], sc=sz_subject[3];

                // try rest of tiles
                while(previous!=-1)
                {

                    //loading target files
                    V3DLONG *sz_target = 0;
                    int datatype_target = 0;
                    unsigned char* target1d = 0;

                    if (loadImage(const_cast<char *>(vim.tilesList.at(previous).fn_image.c_str()), target1d, sz_target, datatype_target)!=true)
                    {
                        fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",vim.tilesList.at(previous).fn_image.c_str());
                        return -1;
                    }
                    V3DLONG tx=sz_target[0], ty=sz_target[1], tz=sz_target[2], tc=sz_target[3];

                    //
                    if(vim.tilesList.at(current).visited)
                    {
                        current = previous;
                        previous = vim.tilesList.at(current).predecessor;

                        //de-alloc
                        if(subject1d) {delete []subject1d; subject1d=0;}
                        if(sz_subject) {delete []sz_subject; sz_subject=0;}

                        //
                        subject1d = target1d;
                        sz_subject = sz_target;
                        sx=sz_subject[0], sy=sz_subject[1], sz=sz_subject[2], sc=sz_subject[3];

                        //
                        continue;
                    }

                    //
                    V3DLONG offsets_tar = channel1*tx*ty*tz;
                    V3DLONG offsets_sub = channel1*sx*sy*sz;

                    //
                    pos->x = vim.tilesList.at(current).offsets[0] + (sx-1);
                    pos->y = vim.tilesList.at(current).offsets[1] + (sy-1);
                    pos->z = vim.tilesList.at(current).offsets[2] + (sz-1);
                    pos->value = vim.tilesList.at(current>previous?current:previous).record.at(current<previous?current:previous).score; //

                    //
                    if(imgdatatype == V3D_UINT8)
                    {
                        success = stitching_bb_thickplanes<unsigned char, Y_IMG_UINT8>((unsigned char *)subject1d+offsets_sub, sz_subject, (unsigned char *)target1d+offsets_tar, sz_target, overlap_percent, m_similarity, scale, pos, 2);
                    }
                    else if(imgdatatype == V3D_UINT16)
                    {
                        success = stitching_bb_thickplanes<unsigned short, Y_IMG_UINT16>((unsigned short *)(subject1d)+offsets_sub, sz_subject, (unsigned short *)(target1d)+offsets_tar, sz_target, overlap_percent, m_similarity, scale, pos, 2);
                    }
                    else if(imgdatatype == V3D_FLOAT32)
                    {
                        success = stitching_bb_thickplanes<REAL, Y_IMG_REAL>((REAL *)(subject1d)+offsets_sub, sz_subject, (REAL *)(target1d)+offsets_tar, sz_target, overlap_percent, m_similarity, scale, pos, 2);
                    }
                    else
                    {
                        printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                        return -1;
                    }
                    if(success!=true) return false;

                    // update offsets of current to previous
                    (&vim.tilesList.at(current))->offsets[0] = pos->x - sx +1;
                    (&vim.tilesList.at(current))->offsets[1] = pos->y - sy +1;
                    (&vim.tilesList.at(current))->offsets[2] = pos->z - sz +1;

                    (&vim.tilesList.at(current))->visited = true;

                    //
                    current = previous;
                    previous = vim.tilesList.at(current).predecessor;

                    //de-alloc
                    if(subject1d) {delete []subject1d; subject1d=0;}
                    if(sz_subject) {delete []sz_subject; sz_subject=0;}

                    //
                    subject1d = target1d;
                    sz_subject = sz_target;
                    sx=sz_subject[0], sy=sz_subject[1], sz=sz_subject[2], sc=sz_subject[3];

                    if(previous==-1)
                    {
                        if(target1d) {delete []target1d; target1d=0;}
                        if(sz_target) {delete []sz_target; sz_target=0;}
                    }

                }


            }

        }
        //de-alloc
        if(scale) {delete []scale; scale=0;}

        // adjusting offset reference to ref. image
        // compute accumulate offsets from path list
        int ref_image = vim.tilesList.at(0).n;
        (&vim.tilesList.at(0))->offsets[0] = 0;
        (&vim.tilesList.at(0))->offsets[1] = 0;
        (&vim.tilesList.at(0))->offsets[2] = 0;

        for(int i=0; i<NTILES; i++)
        {
            vim.tilesList.at(i).visited = false;
        }

        for(int i_path=1; i_path<NTILES; i_path++)
        {
            // ref
            (&vim.tilesList.at(i_path))->ref_n = ref_image;

            V3DLONG current = vim.tilesList.at(i_path).n;
            V3DLONG previous = vim.tilesList.at(i_path).predecessor;

            //
            while(previous!=-1)
            {
                if(vim.tilesList.at(current).visited)
                {
                    break;
                }

                (&vim.tilesList.at(i_path))->offsets[0] += vim.tilesList.at(previous).offsets[0];
                (&vim.tilesList.at(i_path))->offsets[1] += vim.tilesList.at(previous).offsets[1];
                (&vim.tilesList.at(i_path))->offsets[2] += vim.tilesList.at(previous).offsets[2];


                //
                current = previous;
                previous = vim.tilesList.at(current).predecessor;
            }

            (&vim.tilesList.at(i_path))->visited = true;
        }

        // construct lookup table
        vim.y_clut(vim.tilesList.size());

        //------------------------------------------------------------------------------------------------------------------------------------------
        // save lut
        QString tmp_filename = QFileInfo(m_InputFileName).path() + "/" + "stitched_image.tc"; //.tc tile configuration

        vim.y_save(tmp_filename.toStdString());

        //
        int end_t = clock();
        qDebug("time eclapse %d seconds for image stitching.", (end_t-start_t));

        //showing time consuming statistics info
        //	QTextEdit *pText=new QTextEdit(QString("<br>time eclapse %1 seconds for image stitching.<br>").arg((end_t-start_t)/1000000));
        //		pText->resize(800, 200);
        //		pText->setReadOnly(true);
        //		pText->setFontPointSize(12);
        //		pText->show();

        //------------------------------------------------------------------------------------------------------------------------------------------
        //visualization
        //		if(img_show)
        //		{
        //			V3DLONG vx, vy, vz, vc;
        //
        //			vx = vim.max_vim[0] - vim.min_vim[0] + 1; //
        //			vy = vim.max_vim[1] - vim.min_vim[1] + 1;
        //			vz = vim.max_vim[2] - vim.min_vim[2] + 1;
        //			vc = cdim;
        //
        //			V3DLONG pagesz_vim = vx*vy*vz*vc;
        //
        //			qDebug()<<"vim dims ..."<< vx << vy << vz << vc;
        //
        //			if(imgdatatype == V3D_UINT8)
        //			{
        //				// init
        //				unsigned char *pVImg = NULL;
        //
        //				try
        //				{
        //					pVImg = new unsigned char [pagesz_vim];
        //
        //					memset(pVImg, 0, sizeof(unsigned char)*pagesz_vim);
        //				}
        //				catch (...)
        //				{
        //					printf("Fail to allocate memory.\n");
        //					return -1;
        //				}
        //
        //				//
        //				success = groupi_fusing<unsigned char>((unsigned char *)pVImg, vim, vx, vy, vz, vc, axes_show);
        //
        //				//display
        //				Image4DSimple p4DImage;
        //				p4DImage.setData((unsigned char *)pVImg, vx, vy, vz, vc, imgdatatype);
        //
        //				v3dhandle newwin = callback.newImageWindow();
        //				callback.setImage(newwin, &p4DImage);
        //				callback.setImageName(newwin, "Stitched_Image");
        //				callback.updateImageWindow(newwin);
        //			}
        //			else if(imgdatatype == V3D_UINT16)
        //			{
        //				// init
        //				unsigned short *pVImg = NULL;
        //
        //				try
        //				{
        //					pVImg = new unsigned short [pagesz_vim];
        //
        //					memset(pVImg, 0, sizeof(unsigned short)*pagesz_vim);
        //				}
        //				catch (...)
        //				{
        //					printf("Fail to allocate memory.\n");
        //					return -1;
        //				}
        //
        //				//
        //				success = groupi_fusing<unsigned short>((unsigned short *)pVImg, vim, vx, vy, vz, vc, axes_show);
        //
        //				//display
        //				Image4DSimple p4DImage;
        //				p4DImage.setData((unsigned char *)pVImg, vx, vy, vz, vc, imgdatatype);
        //
        //				v3dhandle newwin = callback.newImageWindow();
        //				callback.setImage(newwin, &p4DImage);
        //				callback.setImageName(newwin, "Stitched_Image");
        //				callback.updateImageWindow(newwin);
        //			}
        //			else if(imgdatatype == V3D_FLOAT32)
        //			{
        //				// init
        //				REAL *pVImg = NULL;
        //
        //				try
        //				{
        //					pVImg = new REAL [pagesz_vim];
        //
        //					memset(pVImg, 0, sizeof(REAL)*pagesz_vim);
        //				}
        //				catch (...)
        //				{
        //					printf("Fail to allocate memory.\n");
        //					return -1;
        //				}
        //
        //				//
        //				success = groupi_fusing<REAL>((REAL *)pVImg, vim, vx, vy, vz, vc, axes_show);
        //
        //				//display
        //				Image4DSimple p4DImage;
        //				p4DImage.setData((unsigned char *)pVImg, vx, vy, vz, vc, imgdatatype);
        //
        //				v3dhandle newwin = callback.newImageWindow();
        //				callback.setImage(newwin, &p4DImage);
        //				callback.setImageName(newwin, "Stitched_Image");
        //				callback.updateImageWindow(newwin);
        //			}
        //			else
        //			{
        //				printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
        //				return -1;
        //			}
        //			if(success!=true) return false;
        //
        //			//
        //			return true;
        //
        //		}
    }
    QMessageBox::information(0, "Batch stitch", QObject::tr("batch end"));
    return true;
}

// with configuration file
int group_stitching_wc(V3DPluginCallback2 &callback, QWidget *parent)
{
    // get filename
    QString m_FileName = QFileDialog::getOpenFileName(parent, QObject::tr("Open An Order Information File"),
                                                      QDir::currentPath(),
                                                      QObject::tr("Image Configuration (*)"));
    if(m_FileName.isEmpty())
        return -1;

    string filename =m_FileName.toStdString();

    CONF_INFO<V3DLONG> cinfo;
    std::vector<CONF_INFO<V3DLONG> > cinfoList;

    ifstream pFile(filename.c_str());
    string str;

    if(pFile.is_open())
    {
        char buf[2000];

        while( !pFile.eof() )
        {
            while( getline(pFile, str) )
            {
                istringstream iss(str);

                iss>>buf;

                cinfo.fn_img = buf;

                iss>>cinfo.row;
                iss>>cinfo.col;

                cinfoList.push_back(cinfo);
            }
        }
    }

    qDebug() << cinfoList.size();
    QString m_InputFileName = QString(cinfo.fn_img.c_str()); // absolute path

    // init
    GroupImageStitchingDialog dialog(callback, parent, m_InputFileName);
    if (dialog.exec()!=QDialog::Accepted)
        return -1;

    dialog.update();

    int channel1 = dialog.channel1-1;

    REAL overlap_percent = dialog.overlap;

    bool axes_show = dialog.axes_show; // 20100615
    bool img_show = dialog.img_show; // 20110126

    REAL *scale = new REAL [6];

    scale[0] = dialog.scale_x;
    scale[1] = dialog.scale_y;
    scale[2] = dialog.scale_z;
    scale[3] = 1;
    scale[4] = 1;
    scale[5] = 1;

    bool m_similarity = false;
    bool success = false;

    ImagePixelType imgdatatype;
    V3DLONG cdim;

    int start_t = clock();

    // load tiles and stitch
    //----------------------------------------------------------------------------------------------------------------------------------------------------
    QStringList imgList = importSeriesFileList_addnumbersort(m_InputFileName);

    Y_VIM<REAL, V3DLONG, indexed_t<V3DLONG, REAL>, LUT<V3DLONG> > vim;

    V3DLONG count=0;
    foreach (QString img_str, imgList)
    {
        V3DLONG offset[3];
        offset[0]=0; offset[1]=0; offset[2]=0;

        indexed_t<V3DLONG, REAL> idx_t(offset);

        idx_t.n = count;
        idx_t.ref_n = 0;
        idx_t.fn_image = img_str.toStdString();
        idx_t.score = 0;

        vim.tilesList.push_back(idx_t);

        count++;
    }

    qDebug() << "debug..." <<vim.tilesList.size();

    // stitching image pairs
    // suppose 0 as a reference
    for(int i=0; i<vim.tilesList.size(); i++) // record all the sz_image information
    {

        string str_fn_i = vim.tilesList.at(i).fn_image;
        V3DLONG row_i=-1, col_i=-1;

        for(int ii=0; ii<cinfoList.size(); ii++)
        {
            if(cinfoList.at(ii).fn_img.compare(str_fn_i)==0)
            {
                row_i = cinfoList.at(ii).row;
                col_i = cinfoList.at(ii).col;
                break;
            }
        }

        qDebug() << row_i << col_i;

        if(row_i<0 || col_i<0)
            return -1;

        //loading target files
        V3DLONG *sz_target = 0;
        int datatype_target = 0;
        unsigned char* target1d = 0;

        if (loadImage(const_cast<char *>(vim.tilesList.at(i).fn_image.c_str()), target1d, sz_target, datatype_target)!=true)
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",vim.tilesList.at(i).fn_image.c_str());
            return -1;
        }
        V3DLONG tx=sz_target[0], ty=sz_target[1], tz=sz_target[2], tc=sz_target[3];
        cdim = tc; // init

        if(i==0)
        {
            imgdatatype = (ImagePixelType)datatype_target;
            if(datatype_target==4) imgdatatype = V3D_FLOAT32;
        }
        else
        {
            if(datatype_target != imgdatatype)
            {
                printf("The program only support all tiled images with the same datatype.\n");
                return -1;
            }
        }

        (&vim.tilesList.at(i))->sz_image = new V3DLONG [4];

        (&vim.tilesList.at(i))->sz_image[0] = tx;
        (&vim.tilesList.at(i))->sz_image[1] = ty;
        (&vim.tilesList.at(i))->sz_image[2] = tz;
        (&vim.tilesList.at(i))->sz_image[3] = tc;

        // channel of target
        V3DLONG offsets_tar = (dialog.channel1-1)*tx*ty*tz;

        // try rest of tiles
        for(int j=i+1; j<vim.tilesList.size(); j++)
        {

            string str_fn_j = vim.tilesList.at(j).fn_image;
            V3DLONG row_j=-1, col_j=-1;

            for(int ii=0; ii<cinfoList.size(); ii++)
            {
                if(cinfoList.at(ii).fn_img.compare(str_fn_j)==0)
                {
                    row_j = cinfoList.at(ii).row;
                    col_j = cinfoList.at(ii).col;
                    break;
                }

            }
            qDebug() << row_i << col_i;

            if(row_j<0 || col_j<0)
                return -1;

            if( !((row_i==row_j && y_abs(col_i-col_j)==1) ||  (col_i==col_j && y_abs(row_i-row_j)==1)))
                continue;

            //loading subject files
            V3DLONG *sz_subject = 0;
            int datatype_subject = 0;
            unsigned char* subject1d = 0;

            if (loadImage(const_cast<char *>(vim.tilesList.at(j).fn_image.c_str()), subject1d, sz_subject, datatype_subject)!=true)
            {
                fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n", vim.tilesList.at(j).fn_image.c_str());
                return -1;
            }

            V3DLONG sx=sz_subject[0], sy=sz_subject[1], sz=sz_subject[2], sc=sz_subject[3];

            qDebug()<< "i j ..."<< i <<  j << "row col ..." << row_i << col_i << row_j << col_j;

            // channel of subject
            V3DLONG offsets_sub = (dialog.channel1-1)*sx*sy*sz;

            // try
            PEAKS *pos = new PEAKS;

            //
            if(imgdatatype == V3D_UINT8)
            {
                success = stitching_bb_thickplanes<unsigned char, Y_IMG_UINT8>((unsigned char *)subject1d+offsets_sub, sz_subject, (unsigned char *)target1d+offsets_tar, sz_target, overlap_percent, m_similarity, scale, pos, 1);
            }
            else if(imgdatatype == V3D_UINT16)
            {
                success = stitching_bb_thickplanes<unsigned short, Y_IMG_UINT16>((unsigned short *)(subject1d)+offsets_sub, sz_subject, (unsigned short *)(target1d)+offsets_tar, sz_target, overlap_percent, m_similarity, scale, pos, 1);
            }
            else if(imgdatatype == V3D_FLOAT32)
            {
                success = stitching_bb_thickplanes<REAL, Y_IMG_REAL>((REAL *)(subject1d)+offsets_sub, sz_subject, (REAL *)(target1d)+offsets_tar, sz_target, overlap_percent, m_similarity, scale, pos, 1);
            }
            else
            {
                printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                return -1;
            }
            if(success!=true) return false;

            if(pos->value > vim.tilesList.at(j).score)
            {
                //
                cout << vim.tilesList.at(i).fn_image << " over " << vim.tilesList.at(j).fn_image << endl;

                vim.tilesList.at(j).score = pos->value;

                (&vim.tilesList.at(j))->ref_n = vim.tilesList.at(i).n;

                //offset
                V3DLONG offset[3];
                offset[0] = pos->x - sx +1;
                offset[1] = pos->y - sy +1;
                offset[2] = pos->z - sz +1;

                (&vim.tilesList.at(j))->offsets[0] = offset[0];
                (&vim.tilesList.at(j))->offsets[1] = offset[1];
                (&vim.tilesList.at(j))->offsets[2] = offset[2];
            }

            //de-alloc
            if(subject1d) {delete []subject1d; subject1d=0;}
            if(sz_subject) {delete []sz_subject; sz_subject=0;}

        }

        //de-alloc
        if(target1d) {delete []target1d; target1d=0;}
        if(sz_target) {delete []sz_target; sz_target=0;}

    }
    //de-alloc
    if(scale) {delete []scale; scale=0;}

    // adjusting offset reference to 0
    for(int i=1; i<vim.tilesList.size(); i++)
    {

        while(vim.tilesList.at(i).ref_n!=0)
        {
            V3DLONG ref = vim.tilesList.at(i).ref_n;

            V3DLONG offset_i[3], offset_ref[3];

            offset_i[0] = vim.tilesList.at(i).offsets[0];
            offset_i[1] = vim.tilesList.at(i).offsets[1];
            offset_i[2] = vim.tilesList.at(i).offsets[2];

            offset_ref[0] = vim.tilesList.at(ref).offsets[0];
            offset_ref[1] = vim.tilesList.at(ref).offsets[1];
            offset_ref[2] = vim.tilesList.at(ref).offsets[2];

            // ref
            (&vim.tilesList.at(i))->ref_n = vim.tilesList.at(ref).ref_n;

            // combine
            offset_i[0] += offset_ref[0];
            offset_i[1] += offset_ref[1];
            offset_i[2] += offset_ref[2];

            // update
            (&vim.tilesList.at(i))->offsets[0] = offset_i[0];
            (&vim.tilesList.at(i))->offsets[1] = offset_i[1];
            (&vim.tilesList.at(i))->offsets[2] = offset_i[2];
        }
    }

    // construct lookup table
    vim.y_clut(vim.tilesList.size());

    //------------------------------------------------------------------------------------------------------------------------------------------
    // save lut
    QString tmp_filename = QFileInfo(m_InputFileName).path() + "/" + "stitched_image.tc"; //.tc tile configuration

    vim.y_save(tmp_filename.toStdString());

    //
    int end_t = clock();
    qDebug("time eclapse %d seconds for image stitching.", (end_t-start_t));

    //showing time consuming statistics info
    QTextEdit *pText=new QTextEdit(QString("<br>time eclapse %1 seconds for image stitching.<br>").arg((end_t-start_t)/1000000));
    pText->resize(800, 200);
    pText->setReadOnly(true);
    pText->setFontPointSize(12);
    pText->show();

    //------------------------------------------------------------------------------------------------------------------------------------------
    //visualization
    if(img_show)
    {
        V3DLONG vx, vy, vz, vc;

        vx = vim.max_vim[0] - vim.min_vim[0] + 1; //
        vy = vim.max_vim[1] - vim.min_vim[1] + 1;
        vz = vim.max_vim[2] - vim.min_vim[2] + 1;
        vc = cdim;

        V3DLONG pagesz_vim = vx*vy*vz*vc;

        qDebug()<<"vim dims ..."<< vx << vy << vz << vc;

        if(imgdatatype == V3D_UINT8)
        {
            // init
            unsigned char *pVImg = NULL;

            try
            {
                pVImg = new unsigned char [pagesz_vim];

                memset(pVImg, 0, sizeof(unsigned char)*pagesz_vim);
            }
            catch (...)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }

            //
            success = groupi_fusing<unsigned char>((unsigned char *)pVImg, vim, vx, vy, vz, vc, axes_show);

            //display
            Image4DSimple p4DImage;
            p4DImage.setData((unsigned char *)pVImg, vx, vy, vz, vc, imgdatatype);

            v3dhandle newwin = callback.newImageWindow();
            callback.setImage(newwin, &p4DImage);
            callback.setImageName(newwin, "Stitched_Image");
            callback.updateImageWindow(newwin);
        }
        else if(imgdatatype == V3D_UINT16)
        {
            // init
            unsigned short *pVImg = NULL;

            try
            {
                pVImg = new unsigned short [pagesz_vim];

                memset(pVImg, 0, sizeof(unsigned short)*pagesz_vim);
            }
            catch (...)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }

            //
            success = groupi_fusing<unsigned short>((unsigned short *)pVImg, vim, vx, vy, vz, vc, axes_show);

            //display
            Image4DSimple p4DImage;
            p4DImage.setData((unsigned char *)pVImg, vx, vy, vz, vc, imgdatatype);

            v3dhandle newwin = callback.newImageWindow();
            callback.setImage(newwin, &p4DImage);
            callback.setImageName(newwin, "Stitched_Image");
            callback.updateImageWindow(newwin);
        }
        else if(imgdatatype == V3D_FLOAT32)
        {
            // init
            REAL *pVImg = NULL;

            try
            {
                pVImg = new REAL [pagesz_vim];

                memset(pVImg, 0, sizeof(REAL)*pagesz_vim);
            }
            catch (...)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }

            //
            success = groupi_fusing<REAL>((REAL *)pVImg, vim, vx, vy, vz, vc, axes_show);

            //display
            Image4DSimple p4DImage;
            p4DImage.setData((unsigned char *)pVImg, vx, vy, vz, vc, imgdatatype);

            v3dhandle newwin = callback.newImageWindow();
            callback.setImage(newwin, &p4DImage);
            callback.setImageName(newwin, "Stitched_Image");
            callback.updateImageWindow(newwin);
        }
        else
        {
            printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
            return -1;
        }
        if(success!=true) return false;

        //
        return true;


        //Output
        //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        //	//saving
        //	//temprary image
        //	QString tmp_filename = m_InputFileName + ".tmp.raw";
        //	V3DLONG sz_tmp[4];
        //
        //	sz_tmp[0] = vx; sz_tmp[1] = vy; sz_tmp[2] = vz; sz_tmp[3] = vc;
        //
        //	if (saveImage(tmp_filename.toStdString().c_str(), (const unsigned char *)pVImg, sz_tmp, 1)!=true)
        //	{
        //		fprintf(stderr, "Error happens in file writing. Exit. \n");
        //		return -1;
        //	}

    }

    return true;
}

// point navigation
int point_navigating(V3DPluginCallback2 &callback, QWidget *parent)
{

    // get filename
    QString m_FileName = QFileDialog::getOpenFileName(parent, QObject::tr("Open A Virtual Image"),
                                                      QDir::currentPath(),
                                                      QObject::tr("Image Configuration (*.tc)"));

    if(m_FileName.isEmpty())
        return -1;

    string filename =m_FileName.toStdString();

    // virtual image
    Y_VIM<REAL, V3DLONG, indexed_t<V3DLONG, REAL>, LUT<V3DLONG> > vim;

    if( !vim.y_load(filename) )
        return -1;

    // point navigating
    PointNavigateDialog dialog(parent, vim.sz);
    if (dialog.exec()!=QDialog::Accepted)
        return -1;

    dialog.update();

    // input virtual image point position
    V3DLONG *point = new V3DLONG [3];

    point[0] = dialog.x;
    point[1] = dialog.y;
    point[2] = dialog.z;

    POINT p,q;
    p.x = point[0]; p.y = point[1]; p.z = point[2];

    q.intensity = 0; q.fn = "[No contained in any tile]"; q.x=p.x; q.y=p.y; q.z=p.z;

    //
    V3DLONG x_s = point[0] + vim.min_vim[0];
    V3DLONG y_s = point[1] + vim.min_vim[1];
    V3DLONG z_s = point[2] + vim.min_vim[2];

    // look up lut
    for(V3DLONG ii=0; ii<vim.number_tiles; ii++)
    {
        //
        if( x_s >= vim.lut[ii].start_pos[0] && x_s <= vim.lut[ii].end_pos[0] &&  y_s >= vim.lut[ii].start_pos[1] && y_s <= vim.lut[ii].end_pos[1] &&  z_s >= vim.lut[ii].start_pos[2] && z_s <= vim.lut[ii].end_pos[2] )
        {
            cout << vim.lut[ii].fn_img << endl;

            // loading relative imagg files
            V3DLONG *sz_relative = 0;
            int datatype_relative = 0;
            unsigned char* relative1d = 0;

            if (loadImage(const_cast<char *>(vim.lut[ii].fn_img.c_str()), relative1d, sz_relative, datatype_relative)!=true)
            {
                fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",vim.tilesList.at(ii).fn_image.c_str());
                return -1;
            }
            V3DLONG rx=sz_relative[0], ry=sz_relative[1], rz=sz_relative[2], rc=sz_relative[3];

            //
            V3DLONG x_r = x_s + vim.lut[ii].start_pos[0];
            V3DLONG y_r = y_s + vim.lut[ii].start_pos[1];
            V3DLONG z_r = z_s + vim.lut[ii].start_pos[2];

            V3DLONG idx_r = z_r*rx*ry + y_r*rx + x_r;

            if(datatype_relative == V3D_UINT8)
            {
                unsigned char tmp = relative1d[idx_r];

                if(tmp>q.intensity)
                {
                    q.intensity = (float)tmp;
                    q.fn = vim.lut[ii].fn_img;
                    q.x = x_r; q.y = y_r; q.z = z_r;
                }
            }
            else if(datatype_relative == V3D_UINT16)
            {
                unsigned short tmp = (unsigned short)relative1d[idx_r];

                if(tmp>q.intensity)
                {
                    q.intensity = (float)tmp;
                    q.fn = vim.lut[ii].fn_img;
                    q.x = x_r; q.y = y_r; q.z = z_r;
                }
            }
            else if(datatype_relative == V3D_FLOAT32)
            {
                float tmp = (float)relative1d[idx_r];

                if(tmp>q.intensity)
                {
                    q.intensity = (float)tmp;
                    q.fn = vim.lut[ii].fn_img;
                    q.x = x_r; q.y = y_r; q.z = z_r;
                }
            }
            else
            {
                printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                return -1;
            }

            //de-alloc
            if(relative1d) {delete []relative1d; relative1d=0;}
        }
    }

    //showing corresponding tile image point
    QString qstr(q.fn.c_str());
    QTextEdit *pText=new QTextEdit(QString("You have entered the location ( %1 %2 %3 ).<br> The intensity value of this location is : %4. <br>").arg(q.x+1).arg(q.y+1).arg(q.z+1).arg(q.intensity) +
                                   QString("<br>The corresponding tile is ") + qstr);
    pText->setDocumentTitle("Point Navigation");
    pText->resize(400, 100);
    pText->setReadOnly(true);
    pText->setFontPointSize(12);
    pText->show();

}


// region navigation
int region_navigating(V3DPluginCallback2 &callback, QWidget *parent)
{
    // get filename
    QString m_FileName = QFileDialog::getOpenFileName(parent, QObject::tr("Open A Virtual Image"),
                                                      QDir::currentPath(),
                                                      QObject::tr("Image Configuration (*.tc)"));
    if(m_FileName.isEmpty())
        return -1;

    QString curFilePath = QFileInfo(m_FileName).path();
    curFilePath.append("/");

    string filename =m_FileName.toStdString();

    // virtual image
    Y_VIM<REAL, V3DLONG, indexed_t<V3DLONG, REAL>, LUT<V3DLONG> > vim;

    if( !vim.y_load(filename) )
        return -1;

    //vim.y_save(QString(filename.c_str()).append(".tc").toStdString()); ///< Test y_load and y_save funcs

    // point navigating
    RegionNavigateDialog dialog(parent, vim.sz);
    if (dialog.exec()!=QDialog::Accepted)
        return -1;

    dialog.update();

    // input virtual image point position
    V3DLONG start[3], end[3];

    start[0] = dialog.xs-1; // dialog is 1-based coordinates
    start[1] = dialog.ys-1;
    start[2] = dialog.zs-1;

    end[0] = dialog.xe-1;
    end[1] = dialog.ye-1;
    end[2] = dialog.ze-1;

    //virtual image
    V3DLONG vx, vy, vz, vc;

    vx = end[0] - start[0] + 1; // suppose the size same of all tiles
    vy = end[1] - start[1] + 1;
    vz = end[2] - start[2] + 1;
    vc = vim.sz[3];

    V3DLONG pagesz_vim = vx*vy*vz*vc;

    // flu bird algorithm
    // 000 'f', 'l', 'u' ; 111 'b', 'r', 'd'; relative[2] relative[1] relative[0]
    bitset<3> lut_ss, lut_se, lut_es, lut_ee;

    //
    V3DLONG x_s = start[0] + vim.min_vim[0];
    V3DLONG y_s = start[1] + vim.min_vim[1];
    V3DLONG z_s = start[2] + vim.min_vim[2];

    V3DLONG x_e = end[0] + vim.min_vim[0];
    V3DLONG y_e = end[1] + vim.min_vim[1];
    V3DLONG z_e = end[2] + vim.min_vim[2];

    //
    ImagePixelType datatype;
    bool flag_init = true;

    unsigned char *pVImg_UINT8 = NULL;
    unsigned short *pVImg_UINT16 = NULL;
    float *pVImg_FLOAT32 = NULL;

    // look up lut
    for(V3DLONG ii=0; ii<vim.number_tiles; ii++)
    {
        // init
        lut_ss.reset();
        lut_se.reset();
        lut_es.reset();
        lut_ee.reset();

        //
        if(x_s < vim.lut[ii].start_pos[0]) lut_ss[1] = 1; // r  0 l
        if(y_s < vim.lut[ii].start_pos[1]) lut_ss[0] = 1; // d  0 u
        if(z_s < vim.lut[ii].start_pos[2]) lut_ss[2] = 1; // b  0 f

        if(x_e < vim.lut[ii].start_pos[0]) lut_se[1] = 1; // r  0 l
        if(y_e < vim.lut[ii].start_pos[1]) lut_se[0] = 1; // d  0 u
        if(z_e < vim.lut[ii].start_pos[2]) lut_se[2] = 1; // b  0 f

        if(x_s < vim.lut[ii].end_pos[0]) lut_es[1] = 1; // r  0 l
        if(y_s < vim.lut[ii].end_pos[1]) lut_es[0] = 1; // d  0 u
        if(z_s < vim.lut[ii].end_pos[2]) lut_es[2] = 1; // b  0 f

        if(x_e < vim.lut[ii].end_pos[0]) lut_ee[1] = 1; // r  0 l
        if(y_e < vim.lut[ii].end_pos[1]) lut_ee[0] = 1; // d  0 u
        if(z_e < vim.lut[ii].end_pos[2]) lut_ee[2] = 1; // b  0 f

        // copy data
        if( (!lut_ss.any() && lut_ee.any()) || (lut_es.any() && !lut_ee.any()) || (lut_ss.any() && !lut_se.any()) )
        {
            //
            cout << "satisfied image: "<< vim.lut[ii].fn_img << endl;

            // loading relative image files
            V3DLONG *sz_relative = 0;
            int datatype_relative = 0;
            unsigned char* relative1d = 0;

            QString curPath = curFilePath;

            string fn = curPath.append( QString(vim.lut[ii].fn_img.c_str()) ).toStdString();

            qDebug()<<"testing..."<<curFilePath<< fn.c_str();

            if (loadImage(const_cast<char *>(fn.c_str()), relative1d, sz_relative, datatype_relative)!=true)
            {
                fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",vim.tilesList.at(ii).fn_image.c_str());
                return -1;
            }
            V3DLONG rx=sz_relative[0], ry=sz_relative[1], rz=sz_relative[2], rc=sz_relative[3];

            if(flag_init)
            {
                if(datatype_relative == V3D_UINT8)
                {
                    datatype = V3D_UINT8;

                    try
                    {
                        pVImg_UINT8 = new unsigned char [pagesz_vim];
                    }
                    catch (...)
                    {
                        printf("Fail to allocate memory.\n");
                        return -1;
                    }

                    // init
                    memset(pVImg_UINT8, 0, pagesz_vim*sizeof(unsigned char));

                    flag_init = false;

                }
                else if(datatype_relative == V3D_UINT16)
                {
                    datatype = V3D_UINT16;

                    try
                    {
                        pVImg_UINT16 = new unsigned short [pagesz_vim];
                    }
                    catch (...)
                    {
                        printf("Fail to allocate memory.\n");
                        return -1;
                    }

                    // init
                    memset(pVImg_UINT16, 0, pagesz_vim*sizeof(unsigned short));

                    flag_init = false;
                }
                else if(datatype_relative == V3D_FLOAT32)
                {
                    datatype = V3D_FLOAT32;

                    try
                    {
                        pVImg_FLOAT32 = new float [pagesz_vim];
                    }
                    catch (...)
                    {
                        printf("Fail to allocate memory.\n");
                        return -1;
                    }

                    // init
                    memset(pVImg_FLOAT32, 0, pagesz_vim*sizeof(float));

                    flag_init = false;
                }
                else
                {
                    printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                    return -1;
                }
            }

            if(datatype_relative != datatype)
            {
                printf("The program only support all tiled images with the same datatype.\n");
                return -1;
            }

            //
            V3DLONG tile2vi_xs = vim.lut[ii].start_pos[0]-vim.min_vim[0];
            V3DLONG tile2vi_xe = vim.lut[ii].end_pos[0]-vim.min_vim[0];
            V3DLONG tile2vi_ys = vim.lut[ii].start_pos[1]-vim.min_vim[1];
            V3DLONG tile2vi_ye = vim.lut[ii].end_pos[1]-vim.min_vim[1];
            V3DLONG tile2vi_zs = vim.lut[ii].start_pos[2]-vim.min_vim[2];
            V3DLONG tile2vi_ze = vim.lut[ii].end_pos[2]-vim.min_vim[2];

            V3DLONG x_start = (start[0] > tile2vi_xs) ? start[0] : tile2vi_xs;
            V3DLONG x_end = (end[0] < tile2vi_xe) ? end[0] : tile2vi_xe;
            V3DLONG y_start = (start[1] > tile2vi_ys) ? start[1] : tile2vi_ys;
            V3DLONG y_end = (end[1] < tile2vi_ye) ? end[1] : tile2vi_ye;
            V3DLONG z_start = (start[2] > tile2vi_zs) ? start[2] : tile2vi_zs;
            V3DLONG z_end = (end[2] < tile2vi_ze) ? end[2] : tile2vi_ze;

            x_end++;
            y_end++;
            z_end++;

            //
            cout << x_start << " " << x_end << " " << y_start << " " << y_end << " " << z_start << " " << z_end << endl;

            if(datatype == V3D_UINT8)
            {
                region_groupfusing<unsigned char>(pVImg_UINT8, vim, relative1d,
                                                  vx, vy, vz, vc, rx, ry, rz, rc,
                                                  tile2vi_zs, tile2vi_ys, tile2vi_xs,
                                                  z_start, z_end, y_start, y_end, x_start, x_end, start);

            }
            else if(datatype == V3D_UINT16)
            {
                region_groupfusing<unsigned short>(pVImg_UINT16, vim, relative1d,
                                                   vx, vy, vz, vc, rx, ry, rz, rc,
                                                   tile2vi_zs, tile2vi_ys, tile2vi_xs,
                                                   z_start, z_end, y_start, y_end, x_start, x_end, start);
            }
            else if(datatype == V3D_FLOAT32)
            {
                region_groupfusing<float>(pVImg_FLOAT32, vim, relative1d,
                                          vx, vy, vz, vc, rx, ry, rz, rc,
                                          tile2vi_zs, tile2vi_ys, tile2vi_xs,
                                          z_start, z_end, y_start, y_end, x_start, x_end, start);
            }
            else
            {
                printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                return -1;
            }

            //de-alloc
            if(relative1d) {delete []relative1d; relative1d=0;}
            if(sz_relative) {delete []sz_relative; sz_relative=0;}
        }

    }

    //display
    Image4DSimple p4DImage;

    if(datatype == V3D_UINT8)
    {
        p4DImage.setData((unsigned char*)pVImg_UINT8, vx, vy, vz, vc, datatype);
    }
    else if(datatype == V3D_UINT16)
    {
        p4DImage.setData((unsigned char*)pVImg_UINT16, vx, vy, vz, vc, datatype);
    }
    else if(datatype == V3D_FLOAT32)
    {
        p4DImage.setData((unsigned char*)pVImg_FLOAT32, vx, vy, vz, vc, datatype);
    }
    else
    {
        printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
        return -1;
    }

    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, &p4DImage);
    callback.setImageName(newwin, "ROI of A Virtual Image");
    callback.updateImageWindow(newwin);

    return 0;
}
int batch_region_navigating(V3DPluginCallback2 &callback, QWidget *parent)
{

    QString fn_img = QFileDialog::getExistingDirectory(0, QObject::tr("Choose the directory including all tiled images "),
                                                       QDir::currentPath(),
                                                       QFileDialog::ShowDirsOnly);
    QDir dir(fn_img);

    QString m_FileName;

    QStringList list = dir.entryList();

    QStringList imgSuffix;

    imgSuffix<<"*.tc";

    QStringList rootlist;

    foreach (QString subDir,dir.entryList(QDir::Dirs|QDir::NoDotAndDotDot))
    {
        rootlist += QFileInfo(dir, subDir).absoluteFilePath();
    }
    qDebug()<<"rootlist== "<<rootlist.size();
    for(int i=0; i<rootlist.size();i++)
    {
        QStringList myList;

        myList.clear();

        QDir dir1(rootlist.at(i));

        if (!dir1.exists())
        {
            qWarning("Cannot find the directory");
            return -1;
        }

        foreach (QString file, dir1.entryList(imgSuffix, QDir::Files, QDir::Name))
        {
            myList += QFileInfo(dir1, file).absoluteFilePath();
        }

        qDebug()<<"lisitsize== "<<myList.size();

        //qDebug()<<"dirname= "<<rootlist.at(i);

        QString str = QFileInfo(rootlist.at(i)).fileName();

        qDebug()<<"str=============== "<<str;

        foreach (QString qs, myList) qDebug() << qs;

        for(int j=0; j<myList.size();j++)
        {
            m_FileName = myList.at(j);

            QString curFilePath = QFileInfo(m_FileName).path();

            curFilePath.append("/");

            string filename =m_FileName.toStdString();

            // virtual image
            Y_VIM<REAL, V3DLONG, indexed_t<V3DLONG, REAL>, LUT<V3DLONG> > vim;

            if( !vim.y_load(filename) )
                return -1;

            // input virtual image point position
            V3DLONG start[3], end[3];

            start[0] = 0; // dialog is 1-based coordinates
            start[1] = 0;
            start[2] = 0;

            end[0] = vim.sz[0]-1;
            end[1] = vim.sz[1]-1;
            end[2] = vim.sz[2]-1;

            //virtual image
            V3DLONG vx, vy, vz, vc;

            vx = end[0] - start[0] + 1; // suppose the size same of all tiles
            vy = end[1] - start[1] + 1;
            vz = end[2] - start[2] + 1;
            vc = vim.sz[3];

            V3DLONG pagesz_vim = vx*vy*vz*vc;

            // flu bird algorithm
            // 000 'f', 'l', 'u' ; 111 'b', 'r', 'd'; relative[2] relative[1] relative[0]
            bitset<3> lut_ss, lut_se, lut_es, lut_ee;

            //
            V3DLONG x_s = start[0] + vim.min_vim[0];
            V3DLONG y_s = start[1] + vim.min_vim[1];
            V3DLONG z_s = start[2] + vim.min_vim[2];

            V3DLONG x_e = end[0] + vim.min_vim[0];
            V3DLONG y_e = end[1] + vim.min_vim[1];
            V3DLONG z_e = end[2] + vim.min_vim[2];

            //
            ImagePixelType datatype;
            bool flag_init = true;

            unsigned char *pVImg_UINT8 = NULL;
            unsigned short *pVImg_UINT16 = NULL;
            float *pVImg_FLOAT32 = NULL;

            // look up lut
            for(V3DLONG ii=0; ii<vim.number_tiles; ii++)
            {
                // init
                lut_ss.reset();
                lut_se.reset();
                lut_es.reset();
                lut_ee.reset();

                //
                if(x_s < vim.lut[ii].start_pos[0]) lut_ss[1] = 1; // r  0 l
                if(y_s < vim.lut[ii].start_pos[1]) lut_ss[0] = 1; // d  0 u
                if(z_s < vim.lut[ii].start_pos[2]) lut_ss[2] = 1; // b  0 f

                if(x_e < vim.lut[ii].start_pos[0]) lut_se[1] = 1; // r  0 l
                if(y_e < vim.lut[ii].start_pos[1]) lut_se[0] = 1; // d  0 u
                if(z_e < vim.lut[ii].start_pos[2]) lut_se[2] = 1; // b  0 f

                if(x_s < vim.lut[ii].end_pos[0]) lut_es[1] = 1; // r  0 l
                if(y_s < vim.lut[ii].end_pos[1]) lut_es[0] = 1; // d  0 u
                if(z_s < vim.lut[ii].end_pos[2]) lut_es[2] = 1; // b  0 f

                if(x_e < vim.lut[ii].end_pos[0]) lut_ee[1] = 1; // r  0 l
                if(y_e < vim.lut[ii].end_pos[1]) lut_ee[0] = 1; // d  0 u
                if(z_e < vim.lut[ii].end_pos[2]) lut_ee[2] = 1; // b  0 f

                // copy data
                if( (!lut_ss.any() && lut_ee.any()) || (lut_es.any() && !lut_ee.any()) || (lut_ss.any() && !lut_se.any()) )
                {
                    //
                    cout << "satisfied image: "<< vim.lut[ii].fn_img << endl;

                    // loading relative image files
                    V3DLONG *sz_relative = 0;
                    int datatype_relative = 0;
                    unsigned char* relative1d = 0;

                    QString curPath = curFilePath;

                    string fn = curPath.append( QString(vim.lut[ii].fn_img.c_str()) ).toStdString();

                    qDebug()<<"testing..."<<curFilePath<< fn.c_str();

                    if (loadImage(const_cast<char *>(fn.c_str()), relative1d, sz_relative, datatype_relative)!=true)
                    {
                        fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",vim.tilesList.at(ii).fn_image.c_str());
                        return -1;
                    }
                    V3DLONG rx=sz_relative[0], ry=sz_relative[1], rz=sz_relative[2], rc=sz_relative[3];

                    if(flag_init)
                    {
                        if(datatype_relative == V3D_UINT8)
                        {
                            datatype = V3D_UINT8;

                            try
                            {
                                pVImg_UINT8 = new unsigned char [pagesz_vim];
                            }
                            catch (...)
                            {
                                printf("Fail to allocate memory.\n");
                                return -1;
                            }

                            // init
                            memset(pVImg_UINT8, 0, pagesz_vim*sizeof(unsigned char));

                            flag_init = false;

                        }
                        else if(datatype_relative == V3D_UINT16)
                        {
                            datatype = V3D_UINT16;

                            try
                            {
                                pVImg_UINT16 = new unsigned short [pagesz_vim];
                            }
                            catch (...)
                            {
                                printf("Fail to allocate memory.\n");
                                return -1;
                            }

                            // init
                            memset(pVImg_UINT16, 0, pagesz_vim*sizeof(unsigned short));

                            flag_init = false;
                        }
                        else if(datatype_relative == V3D_FLOAT32)
                        {
                            datatype = V3D_FLOAT32;

                            try
                            {
                                pVImg_FLOAT32 = new float [pagesz_vim];
                            }
                            catch (...)
                            {
                                printf("Fail to allocate memory.\n");
                                return -1;
                            }

                            // init
                            memset(pVImg_FLOAT32, 0, pagesz_vim*sizeof(float));

                            flag_init = false;
                        }
                        else
                        {
                            printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                            return -1;
                        }
                    }

                    if(datatype_relative != datatype)
                    {
                        printf("The program only support all tiled images with the same datatype.\n");
                        return -1;
                    }

                    //
                    V3DLONG tile2vi_xs = vim.lut[ii].start_pos[0]-vim.min_vim[0];
                    V3DLONG tile2vi_xe = vim.lut[ii].end_pos[0]-vim.min_vim[0];
                    V3DLONG tile2vi_ys = vim.lut[ii].start_pos[1]-vim.min_vim[1];
                    V3DLONG tile2vi_ye = vim.lut[ii].end_pos[1]-vim.min_vim[1];
                    V3DLONG tile2vi_zs = vim.lut[ii].start_pos[2]-vim.min_vim[2];
                    V3DLONG tile2vi_ze = vim.lut[ii].end_pos[2]-vim.min_vim[2];

                    V3DLONG x_start = (start[0] > tile2vi_xs) ? start[0] : tile2vi_xs;
                    V3DLONG x_end = (end[0] < tile2vi_xe) ? end[0] : tile2vi_xe;
                    V3DLONG y_start = (start[1] > tile2vi_ys) ? start[1] : tile2vi_ys;
                    V3DLONG y_end = (end[1] < tile2vi_ye) ? end[1] : tile2vi_ye;
                    V3DLONG z_start = (start[2] > tile2vi_zs) ? start[2] : tile2vi_zs;
                    V3DLONG z_end = (end[2] < tile2vi_ze) ? end[2] : tile2vi_ze;

                    x_end++;
                    y_end++;
                    z_end++;

                    //
                    cout << x_start << " " << x_end << " " << y_start << " " << y_end << " " << z_start << " " << z_end << endl;

                    if(datatype == V3D_UINT8)
                    {
                        region_groupfusing<unsigned char>(pVImg_UINT8, vim, relative1d,
                                                          vx, vy, vz, vc, rx, ry, rz, rc,
                                                          tile2vi_zs, tile2vi_ys, tile2vi_xs,
                                                          z_start, z_end, y_start, y_end, x_start, x_end, start);

                    }
                    else if(datatype == V3D_UINT16)
                    {
                        region_groupfusing<unsigned short>(pVImg_UINT16, vim, relative1d,
                                                           vx, vy, vz, vc, rx, ry, rz, rc,
                                                           tile2vi_zs, tile2vi_ys, tile2vi_xs,
                                                           z_start, z_end, y_start, y_end, x_start, x_end, start);
                    }
                    else if(datatype == V3D_FLOAT32)
                    {
                        region_groupfusing<float>(pVImg_FLOAT32, vim, relative1d,
                                                  vx, vy, vz, vc, rx, ry, rz, rc,
                                                  tile2vi_zs, tile2vi_ys, tile2vi_xs,
                                                  z_start, z_end, y_start, y_end, x_start, x_end, start);
                    }
                    else
                    {
                        printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                        return -1;
                    }

                    //de-alloc
                    if(relative1d) {delete []relative1d; relative1d=0;}
                    if(sz_relative) {delete []sz_relative; sz_relative=0;}
                }

            }

            //save data

            if(datatype == V3D_UINT8)
            {

                V3DLONG sz[4];

                sz[0] = vx; sz[1] = vy; sz[2] = vz; sz[3] = vc;

                QString tmp_filename = curFilePath;

                tmp_filename += str + "_" + "stitched.raw";

                qDebug()<<"tmp_filename=="<<tmp_filename;

                if (saveImage(tmp_filename.toStdString().c_str(), (const unsigned char *)pVImg_UINT8, sz, datatype)!=true)
                {
                    fprintf(stderr, "Error happens in file writing. Exit. \n");
                    return -1;
                }
                if(pVImg_UINT8) {delete []pVImg_UINT8; pVImg_UINT8=0;}
            }
            else if(datatype == V3D_UINT16)
            {
                V3DLONG sz[4];

                sz[0] = vx; sz[1] = vy; sz[2] = vz; sz[3] = vc;

                QString tmp_filename = curFilePath;

                tmp_filename += str + "_" + "stitched.raw";

                qDebug()<<"tmp_filename=="<<tmp_filename;

                if (saveImage(tmp_filename.toStdString().c_str(), (const unsigned char *)pVImg_UINT16, sz, datatype)!=true)
                {
                    fprintf(stderr, "Error happens in file writing. Exit. \n");
                    return -1 ;
                }
                if(pVImg_UINT16) {delete []pVImg_UINT16; pVImg_UINT16=0;}

            }
            else if(datatype == V3D_FLOAT32)
            {
                V3DLONG sz[4];

                sz[0] = vx; sz[1] = vy; sz[2] = vz; sz[3] = vc;

                QString tmp_filename = curFilePath;

                tmp_filename += str + "_" + "stitched.raw";

                qDebug()<<"tmp_filename=="<<tmp_filename;

                if (saveImage(tmp_filename.toStdString().c_str(), (const unsigned char *)pVImg_FLOAT32, sz, datatype)!=true)
                {
                    fprintf(stderr, "Error happens in file writing. Exit. \n");
                    return -1;
                }
                if(pVImg_FLOAT32) {delete []pVImg_FLOAT32; pVImg_FLOAT32=0;}
            }
            else
            {
                printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                return -1;
            }

        }
    }
    QMessageBox::information(0, "Batch stitch", QObject::tr("batch end"));
}
// 3d roi navigating function
int roi_navigating(V3DPluginCallback2 &callback, QWidget *parent)
{
    // passing resolution and origin and roi boundary parameters
    // current window
    v3dhandle wincurr = callback.curHiddenSelectedWindow(); //by PHC, 101009. currentImageWindow(); //
    Image4DSimple* pImgIn = callback.getImage(wincurr); //not using the image class pointer in the parameter-struct, - is there correct in the V3DLONG run? noted by PHC
    if (! pImgIn)
    {
        QMessageBox::information(0, title, QObject::tr("The pointer to the current image window is invalid. Do nothing."));
        return -2;
    }

    // parameters passed from 3d view
    v3d_imaging_paras *p = (v3d_imaging_paras *)pImgIn->getCustomStructPointer();
    if(!p)
    {
        QMessageBox::information(0, title, QObject::tr("The pointer to the parameter-struct is invalid. Do nothing."));
        return -3;
    }
    if (!(p->imgp))
    {
        QMessageBox::information(0, title, QObject::tr("The image pointer inside the parameter-struct is invalid. Do nothing."));
        return -4;
    }

    // load .tc file
    QString m_FileName = QFileDialog::getOpenFileName(parent, QObject::tr("Open A Virtual Image"),
                                                      QDir::currentPath(),
                                                      QObject::tr("Image Configuration (*.tc)"));
    if(m_FileName.isEmpty())
        return -1;

    QString curFilePath = QFileInfo(m_FileName).path();
    curFilePath.append("/");

    string filename =m_FileName.toStdString();

    // virtual image
    Y_VIM<REAL, V3DLONG, indexed_t<V3DLONG, REAL>, LUT<V3DLONG> > vim;

    if( !vim.y_load(filename) )
        return -1;


    // print info message
    qDebug()<<"-----------------------------------------------------------------------";

    qDebug()<<"-----------passing para through interface---------------";

    qDebug()<<"bb ..."<< p->xs << p->xe << p->ys << p->ye << p->zs << p->ze;
    qDebug()<<"resolution ..."<< p->xrez << p->yrez << p->zrez; // not use in this context

    qDebug()<<"-----------------------------------------------------------------------";


    // load image; if bigger than (512, 512, 512) downsample it
    // set current origin and resolution
    // input virtual image point position
    // compute FOV find start and end position in 4D virtual image
    Image4DSimple *pCur4DImg = p->imgp;

    double o_origin_x = pCur4DImg->getOriginX();
    double o_origin_y = pCur4DImg->getOriginY();
    double o_origin_z = pCur4DImg->getOriginZ();

    double o_rez_x = pCur4DImg->getRezX();
    double o_rez_y = pCur4DImg->getRezY();
    double o_rez_z = pCur4DImg->getRezZ();

    V3DLONG o_sx = pCur4DImg->getXDim();
    V3DLONG o_sy = pCur4DImg->getYDim();
    V3DLONG o_sz = pCur4DImg->getZDim();
    V3DLONG o_sc = pCur4DImg->getCDim();

    V3DLONG bb_start_x = p->xs;
    V3DLONG bb_end_x = p->xe;
    V3DLONG bb_start_y = p->ys;
    V3DLONG bb_end_y = p->ye;
    V3DLONG bb_start_z = p->zs;
    V3DLONG bb_end_z = p->ze;

    //
    V3DLONG xs, xe, ys, ye, zs, ze;

    double vz_x = vim.rez_x;
    double vz_y = vim.rez_y;
    double vz_z = vim.rez_z;

    double vo_x = vim.origin_x;
    double vo_y = vim.origin_y;
    double vo_z = vim.origin_z;

    double n_origin_x = o_origin_x + (o_rez_x * (double)bb_start_x);
    double n_origin_y = o_origin_y + (o_rez_y * (double)bb_start_y);
    double n_origin_z = o_origin_z + (o_rez_z * (double)bb_start_z);

    V3DLONG n_sx = ((double)(bb_end_x - bb_start_x + 1))*o_rez_x / vz_x;
    V3DLONG n_sy = ((double)(bb_end_y - bb_start_y + 1))*o_rez_y / vz_y;
    V3DLONG n_sz = ((double)(bb_end_z - bb_start_z + 1))*o_rez_z / vz_z;

    xs = (n_origin_x - vo_x) / vz_x;	xe = n_sx - 1 + xs;
    ys = (n_origin_y - vo_y) / vz_y;	ye = n_sy - 1 + ys;
    zs = (n_origin_z - vo_z) / vz_z;	ze = n_sz - 1 + zs;

    // error checking
    V3DLONG sx = vim.sz[0];
    V3DLONG sy = vim.sz[1];
    V3DLONG sz = vim.sz[2];
    V3DLONG sc = vim.sz[3];

    if(n_sx>sx || n_sy>sy || n_sz>sz)
    {
        printf("The ROI image is too large. \n");
        return -1;
    }

    // generate ROI image
    V3DLONG start[3], end[3];

    start[0] = xs;
    start[1] = ys;
    start[2] = zs;

    end[0] = xe;
    end[1] = ye;
    end[2] = ze;

    //virtual image
    V3DLONG vx, vy, vz, vc;

    vx = end[0] - start[0] + 1; // suppose the size same of all tiles
    vy = end[1] - start[1] + 1;
    vz = end[2] - start[2] + 1;
    vc = vim.sz[3];

    V3DLONG pagesz_vim = vx*vy*vz*vc;

    unsigned char *pVImg = 0;

    try
    {
        pVImg = new unsigned char [pagesz_vim];
    }
    catch (...)
    {
        printf("Fail to allocate memory.\n");
        return -1;
    }

    // init
    for(V3DLONG i=0; i<pagesz_vim; i++)
    {
        pVImg[i] = 0;
    }

    //
    if(o_rez_x<=vz_x && o_rez_y<=vz_y && o_rez_z<=vz_z)
    {
        // load image in memory
        unsigned char* curdata1d = pCur4DImg->getRawData();

        for(V3DLONG c=0; c<vc; c++)
        {
            V3DLONG offset_c = c*vx*vy*vz;
            V3DLONG offset_cur_c = c*sx*sy*sz;
            for(V3DLONG k=0; k<vz; k++)
            {
                V3DLONG offset_k = offset_c + k*vx*vy;
                V3DLONG offset_cur_k = offset_cur_c + (k+zs)*sx*sy;
                for(V3DLONG j=0; j<vy; j++)
                {
                    V3DLONG offset_j = offset_k + j*vx;
                    V3DLONG offset_cur_j = offset_cur_k + (j+ys)*sx;
                    for(V3DLONG i=0; i<vx; i++)
                    {
                        V3DLONG idx = offset_j + i;
                        V3DLONG idx_cur = offset_cur_j + i + xs;

                        pVImg[idx] = curdata1d[idx_cur];
                    }
                }
            }
        }

        // result handling
        Image4DSimple p4DImage;
        p4DImage.setData(pVImg, vx, vy, vz, vc, pCur4DImg->getDatatype());

        p4DImage.setRezX(vz_x); p4DImage.setRezY(vz_y); p4DImage.setRezZ(vz_z);
        p4DImage.setOriginX(n_origin_x); p4DImage.setOriginY(n_origin_y); p4DImage.setOriginZ(n_origin_z);

        v3dhandle newwin = callback.newImageWindow();
        callback.setImage(newwin, &p4DImage);
        callback.setImageName(newwin, "ROI of A Virtual Image");
        callback.updateImageWindow(newwin);

    }
    else
    {
        // load image from hard drive

        // flu bird algorithm
        // 000 'f', 'l', 'u' ; 111 'b', 'r', 'd'; relative[2] relative[1] relative[0]
        bitset<3> lut_ss, lut_se, lut_es, lut_ee;

        //
        V3DLONG x_s = start[0]; // + vim.min_vim[0];
        V3DLONG y_s = start[1]; // + vim.min_vim[1];
        V3DLONG z_s = start[2]; // + vim.min_vim[2];

        V3DLONG x_e = end[0]; // + vim.min_vim[0];
        V3DLONG y_e = end[1]; // + vim.min_vim[1];
        V3DLONG z_e = end[2]; // + vim.min_vim[2];

        //
        ImagePixelType datatype;

        // look up lut
        for(V3DLONG ii=0; ii<vim.number_tiles; ii++)
        {
            // init
            lut_ss.reset();
            lut_se.reset();
            lut_es.reset();
            lut_ee.reset();

            //
            if(x_s < vim.lut[ii].start_pos[0]) lut_ss[1] = 1; // r  0 l
            if(y_s < vim.lut[ii].start_pos[1]) lut_ss[0] = 1; // d  0 u
            if(z_s < vim.lut[ii].start_pos[2]) lut_ss[2] = 1; // b  0 f

            if(x_e < vim.lut[ii].start_pos[0]) lut_se[1] = 1; // r  0 l
            if(y_e < vim.lut[ii].start_pos[1]) lut_se[0] = 1; // d  0 u
            if(z_e < vim.lut[ii].start_pos[2]) lut_se[2] = 1; // b  0 f

            if(x_s < vim.lut[ii].end_pos[0]) lut_es[1] = 1; // r  0 l
            if(y_s < vim.lut[ii].end_pos[1]) lut_es[0] = 1; // d  0 u
            if(z_s < vim.lut[ii].end_pos[2]) lut_es[2] = 1; // b  0 f

            if(x_e < vim.lut[ii].end_pos[0]) lut_ee[1] = 1; // r  0 l
            if(y_e < vim.lut[ii].end_pos[1]) lut_ee[0] = 1; // d  0 u
            if(z_e < vim.lut[ii].end_pos[2]) lut_ee[2] = 1; // b  0 f

            // copy data
            if( (!lut_ss.any() && lut_ee.any()) || (lut_es.any() && !lut_ee.any()) || (lut_ss.any() && !lut_se.any()) )
            {
                //
                cout << "satisfied image: "<< vim.lut[ii].fn_img << endl;

                // loading relative imagg files
                V3DLONG *sz_relative = 0;
                int datatype_relative = 0;
                unsigned char* relative1d = 0;

                QString curPath = curFilePath;

                string fn = curPath.append( QString(vim.lut[ii].fn_img.c_str()) ).toStdString();

                qDebug()<<"testing..."<<curFilePath<< fn.c_str();

                if (loadImage(const_cast<char *>(fn.c_str()), relative1d, sz_relative, datatype_relative)!=true)
                {
                    fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",vim.tilesList.at(ii).fn_image.c_str());
                    return -1;
                }
                V3DLONG rx=sz_relative[0], ry=sz_relative[1], rz=sz_relative[2], rc=sz_relative[3];

                if(datatype_relative==1)
                    datatype = V3D_UINT8;

                //
                V3DLONG tile2vi_xs = vim.lut[ii].start_pos[0]-vim.min_vim[0];
                V3DLONG tile2vi_xe = vim.lut[ii].end_pos[0]-vim.min_vim[0];
                V3DLONG tile2vi_ys = vim.lut[ii].start_pos[1]-vim.min_vim[1];
                V3DLONG tile2vi_ye = vim.lut[ii].end_pos[1]-vim.min_vim[1];
                V3DLONG tile2vi_zs = vim.lut[ii].start_pos[2]-vim.min_vim[2];
                V3DLONG tile2vi_ze = vim.lut[ii].end_pos[2]-vim.min_vim[2];

                V3DLONG x_start = (start[0] > tile2vi_xs) ? start[0] : tile2vi_xs;
                V3DLONG x_end = (end[0] < tile2vi_xe) ? end[0] : tile2vi_xe;
                V3DLONG y_start = (start[1] > tile2vi_ys) ? start[1] : tile2vi_ys;
                V3DLONG y_end = (end[1] < tile2vi_ye) ? end[1] : tile2vi_ye;
                V3DLONG z_start = (start[2] > tile2vi_zs) ? start[2] : tile2vi_zs;
                V3DLONG z_end = (end[2] < tile2vi_ze) ? end[2] : tile2vi_ze;

                x_end++;
                y_end++;
                z_end++;

                //
                cout << x_start << " " << x_end << " " << y_start << " " << y_end << " " << z_start << " " << z_end << endl;

                //
                for(V3DLONG c=0; c<rc; c++)
                {
                    V3DLONG o_c = c*vx*vy*vz;
                    V3DLONG o_r_c = c*rx*ry*rz;
                    for(V3DLONG k=z_start; k<z_end; k++)
                    {
                        V3DLONG o_k = o_c + (k-start[2])*vx*vy;
                        V3DLONG o_r_k = o_r_c + (k-z_start)*rx*ry;

                        for(V3DLONG j=y_start; j<y_end; j++)
                        {
                            V3DLONG o_j = o_k + (j-start[1])*vx;
                            V3DLONG o_r_j = o_r_k + (j-y_start)*rx;
                            for(V3DLONG i=x_start; i<x_end; i++)
                            {
                                V3DLONG idx = o_j + i-start[0];
                                V3DLONG idx_r = o_r_j + (i-x_start);

                                if(pVImg[idx]>0)
                                    pVImg[idx] = (pVImg[idx]>relative1d[idx_r])?pVImg[idx]:relative1d[idx_r];
                                else
                                    pVImg[idx] = relative1d[idx_r];
                            }
                        }
                    }
                }

                //de-alloc
                if(relative1d) {delete []relative1d; relative1d=0;}
                if(sz_relative) {delete []sz_relative; sz_relative=0;}
            }

        }

        // down sampling ?
        REAL *scale = new REAL [3];

        scale[0] = 512.0 / (REAL)vx; // default settings
        scale[1] = 512.0 / (REAL)vy;
        scale[2] = 512.0 / (REAL)vz;

        if( scale[0]<1 || scale[1]<1 || scale[2]<1)
        {
            if(scale[0]>1.0) scale[0] = 1.0;
            if(scale[1]>1.0) scale[1] = 1.0;
            if(scale[2]>1.0) scale[2] = 1.0;

            // downsampling
            V3DLONG ssx, ssy, ssz;

            ssx = (V3DLONG)(ceil(vx*scale[0]));
            ssy = (V3DLONG)(ceil(vy*scale[1]));
            ssz = (V3DLONG)(ceil(vz*scale[2]));

            V3DLONG pagesz_ds_sub = ssx*ssy*ssz*vc;

            //
            V3DLONG *sz_down_subject = new V3DLONG [3];

            sz_down_subject[0] = ssx;
            sz_down_subject[1] = ssy;
            sz_down_subject[2] = ssz;

            V3DLONG *sz_vim = new V3DLONG [3];

            sz_vim[0] = vx;
            sz_vim[1] = vy;
            sz_vim[2] = vz;

            unsigned char* p_sub = NULL;

            try
            {
                p_sub = new unsigned char [pagesz_ds_sub];

                for(V3DLONG c=0; c<vc; c++)
                {
                    Y_IMG_UINT8 pIn(pVImg+c*vx*vy*vz, sz_vim);
                    Y_IMG_UINT8 pOut(p_sub+c*ssx*ssy*ssz, sz_down_subject);

                    YImg<REAL, V3DLONG, Y_IMG_UINT8, Y_IMG_UINT8> tmp;
                    tmp.down_sampling(pOut, pIn, scale);

                }
            }
            catch(...)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }

            // result handling
            Image4DSimple p4DImage;
            p4DImage.setData(p_sub, ssx, ssy, ssz, vc, pCur4DImg->getDatatype());

            p4DImage.setRezX(vz_x/(double)scale[0]); p4DImage.setRezY(vz_y/(double)scale[1]); p4DImage.setRezZ(vz_z/(double)scale[2]);
            p4DImage.setOriginX(n_origin_x); p4DImage.setOriginY(n_origin_y); p4DImage.setOriginZ(n_origin_z);

            v3dhandle newwin = callback.newImageWindow();
            callback.setImage(newwin, &p4DImage);
            callback.setImageName(newwin, "ROI of A Virtual Image");
            callback.updateImageWindow(newwin);

        }
        else
        {
            // result handling
            Image4DSimple p4DImage;
            p4DImage.setData(pVImg, vx, vy, vz, vc, pCur4DImg->getDatatype());

            p4DImage.setRezX(vz_x); p4DImage.setRezY(vz_y); p4DImage.setRezZ(vz_z);
            p4DImage.setOriginX(n_origin_x); p4DImage.setOriginY(n_origin_y); p4DImage.setOriginZ(n_origin_z);

            v3dhandle newwin = callback.newImageWindow();
            callback.setImage(newwin, &p4DImage);
            callback.setImageName(newwin, "ROI of A Virtual Image");
            callback.updateImageWindow(newwin);

        }


    }

    return true;
}

// image stitch plugin dofunc
bool IStitchPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("v3dstitch"))
    {
        if(input.size()<1) return false; // no inputs

        vector<char*> * infilelist = (vector<char*> *)(input.at(0).p);
        vector<char*> * paralist;
        vector<char*> * outfilelist;
        if(infilelist->empty())
        {
            //print Help info
            printf("\nUsage: v3d -x imageStitch.dylib -f v3dstitch -i <input_image_folder> -o <output_image_file> -p \"[#c <channalNo_reference> #x <downsample_factor_x> #y <downsample_factor_y> #z <downsample_factor_z> #l <overlap_ratio>] #sb [saving_tile_boundary 0/1] #si <saving_stitching_result 0/1>\"\n");


            return true;
        }

        char * infile = infilelist->at(0); // input_image_folder
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_image_file

        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p); outfile = outfilelist->at(0);}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        // init
        QString m_InputFolder(infile);

        int channel1 = 0;

        REAL overlap_percent = 0.01;

        bool axes_show = false; // show tile boundary
        bool img_show = true; // save stitching file

        REAL *scale = new REAL [6];

        scale[0] = 0.2;
        scale[1] = 0.2;
        scale[2] = 0.2;
        scale[3] = 1;
        scale[4] = 1;
        scale[5] = 1;

        bool m_similarity = false;
        bool success = false;

        ImagePixelType imgdatatype;
        V3DLONG cdim;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[200];
            char * myparas = new char[len];
            strcpy(myparas, paras);
            for(int i = 0; i < len; i++)
            {
                if(i==0 && myparas[i] != ' ' && myparas[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((myparas[i-1] == ' ' || myparas[i-1] == '\t') && (myparas[i] != ' ' && myparas[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }
            char ** argv = new char* [argc];
            for(int i = 0; i < argc; i++)
            {
                argv[i] = myparas + posb[i];
            }
            for(int i = 0; i < len; i++)
            {
                if(myparas[i]==' ' || myparas[i]=='\t')
                    myparas[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "c"))
                            {
                                channel1 = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
                                i++;
                            }
                            else if (!strcmp(key, "x"))
                            {
                                scale[0] = atof( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "y"))
                            {
                                scale[1] = atof( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "z"))
                            {
                                scale[2] = atof( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "l"))
                            {
                                overlap_percent = atof( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "sb"))
                            {
                                key++;

                                axes_show = atoi( argv[i+1] )?true:false;
                                i++;
                            }
                            else if (!strcmp(key, "si"))
                            {
                                key++;

                                img_show = atoi( argv[i+1] )?true:false;
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }

                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }

                }
            }

            // error check
            if(channel1<0 || scale[0]<0.0 || scale[1]<0.0 || scale[2]<0.0 || overlap_percent<0.0 || overlap_percent>1.0)
            {
                cout<<"illegal input parameters"<<endl;
                return false;
            }
        }

        // group stitch
        int start_t = clock();

        // load tiles and stitch
        //----------------------------------------------------------------------------------------------------------------------------------------------------
        QStringList imgList = importSeriesFileList_addnumbersort(m_InputFolder);
        QString m_InputFileName = imgList.at(0);

        Y_VIM<REAL, V3DLONG, indexed_t<V3DLONG, REAL>, LUT<V3DLONG> > vim;

        V3DLONG count=0;
        foreach (QString img_str, imgList)
        {
            V3DLONG offset[3];
            offset[0]=0; offset[1]=0; offset[2]=0;

            indexed_t<V3DLONG, REAL> idx_t(offset);

            idx_t.n = count;
            idx_t.ref_n = 0; // init with default values
            idx_t.fn_image = img_str.toStdString();
            idx_t.score = 0;

            vim.tilesList.push_back(idx_t);

            count++;
        }

        // stitching image pairs
        // suppose 0 as a reference
        int NTILES = vim.tilesList.size();
        int NTILES_I = NTILES - 1;
        int NTILES_II = NTILES_I - 1;

        // first step: rough estimation in a coarse scale
        V3DLONG offsets[3];
        for(int i=0; i<NTILES_I; i++) // record all the sz_image information
        {
            //loading target files
            V3DLONG *sz_target = 0;
            int datatype_target = 0;
            unsigned char* target1d = 0;

            if (loadImage(const_cast<char *>(vim.tilesList.at(i).fn_image.c_str()), target1d, sz_target, datatype_target)!=true)
            {
                fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",vim.tilesList.at(i).fn_image.c_str());
                return false;
            }
            V3DLONG tx=sz_target[0], ty=sz_target[1], tz=sz_target[2], tc=sz_target[3];
            cdim = tc; // init

            if(i==0)
            {
                imgdatatype = (ImagePixelType)datatype_target;
                if(datatype_target==4) imgdatatype = V3D_FLOAT32;
            }
            else
            {
                if(datatype_target != imgdatatype)
                {
                    printf("The program only support all tiled images with the same datatype.\n");
                    return false;
                }
            }

            (&vim.tilesList.at(i))->sz_image = new V3DLONG [4];

            (&vim.tilesList.at(i))->sz_image[0] = tx;
            (&vim.tilesList.at(i))->sz_image[1] = ty;
            (&vim.tilesList.at(i))->sz_image[2] = tz;
            (&vim.tilesList.at(i))->sz_image[3] = tc;

            // channel of target
            V3DLONG offsets_tar = channel1*tx*ty*tz;

            // try rest of tiles
            for(int j=i+1; j<NTILES; j++)
            {
                //loading subject files
                V3DLONG *sz_subject = 0;
                int datatype_subject = 0;
                unsigned char* subject1d = 0;

                if (loadImage(const_cast<char *>(vim.tilesList.at(j).fn_image.c_str()), subject1d, sz_subject, datatype_subject)!=true)
                {
                    fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n", vim.tilesList.at(j).fn_image.c_str());
                    return false;
                }

                V3DLONG sx=sz_subject[0], sy=sz_subject[1], sz=sz_subject[2], sc=sz_subject[3];

                // init
                if(i==NTILES_II)
                {
                    (&vim.tilesList.at(j))->sz_image = new V3DLONG [4];

                    (&vim.tilesList.at(j))->sz_image[0] = sx;
                    (&vim.tilesList.at(j))->sz_image[1] = sy;
                    (&vim.tilesList.at(j))->sz_image[2] = sz;
                    (&vim.tilesList.at(j))->sz_image[3] = sc;
                }

                // channel of subject
                V3DLONG offsets_sub = channel1*sx*sy*sz;

                // try
                PEAKS *pos = new PEAKS;

                if(imgdatatype == V3D_UINT8)
                {
                    success = stitching_bb_thickplanes<unsigned char, Y_IMG_UINT8>((unsigned char *)subject1d+offsets_sub, sz_subject, (unsigned char *)target1d+offsets_tar, sz_target, overlap_percent, m_similarity, scale, pos, 1);
                }
                else if(imgdatatype == V3D_UINT16)
                {
                    success = stitching_bb_thickplanes<unsigned short, Y_IMG_UINT16>((unsigned short *)(subject1d)+offsets_sub, sz_subject, (unsigned short *)(target1d)+offsets_tar, sz_target, overlap_percent, m_similarity, scale, pos, 1);
                }
                else if(imgdatatype == V3D_FLOAT32)
                {
                    success = stitching_bb_thickplanes<REAL, Y_IMG_REAL>((REAL *)(subject1d)+offsets_sub, sz_subject, (REAL *)(target1d)+offsets_tar, sz_target, overlap_percent, m_similarity, scale, pos, 1);
                }
                else
                {
                    printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                    return false;
                }
                if(success!=true) return false;

                cout<< "pairwise pos " << pos->x << " " << pos->y  << " " << pos->z << " " << pos->value << endl;

                //record n by n-1
                offsets[0] = pos->x - sx +1;
                offsets[1] = pos->y - sy +1;
                offsets[2] = pos->z - sz +1;

                indexed_t<V3DLONG, REAL> t(offsets);  //

                t.score = pos->value;
                t.n = vim.tilesList.at(i).n;

                (&vim.tilesList.at(j))->record.push_back(t);

                qDebug()<<"subimg"<<j<<"tarimg"<<i<<"offsets ..."<<t.offsets[0]<<t.offsets[1]<<t.offsets[2];

                cout << " subject "<< vim.tilesList.at(i).fn_image << " over " << " target " <<vim.tilesList.at(j).fn_image << endl;

                //de-alloc
                if(subject1d) {delete []subject1d; subject1d=0;}
                if(sz_subject) {delete []sz_subject; sz_subject=0;}

            }

            //de-alloc
            if(target1d) {delete []target1d; target1d=0;}
            if(sz_target) {delete []sz_target; sz_target=0;}

        }

        // find mst of whole tiled images
        for(int i=0; i<NTILES; i++)
        {
            vim.tilesList.at(i).visited = false;
        }
        if(mstPrim(vim.tilesList)) // run Prim's algorithm
        {
            cout<<"Fail to call MST approach!"<<endl;
            return false;
        }

        //define threshold of correlation coeffecient score
        //REAL threshold_corr_score = 0.75; //

        //find a ref. image
        //std::vector<indexed_t>::iterator iterStart = vim.tilesList.begin();

        //	bool del_tile = true;
        //
        //	for(int iter = 0; iter < NTILES_I; iter++)
        //	{
        //
        //		for(int j=iter+1; j<NTILES; j++)
        //		{
        //
        //			if( vim.tilesList.at(j).ref_n == vim.tilesList.at(iter).n && vim.tilesList.at(j).score>threshold_corr_score) //
        //			{
        //				del_tile = false;
        //				break;
        //			}
        //
        //		}
        //
        //		if(del_tile)
        //		{
        //			vim.tilesList.erase( vim.tilesList.begin()+iter );
        //
        //			NTILES = vim.tilesList.size();
        //			NTILES_I = NTILES - 1;
        //
        //			iter--;
        //		}
        //		else
        //			break;
        //
        //	}

        //stitch at a fine scale
        NTILES = vim.tilesList.size();
        NTILES_I = NTILES - 1;
        NTILES_II = NTILES_I - 1;

        if(scale[0]==1 && scale[1]==1 && scale[2]==1)
        {
            //continue;
        }
        else
        {
            //
            for(int i=0; i<NTILES; i++)
            {
                vim.tilesList.at(i).visited = false;
            }

            //
            for(int i=1; i<NTILES; i++) // traverse all tiled images
            {
                PEAKS *pos = new PEAKS;

                V3DLONG current = vim.tilesList.at(i).n;
                V3DLONG previous = vim.tilesList.at(i).predecessor;

                if(vim.tilesList.at(current).visited) // computed
                {
                    continue;
                }

                //loading subject files
                V3DLONG *sz_subject = 0;
                int datatype_subject = 0;
                unsigned char* subject1d = 0;

                if (loadImage(const_cast<char *>(vim.tilesList.at(current).fn_image.c_str()), subject1d, sz_subject, datatype_subject)!=true)
                {
                    fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n", vim.tilesList.at(current).fn_image.c_str());
                    return false;
                }
                V3DLONG sx=sz_subject[0], sy=sz_subject[1], sz=sz_subject[2], sc=sz_subject[3];

                // try rest of tiles
                while(previous!=-1)
                {

                    //loading target files
                    V3DLONG *sz_target = 0;
                    int datatype_target = 0;
                    unsigned char* target1d = 0;

                    if (loadImage(const_cast<char *>(vim.tilesList.at(previous).fn_image.c_str()), target1d, sz_target, datatype_target)!=true)
                    {
                        fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",vim.tilesList.at(previous).fn_image.c_str());
                        return false;
                    }
                    V3DLONG tx=sz_target[0], ty=sz_target[1], tz=sz_target[2], tc=sz_target[3];

                    //
                    if(vim.tilesList.at(current).visited)
                    {
                        current = previous;
                        previous = vim.tilesList.at(current).predecessor;

                        //de-alloc
                        if(subject1d) {delete []subject1d; subject1d=0;}
                        if(sz_subject) {delete []sz_subject; sz_subject=0;}

                        //
                        subject1d = target1d;
                        sz_subject = sz_target;
                        sx=sz_subject[0], sy=sz_subject[1], sz=sz_subject[2], sc=sz_subject[3];

                        //
                        continue;
                    }

                    //
                    V3DLONG offsets_tar = channel1*tx*ty*tz;
                    V3DLONG offsets_sub = channel1*sx*sy*sz;

                    //
                    pos->x = vim.tilesList.at(current).offsets[0] + (sx-1);
                    pos->y = vim.tilesList.at(current).offsets[1] + (sy-1);
                    pos->z = vim.tilesList.at(current).offsets[2] + (sz-1);
                    pos->value = vim.tilesList.at(current>previous?current:previous).record.at(current<previous?current:previous).score; //

                    //
                    if(imgdatatype == V3D_UINT8)
                    {
                        success = stitching_bb_thickplanes<unsigned char, Y_IMG_UINT8>((unsigned char *)subject1d+offsets_sub, sz_subject, (unsigned char *)target1d+offsets_tar, sz_target, overlap_percent, m_similarity, scale, pos, 2);
                    }
                    else if(imgdatatype == V3D_UINT16)
                    {
                        success = stitching_bb_thickplanes<unsigned short, Y_IMG_UINT16>((unsigned short *)(subject1d)+offsets_sub, sz_subject, (unsigned short *)(target1d)+offsets_tar, sz_target, overlap_percent, m_similarity, scale, pos, 2);
                    }
                    else if(imgdatatype == V3D_FLOAT32)
                    {
                        success = stitching_bb_thickplanes<REAL, Y_IMG_REAL>((REAL *)(subject1d)+offsets_sub, sz_subject, (REAL *)(target1d)+offsets_tar, sz_target, overlap_percent, m_similarity, scale, pos, 2);
                    }
                    else
                    {
                        printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                        return -1;
                    }
                    if(success!=true) return false;

                    // update offsets of current to previous
                    (&vim.tilesList.at(current))->offsets[0] = pos->x - sx +1;
                    (&vim.tilesList.at(current))->offsets[1] = pos->y - sy +1;
                    (&vim.tilesList.at(current))->offsets[2] = pos->z - sz +1;

                    (&vim.tilesList.at(current))->visited = true;

                    //
                    current = previous;
                    previous = vim.tilesList.at(current).predecessor;

                    //de-alloc
                    if(subject1d) {delete []subject1d; subject1d=0;}
                    if(sz_subject) {delete []sz_subject; sz_subject=0;}

                    //
                    subject1d = target1d;
                    sz_subject = sz_target;
                    sx=sz_subject[0], sy=sz_subject[1], sz=sz_subject[2], sc=sz_subject[3];

                    if(previous==-1)
                    {
                        if(target1d) {delete []target1d; target1d=0;}
                        if(sz_target) {delete []sz_target; sz_target=0;}
                    }

                }
            }
        }
        //de-alloc
        y_del<REAL>(scale);

        // adjusting offset reference to ref. image
        // compute accumulate offsets from path list
        int ref_image = vim.tilesList.at(0).n;
        (&vim.tilesList.at(0))->offsets[0] = 0;
        (&vim.tilesList.at(0))->offsets[1] = 0;
        (&vim.tilesList.at(0))->offsets[2] = 0;

        for(int i=0; i<NTILES; i++)
        {
            vim.tilesList.at(i).visited = false;
        }

        for(int i_path=1; i_path<NTILES; i_path++)
        {
            // record final shifts
            vim.tilesList.at(i_path).record.at(0).offsets[0] = vim.tilesList.at(i_path).offsets[0];
            vim.tilesList.at(i_path).record.at(0).offsets[1] = vim.tilesList.at(i_path).offsets[1];
            vim.tilesList.at(i_path).record.at(0).offsets[2] = vim.tilesList.at(i_path).offsets[2];
        }

        for(int i_path=1; i_path<NTILES; i_path++)
        {
            // ref
            (&vim.tilesList.at(i_path))->ref_n = ref_image;

            V3DLONG current = vim.tilesList.at(i_path).n;
            V3DLONG previous = vim.tilesList.at(i_path).predecessor;

            //
            while(previous!=-1)
            {
                if(vim.tilesList.at(current).visited)
                {
                    break;
                }

                (&vim.tilesList.at(i_path))->offsets[0] += vim.tilesList.at(previous).offsets[0];
                (&vim.tilesList.at(i_path))->offsets[1] += vim.tilesList.at(previous).offsets[1];
                (&vim.tilesList.at(i_path))->offsets[2] += vim.tilesList.at(previous).offsets[2];


                //
                current = previous;
                previous = vim.tilesList.at(current).predecessor;
            }

            (&vim.tilesList.at(i_path))->visited = true;
        }

        // construct lookup table
        vim.y_clut(vim.tilesList.size());

        //------------------------------------------------------------------------------------------------------------------------------------------
        // save lut
        QString tmp_filename = QFileInfo(m_InputFileName).path() + "/" + "stitched_image.tc"; //.tc tile configuration

        vim.y_save(tmp_filename.toStdString());

        //
        int end_t = clock();
        qDebug("time eclapse %d seconds for image stitching.", (end_t-start_t));


        //------------------------------------------------------------------------------------------------------------------------------------------
        // output
        if(img_show)
        {
            V3DLONG vx, vy, vz, vc;

            vx = vim.max_vim[0] - vim.min_vim[0] + 1; //
            vy = vim.max_vim[1] - vim.min_vim[1] + 1;
            vz = vim.max_vim[2] - vim.min_vim[2] + 1;
            vc = cdim;

            V3DLONG pagesz_vim = vx*vy*vz*vc;

            qDebug()<<"vim dims ..."<< vx << vy << vz << vc;

            QString stitchFileName;
            m_InputFileName.chop(4);
            if(!outfile)
                stitchFileName = m_InputFileName + "_stitched.raw";
            else
                stitchFileName = QString(outfile);

            V3DLONG sz_tmp[4];

            sz_tmp[0] = vx; sz_tmp[1] = vy; sz_tmp[2] = vz; sz_tmp[3] = vc;

            if(imgdatatype == V3D_UINT8)
            {
                // init
                unsigned char *pVImg = NULL;

                try
                {
                    pVImg = new unsigned char [pagesz_vim];

                    memset(pVImg, 0, sizeof(unsigned char)*pagesz_vim);
                }
                catch (...)
                {
                    printf("Fail to allocate memory.\n");
                    return false;
                }

                //
                success = groupi_fusing<unsigned char>((unsigned char *)pVImg, vim, vx, vy, vz, vc, axes_show);

                //save
                if (saveImage(stitchFileName.toStdString().c_str(), (const unsigned char *)pVImg, sz_tmp, 1)!=true)
                {
                    fprintf(stderr, "Error happens in file writing. Exit. \n");
                    return false;
                }

            }
            else if(imgdatatype == V3D_UINT16)
            {
                // init
                unsigned short *pVImg = NULL;

                try
                {
                    pVImg = new unsigned short [pagesz_vim];

                    memset(pVImg, 0, sizeof(unsigned short)*pagesz_vim);
                }
                catch (...)
                {
                    printf("Fail to allocate memory.\n");
                    return false;
                }

                //
                success = groupi_fusing<unsigned short>((unsigned short *)pVImg, vim, vx, vy, vz, vc, axes_show);

                //save
                if (saveImage(stitchFileName.toStdString().c_str(), (const unsigned char *)pVImg, sz_tmp, 2)!=true)
                {
                    fprintf(stderr, "Error happens in file writing. Exit. \n");
                    return false;
                }
            }
            else if(imgdatatype == V3D_FLOAT32)
            {
                // init
                REAL *pVImg = NULL;

                try
                {
                    pVImg = new REAL [pagesz_vim];

                    memset(pVImg, 0, sizeof(REAL)*pagesz_vim);
                }
                catch (...)
                {
                    printf("Fail to allocate memory.\n");
                    return false;
                }

                //
                success = groupi_fusing<REAL>((REAL *)pVImg, vim, vx, vy, vz, vc, axes_show);

                //save
                if (saveImage(stitchFileName.toStdString().c_str(), (const unsigned char *)pVImg, sz_tmp, 4)!=true)
                {
                    fprintf(stderr, "Error happens in file writing. Exit. \n");
                    return false;
                }
            }
            else
            {
                printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                return false;
            }
            if(success!=true) return false;
        }

        return true;
    }
    else if (func_name == tr("istitch-subspace"))
    {
        // subpixel translation registration based on pixel-level translation estimation
        if(input.size()<1) return false; // no inputs

        vector<char*> * infilelist = (vector<char*> *)(input.at(0).p);
        vector<char*> * paralist;
        vector<char*> * outfilelist;
        if(infilelist->empty())
        {
            //print Help info
            printf("\nUsage: v3d -x imageStitch.dylib -f istitch-subspace -i <input_image_folder> -o <output_image_file> -p \"[#c <channalNo_reference>] #s <saving_stitching_result 0/1>\"\n");

            return true;
        }

        char * infile = infilelist->at(0); // input_image_folder
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_image_file

        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p); outfile = outfilelist->at(0);}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        // init
        QString m_InputFolder(infile);

        int channel1 = 0;
        bool img_show = true; // save stitching file

        bool success = false;

        ImagePixelType imgdatatype;
        V3DLONG cdim;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[200];
            char * myparas = new char[len];
            strcpy(myparas, paras);
            for(int i = 0; i < len; i++)
            {
                if(i==0 && myparas[i] != ' ' && myparas[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((myparas[i-1] == ' ' || myparas[i-1] == '\t') && (myparas[i] != ' ' && myparas[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }
            char ** argv = new char* [argc];
            for(int i = 0; i < argc; i++)
            {
                argv[i] = myparas + posb[i];
            }
            for(int i = 0; i < len; i++)
            {
                if(myparas[i]==' ' || myparas[i]=='\t')
                    myparas[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "c"))
                            {
                                channel1 = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
                                i++;
                            }
                            else if (!strcmp(key, "s"))
                            {
                                img_show = atoi( argv[i+1] )?true:false;
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }

                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }

                }
            }

            // error check
            if(channel1<0)
            {
                cout<<"illegal input parameters"<<endl;
                return false;
            }
        }

        // get stitch configuration
        QDir myDir(infile);
        QStringList list = myDir.entryList(QStringList("*.tc"));

        if(list.size()!=1)
        {
            printf("Must have only one stitching configuration file!\n");
            return false;
        }

        // group stitch in subspace
        int start_t = clock();

        // load .tc
        Y_VIM<REAL, V3DLONG, indexed_t<V3DLONG, REAL>, LUT<V3DLONG> > vim;

        QString tcfile = QString(infile).append("/").append(list.at(0));

        if( !vim.y_load(tcfile.toStdString()) )
        {
            printf("Wrong stitching configuration file to be load!\n");
            return false;
        }

        //
        V3DLONG vx, vy, vz, vc;

        vx = vim.sz[0]; //
        vy = vim.sz[1];
        vz = vim.sz[2];
        vc = vim.sz[3];

        V3DLONG pagesz_vim = vx*vy*vz*vc;

        int datatype_tile = 0; // assume all tiles with the same datatype
        for(V3DLONG ii=0; ii<vim.number_tiles; ii++)
        {
            // load tile
            V3DLONG *sz_relative = 0;
            unsigned char* relative1d = 0;

            if(QFile(QString(vim.tilesList.at(ii).fn_image.c_str())).exists())
            {
                if (loadImage(const_cast<char *>(vim.tilesList.at(ii).fn_image.c_str()), relative1d, sz_relative, datatype_tile)!=true)
                {
                    fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",vim.tilesList.at(ii).fn_image.c_str());
                    return -1;
                }

                // de-alloca
                if(relative1d) {delete []relative1d; relative1d=NULL;}

                break;
            }
            else
            {
                continue;
            }
        }
        if(datatype_tile == V3D_UINT8)
        {
            // init
            unsigned char *pVImg = NULL;

            try
            {
                pVImg = new unsigned char [pagesz_vim];

                memset(pVImg, 0, sizeof(unsigned char)*pagesz_vim);
            }
            catch (...)
            {
                printf("Fail to allocate memory for pVImg.\n");
                if(pVImg) {delete []pVImg; pVImg=NULL;}
                return false;
            }

            //
            unsigned char intensityrange = 255;
            bool success = iSubspaceStitching<unsigned char, Y_IMG_UINT8>((unsigned char *)pVImg, vim, intensityrange, channel1, img_show);
            if(!success)
            {
                printf("Fail to call function iSubspaceStitching! \n");
                return false;
            }

            // output
            if(img_show)
            {
                QString stitchFileName;
                if(!outfile)
                    stitchFileName = m_InputFolder + "/subspaceStitched.v3draw";
                else
                    stitchFileName = QString(outfile);

                //save
                if (saveImage(stitchFileName.toStdString().c_str(), (const unsigned char *)pVImg, vim.sz, 1)!=true)
                {
                    fprintf(stderr, "Error happens in file writing. Exit. \n");
                    return false;
                }

                //de-alloc
                if(pVImg) {delete []pVImg; pVImg=NULL;}
            }
            else
            {
                V3DPluginArgItem arg;

                arg.type = "data"; arg.p = (void *)(pVImg); output << arg;

                V3DLONG metaImg[5]; // xyzc datatype
                metaImg[0] = vim.sz[0];
                metaImg[1] = vim.sz[1];
                metaImg[2] = vim.sz[2];
                metaImg[3] = vim.sz[3];
                metaImg[4] = datatype_tile;

                arg.type = "metaImage"; arg.p = (void *)(metaImg); output << arg;
            }
        }
        else if(datatype_tile == V3D_UINT16)
        {
            // init
            unsigned short *pVImg = NULL;

            try
            {
                pVImg = new unsigned short [pagesz_vim];

                memset(pVImg, 0, sizeof(unsigned short)*pagesz_vim);
            }
            catch (...)
            {
                printf("Fail to allocate memory for pVImg.\n");
                if(pVImg) {delete []pVImg; pVImg=NULL;}
                return false;
            }

            //
            unsigned short intensityrange = 4095;
            bool success = iSubspaceStitching<unsigned short, Y_IMG_UINT16>((unsigned short *)pVImg, vim, intensityrange, channel1, img_show);
            if(!success)
            {
                printf("Fail to call function iSubspaceStitching! \n");
                return false;
            }

            // output
            if(img_show)
            {
                QString stitchFileName;
                if(!outfile)
                    stitchFileName = m_InputFolder + "/subspaceStitched.v3draw";
                else
                    stitchFileName = QString(outfile);

                //save
                if (saveImage(stitchFileName.toStdString().c_str(), (const unsigned char *)pVImg, vim.sz, 2)!=true)
                {
                    fprintf(stderr, "Error happens in file writing. Exit. \n");
                    return false;
                }

                //de-alloc
                if(pVImg) {delete []pVImg; pVImg=NULL;}
            }
            else
            {
                V3DPluginArgItem arg;

                arg.type = "data"; arg.p = (void *)(pVImg); output << arg;

                V3DLONG metaImg[5]; // xyzc datatype
                metaImg[0] = vim.sz[0];
                metaImg[1] = vim.sz[1];
                metaImg[2] = vim.sz[2];
                metaImg[3] = vim.sz[3];
                metaImg[4] = datatype_tile;

                arg.type = "metaImage"; arg.p = (void *)(metaImg); output << arg;
            }

        }
        else if(datatype_tile == V3D_FLOAT32)
        {
            // current not supported
        }
        else
        {
            printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
            return false;
        }

        cout<<"time elapse ... "<<clock()-start_t<<endl;

        //
        return true;
    }
    else if (func_name == tr("istitch-gc"))
    {
        // to obtain global consistency by solving loop contradiction ("ghosted/blurry" problem)
        if(input.size()<1) return false; // no inputs

        vector<char*> * infilelist = (vector<char*> *)(input.at(0).p);
        vector<char*> * paralist;
        vector<char*> * outfilelist;
        if(infilelist->empty())
        {
            //print Help info
            printf("\nUsage: v3d -x imageStitch.dylib -f v3dstitch -i <input_image_folder> -o <output_image_file> -p \"[#c <channalNo_reference> #x <downsample_factor_x> #y <downsample_factor_y> #z <downsample_factor_z> #l <overlap_ratio>] #sb [saving_tile_boundary 0/1] #si <saving_stitching_result 0/1>\"\n");

            return true;
        }

        char * infile = infilelist->at(0); // input_image_folder
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_image_file

        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p); outfile = outfilelist->at(0);}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        // init
        QString m_InputFolder(infile);

        int channel1 = 0;

        REAL overlap_percent = 0.01;

        bool axes_show = false; // show tile boundary
        bool img_show = true; // save stitching file

        REAL *scale = new REAL [6];

        scale[0] = 0.2;
        scale[1] = 0.2;
        scale[2] = 0.2;
        scale[3] = 1;
        scale[4] = 1;
        scale[5] = 1;

        bool m_similarity = false;
        bool success = false;

        ImagePixelType imgdatatype;
        V3DLONG cdim;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[200];
            char * myparas = new char[len];
            strcpy(myparas, paras);
            for(int i = 0; i < len; i++)
            {
                if(i==0 && myparas[i] != ' ' && myparas[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((myparas[i-1] == ' ' || myparas[i-1] == '\t') && (myparas[i] != ' ' && myparas[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }
            char ** argv = new char* [argc];
            for(int i = 0; i < argc; i++)
            {
                argv[i] = myparas + posb[i];
            }
            for(int i = 0; i < len; i++)
            {
                if(myparas[i]==' ' || myparas[i]=='\t')
                    myparas[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "c"))
                            {
                                channel1 = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
                                i++;
                            }
                            else if (!strcmp(key, "x"))
                            {
                                scale[0] = atof( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "y"))
                            {
                                scale[1] = atof( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "z"))
                            {
                                scale[2] = atof( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "l"))
                            {
                                overlap_percent = atof( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "sb"))
                            {
                                key++;

                                axes_show = atoi( argv[i+1] )?true:false;
                                i++;
                            }
                            else if (!strcmp(key, "si"))
                            {
                                key++;

                                img_show = atoi( argv[i+1] )?true:false;
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }

                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }

                }
            }

            // error check
            if(channel1<0 || scale[0]<0.0 || scale[1]<0.0 || scale[2]<0.0 || overlap_percent<0.0 || overlap_percent>1.0)
            {
                cout<<"illegal input parameters"<<endl;
                return false;
            }
        }

        // group stitch
        int start_t = clock();

        // load tiles and stitch
        QStringList imgList = importSeriesFileList_addnumbersort(m_InputFolder);
        QString m_InputFileName = imgList.at(0);

        Y_VIM<REAL, V3DLONG, indexed_t<V3DLONG, REAL>, LUT<V3DLONG> > vim; // pixel-precise offsets lookup table

        Y_TLUT<REAL, V3DLONG, DF<REAL, V3DLONG> > tlut;// global tile configurations T

        V3DLONG count=0;
        foreach (QString img_str, imgList)
        {
            V3DLONG offset[3];
            offset[0]=0; offset[1]=0; offset[2]=0;

            indexed_t<V3DLONG, REAL> idx_t(offset);

            idx_t.n = count;
            idx_t.ref_n = 0; // init with default values
            idx_t.fn_image = img_str.toStdString();
            idx_t.score = 0;

            vim.tilesList.push_back(idx_t);

            DF<REAL, V3DLONG> df;
            df.n = count;
            df.pre = 0; // suppose 0 is the reference

            df.coeff = 0.0;
            df.fn_image = img_str.toStdString();

            tlut.tcList.push_back(df);

            count++;
        }

        // construct connected-graph from coarse-scale
        int NTILES = vim.tilesList.size();
        int NTILES_I = NTILES - 1;
        int NTILES_II = NTILES_I - 1;

        //
        V3DLONG offsets[3];
        for(int i=0; i<NTILES_I; i++) // record all the sz_image information
        {
            //loading target files
            V3DLONG *sz_target = 0;
            int datatype_target = 0;
            unsigned char* target1d = 0;

            if (loadImage(const_cast<char *>(vim.tilesList.at(i).fn_image.c_str()), target1d, sz_target, datatype_target)!=true)
            {
                fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",vim.tilesList.at(i).fn_image.c_str());
                return false;
            }
            V3DLONG tx=sz_target[0], ty=sz_target[1], tz=sz_target[2], tc=sz_target[3];
            cdim = tc; // init

            if(i==0)
            {
                imgdatatype = (ImagePixelType)datatype_target;
                if(datatype_target==4) imgdatatype = V3D_FLOAT32;
            }
            else
            {
                if(datatype_target != imgdatatype)
                {
                    printf("The program only support all tiled images with the same datatype.\n");
                    return false;
                }
            }

            (&vim.tilesList.at(i))->sz_image = new V3DLONG [4];

            (&vim.tilesList.at(i))->sz_image[0] = tx;
            (&vim.tilesList.at(i))->sz_image[1] = ty;
            (&vim.tilesList.at(i))->sz_image[2] = tz;
            (&vim.tilesList.at(i))->sz_image[3] = tc;

            // channel of target
            V3DLONG offsets_tar = channel1*tx*ty*tz;

            // try rest of tiles
            for(int j=i+1; j<NTILES; j++)
            {
                //loading subject files
                V3DLONG *sz_subject = 0;
                int datatype_subject = 0;
                unsigned char* subject1d = 0;

                if (loadImage(const_cast<char *>(vim.tilesList.at(j).fn_image.c_str()), subject1d, sz_subject, datatype_subject)!=true)
                {
                    fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n", vim.tilesList.at(j).fn_image.c_str());
                    return false;
                }

                V3DLONG sx=sz_subject[0], sy=sz_subject[1], sz=sz_subject[2], sc=sz_subject[3];

                // init
                if(i==NTILES_II)
                {
                    (&vim.tilesList.at(j))->sz_image = new V3DLONG [4];

                    (&vim.tilesList.at(j))->sz_image[0] = sx;
                    (&vim.tilesList.at(j))->sz_image[1] = sy;
                    (&vim.tilesList.at(j))->sz_image[2] = sz;
                    (&vim.tilesList.at(j))->sz_image[3] = sc;
                }

                // channel of subject
                V3DLONG offsets_sub = channel1*sx*sy*sz;

                // try
                PEAKS *pos = new PEAKS;

                if(imgdatatype == V3D_UINT8)
                {
                    success = stitching_bb_thickplanes<unsigned char, Y_IMG_UINT8>((unsigned char *)subject1d+offsets_sub, sz_subject, (unsigned char *)target1d+offsets_tar, sz_target, overlap_percent, m_similarity, scale, pos, 1);
                }
                else if(imgdatatype == V3D_UINT16)
                {
                    success = stitching_bb_thickplanes<unsigned short, Y_IMG_UINT16>((unsigned short *)(subject1d)+offsets_sub, sz_subject, (unsigned short *)(target1d)+offsets_tar, sz_target, overlap_percent, m_similarity, scale, pos, 1);
                }
                else if(imgdatatype == V3D_FLOAT32)
                {
                    success = stitching_bb_thickplanes<REAL, Y_IMG_REAL>((REAL *)(subject1d)+offsets_sub, sz_subject, (REAL *)(target1d)+offsets_tar, sz_target, overlap_percent, m_similarity, scale, pos, 1);
                }
                else
                {
                    printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                    return false;
                }
                if(success!=true) return false;

                //record n by n-1
                offsets[0] = pos->x - sx +1;
                offsets[1] = pos->y - sy +1;
                offsets[2] = pos->z - sz +1;

                indexed_t<V3DLONG, REAL> t(offsets);  //

                t.score = pos->value;
                t.n = vim.tilesList.at(i).n;

                (&vim.tilesList.at(j))->record.push_back(t);

                cout << " subject "<< vim.tilesList.at(i).fn_image << " over " << " target " <<vim.tilesList.at(j).fn_image << endl;

                //de-alloc
                if(subject1d) {delete []subject1d; subject1d=0;}
                if(sz_subject) {delete []sz_subject; sz_subject=0;}

            }
            //de-alloc
            if(target1d) {delete []target1d; target1d=0;}
            if(sz_target) {delete []sz_target; sz_target=0;}

        }

        // find mst of whole tiled images
        for(int i=0; i<NTILES; i++)
        {
            vim.tilesList.at(i).visited = false;
        }
        if(mstPrim(vim.tilesList)) // run Prim's algorithm
        {
            cout<<"Fail to call MST approach!"<<endl;
            return false;
        }

        // adjusting offset reference to ref. image
        // compute accumulate offsets from path list
        int ref_image = vim.tilesList.at(0).n;
        (&vim.tilesList.at(0))->offsets[0] = 0;
        (&vim.tilesList.at(0))->offsets[1] = 0;
        (&vim.tilesList.at(0))->offsets[2] = 0;

        for(int i=0; i<NTILES; i++)
        {
            vim.tilesList.at(i).visited = false;
        }

        for(int i_path=1; i_path<NTILES; i_path++)
        {
            // record final shifts
            vim.tilesList.at(i_path).record.at(0).offsets[0] = vim.tilesList.at(i_path).offsets[0];
            vim.tilesList.at(i_path).record.at(0).offsets[1] = vim.tilesList.at(i_path).offsets[1];
            vim.tilesList.at(i_path).record.at(0).offsets[2] = vim.tilesList.at(i_path).offsets[2];
        }

        for(int i_path=1; i_path<NTILES; i_path++)
        {
            // ref
            (&vim.tilesList.at(i_path))->ref_n = ref_image;

            V3DLONG current = vim.tilesList.at(i_path).n;
            V3DLONG previous = vim.tilesList.at(i_path).predecessor;

            //
            while(previous!=-1)
            {
                if(vim.tilesList.at(current).visited)
                {
                    break;
                }

                (&vim.tilesList.at(i_path))->offsets[0] += vim.tilesList.at(previous).offsets[0];
                (&vim.tilesList.at(i_path))->offsets[1] += vim.tilesList.at(previous).offsets[1];
                (&vim.tilesList.at(i_path))->offsets[2] += vim.tilesList.at(previous).offsets[2];


                //
                current = previous;
                previous = vim.tilesList.at(current).predecessor;
            }

            (&vim.tilesList.at(i_path))->visited = true;
        }

        // construct lookup table
        vim.y_clut(vim.tilesList.size());

        // joint registration from pair-wise images with overlap volumes
        if(!(scale[0]==1 && scale[1]==1 && scale[2]==1))
        {
            DFList dfVector; // D

            for(int j=0; j<NTILES_I; j++) // traverse all tiled images except the last one
            {
                rPEAKS pos;

                //loading target files
                V3DLONG *sz_target = 0;
                int datatype_target = 0;
                unsigned char* target1d = 0;

                if (loadImage(const_cast<char *>(vim.tilesList.at(j).fn_image.c_str()), target1d, sz_target, datatype_target)!=true)
                {
                    fprintf (stderr, "Error happens in reading the target file [%s]. Exit. \n",vim.tilesList.at(j).fn_image.c_str());
                    return false;
                }
                V3DLONG tx=sz_target[0], ty=sz_target[1], tz=sz_target[2], tc=sz_target[3];

                (&tlut.tcList.at(j))->sz_image[0] = tx;
                (&tlut.tcList.at(j))->sz_image[1] = ty;
                (&tlut.tcList.at(j))->sz_image[2] = tz;

                for(int i=j+1; i<NTILES; i++)  // traverse all tiled images except the reference
                {
                    //loading subject files
                    V3DLONG *sz_subject = 0;
                    int datatype_subject = 0;
                    unsigned char* subject1d = 0;

                    if (loadImage(const_cast<char *>(vim.tilesList.at(i).fn_image.c_str()), subject1d, sz_subject, datatype_subject)!=true)
                    {
                        fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n", vim.tilesList.at(i).fn_image.c_str());
                        return false;
                    }
                    V3DLONG sx=sz_subject[0], sy=sz_subject[1], sz=sz_subject[2], sc=sz_subject[3];

                    if(j==NTILES_II && i==NTILES_I)
                    {
                        (&tlut.tcList.at(i))->sz_image[0] = sx;
                        (&tlut.tcList.at(i))->sz_image[1] = sy;
                        (&tlut.tcList.at(i))->sz_image[2] = sz;
                    }

                    //
                    V3DLONG jx_s, jy_s, jz_s, jx_e, jy_e, jz_e;
                    V3DLONG ix_s, iy_s, iz_s, ix_e, iy_e, iz_e;

                    jx_s = vim.lut[j].start_pos[0] + vim.min_vim[0];
                    jy_s = vim.lut[j].start_pos[1] + vim.min_vim[1];
                    jz_s = vim.lut[j].start_pos[2] + vim.min_vim[2];

                    jx_e = vim.lut[j].end_pos[0] + vim.min_vim[0];
                    jy_e = vim.lut[j].end_pos[1] + vim.min_vim[1];
                    jz_e = vim.lut[j].end_pos[2] + vim.min_vim[2];

                    ix_s = vim.lut[i].start_pos[0] + vim.min_vim[0];
                    iy_s = vim.lut[i].start_pos[1] + vim.min_vim[1];
                    iz_s = vim.lut[i].start_pos[2] + vim.min_vim[2];

                    ix_e = vim.lut[i].end_pos[0] + vim.min_vim[0];
                    iy_e = vim.lut[i].end_pos[1] + vim.min_vim[1];
                    iz_e = vim.lut[i].end_pos[2] + vim.min_vim[2];

                    // with overlap
                    if( ((jx_s<=ix_e && jx_s>=ix_s) || (jx_e<=ix_e && jx_e>=ix_s))
                            && ((jy_s<=iy_e && jy_s>=iy_s) || (jy_e<=iy_e && jy_e>=iy_s))
                            && ((jz_s<=iz_e && jz_s>=iz_s) || (jz_e<=iz_e && jz_e>=iz_s)) )
                    {

                        //
                        V3DLONG offsets_tar = channel1*tx*ty*tz;
                        V3DLONG offsets_sub = channel1*sx*sy*sz;

                        //
                        if(imgdatatype == V3D_UINT8)
                        {
                            success = pistitching<unsigned char, Y_IMG_UINT8>((unsigned char *)subject1d+offsets_sub, sz_subject, (unsigned char *)target1d+offsets_tar, sz_target, overlap_percent, scale, pos);
                        }
                        else if(imgdatatype == V3D_UINT16)
                        {
                            success = pistitching<unsigned short, Y_IMG_UINT16>((unsigned short *)(subject1d)+offsets_sub, sz_subject, (unsigned short *)(target1d)+offsets_tar, sz_target, overlap_percent, scale, pos);
                        }
                        else if(imgdatatype == V3D_FLOAT32)
                        {
                            success = pistitching<REAL, Y_IMG_REAL>((REAL *)(subject1d)+offsets_sub, sz_subject, (REAL *)(target1d)+offsets_tar, sz_target, overlap_percent, scale, pos);
                        }
                        else
                        {
                            printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                            return -1;
                        }
                        if(success!=true) return false;

                        DF<REAL, V3DLONG> df;
                        df.n = i;
                        df.pre = j; // suppose 0 is the reference

                        df.coeff = pos.value;

                        df.offsets[0] = pos.x - REAL(sx-1);
                        df.offsets[1] = pos.y - REAL(sy-1);
                        df.offsets[2] = pos.z - REAL(sz-1);

                        dfVector.push_back(df);
                    }
                    else
                    {
                        continue;
                    }

                    // delloc
                    y_del<V3DLONG>(sz_subject);
                    y_del<unsigned char>(subject1d);

                }

                // delloc
                y_del<V3DLONG>(sz_target);
                y_del<unsigned char>(target1d);

            }

            //
            V3DLONG rows = dfVector.size();
            V3DLONG columns = tlut.tcList.size()-1;

            Y_MAT<double, V3DLONG> A(rows, columns, 0.0); // A
            Y_MAT<double, V3DLONG> D(rows, 1, 0.0);

            for(V3DLONG i=0; i<rows; i++)
            {
                V3DLONG ii = dfVector.at(i).n;
                V3DLONG jj = dfVector.at(i).pre;

                if(jj>=1) A.v[jj-1][i] = -1.0;

                A.v[ii-1][i] = 1.0;
            }

            // inv(A'A)A'
            Y_MAT<double, V3DLONG> AT;
            AT.clone(A, false);
            AT.transpose();

            Y_MAT<double, V3DLONG> ATA;
            ATA.clone(AT, false);
            ATA.prod(A);

            Y_MAT<double, V3DLONG> invATA;
            invATA.clone(ATA, false);
            invATA.pseudoinverse();

            invATA.prod(AT);

            for(V3DLONG dim=0; dim<3; dim++)
            {
                for(V3DLONG i=0; i<rows; i++)
                {
                    D.v[0][i] = dfVector.at(i).offsets[dim];
                }
                Y_MAT<double, V3DLONG> M;
                M.clone(invATA, false);

                M.prod(D);

                for(V3DLONG i=1; i<tlut.tcList.size(); i++)
                {
                    (&tlut.tcList.at(i))->offsets[dim] = M.v[0][i-1];
                }
            }
        }

        // subpixel shifts and linear blending
        tlut.clut();
        tlut.setDimC(cdim);

        QString tmp_filename = QFileInfo(m_InputFileName).path() + "/" + "stitched_image.tc"; //.tc tile configuration
        tlut.y_save(tmp_filename.toStdString());

        V3DLONG pagesz_vim = tlut.vx*tlut.vy*tlut.vz*tlut.vc;

        if(imgdatatype == V3D_UINT8)
        {
            // init
            unsigned char *pVImg = NULL;
            y_new<unsigned char, V3DLONG>(pVImg, pagesz_vim);
            if(pVImg)
            {
                memset(pVImg, 0, sizeof(unsigned char)*pagesz_vim);

                unsigned char intensityrange = 255;
                bool success = ifusing<unsigned char, Y_TLUT<REAL, V3DLONG, DF<REAL, V3DLONG> > >((unsigned char *)pVImg, tlut, intensityrange);
                if(!success)
                {
                    printf("Fail to call function ifusing! \n");
                    return false;
                }
            }

            // output
            if(img_show)
            {
                QString stitchFileName;
                if(!outfile)
                    stitchFileName = m_InputFolder + "/StitchedImage.v3draw";
                else
                    stitchFileName = QString(outfile);

                V3DLONG savedsz[4];
                savedsz[0] = tlut.vx;
                savedsz[1] = tlut.vy;
                savedsz[2] = tlut.vz;
                savedsz[3] = tlut.vc;

                //save
                if (saveImage(stitchFileName.toStdString().c_str(), (const unsigned char *)pVImg, savedsz, 1)!=true)
                {
                    fprintf(stderr, "Error happens in file writing. Exit. \n");
                    return false;
                }

                //de-alloc
                y_del<unsigned char>(pVImg);
            }
            else
            {
                V3DPluginArgItem arg;

                arg.type = "data"; arg.p = (void *)(pVImg); output << arg;

                V3DLONG metaImg[5]; // xyzc datatype
                metaImg[0] = tlut.vx;
                metaImg[1] = tlut.vy;
                metaImg[2] = tlut.vz;
                metaImg[3] = tlut.vc;
                metaImg[4] = imgdatatype;

                arg.type = "metaImage"; arg.p = (void *)(metaImg); output << arg;
            }
        }
        else if(imgdatatype == V3D_UINT16)
        {
            // init
            unsigned short *pVImg = NULL;
            y_new<unsigned short, V3DLONG>(pVImg, pagesz_vim);
            if(pVImg)
            {
                memset(pVImg, 0, sizeof(unsigned short)*pagesz_vim);

                unsigned short intensityrange = 4095;
                bool success = ifusing<unsigned short, Y_TLUT<REAL, V3DLONG, DF<REAL, V3DLONG> > >((unsigned short *)pVImg, tlut, intensityrange);
                if(!success)
                {
                    printf("Fail to call function ifusing! \n");
                    return false;
                }
            }

            // output
            if(img_show)
            {
                QString stitchFileName;
                if(!outfile)
                    stitchFileName = m_InputFolder + "/StitchedImage.v3draw";
                else
                    stitchFileName = QString(outfile);

                V3DLONG savedsz[4];
                savedsz[0] = tlut.vx;
                savedsz[1] = tlut.vy;
                savedsz[2] = tlut.vz;
                savedsz[3] = tlut.vc;

                //save
                if (saveImage(stitchFileName.toStdString().c_str(), (const unsigned char *)pVImg, savedsz, 2)!=true)
                {
                    fprintf(stderr, "Error happens in file writing. Exit. \n");
                    return false;
                }

                //de-alloc
                y_del<unsigned short>(pVImg);
            }
            else
            {
                V3DPluginArgItem arg;

                arg.type = "data"; arg.p = (void *)(pVImg); output << arg;

                V3DLONG metaImg[5]; // xyzc datatype
                metaImg[0] = tlut.vx;
                metaImg[1] = tlut.vy;
                metaImg[2] = tlut.vz;
                metaImg[3] = tlut.vc;
                metaImg[4] = imgdatatype;

                arg.type = "metaImage"; arg.p = (void *)(metaImg); output << arg;
            }

        }
        else if(imgdatatype == V3D_FLOAT32)
        {
            // current not supported
        }
        else
        {
            printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
            return false;
        }

        qDebug()<<"time cost for whole joint stitching ..."<<clock()-start_t;

        return true;

    }
    else if (func_name == tr("istitch-warp"))
    {
        // fluid-type deamon registration (intensity-based)

        //
        return true;
    }
    else if (func_name == tr("istitch-grouping"))
    {
        // to group tiles so that each group only contains tiles that could be stitched
        if(input.size()<1) return false; // no inputs

        vector<char*> * infilelist = (vector<char*> *)(input.at(0).p);
        vector<char*> * paralist;
        vector<char*> * outfilelist;
        if(infilelist->empty())
        {
            //print Help info
            printf("\nUsage: v3d -x imageStitch.dylib -f v3dstitch -i <input_image_folder> -o <output_image_file> -p \"[#c <channalNo_reference> #x <downsample_factor_x> #y <downsample_factor_y> #z <downsample_factor_z> #l <overlap_ratio>] #sb [saving_tile_boundary 0/1] #si <saving_stitching_result 0/1>\"\n");

            return true;
        }

        char * infile = infilelist->at(0); // input_image_folder
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_image_file

        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p); outfile = outfilelist->at(0);}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        // init
        QString m_InputFolder(infile);

        int channel1 = 0;

        REAL overlap_percent = 0.01;

        bool axes_show = false; // show tile boundary
        bool img_show = true; // save stitching file

        REAL *scale = new REAL [6];

        scale[0] = 0.2;
        scale[1] = 0.2;
        scale[2] = 0.2;
        scale[3] = 1;
        scale[4] = 1;
        scale[5] = 1;

        bool m_similarity = false;
        bool success = false;

        ImagePixelType imgdatatype;
        V3DLONG cdim;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[200];
            char * myparas = new char[len];
            strcpy(myparas, paras);
            for(int i = 0; i < len; i++)
            {
                if(i==0 && myparas[i] != ' ' && myparas[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((myparas[i-1] == ' ' || myparas[i-1] == '\t') && (myparas[i] != ' ' && myparas[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }
            char ** argv = new char* [argc];
            for(int i = 0; i < argc; i++)
            {
                argv[i] = myparas + posb[i];
            }
            for(int i = 0; i < len; i++)
            {
                if(myparas[i]==' ' || myparas[i]=='\t')
                    myparas[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "c"))
                            {
                                channel1 = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
                                i++;
                            }
                            else if (!strcmp(key, "x"))
                            {
                                scale[0] = atof( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "y"))
                            {
                                scale[1] = atof( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "z"))
                            {
                                scale[2] = atof( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "l"))
                            {
                                overlap_percent = atof( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "sb"))
                            {
                                key++;

                                axes_show = atoi( argv[i+1] )?true:false;
                                i++;
                            }
                            else if (!strcmp(key, "si"))
                            {
                                key++;

                                img_show = atoi( argv[i+1] )?true:false;
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }

                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }

                }
            }

            // error check
            if(channel1<0 || scale[0]<0.0 || scale[1]<0.0 || scale[2]<0.0 || overlap_percent<0.0 || overlap_percent>1.0)
            {
                cout<<"illegal input parameters"<<endl;
                return false;
            }
        }

        // group stitch
        int start_t = clock();

        // load tiles and stitch
        QStringList imgList = importSeriesFileList_addnumbersort(m_InputFolder);
        QString m_InputFileName = imgList.at(0);

        Y_VIM<REAL, V3DLONG, indexed_t<V3DLONG, REAL>, LUT<V3DLONG> > vim; // pixel-precise offsets lookup table

        Y_TLUT<REAL, V3DLONG, DF<REAL, V3DLONG> > tlut;// global tile configurations T

        V3DLONG count=0;
        foreach (QString img_str, imgList)
        {
            V3DLONG offset[3];
            offset[0]=0; offset[1]=0; offset[2]=0;

            indexed_t<V3DLONG, REAL> idx_t(offset);

            idx_t.n = count;
            idx_t.ref_n = 0; // init with default values
            idx_t.fn_image = img_str.toStdString();
            idx_t.score = 0;

            vim.tilesList.push_back(idx_t);

            DF<REAL, V3DLONG> df;
            df.n = count;
            df.pre = 0; // suppose 0 is the reference

            df.coeff = 0.0;
            df.fn_image = img_str.toStdString();

            tlut.tcList.push_back(df);

            count++;
        }

        // construct connected-graph from coarse-scale
        int NTILES = vim.tilesList.size();
        int NTILES_I = NTILES - 1;
        int NTILES_II = NTILES_I - 1;

        //
        V3DLONG offsets[3];
        for(int i=0; i<NTILES_I; i++) // record all the sz_image information
        {
            //loading target files
            V3DLONG *sz_target = 0;
            int datatype_target = 0;
            unsigned char* target1d = 0;

            if (loadImage(const_cast<char *>(vim.tilesList.at(i).fn_image.c_str()), target1d, sz_target, datatype_target)!=true)
            {
                fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",vim.tilesList.at(i).fn_image.c_str());
                return false;
            }
            V3DLONG tx=sz_target[0], ty=sz_target[1], tz=sz_target[2], tc=sz_target[3];
            cdim = tc; // init

            if(i==0)
            {
                imgdatatype = (ImagePixelType)datatype_target;
                if(datatype_target==4) imgdatatype = V3D_FLOAT32;
            }
            else
            {
                if(datatype_target != imgdatatype)
                {
                    printf("The program only support all tiled images with the same datatype.\n");
                    return false;
                }
            }

            (&vim.tilesList.at(i))->sz_image = new V3DLONG [4];

            (&vim.tilesList.at(i))->sz_image[0] = tx;
            (&vim.tilesList.at(i))->sz_image[1] = ty;
            (&vim.tilesList.at(i))->sz_image[2] = tz;
            (&vim.tilesList.at(i))->sz_image[3] = tc;

            // channel of target
            V3DLONG offsets_tar = channel1*tx*ty*tz;

            // try rest of tiles
            for(int j=i+1; j<NTILES; j++)
            {
                //loading subject files
                V3DLONG *sz_subject = 0;
                int datatype_subject = 0;
                unsigned char* subject1d = 0;

                if (loadImage(const_cast<char *>(vim.tilesList.at(j).fn_image.c_str()), subject1d, sz_subject, datatype_subject)!=true)
                {
                    fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n", vim.tilesList.at(j).fn_image.c_str());
                    return false;
                }

                V3DLONG sx=sz_subject[0], sy=sz_subject[1], sz=sz_subject[2], sc=sz_subject[3];

                // init
                if(i==NTILES_II)
                {
                    (&vim.tilesList.at(j))->sz_image = new V3DLONG [4];

                    (&vim.tilesList.at(j))->sz_image[0] = sx;
                    (&vim.tilesList.at(j))->sz_image[1] = sy;
                    (&vim.tilesList.at(j))->sz_image[2] = sz;
                    (&vim.tilesList.at(j))->sz_image[3] = sc;
                }

                // channel of subject
                V3DLONG offsets_sub = channel1*sx*sy*sz;

                // try
                PEAKS *pos = new PEAKS;

                if(imgdatatype == V3D_UINT8)
                {
                    success = stitching_bb_thickplanes<unsigned char, Y_IMG_UINT8>((unsigned char *)subject1d+offsets_sub, sz_subject, (unsigned char *)target1d+offsets_tar, sz_target, overlap_percent, m_similarity, scale, pos, 1);
                }
                else if(imgdatatype == V3D_UINT16)
                {
                    success = stitching_bb_thickplanes<unsigned short, Y_IMG_UINT16>((unsigned short *)(subject1d)+offsets_sub, sz_subject, (unsigned short *)(target1d)+offsets_tar, sz_target, overlap_percent, m_similarity, scale, pos, 1);
                }
                else if(imgdatatype == V3D_FLOAT32)
                {
                    success = stitching_bb_thickplanes<REAL, Y_IMG_REAL>((REAL *)(subject1d)+offsets_sub, sz_subject, (REAL *)(target1d)+offsets_tar, sz_target, overlap_percent, m_similarity, scale, pos, 1);
                }
                else
                {
                    printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                    return false;
                }
                if(success!=true) return false;

                //record n by n-1
                offsets[0] = pos->x - sx +1;
                offsets[1] = pos->y - sy +1;
                offsets[2] = pos->z - sz +1;

                indexed_t<V3DLONG, REAL> t(offsets);  //

                t.score = pos->value;
                t.n = vim.tilesList.at(i).n;

                (&vim.tilesList.at(j))->record.push_back(t);

                cout << " subject "<< vim.tilesList.at(i).fn_image << " over " << " target " <<vim.tilesList.at(j).fn_image << endl;

                //de-alloc
                if(subject1d) {delete []subject1d; subject1d=0;}
                if(sz_subject) {delete []sz_subject; sz_subject=0;}

            }
            //de-alloc
            if(target1d) {delete []target1d; target1d=0;}
            if(sz_target) {delete []sz_target; sz_target=0;}

        }

        // test
//        for(int i=0; i<NTILES_I; i++)
//        {
//            //
//            for(int j=i+1; j<NTILES; j++)
//            {
//                qDebug()<<"current "<<j<<"previous "<<i<<"edge "<<vim.tilesList.at(j).record.at(i).score;
//            }
//        }


        // find mst of whole tiled images
        for(int i=0; i<NTILES; i++)
        {
            vim.tilesList.at(i).visited = false;
        }
        if(mstPrim(vim.tilesList)) // run Prim's algorithm
        {
            cout<<"Fail to call MST approach!"<<endl;
            return false;
        }

        // undirected graph
        bool **udgraph = NULL;
        int *visited=NULL;
        try
        {
            udgraph = new bool *[NTILES];
            for(V3DLONG i=0; i<NTILES; i++)
                udgraph[i] = new bool [NTILES];

            for(V3DLONG i=0; i<NTILES; i++)
            {
                for(V3DLONG j=0; j<NTILES; j++)
                {
                    udgraph[i][j] = false;
                }
            }

            visited = new int [NTILES];
            memset(visited, 0, sizeof(int)*NTILES);
        }
        catch(...)
        {
            cout<<"Fail to allocate memory for undirected graph!"<<endl;
            return false;
        }

        // grouping tiles
        REAL stitch_threshold = 0.885;
        for(int i=1; i<NTILES; i++)
        {
            //
            V3DLONG pre = vim.tilesList.at(i).predecessor;
            V3DLONG cur = i;

            if(pre>cur)
            {
                cur = pre;
                pre = i;
            }

            if(vim.tilesList.at(cur).record.at(pre).score>stitch_threshold)
            {
                udgraph[pre][cur] = true;
            }

        }

        // output
        QString tg_filename = QFileInfo(m_InputFileName).path() + "/" + "igroups.txt"; // tiled image groups
        FILE *pTGFile=0;
        pTGFile = fopen(tg_filename.toStdString().c_str(),"wt");

        V3DLONG count_group = 1;
        for(V3DLONG i=0; i<NTILES_I; i++)
        {
            if(visited[i])
            {
                continue;
            }

            fprintf(pTGFile, "# tiled image group %ld \n", count_group);

            visited[i] = count_group;
            fprintf(pTGFile, "%s \n", vim.tilesList.at(i).fn_image.c_str());

            for(int j=i; j<NTILES_I; j++)
            {
                if(visited[j] && visited[j]!=count_group)
                {
                    continue;
                }

                for(int k=0; k<j; k++)
                {
                    if(udgraph[k][j] && (visited[j]==count_group || visited[k]==count_group))
                    {
                        if(visited[j]!=count_group)
                            fprintf(pTGFile, "%s \n", vim.tilesList.at(j).fn_image.c_str());
                        if(visited[k]!=count_group)
                            fprintf(pTGFile, "%s \n", vim.tilesList.at(k).fn_image.c_str());

                        visited[j] = count_group;
                        visited[k] = count_group;
                    }
                }

                for(int k=j+1; k<NTILES; k++)
                {
                    if(udgraph[j][k] && (visited[j]==count_group || visited[k]==count_group))
                    {
                        if(visited[j]!=count_group)
                            fprintf(pTGFile, "%s \n", vim.tilesList.at(j).fn_image.c_str());
                        if(visited[k]!=count_group)
                            fprintf(pTGFile, "%s \n", vim.tilesList.at(k).fn_image.c_str());

                        visited[j] = count_group;
                        visited[k] = count_group;
                    }
                }
            }
            fprintf(pTGFile, "\n");
            count_group++;


        }
        fclose(pTGFile);

        // de-alloc
        y_del<int>(visited);
        for(V3DLONG i=0; i<NTILES; i++)
            delete[] udgraph[i];
        delete[] udgraph;

        return true;

    }
    else
    {
        printf("\nWrong function specified.\n");
        return false;
    }
}

#endif

#endif

