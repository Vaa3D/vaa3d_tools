//------------------------------------------------------------------------------------------------
// Copyright (c) 2012  Alessandro Bria and Giulio Iannello (University Campus Bio-Medico of Rome).  
// All rights reserved.
//------------------------------------------------------------------------------------------------

/*******************************************************************************************************************************************************************************************
*    LICENSE NOTICE
********************************************************************************************************************************************************************************************
*    By downloading/using/running/editing/changing any portion of codes in this package you agree to this license. If you do not agree to this license, do not download/use/run/edit/change
*    this code.
********************************************************************************************************************************************************************************************
*    1. This material is free for non-profit research, but needs a special license for any commercial purpose. Please contact Alessandro Bria at a.bria@unicas.it or Giulio Iannello at 
*       g.iannello@unicampus.it for further details.
*    2. You agree to appropriately cite this work in your related studies and publications.
*    3. This material is provided by  the copyright holders (Alessandro Bria  and  Giulio Iannello),  University Campus Bio-Medico and contributors "as is" and any express or implied war-
*       ranties, including, but  not limited to,  any implied warranties  of merchantability,  non-infringement, or fitness for a particular purpose are  disclaimed. In no event shall the
*       copyright owners, University Campus Bio-Medico, or contributors be liable for any direct, indirect, incidental, special, exemplary, or  consequential  damages  (including, but not 
*       limited to, procurement of substitute goods or services; loss of use, data, or profits;reasonable royalties; or business interruption) however caused  and on any theory of liabil-
*       ity, whether in contract, strict liability, or tort  (including negligence or otherwise) arising in any way out of the use of this software,  even if advised of the possibility of
*       such damage.
*    4. Neither the name of University  Campus Bio-Medico of Rome, nor Alessandro Bria and Giulio Iannello, may be used to endorse or  promote products  derived from this software without
*       specific prior written permission.
********************************************************************************************************************************************************************************************/

/******************
*    CHANGELOG    *
*******************
*******************
* 2016-10-05. Giulio. @FIXED added deallocation of 'vol_dimes' in the destructor of class 'BDV_HDF5_fdescr_t'
* 2015-12-29. Giulio. @ADDED support for extracting 16 bits subvolumed 
* 2015-12-10. Giulio. @ADDED conditional compilation to exclude HDF5 dependent code
* 2015-12-09. Giulio. @RELEASED first working version
* 2015-11-17. Giulio. @CREATED 
*/

#include "HDF5Mngr.h"
#include "RawFmtMngr.h"
#include <stdlib.h> // needed by clang: defines size_t
#include <sstream>

#ifdef ENABLE_BDV_HDF5
#include "hdf5.h"
#endif

#ifdef _VAA3D_TERAFLY_PLUGIN_MODE
#include <QElapsedTimer>
#include "PLog.h"
#include "COperation.h"
#endif


#ifdef ENABLE_BDV_HDF5

#define HDF5_SUFFIX   "h5"

#include <string.h>


#define MAXSTP   10
#define MAXRES   10
#define MAXTPS   10

#define DEF_CHNK_DIM   16

#define MAX_NAME 1024

typedef double vxl_size_t[3];
typedef hsize_t dims_t[3];
typedef int subdvsns_t[3];
 
/* WARNINIG:
 * current implementation assumes that all setups from 0 to i-1 have been created before creating setup i
 * current implementation assumes that all time points from 0 to i-1 have been created before creating timepoint i
 */

/****************************************************************************
* BDV-HDF5 file descriptor
****************************************************************************/

class BDV_HDF5_fdescr_t {
	char       *fname;           // complete file path and name
	hid_t       file_id;         // file handle
	int         n_setups;        // number of setups
	hid_t      *setup_groups_id; // handles of setup groups
	int         n_timepoints;    // number of time points
	hid_t      *tp_groups_id;    // handles of time point groups
	int         n_res;           // number of resolutions
	vxl_size_t *vxl_sizes;       // voxels sizes of each resolution
	dims_t     *vol_dims;        // volume dimensions of each resolution
	subdvsns_t *chunk_dims;      // chunk dimensions of each resolution
	int         vxl_nbytes;      // number of bytes of each channel 
	hid_t       vxl_type;        // HDF5 type of voxel values
	hsize_t  ***n_slices;        // number of slices of each time point at each setup at each resolutions (all setups should have the same number of slices)
	
	// private methods
	void scan_root ( );

public:
	BDV_HDF5_fdescr_t ( );
	/* default constructor: returns and empty descriptor */

	BDV_HDF5_fdescr_t ( const char *_fname, int _vxl_nbytes = 2, int maxstp = MAXSTP, int maxres = MAXRES, int maxtps = MAXTPS );
	/*  */

	~BDV_HDF5_fdescr_t ( );
	/* close the HDF5 file and deallocates memory */
	
	// getters
	int      getN_SETUPS ( )               { return n_setups; }
	int      getN_TPS ( )                  { return n_timepoints; }
	int      getN_RES ( )                  { return n_res; }
	hid_t    getSETUP_GROUPS_ID ( int sg ) { return setup_groups_id[sg]; }
	hid_t    getTP_GROUPS_ID ( int tp )    { return tp_groups_id[tp]; }
	double  *getVXL_SZ ( int r )           { return vxl_sizes[r]; }
	hsize_t *getVOL_DIMS ( int r )         { return vol_dims[r]; }
	int     *getCHUNK_DIMS ( int r )       { return chunk_dims[r]; }
	int      getVXL_NBYTES ( )             { return vxl_nbytes; }
	hsize_t  getVXL_TYPE ( )               { return vxl_type; }
	hsize_t  getN_SLICES ( int tp, int s, int r ) { return n_slices[tp][s][r]; }

