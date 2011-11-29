/*************************************************************************
 * From /picb/pattern/PROG/SYNTH_IMP/palette*
 * Small changed including: 
 * 1. define CHAR_BIT, 
 * 2. delete png related info
 * 3. palette -> Palette
**************************************************************************/
#include "palette.h"

#include <iostream>
#include <vector>
#include <assert.h>

#define CHAR_BIT 8

// =====================================================
rgb_pixel::rgb_pixel(unsigned char r_, unsigned char g_, unsigned char b_)
// =====================================================
{
	r=r_; g=g_; b=b_; 
}

// =====================================================
rgb_pixel::rgb_pixel(unsigned int rgb)
// =====================================================
{
	r=rgb%256;
	rgb/=256;
	g=rgb%256;
	rgb/=256;
	b=rgb%256;
}

// =====================================================
rgb_pixel::rgb_pixel()
// =====================================================
{
}

// =====================================================
bool rgb_pixel::operator==(const rgb_pixel& P)
// =====================================================
{
	return (r==P.r && g==P.g && b==P.b);
}

// =====================================================
bool rgb_pixel::operator<(const rgb_pixel& P) const
// =====================================================
{
	// return < according to lexicographical order by (r,g,b)
	if (r<P.r)
		return true;
	if (r>P.r)
		return false;
	// r==P.r
	if (g<P.g)
		return true;
	if (g>P.g)
		return false;
	// r==P.r && g==P.g
	if (b<P.b)
		return true;
	return false;
}

// =====================================================
bool rgb_pixel::operator!=(const rgb_pixel& P)
// =====================================================
{
	return (r!=P.r || g!=P.g || b!=P.b);
}

// =====================================================
Palette::int_set& Palette::get_all_colors()
// =====================================================
{
	return all_colors;
}

// =====================================================
void Palette::init(int size)
// =====================================================
{
	all_colors.clear();
	palette_size = size;
	this_palette.resize(palette_size);
	
	int i;
	for (i=0; i<palette_size; ++i)
	{
		all_colors.insert(i);
		// start from 1 to avoid black as a palette color
		int j=i+1;
		unsigned char r=0;
		unsigned char g=0;
		unsigned char b=0;
		int bit=sizeof(unsigned char)*CHAR_BIT-1;
		for (; j>0; bit--)
		{
			// lowest bit should be unused
			assert(bit>0);
			unsigned char add=1<<bit;
			if (j%2)
				r+=add;
			j/=2;
			if (j%2)
				g+=add;
			j/=2;
			if (j%2)
				b+=add;
			j/=2;
		}
		
		if (r==0)
			r=1;
		rgb_pixel col_i=rgb_pixel(r,g,b);
		this_palette[i]=col_i;
		//std::cout<<"palette["<<i<<"]=("<<(int)col_i.r<<","<<(int)col_i.g<<","<<(int)col_i.b<<")\n";
		reverse_palette[col_i]=i;
		//std::cerr<<"> "<<reverse_palette[col_i]<<"\n";
	}
}

// =====================================================
Palette::Palette()
// =====================================================
{
	init(256);
}

// =====================================================
Palette::Palette(int size)
// =====================================================
{
	init(size);
}

// =====================================================
int Palette::operator()(const rgb_pixel& p)
// =====================================================
{
	if (reverse_palette.find(p)==reverse_palette.end())
		return -1;
	return reverse_palette[p];
}

// =====================================================
rgb_pixel Palette::operator()(int i)
// =====================================================
{
	assert(i<size());
	return this_palette[i];
}

// =====================================================
int Palette::size()
// =====================================================
{
	return palette_size;
}

