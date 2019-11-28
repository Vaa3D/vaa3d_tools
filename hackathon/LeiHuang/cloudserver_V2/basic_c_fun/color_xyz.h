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
 *  color_xyz.h
 *
 * Copyright: Hanchuan Peng, Zongcai Ruan (Howard Hughes Medical Institute, Janelia Farm Research Campus).
 * The License Information and User Agreement should be seen at http://penglab.janelia.org/proj/v3d .
 *
 * Last edit. 2009-Aug-21
 * Last edit. 2010-Jan-19. Move Hanchuan_color_table here
 *
 */

#ifndef __V3D_COLOR_XYZ_H__
#define __V3D_COLOR_XYZ_H__

#include <math.h> // for sqrt(), rand()...
#include <stdlib.h>
#include "c_array_struct.hpp" // CMB 2011 Jan 08

#if defined(_WIN32) || defined(_WIN64)
#undef min
#undef max
#endif

#define BIG_FLOAT 1e+38


///////////////////////////////////////////////////////

#ifndef ABS
#define ABS(a)  ( ((a)>0)? (a) : -(a) )
#endif
#ifndef MIN
#define MIN(a, b)  ( ((a)<(b))? (a) : (b) )
#endif
#ifndef MAX
#define MAX(a, b)  ( ((a)>(b))? (a) : (b) )
#endif
#ifndef ABSMIN
#define ABSMIN(a, b)  ( (ABS(a)<ABS(b))? (a) : (b) )
#endif
#ifndef ABSMAX
#define ABSMAX(a, b)  ( (ABS(a)>ABS(b))? (a) : (b) )
#endif
#ifndef CLAMP
#define CLAMP(a, b, x)  MIN( MAX(MIN(a,b), x), MAX(a,b))
#endif
#ifndef BETWEEN
#define BETWEEN(a, b, x)  (MIN(a,b)<(x) && (x)<MAX(a,b))
#endif
#ifndef BETWEENEQ
#define BETWEENEQ(a, b, x)  (MIN(a,b)<=(x) && (x)<=MAX(a,b))
#endif
#ifndef IROUND
#define IROUND(x)  int(x + .5)
#endif
inline int power_of_two_ceil(int x)  { int y; for (y = 1; y < x;  y = y*2) {} return y; }
inline int power_of_two_floor(int x) { int y; for (y = 1; y <= x; y = y*2) {} return y/2; }

////////////////////////////////////////////////////////

struct RGB8 { union {
	struct { unsigned char r,g,b; };
    c_array<unsigned char, 3> c;
	// unsigned char c[3];
};};
struct RGBA8 {
    union
    {
        struct { unsigned char r,g,b,a; };
        c_array<unsigned char, 4> c;
        unsigned int i;
    };
};
struct BGR8 { union { // Windows-DIB
	struct { unsigned char b,g,r; };
	c_array<unsigned char, 3> c;
};};
struct BGRA8 { union { // Windows-DIB  // Qt-QRgb
	struct { unsigned char b,g,r,a; };
	c_array<unsigned char, 4> c;
	unsigned int i;
};};
struct ABGR8 { union { // little endian
	struct { unsigned char a,b,g,r; };
	c_array<unsigned char, 4> c;
	unsigned int i;
};};

struct RGB16i { union {
	struct { short r,g,b; };
	c_array<short, 3> c;
};};
struct RGBA16i { union {
	struct { short r,g,b,a; };
	c_array<short, 4> c;
};};
struct RGB32i { union {
	struct { int r,g,b; };
	c_array<int, 3> c;
};};
struct RGBA32i { union {
	struct { int r,g,b,a; };
	c_array<int, 4> c;
};};
struct RGB32f { union {
	struct { float r,g,b; };
	c_array<float, 3> c;
};};
struct RGBA32f { union {
	struct { float r,g,b,a; };
	c_array<float, 4> c;
};};

RGB8 random_rgb8(); // use normalize(XYZ)
RGBA8 random_rgba8();
RGBA8 random_rgba8(unsigned char a);

