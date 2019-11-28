/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).
 * All rights reserved.
 */


/************
                                            ********* LICENSE NOTICE ************

This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it.

You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.

1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.

2. You agree to appropriately cite this work in your related studies and publications.

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/




/*******************************************************************************************
 *
 * basic_4dimage.h
 *
 * This function is a basic function interface of the V3D project.
 *
 * Copyright: Hanchuan Peng (Howard Hughes Medical Institute, Janelia Farm Research Campus).
 * The License Information and User Agreement should be seen at http://penglab.janelia.org/proj/v3d .
 *
 * Last edit: 2009-Aug-21
 * Last edit: 2010-May-19: replace long with V3DLONG
 * Last edit: 2010-May-30: add the value_at() function for Image4DProxy class
 * Last edit: 2010-Jun-26: add three new members rez_x, rez_y, and rez_z which indicate the pixel sizes and thus the anisotropy of the image
 *
 * Last edit: 2010-Aug-1: add a function to determine if the data buffers of two images are exactly the same (but not their contents!)
 * Last edit: 2010-Aug-3: add the put_at() function for Image4DProxy class
 * Last edit: 2010-Aug-10: virtual destructor for other virtual functions in Image4DSimple
 * Last edit: 2010-Aug-31: PHC. move private members to protected to fix the hackers' bug!.
 * Last edit: 2010-Oct-06. PHC. add the original_x,y,z fields
 * Last edit: 2010-Oct-7. PHC. add a customary void pointer for a unknown struct for parameters passing of plugins
 * Last edit: 2010-Dec-18. PHC. add a valid valid_zslicenum to indicate the reading status of the data
 *
 *******************************************************************************************
 */

#ifndef _BASIC_4DIMAGE_H_
#define _BASIC_4DIMAGE_H_

#include <vector>
#include "v3d_basicdatatype.h"
#include "v3d_message.h"
#include <stdio.h>

/*!
 * Volume image with dimensions X, Y, Z, and time.
 */
class Image4DSimple
{
protected:
	unsigned char * data1d;
	V3DLONG sz0;
	V3DLONG sz1;
	V3DLONG sz2;
	V3DLONG sz3;
	V3DLONG sz_time;
	TimePackType timepacktype;
	ImagePixelType datatype;
	char imgSrcFile[1024]; //the full file path
	int b_error;
	double rez_x, rez_y, rez_z; //the resolution of a image pixel along the 3 axes
	double origin_x, origin_y, origin_z; //the "true" orgin of an image, in term of the physical units (not pixels) using resolution information

	V3DLONG valid_zslicenum; //indicate how many zslices are usable. This can be used by a plugin program to stream read data
	V3DLONG prevalid_zslicenum; //indicate previous valid slices loaded before update GUI
	void * p_customStruct; //a convenient pointer to pass back and forth some useful parameter information for a plugin

	void setError( int v ) {b_error = v;}

private:

public:
	Image4DSimple()
	{
		data1d = 0;
		sz0 = sz1 = sz2 = sz3 = 0;
		sz_time = 0;
		datatype = V3D_UNKNOWN;
		timepacktype = TIME_PACK_NONE;
		imgSrcFile[0] = '\0';
		b_error = 0;
		rez_x = rez_y = rez_z = 1;

		origin_x = origin_y = origin_z = 0;
		p_customStruct = 0;

		valid_zslicenum = 0;
	}
	virtual ~Image4DSimple()
	{
		cleanExistData();
	}
	virtual void cleanExistData()
	{
		this->deleteRawDataAndSetPointerToNull();
		sz0 = sz1 = sz2 = sz3 = 0;
		sz_time = 0;
		datatype = V3D_UNKNOWN;
		timepacktype = TIME_PACK_NONE;
		imgSrcFile[0] = '\0';
		b_error = 0;
		rez_x = rez_y = rez_z = 1;
		valid_zslicenum = 0;
	}

