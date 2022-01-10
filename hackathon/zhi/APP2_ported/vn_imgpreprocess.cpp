//by Hanchuan Peng
//2012-Dec

#include "vn_imgpreprocess.h"


bool downsampling_img_xyz(unsigned char *inimg, V3DLONG in_sz[4], double dfactor_xy, double dfactor_z, unsigned char *&outimg, V3DLONG out_sz[4])
{
    if (!inimg || !in_sz || !out_sz ||
        in_sz[0]<=0 || in_sz[1]<=0 || in_sz[2]<=0 || in_sz[3]<=0 ||
        dfactor_xy <= 1 || dfactor_z < 1) //note allow dfactor_z=1
    {
        v3d_msg("The input parameters of downsampling_xyz() is not valid.");
        return false;
    }
    
    V3DLONG ntotal = 1;
    for (int i=0; i<4; i++) 
    {
        out_sz[i] = in_sz[i]; 
        ntotal *= out_sz[i];
    }

    if (outimg) {delete []outimg; outimg=0;}
    try {
        outimg = new unsigned char [ntotal];
    }
    catch(...)
    {
        v3d_msg("Fail to allocate memory in downsampling_xyz()."); 
        return false;
    }
    memcpy(outimg, inimg, ntotal);
    
    int interp_method = 1;
    return resample3dimg_interp( outimg, out_sz, dfactor_xy, dfactor_xy, dfactor_z, interp_method);    
}

void downsampling_marker_xyz(LandmarkList & listLandmarks, double dfactor_xy, double dfactor_z)
{
	LocationSimple tmp_pt(-1,-1,-1);
	for (V3DLONG i=0;i<listLandmarks.count();i++)
	{
		tmp_pt = listLandmarks.at(i);
		tmp_pt.x /= dfactor_xy;
		tmp_pt.y /= dfactor_xy;
		tmp_pt.z /= dfactor_z;
		listLandmarks.replace(i, tmp_pt);
	}
}


bool scale_img_and_convert28bit(Image4DSimple * p4dImage, int lb, int ub) //lb, ub: lower bound, upper bound
{
    if (!p4dImage || !(p4dImage->valid()))
    {
        v3d_msg("Your data is invalid in scale_img_and_convert28bit().\n");
        return false;
    }
    
	V3DLONG k;
	for (k=0;k<p4dImage->getCDim();k++)
		scaleintensity(p4dImage, k, -9999, -9999, double(lb), double(ub)); //when the second and third parameters are set as -9999, then search the min and max automatically
    
	V3DLONG tsz0 = p4dImage->getXDim(), tsz1 = p4dImage->getYDim(), tsz2 = p4dImage->getZDim(), tsz3 = p4dImage->getCDim();
	V3DLONG tunits =tsz0*tsz1*tsz2*tsz3;
	V3DLONG tbytes = tunits;
    
	unsigned char * outvol1d = 0;
	try
	{
		outvol1d = new unsigned char [tbytes];
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in scale_img_and_convert28bit().\n");
		return false;
	}
    
	//now cpy data
	V3DLONG i;
	if ( p4dImage->getDatatype() ==V3D_UINT8)
	{
		unsigned char * cur_data1d = (unsigned char *)p4dImage->getRawData();
		for (i=0;i<tunits;i++)
			outvol1d[i] = cur_data1d[i];
	}
	else if ( p4dImage->getDatatype() ==V3D_UINT16)
	{
		unsigned short int * cur_data1d = (unsigned short int *)p4dImage->getRawData();
		for (i=0;i<tunits;i++)
			outvol1d[i] = (unsigned char)(cur_data1d[i]);
	}
	else if ( p4dImage->getDatatype() ==V3D_FLOAT32)
	{
		float * cur_data1d = (float *)p4dImage->getRawData();
		for (i=0;i<tunits;i++)
			outvol1d[i] = (unsigned char)(cur_data1d[i]);
	}
	else
	{
		v3d_msg("should not get here in proj_general_scaleandconvert28bit(). Check your code/data.");
		if (outvol1d) {delete []outvol1d;outvol1d=0;}
		return false;
	}
    
	p4dImage->setData(outvol1d, tsz0, tsz1, tsz2, tsz3, V3D_UINT8);
    
	return true;
}


