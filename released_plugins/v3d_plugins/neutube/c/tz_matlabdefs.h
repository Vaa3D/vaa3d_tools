/**@file tz_matlabdefs.h
 * @brief definitions for Matlab
 * @author Ting Zhao
 * @date 23-Nov-2007
 */

#ifndef _TZ_MATLABDEFS_H_
#define _TZ_MATLABDEFS_H_

#define miINT8 1  /* 8 bit, signed */
#define miUINT8 2 /* 8 bit, unsigned */
#define miINT16 3 /* 16-bit, signed */
#define miUINT16 4 /* 16-bit, unsigned */
#define miINT32 5 /* 32-bit, signed */
#define miUINT32 6 /* 32-bit, unsigned */
#define miSINGLE 7 /* IEEE 754 single format */
#define miDOUBLE 9 /* IEEE 754 double format */
#define miINT64 12 /* 64-bit, signed */
#define miUINT64 13 /* 64-bit, unsigned */
#define miMATRIX 14 /* MATLAB array */
#define miCOMPRESSED 15 //Compressed Data
#define miUTF8 16 //Unicode UTF-8 Encoded Character Data
#define miUTF16 17 //Unicode UTF-16 Encoded Character Data
#define miUTF32 18 //Unicode UTF-32 Encoded Character Data

#define mxCELL_CLASS 1 //Cell array
#define mxSTRUCT_CLASS 2 //Structure
#define mxOBJECT_CLASS 3 //Oject
#define mxCHAR_CLASS 4 //Character array
#define mxSPARSE_CLASS 5 //Sparse array
#define mxDOUBLE_CLASS 6 //Double precision array
#define mxSINGLE_CLASS 7 //Single precision array
#define mxINT8_CLASS 8 //8-bit, signed integer
#define mxUINT8_CLASS 9 //8-bit, unsigned integer
#define mxINT16_CLASS 10 //16-bit, signed integer
#define mxUINT16_CLASS 11 //16-bit, unsigned integer
#define mxINT32_CLASS 12 //32-bit, signed integer
#define mxUINT32_CLASS 13 //32-bit, unsigned integer

#endif 