	//main interface to the data
	unsigned char * getRawData() {return data1d;}
        const unsigned char * getRawData() const {return data1d;} // const version 25-Apr 2011 CMB
        V3DLONG getXDim() const {return sz0;}
        V3DLONG getYDim() const {return sz1;}
        V3DLONG getZDim() const {return sz2;}
        V3DLONG getCDim() const {return sz3;}
        V3DLONG getTDim() const {return sz_time;}
        V3DLONG getValidZSliceNum() const {return valid_zslicenum;}
        V3DLONG getPreValidZSliceNum() const {return prevalid_zslicenum;}
        int getError() const {return b_error;}
        ImagePixelType getDatatype() const {return datatype;}
        TimePackType getTimePackType() const {return timepacktype;}
        V3DLONG getTotalUnitNumber() const {return sz0*sz1*sz2*sz3;}
        V3DLONG getTotalUnitNumberPerPlane() const {return sz0*sz1;}
        V3DLONG getTotalUnitNumberPerChannel() const {return sz0*sz1*sz2;}
        V3DLONG getUnitBytes() const
	{
		switch (datatype)
		{
			case V3D_UINT8: return 1;
			case V3D_UINT16: return 2;
			case V3D_FLOAT32: return 4;
			default: return 1;
		}
	}
        V3DLONG getTotalBytes() const {return getUnitBytes()*sz0*sz1*sz2*sz3;}
	unsigned char * getRawDataAtChannel(V3DLONG cid)
	{
		V3DLONG myid = cid; if (myid<0) myid=0; else if (myid>=sz3) myid = sz3-1;
		return data1d + myid*getTotalUnitNumberPerChannel()*getUnitBytes();
	}
	int isSuccess() {if (sz0<=0 || sz1<=0 || sz2<=0 || sz3<=0) b_error=1; return !b_error;}
	virtual bool valid() {
		return (
				!data1d ||
				sz0<=0 ||
				sz1<=0 ||
				sz2<=0 ||
				sz3<=0 ||
				b_error ||
				(datatype!=V3D_UINT8 &&
				 datatype!=V3D_UINT16 &&
				 datatype!=V3D_FLOAT32)) ?  false : true;
	}
        double getRezX() const {return rez_x;}
        double getRezY() const {return rez_y;}
        double getRezZ() const {return rez_z;}
        double getOriginX() const {return origin_x;}
        double getOriginY() const {return origin_y;}
        double getOriginZ() const {return origin_z;}

	void setXDim(V3DLONG v) {sz0=v;}
	void setYDim(V3DLONG v) {sz1=v;}
	void setZDim(V3DLONG v) {sz2=v;}
	void setCDim(V3DLONG v) {sz3=v;}
	void setTDim(V3DLONG v) {sz_time=v;}
	bool setValidZSliceNum(V3DLONG v)
	{
		if (v==0 && sz2==0) {valid_zslicenum=0; return true;}
		if (v>=0 && v<sz2) //in this case sz2>0
		{valid_zslicenum = v; return true;}
		else
			return false;
	}

	bool setPreValidZSliceNum(V3DLONG v)
	{
		if (v==0 && sz2==0) {prevalid_zslicenum=0; return true;}
		if (v>=0 && v<sz2) //in this case sz2>0
		{prevalid_zslicenum = v; return true;}
		else
			return false;
	}

	void setDatatype(ImagePixelType v) {datatype=v;}
	void setTimePackType(TimePackType v) {timepacktype=v;}
	bool setNewRawDataPointer(unsigned char *p) {if (!p) return false; if (data1d) delete []data1d; data1d = p; return true;}
	void setRawDataPointerToNull() { this->data1d = 0; }
	void deleteRawDataAndSetPointerToNull() { if (data1d) {delete []data1d; data1d = 0;} }
	void setRawDataPointer(unsigned char *p) { this->data1d = p; }

        bool  setValueUINT8(V3DLONG  x,  V3DLONG  y,  V3DLONG z, V3DLONG chanel, v3d_uint8 val)
        {
            if(datatype != 1){
                printf("Warning: This image is not stored in 8bit.");
                return false;
            }
            V3DLONG im_total_sz = sz0*sz1*sz2;
            V3DLONG idx = chanel * (im_total_sz) + z*(sz0*sz1) + y*sz0 + x;
            if (idx < sz0*sz1*sz2*sz3)
            {
                  this->data1d[idx] = val;
                  return true;
            }
            else
            {
                printf("Image4DSimple::getIntensity() error: index exceeds the image size");
                return false;
            }

        }

