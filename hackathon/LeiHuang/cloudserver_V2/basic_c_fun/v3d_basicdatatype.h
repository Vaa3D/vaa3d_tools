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




/*
v3d_basicdatatype.h: by Hanchuan Peng
2010-05-19
2011-02-15: add v3d_ in front of some basic data types 
2012-04-10: add V3D_THREEBYTE to make the  V3D_FLOAT32 type has a default value of 4 instead of 3 when forced to convert to int
*/

#ifndef __V3D_BASICDATATYPE_H__
#define __V3D_BASICDATATYPE_H__

// be compatible with LP64(unix64) and LLP64(win64)
typedef unsigned char        v3d_uint8;
typedef unsigned short       v3d_uint16;
typedef unsigned int         v3d_uint32;
typedef unsigned long long   v3d_uint64;
typedef          char        v3d_sint8;
typedef          short       v3d_sint16;
typedef          int         v3d_sint32;
typedef          long long   v3d_sint64;
typedef          float       v3d_float32;
typedef          double      v3d_float64;

typedef void* v3dhandle;

//2010-05-19: by Hanchuan Peng. add the MSVC specific version # (vc 2008 has a _MSC_VER=1500) and win64 macro. 
//Note that _WIN32 seems always defined for any windows application.
//For more info see page for example: http://msdn.microsoft.com/en-us/library/b0084kay%28VS.80%29.aspx

#if defined(_MSC_VER) && (_WIN64)
//#if defined(_MSC_VER) && defined(_WIN64) //correct?

#define V3DLONG long long

#else

#define V3DLONG long

#endif

#if defined (_MSC_VER)

#define strcasecmp strcmpi

#endif

enum ImagePixelType {V3D_UNKNOWN, V3D_UINT8, V3D_UINT16, V3D_THREEBYTE, V3D_FLOAT32};
enum TimePackType {TIME_PACK_NONE,TIME_PACK_Z,TIME_PACK_C}; 


#endif

