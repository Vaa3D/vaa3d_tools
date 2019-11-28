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




/* basic_memory.cpp
   Some basic memory functions shared by various programs

   by Hanchuan Peng
   2007-02-16: separated from the original basic_func program.
   2007-02-16: due to the "template instantiation" problem I met so that 
               I re-wrote some interface functions in the following. 
			   
			   *** 
			   
			   Note that this program should always be physically "included"
			   in another program which calls some of the functions, so that there
			   would not be the linking problem like "/usr/bin/ld: Undefined symbols:". 
   
               ***
			   
	2007-02-16: add a series of copy1dMem* functions for explicit type conversion of arrayed data. *** Note: have not been tested ***.
2007-03-26: as a __BASIC_MEMORY_CPP__ definition, as this file will also be included	
2008-07-27: add a stupid sentence to eliminate some warning messages
 2009-08-02: add 5d memory management
*/

#ifndef __BASIC_MEMORY_CPP__
#define __BASIC_MEMORY_CPP__

#include "basic_memory.h"

#include <stdio.h>

template <class T> bool new2dpointer(T ** & p, V3DLONG sz0, V3DLONG sz1, const T * p1d)
{
  if (p!=0) {return false;} //if the "p" is not empty initially, then do nothing and return un-successful
  
  p = new T * [sz1];
  if (!p) {return false;}
  
  for (V3DLONG j=0;j<sz1; j++)
  {
    p[j] = (T *)p1d + j*sz0; 
  }
 
  return true;
}


template <class T> void delete2dpointer(T ** & p, V3DLONG sz0, V3DLONG sz1)
{
  if (p==0) {return;} //if the "p" is empty initially, then do nothing
  delete [] p;
  p = 0;

//stupid method to eliminate some warning msg
  sz0=sz0; sz1=sz1; 

  return;
}

template <class T> bool new3dpointer(T *** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, const T * p1d)
{
  if (p!=0) {return false;} //if the "p" is not empty initially, then do nothing and return un-successful
  
  p = new T ** [sz2];
  if (!p) {return false;}
  
  for (V3DLONG i=0;i<sz2; i++)
  {
    p[i] = new T * [sz1];
	if (!p[i])
	{
	  for (V3DLONG j=0;j<i;j++) {delete [] (p[i]);}
	  delete []p; 
	  p=0;
	  return false;
	}
	else
	{
	  for (V3DLONG j=0;j<sz1; j++)
	    p[i][j] = (T *)p1d + i*sz1*sz0 + j*sz0; //2007-02-16: it seems it would make no sense to force a pointer change type. And it would be very dangeous to do this.
	}
  }
  
  return true;
}


template <class T> void delete3dpointer(T *** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2)
{
  if (p==0) {return;} //if the "p" is empty initially, then do nothing
  
  for (V3DLONG i=0;i<sz2; i++)
  {
    if (p[i])
	{
	  delete [] (p[i]);
	  p[i] = 0;
	}
  }
  
  delete [] p;
  p = 0;

  //080727 stupid method to eliminate some warning msg
  sz0=sz0; sz1=sz1; sz2=sz2;

  return;
}

template <class T> bool new4dpointer(T **** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, const T * p1d)
{
  if (p!=0) {return false;} //if the "p" is not empty initially, then do nothing and return un-successful
  
  p = new T *** [sz3];
  if (!p) {return false;}
  
  for (V3DLONG i=0;i<sz3; i++)
  {
    p[i] = 0; //this sentence is very important to assure the function below knows this pointer is initialized as empty!!
    if (!new3dpointer(p[i], sz0, sz1, sz2, p1d+i*sz2*sz1*sz0)) 
	{
	  printf("Problem happened in creating 3D pointers for channel-%ld.\n", i);
	  for (V3DLONG j=0;j<i;j++) {delete [] (p[i]);}
	  delete []p; 
	  p=0;
	  return false;
	}
  }

  return true;
}


template <class T> void delete4dpointer(T **** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3)
{
  if (p==0) {return;} //if the "p" is empty initially, then do nothing
  
  for (V3DLONG i=0;i<sz3; i++)
  {
    delete3dpointer(p[i], sz0, sz1, sz2);
  }
  
  delete [] p;
  p = 0;

  //stupid method to eliminate some warning msg
  sz0=sz0; sz1=sz1; sz2=sz2; sz3=sz3;
 
  return;
}