inline RGBA8 rgba8_from(ABGR8 d) {
	RGBA8 c;	c.r=d.r; c.g=d.g; c.b=d.b;	c.a=d.a;	return c;
}
inline RGBA8 rgba8_from(BGRA8 d) {
	RGBA8 c;	c.r=d.r; c.g=d.g; c.b=d.b;	c.a=d.a;	return c;
}

inline ABGR8 abgr8_from(RGBA8 c) {
	ABGR8 d;	d.r=c.r; d.g=c.g; d.b=c.b;	d.a=c.a;	return d;
}
inline BGRA8 bgra8_from(RGBA8 c) {
	BGRA8 d;	d.r=c.r; d.g=c.g; d.b=c.b;	d.a=c.a;	return d;
}


/////////////////////////////////////////////////////////////
//struct XYZ;
//struct XYZW;

struct XYZ {
	union {
	struct {float x, y, z;};
	c_array<float, 3> v;
	};

        XYZ(float px, float py, float pz)	{this->x=px; this->y=py; this->z=pz;}
	XYZ(float a=0)	{x=a; y=a; z=a;}
	XYZ(RGB8 c)	    {x=c.r; y=c.g; z=c.b;}
	XYZ(RGB16i c)	{x=c.r; y=c.g; z=c.b;}
	XYZ(RGB32i c)	{x=c.r; y=c.g; z=c.b;}
	XYZ(RGB32f c)	{x=c.r; y=c.g; z=c.b;}
	operator RGB8()   {RGB8 c; c.r=(unsigned char)x; c.g=(unsigned char)y; c.b=(unsigned char)z; return c;}
	operator RGB16i() {RGB16i c; c.r=(short)x; c.g=(short)y; c.b=(short)z; return c;}
	operator RGB32i() {RGB32i c; c.r=(int)x; c.g=(int)y; c.b=(int)z; return c;}
	operator RGB32f() {RGB32f c; c.r=x; c.g=y; c.b=z; return c;}
};

struct XYZW {
	union {
	struct {float x, y, z, w;};
	float v[4];
	};

        XYZW(float px, float py, float pz, float pw)	{this->x=px, this->y=py; this->z=pz; this->w=pw;}
	XYZW(float a=0)	{x=a; y=a; z=a; w=a;}
	XYZW(RGBA8 c)	{x=c.r; y=c.g; z=c.b; w=c.a;}
	XYZW(RGBA16i c)	{x=c.r; y=c.g; z=c.b; w=c.a;}
	XYZW(RGBA32i c)	{x=c.r; y=c.g; z=c.b; w=c.a;}
	XYZW(RGBA32f c)	{x=c.r; y=c.g; z=c.b; w=c.a;}
	operator RGBA8()   {RGBA8 c; c.r=(unsigned char)x; c.g=(unsigned char)y; c.b=(unsigned char)z; c.a=(unsigned char)w; return c;}
	operator RGBA16i() {RGBA16i c; c.r=(short)x; c.g=(short)y; c.b=(short)z; c.a=(short)w; return c;}
	operator RGBA32i() {RGBA32i c; c.r=(int)x; c.g=(int)y; c.b=(int)z; c.a=(int)w; return c;}
	operator RGBA32f() {RGBA32f c; c.r=x; c.g=y; c.b=z; c.a=w; return c;}
        XYZW(XYZ pv, float pw=1) {x=pv.x; y=pv.y; z=pv.z; this->w=pw;}
        operator XYZ()   {XYZ pv; pv.x=x; pv.y=y; pv.z=z; return pv;}
};

/////////////////////////////////////////////////////////////
//#define NULL_BoundingBox BoundingBox(XYZ(BIG_FLOAT), XYZ(-BIG_FLOAT))
#define NULL_BoundingBox BoundingBox(XYZ(1), XYZ(-1))
#define UNIT_BoundingBox BoundingBox(XYZ(0), XYZ(1))

struct BoundingBox {
	union {
	struct {float x0, y0, z0;	float x1, y1, z1;};
	float box[6];
	};

