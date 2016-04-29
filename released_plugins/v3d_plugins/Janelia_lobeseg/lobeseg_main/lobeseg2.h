//lobeseg.h
//created by Hanchuan Peng
//080822
//Last update by BL 080826 separate mask generating function from do_lobeseg_bdbminus(), add gen_lobeseg_mask()
//Last update Aug 28, 2008 add Ting's shortest path function do_shortestpath()
//Last update by BL Aug 18, 2008 add do_shortestpath_xz and do_lobeseg_bdbminus3()
//Last update by BLAug 19, 2008 add do_shortestpath_cm(), shortest path with center of mass

#ifndef __LOBESEG2__H__
#define __LOBESEG2__H__

#include <vector>
#include "../worm_straighten_c/bdb_minus.h"

class BDB_Minus_ConfigParameter;

bool do_lobeseg_bdbminus(unsigned char *inimg1d, const V3DLONG sz[4], int in_channel_no, const BDB_Minus_ConfigParameter & mypara, vector<vector<Coord2D> > &vl, vector<vector<Coord2D> > &vr);
bool gen_lobeseg_mask(const V3DLONG sz[4], unsigned char *outimg1d, int out_channel_no, vector<vector<Coord2D> > &vl, vector<vector<Coord2D> > &vr, int *left_bound1d, int *right_bound1d);
bool do_shortestpath(unsigned char *inimg1d, const V3DLONG sz[4], int in_channel_no, int out_channel_no);
bool do_shortestpath_cm(unsigned char *inimg1d, const V3DLONG sz[4], int in_channel_no, int out_channel_no);
bool do_shortestpath_xz(unsigned char *inimg1d, const V3DLONG sz[4], unsigned char *outimg1d, int in_channel_no, int out_channel_no, vector<int> &pp_left, vector<int> &pp_right, vector<int> &left_zz, vector<int> &right_zz);
bool do_lobeseg_bdbminus2(unsigned char *inimg1d, const V3DLONG sz[4], int in_channel_no, const BDB_Minus_ConfigParameter & mypara, vector<vector<Coord2D> > &vl_high, vector<vector<Coord2D> > &vr_high, vector<vector<Coord2D> > &vl_low, vector<vector<Coord2D> > &vr_low, vector<int> a_middle_left, vector<int> a_middle_right);
bool gen_lobeseg_mask2(const V3DLONG sz[4], unsigned char *outimg1d, int out_channel_no, vector<vector<Coord2D> > &vl_high, vector<vector<Coord2D> > &vr_high, 
vector<vector<Coord2D> > &vl_low, vector<vector<Coord2D> > &vr_low, int *left_bound1d, int *right_bound1d);
bool do_lobeseg_bdbminus3(unsigned char *inimg1d, const V3DLONG sz[4], int in_channel_no, const BDB_Minus_ConfigParameter & mypara, vector<vector<Coord2D> > &vl_low, 
vector<vector<Coord2D> > &vr_low, vector<int> a_middle_left, vector<int> a_middle_right);

#endif


