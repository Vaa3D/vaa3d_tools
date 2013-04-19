#include "autoseed.h"

#include "img_definition.h"
#include "basic_landmark.h"

#include "stackutil.h"

typedef unsigned short int USHORTINT16;

#include "template_matching_seg.h"

QList <ImageMarker> autoMarkerFromImg(unsigned char ***img, V3DLONG sz[4], float zthickness);
bool compute_statistics_objects(Vol3DSimple<unsigned char> *grayimg, Vol3DSimple<unsigned short int> * maskimg, LocationSimple * & p_ano, V3DLONG & n_objects);
template <class T> bool downsample3dimg(T *** inimg, V3DLONG insz[3], double dfactor, T *** outimg, V3DLONG outsz[3]);

QList <ImageMarker> autoSeed(unsigned char ***img, V3DLONG sz[4], double weight_xy_z)
{
	QList <ImageMarker> seed = autoMarkerFromImg(img, sz, weight_xy_z);
	return seed;
}


QList <ImageMarker> autoMarkerFromImg(unsigned char ***img, V3DLONG sz[4], float zthickness)
{
	QList <ImageMarker> ql;

	if (!img || !sz || sz[0]<=0 || sz[1]<=0 || sz[2]<=0)
	{
		v3d_msg("Now only support UINT8 type of data. Your data is not this type, or your channel info is not correct. Do nothing.\n");
		return ql;
	}

	//prepare input and output data

    double dfactor = 4;
	V3DLONG xb=0,yb=0,zb=0,xe=sz[0]-1,ye=sz[1]-1,ze=sz[2]-1;
	V3DLONG vsz0=xe-xb+1, vsz1=ye-yb+1, vsz2=ze-zb+1;
	V3DLONG vsz0out=vsz0/dfactor, vsz1out=vsz1/dfactor, vsz2out=vsz2/dfactor;
    V3DLONG szout[4]; szout[0] = vsz0out; szout[1] = vsz1out; szout[2] = vsz2out; szout[3]=1; 

	Vol3DSimple <unsigned char> * tmp_inimg = 0;
	Vol3DSimple <USHORTINT16> * tmp_outimg = 0;

	try
	{
		tmp_inimg = new Vol3DSimple <unsigned char> (vsz0out, vsz1out, vsz2out);
		tmp_outimg = new Vol3DSimple <USHORTINT16> (vsz0out, vsz1out, vsz2out);
	}
	catch (...)
	{
		v3d_msg("Unable to allocate memory for processing. Do nothing.\n");
		if (tmp_inimg) {delete tmp_inimg; tmp_inimg=0;}
		if (tmp_outimg) {delete tmp_outimg; tmp_outimg=0;}
		return ql;
	}

	//copy data
    downsample3dimg(img, sz, dfactor, tmp_inimg->getData3dHandle(), szout);
    saveImage("1.raw", tmp_inimg->getData1dHandle(), szout, 1);
//    saveImage("1.raw", img[0][0], sz, 1);

	//define the template matching parameters
	para_template_matching_cellseg segpara;
	segpara.channo = 0;
	segpara.szx=3;
	segpara.szy=3;
	segpara.szz=3/zthickness; //dim
	segpara.stdx=1;
	segpara.stdy=1;
	segpara.stdz=1; //std
	segpara.t_pixval=100;
	segpara.t_rgnval=27;
	segpara.t_corrcoef=0.3; //thresholds
	segpara.merge_radius = 20;

	//now set the two thresholds adaptively
//	double mean_val=0, std_val=0;
//	data_array_mean_and_std(tmp_inimg->getData1dHandle(), tmp_inimg->getTotalElementNumber(), mean_val, std_val);
//	segpara.t_pixval=qMax(double(150.0), mean_val+3.0*std_val);
//	segpara.t_rgnval=qMax(double(20.0), mean_val+1.0*std_val);

	//do computation
	bool b_res = template_matching_seg(tmp_inimg, tmp_outimg, segpara);
	if (!b_res)
	{
		v3d_msg("Fail to do the cell segmentation().\n", 0);
	}
	else
	{
		USHORTINT16 * tmpImg_d1d = (USHORTINT16 *)(tmp_outimg->getData1dHandle());

		LocationSimple * p_ano = 0;
		V3DLONG n_objects = 0;
		if (!compute_statistics_objects(tmp_inimg, tmp_outimg, p_ano, n_objects))
		{
			v3d_msg("Some errors happen during the computation of image objects' statistics. The annotation is not generated.", 0);
			return ql;
		}

		double largestMass; V3DLONG iLargestMass = -1;
		for (V3DLONG i=1;i<n_objects;i++) //do not process 0 values, as it is background. Thus starts from 1
		{
			if (i==1)
			{
			   largestMass = p_ano[i].mass;
			   iLargestMass = 1;
			}
			else
			{
			    if (largestMass<p_ano[i].mass)
			    {
			        largestMass = p_ano[i].mass;
			        iLargestMass = i;
			    }
			}
		}
		
		if (iLargestMass>0) 
		{
			ImageMarker m;
			m.x = p_ano[iLargestMass].x * dfactor + xb;
			m.y = p_ano[iLargestMass].y * dfactor + yb;
			m.z = p_ano[iLargestMass].z * dfactor + zb;
			ql << m;
		}
			   
		
			//			fprintf(f_ano, "%ld,%ld,%s,%s,%d,%d,%d,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,,,\n",
			//					i,i,"","", int(p_ano[i].z+0.5), int(p_ano[i].x+0.5), int(p_ano[i].y+0.5),
			//					p_ano[i].pixmax, p_ano[i].ave, p_ano[i].sdev, p_ano[i].size, p_ano[i].mass);
		
		if (p_ano) {delete []p_ano; p_ano=0;}
	}

	//free variables
	if (tmp_inimg) {delete tmp_inimg; tmp_inimg=0;}
	if (tmp_outimg) {delete tmp_outimg; tmp_outimg=0;}

	return ql;
}

