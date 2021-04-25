/* basic_memory.h
   Some basic memory functions for applications.

   by Hanchuan Peng
   2007-02-16: separated from the original "basic_func.h" so that only include the memory functions. Note that 
               the interface functions have been rewritten so that they only have one template function without 
			   type conversion. Also change the return type from "int" to "bool", so that they are clearer.
			   
   2007-02-16: Because I removed all the type conversion in the "new*" functions, an explicit copying function
               from a type such as "unsigned char" to "double" is needed. Thus I added a few often-needed wrappers
			   for these EXPLICIT conversions. These functions are named using "copy1dMem*".
    
*/

#ifndef __BASIC_MEMORY_H__
#define __BASIC_MEMORY_H__

template <class T> bool new2dpointer(T ** & p, long sz0, long sz1, const T * p1d);
template <class T> void delete2dpointer(T ** & p, long sz0, long sz1);

template <class T> bool new3dpointer(T *** & p, long sz0, long sz1, long sz2, const T * p1d);
template <class T> void delete3dpointer(T *** & p, long sz0, long sz1, long sz2);

template <class T> bool new4dpointer(T **** & p, long sz0, long sz1, long sz2, long sz3, const T * p1d);
template <class T> void delete4dpointer(T **** & p, long sz0, long sz1, long sz2, long sz3);

template <class T> bool copy1dMem(T * &p, long len, const char * p1d);
template <class T> bool copy1dMem(T * &p, long len, const unsigned char * p1d);
template <class T> bool copy1dMem(T * &p, long len, const int * p1d);
template <class T> bool copy1dMem(T * &p, long len, const unsigned int * p1d);
template <class T> bool copy1dMem(T * &p, long len, const short int * p1d);
template <class T> bool copy1dMem(T * &p, long len, const unsigned short int * p1d);
template <class T> bool copy1dMem(T * &p, long len, const long * p1d);
template <class T> bool copy1dMem(T * &p, long len, const unsigned long * p1d);
template <class T> bool copy1dMem(T * &p, long len, const float * p1d);
template <class T> bool copy1dMem(T * &p, long len, const double * p1d);


/*
template <class T> int new3dpointer(T *** & p, long sz0, long sz1, long sz2, unsigned char * p1d);
template <class T> int new3dpointer(T *** & p, long sz0, long sz1, long sz2, double * p1d);
template <class T> int new3dpointer(T *** & p, long sz0, long sz1, long sz2, float * p1d);
//template <class T> int new3dpointer(T *** & p, long sz0, long sz1, long sz2, T * p1d);
//template <class T> int new3dpointer(double *** & p, long sz0, long sz1, long sz2, T * p1d);
template <class T> void delete3dpointer(T *** & p, long sz0, long sz1, long sz2);

template <class T> int new4dpointer(T **** & p, long sz0, long sz1, long sz2, long sz3, unsigned char * p1d);
template <class T> int new4dpointer(T **** & p, long sz0, long sz1, long sz2, long sz3, double * p1d);
template <class T> int new4dpointer(T **** & p, long sz0, long sz1, long sz2, long sz3, float * p1d);
//template <class T> int new4dpointer(T **** & p, long sz0, long sz1, long sz2, long sz3, T * p1d);
template <class T> void delete4dpointer(T **** & p, long sz0, long sz1, long sz2, long sz3);
*/

#endif