        v3d_uint8  getValueUINT8(V3DLONG  x,  V3DLONG  y,  V3DLONG z,V3DLONG chanel )
        {
            if(datatype!=1){
                printf("Warning: This image is not stored in 8bit.");
                return 0;
            }

            V3DLONG im_total_sz = sz0*sz1*sz2;
            V3DLONG idx = chanel * (im_total_sz) + z*(sz0*sz1) + y*sz0 + x;
            if (idx < sz0*sz1*sz2*sz3)
            {
                return this->data1d[idx];
            }
            else
            {
                printf("Image4DSimple::getIntensity() error: index exceeds the image size");
                return 0;
            }
        }


        bool convert_to_UINT8(){

            if (this->datatype ==1 )
            {
                return true;
            }

            if (this->datatype!=2 && this->datatype!=4)
            {
                    fprintf(stderr, "This function convert_type2uint8_3dimg_1dpt() is designed to convert 16 bit and single-precision-float only [%s][%d].\n", __FILE__, __LINE__);
                    return false;
            }

            if (sz0<1 || sz1<1 || sz2<1 || sz3<1)
            {
                    fprintf(stderr, "Input image size is not valid [%s][%d].\n", __FILE__, __LINE__);
                    return false;
            }

            V3DLONG totalunits = sz0*sz1*sz2*sz3;
            v3d_uint8 * outimg = new v3d_uint8 [totalunits];
            if (!outimg)
            {
                    fprintf(stderr, "Fail to allocate memory. [%s][%d].\n", __FILE__, __LINE__);
                    return false;
            }

            if (datatype==2)
            {
                    v3d_uint16 * tmpimg = (v3d_uint16 *)this->getRawData();
                    V3DLONG i; double maxvv=tmpimg[0];
                    for (i=0;i<totalunits;i++)
                    {
                            maxvv = (maxvv<tmpimg[i]) ? tmpimg[i] : maxvv;
                    }
                    if (maxvv>255.0)
                    {
                            maxvv = 255.0/maxvv;
                for (/*V3DLONG*/ i=0;i<totalunits;i++)
                            {
                                    outimg[i] = (v3d_uint8)(double(tmpimg[i])*maxvv);
                            }
                    }
                    else
                    {
                for (/*V3DLONG*/ i=0;i<totalunits;i++)
                            {
                                    outimg[i] = (v3d_uint8)(tmpimg[i]); //then no need to rescale
                            }
                    }
            }
            else
            {
                    v3d_float32 * tmpimg = (v3d_float32 *)this->getRawData();
                    V3DLONG i; double maxvv=tmpimg[0], minvv=tmpimg[0];
                    for (i=0;i<totalunits;i++)
                    {
                            if (tmpimg[i]>maxvv) maxvv = tmpimg[i];
                            else if (tmpimg[i]<minvv) minvv = tmpimg[i];
                    }
                    if (maxvv!=minvv)
                    {
                            double w = 255.0/(maxvv-minvv);
                for (/*V3DLONG*/ i=0;i<totalunits;i++)
                            {
                                    outimg[i] = (v3d_uint8)(double(tmpimg[i]-minvv)*w);
                            }
                    }
                    else
                    {
                for (/*V3DLONG*/ i=0;i<totalunits;i++)
                            {
                                    outimg[i] = (v3d_uint8)(tmpimg[i]); //then no need to rescale. If the original value is small than 0 or bigger than 255, then let it be during the type-conversion
                            }
                    }
            }

            //delete [] (this->getRawData());
            this->setNewRawDataPointer(outimg);
            this->setDatatype(V3D_UINT8);
            return true;


        }



        bool setRezX(double a) { if (a<=0) return false; rez_x = a; return true;}
	bool setRezY(double a) { if (a<=0) return false; rez_y = a; return true;}
	bool setRezZ(double a) { if (a<=0) return false; rez_z = a; return true;}
	void setOriginX(double a) { origin_x = a;}
	void setOriginY(double a) { origin_y = a;}
	void setOriginZ(double a) { origin_z = a;}