bool compute_statistics_objects(Vol3DSimple<unsigned char> *grayimg, Vol3DSimple<unsigned short int> * maskimg, LocationSimple * & p_ano, V3DLONG & n_objects)
{
	if (!grayimg || !grayimg->valid() || !maskimg || !maskimg->valid() || p_ano) //p_ano MUST be 0 as this function need to alocate memory for it
	{
		v3d_msg("The inputs of compute_statistics_objects() are invalid.\n");
		return false;
	}

	if (!isSameSize(grayimg, maskimg))
	{
		v3d_msg("The sizes of the grayimg and maskimg do not match in compute_statistics_objects().\n");
		return false;
	}

	//first find the largest index
	V3DLONG i,j,k;

	unsigned short int *p_maskimg_1d = maskimg->getData1dHandle();
	unsigned short int ***p_maskimg_3d = maskimg->getData3dHandle();
	unsigned char *p_grayimg_1d = grayimg->getData1dHandle();
	unsigned char ***p_grayimg_3d = grayimg->getData3dHandle();

	n_objects = 0;
	for (i=0;i<maskimg->getTotalElementNumber();i++)
		n_objects = (p_maskimg_1d[i]>n_objects)?p_maskimg_1d[i]:n_objects;

	n_objects += 1; //always allocate one more, as the object index starts from 1. This will provide some convenience for later indexing (i.e. no need to minus 1)
	if (n_objects==1)
	{
		v3d_msg("The maskimg is all 0s. Nothing to generate!.\n");
		return false;
	}

	//then allocate memory and collect statistics

	try
	{
		p_ano = new LocationSimple [n_objects];
	}
	catch(...)
	{
		v3d_msg("Fail to allocate memory in compute_statistics_objects().\n");
		return false;
	}

	for (k=0;k<maskimg->sz2();k++)
		for (j=0;j<maskimg->sz1();j++)
			for (i=0;i<maskimg->sz0();i++)
			{
				V3DLONG cur_ind = p_maskimg_3d[k][j][i];
				if (p_grayimg_3d[k][j][i]==0) continue; //do not process 0 values, as it is background

				double cur_pix = double(p_grayimg_3d[k][j][i]);


				p_ano[cur_ind].size += 1;
				p_ano[cur_ind].mass += cur_pix;
				p_ano[cur_ind].sdev += cur_pix*cur_pix; //use the incremental formula
				if (cur_pix > p_ano[cur_ind].pixmax) p_ano[cur_ind].pixmax =  cur_pix;

				p_ano[cur_ind].x += i*cur_pix;
				p_ano[cur_ind].y += j*cur_pix;
				p_ano[cur_ind].z += k*cur_pix;
			}

	for (k=0;k<n_objects;k++)
	{
		if (p_ano[k].size>0)
		{
			p_ano[k].ave = p_ano[k].mass / p_ano[k].size;
			p_ano[k].sdev = sqrt(p_ano[k].sdev/p_ano[k].size - p_ano[k].ave*p_ano[k].ave); //use the incremental formula

			p_ano[k].x /= p_ano[k].mass;
			p_ano[k].y /= p_ano[k].mass;
			p_ano[k].z /= p_ano[k].mass;
		}
	}
	return true;
}