template <class T> bool new5dpointer(T **** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, V3DLONG sz4, const T * p1d)
{
	if (p!=0) {return false;} //if the "p" is not empty initially, then do nothing and return un-successful
	
	p = new T **** [sz4];
	if (!p) {return false;}
	
	for (V3DLONG i=0;i<sz4; i++)
	{
		p[i] = 0; //this sentence is very important to assure the function below knows this pointer is initialized as empty!!
		if (!new4dpointer(p[i], sz0, sz1, sz2, sz3, p1d+i*sz3*sz2*sz1*sz0)) 
		{
			printf("Problem happened in creating 4D pointers for channel-%ld.\n", i);
			for (V3DLONG j=0;j<i;j++) {delete [] (p[i]);}
			delete []p; 
			p=0;
			return false;
		}
	}
	
	return true;
}

template <class T> void delete5dpointer(T **** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, V3DLONG sz4)
{
	if (p==0) {return;} //if the "p" is empty initially, then do nothing
	
	for (V3DLONG i=0;i<sz4; i++)
	{
		delete4dpointer(p[i], sz0, sz1, sz2, sz3);
	}
	
	delete [] p;
	p = 0;
	
	//stupid method to eliminate some warning msg
	sz0=sz0; sz1=sz1; sz2=sz2; sz3=sz3; sz4=sz4;
	
	return;
}

template <class T> bool copy1dMem(T * &p, V3DLONG len, const char * p1d)
{
  if (p!=0 || len<0) {return false;} //if the "p" is not empty initially, then do nothing and return un-successful
  p = new T [len];
  if (!p) {return false;}
  for (V3DLONG i=0;i<len;i++)
    p[i] = (T)(p1d[i]);
  return true;	
}

template <class T> bool copy1dMem(T * &p, V3DLONG len, const unsigned char * p1d)
{
  if (p!=0 || len<0) {return false;} //if the "p" is not empty initially, then do nothing and return un-successful
  p = new T [len];
  if (!p) {return false;}
  for (V3DLONG i=0;i<len;i++)
    p[i] = (T)(p1d[i]);
  return true;	
}

template <class T> bool copy1dMem(T * &p, V3DLONG len, const int * p1d)
{
  if (p!=0 || len<0) {return false;} //if the "p" is not empty initially, then do nothing and return un-successful
  p = new T [len];
  if (!p) {return false;}
  for (V3DLONG i=0;i<len;i++)
    p[i] = (T)(p1d[i]);
  return true;	
}

template <class T> bool copy1dMem(T * &p, V3DLONG len, const unsigned int * p1d)
{
  if (p!=0 || len<0) {return false;} //if the "p" is not empty initially, then do nothing and return un-successful
  p = new T [len];
  if (!p) {return false;}
  for (V3DLONG i=0;i<len;i++)
    p[i] = (T)(p1d[i]);
  return true;	
}

template <class T> bool copy1dMem(T * &p, V3DLONG len, const short int * p1d)
{
  if (p!=0 || len<0) {return false;} //if the "p" is not empty initially, then do nothing and return un-successful
  p = new T [len];
  if (!p) {return false;}
  for (V3DLONG i=0;i<len;i++)
    p[i] = (T)(p1d[i]);
  return true;	
}

template <class T> bool copy1dMem(T * &p, V3DLONG len, const unsigned short int * p1d)
{
  if (p!=0 || len<0) {return false;} //if the "p" is not empty initially, then do nothing and return un-successful
  p = new T [len];
  if (!p) {return false;}
  for (V3DLONG i=0;i<len;i++)
    p[i] = (T)(p1d[i]);
  return true;	
}

template <class T> bool copy1dMem(T * &p, V3DLONG len, const V3DLONG * p1d)
{
  if (p!=0 || len<0) {return false;} //if the "p" is not empty initially, then do nothing and return un-successful
  p = new T [len];
  if (!p) {return false;}
  for (V3DLONG i=0;i<len;i++)
    p[i] = (T)(p1d[i]);
  return true;	
}

template <class T> bool copy1dMem(T * &p, V3DLONG len, const unsigned V3DLONG * p1d)
{
  if (p!=0 || len<0) {return false;} //if the "p" is not empty initially, then do nothing and return un-successful
  p = new T [len];
  if (!p) {return false;}
  for (V3DLONG i=0;i<len;i++)
    p[i] = (T)(p1d[i]);
  return true;	
}

