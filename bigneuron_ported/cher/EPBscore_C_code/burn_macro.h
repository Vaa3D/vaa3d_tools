/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */

#define MA_MARK   -11
#define	RSEALED1  -12
#define	RSEALED2  -13
#define	BSEALED1  -14
#define	BSEALED2  -15
#define	RRSEALED  -16
#define	BBSEALED  -17
#define	RRSEALED1 -18
#define	RRSEALED2 -19
#define	BBSEALED1 -20
#define	BBSEALED2 -21

#define UNBURNED    9
#define	BURNED	   10

#define MS	   98
#define MA	   99
#define MA_OFF_MS 100
#define MA_TRIM   101

#define	BARRIER	  252
#define	GRN_BDRY  253
#define	GRAIN	  254
#define	EXT_VAL	  255

/*
*	variables for boundary identification
*/

#define	XL_SIDE	 1	/* voxel borders ix = 0    side */
#define	XU_SIDE	 2	/* voxel borders ix = nx-1 side */
#define	YL_SIDE	 4	/* voxel borders iy = 0    side */
#define	YU_SIDE	 8	/* voxel borders iy = ny-1 side */
#define	ZL_SIDE	16	/* voxel borders iz = 0    side */
#define	ZU_SIDE	32	/* voxel borders iz = nz-1 side */
#define	FP_SIDE 64	/* voxel borders fiducial polygon (in x-y plane) */


#define	Set_unburned(_phase_num)\
	for( i = 0;  i < nxyz;  i++ )\
	{\
		if( cube[i] == _phase_num ) cube[i] = UNBURNED;\
	}


#define Set_icube(js,val)\
{\
	int	j, b_count;\
\
	b_count = 0;\
	j = js;\
	while( (j < nxyz) && (b_count < burnt_voxels) )\
	{\
		if( (cube[j] ==   BURNED ) || (cube[j] ==  RSEALED1) ||\
		    (cube[j] ==  RSEALED2) || (cube[j] == RRSEALED ) ||\
		    (cube[j] == RRSEALED1) || (cube[j] == RRSEALED2)  )\
		{\
			cube[j] = val;\
			b_count++;\
		}\
		else if ( (cube[j] ==  BSEALED1) || (cube[j] ==  BSEALED2) ||\
			  (cube[j] == BBSEALED ) || (cube[j] == BBSEALED1) ||\
			  (cube[j] == BBSEALED2) )\
  		{\
			cube[j] = BARRIER;\
			b_count++;\
		}\
		j++;\
	}\
}

#define Set_cube(js,val)\
{\
	int	j, b_count;\
\
	b_count = 0;\
	j = js;\
	while( (j < nxyz) && (b_count < burnt_voxels) )\
	{\
		if( cube[j] ==   BURNED )\
		{\
			cube[j] = val;\
			b_count++;\
		}\
	   j++;\
	}\
}