	hid_t *getDATASETS_ID ( int tp, int r );
	/* returns a list of the n_setups handles of datasets of resolution r of time point tp */
	
	// other operations	
	int addSetup ( int s );
	/* add setup s to file */

	int addResolution ( int r, float vxlszV, float vxlszH, float vxlszD, hsize_t dimV, hsize_t dimH, hsize_t dimD, hsize_t chnk_dimV, hsize_t chnk_dimH, hsize_t chnk_dimD );	
	/* add resolution r with voxel size (vxlszV x vxlszH x vxlszD), dimensions (dimV x dimH x dimD) and chunk dimensions (chnk_dimV x chnk_dimH x chnk_dimD) to all setups */

	int addTimePoint ( int t );
	/* add time point t (with all setups and all resolutions) */

	int writeHyperslab ( int tp, int s, int r, iim::uint8 *buf, hsize_t *dims_buf, hsize_t *hl_buf, hsize_t *hl_file = 0 ); 
	/* write hyperslab hl_buf stored in buffer buf to hyperslab hl_file at time point tp and resolution r */
};


/* Descriptor for reading subvolumes with direct access to datasets
 */
struct BDV_volume_descr_t {
	int n_setups;
	hid_t  vxl_type;
	int    vxl_nbytes;
	hid_t *datasets_id; //
};



BDV_HDF5_fdescr_t::BDV_HDF5_fdescr_t ( ) {
	fname = (char *) 0;
	file_id = (hid_t) -1;
	n_setups = 0;
	setup_groups_id = (hid_t *) 0;
	n_timepoints = 0;
	tp_groups_id = (hid_t *) 0;
	n_res = 0;
	vxl_sizes = (vxl_size_t *) 0;
	vol_dims = (dims_t *) 0;	
	chunk_dims = (subdvsns_t *) 0;	
	vxl_nbytes = 0;
	vxl_type = -1;
	n_slices = (hsize_t ***) 0;
}


