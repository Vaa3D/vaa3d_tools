//------------------------------------------------------------------------------------------------
// Copyright (c) 2012  Alessandro Bria and Giulio Iannello (University Campus Bio-Medico of Rome).  
// All rights reserved.
//------------------------------------------------------------------------------------------------

/*******************************************************************************************************************************************************************************************
*    LICENSE NOTICE
********************************************************************************************************************************************************************************************
*    By downloading/using/running/editing/changing any portion of codes in this package you agree to this license. If you do not agree to this license, do not download/use/run/edit/change
*    this code.
********************************************************************************************************************************************************************************************
*    1. This material is free for non-profit research, but needs a special license for any commercial purpose. Please contact Alessandro Bria at a.bria@unicas.it or Giulio Iannello at 
*       g.iannello@unicampus.it for further details.
*    2. You agree to appropriately cite this work in your related studies and publications.
*    3. This material is provided by  the copyright holders (Alessandro Bria  and  Giulio Iannello),  University Campus Bio-Medico and contributors "as is" and any express or implied war-
*       ranties, including, but  not limited to,  any implied warranties  of merchantability,  non-infringement, or fitness for a particular purpose are  disclaimed. In no event shall the
*       copyright owners, University Campus Bio-Medico, or contributors be liable for any direct, indirect, incidental, special, exemplary, or  consequential  damages  (including, but not 
*       limited to, procurement of substitute goods or services; loss of use, data, or profits;reasonable royalties; or business interruption) however caused  and on any theory of liabil-
*       ity, whether in contract, strict liability, or tort  (including negligence or otherwise) arising in any way out of the use of this software,  even if advised of the possibility of
*       such damage.
*    4. Neither the name of University  Campus Bio-Medico of Rome, nor Alessandro Bria and Giulio Iannello, may be used to endorse or  promote products  derived from this software without
*       specific prior written permission.
********************************************************************************************************************************************************************************************/

#include "VirtualFmtMngr.h"
#include "Tiff3DMngr.h"

using namespace iim;


/*****************************************
 * class VirtualFmtMngr
 *****************************************/

void VirtualFmtMngr::copyBlock2SubBuf ( unsigned char *src, unsigned char *dst, int dimi, int dimj, int dimk, int typesize,
	 						   sint64 s_stridej, sint64 s_strideij, sint64 d_stridej, sint64 d_strideij ) {
	 						   
	unsigned char *s_slice = src;
	unsigned char *d_slice = dst;
	unsigned char *s_stripe;
	unsigned char *d_stripe;
	int i, k;
	
	s_stridej  *= typesize;
	s_strideij *= typesize;
	d_stridej  *= typesize;
	d_strideij *= typesize;
	
	for ( k=0; k<dimk; k++, s_slice+=s_strideij, d_slice+=d_strideij ) {
		for ( i=0, s_stripe=s_slice, d_stripe=d_slice; i<dimi; i++, s_stripe+=s_stridej, d_stripe+=d_stridej ) {
			memcpy(d_stripe,s_stripe,(dimj * typesize));
		}
	}
}

void VirtualFmtMngr::copyRGBBlock2Vaa3DRawSubBuf ( unsigned char *src, unsigned char *dst, int dimi, int dimj, int dimk, int typesize,
	 						   sint64 s_stridej, sint64 s_strideij, sint64 d_stridej, sint64 d_strideij, sint64 d_strideijk ) {
	 						   
	int c, i, j, k;
	
	if ( typesize == 1 ) {
	
		unsigned char *s_slice;
		unsigned char *d_slice;
		unsigned char *s_stripe;
		unsigned char *d_stripe;
		
		for ( c=0; c<3; c++, src++, dst+=d_strideijk ) {
			s_slice = src;
			d_slice = dst;
			for ( k=0; k<dimk; k++, s_slice+=3*s_strideij, d_slice+=d_strideij ) {
				for ( i=0, s_stripe=s_slice, d_stripe=d_slice; i<dimi; i++, s_stripe+=3*s_stridej, d_stripe+=d_stridej ) {
					for ( j=0; j<dimj; j++ )
						d_stripe[j] = s_stripe[3*j];
				}
			}
		}
	}
	else { // must be guaranteed that typesize ==2
	
		iim::uint16 *src16 = (iim::uint16 *) src;
		iim::uint16 *dst16 = (iim::uint16 *) dst;
			
		iim::uint16 *s_slice;
		iim::uint16 *d_slice;
		iim::uint16 *s_stripe;
		iim::uint16 *d_stripe;
		
// 		s_stridej  *= typesize;
// 		s_strideij *= typesize;
// 		d_stridej  *= typesize;
// 		d_strideij *= typesize;
	
		for ( c=0; c<3; c++, src16++, dst16+=d_strideijk ) {
			s_slice = src16;
			d_slice = dst16;
			for ( k=0; k<dimk; k++, s_slice+=3*s_strideij, d_slice+=d_strideij ) {
				for ( i=0, s_stripe=s_slice, d_stripe=d_slice; i<dimi; i++, s_stripe+=3*s_stridej, d_stripe+=d_stridej ) {
					for ( j=0; j<dimj; j++ )
						d_stripe[j] = s_stripe[3*j];
				}
			}
		}
	}
}


