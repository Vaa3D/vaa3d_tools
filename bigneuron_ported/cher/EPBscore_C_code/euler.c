/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */

/*
 *	This is Table 2 from
 *
 *	Lee, Kashyap and Chu,
 *	`3-d medial surface/axis thinning algorithms',
 *	CVGIP: Graphical Models and Image Processing, 56 (1994) 462-478.
 */

#include "stdafx.h"


static	char	lookup[128];

void	init_deuler_table()
{
	lookup[  0] =  1;
	lookup[  1] = -1;
	lookup[  2] = -1;
	lookup[  3] =  1;
	lookup[  4] = -3;
	lookup[  5] = -1;
	lookup[  6] = -1;
	lookup[  7] =  1;
	lookup[  8] = -1;
	lookup[  9] =  1;

	lookup[ 10] =  1;
	lookup[ 11] = -1;
	lookup[ 12] =  3;
	lookup[ 13] =  1;
	lookup[ 14] =  1;
	lookup[ 15] = -1;
	lookup[ 16] = -3;
	lookup[ 17] = -1;
	lookup[ 18] =  3;
	lookup[ 19] =  1;

	lookup[ 20] =  1;
	lookup[ 21] = -1;
	lookup[ 22] =  3;
	lookup[ 23] =  1;
	lookup[ 24] = -1;
	lookup[ 25] =  1;
	lookup[ 26] =  1;
	lookup[ 27] = -1;
	lookup[ 28] =  3;
	lookup[ 29] =  1;

	lookup[ 30] =  1;
	lookup[ 31] = -1;
	lookup[ 32] = -3;
	lookup[ 33] =  3;
	lookup[ 34] = -1;
	lookup[ 35] =  1;
	lookup[ 36] =  1;
	lookup[ 37] =  3;
	lookup[ 38] = -1;
	lookup[ 39] =  1;

	lookup[ 40] = -1;
	lookup[ 41] =  1;
	lookup[ 42] =  1;
	lookup[ 43] = -1;
	lookup[ 44] =  3;
	lookup[ 45] =  1;
	lookup[ 46] =  1;
	lookup[ 47] = -1;
	lookup[ 48] =  1;
	lookup[ 49] =  3;

	lookup[ 50] =  3;
	lookup[ 51] =  1;
	lookup[ 52] =  5;
	lookup[ 53] =  3;
	lookup[ 54] =  3;
	lookup[ 55] =  1;
	lookup[ 56] = -1;
	lookup[ 57] =  1;
	lookup[ 58] =  1;
	lookup[ 59] = -1;

	lookup[ 60] =  3;
	lookup[ 61] =  1;
	lookup[ 62] =  1;
	lookup[ 63] = -1;
	lookup[ 64] = -7;
	lookup[ 65] = -1;
	lookup[ 66] = -1;
	lookup[ 67] =  1;
	lookup[ 68] = -3;
	lookup[ 69] = -1;

	lookup[ 70] = -1;
	lookup[ 71] =  1;
	lookup[ 72] = -1;
	lookup[ 73] =  1;
	lookup[ 74] =  1;
	lookup[ 75] = -1;
	lookup[ 76] =  3;
	lookup[ 77] =  1;
	lookup[ 78] =  1;
	lookup[ 79] = -1;

	lookup[ 80] = -3;
	lookup[ 81] = -1;
	lookup[ 82] =  3;
	lookup[ 83] =  1;
	lookup[ 84] =  1;
	lookup[ 85] = -1;
	lookup[ 86] =  3;
	lookup[ 87] =  1;
	lookup[ 88] = -1;
	lookup[ 89] =  1;

	lookup[ 90] =  1;
	lookup[ 91] = -1;
	lookup[ 92] =  3;
	lookup[ 93] =  1;
	lookup[ 94] =  1;
	lookup[ 95] = -1;
	lookup[ 96] = -3;
	lookup[ 97] =  3;
	lookup[ 98] = -1;
	lookup[ 99] =  1;

	lookup[100] =  1;
	lookup[101] =  3;
	lookup[102] = -1;
	lookup[103] =  1;
	lookup[104] = -1;
	lookup[105] =  1;
	lookup[106] =  1;
	lookup[107] = -1;
	lookup[108] =  3;
	lookup[109] =  1;

	lookup[110] =  1;
	lookup[111] = -1;
	lookup[112] =  1;
	lookup[113] =  3;
	lookup[114] =  3;
	lookup[115] =  1;
	lookup[116] =  5;
	lookup[117] =  3;
	lookup[118] =  3;
	lookup[119] =  1;

	lookup[120] = -1;
	lookup[121] =  1;
	lookup[122] =  1;
	lookup[123] = -1;
	lookup[124] =  3;
	lookup[125] =  1;
	lookup[126] =  1;
	lookup[127] = -1;
}


#define Set_decim(_I1,_I2,_I3,_I4,_I5,_I6,_I7)\
	decim = 64 * nodeval[_I1] + 32 * nodeval[_I2] + 16 * nodeval[_I3]\
	      +  8 * nodeval[_I4] +  4 * nodeval[_I5] +  2 * nodeval[_I6]\
	      +      nodeval[_I7]


#define Set_deuler(_nd)\
{\
	Set_decim( 0, 3, 9,12, 1, 4,10);	deuler += lookup[_nd];\
	Set_decim( 6, 7,14,15, 3, 4,12);	deuler += lookup[_nd];\
	Set_decim( 8, 5,16,13, 7, 4,15);	deuler += lookup[_nd];\
	Set_decim( 2, 1,11,10, 5, 4,13);	deuler += lookup[_nd];\
	Set_decim(17,20, 9,12,18,21,10);	deuler += lookup[_nd];\
	Set_decim(23,24,14,15,20,21,12);	deuler += lookup[_nd];\
	Set_decim(25,22,16,13,24,21,15);	deuler += lookup[_nd];\
	Set_decim(19,18,11,10,22,21,13);	deuler += lookup[_nd];\
}

	
char	calc_deuler(unsigned char *nodeval, char conn_type)
{
	int	decim;
	char	deuler;
  
	deuler = 0;
	     if( conn_type == 26 ) Set_deuler(decim)
	else if( conn_type ==  6 ) Set_deuler(127-decim)

	return deuler;
}