	void setCustomStructPointer(void *a) {p_customStruct = a;}
	void * getCustomStructPointer() {return p_customStruct;}
	bool isValidCustomStructPointer() {return (p_customStruct!=0)?true:false;}

	//this function is the main place to call if you want to set your own 1d pointer data to this data structure
	bool setData(unsigned char *p, Image4DSimple * image )
	{
		bool result = this->setData( p, image->sz0, image->sz1, image->sz2, image->sz3, image->datatype );
		return result;
	}
	bool setData(unsigned char *p, V3DLONG s0, V3DLONG s1, V3DLONG s2, V3DLONG s3, ImagePixelType dt)
	{
		if (p && s0>0 && s1>0 && s2>0 && s3>0 && (dt==V3D_UINT8 || dt==V3D_UINT16 || dt==V3D_FLOAT32))
			if (setNewRawDataPointer(p))
			{
				setXDim(s0);
				setYDim(s1);
				setZDim(s2);
				setCDim(s3);
				setDatatype(dt);
				return true;
			}
		return false;
	}

	bool setFileName(const char * myfile)
	{
		if (!myfile) return false;
		V3DLONG clen=1023;
		V3DLONG i;
		for (i=0;i<clen;i++)
		{
			if (myfile[i]!='\0')
			{
				imgSrcFile[i] = myfile[i];
			}
			else
			{
				imgSrcFile[i] = myfile[i];
				break;
			}
		}
		imgSrcFile[i]='\0';
		return true;
	}
	const char * getFileName() const { return imgSrcFile; }

    //to call the following 4 functions you must link your project with basic_4dimage.cpp
	//Normally for the plugin interfaces you don't need to call the following functions
	void loadImage(const char* filename);
	void loadImage(const char* filename, bool b_useMylib);
    void loadImage_slice(char filename[], bool b_useMyLib, V3DLONG zsliceno);
	bool saveImage(const char filename[]);

	bool createImage(V3DLONG mysz0, V3DLONG mysz1, V3DLONG mysz2, V3DLONG mysz3, ImagePixelType mytype);
    void createBlankImage(V3DLONG imgsz0, V3DLONG imgsz1, V3DLONG imgsz2, V3DLONG imgsz3, int imgdatatype);

	//a function to check if the data buffer is the same as another image
	bool isSameDataBuffer( Image4DSimple *p)
	{
		if (!p) return false; // cannot be the same if the pointer to be compared is null
		if (data1d!=p->getRawData())
			return false; //the data of course are different if the pointers are different
		else //there is a chance that the data pointers are the same, but their sizes are different
		{
			if (getTotalBytes() == p->getTotalBytes())
				return false;
			else return true;
		}
	}
};

bool convert_data_to_8bit(void * &img, V3DLONG * sz, int datatype);