	BoundingBox() 					{ *this = NULL_BoundingBox; }
	BoundingBox(float a) 			{x0=y0=z0=x1=y1=z1 = a;}
        BoundingBox(float px0, float py0, float pz0, float px1, float py1, float pz1)
                             {x0=px0; y0=py0; z0=pz0;  x1=px1; y1=py1; z1=pz1;}
        BoundingBox(XYZ pV0, XYZ pV1)		{x0=pV0.x; y0=pV0.y; z0=pV0.z;  x1=pV1.x; y1=pV1.y; z1=pV1.z;}
        float Dx() const		{return (x1-x0); }
        float Dy() const		{return (y1-y0); }
        float Dz() const		{return (z1-z0); }
        float Dmin() const	{return MIN(MIN(Dx(),Dy()),Dz());}
        float Dmax() const	{return MAX(MAX(Dx(),Dy()),Dz());}
        XYZ V0() const	 	{return XYZ(x0,y0,z0);}
        XYZ V1() const	 	{return XYZ(x1,y1,z1);}
        XYZ Vabsmin() const	{return XYZ(ABSMIN(x0,x1), ABSMIN(y0,y1), ABSMIN(z0,z1));}
        XYZ Vabsmax() const	{return XYZ(ABSMAX(x0,x1), ABSMAX(y0,y1), ABSMAX(z0,z1));}
        bool isNegtive()	const 	{return (Dx()<0 || Dy()<0 || Dz()<0);}
        bool isInner(XYZ V, float d=0) const	{
		return BETWEENEQ(x0-d,x1+d, V.x) && BETWEENEQ(y0-d,y1+d, V.y) && BETWEENEQ(z0-d,z1+d, V.z);
	}
	void clamp(XYZ & V) {
		V.x = CLAMP(x0, x1, V.x); V.y = CLAMP(y0, y1, V.y); V.z = CLAMP(z0, z1, V.z);
	}
	void expand(XYZ V) {
		if (Dx()<0) { x0 = x1 = V.x;} else { x0=MIN(x0, V.x); x1=MAX(x1, V.x); }
		if (Dy()<0) { y0 = y1 = V.y;} else { y0=MIN(y0, V.y); y1=MAX(y1, V.y); }
		if (Dz()<0) { z0 = z1 = V.z;} else { z0=MIN(z0, V.z); z1=MAX(z1, V.z); }
	}
	void expand(BoundingBox B) {
		if (B.isNegtive()) return;
		expand(B.V0()); expand(B.V1());
	}
	void shift(float x, float y, float z) 	{x0+=x; y0+=y; z0+=z;	x1+=x; y1+=y; z1+=z;}
	void shift(XYZ S) 						{shift(S.x, S.y, S.z);}
};


/////////////////////////////////////////////////////////////////
// operators of XYZ & XYZW & BoundingBox
/////////////////////////////////////////////////////////////////

inline bool operator == (const BoundingBox& A, const BoundingBox& B)
{
	return (A.x0==B.x0 && A.y0==B.y0 && A.z0==B.z0 && A.x1==B.x1 && A.y1==B.y1 && A.z1==B.z1);
}

//XYZW ----------------------------------------
inline bool operator == (const XYZW& a, const XYZW& b)
{
	return (a.x==b.x && a.y==b.y && a.z==b.z && a.w==b.w);
}

inline XYZW operator + (const XYZW& a, const XYZW& b)
{
	XYZW c;	c.x = a.x+b.x;	c.y = a.y+b.y;	c.z = a.z+b.z;	c.w = a.w+b.w;  return c;
}

inline XYZW operator - (const XYZW& a, const XYZW& b)
{
	XYZW c;	c.x = a.x-b.x;	c.y = a.y-b.y;	c.z = a.z-b.z;	c.w = a.w-b.w;  return c;
}

inline XYZW operator * (const XYZW& a, const XYZW& b)
{
	XYZW c;	c.x = a.x*b.x;	c.y = a.y*b.y;	c.z = a.z*b.z;	c.w = a.w*b.w;  return c;
}

