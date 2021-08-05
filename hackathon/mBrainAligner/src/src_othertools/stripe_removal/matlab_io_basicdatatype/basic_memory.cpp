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
	
*/

#include "basic_memory.h"

#include <stdio.h>

template <class T> bool new2dpointer(T ** & p, long sz0, long sz1, const T * p1d)
{
  if (p!=0) {return false;} //if the "p" is not empty initially, then do nothing and return un-successful
  
  p = new T * [sz1];
  if (!p) {return false;}
  
  for (long j=0;j<sz1; j++)
  {
    p[j] = (T *)p1d + j*sz0; 
  }
  
  return true;
}


template <class T> void delete2dpointer(T ** & p, long sz0, long sz1)
{
  if (p==0) {return;} //if the "p" is empty initially, then do nothing
  delete [] p;
  p = 0;
  return;
}

template <class T> bool new3dpointer(T *** & p, long sz0, long sz1, long sz2, const T * p1d)
{
  if (p!=0) {return false;} //if the "p" is not empty initially, then do nothing and return un-successful
  
  p = new T ** [sz2];
  if (!p) {return false;}
  
  for (long i=0;i<sz2; i++)
  {
    p[i] = new T * [sz1];
	if (!p[i])
	{
	  for (long j=0;j<i;j++) {delete [] (p[i]);}
	  delete []p; 
	  p=0;
	  return false;
	}
	else
	{
	  for (long j=0;j<sz1; j++)
	    p[i][j] = (T *)p1d + i*sz1*sz0 + j*sz0; //2007-02-16: it seems it would make no sense to force a pointer change type. And it would be very dangeous to do this.
	}
  }
  
  return true;
}


template <class T> void delete3dpointer(T *** & p, long sz0, long sz1, long sz2)
{
  if (p==0) {return;} //if the "p" is empty initially, then do nothing
  
  for (long i=0;i<sz2; i++)
  {
    if (p[i])
	{
	  delete [] (p[i]);
	  p[i] = 0;
	}
  }
  
  delete [] p;
  p = 0;

  return;
}

template <class T> bool new4dpointer(T **** & p, long sz0, long sz1, long sz2, long sz3, const T * p1d)
{
  if (p!=0) {return false;} //if the "p" is not empty initially, then do nothing and return un-successful
  
  p = new T *** [sz3];
  if (!p) {return false;}
  
  for (long i=0;i<sz3; i++)
  {
    p[i] = 0; //this sentence is very important to assure the function below knows this pointer is initialized as empty!!
    if (!new3dpointer(p[i], sz0, sz1, sz2, p1d+i*sz2*sz1*sz0)) 
	{
	  printf("Problem happened in creating 3D pointers for channel-%d.\n", i);
	  for (long j=0;j<i;j++) {delete [] (p[i]);}
	  delete []p; 
	  p=0;
	  return false;
	}
  }

  return true;
}


template <class T> void delete4dpointer(T **** & p, long sz0, long sz1, long sz2, long sz3)
{
  if (p==0) {return;} //if the "p" is empty initially, then do nothing
  
  for (long i=0;i<sz3; i++)
  {
    delete3dpointer(p[i], sz0, sz1, sz2);
  }
  
  delete [] p;
  p = 0;

  return;
}


template <class T> bool copy1dMem(T * &p, long len, const char * p1d)
{
  if (p!=0 || len<0) {return false;} //if the "p" is not empty initially, then do nothing and return un-successful
  p = new T [len];
  if (!p) {return false;}
  for (long i=0;i<len;i++)
    p[i] = (T)(p1d[i]);
  return true;	
}

template <class T> bool copy1dMem(T * &p, long len, const unsigned char * p1d)
{
  if (p!=0 || len<0) {return false;} //if the "p" is not empty initially, then do nothing and return un-successful
  p = new T [len];
  if (!p) {return false;}
  for (long i=0;i<len;i++)
    p[i] = (T)(p1d[i]);
  return true;	
}

template <class T> bool copy1dMem(T * &p, long len, const int * p1d)
{
  if (p!=0 || len<0) {return false;} //if the "p" is not empty initially, then do nothing and return un-successful
  p = new T [len];
  if (!p) {return false;}
  for (long i=0;i<len;i++)
    p[i] = (T)(p1d[i]);
  return true;	
}

template <class T> bool copy1dMem(T * &p, long len, const unsigned int * p1d)
{
  if (p!=0 || len<0) {return false;} //if the "p" is not empty initially, then do nothing and return un-successful
  p = new T [len];
  if (!p) {return false;}
  for (long i=0;i<len;i++)
    p[i] = (T)(p1d[i]);
  return true;	
}

template <class T> bool copy1dMem(T * &p, long len, const short int * p1d)
{
  if (p!=0 || len<0) {return false;} //if the "p" is not empty initially, then do nothing and return un-successful
  p = new T [len];
  if (!p) {return false;}
  for (long i=0;i<len;i++)
    p[i] = (T)(p1d[i]);
  return true;	
}

template <class T> bool copy1dMem(T * &p, long len, const unsigned short int * p1d)
{
  if (p!=0 || len<0) {return false;} //if the "p" is not empty initially, then do nothing and return un-successful
  p = new T [len];
  if (!p) {return false;}
  for (long i=0;i<len;i++)
    p[i] = (T)(p1d[i]);
  return true;	
}