BDV_HDF5_fdescr_t::BDV_HDF5_fdescr_t ( const char *_fname, int _vxl_nbytes, int maxstp, int maxres, int maxtps ) {
	if ( (file_id = H5Fopen(_fname, H5F_ACC_RDWR, H5P_DEFAULT)) < 0 ) { // non existing file: create it empty

		file_id = H5Fcreate(_fname, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

		fname = new char[strlen(_fname)+1];
		strcpy(fname,_fname);
		n_setups = 0;
		setup_groups_id = new hid_t[maxstp];
		n_timepoints = 0;
		tp_groups_id = new hid_t[maxtps];
		n_res = 0;
		vxl_sizes = new vxl_size_t[maxres];
		vol_dims = new dims_t[maxres];	
		chunk_dims = new subdvsns_t[maxres];
		vxl_nbytes = _vxl_nbytes;

		n_slices = new hsize_t **[maxtps];
			
		/*
    	 * Create the data types group.
		 */
		hid_t group_id = H5Gcreate2(file_id, "/__DATA_TYPES__", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

		/* Create a named datatype for boolean values */
		unsigned char val;
		hid_t type_id = H5Tcreate(H5T_ENUM,sizeof(unsigned char));
		H5Tenum_insert(type_id, "FALSE", (val=0,&val));
		H5Tenum_insert(type_id, "TRUE",  (val=1,&val));
		herr_t status = H5Tcommit(group_id,"Enum_Boolean",type_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		H5Tclose(type_id);

		/* Create a named datatype for variable length strings */
		type_id = H5Tcopy(H5T_C_S1);
		status = H5Tset_size(type_id, H5T_VARIABLE);
		status = H5Tcommit(group_id,"String_VariableLength",type_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		H5Tclose(type_id);
	
		/* Close the group. */
		status = H5Gclose(group_id);
	}
	else { // existing file: initialize descriptor
		fname = new char[strlen(_fname)+1];
		strcpy(fname,_fname);
		scan_root();
	}

	if ( vxl_nbytes == 1 )
		vxl_type = H5T_NATIVE_CHAR;
	else if ( vxl_nbytes == 2 )
		vxl_type = H5T_NATIVE_SHORT;
	else
		throw iim::IOException(iim::strprintf("number of bytes per voxel not allowed (%d)",vxl_nbytes).c_str(),__iim__current__function__);
}


BDV_HDF5_fdescr_t::~BDV_HDF5_fdescr_t ( ) {
	if ( fname )
		delete fname;
	if ( setup_groups_id ) {
		for ( int s=0; s<n_setups; s++ )
			H5Gclose(setup_groups_id[s]);
		delete[] setup_groups_id;
	}
	if ( tp_groups_id ) {
		for ( int t=0; t<n_timepoints; t++ )
			H5Gclose(tp_groups_id[t]);
		delete[] tp_groups_id;
	}
	if ( vol_dims )
		delete[] vol_dims;
	if ( vxl_sizes )
		delete[] vxl_sizes;
	if ( chunk_dims )
		delete[] chunk_dims;
	if ( n_slices ) {
		for ( int t=0; t<n_timepoints; t++ )
			if ( n_slices[t] ) {
				for ( int s=0; s<n_setups; s++ )
						delete [] n_slices[t][s];
				delete[]  n_slices[t];
			}
		delete[] n_slices;
	}
	H5Fclose(file_id);
}


int BDV_HDF5_fdescr_t::addSetup ( int s ) {
	std::string setup_dir;
	int n_digits = 2; // BDV uses 2 digits to number setups
	std::stringstream setup_ord;
	setup_ord.width(n_digits);
	setup_ord.fill('0');
	setup_ord << s;
    setup_dir = "/" + ("s" + setup_ord.str()); // setups names have the format sXX

    /*
     * Create the group for setup descriptions
     */
	hid_t group_id;
	hid_t dataspace_id;
	hid_t dataset_id;
	herr_t status;
	hsize_t dims[2];
	hsize_t chunk_dims[2];
	hid_t cparms;
    hsize_t maxdims[2] = {H5S_UNLIMITED, H5S_UNLIMITED};
	
	group_id = H5Gcreate2(file_id, setup_dir.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

	/* Create the data space for the resolution and subdivisions datasets. */
	chunk_dims[0] = dims[0] = 1;
	chunk_dims[1] = dims[1] = 3;
	dataspace_id = H5Screate_simple(2, dims, maxdims);

	/* Create the parameter list for the resolution and subdivisions datasets. */
    cparms = H5Pcreate(H5P_DATASET_CREATE);
    status = H5Pset_chunk(cparms, 2, chunk_dims);
    status = H5Pset_fill_time(cparms,H5D_FILL_TIME_ALLOC);
     
	/* Create the resolutions dataset. */
	dataset_id = H5Dcreate(group_id, "resolutions", H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT, cparms, H5P_DEFAULT);

	/* End access to the dataset and release resources used by it. */
	status = H5Dclose(dataset_id);

	/* Create the subdivisions dataset. */
	dataset_id = H5Dcreate(group_id, "subdivisions", H5T_NATIVE_INT, dataspace_id, H5P_DEFAULT, cparms, H5P_DEFAULT);

	/* End access to the dataset and release resources used by it. */
	status = H5Dclose(dataset_id);

	/* Terminate access to the parameters. */ 
	status = H5Pclose(cparms);

	/* Terminate access to the data space. */ 
	status = H5Sclose(dataspace_id);

	// update descriptor
	if ( n_setups == MAXSTP )
		throw iim::IOException(iim::strprintf("too much setups (n_setups = %d",n_setups).c_str(),__iim__current__function__);

	setup_groups_id[n_setups] = group_id;

	n_setups++;

	/* Close the group. */
	//status = H5Gclose(group_id);

	return s;
}


int BDV_HDF5_fdescr_t::addResolution ( int r, float vxlszV, float vxlszH, float vxlszD, 
									  hsize_t dimV, hsize_t dimH, hsize_t dimD, hsize_t chnk_dimV, hsize_t chnk_dimH, hsize_t chnk_dimD ) {

	hid_t r_dataset_id;
	hid_t s_dataset_id;
	hsize_t size[2];
	herr_t status;
	hid_t filespace_id;
	hsize_t offset[2];
	hid_t dataspace_id;

	// confrontare r con n_res (sono sempre uguali?)

	// update descriptor
	if ( n_res == MAXRES )
		throw iim::IOException(iim::strprintf("too much resolutions (n_res = %d)",n_res).c_str(),__iim__current__function__);

	vxl_sizes[n_res][0] = vxlszD;    
	vxl_sizes[n_res][1] = vxlszV;    
	vxl_sizes[n_res][2] = vxlszH;    
	vol_dims[n_res][0] = dimD; 
	vol_dims[n_res][1] = dimV; 
	vol_dims[n_res][2] = dimH; 
	chunk_dims[n_res][0] = (chnk_dimD==-1) ? DEF_CHNK_DIM : chnk_dimD; 
	chunk_dims[n_res][1] = (chnk_dimV==-1) ? DEF_CHNK_DIM : chnk_dimV; 
	chunk_dims[n_res][2] = (chnk_dimH==-1) ? DEF_CHNK_DIM : chnk_dimH; 

	// Define memory space 1x3
	size[0] = 1;
	size[1] = 3;
    dataspace_id = H5Screate_simple(2, size, NULL);

	for ( int s=0; s<n_setups; s++ ) {
		r_dataset_id = H5Dopen(setup_groups_id[s],"resolutions", H5P_DEFAULT);
		s_dataset_id = H5Dopen(setup_groups_id[s],"subdivisions", H5P_DEFAULT);
		// extends resolutions dataset
		size[0]   = r+1;
		size[1]   = 3;
		status = H5Dset_extent(r_dataset_id, size);
		status = H5Dset_extent(s_dataset_id, size);
		// select hyperslab
		offset[0] = r;
		offset[1] = 0;
		size[0]   = 1;
		size[1]   = 3;
		// Write the data to the hyperslab.
		// resolutions
		filespace_id = H5Dget_space(r_dataset_id);
		status = H5Sselect_hyperslab(filespace_id, H5S_SELECT_SET, offset, NULL, size, NULL);
		status = H5Dwrite(r_dataset_id, H5T_NATIVE_DOUBLE, dataspace_id, filespace_id,H5P_DEFAULT,vxl_sizes[n_res]);
    	H5Sclose(filespace_id);
		H5Dclose(r_dataset_id);
		// subdivisions
		filespace_id = H5Dget_space(s_dataset_id);
		status = H5Sselect_hyperslab(filespace_id, H5S_SELECT_SET, offset, NULL, size, NULL);
		status = H5Dwrite(s_dataset_id, H5T_NATIVE_INT, dataspace_id, filespace_id,H5P_DEFAULT,chunk_dims[n_res]);
    	H5Sclose(filespace_id);
		H5Dclose(s_dataset_id);
	}

    H5Sclose(dataspace_id);

	n_res++;

	return r;
}



int BDV_HDF5_fdescr_t::addTimePoint ( int t ) {
	std::string timepoint_dir;
	int n_digits = 5; // BDV uses 2 digits to number setups
	std::stringstream timepoint_ord;
	timepoint_ord.width(n_digits);
	timepoint_ord.fill('0');
	timepoint_ord << t;
    timepoint_dir = "/" + ("t" + timepoint_ord.str()); // setups names have the format sXX

    hsize_t maxdims[3] = {H5S_UNLIMITED, H5S_UNLIMITED, H5S_UNLIMITED};

	hid_t tp_group_id;
	hid_t tp_subgroup_id;
	hid_t tp_subsubgroup_id;
	herr_t status;
	hsize_t dims[3];
	hid_t dataspace_id;
	hid_t cparms;
	hid_t dataset_id;
	hsize_t len;
	char gname[MAX_NAME];

     // Create the group with timepoint data
	tp_group_id = H5Gcreate2(file_id, timepoint_dir.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

	/* Create the setup groups  */
	for ( int s=0; s<n_setups; s++ ) {

		/* Create the setup dataset group  */
		len = H5Iget_name(setup_groups_id[s], gname, (size_t)MAX_NAME );
		tp_subgroup_id = H5Gcreate2(tp_group_id, gname+1, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT); // skip the initial '/'

		for ( int r=0; r<n_res; r++ ) {

			std::stringstream res;
			res << r;

			/* Create the volume dataset group  */
			tp_subsubgroup_id = H5Gcreate2(tp_subgroup_id, res.str().c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

			/* Create the data space for the cells datasets. */
			dims[0] = vol_dims[r][0];
			dims[1] = vol_dims[r][1];
			dims[2] = vol_dims[r][2];
			dataspace_id = H5Screate_simple(3, dims, maxdims);

			/* Create the parameter list for cells datasets. */
		    cparms = H5Pcreate(H5P_DATASET_CREATE);
			dims[0] = chunk_dims[r][0]; // copy from int to hsize_t
			dims[1] = chunk_dims[r][1]; // copy from int to hsize_t
			dims[2] = chunk_dims[r][2]; // copy from int to hsize_t
		    status = H5Pset_chunk( cparms, 3, dims);
		    //status = H5Pset_fill_time(cparms,H5D_FILL_TIME_ALLOC);
		    //status = H5Pset_fill_time(cparms,H5D_FILL_TIME_NEVER);

			/* Set ZLIB / DEFLATE Compression using compression level 6.
			 * To use SZIP Compression comment out these lines. 
			 */ 
			status = H5Pset_deflate (cparms, 3); 

			/* Uncomment these lines to set SZIP Compression 
			szip_options_mask = H5_SZIP_NN_OPTION_MASK;
			szip_pixels_per_block = 16;
			status = H5Pset_szip (cparms, szip_options_mask, szip_pixels_per_block);
			*/

			/* Create the cells dataset. */
			dataset_id = H5Dcreate(tp_subsubgroup_id, "cells", vxl_type, dataspace_id, H5P_DEFAULT, cparms, H5P_DEFAULT);

			/* Write data */
			//status = H5Dwrite (dataset_id, H5T_NATIVE_SHORT, H5S_ALL, H5S_ALL, H5P_DEFAULT, buf);

			/* Terminate access to the parameters. */ 
			status = H5Pclose(cparms);

			/* Terminate access to the data space. */ 
			status = H5Sclose(dataspace_id);

			/* End access to the dataset and release resources used by it. */
			status = H5Dclose(dataset_id);

			/* Close the subsubgroup. */
			status = H5Gclose(tp_subsubgroup_id);
		}

		/* Close the subgroup. */
		status = H5Gclose(tp_subgroup_id);

	}

	/* Close the group. */
	//status = H5Gclose(tp_group_id);

	tp_groups_id[t] = tp_group_id;
	if ( t == n_timepoints ) { // one more time point has been added
		n_slices[t] = new hsize_t *[MAXSTP];
		for ( int s=0; s<n_setups; s++ ) {
			n_slices[t][s] = new hsize_t[MAXRES];
			memset(n_slices[t][s],0,MAXRES*sizeof(hsize_t));
		}
		n_timepoints++;
	}

	return t;
}


// private methods implementation

void BDV_HDF5_fdescr_t::scan_root ( ) {
	int i, num_ord;
	ssize_t len;
	hsize_t nobj;
	herr_t err;
	int otype;
	hid_t grpid;
	hid_t subgrpid;
	hid_t dset_id;
	hid_t dspace_id;
	hid_t dtype_id;
	herr_t status;
	hsize_t dims[3];

	char group_name[MAX_NAME];
	char memb_name[MAX_NAME];
	bool first_setup, first_timepoint;

	hid_t root_group_id = H5Gopen(file_id,"/", H5P_DEFAULT);
	/*
	 * Information about the group:
     */
	len = H5Iget_name (root_group_id, group_name, MAX_NAME);

    /*
     *  Get all the members of the groups, one at a time.
     */
	err = H5Gget_num_objs(root_group_id, &nobj);
	
	n_setups        = 0;
	setup_groups_id = new hid_t[MAXSTP];	
	n_timepoints    = 0;
	tp_groups_id    = new hid_t[MAXTPS];	
	n_res           = 0;
	n_slices        = new hsize_t **[MAXTPS]; // allocate only the timepoints list
	memset(n_slices,0,MAXTPS*sizeof(hsize_t **)); // and initialize to 0 to control deallocation in destructor

	// process setups first
	first_setup     = true;
	for (i = 0; i < nobj; i++) {
        /*
         *  For each object in the group, get the name and
         *  what type of object it is.
		 */
		len = H5Gget_objname_by_idx(root_group_id, (hsize_t)i, memb_name, (size_t)MAX_NAME );
		otype =  H5Gget_objtype_by_idx(root_group_id, (size_t)i );

        /*
         * process each object according to its type
         */
		switch(otype) {
			case H5G_LINK:
				break;
			case H5G_GROUP:
				grpid = H5Gopen(root_group_id,memb_name, H5P_DEFAULT);
				if ( memb_name[0] == 's' ) { // is a setup group
					num_ord = atoi(memb_name+1);
					setup_groups_id[num_ord] = grpid;
					n_setups++;
					if ( first_setup ) {
						first_setup = false;
						// initialize resolusions and subdivisions
						dset_id = H5Dopen(grpid,"resolutions",H5P_DEFAULT);
						dspace_id = H5Dget_space(dset_id);    
						status  = H5Sget_simple_extent_dims(dspace_id, dims, NULL);
						n_res = (int) dims[0];
						vxl_sizes       = new vxl_size_t[n_res];
						chunk_dims      = new subdvsns_t[n_res];
						status = H5Dread(dset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, vxl_sizes);	
						H5Sclose(dspace_id);
						H5Dclose(dset_id);
						dset_id = H5Dopen(grpid,"subdivisions",H5P_DEFAULT);
						dspace_id = H5Dget_space(dset_id);    
						status = H5Dread(dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, chunk_dims);	
						H5Sclose(dspace_id);
						H5Dclose(dset_id);
					}
					// grpid must not be closed because it has been assigned to setup_groups_id[num_ord]
				}
				else // if it is a time point do nothing now
					H5Gclose(grpid); 
				break;
			case H5G_DATASET:
				break;
			case H5G_TYPE:
				break;
			default:
				break;
		}

	}

	// process time points now
	first_timepoint = true;
	for (i = 0; i < nobj; i++) {
        /*
         *  For each object in the group, get the name and
         *  what type of object it is.
		 */
		len = H5Gget_objname_by_idx(root_group_id, (hsize_t)i, memb_name, (size_t)MAX_NAME );
		otype =  H5Gget_objtype_by_idx(root_group_id, (size_t)i );

        /*
         * process each object according to its type
         */
		switch(otype) {
			case H5G_LINK:
				break;
			case H5G_GROUP:
				grpid = H5Gopen(root_group_id,memb_name, H5P_DEFAULT);
				if ( memb_name[0] == 't' ) { // is a time point group
					num_ord = atoi(memb_name+1);
					tp_groups_id[num_ord] = grpid;
					n_timepoints++;
					if ( first_timepoint ) {
						first_timepoint = false;
						vol_dims        = new dims_t[n_res];
						// get name of setup to be written
						len = H5Iget_name(setup_groups_id[0], memb_name, (size_t)MAX_NAME );
						// grpid must not be closed
						grpid = H5Gopen(tp_groups_id[num_ord],memb_name+1,H5P_DEFAULT);
						for ( int r=0; r<n_res; r++ ) {
							// get group (resolution) of dataset
							std::stringstream res;
							res << r;
							subgrpid = H5Gopen(grpid, res.str().c_str(), H5P_DEFAULT);
							// get dataset
							dset_id = H5Dopen(subgrpid,"cells",H5P_DEFAULT);
							dspace_id = H5Dget_space(dset_id);    
							status = H5Sget_simple_extent_dims(dspace_id, vol_dims[r], NULL);
							dtype_id = H5Dget_type(dset_id);     /* datatype handle */ 
							//class_id     = H5Tget_class(dtype_id);
							//if (_class == H5T_INTEGER) printf("Data set has INTEGER type \n");
							//order     = H5Tget_order(datatype);
							//if (order == H5T_ORDER_LE) printf("Little endian order \n");
							vxl_nbytes = H5Tget_size(dtype_id);

							H5Tclose(dtype_id);
							H5Sclose(dspace_id);
							H5Dclose(dset_id);
							H5Gclose(subgrpid);
						}
						H5Gclose(grpid); // setup group (subgroup of the time point) must be closed
					}
				}
				else // if it is a setup group do nothing
					H5Gclose(grpid); 
				//H5Gclose(grpid);
				break;
			case H5G_DATASET:
				break;
			case H5G_TYPE:
				break;
			default:
				break;
		}

	}

	H5Gclose(root_group_id);
}


int BDV_HDF5_fdescr_t::writeHyperslab ( int tp, int s, int r, iim::uint8 *buf, hsize_t *dims_buf, hsize_t *hl_buf, hsize_t *hl_file ) {

	hsize_t len;
	char gname[MAX_NAME];

	hid_t data_group_id;
	hid_t data_subgroup_id;
	hid_t dataset_id;
	hid_t datatype_id;
	hid_t dataspace_id;
	hid_t bufspace_id;

	H5T_class_t class_id;
	hsize_t size; // data size (#bytes)
	int rank;   // dataset #dimensions
	
	hsize_t dims_file[3]; // dataset dimensions
	hsize_t start_file[3];    // dataset dimensions
	hsize_t count_file[3];    // dataset dimensions

	hsize_t start_buf[3];    // dataset dimensions
	hsize_t count_buf[3];    // dataset dimensions

	herr_t status;

    /*
     * Define the memory dataspace.
     */
    bufspace_id = H5Screate_simple(3,dims_buf,NULL);   

    /* 
     * Define memory hyperslab. 
     */
    start_buf[0] = hl_buf[0];
    start_buf[1] = hl_buf[1];
    start_buf[2] = hl_buf[2];
    count_buf[0]  = hl_buf[6];
    count_buf[1]  = hl_buf[7];
    count_buf[2]  = hl_buf[8];
    status = H5Sselect_hyperslab(bufspace_id, H5S_SELECT_SET, start_buf, NULL, 
				 count_buf, NULL);

	// get name of setup to be written
	len = H5Iget_name(setup_groups_id[s], gname, (size_t)MAX_NAME );
	data_group_id = H5Gopen(tp_groups_id[tp],gname+1,H5P_DEFAULT);

	// get group (resolution) of dataset
	std::stringstream res;
	res << r;
	data_subgroup_id = H5Gopen(data_group_id, res.str().c_str(), H5P_DEFAULT);

	// get dataset
	dataset_id = H5Dopen(data_subgroup_id,"cells",H5P_DEFAULT);

    /*
     * Get datatype and dataspace handles and then query
     * dataset class, order, size, rank and dimensions.
     */
    datatype_id  = H5Dget_type(dataset_id);     /* datatype handle */ 
    class_id     = H5Tget_class(datatype_id);
    //if (_class == H5T_INTEGER) printf("Data set has INTEGER type \n");
    //order     = H5Tget_order(datatype);
    //if (order == H5T_ORDER_LE) printf("Little endian order \n");

	// number of bytes of the data type
    size  = H5Tget_size(datatype_id);

    dataspace_id = H5Dget_space(dataset_id);    /* dataspace handle */
    rank      = H5Sget_simple_extent_ndims(dataspace_id);
    status  = H5Sget_simple_extent_dims(dataspace_id, dims_file, NULL);
    
    /* 
     * Define hyperslab in the dataset. 
     */
    if ( hl_file ) // data have not to be appended
		throw iim::IOException(iim::strprintf("File hiperslab provided: only append operation is supported").c_str(),__iim__current__function__);
    else { // append slice
    	start_file[0] = n_slices[tp][s][r];
    	start_file[1] = 0;
    	start_file[2] = 0;
		count_file[0]  = count_buf[0];
		count_file[1]  = count_buf[1];
		count_file[2]  = count_buf[2];
		status = H5Sselect_hyperslab(dataspace_id, H5S_SELECT_SET, start_file, NULL, 
				 count_file, NULL);

		/*
		 * Write the data to the hyperslab.
		 */
		status = H5Dwrite(dataset_id, vxl_type, bufspace_id, dataspace_id, H5P_DEFAULT, buf);

		n_slices[tp][s][r] += count_buf[0];
	}

	H5Tclose(datatype_id);
	H5Sclose(bufspace_id);
	H5Sclose(dataspace_id);
	H5Dclose(dataset_id);
	H5Gclose(data_subgroup_id);
	H5Gclose(data_group_id);

	return 0;
}


hid_t *BDV_HDF5_fdescr_t::getDATASETS_ID ( int tp, int r ) {
	int len;
	char gname[MAX_NAME];
	hid_t tp_subgroup_id;
	hid_t tp_subsubgroup_id;

	hid_t *dsets_id = new hid_t[n_setups];

	for ( int s=0; s<n_setups; s++ ) {
		/* Open the setup dataset group of time point tp */
		len = H5Iget_name(setup_groups_id[s], gname, (size_t)MAX_NAME );
		tp_subgroup_id = H5Gopen2(tp_groups_id[tp], gname+1, H5P_DEFAULT); // skip the initial '/'
		/* Open the dataset at resolution r */
		std::stringstream res;
		res << r;
		tp_subsubgroup_id = H5Gopen2(tp_subgroup_id, res.str().c_str(), H5P_DEFAULT);
		dsets_id[s] = H5Dopen2(tp_subsubgroup_id,"cells",H5P_DEFAULT);

		H5Gclose(tp_subsubgroup_id);
		H5Gclose(tp_subgroup_id);
	}

	return dsets_id;
}


void readSubvol ( int V0, int V1, int H0, int H1, int D0, int D1, int setup, unsigned char *buf ) {
}

#endif // ENABLE_BDV_HDF5


/****************************************************************************
* HDF5 Manager implementation
****************************************************************************/

void BDV_HDF5init ( std::string fname, void *&descr, int vxl_nbytes ) throw (iim::IOException) {
#ifdef ENABLE_BDV_HDF5
	BDV_HDF5_fdescr_t *int_descr = new BDV_HDF5_fdescr_t(fname.c_str(),vxl_nbytes);
	descr = int_descr;
#else
	throw iim::IOException(iim::strprintf(
			"Support to BDV_HDF5 files not available: please verify there is are valid hdf5 static libs (hdf5 and szip) "
			"in ""3rdparty/libs"" directory and set the ""ENABLED_BDV_HDF5"" checkbox before configuring CMake project").c_str(),__iim__current__function__);
#endif
}


int BDV_HDF5n_resolutions ( void *descr ) { 
#ifdef ENABLE_BDV_HDF5
	return ((BDV_HDF5_fdescr_t *) descr)->getN_RES();
#else
	throw iim::IOException(iim::strprintf(
			"Support to BDV_HDF5 files not available: please verify there is are valid hdf5 static libs (hdf5 and szip) "
			"in ""3rdparty/libs"" directory and set the ""ENABLED_BDV_HDF5"" checkbox before configuring CMake project").c_str(),__iim__current__function__);
#endif
}


void BDV_HDF5close ( void *descr ) {
#ifdef ENABLE_BDV_HDF5
	delete (BDV_HDF5_fdescr_t *) descr;
#else
	throw iim::IOException(iim::strprintf(
			"Support to BDV_HDF5 files not available: please verify there is are valid hdf5 static libs (hdf5 and szip) "
			"in ""3rdparty/libs"" directory and set the ""ENABLED_BDV_HDF5"" checkbox before configuring CMake project").c_str(),__iim__current__function__);
#endif
}


void BDV_HDF5addSetups ( void *file_descr, iim::sint64 height, iim::sint64 width, iim::sint64 depth, 
				 float vxlszV, float vxlszH, float vxlszD, bool *res, int res_size, int chans, int block_height, int block_width, int block_depth ) {
#ifdef ENABLE_BDV_HDF5

	BDV_HDF5_fdescr_t *int_descr = (BDV_HDF5_fdescr_t *) file_descr;

	if ( int_descr->getN_SETUPS() ) { // file already contains timepoints: check consistency of additional data
		throw iim::IOException(iim::strprintf("updating already existing files not supported yet").c_str(),__iim__current__function__);
	}
	else { // file is empty create setups descriptors
		for ( int c=0; c<chans; c++ )
			if ( int_descr->addSetup(c) != c )
				throw iim::IOException(iim::strprintf("cannot add setup %d",c).c_str(),__iim__current__function__);
		for ( int r=0; r<res_size; r++ )
			if ( res[r] )
				if ( int_descr->addResolution(r,vxlszV*iim::powInt(2,r),vxlszH*iim::powInt(2,r),vxlszD*iim::powInt(2,r),
							height/iim::powInt(2,r),width/iim::powInt(2,r),depth/iim::powInt(2,r),block_height, block_width, block_depth) != r )
					throw iim::IOException(iim::strprintf("cannot add resolution %d",r).c_str(),__iim__current__function__);
	}
#else
	throw iim::IOException(iim::strprintf(
			"Support to BDV_HDF5 files not available: please verify there is are valid hdf5 static libs (hdf5 and szip) "
			"in ""3rdparty/libs"" directory and set the ""ENABLED_BDV_HDF5"" checkbox before configuring CMake project").c_str(),__iim__current__function__);
#endif
}


void BDV_HDF5addTimepoint ( void *file_descr, int tp ) {
#ifdef ENABLE_BDV_HDF5
	BDV_HDF5_fdescr_t *int_descr = (BDV_HDF5_fdescr_t *) file_descr;

	if ( int_descr->addTimePoint(tp) != tp )
		throw iim::IOException(iim::strprintf("cannot add time point %d",tp).c_str(),__iim__current__function__);
#else
	throw iim::IOException(iim::strprintf(
			"Support to BDV_HDF5 files not available: please verify there is are valid hdf5 static libs (hdf5 and szip) "
			"in ""3rdparty/libs"" directory and set the ""ENABLED_BDV_HDF5"" checkbox before configuring CMake project").c_str(),__iim__current__function__);
#endif
}


void BDV_HDF5writeHyperslab ( void *file_descr, iim::uint8 *buf, iim::sint64 *dims_buf, iim::sint64 *hl, int r, int s, int tp ) {
#ifdef ENABLE_BDV_HDF5
	BDV_HDF5_fdescr_t *int_descr = (BDV_HDF5_fdescr_t *) file_descr;

	if ( int_descr->writeHyperslab(tp,s,r,buf,(hsize_t *)dims_buf,(hsize_t *)hl) )
		throw iim::IOException(iim::strprintf("cannot add hyperslab: buffer size=(%d,%d,%d), hyperslab=[offset=(%d,%d,%d), stride=(%d,%d,%d), size=(%d,%d,%d), block=(%d,%d,%d)]",
									dims_buf[0],dims_buf[1],dims_buf[2],hl[0],hl[1],hl[2],hl[3],hl[4],hl[5],hl[6],hl[7],hl[8],hl[9],hl[10],hl[11]).c_str(),__iim__current__function__);
#else
	throw iim::IOException(iim::strprintf(
			"Support to BDV_HDF5 files not available: please verify there is are valid hdf5 static libs (hdf5 and szip) "
			"in ""3rdparty/libs"" directory and set the ""ENABLED_BDV_HDF5"" checkbox before configuring CMake project").c_str(),__iim__current__function__);
#endif
}



void BDV_HDF5getVolumeInfo ( void *descr, int tp, int res, void *&volume_descr, 
								float &VXL_1, float &VXL_2, float &VXL_3, 
								float &ORG_V, float &ORG_H, float &ORG_D, 
								iim::uint32 &DIM_V, iim::uint32 &DIM_H, iim::uint32 &DIM_D,
							    int &DIM_C, int &BYTESxCHAN, int &DIM_T, int &t0, int &t1 ) {
#ifdef ENABLE_BDV_HDF5
	
	BDV_HDF5_fdescr_t *int_descr = (BDV_HDF5_fdescr_t *) descr;
	BDV_volume_descr_t *int_volume_descr = new BDV_volume_descr_t;

	// in HDF5 dimensions are ordered as DVH
	VXL_1 = (float) int_descr->getVXL_SZ(res)[1];
	VXL_2 = (float) int_descr->getVXL_SZ(res)[2];
	VXL_3 = (float) int_descr->getVXL_SZ(res)[0];

	ORG_V = ORG_H = ORG_D = 0.0;

	DIM_V = int_descr->getVOL_DIMS(res)[1];
	DIM_H = int_descr->getVOL_DIMS(res)[2];
	DIM_D = int_descr->getVOL_DIMS(res)[0];

	DIM_C = int_descr->getN_SETUPS();
	BYTESxCHAN = int_descr->getVXL_NBYTES();

	DIM_T = int_descr->getN_TPS();
	t0 = t1 = 0;

	int_volume_descr->n_setups   = int_descr->getN_SETUPS();
	int_volume_descr->vxl_type   = int_descr->getVXL_TYPE();
	int_volume_descr->vxl_nbytes = int_descr->getVXL_NBYTES();

	int_volume_descr->datasets_id = int_descr->getDATASETS_ID(tp,res);

	volume_descr = int_volume_descr;
#else
	throw iim::IOException(iim::strprintf(
			"Support to BDV_HDF5 files not available: please verify there is are valid hdf5 static libs (hdf5 and szip) "
			"in ""3rdparty/libs"" directory and set the ""ENABLED_BDV_HDF5"" checkbox before configuring CMake project").c_str(),__iim__current__function__);
#endif
}


void BDV_HDF5getSubVolume ( void *descr, int V0, int V1, int H0, int H1, int D0, int D1, int setup, iim::uint8 *buf, int red_factor ) {
#ifdef ENABLE_BDV_HDF5

	BDV_volume_descr_t *int_volume_descr = (BDV_volume_descr_t *) descr;
	
	hsize_t start_buf[3];     
	hsize_t start_file[3]; 
	hsize_t dims_buf[3];
	herr_t status;

	dims_buf[0] = D1 - D0;
	dims_buf[1] = V1 - V0;
	dims_buf[2] = H1 - H0;
	
	/* Get filespace handle first. */
	hid_t filespace_id = H5Dget_space(int_volume_descr->datasets_id[setup]);    /* dataspace handle */
	//int rank      = H5Sget_simple_extent_ndims(filespace_id);
	//hsize_t dims[3];   
	//status  = H5Sget_simple_extent_dims(filespace_id, dims, NULL);
    start_file[0] = D0;
    start_file[1] = V0;
    start_file[2] = H0;
	status = H5Sselect_hyperslab(filespace_id, H5S_SELECT_SET, start_file, NULL, 
				dims_buf, NULL);

	/* Create memory space handle. */
	hid_t bufspace_id = H5Screate_simple(3,dims_buf,NULL); 
	start_buf[0] = 0;
	start_buf[1] = 0;
	start_buf[2] = 0;
    status = H5Sselect_hyperslab(bufspace_id, H5S_SELECT_SET, start_buf, NULL, 
				 dims_buf, NULL);

	if ( int_volume_descr->vxl_nbytes == 1 ) {
		status = H5Dread(int_volume_descr->datasets_id[setup],int_volume_descr->vxl_type,bufspace_id,filespace_id,H5P_DEFAULT,buf);
	}
	else if ( int_volume_descr->vxl_nbytes == 2 ) {
		
		if ( red_factor == 1 ) /* 2015-12-29. Giulio. @ADDED no conversion from 16 to 8 bits depth */ { 
			status = H5Dread(int_volume_descr->datasets_id[setup],int_volume_descr->vxl_type,bufspace_id,filespace_id,H5P_DEFAULT,buf);
		}
		else if ( red_factor == 2 ) { 
			iim::sint64 sbv_ch_dim = dims_buf[0] * dims_buf[1] * dims_buf[2];
			iim::uint8 *tempbuf = new iim::uint8[red_factor * sbv_ch_dim];
			status = H5Dread(int_volume_descr->datasets_id[setup],int_volume_descr->vxl_type,bufspace_id,filespace_id,H5P_DEFAULT,tempbuf);

			// convert to 8 bit and copy to buf
			char *err_rawfmt;
			if ( (err_rawfmt = convert2depth8bits(int_volume_descr->vxl_nbytes, sbv_ch_dim, 1, tempbuf, buf)) != 0 )
				throw iim::IOException(iim::strprintf("cannot convert buffer from %d bits to 8 bits (%s)",8*int_volume_descr->vxl_nbytes,err_rawfmt).c_str(),__iim__current__function__);
		}
		else 
  			throw iim::IOException(iim::strprintf("%d wrong reduction factor (%d) for 16 bits images", red_factor), __iim__current__function__);
	}

	H5Sclose(bufspace_id);
	H5Sclose(filespace_id);
#else
	throw iim::IOException(iim::strprintf(
			"Support to BDV_HDF5 files not available: please verify there is are valid hdf5 static libs (hdf5 and szip) "
			"in ""3rdparty/libs"" directory and set the ""ENABLED_BDV_HDF5"" checkbox before configuring CMake project").c_str(),__iim__current__function__);
#endif
}

void BDV_HDF5closeVolume ( void *descr ) {
#ifdef ENABLE_BDV_HDF5
	BDV_volume_descr_t *int_volume_descr = (BDV_volume_descr_t *) descr;
	if ( int_volume_descr->datasets_id )
		delete int_volume_descr->datasets_id;
	delete (BDV_volume_descr_t *) descr;
#else
	throw iim::IOException(iim::strprintf(
			"Support to BDV_HDF5 files not available: please verify there is are valid hdf5 static libs (hdf5 and szip) "
			"in ""3rdparty/libs"" directory and set the ""ENABLED_BDV_HDF5"" checkbox before configuring CMake project").c_str(),__iim__current__function__);
#endif
}