inline XYZW operator / (const XYZW& a, const XYZW& b)
{
	XYZW c;	c.x = a.x/b.x;	c.y = a.y/b.y;	c.z = a.z/b.z;	c.w = a.w/b.w;  return c;
}

//XYZ -----------------------------------------
inline bool operator == (const XYZ& a, const XYZ& b)
{
	return (a.x==b.x && a.y==b.y && a.z==b.z);
}

inline XYZ operator + (const XYZ& a, const XYZ& b)
{
	XYZ c;	c.x = a.x+b.x;	c.y = a.y+b.y;	c.z = a.z+b.z;	return c;
}

inline XYZ operator - (const XYZ& a, const XYZ& b)
{
	XYZ c;	c.x = a.x-b.x;	c.y = a.y-b.y;	c.z = a.z-b.z;	return c;
}

inline XYZ operator * (const XYZ& a, const XYZ& b)
{
	XYZ c;	c.x = a.x*b.x;	c.y = a.y*b.y;	c.z = a.z*b.z;	return c;
}

inline XYZ operator / (const XYZ& a, const XYZ& b)
{
	XYZ c;	c.x = a.x/b.x;	c.y = a.y/b.y;	c.z = a.z/b.z;	return c;
}

inline float dot(const XYZ& a, const XYZ& b)
{
	return (a.x*b.x + a.y*b.y + a.z*b.z);
}

inline XYZ cross(const XYZ& a, const XYZ& b)
{
	XYZ c;
	c.x = a.y*b.z - a.z*b.y;
	c.y = a.z*b.x - a.x*b.z;
	c.z = a.x*b.y - a.y*b.x;
	return c;
}

inline float norm(const XYZ& a)
{
	//return sqrt(a.x*a.x + a.y*a.y +a.z*a.z);
	return sqrt(dot(a,a));
}

inline float dist_L2(const XYZ& a, const XYZ& b)
{
	XYZ c(a.x-b.x, a.y-b.y, a.z-b.z);
	return sqrt(dot(c,c));
}

inline XYZ& normalize(XYZ& a)
{
	//float m = sqrt(a.x*a.x + a.y*a.y +a.z*a.z);
	float m = norm(a);
	if (m>0) {a.x /= m;		a.y /= m;		a.z /= m;}
	return a;
}

inline float min(const XYZ& a)
{
	return MIN(a.x, MIN(a.y, a.z));
}

inline float max(const XYZ& a)
{
	return MAX(a.x, MAX(a.y, a.z));
}

//random color -----------------------------------------------
inline RGB8 random_rgb8()
{
	XYZ d (rand()%255, rand()%255, rand()%255);
	normalize(d);
	RGB8 c;
	c.r=(unsigned char)(d.x*255);
	c.g=(unsigned char)(d.y*255);
	c.b=(unsigned char)(d.z*255);
	return c;
}

inline RGBA8 random_rgba8()
{
	RGB8 c=random_rgb8();
	RGBA8 cc;
	cc.r=c.r; cc.g=c.g; cc.b=c.b; cc.a=rand()%255;
	return cc;
}  // Caution: a==rand, 081114,0812210

inline RGBA8 random_rgba8(unsigned char a)
{
	RGB8 c=random_rgb8();
	RGBA8 cc;
	cc.r=c.r; cc.g=c.g; cc.b=c.b; cc.a=(a);
	return cc;
}

static unsigned char hanchuan_colortable[][3]={
{255,   0,    0},
{  0, 255,    0},
{  0,   0,  255},
{255, 255,    0},
{  0, 255,  255},
{255,   0,  255},
{255, 128,    0},
{  0, 255,  128},
{128,   0,  255},
{128, 255,    0},
{  0, 128,  255},
{255,   0,  128},
{128,   0,    0},
{  0, 128,    0},
{  0,   0,  128},
{128, 128,    0},
{  0, 128,  128},
{128,   0,  128},
{255, 128,  128},
{128, 255,  128},
{128, 128,  255},
};
inline int hanchuan_colortable_size() {return sizeof(hanchuan_colortable)/3;}

////////////////////////////////////////////////////////////////////

#endif

