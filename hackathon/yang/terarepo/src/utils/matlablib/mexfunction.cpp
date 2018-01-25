/*=================================================================
 * mexfunction.c 
 *
 * This example demonstrates how to use mexFunction.  It returns
 * the number of elements for each input argument, providing the 
 * function is called with the same number of output arguments
 * as input arguments.
 
 * This is a MEX-file for MATLAB.  
 * Copyright 1984-2011 The MathWorks, Inc.
 * All rights reserved.
 *=================================================================*/
/* $Revision: 1.5.6.3 $ */

/******************
*    CHANGELOG    *
*******************
* 2016-07-20. Giulio. @CREATED 
*/


#include <string.h>
//#include "VirtualVolume.h"

#include "MexOperations.h"

//#ifndef __APPLE__
typedef unsigned short char16_t;
//#endif

#include "mex.h"

# define N_INARGS    1   // minumum number of input arguments
# define N_OUTARGS   1   // maximum number of output arguments

# define MAX_OBJECTS 10 // max number of volumes that can be instantiated at the same time



static int initialized = 0;
static mxArray *persistent_array_ptr = NULL;

void cleanup(void) {
    mexPrintf("MEX-file is terminating, destroying array\n");

	// release C++ allocated memory
	disposeObjects((void **) mxGetData(persistent_array_ptr),MAX_OBJECTS);

    mxDestroyArray(persistent_array_ptr);
}