template <class T> bool copy1dMem(T * &p, V3DLONG len, const float * p1d)
{
  if (p!=0 || len<0) {return false;} //if the "p" is not empty initially, then do nothing and return un-successful
  p = new T [len];
  if (!p) {return false;}
  for (V3DLONG i=0;i<len;i++)
    p[i] = (T)(p1d[i]);
  return true;	
}

template <class T> bool copy1dMem(T * &p, V3DLONG len, const double * p1d)
{
  if (p!=0 || len<0) {return false;} //if the "p" is not empty initially, then do nothing and return un-successful
  p = new T [len];
  if (!p) {return false;}
  for (V3DLONG i=0;i<len;i++)
    p[i] = (T)(p1d[i]);
  return true;	
}


/*

template <class T> int new3dpointer(T *** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, double * p1d)
{
  if (p!=0) {return 0;} //if the "p" is not empty initially, then do nothing and return un-successful
  
  p = new T ** [sz2];
  if (!p) {return 0;}
  
  for (V3DLONG i=0;i<sz2; i++)
  {
    p[i] = new T * [sz1];
	if (!p[i])
	{
	  for (V3DLONG j=0;j<i;j++) {delete [] (p[i]);}
	  delete []p; 
	  p=0;
	  return 0;
	}
	else
	{
	  for (V3DLONG j=0;j<sz1; j++)
//	    p[i][j] = (T *)p1d + i*sz1*sz0*sizeof(T) + j*sz0*sizeof(T);
	    p[i][j] = (T *)p1d + i*sz1*sz0 + j*sz0; //060424
	}
  }
  
  return 1;
}

template <class T> int new3dpointer(T *** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, float * p1d)
{
  if (p!=0) {return 0;} //if the "p" is not empty initially, then do nothing and return un-successful
  
  p = new T ** [sz2];
  if (!p) {return 0;}
  
  for (V3DLONG i=0;i<sz2; i++)
  {
    p[i] = new T * [sz1];
	if (!p[i])
	{
	  for (V3DLONG j=0;j<i;j++) {delete [] (p[i]);}
	  delete []p; 
	  p=0;
	  return 0;
	}
	else
	{
	  for (V3DLONG j=0;j<sz1; j++)
//	    p[i][j] = (T *)p1d + i*sz1*sz0*sizeof(T) + j*sz0*sizeof(T);
	    p[i][j] = (T *)p1d + i*sz1*sz0 + j*sz0; //060424
	}
  }
  
  return 1;
}



template <class T> int new4dpointer(T **** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, unsigned char * p1d)
{
  if (p!=0) {return 0;} //if the "p" is not empty initially, then do nothing and return un-successful
  
  p = new T *** [sz3];
  if (!p) {return 0;}
  
  for (V3DLONG i=0;i<sz3; i++)
  {
    p[i] = 0; //this sentence is very important to assure the function below knows this pointer is initialized as empty!!
    //if (!new3dpointer(p[i], sz0, sz1, sz2, p1d+i*sz2*sz1*sz0*sizeof(T))) //070207:if this a bug? guess should not have *sizeof(T)
    if (!new3dpointer(p[i], sz0, sz1, sz2, p1d+i*sz2*sz1*sz0)) //070207:if this a bug? guess should not have *sizeof(T)
	{
	  printf("Problem happened in creating 3D pointers for channel-%d.\n", i);
	  for (V3DLONG j=0;j<i;j++) {delete [] (p[i]);}
	  delete []p; 
	  p=0;
	  return 0;
	}
  }

  return 1;
}

template <class T> int new4dpointer(T **** & p, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3, double * p1d)
{
  if (p!=0) {return 0;} //if the "p" is not empty initially, then do nothing and return un-successful
  
  p = new T *** [sz3];
  if (!p) {return 0;}
  
  for (V3DLONG i=0;i<sz3; i++)
  {
    p[i] = 0; //this sentence is very important to assure the function below knows this pointer is initialized as empty!!
    if (!new3dpointer(p[i], sz0, sz1, sz2, p1d+i*sz2*sz1*sz0)) //070207: seems correct an error
	{
	  printf("Problem happened in creating 3D pointers for channel-%d.\n", i);
	  for (V3DLONG j=0;j<i;j++) {delete [] (p[i]);}
	  delete []p; 
	  p=0;
	  return 0;
	}
  }

  return 1;
}

*/

#endif



