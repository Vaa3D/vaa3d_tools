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




/* basic_memory.h
   Some basic memory functions for applications.

   by Hanchuan Peng
   2007-02-16: separated from the original "basic_func.h" so that only include the memory functions. Note that 
               the interface functions have been rewritten so that they only have one template function without 
			   type conversion. Also change the return type from "int" to "bool", so that they are clearer.
			   
   2007-02-16: Because I removed all the type conversion in the "new*" functions, an explicit copying function
               from a type such as "unsigned char" to "double" is needed. Thus I added a few often-needed wrappers
			   for these EXPLICIT conversions. These functions are named using "copy1dMem*".
   2009-08-2: add 5d memory management 

   2010-05-19: add v3d_basicdatatype.h

*/

#ifndef __BASIC_MEMORY_H__
#define __BASIC_MEMORY_H__

#include "v3d_basicdatatype.h"

template <class T> bool new2dpointer(T ** & p, V3DLONG sz0, V3DLONG sz1, const T * p1d);
template <class T> void delete2dpointer(T ** & p, V3DLONG sz0, V3DLONG sz1);

template <class T> bool new3dpointer(T *** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, const T * p1d);
template <class T> void delete3dpointer(T *** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2);

template <class T> bool new4dpointer(T **** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, const T * p1d);
template <class T> void delete4dpointer(T **** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3);

template <class T> bool new5dpointer(T **** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, V3DLONG sz4, const T * p1d);
template <class T> void delete5dpointer(T **** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, V3DLONG sz4);

template <class T> bool copy1dMem(T * &p, V3DLONG len, const char * p1d);
template <class T> bool copy1dMem(T * &p, V3DLONG len, const unsigned char * p1d);
template <class T> bool copy1dMem(T * &p, V3DLONG len, const int * p1d);
template <class T> bool copy1dMem(T * &p, V3DLONG len, const unsigned int * p1d);
template <class T> bool copy1dMem(T * &p, V3DLONG len, const short int * p1d);
template <class T> bool copy1dMem(T * &p, V3DLONG len, const unsigned short int * p1d);
template <class T> bool copy1dMem(T * &p, V3DLONG len, const V3DLONG * p1d);
template <class T> bool copy1dMem(T * &p, V3DLONG len, const unsigned V3DLONG * p1d);
template <class T> bool copy1dMem(T * &p, V3DLONG len, const float * p1d);
template <class T> bool copy1dMem(T * &p, V3DLONG len, const double * p1d);


/*
template <class T> int new3dpointer(T *** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, unsigned char * p1d);
template <class T> int new3dpointer(T *** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, double * p1d);
template <class T> int new3dpointer(T *** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, float * p1d);
//template <class T> int new3dpointer(T *** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, T * p1d);
//template <class T> int new3dpointer(double *** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, T * p1d);
template <class T> void delete3dpointer(T *** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2);

template <class T> int new4dpointer(T **** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, unsigned char * p1d);
template <class T> int new4dpointer(T **** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, double * p1d);
template <class T> int new4dpointer(T **** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, float * p1d);
//template <class T> int new4dpointer(T **** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, T * p1d);
template <class T> void delete4dpointer(T **** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3);
*/

#endif