template <class T> bool copy1dMem(T * &p, long len, const long * p1d)
{
  if (p!=0 || len<0) {return false;} //if the "p" is not empty initially, then do nothing and return un-successful
  p = new T [len];
  if (!p) {return false;}
  for (long i=0;i<len;i++)
    p[i] = (T)(p1d[i]);
  return true;	
}

template <class T> bool copy1dMem(T * &p, long len, const unsigned long * p1d)
{
  if (p!=0 || len<0) {return false;} //if the "p" is not empty initially, then do nothing and return un-successful
  p = new T [len];
  if (!p) {return false;}
  for (long i=0;i<len;i++)
    p[i] = (T)(p1d[i]);
  return true;	
}

template <class T> bool copy1dMem(T * &p, long len, const float * p1d)
{
  if (p!=0 || len<0) {return false;} //if the "p" is not empty initially, then do nothing and return un-successful
  p = new T [len];
  if (!p) {return false;}
  for (long i=0;i<len;i++)
    p[i] = (T)(p1d[i]);
  return true;	
}

template <class T> bool copy1dMem(T * &p, long len, const double * p1d)
{
  if (p!=0 || len<0) {return false;} //if the "p" is not empty initially, then do nothing and return un-successful
  p = new T [len];
  if (!p) {return false;}
  for (long i=0;i<len;i++)
    p[i] = (T)(p1d[i]);
  return true;	
}


/*

template <class T> int new3dpointer(T *** & p, long sz0, long sz1, long sz2, double * p1d)
{
  if (p!=0) {return 0;} //if the "p" is not empty initially, then do nothing and return un-successful
  
  p = new T ** [sz2];
  if (!p) {return 0;}
  
  for (long i=0;i<sz2; i++)
  {
    p[i] = new T * [sz1];
	if (!p[i])
	{
	  for (long j=0;j<i;j++) {delete [] (p[i]);}
	  delete []p; 
	  p=0;
	  return 0;
	}
	else
	{
	  for (long j=0;j<sz1; j++)
//	    p[i][j] = (T *)p1d + i*sz1*sz0*sizeof(T) + j*sz0*sizeof(T);
	    p[i][j] = (T *)p1d + i*sz1*sz0 + j*sz0; //060424
	}
  }
  
  return 1;
}

template <class T> int new3dpointer(T *** & p, long sz0, long sz1, long sz2, float * p1d)
{
  if (p!=0) {return 0;} //if the "p" is not empty initially, then do nothing and return un-successful
  
  p = new T ** [sz2];
  if (!p) {return 0;}
  
  for (long i=0;i<sz2; i++)
  {
    p[i] = new T * [sz1];
	if (!p[i])
	{
	  for (long j=0;j<i;j++) {delete [] (p[i]);}
	  delete []p; 
	  p=0;
	  return 0;
	}
	else
	{
	  for (long j=0;j<sz1; j++)
//	    p[i][j] = (T *)p1d + i*sz1*sz0*sizeof(T) + j*sz0*sizeof(T);
	    p[i][j] = (T *)p1d + i*sz1*sz0 + j*sz0; //060424
	}
  }
  
  return 1;
}



template <class T> int new4dpointer(T **** & p, long sz0, long sz1, long sz2, long sz3, unsigned char * p1d)
{
  if (p!=0) {return 0;} //if the "p" is not empty initially, then do nothing and return un-successful
  
  p = new T *** [sz3];
  if (!p) {return 0;}
  
  for (long i=0;i<sz3; i++)
  {
    p[i] = 0; //this sentence is very important to assure the function below knows this pointer is initialized as empty!!
    //if (!new3dpointer(p[i], sz0, sz1, sz2, p1d+i*sz2*sz1*sz0*sizeof(T))) //070207:if this a bug? guess should not have *sizeof(T)
    if (!new3dpointer(p[i], sz0, sz1, sz2, p1d+i*sz2*sz1*sz0)) //070207:if this a bug? guess should not have *sizeof(T)
	{
	  printf("Problem happened in creating 3D pointers for channel-%d.\n", i);
	  for (long j=0;j<i;j++) {delete [] (p[i]);}
	  delete []p; 
	  p=0;
	  return 0;
	}
  }

  return 1;
}

template <class T> int new4dpointer(T **** & p, long sz0, long sz1, long sz2, long sz3, double * p1d)
{
  if (p!=0) {return 0;} //if the "p" is not empty initially, then do nothing and return un-successful
  
  p = new T *** [sz3];
  if (!p) {return 0;}
  
  for (long i=0;i<sz3; i++)
  {
    p[i] = 0; //this sentence is very important to assure the function below knows this pointer is initialized as empty!!
    if (!new3dpointer(p[i], sz0, sz1, sz2, p1d+i*sz2*sz1*sz0)) //070207: seems correct an error
	{
	  printf("Problem happened in creating 3D pointers for channel-%d.\n", i);
	  for (long j=0;j<i;j++) {delete [] (p[i]);}
	  delete []p; 
	  p=0;
	  return 0;
	}
  }

  return 1;
}

*/