/*****************************************
 * class Tiff3DFmtMngr
 *****************************************/

char *Tiff3DFmtMngr::loadMetadata ( char * filename, sint64 * &sz, int &datatype, int &b_swap, void * &fhandle, int &header_len ) {
	unsigned int sz0, sz1, sz2, sz3;
	char *err_msg;
	if ( (err_msg = loadTiff3D2Metadata(filename,sz0,sz1,sz2,sz3,datatype,b_swap,fhandle,header_len)) != 0 ) {
		return err_msg;
	}

	sz = new sint64[4];
	sz[0] = (sint64) sz0;
	sz[1] = (sint64) sz1;
	sz[2] = (sint64) sz2;
	sz[3] = (sint64) sz3;
	return err_msg;
}

char *Tiff3DFmtMngr::load2SubStack ( void *fhandle, unsigned char *img, sint64 *sz, 
							 sint64 startx, sint64 starty, sint64 startz, 
							 sint64 endx, sint64 endy, sint64 endz, 
							 int datatype, int b_swap, int header_len ) {
	char *err_msg;
	unsigned char *img_t = new unsigned char[sz[0] * sz[1] * (endz - startz + 1) * sz[3] * datatype];
	if ( (err_msg = readTiff3DFile2Buffer(fhandle,img_t,(uint32)sz[0],(uint32)sz[1],(uint32)startz,(uint32)endz,b_swap)) != 0 ) {
		delete [] img_t;
		return err_msg;      
	}
	
	sint64 s_stridej = sz[0];
	sint64 s_strideij = sz[0] * sz[1];
	int dimi = (int)(endy - starty + 1);
	int dimj = (int)(endx - startx + 1);
	int dimk = (int)(endz - startz + 1);
	
	if ( sz[3] == 1 ) { // single channel Tiff
		VirtualFmtMngr::copyBlock2SubBuf(
			img_t + s_strideij*startz + s_stridej*startx + starty,
			img,
			dimi,dimj,dimk,datatype,
	 		s_stridej,s_strideij,
			dimj,(dimi * dimj)
		 );
	}
	else if ( sz[3] == 3 ) { // RGB Tiff
		VirtualFmtMngr::copyRGBBlock2Vaa3DRawSubBuf(
			img_t + s_strideij*startz + s_stridej*startx + starty,
			img,
			dimi,dimj,dimk,datatype,
	 		s_stridej,s_strideij,
			dimj,(dimi * dimj),(dimi * dimj * dimk)
		 );
	}
	else
		return ((char *) "in Tiff3DFmtMngr::load2SubStack(...): unsupported number of channels.");
	 						   
	delete [] img_t;
	
	return ((char *) 0);
}

char *Tiff3DFmtMngr::initFile ( char *filename, const iim::sint64 *sz, int datatype ) {
	return initTiff3DFile(filename,(unsigned int)sz[0],(unsigned int)sz[1],(unsigned int)sz[2],(unsigned int)sz[3],datatype);
}

char *Tiff3DFmtMngr::writeSlice2File ( char *filename, int slice, unsigned char *img, int img_height, int img_width ) {
	
	// must convert the buffer img of multichannel images from Vaa3D raw to Tiff3D layout
	
	return ((char *) "in Tiff3DFmtMngr::writeSlice2File(...): not implemented yet.");
	
	//return appendSlice2Tiff3DFile(filename,slice,img,img_width,img_height);
}