bool scaleintensity(Image4DSimple * p4dImage, int channo, double lower_th, double higher_th, double target_min, double target_max) //map the value linear from [lower_th, higher_th] to [target_min, target_max].
{
    if (!p4dImage || !(p4dImage->valid()) || channo>=p4dImage->getCDim() || channo < 0)
	{
		v3d_msg("Invalid chan parameter in scaleintensity();\n");
		return false;
	}
    
	V3DLONG i,j,k,c;    
	V3DLONG channelPageSize = p4dImage->getTotalUnitNumberPerChannel();

    V3DLONG pos_min, pos_max;
    if (lower_th==-9999 && higher_th==-9999)
    {
        switch ( p4dImage->getDatatype() )
        {
            case V3D_UINT8:
            {
                unsigned char * ptmp = p4dImage->getRawDataAtChannel(channo);
                unsigned char minv, maxv;
                minMaxInVector(ptmp, channelPageSize, pos_min, minv, pos_max, maxv);
                lower_th = minv; higher_th = maxv;
            }   
                break;
                
            case V3D_UINT16:
            {
                unsigned short int * ptmp = (unsigned short int *)(p4dImage->getRawDataAtChannel(channo));
                unsigned short int minv, maxv;
                minMaxInVector(ptmp, channelPageSize, pos_min, minv, pos_max, maxv);
                lower_th = minv; higher_th = maxv;
            }            
                break;
                
            case V3D_FLOAT32:
            {
                float * ptmp = (float *)(p4dImage->getRawDataAtChannel(channo));
                float minv, maxv;
                minMaxInVector(ptmp, channelPageSize, pos_min, minv, pos_max, maxv);
                lower_th = minv; higher_th = maxv;
            }
                break;
                
            default:
                v3d_msg("invalid datatype in scaleintensity();\n");
                return false;
        }
    }
    
	double t;
	if (lower_th>higher_th) {t=lower_th; lower_th=higher_th; higher_th=t;}
	if (target_min>target_max) {t=target_min; target_min=target_max; target_max=t;}
    
	double rate = (higher_th==lower_th) ? 1 : (target_max-target_min)/(higher_th-lower_th); //if the two th vals equal, then later-on t-lower_th will be 0 anyway
    
	switch ( p4dImage->getDatatype() )
	{
		case V3D_UINT8:
            {
                unsigned char * ptmp = p4dImage->getRawDataAtChannel(channo);
                for (k=0;k<channelPageSize;k++)
                {
                    t = ptmp[k];
                    if (t>higher_th) t=higher_th;
                    else if (t<lower_th) t=lower_th;
                    ptmp[k] = (unsigned char)((t - lower_th)*rate + target_min);
                }
            }   
			break;
            
		case V3D_UINT16:
            {
                unsigned short int * ptmp = (unsigned short int *)(p4dImage->getRawDataAtChannel(channo));
                for (k=0;k<channelPageSize;k++)
                {
                    t = ptmp[k];
                    if (t>higher_th) t=higher_th;
                    else if (t<lower_th) t=lower_th;
                    ptmp[k] = (unsigned short int)((t - lower_th)*rate + target_min);
                }
            }            
			break;
            
		case V3D_FLOAT32:
            {
                float * ptmp = (float *)(p4dImage->getRawDataAtChannel(channo));
                for (k=0;k<channelPageSize;k++)
                {
                    t = ptmp[k];
                    if (t>higher_th) t=higher_th;
                    else if (t<lower_th) t=lower_th;
                    ptmp[k] = (float)((t - lower_th)*rate + target_min);
                }
            }
			break;

		default:
			v3d_msg("invalid datatype in scaleintensity();\n");
			return false;
	}
    
	return true;
}

