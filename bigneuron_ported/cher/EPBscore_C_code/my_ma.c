/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */

#include "stdafx.h"


#include <string.h>
#include <math.h>
#include "isoseg.h"
#include "burn_macro.h"
#include "arch.h"
#include "ma_prototype.h"
#include "burn_prototype.h"

mxArray * mexburnmain(const mxArray *input)
{
	Bchains bc; 
	Fdat tmp;
	mxArray * input_cube; 
	mxArray * burn_cube;
	mxArray * fout;
	mwSize nx,ny,nz,number_of_dims;
	const mwSize * pind;

	input_cube = mxGetCell(input,0);
	number_of_dims = mxGetNumberOfDimensions(input_cube);
	if (number_of_dims==2)
	{
		 pind = mxGetDimensions(input_cube);
		 nx=pind[0];
		 ny=pind[1];
		 nz=1;
	}
		else if (number_of_dims==3)
		{
		 pind = mxGetDimensions(input_cube);
		 nx=pind[0];
		 ny=pind[1];
		 nz=pind[2];
		}
		else
		mexErrMsgTxt("Input must be 2 or 3 dimensional");
	if (mxIsUint8(input_cube) == 0 )
		mexErrMsgTxt("Input must be uint8 type.");

	burnmain(mxGetData(input_cube), &bc, &tmp,nx,ny, nz);

	burn_cube = mxCreateNumericArray(number_of_dims, pind, mxUINT8_CLASS, mxREAL);
	mxSetData(burn_cube,tmp.bfdat);
	fout = mxCreateCellMatrix(1, 1);
	mxSetCell(fout,0,burn_cube);
	return fout;
}

void	burnmain(
			unsigned char *seg_cube, 
			Bchains *bc, 
			Fdat *tmp,
			int nx,
			int ny, 
			int nz
			)
{

/* Fdat strcture	
	unsigned char  *temp;	  temporary storage for one slice of data 
	unsigned char  *fdat;	  storage for volume of data 
	unsigned char  *bfdat;	  burn directions 
	unsigned char  *cube;	  work space 

  BChains struct
   	char	*cnt;		 controls fire direction 
	int	*o_chain,	 step n   burn chain (i,j,k) 
		*n_chain;	 step n+1 burn chain (i,j,k) 
	int	o_num,		 length of o_chain 
		n_num;		 length of n_chain 
	int	chain_size;	 chain length allocated 
*/

	unsigned char	*pfdat;

	int	NX, NY, NXY, NXYZ;
	int	i, g_num;
	int	mbn, period;
	int	dbg = 0;
	int task;
	int sur = 0;	/* voxel face count for specific surface area */
	
	int	csz = sizeof(char);
	int	isz = sizeof(int);
	int	usz = sizeof(unsigned char);

	NX = nx;        NY = ny;        NXY = nx*ny;    NXYZ = NXY*nz;

	tmp->fdat= seg_cube;
	pfdat = tmp->fdat;
	task=ERODE;
	
	tmp->cube= (unsigned char*)MALLOC(NXYZ*usz);
	g_num = 0;
	create_gbdry(bc,tmp,&g_num,nx,ny,nz,task,&sur);
	
   
		/**********************************
	    Create a direction file as bfdat[] as same as fdat[].
	    Now setting tmpdata.bfdat[] data to be 0 where is pore pixel.
	    Then, use this data file to note the direction value with
	    indicating the pixel is MA or not.
	    **********************************/

    tmp->bfdat = (unsigned char *)CALLOC(NXYZ,usz);

	/* reset cube array to 0 */
    for( i = 0;  i < NXYZ;  i++ ) tmp->cube[i] = 0;

    dir_set(tmp,task,NXYZ);
  
    bc->cnt = (char *)CALLOC(6,csz);
	
	mbn = 0; period = 0;
	surf_burn(bc,tmp,nx,ny,nz,0,task,&mbn,&period);

	FREE(bc->cnt,6*csz);

	if( task == MED_AX )
	{
			printf("\n\nWriting medial axis data sets .....\n"); 
			//out_ma_data(tmp,nx,ny,nz,outfile_base,zs,p_num,
			//				tfirst,tlast,out_cmprss);
			/* tmp->temp NOT FREED IN THIS CASE?, done in out_ma_data */
	}
	else if( (task == ERODE) || (task == NO_OPT) )
	{
			printf("\n\nWriting burn data sets .....\n"); 
			for( i = 0;  i < NXYZ;  i++ )
			{
				if( (tmp->bfdat[i] != GRAIN) && (tmp->bfdat[i] != EXT_VAL) )
					tmp->bfdat[i] = tmp->bfdat[i] - 126;
			}
	}
	FREE(tmp->cube ,NXYZ*usz);
}

