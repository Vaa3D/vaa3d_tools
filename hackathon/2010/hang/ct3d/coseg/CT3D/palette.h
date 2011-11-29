//
//=======================================================================
// Copyright 2010 Institute PICB.
// Authors: Hang Xiao, Axel Mosig
// Data : July 14, 2010
//=======================================================================
//

#ifndef PALETTE_H_H
#define PALETTE_H_H
#include <vector>
#include <set>
#include <map>

using namespace std;

// =====================================================
class rgb_pixel
// =====================================================
{

	public:

		unsigned char r,g,b;

	public:

		rgb_pixel(unsigned char r_, unsigned char g_, unsigned char b_);

		rgb_pixel(unsigned int rgb);

		rgb_pixel();

		bool operator==(const rgb_pixel& P);

		bool operator<(const rgb_pixel& P) const;

		bool operator!=(const rgb_pixel& P);

};
// =====================================================
// =====================================================



// =====================================================
class Palette
// =====================================================
{

	public:

		typedef set<int> int_set;

	private:

		typedef map<rgb_pixel,int> reverse_palette_map;

		int palette_size;

		int_set all_colors;

		vector<rgb_pixel> this_palette;
		reverse_palette_map reverse_palette;

	public:

		int_set& get_all_colors();

		Palette();

		Palette(int size);

		int size();

		rgb_pixel operator()(int i);

		int operator()(const rgb_pixel& p);

		void init(int size);

};
// =====================================================

#endif