char *Tiff3DFmtMngr::copyFileBlock2Buffer ( char *filename, int sV0, int sV1, int sH0, int sH1, int sD0, int sD1,
                                unsigned char *buf, int pxl_size, iim::sint64 offs, iim::sint64 stridex, iim::sint64 stridexy, iim::sint64 stridexyz ) {

	char *err_msg;
	unsigned int sz[4];
	int datatype;
	int b_swap;
	void *fhandle;
	int header_len;
	
	// get metadata
    if ( (err_msg = loadTiff3D2Metadata(filename,sz[0],sz[1],sz[2],sz[3],datatype,b_swap,fhandle,header_len)) != 0 ) {
		return ((char *) "in Tiff3DFmtMngr::copyFileBlock2Buffer(...): cannot load metadata of Tiff file");
	}
	// fhandle should be left open

	if ( datatype != pxl_size ) {
		return ((char *) "in Tiff3DFmtMngr::copyFileBlock2Buffer(...): source data type differs from destination pixel size");
	}

	unsigned char *buf_t = new unsigned char[sz[0] * sz[1] * (sD1 - sD0) * sz[3] * datatype];
	
	if ( (err_msg = readTiff3DFile2Buffer(fhandle,buf_t,sz[0],sz[1],sD0,sD1-1,b_swap)) != 0 ) {
		closeTiff3DFile(fhandle);
		delete [] buf_t;
		return err_msg;      
	}

	closeTiff3DFile(fhandle);

	sint64 s_stridej  = sz[0];
	sint64 s_strideij = sz[0] * sz[1];
	sint64 d_stridej  = stridex;
	sint64 d_strideij = stridexy;
	
	int dimi = sV1 - sV0 ;
	int dimj = sH1 - sH0;
	int dimk = sD1 - sD0;
	
	if ( sz[3] == 1 ) { // single channeel Tiff
		VirtualFmtMngr::copyBlock2SubBuf(
			buf_t + pxl_size*(s_stridej*sV0 + sH0),
			buf + pxl_size*offs,
			dimi,dimj,dimk,pxl_size,
	 		s_stridej,s_strideij,d_stridej,d_strideij
	 	);
	 }
	else if ( sz[3] == 3 ) { // RGB Tiff
		sint64 d_strideijk = stridexyz;
		VirtualFmtMngr::copyRGBBlock2Vaa3DRawSubBuf(
			buf_t + 3*pxl_size*(s_stridej*sV0 + sH0),
			buf + pxl_size*offs,
			dimi,dimj,dimk,pxl_size,
	 		s_stridej,s_strideij,
			d_stridej,d_strideij,d_strideijk
			);
	}
	else {
		delete []buf_t;
		return ((char *) "in Tiff3DFmtMngr::copyFileBlock2Buffer(...): unsupported number of channels.");
	}
	 						   
	delete [] buf_t;
	
	return ((char *) 0);
}


/*****************************************
 * class Vaa3DRawFmtMngr
 *****************************************/

char *Vaa3DRawFmtMngr::loadMetadata ( char * filename, sint64 * &sz, int &datatype, int &b_swap, void * &fhandle, int &header_len ) {
	V3DLONG *_sz = 0; // Windows has invalid non-zero pointers
	char *err_msg;
	if ( (err_msg = loadRaw2Metadata(filename,_sz,datatype,b_swap,fhandle,header_len)) != 0 ) {
		return err_msg;
	}

	if ( sizeof(sint64) == sizeof(V3DLONG) ) {
		sz = (sint64 *) _sz;
	}
	else {
		sz = new sint64[4];
		for ( int i=0; i<4; i++ )
			sz[i] = (sint64) _sz[i];
		delete _sz;
	}

	return ((char *) 0);
}

char *Vaa3DRawFmtMngr::load2SubStack ( void *fhandle, unsigned char *img,sint64 *sz, sint64 startx, sint64 starty, sint64 startz, 
							 		   sint64 endx, sint64 endy, sint64 endz,int datatype, int b_swap, int header_len ) {
	if ( sizeof(sint64) == sizeof(V3DLONG) ) {
		return loadRaw2SubStack(fhandle,img,(V3DLONG *)sz,(V3DLONG)startx,(V3DLONG)starty,(V3DLONG)startz,
								(V3DLONG)endx,(V3DLONG)endy,(V3DLONG)endz,datatype,b_swap,header_len);
	}
	else {
		V3DLONG *_sz = new V3DLONG[4];
		_sz[0] = sz[0];
		_sz[1] = sz[1];
		_sz[2] = sz[2];
		_sz[3] = sz[3];
		return loadRaw2SubStack(fhandle,img,_sz,(V3DLONG)startx,(V3DLONG)starty,(V3DLONG)startz,
								(V3DLONG)endx,(V3DLONG)endy,(V3DLONG)endz,datatype,b_swap,header_len);
	}
}

char *Vaa3DRawFmtMngr::initFile ( char *filename, const sint64 *sz, int datatype ) {
	if ( sizeof(sint64) == sizeof(V3DLONG) ) {
		return initRawFile(filename,(V3DLONG *) sz,datatype);
	}
	else {
		V3DLONG *_sz = new V3DLONG[4];
		_sz[0] = sz[0];
		_sz[1] = sz[1];
		_sz[2] = sz[2];
		_sz[3] = sz[3];
		return initRawFile(filename,_sz,datatype);
	}
}

char *Vaa3DRawFmtMngr::writeSlice2File ( char *filename, int slice, unsigned char *img, int img_height, int img_width ) {
	return writeSlice2RawFile(filename,slice,img,img_height,img_width );
}

char *Vaa3DRawFmtMngr::copyFileBlock2Buffer ( char *filename, int sV0, int sV1, int sH0, int sH1, int sD0, int sD1,
                                unsigned char *buf, int pxl_size, sint64 offs, sint64 stridex, sint64 stridexy, sint64 stridexyz ) {
	return copyRawFileBlock2Buffer(filename,sV0,sV1,sH0,sH1,sD0,sD1,buf,pxl_size,offs,stridex,stridexy,stridexyz);
}