template <class T> bool downsample3dimg(T *** inimg, V3DLONG insz[3], double dfactor, T *** outimg, V3DLONG outsz[3])
{
	if (!inimg || !insz || !outimg || !outsz)
	{
		fprintf(stderr, "The input to downsample3dimg() are invalid [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}
    
	if (int(dfactor)<1)
	{
		fprintf(stderr, "The downsampling factor must be >=1 [%s][%d] dfactor=%.2f.\n", __FILE__, __LINE__, dfactor);
		return false;
	}
    
	if (insz[0]<1 || insz[1]<1 || insz[2]<1 || outsz[0]<1 || outsz[1]<1 || outsz[2]<1)
	{
		fprintf(stderr, "Input images sizes are not valid [%s][%d][%5.3f].\n", __FILE__, __LINE__, dfactor);
		return false;
	}
    
	V3DLONG cur_sz0 = (V3DLONG)(floor(double(insz[0]) / double(dfactor)));
	V3DLONG cur_sz1 = (V3DLONG)(floor(double(insz[1]) / double(dfactor)));
	V3DLONG cur_sz2 = (V3DLONG)(floor(double(insz[2]) / double(dfactor)));
    
    if (outsz[0]<cur_sz0 || outsz[1]<cur_sz1 || outsz[2]<cur_sz2)
	{
		fprintf(stderr, "Output images sizes are not valid [%s][%d][%5.3f].\n", __FILE__, __LINE__, dfactor);
		return false;
	}
    
    for (V3DLONG k=0;k<cur_sz2;k++)
    {
        V3DLONG k2low=V3DLONG(floor(k*dfactor)), k2high=V3DLONG(floor((k+1)*dfactor-1));
        if (k2high>insz[2]-1) k2high = insz[2]-1;
        V3DLONG kw = k2high - k2low + 1;
        
        for (V3DLONG j=0;j<cur_sz1;j++)
        {
            V3DLONG j2low=V3DLONG(floor(j*dfactor)), j2high=V3DLONG(floor((j+1)*dfactor-1));
            if (j2high>insz[1]-1) j2high = insz[1]-1;
            V3DLONG jw = j2high - j2low + 1;
            
            for (V3DLONG i=0;i<cur_sz0;i++)
            {
                V3DLONG i2low=V3DLONG(floor(i*dfactor)), i2high=V3DLONG(floor((i+1)*dfactor-1));
                if (i2high>insz[0]-1) i2high = insz[0]-1;
                V3DLONG iw = i2high - i2low + 1;
                
                double cubevolume = double(kw) * jw * iw;
                
                double s=0.0;
                for (V3DLONG k1=k2low;k1<=k2high;k1++)
                {
                    for (V3DLONG j1=j2low;j1<=j2high;j1++)
                    {
                        for (V3DLONG i1=i2low;i1<=i2high;i1++)
                        {
                            s += inimg[k1][j1][i1];
                        }
                    }
                }
                
                outimg[k][j][i] = (T)(s/cubevolume);
            }
        }
    }
    
	return true;
}