void	dir_set(Fdat *tmp, int task, int NXYZ)
{
	int	i;


	/******* setting as ********

		  (-y)
	      7	   0    1 	in all 3 slices: top, middle, and bottom.
		\  |  / 			 (+z)		   (-z)	

	 (-x) 6	-  8  - 2 (+x)

		/  |  \ 
	      5    4	3
		  (+y)


	*** bottom slice *** all values are added 2 from middle slice. *
	dir->set[2][0] = 10;		-z-y
	dir->set[2][1] = 26;		-z-y+x
	dir->set[2][2] = 18;		-z  +x
	dir->set[2][3] = 22;		-z+y+x
	dir->set[2][4] = 6;		-z+y
	dir->set[2][5] = 38;		-z+y-x
	dir->set[2][6] = 34;		-z  -x
	dir->set[2][7] = 42;		-z-y-x
	dir->set[2][8] = 2;		-z
	
	*** middle slice ***
	dir->set[0][0] = 8;		  -y
	dir->set[0][1] = 24;		  -y+x
	dir->set[0][2] = 16;		    +x
	dir->set[0][3] = 20;		  +y+x
	dir->set[0][4] = 4;		  +y
	dir->set[0][5] = 36;		  +y-x
	dir->set[0][6] = 32;		    -x
	dir->set[0][7] = 40;		  -y-x
	dir->set[0][8] = 0;	-> unused one : need to calc. direction *

	***  top slice   *** all values are added 1 from middle slice. *
	dir->set[1][0] = 9;		+z-y
	dir->set[1][1] = 25;		+z-y+x
	dir->set[1][2] = 17;		+z  +x
	dir->set[1][3] = 21;		+z+y+x
	dir->set[1][4] = 5;		+z+y
	dir->set[1][5] = 37;		+z+y-x
	dir->set[1][6] = 33;		+z  -x
	dir->set[1][7] = 41;		+z-y-x
	dir->set[1][8] = 1;		+z

	Initialize direction array tmp->bfdat[][] equal to fdat[][],
	except tmp->bfdat[][] = 0 for pore voxels.
	Use this bfdat array to store burn direction values.

	Burn directions stored as lower 6 bits in 8 bit chars. The 6 bits
	are ordered (-x, +x, -y, +y, -z, +z).
	Thus the char containing
	
		0010 0000 (ie 32 decimal) indicates the -x direction.
		0001 1001 (ie 25 decimal) indicates +x-y+z direction.

	***************************/

	if( task == MED_AX )
	{
	    for( i = 0;  i < NXYZ;  i++ )
	    {
		if( tmp->fdat[i] == 0     ) tmp->bfdat[i] = GRAIN;
	        else if( tmp->fdat[i] >= GRAIN ) tmp->bfdat[i] = tmp->fdat[i];
		else				 tmp->bfdat[i] = 0;
	    }
	}
	else if( (task == ERODE) || (task == NO_OPT) )
	{
	    for( i = 0;  i < NXYZ;  i++ )
	    {
		if( tmp->fdat[i] == 1 )	tmp->bfdat[i] = GRAIN;
		else			tmp->bfdat[i] = tmp->fdat[i];
	    }
	}
}