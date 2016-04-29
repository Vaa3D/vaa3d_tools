//lobeseg.h
//created by Hanchuan Peng
//080822
//110525
//110709: add datatype_input

#ifndef __LOBESEG__H__
#define __LOBESEG__H__

class BDB_Minus_ConfigParameter;
bool do_lobeseg_bdbminus(unsigned char *inimg1d, const V3DLONG sz[4], int datatype_input, unsigned char *outimg1d, int in_channel_no, int out_channel_no, const BDB_Minus_ConfigParameter & mypara);


//bool do_lobeseg_bdbminus_onesideonly(unsigned char *inimg1d, const V3DLONG sz[4], unsigned char *outimg1d, int in_channel_no, int out_channel_no, const BDB_Minus_ConfigParameter & mypara);

bool do_lobeseg_bdbminus_onesideonly(unsigned char *inimg1d, const V3DLONG sz[4], int datatype_input, unsigned char *outimg1d, int in_channel_no, int out_channel_no, const BDB_Minus_ConfigParameter & mypara, int ini_x1, int ini_y1, int ini_x2, int ini_y2, int keep_which, int num_ctrls, bool output_surface);

#endif


