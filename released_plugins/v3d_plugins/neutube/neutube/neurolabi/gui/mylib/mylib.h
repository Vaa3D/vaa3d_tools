/*****************************************************************************************\
*                                                                                         *
*  Central library declarations and initialization routine                                *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  January 2007                                                                  *
*                                                                                         *
*  (c) June 19, '09, Dr. Gene Myers and Howard Hughes Medical Institute                   *
*      Copyrighted as per the full copy in the associated 'README' file                   *
*                                                                                         *
\*****************************************************************************************/

#ifndef _MYLIB
#define _MYLIB

#include <stdlib.h>
#include <stdio.h>

#define NUM_THREADS 8    //  number of simultaneously executing threads you can have on
                         //    your machine (not the number of threads you can spawn).
                         //    Typically this equals the number of cores.

#define  M(x) x   //  parameter x is (M)odified by the routine, i.e. input & output
#define  O(x) x   //  parameter x is set by the routine, i.e. (O)utput only

#define  G(x) x   //  function is a (G)enerator.  If function returns a bundle then all objects of
                  //    the bundle are generated (but not the bundle!)
#define  R(x) x   //  parameter x is the (R)eturn value

#define  I(x) x   //  parameter x has its reference count (I)ncremented
#define  C(x) x   //  a reference to parameter x is (C) consumed
#define  S(x) x   //  a reference to parameter x is (S) subsumed, i.e. made a (S)ub-object
#define  F(x) x   //  parameter x is (F)reed by the routine
#define  K(x) x   //  parameter x is (K)illed by the routine

#define  X(x)     //  comment on parameters or effect of routine

#define DIMN_TYPE  INT32_TYPE
#define INDX_TYPE  INT64_TYPE
#define SIZE_TYPE  INT64_TYPE
#define OFFS_TYPE  INT64_TYPE

#define DIMN_SCALE 32
#define INDX_SCALE 64
#define SIZE_SCALE 64
#define OFFS_SCALE 64

namespace mylib {
typedef unsigned char      uint8;
typedef unsigned short     uint16;
typedef unsigned int       uint32;
typedef unsigned long long uint64;
typedef signed char        int8;
typedef signed short       int16;
typedef signed int         int32;
typedef signed long long   int64;
typedef float              float32;
typedef double             float64;

typedef char              *string;     //  Used to emphasize '\0' terminated string
typedef int                boolean;    //  Used to emphasize == 0 is false, != 0 is on
typedef int                tristate;   //  Used to emphasize == 0, > 0, < 0 is what matters

typedef int32 Dimn_Type;   //  Type of an array dimension
typedef int64 Indx_Type;   //  Type of an index into the data space of an array
typedef int64 Size_Type;   //  Type of the size of an array
typedef int64 Offs_Type;   //  Type of an offset within an array

typedef enum
  { UINT8_TYPE   = 0,
    UINT16_TYPE  = 1,
    UINT32_TYPE  = 2,
    UINT64_TYPE  = 3,
    INT8_TYPE    = 4,
    INT16_TYPE   = 5,
    INT32_TYPE   = 6,
    INT64_TYPE   = 7,
    FLOAT32_TYPE = 8,
    FLOAT64_TYPE = 9,
    UNKNOWN_TYPE = 20
  } Value_Type;

typedef enum            //  Operations on array elements
  { SET_OP = 0,         //    See Array_Op_Scalar and Array_Op_Array below
    ADD_OP = 1,
    SUB_OP = 2,
    MUL_OP = 3,
    DIV_OP = 4,
    POW_OP = 5,
    LSH_OP = 6,
    RSH_OP = 7,
    MIN_OP = 8,
    MAX_OP = 9
  } Operator;

typedef enum            //  Comparator ops for defining regions: see Draw_Floodfill in draw.h &
  { LE_COMP = 0,        //     Record_Region in region.h
    LT_COMP = 1,
    EQ_COMP = 2,
    NE_COMP = 3,
    GT_COMP = 4,
    GE_COMP = 5
  } Comparator;


typedef enum            //  The three possible "kinds" of types in a Value
  { UVAL = 0,
    IVAL = 1,
    FVAL = 2
  } Value_Kind;



void Use_Zero_Boundary();
void Use_Reflective_Boundary();
void Use_Wrap_Boundary();
void Use_Extend_Boundary();
void Use_Inversion_Boundary();
}

typedef union
  { mylib::uint64  uval;       //    Use this variant for the 4 unsigned integers types
    mylib::int64   ival;       //    Use this variant for the 4 signed integer types
    mylib::float64 fval;       //    Use this variant for the 2 floating point number types
  } Value;


inline Value VALU( mylib::uint64 u) { Value v; v.uval = u; return v; }
inline Value VALI(  mylib::int64 i) { Value v; v.ival = i; return v; }
inline Value VALF(mylib::float64 f) { Value v; v.fval = f; return v; }

#endif