// Image4DProxy for easy accessing pixels in Image4DSimple
template<class T> class Image4DProxy
{
public:
	std::vector<double> vmin, vmax;
	T *img0;
	v3d_uint8* data_p;
	V3DLONG nbytes, su, sx, sy, sz, sc;
	V3DLONG stride_x, stride_y, stride_z, stride_c;
	Image4DProxy(T *img)
	{
		img0 = img; //keep a copy for future accessing of the data
		data_p = img->getRawData();
		nbytes = img->getTotalBytes();
		su = img->getUnitBytes();
		sx = img->getXDim();
		sy = img->getYDim();
		sz = img->getZDim();
		sc = img->getCDim();
		stride_x = su;
		stride_y = su*sx;
		stride_z = su*sx*sy;
		stride_c = su*sx*sy*sz;
	}
	inline v3d_uint8* at(V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG c)
	{
		return (data_p + stride_x*x + stride_y*y + stride_z*z + stride_c*c);
	}
        inline const v3d_uint8* at(V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG c) const
        {
                return (data_p + stride_x*x + stride_y*y + stride_z*z + stride_c*c);
        }
        inline v3d_uint8* begin()
	{
		return data_p;
	}
	inline v3d_uint8* end()
	{
		return (data_p + nbytes-1);
	}
	inline bool is_inner(V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG c)
	{
		return !(x<0 || x>=sx || y<0 || y>=sy || z<0 || z>=sz || c<0 || c>=sc);
	}
	inline v3d_uint8* at_uint8(V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG c)
	{
		return (v3d_uint8*)at(x,y,z,c);
	}
	inline v3d_uint16* at_uint16(V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG c)
	{
		return (v3d_uint16*)at(x,y,z,c);
	}
	inline v3d_float32* at_float32(V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG c)
	{
		return (v3d_float32*)at(x,y,z,c);
	}
        inline double value_at(V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG c) const
	{
		double v;
		switch (su)
		{
			case 4: v = (double)(*(v3d_float32 *)at(x,y,z,c)); break;
			case 2: v = (double)(*(v3d_uint16 *)at(x,y,z,c)); break;
			case 1:
			default: v = (double)(*at(x,y,z,c)); break;
		}
		return v;
	}
	inline void put_at(V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG c, double v)
	{
		switch (su)
		{
			case 4: *at_float32(x,y,z,c) = (v3d_float32)v; break;
			case 2: *at_uint16(x,y,z,c) = (v3d_uint16)v; break;
			case 1:
			default: *at_uint8(x,y,z,c) = (v3d_uint8)v; break;
		}
	}
	void set_minmax(double* p_vmin, double* p_vmax)
	{
		vmin.clear();
		vmax.clear();
		for (int i=0; i<sc; i++)
		{
			vmin.push_back( p_vmin[i] );
			vmax.push_back( p_vmax[i] );
		}
	}
        inline bool has_minmax() const
	{
		return vmin.size()==sc && vmax.size()==sc && su>1;
	}
        inline v3d_uint8 value8bit_at(V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG c) const
	{
		//double v = value_at(x,y,z);
		double v;
		switch (su)
		{
			case 4: v = (double)(*(v3d_float32 *)at(x,y,z,c)); break;
			case 2: v = (double)(*(v3d_uint16 *)at(x,y,z,c)); break;
			case 1:
			default: v = (double)(*at(x,y,z,c)); break;
		}
		if (has_minmax())
		{
			double r = 255./(vmax[c]-vmin[c]);
			v = (v-vmin[c])*r;
		}
		return (v3d_uint8)v;
	}
	inline void put8bit_fit_at(V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG c, v3d_uint8 v)
	{
		switch (su)
		{
		case 4:
			{
				double vv = v;
				if (has_minmax())
				{
					double r = (vmax[c]-vmin[c])/255.;
					vv = vmin[c]+(vv*r);
				}
				else {
					double r = 1/255.;
					vv = vv*r;
				}
				*at_float32(x,y,z,c) = (v3d_float32)vv;
			} break;
		case 2:
			{
				double vv = v;
				if (has_minmax())
				{
					double r = (vmax[c]-vmin[c])/255.;
					vv = vmin[c]+(vv*r);
				}
				else {
					double r = 0xffff/255.;
					vv = vv*r;
				}
				*at_uint16(x,y,z,c) = (v3d_uint16)v;
			} break;
		case 1:
		default: *at_uint8(x,y,z,c) = (v3d_uint8)v; break;
		}
	}
};

#define Image4DProxy_foreach(p, x,y,z,c) \
	for (V3DLONG c = 0; c < p.sc; c++) \
	for (V3DLONG z = 0; z < p.sz; z++) \
	for (V3DLONG y = 0; y < p.sy; y++) \
	for (V3DLONG x = 0; x < p.sx; x++)


//The following struct is provided for convenience for working with a channel of an Image4DSimple instance in some cases
struct V3D_Image3DBasic
{
	unsigned char * data1d;
	V3DLONG sz0, sz1, sz2;
	ImagePixelType datatype;
	V3DLONG cid; //the color channel in the original 4D image

	V3D_Image3DBasic() {data1d=0; sz0=sz1=sz2=0; datatype=V3D_UNKNOWN; cid=-1;}
	bool setData(Image4DSimple *p, V3DLONG myid)
	{
		if (!p || !p->valid() || myid<0 || myid>=p->getCDim())
			return false;
		cid = myid;
		data1d = p->getRawDataAtChannel(cid);
		sz0 = p->getXDim(); sz1 = p->getYDim(); sz2 = p->getZDim();
		datatype = p->getDatatype();
		return true;
	}
};


#endif /* _BASIC_4DIMAGE_H_ */