void mexFunction(int nlhs,
    mxArray *plhs[],
    int nrhs,
    const mxArray *prhs[])
{
	if ( nrhs == 0 && nlhs == 0 ) {
		mexPrintf("Library to create and manipulate VirtualVolumes\n\nUsage: \n");
		mexPrintf("\t[result] = matlablib(operation, ... ) \n\n");
		mexPrintf("\t\tID = matlablib('create') \n");
		mexPrintf("\t\tmatlablib('dispose',ID) \n");
		mexPrintf("\t\tinfo = matlablib('getinfo',ID) \n");
		mexPrintf("\t\tI = matlablib('loadsubvolume',ID) \n");
		mexPrintf("\t\tI = matlablib('loadsubvolume',ID,V) \n");
		mexPrintf("\t\tI = matlablib('loadsubvolume',ID,V,H) \n");
		mexPrintf("\t\tI = matlablib('loadsubvolume',ID,V,H,D) \n");
		return;
	}
	
	if (!initialized) {
		//mexPrintf("MEX-file initializing, creating array\n");

		mxClassID ptrClass = mxUINT64_CLASS;
		if ( sizeof(void *) == 4 )
			ptrClass = mxUINT32_CLASS;
		//mexPrintf("Pointer size: %d\n",sizeof(void *));
        
		/* Create persistent array and register its cleanup. */
		persistent_array_ptr = mxCreateNumericMatrix(1, MAX_OBJECTS+1, ptrClass, mxREAL); // last element is a sentinel
		mexMakeArrayPersistent(persistent_array_ptr);
		mexAtExit(cleanup);
		initialized = 1;

		void **data = (void **) mxGetData(persistent_array_ptr);
		for ( int i=0; i<MAX_OBJECTS+1; i++ )
			data[i] = (void *) 0;

		//for ( int i=0; i<MAX_OBJECTS; i++ )
		//	mexPrintf("%p ",data[i]);
		//mexPrintf("\n");

		/* Set the data of the array to some interesting value. */
		//*mxGetPr(persistent_array_ptr) = 1.0;
	} 
// 	else {
// 		mexPrintf("MEX-file executing; value of first array element is %g\n",
// 				*mxGetPr(persistent_array_ptr));
// 	}

	/* ------------------------   INPUT PARAMETERS   ----------------------------- */

    int argn = 0;

	mwSize nDims2;
	mwSize dims2[2];



	//double *matData;

	//int nRows, nCols;



	int N;
	int nmlen;
	char *name;

	int index;

    // examine input (right-hand-side) arguments
	if ( nrhs < N_INARGS ) {
		mexPrintf("\nArgument missing.\n");
		return;
	}

	// string argument: operation to be executed
	if ( (nDims2 = mxGetNumberOfDimensions(prhs[argn])) != 2 ) {
		mexPrintf("\nArgument is not a string (#dimensions: %d)\n",nDims2);
		return;

	}

	N = (int) mxGetM(prhs[argn]); // #rows (should be 1)
	nmlen = (int) mxGetN(prhs[argn]); // string len
	if ( !strcmp(mxGetClassName(prhs[argn]),"char") ) {
		name = (char *) malloc((nmlen+1)*sizeof(char));
		mxGetString(prhs[argn],name,nmlen+1);
		//mexPrintf("\nFirst argument is a string (%s).\n",name);
	}
	else {
		mexPrintf("\nArgument is not a string (%s).\n",mxGetClassName(prhs[0]));
		return;
	}
	argn++;

	if ( strcmp(name,"create") == 0 ) {
		//mexPrintf("\nPerform 'create' operation.\n");

		char *initString;
		if ( nrhs < 2 ) {
			mexPrintf("\nArgument missing.\n");
			return;
		}

		// string argument: volume to be created
		if ( (nDims2 = mxGetNumberOfDimensions(prhs[argn])) != 2 ) {
			mexPrintf("\nArgument is not a string (#dimensions: %d)\n",nDims2);
			return;
		}
		N = (int) mxGetM(prhs[argn]); // #rows (should be 1)
		nmlen = (int) mxGetN(prhs[argn]); // string len
		if ( !strcmp(mxGetClassName(prhs[argn]),"char") ) {
			initString = (char *) new char[(nmlen+1)*sizeof(char)];
			mxGetString(prhs[argn],initString,nmlen+1);
			//mexPrintf("\nSecond argument is a string (%s).\n",initString);
		}
		else {
			mexPrintf("\nArgument is not a string (%s).\n",mxGetClassName(prhs[0]));
			return;
		}
		argn++;

		index = createObject((void **)mxGetData(persistent_array_ptr),MAX_OBJECTS,initString);
		delete initString;
		if ( index < 0 ) {
			mexPrintf("\nCannot create object (%s).\n",initString);
			return;
		}
		else { // operation succeded, return volume ID
			//mexPrintf("\nReturned index: %d.\n",index);

			/************ OUTPUT PARAMETER ************/
    		plhs[0]=mxCreateDoubleMatrix(1,1,mxREAL);
    		*mxGetPr(plhs[0])=(double)index;   
		}
	}
	else if ( strcmp(name,"dispose") == 0 ) {
		//mexPrintf("\nPerform 'dispose' operation.\n");

		if ( nrhs < 2 ) {
			mexPrintf("\nArgument missing.\n");
			return;
		}

		double value;
		// numeric argument: volume ID
		if ( (nDims2 = mxGetNumberOfDimensions(prhs[argn])) != 2 ) {
			mexPrintf("\nArgument is not a volume ID (#dimensions: %d)\n",nDims2);
			return;
		}
		if ( !strcmp(mxGetClassName(prhs[argn]),"double") ) {
			dims2[0] = mxGetM(prhs[argn]);
			dims2[1] = mxGetN(prhs[argn]);
			if ( dims2[0] != 1 || dims2[1] != 1 ) {
				mexPrintf("\nArgument is not a volume ID (array of size: %d x %d).\n",dims2[0],dims2[1]);
				return;
			}
			value = *((double *) mxGetData(prhs[argn]));
			//mexPrintf("\nVolume ID: %f.\n",value);

			int ind = (int) value;
			disposeObject(((void **)mxGetData(persistent_array_ptr))[ind]);
			((void **)mxGetData(persistent_array_ptr))[ind] = 0;
		}
		else {
			mexPrintf("\nArgument is not a volume ID (argument class: %s).\n",mxGetClassName(prhs[argn]));
			return;
		}
		argn++;
	}
	else if ( strcmp(name,"getinfo") == 0 ) {
		//mexPrintf("\nPerform 'getinfo' operation.\n");

		if ( nrhs < 2 ) {
			mexPrintf("\nArgument missing.\n");
			return;
		}

		double value;
		// numeric argument: volume ID
		if ( (nDims2 = mxGetNumberOfDimensions(prhs[argn])) != 2 ) {
			mexPrintf("\nArgument is not a volume ID (#dimensions: %d)\n",nDims2);
			return;
		}
		if ( !strcmp(mxGetClassName(prhs[argn]),"double") ) {
			dims2[0] = mxGetM(prhs[argn]);
			dims2[1] = mxGetN(prhs[argn]);
			if ( dims2[0] != 1 || dims2[1] != 1 ) {
				mexPrintf("\nArgument is not a volume ID (array of size: %d x %d).\n",dims2[0],dims2[1]);
				return;
			}
			value = *((double *) mxGetData(prhs[argn]));
			//mexPrintf("\nVolume ID: %f.\n",value);

			info_t info;
			int ind = (int) value;
			getInfo(((void **)mxGetData(persistent_array_ptr))[ind],&info);
			
			/************ OUTPUT PARAMETER ************/
			const char *fnames[] =       { "root_dir",   "vxl_V",       "vxl_H",         "vxl_D",        "org_V",       "org_H",         "org_D", 
										   "dim_V",        "dim_H",        "dim_D",        "dim_C",      "bytes_x_chan", "dimT",        "t0",          "t1" };
 			mxClassID   classIDflags[] = { mxCHAR_CLASS, mxSINGLE_CLASS, mxSINGLE_CLASS, mxSINGLE_CLASS, mxSINGLE_CLASS, mxSINGLE_CLASS, mxSINGLE_CLASS, 
										   mxUINT32_CLASS, mxUINT32_CLASS, mxUINT32_CLASS, mxINT32_CLASS, mxINT32_CLASS, mxINT32_CLASS, mxINT32_CLASS, mxINT32_CLASS };
	   		int         nfields = 15;
    		int         ifield;
    		mxArray    *fout;
    		// create a 1x1 struct matrix for output 
    		plhs[0] = mxCreateStructMatrix(1, 1, nfields, fnames);
			fout = mxCreateString(info.root_dir);
        	mxSetFieldByNumber(plhs[0], 0, 0, fout);
	   		for(ifield=1; ifield<nfields; ifield++) {
            	fout = mxCreateNumericMatrix(1, 1, classIDflags[ifield], mxREAL);
        		mxSetFieldByNumber(plhs[0], 0, ifield, fout);
			}
			*((float *) mxGetData(mxGetFieldByNumber(plhs[0],0,1))) = info.VXL_V;
			*((float *) mxGetData(mxGetFieldByNumber(plhs[0],0,2))) = info.VXL_H;
			*((float *) mxGetData(mxGetFieldByNumber(plhs[0],0,3))) = info.VXL_D;
			*((float *) mxGetData(mxGetFieldByNumber(plhs[0],0,4))) = info.ORG_V;
			*((float *) mxGetData(mxGetFieldByNumber(plhs[0],0,5))) = info.ORG_H;
			*((float *) mxGetData(mxGetFieldByNumber(plhs[0],0,6))) = info.ORG_D;
			*((int *) mxGetData(mxGetFieldByNumber(plhs[0],0,7)))   = info.DIM_V;
			*((int *) mxGetData(mxGetFieldByNumber(plhs[0],0,8)))   = info.DIM_H;
			*((int *) mxGetData(mxGetFieldByNumber(plhs[0],0,9)))   = info.DIM_D;
			*((int *) mxGetData(mxGetFieldByNumber(plhs[0],0,10)))  = info.DIM_C;
			*((int *) mxGetData(mxGetFieldByNumber(plhs[0],0,11)))  = info.BYTESxCHAN;
			*((int *) mxGetData(mxGetFieldByNumber(plhs[0],0,12)))  = info.DIM_T;
			*((int *) mxGetData(mxGetFieldByNumber(plhs[0],0,13)))  = info.t0;
			*((int *) mxGetData(mxGetFieldByNumber(plhs[0],0,14)))  = info.t1;
		}
		else {
			mexPrintf("\nArgument is not a volume ID (argument class: %s).\n",mxGetClassName(prhs[argn]));
			return;
		}
		argn++;
	}
	else if ( strcmp(name,"loadsubvolume") == 0 ) {
		//mexPrintf("\nPerform 'loadsubvolume' operation.\n");

		if ( nrhs < 2 ) {
			mexPrintf("\nArgument missing.\n");
			return;
		}

		double value;
		// numeric argument: volume ID
		if ( (nDims2 = mxGetNumberOfDimensions(prhs[argn])) != 2 ) {
			mexPrintf("\nArgument is not a volume ID (#dimensions: %d)\n",nDims2);
			return;
		}
		if ( !strcmp(mxGetClassName(prhs[argn]),"double") ) {
			dims2[0] = mxGetM(prhs[argn]);
			dims2[1] = mxGetN(prhs[argn]);
			if ( dims2[0] != 1 || dims2[1] != 1 ) {
				mexPrintf("\nArgument is not a volume ID (array of size: %d x %d).\n",dims2[0],dims2[1]);
				return;
			}
			value = *((double *) mxGetData(prhs[argn]));
			//mexPrintf("\nVolume ID: %f.\n",value);

		}
		else {
			mexPrintf("\nArgument is not a volume ID (argument class: %s).\n",mxGetClassName(prhs[argn]));
			return;
		}
		argn++;

		// get info about volume
		int ind = (int) value;
		info_t info;
		// get volume info
		getInfo(((void **)mxGetData(persistent_array_ptr))[ind],&info);	

		// volume limiting arguments: [V0 v1], [H0 H1], [D0 D1]
		int V0, H0, D0, V1, H1, D1;
		
		if ( nrhs >= 3 ) { // get V0, V1
			if ( (nDims2 = mxGetNumberOfDimensions(prhs[argn])) != 2 ) {
				mexPrintf("\nArgument is not a volume ID (#dimensions: %d)\n",nDims2);
				return;
			}
			if ( !strcmp(mxGetClassName(prhs[argn]),"double") ) {
				dims2[0] = mxGetM(prhs[argn]);
				dims2[1] = mxGetN(prhs[argn]);
				if ( dims2[0] != 1 || dims2[1] != 2 ) {
					mexPrintf("\nArgument is not an index interval (array of size: %d x %d).\n",dims2[0],dims2[1]);
					return;
				}
				V0 = ((double *) mxGetData(prhs[argn]))[0];
				V1 = ((double *) mxGetData(prhs[argn]))[1];
			}
			else {
				mexPrintf("\nArgument is not a volume ID (argument class: %s).\n",mxGetClassName(prhs[argn]));
				return;
			}
			argn++;		
		}
		else { // default values
			V0 = 0;
			V1 = info.DIM_V;
		}
		
		if ( nrhs >= 4 ) { // get H0, H1
			if ( (nDims2 = mxGetNumberOfDimensions(prhs[argn])) != 2 ) {
				mexPrintf("\nArgument is not a volume ID (#dimensions: %d)\n",nDims2);
				return;
			}
			if ( !strcmp(mxGetClassName(prhs[argn]),"double") ) {
				dims2[0] = mxGetM(prhs[argn]);
				dims2[1] = mxGetN(prhs[argn]);
				if ( dims2[0] != 1 || dims2[1] != 2 ) {
					mexPrintf("\nArgument is not an index interval (array of size: %d x %d).\n",dims2[0],dims2[1]);
					return;
				}
				H0 = ((double *) mxGetData(prhs[argn]))[0];
				H1 = ((double *) mxGetData(prhs[argn]))[1];
			}
			else {
				mexPrintf("\nArgument is not a volume ID (argument class: %s).\n",mxGetClassName(prhs[argn]));
				return;
			}
			argn++;		
		}
		else { // default values
			H0 = 0;
			H1 = info.DIM_H;
		}

		if ( nrhs >= 5 ) { // get D0, D1
			if ( (nDims2 = mxGetNumberOfDimensions(prhs[argn])) != 2 ) {
				mexPrintf("\nArgument is not a volume ID (#dimensions: %d)\n",nDims2);
				return;
			}
			if ( !strcmp(mxGetClassName(prhs[argn]),"double") ) {
				dims2[0] = mxGetM(prhs[argn]);
				dims2[1] = mxGetN(prhs[argn]);
				if ( dims2[0] != 1 || dims2[1] != 2 ) {
					mexPrintf("\nArgument is not an index interval (array of size: %d x %d).\n",dims2[0],dims2[1]);
					return;
				}
				D0 = ((double *) mxGetData(prhs[argn]))[0];
				D1 = ((double *) mxGetData(prhs[argn]))[1];
			}
			else {
				mexPrintf("\nArgument is not a volume ID (argument class: %s).\n",mxGetClassName(prhs[argn]));
				return;
			}
			argn++;		
		}
		else { // default values
			D0 = 0;
			D1 = info.DIM_D;
		}

		mexPrintf("\nSubvolume limits: [%d,%d] x [%d,%d] x [%d,%d].\n",V0,V1,H0,H1,D0,D1);

		// perform loadsubvolume operation
		if ( info.DIM_T > 1 ) {
			mexPrintf("\nMore than one time point (%d).\n",info.DIM_T);
			return;
		}
		// get data		
		unsigned char *subvol = loadSubvolume(((void **)mxGetData(persistent_array_ptr))[ind],V0,V1,H0,H1,D0,D1);
		mexPrintf("\nReturned buffer: %p.\n",subvol);
		
		// build a 3D matrix
		//mwSize dims[] = { info.DIM_V, info.DIM_H, info.DIM_D, info.n_active };
		mwSize dims[] = { V1-V0, H1-H0, D1-D0, info.n_active };
		mwSize ndim   = 4;
		plhs[0] = mxCreateNumericArray(ndim, dims, (info.BYTESxCHAN == 1) ? mxUINT8_CLASS : mxUINT16_CLASS, mxREAL);
    		
		mwSize rowsize = H1-H0; //info.DIM_H;
		mwSize colsize = V1-V0; //info.DIM_V;
    	mwSize slicesize = rowsize * colsize;
		//mwSize channelsize = slicesize * (D1-D0); //info.DIM_D;
    		
    	//memcpy(mxGetData(plhs[0]),subvol,channelsize * info.n_active * info.BYTESxCHAN);

    	if ( info.BYTESxCHAN == 1 ) {
    		unsigned char *sPtr = subvol;
			unsigned char *dPtr = (unsigned char *) mxGetData(plhs[0]);
    		for ( mwSize c = 0; c < info.n_active; c++ )
    			for ( mwSize k = 0; k < (D1-D0); k++, sPtr += slicesize, dPtr += slicesize )
    				for ( mwSize i = 0; i < colsize; i++ )
						for ( mwSize j = 0; j < rowsize; j++ )
    						dPtr[i + j*colsize] = sPtr[j + i*rowsize];
    	}
    	else {
			unsigned short *sPtr = (unsigned short *) subvol;
    		unsigned short *dPtr = (unsigned short *) mxGetData(plhs[0]);
    		for ( mwSize c = 0; c < info.n_active; c++ )
    			for ( mwSize k = 0; k < (D1-D0); k++, sPtr += slicesize, dPtr += slicesize )
    				for ( mwSize i = 0; i < colsize; i++ )
						for ( mwSize j = 0; j < rowsize; j++ )
							dPtr[i + j*colsize] = sPtr[j + i*rowsize];
    	}
    									
		delete subvol;

	}
	else if ( strcmp(name,"debug") == 0 ) {
		void **data = (void **) mxGetData(persistent_array_ptr);
		for ( int i=0; i<MAX_OBJECTS; i++ )
			mexPrintf("%p ",data[i]);
		mexPrintf("\n");
	}
	else 
		mexPrintf("\nInvalid operation (%s).\n",name);


	/* ------------------------   OUTPUT PARAMETERS   ---------------------------- */

    /* Examine output (left-hand-side) arguments. */
    //mexPrintf("\n\nThere are %d left-hand-side argument(s).\n", nlhs);
    if ( nlhs > N_OUTARGS ) {
      mexErrMsgIdAndTxt("MATLAB:mexfunction:","Too many output parameters.\n");
      //mexPrintf( "Too many output parameters (%d).\n",nlhs);
    }
    
    for (int i=1; i<nlhs; i++)  {
        plhs[i]=mxCreateDoubleMatrix(1,1,mxREAL);
        *mxGetPr(plhs[i])=0;
    }
}