bool subvolumecopy(Image4DSimple * dstImg,
                   Image4DSimple * srcImg,
                   V3DLONG x0, V3DLONG szx,
                   V3DLONG y0, V3DLONG szy,
                   V3DLONG z0, V3DLONG szz,
                   V3DLONG c0, V3DLONG szc)
{
    if (!dstImg || !dstImg->valid() ||
        !srcImg || !srcImg->valid() )
    {
        v3d_msg("Invalid parameters for the function subvolumecopy(). 111");
        return false;
    }
    if (x0<0 || szx<1 || szx> dstImg->getXDim() || x0+szx > srcImg->getXDim() )
    {
        v3d_msg("Invalid parameters for the function subvolumecopy() 21.");
        return false;
    }
    if (y0<0 || szy<1 || szy> dstImg->getYDim() || y0+szy > srcImg->getYDim() )
    {
        v3d_msg("Invalid parameters for the function subvolumecopy() 22.");
        return false;
    }
    if (z0<0 || szz<1 || szz> dstImg->getZDim() || z0+szz > srcImg->getZDim() )
    {
        v3d_msg("Invalid parameters for the function subvolumecopy() 23.");
        return false;
    }
    if ( c0<0 || szc<1 || szc> dstImg->getCDim() || c0+szc > srcImg->getCDim() )
    {
        v3d_msg("Invalid parameters for the function subvolumecopy() 24.");
        return false;
    }
    if (dstImg->getDatatype() != srcImg->getDatatype() )
    {
        v3d_msg("Invalid parameters for the function subvolumecopy() 3.");
        return false;
    }
    
    unsigned char *dst1d = dstImg->getRawData();
    unsigned char *src1d = srcImg->getRawData();
    
    V3DLONG srcChannelLen = srcImg->getTotalUnitNumberPerChannel();
    V3DLONG srcPlaneLen = srcImg->getTotalUnitNumberPerPlane();
    V3DLONG srcLineLen = srcImg->getXDim();
    
    V3DLONG i,j,k,c;
    V3DLONG n=0;
    
    switch (dstImg->getDatatype()) 
    {
        case V3D_UINT8:
            {
                for (c=0;c<szc;c++)
                    for (k=0;k<szz;k++)
                        for (j=0;j<szy;j++)
                            for (i=0;i<szx;i++)
                            {
                                dst1d[n] = src1d[(c+c0)*srcChannelLen + (k+z0)*srcPlaneLen + (j+y0)*srcLineLen + (i+x0)];
                                n++;
                            }
            }
            break;
            
        case V3D_UINT16:
            {
                unsigned short int *dtmp = (unsigned short int *)dst1d;
                unsigned short int *stmp = (unsigned short int *)src1d;
                for (c=0;c<szc;c++)
                    for (k=0;k<szz;k++)
                        for (j=0;j<szy;j++)
                            for (i=0;i<szx;i++)
                            {
                                dtmp[n] = stmp[(c+c0)*srcChannelLen + (k+z0)*srcPlaneLen + (j+y0)*srcLineLen + (i+x0)];
                                n++;
                            }
            }
            break;
            
        case V3D_FLOAT32:
            {
                float *dtmp = (float *)dst1d;
                float *stmp = (float *)src1d;
                for (c=0;c<szc;c++)
                    for (k=0;k<szz;k++)
                        for (j=0;j<szy;j++)
                            for (i=0;i<szx;i++)
                            {
                                dtmp[n] = stmp[(c+c0)*srcChannelLen + (k+z0)*srcPlaneLen + (j+y0)*srcLineLen + (i+x0)];
                                n++;
                            }
            }
            break;
            
        default:
            v3d_msg("Invalid data type in subvolumecopy(). do nothing.");
            return false;
    }
         
    return true;
}


