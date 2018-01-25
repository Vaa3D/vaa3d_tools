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
* 2017-09-09. Giulio. @ADDED code to manage compression algorithms to be used in Imaris IMS files generation
* 2017-06-27. Giulio  @ADDED code for managing the addition of timepoints to an existing file (many changes search for '2017-06-27')
* 2017-04-22. Giulio  @ADDED adjustment in the file structure inheroted from another file 
* 2017-04-20. Giulio  @CHANGED creation of a default file strucure 
* 2017-04-20. Giulio. @FIXED HDF5 error messages when resolutions were added after the first
* 2017-04-20. Giulio. @ADDED an operation to adjust the object list
* 2017-04-17. Giulio. @ADDED generation of a default file structure 
* 2017-04-17. Giulio. @ADDED in 'IMS_HDF5init' file structure is extracted only upon request
* 2017-04-09. Giulio. @FIXED include the correct header file 'IMS_HDF5Mngr.h'
* 2017-04-08. Giulio. @ADDED support for additional attributes required by the IMS format
* 2016-10-27. Giulio. @FIXED n_slices had been initialized to NULL pointers
* 2016-10-04. Giulio. @CREATED 
*/

#include "IMS_HDF5Mngr.h"
#include "RawFmtMngr.h"
#include <stdlib.h> // needed by clang: defines size_t
#include <sstream>
#include <algorithm>


#ifdef ENABLE_IMS_HDF5
#include "hdf5.h"
#endif

#ifdef _VAA3D_TERAFLY_PLUGIN_MODE
#include <QElapsedTimer>
#include "PLog.h"
#include "COperation.h"
#endif


#ifdef ENABLE_IMS_HDF5

#define HDF5_SUFFIX   "ims"

#include <string.h>
#include "iomanager.config.h"


// code to get time in string format
#include <time.h>

static char time_str[1024];
static char *get_time_str ( ) {
	time_t rawtime;
	time(&rawtime);
	strftime(time_str,1024,"%Y-%m-%d %H:%M:%S.000",localtime(&rawtime));
	return time_str;
}


#define FILTER_NOTHING 0
#define FILTER_GZIP 1
#define FILTER_SZIP 2
#define DELIMITATOR ":"


#define MAXSTP   10     // maximim number of channels (aka setups)
#define MAXRES   10     // maximim number of resolutions
#define MAXTPS   1024   // maximum number of timepoints

#define DEF_CHNK_DIM_XY   128
#define DEF_CHNK_DIM_Z   8

#define MAX_NAME 1024

typedef double vxl_size_t[3];
typedef hsize_t dims_t[3];
typedef int subdvsns_t[3];

const char *excluded_attrs[] = { 
	// CustomData attributes
	"DimensionInformation",
	//"Height",
	"Left",
	//"Width",
	//"XPosition",
	//"YPosition",
	// Image attributes
	"ExtMax0",
	"ExtMax1",
	"ExtMax2",
	"ExtMin0",
	"ExtMin1",
	"ExtMin2",
	"X",
	"Y",
	"Z", 
	// end list
	"__stop__"     
};

/* WARNING:
 * Currently assumed that:
 * X (index 2) correspond to H, and Y (index 1) to V. THIS SHOULD BE CHECKED
 * Z (index 0) corrsponds as usual to D
 */
 
/* WARNINIG:
 * current implementation assumes that all chans from 0 to i-1 have been created before creating chan i
 * current implementation assumes that all time points from 0 to i-1 have been created before creating timepoint i
 */


/****************************************************************************
* IMS-HDF5 procedures for exploring file structures
****************************************************************************/

#include <map>

static int obj_info_cnt = 0;            // check if obj_info records have been all released

struct IMS_obj_info_t;

typedef std::map<std::string, IMS_obj_info_t> IMS_obj_list_t;
typedef std::map<std::string, std::string> IMS_attr_list_t;


struct IMS_obj_info_t {
	int otype;
	IMS_obj_list_t *olist;
	IMS_attr_list_t *alist;

	IMS_obj_info_t ( int _otype, IMS_obj_list_t *_olist, IMS_attr_list_t *_alist ) {
		otype = _otype;
	    olist = _olist;
		alist = _alist;
		//obj_info_cnt++;
	}

	// @FIXED by Alessandro on 2016-12-01: default constructor required to use this class within STL std::map
	IMS_obj_info_t(){
		otype = -1;
		olist = (IMS_obj_list_t *) 0;
		alist = (IMS_attr_list_t *) 0;
	}

	// this copy constructor must used to pass by value parameters created on the fly that will be immediately released
	IMS_obj_info_t ( const IMS_obj_info_t &ex ) {
		otype = ex.otype;
		// either assign a copy 
		olist = new IMS_obj_list_t(*ex.olist);
	    alist = new IMS_attr_list_t(*ex.alist);
		// or pass ownership (more efficient if ex is immediately released)
		//olist = ex.olist;
		//alist = ex.alist;
		// and prevent from deleting passed objects (requires ex be modified: this does not compile on mac)
		//ex.olist = (IMS_obj_list_t *) 0;
		//ex.alist = (IMS_attr_list_t *) 0;
		//obj_info_cnt++;
	}

	~IMS_obj_info_t ( ) {
		if ( olist )
			delete olist;
		if ( alist )
			delete alist;
		//obj_info_cnt--;
	}
};


// return a standard list of attributed of root object
IMS_attr_list_t *build_std_rootattributes ( ) {

	IMS_attr_list_t *alist = new IMS_attr_list_t;

	alist->insert(std::make_pair("DataSetDirectoryName","DataSet"));
	alist->insert(std::make_pair("DataSetInfoDirectoryName","DataSetInfo"));
	alist->insert(std::make_pair("ThumbnailDirectoryName","Thumbnail"));
	alist->insert(std::make_pair("ImarisDataSet","ImarisDataSet"));
	alist->insert(std::make_pair("ImarisVersion","5.5.0"));

	return alist;
}


// return a standard IMS file structure
IMS_obj_list_t *build_std_filestruct ( std::string fname, int height, int width, int z_points, int n_chans = 1, int n_timepoints = 1, float abs_V = 0.0, float abs_H = 0.0 ) {

	IMS_obj_list_t *olist;
	IMS_attr_list_t *alist;
	char num_str[256];

	IMS_obj_list_t *rootlist = new IMS_obj_list_t;

	rootlist->insert(std::make_pair("DataSet",IMS_obj_info_t(H5G_GROUP,new IMS_obj_list_t,new IMS_attr_list_t)));

	olist = new IMS_obj_list_t; // DataSetInfo list

	alist = new IMS_attr_list_t; // CustomData attribute info list
	alist->insert(std::make_pair("DateAndTime",std::string(get_time_str())));
	sprintf(num_str,"%u",height);
	alist->insert(std::make_pair("Height",std::string(num_str)));
	sprintf(num_str,"%u",width);
	alist->insert(std::make_pair("Width",std::string(num_str)));
	sprintf(num_str,"%u",z_points);
	alist->insert(std::make_pair("NumberOfZPoints",std::string(num_str)));
	sprintf(num_str,"%u",n_chans);
	alist->insert(std::make_pair("NumberOfChannels",std::string(num_str)));
	sprintf(num_str,"%u",n_timepoints);
	alist->insert(std::make_pair("NumberOfTimePoints",std::string(num_str)));
	sprintf(num_str,"%.2f",abs_H);
	alist->insert(std::make_pair("XPosition",std::string(num_str)));
	sprintf(num_str,"%.2f",abs_V);
	alist->insert(std::make_pair("YPosition",std::string(num_str)));
	olist->insert(std::make_pair("CustomData",IMS_obj_info_t(H5G_GROUP,new IMS_obj_list_t,alist)));

	alist = new IMS_attr_list_t; // ImarisDataSet attribute info list
	alist->insert(std::make_pair("Creator","(creator not specified)"));
	alist->insert(std::make_pair("NumberOfImages","1"));
	alist->insert(std::make_pair("Version","5.5"));
	olist->insert(std::make_pair("ImarisDataSet",IMS_obj_info_t(H5G_GROUP,new IMS_obj_list_t,alist)));

	alist = new IMS_attr_list_t; // Image attribute info list
	alist->insert(std::make_pair("Description","(description not specified)"));
	alist->insert(std::make_pair("Name",fname));
	alist->insert(std::make_pair("RecordingDate",get_time_str()));
	alist->insert(std::make_pair("Unit","um"));
	olist->insert(std::make_pair("Image",IMS_obj_info_t(H5G_GROUP,new IMS_obj_list_t,alist)));

	for ( int c=0; c<n_chans; c++ ) {
		alist = new IMS_attr_list_t; // ChannelX attribute info list
		alist->insert(std::make_pair("Description",""));
		sprintf(num_str,"%u",c+1);
		alist->insert(std::make_pair("Name","Channel " + std::string(num_str)));
		sprintf(num_str,"%u",c);
		olist->insert(std::make_pair("Channel " + std::string(num_str),IMS_obj_info_t(H5G_GROUP,new IMS_obj_list_t,alist)));
	}

	alist = new IMS_attr_list_t; // Log attribute info list
	alist->insert(std::make_pair("Entries","0"));
	olist->insert(std::make_pair("Log",IMS_obj_info_t(H5G_GROUP,new IMS_obj_list_t,alist)));

	alist = new IMS_attr_list_t; // TimeInfo attribute info list
	sprintf(num_str,"%u",n_timepoints);
	alist->insert(std::make_pair("DataSetTimePoints",std::string(num_str)));
	alist->insert(std::make_pair("FileTimePoints",std::string(num_str)));
	for ( int t=1; t<=n_timepoints; t++ ) {
		sprintf(num_str,"%u",1);
		alist->insert(std::make_pair("TimePoint" + std::string(num_str),std::string(get_time_str())));
	}
	olist->insert(std::make_pair("TimeInfo",IMS_obj_info_t(H5G_GROUP,new IMS_obj_list_t,alist)));

	rootlist->insert(std::make_pair("DataSetInfo",IMS_obj_info_t(H5G_GROUP,olist,new IMS_attr_list_t)));

	rootlist->insert(std::make_pair("Thumbnail",IMS_obj_info_t(H5G_GROUP,new IMS_obj_list_t,new IMS_attr_list_t)));

	return rootlist;
}


// return the list of attributed of object obj
IMS_attr_list_t *get_attr_list ( hid_t obj ) {

	herr_t status;
	hsize_t nattr;
	size_t len;
	char name[MAX_NAME];
	//char *attr_name;
	char *value_str;
	hid_t attrid;
	hid_t typid;
	hid_t spaceid;

	IMS_attr_list_t *alist = new IMS_attr_list_t;

	nattr = H5Aget_num_attrs(obj);

	for ( unsigned int i=0; i<nattr; i++ ) {
		attrid = H5Aopen_idx(obj, (hsize_t)i);
		H5Aget_name(attrid,MAX_NAME,name);
		//attr_name = strdup(name);
		typid = H5Aget_type(attrid);
		if ( H5Tget_class(typid) != H5T_STRING )
			throw iim::IOException(iim::strprintf("No string type: all attributes should be strings of characters").c_str(),__iim__current__function__);
		spaceid = H5Aget_space(attrid);
		len = H5Sget_simple_extent_npoints(spaceid);
		value_str = new char[len+1];
		status = H5Aread(attrid,typid,value_str);
		value_str[len] = '\0';
		alist->insert(std::make_pair(name,value_str));
		delete value_str;
	}

	return alist;
}

// return the list of attributed of root object
IMS_attr_list_t *get_root_attributes ( hid_t root ) {

	herr_t status;
	hsize_t nattr;
	size_t len;
	char name[MAX_NAME];
	//char *attr_name;
	char *value_str;
	hid_t attrid;
	hid_t typid;
	hid_t spaceid;

	IMS_attr_list_t *alist = new IMS_attr_list_t;

	nattr = H5Aget_num_attrs(root);

	for ( unsigned int i=0; i<nattr; i++ ) {
		attrid = H5Aopen_idx(root, (hsize_t)i);
		H5Aget_name(attrid,MAX_NAME,name);
		if ( strcmp(name,"ImarisDataSet") == 0 || strcmp(name,"ImarisVersion") == 0) {
			// is an attribute requested by the format IMARIS 5.5 (IMS)
			typid = H5Aget_type(attrid);
			if ( H5Tget_class(typid) != H5T_STRING )
				throw iim::IOException(iim::strprintf("No string type: all attributes should be strings of characters").c_str(),__iim__current__function__);
			spaceid = H5Aget_space(attrid);
			len = H5Sget_simple_extent_npoints(spaceid);
			value_str = new char[len+1];
			status = H5Aread(attrid,typid,value_str);
			value_str[len] = '\0';
			alist->insert(std::make_pair(name,value_str));
			delete value_str;
		}
	}

	alist->insert(std::make_pair("DataSetDirectoryName","DataSet"));
	alist->insert(std::make_pair("DataSetInfoDirectoryName","DataSetInfo"));
	alist->insert(std::make_pair("ThumbnailDirectoryName","Thumbnail"));

	return alist;
}

// return the list of objects in group 
IMS_obj_list_t *get_obj_list ( hid_t group ) {

	herr_t status;
	hsize_t nobj;
	size_t len;
	int otype;
	char name[MAX_NAME];
	//char *obj_name;
	hid_t subgroup;
	
	IMS_obj_list_t *olist = new IMS_obj_list_t;

	status = H5Gget_num_objs(group, &nobj);

	for ( int i=0; i<nobj; i++ ) {
		len = H5Gget_objname_by_idx(group, (hsize_t)i, name, (size_t)MAX_NAME );
		otype =  H5Gget_objtype_by_idx(group, (size_t)i);
		switch(otype) {
			case H5G_LINK:
				break;
			case H5G_GROUP:
				//obj_name = strdup(name);
				subgroup = H5Gopen(group,name, H5P_DEFAULT);
				olist->insert(std::make_pair(name,IMS_obj_info_t(otype,get_obj_list(subgroup),get_attr_list(subgroup))));
				break;
			case H5G_DATASET:
				break;
			case H5G_TYPE:
				break;
			default:
				break;
		}

	}

	return olist;
}

// adjust the file structure retrieved by another file
IMS_obj_list_t *adjust_obj_list ( IMS_obj_list_t *olist, std::string fname, int height, int width, int z_points, iim::uint32 *chans, int n_chans, float abs_V = 0.0, float abs_H = 0.0 ) {

	char num_str[256];
	IMS_attr_list_t *attr_list;

	IMS_obj_list_t *dsi_obj_list = (*olist)["DataSetInfo"].olist;
	
	// adjust Channel information
	IMS_attr_list_t *chan_attr_list[MAXSTP]; // list of channels attribute lists
	std::string chan_names_list[MAXSTP];
	int org_n_chans = 0;
	for ( IMS_obj_list_t::iterator it = dsi_obj_list->begin(); it != dsi_obj_list->end(); it++ ) {
		if ( it->first.substr(0,7) == "Channel" ) {
			org_n_chans++;
			chan_attr_list[atoi(it->first.substr(7).c_str())] = it->second.alist;
			chan_names_list[atoi(it->first.substr(7).c_str())] = it->first;
			it->second.alist = (IMS_attr_list_t *) 0;
		}
	}
	for ( int c=0; c<org_n_chans; c++ ) {
		if ( chan_names_list[c].substr(0,7) == "Channel" ) 
			dsi_obj_list->erase(chan_names_list[c]);
		else
			throw iim::IOException(iim::strprintf("Missing number in names of channel groups (%d)",c).c_str(),__iim__current__function__);
	}
	for ( int c=0; c<n_chans; c++ ) {
		sprintf(num_str,"%u",c);
		dsi_obj_list->insert(std::make_pair("Channel " + std::string(num_str),IMS_obj_info_t(H5G_GROUP,new IMS_obj_list_t,chan_attr_list[chans[c]])));
	}

	// adjust DataSetInfo
	if ( dsi_obj_list->find("CustomData") != dsi_obj_list->end() ) { // there is group CustomData: adjust and complete its attributes
		attr_list = (*dsi_obj_list)["CustomData"].alist;
		if ( attr_list->find("DateAndTime") != attr_list->end() ) 
			attr_list->erase("DateAndTime");
		attr_list->insert(std::make_pair("DateAndTime",std::string(get_time_str())));
		if ( attr_list->find("Height") != attr_list->end() ) 
			attr_list->erase("Height");
		sprintf(num_str,"%u",height);
		attr_list->insert(std::make_pair("Height",std::string(num_str)));
		if ( attr_list->find("Width") != attr_list->end() ) 
			attr_list->erase("Width");
		sprintf(num_str,"%u",width);
		attr_list->insert(std::make_pair("Width",std::string(num_str)));
		if ( attr_list->find("NumberOfZPoints") != attr_list->end() ) 
			attr_list->erase("NumberOfZPoints");
		sprintf(num_str,"%u",z_points);
		attr_list->insert(std::make_pair("NumberOfZPoints",std::string(num_str)));
		if ( attr_list->find("NumberOfChannels") != attr_list->end() ) 
			attr_list->erase("NumberOfChannels");
		sprintf(num_str,"%u",n_chans);
		attr_list->insert(std::make_pair("NumberOfChannels",std::string(num_str)));
		if ( attr_list->find("NumberOfTimePoints") != attr_list->end() ) 
			attr_list->erase("NumberOfTimePoints");
		sprintf(num_str,"%u",1); // assume 1 time point
		attr_list->insert(std::make_pair("NumberOfTimePoints",std::string(num_str)));
		if ( attr_list->find("XPosition") != attr_list->end() ) 
			attr_list->erase("XPosition");
		sprintf(num_str,"%.2f",abs_H);
		attr_list->insert(std::make_pair("XPosition",std::string(num_str)));
		if ( attr_list->find("YPosition") != attr_list->end() ) 
			attr_list->erase("YPosition");
		sprintf(num_str,"%.2f",abs_V);
		attr_list->insert(std::make_pair("YPosition",std::string(num_str)));
	}
	
	// adjust Image
	if ( dsi_obj_list->find("Image") != dsi_obj_list->end() ) { // there is group Image: adjust and complete its attributes
		attr_list = (*dsi_obj_list)["Image"].alist;
		if ( attr_list->find("RecordingDate") != attr_list->end() ) 
			attr_list->erase("RecordingDate");
		attr_list->insert(std::make_pair("RecordingDate",std::string(get_time_str())));
		if ( attr_list->find("Name") != attr_list->end() ) 
			attr_list->erase("Name");
		attr_list->insert(std::make_pair("Name",fname));
	}
		
	return olist;
}

//creates and associates the attributes <name,value_str> to node parent
herr_t create_string_attribute ( hid_t parent, const char *name, const char *value_str ) {

	herr_t status;

	hid_t attr_id;
	hid_t type_id;
	hid_t dspace_id;
	hsize_t len[1];

	type_id = H5Tcopy(H5T_C_S1);
	status = H5Tset_size(type_id, 1);
	dspace_id = H5Screate(H5S_SIMPLE);
	len[0] = strlen(value_str);
	status = H5Sset_extent_simple(dspace_id,1,len,len);
	if ( (attr_id = H5Acreate2(parent,name, type_id,dspace_id,H5P_DEFAULT,H5P_DEFAULT)) < 0 ) {
		// the attribute already exist: do nothing
		status = H5Sclose(dspace_id);
		status = H5Tclose(type_id);
		iim::warning(iim::strprintf("attribute %s already exists",name).c_str());
	}
	status = H5Awrite(attr_id, type_id,value_str);
	status = H5Sclose(dspace_id);
	status = H5Tclose(type_id);
	status = H5Aclose(attr_id);

	return status;
}

//creates and associates the list of attributes alist to node parent
herr_t create_attributes ( hid_t parent, IMS_attr_list_t *alist ) {

	herr_t status = 0;

	IMS_attr_list_t::iterator ita;
	for ( ita = alist->begin(); ita != alist->end(); ita++ ) {
		const char *tmp = ita->first.c_str();
		bool copy = true;
		int i = 0;
		while ( strcmp(excluded_attrs[i],"__stop__") != 0 && copy ) {
			if ( strcmp(tmp,excluded_attrs[i]) == 0 )
				copy = false;
			else
				i++;
		}
		if ( copy ) {
		//if ( strcmp(tmp,"Height") && strcmp(tmp,"Left") &&strcmp(tmp,"Width") && strcmp(tmp,"XPosition") && strcmp(tmp,"YPosition") &&                              // ChannelData metadata
		//	 strcmp(tmp,"ExtMax0") && strcmp(tmp,"ExtMax1") &&strcmp(tmp,"ExtMax2") && strcmp(tmp,"ExtMin0") && strcmp(tmp,"ExtMin1") &&  strcmp(tmp,"ExtMin2") &&  // Image metadata
		//	 strcmp(tmp,"X") && strcmp(tmp,"Y") &&strcmp(tmp,"Z") &&                                                                                                // Image metadata
		//	 strcmp(tmp,"") ) { // dummy value
			status = create_string_attribute(parent,ita->first.c_str(),ita->second.c_str());
		}
	}

	return status;
}

// creates the list of objects olist with their attributes in group group_id
herr_t create_file_struct ( hid_t group_id, IMS_obj_list_t *olist ) {

	herr_t status = 0;
	hid_t subgroup_id;

	IMS_obj_list_t::iterator ito;
	for ( ito = olist->begin(); ito != olist->end(); ito++ ) {
		if ( ito->second.otype == H5G_GROUP ) {
			if ( strcmp(ito->first.c_str(),"TimestampsPerFrame") != 0 ) { // // 2017-04-21. Giulio. excluded group 'TimestampsPerFrame'
				subgroup_id = H5Gcreate2(group_id, ito->first.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
				status = create_attributes(subgroup_id,ito->second.alist);
				const char *tmp = ito->first.c_str();
				if ( strcmp(tmp,"DataSet") != 0 ) { 
					status = create_file_struct(subgroup_id,ito->second.olist);
				}
				status = H5Gclose(subgroup_id);
			}
		}
	}

	return status;
}




/****************************************************************************
* IMS-HDF5 file descriptor
****************************************************************************/

class IMS_HDF5_fdescr_t {
	char       *fname;           // complete file path and name
	hid_t       file_id;         // file handle
	hsize_t     n_res;           // number of resolutions
	hid_t      *res_groups_id;   // handles of resolution groups
	int         active_res;      // active resolution
	hsize_t     n_timepoints;    // number of time points
	hid_t      *tp_groups_id;    // handles of time point groups at resolution 'active_res'
	int         active_tp;       // active timepoint
	hsize_t     n_chans;         // number of chans
	hid_t      *chan_groups_id;  // handles of chan groups at resolution 'active_res'
	vxl_size_t *vxl_sizes;       // voxels sizes of each resolution
	dims_t     *vol_dims;        // volume dimensions at all resolutions
	subdvsns_t *chunk_dims;      // chunk dimensions of each resolution
	int         vxl_nbytes;      // number of bytes of each channel 
	hid_t       vxl_type;        // HDF5 type of voxel values
	hsize_t  ***n_slices;        // number of slices of each time point at each resolutions at each chan (all channels should have the same number of slices)

	IMS_obj_list_t *olist;       // hierarchy of objects in the file with their type, value and attributes
	IMS_attr_list_t *rootalist;  // list of attributes of root group ("/")

	// info required by IMS file format
	histogram_t ***hist;         // histograms of each time point at each resolutions at each chan
	iim::uint8 *thumbnail;
	iim::uint32 thumbnail_sz;
	bool creating;               // if true means that the object represent a file that is being created 
	
	// private methods
	void scan_root ( );
	void init_voxel_size ( IMS_obj_list_t *obj_info ); // compute and set voxel size at resolution 0 extracting data from obj_info (initialized with source metadata)

	herr_t get_string_attribute ( hid_t parent, const char *name, char *&value_str );

	IMS_obj_list_t *getOLIST ( );
	IMS_attr_list_t *getROOTALIST ( );

public:
	IMS_HDF5_fdescr_t ( );
	/* default constructor: returns and empty descriptor */

	IMS_HDF5_fdescr_t ( const char *_fname, bool loadstruct = false, int _vxl_nbytes = 2, IMS_obj_list_t *obj_info = (IMS_obj_list_t *)0, 
		IMS_attr_list_t *root_attr_info = (IMS_attr_list_t *)0, int maxstp = MAXSTP, int maxres = MAXRES, int maxtps = MAXTPS );
	/*  */

	~IMS_HDF5_fdescr_t ( );
	/* close the HDF5 file and deallocates memory */
	
	// getters
	int      getACTIVE_RES ( )                    { return active_res; }
	int      getACTIVE_TP ( )                     { return active_tp; }
	int      getN_CHANS ( )                       { return (int)n_chans; }
	int      getN_TPS ( )                         { return (int)n_timepoints; }
	int      getN_RES ( )                         { return (int)n_res; }
	hid_t    getCHAN_GROUPS_ID ( int sg )         { return chan_groups_id[sg]; }
	hid_t    getTP_GROUPS_ID ( int tp )           { return tp_groups_id[tp]; }
	double  *getVXL_SZ ( int r )                  { return vxl_sizes[r]; }
	hsize_t *getVOL_DIMS ( int r )                { return vol_dims[r]; }
	int     *getCHUNK_DIMS ( int r )              { return chunk_dims[r]; }
	int      getVXL_NBYTES ( )                    { return vxl_nbytes; }  
	hsize_t  getVXL_TYPE ( )                      { return vxl_type; }
	hsize_t  getN_SLICES ( int tp, int s, int r ) { return n_slices[tp][s][r]; }

	hid_t *getDATASETS_ID ( int tp, int r, bool get = true );
	/* set tp and r as current timepoint and resolution
	 * returns a list of the n_chans handles of datasets of resolution r of time point tp
	   if get = true, otherwise returns a null pointer */

	void *extractOLIST ( );
	void *extractROOTALIST ( );

	// other operations	
	int addChan ( int s );
	/* add chan s to file */

	int addResolution ( int r, hsize_t dimV, hsize_t dimH, hsize_t dimD, int dimC, bool is_first = false );	
	/* add resolution r; other parameters:
     *
     * height: image height at resolution 0
     * width:  image width at resolution 0
     * depth:  image depth at resolution 0
     */

	//int addResolution ( int r, float vxlszV, float vxlszH, float vxlszD, hsize_t dimV, hsize_t dimH, hsize_t dimD, hsize_t chnk_dimV, hsize_t chnk_dimH, hsize_t chnk_dimD );	
	/* add resolution r with voxel size (vxlszV x vxlszH x vxlszD), dimensions (dimV x dimH x dimD) and chunk dimensions (chnk_dimV x chnk_dimH x chnk_dimD) to all chans */

	int addTimePoint ( int t, std::string params = "" );
	/* add time point t (with all chans and all resolutions) */

	void setVXL_SZ ( double szV, double szH, double szD );
	/* set voxel size at resolution 0 */

	int writeHyperslab ( int tp, int s, int r, iim::uint8 *buf, hsize_t *dims_buf, hsize_t *hl_buf, hsize_t *hl_file = 0 ); 
	/* write hyperslab hl_buf stored in buffer buf to hyperslab hl_file at time point tp and resolution r */

	void setHistogram ( histogram_t *buf, int r, int ch, int tp );
	/* set the histogram of channel ch at time point tp at resolution r to be saved when the file is closed 
	 * (has effect only if the file is being created) 
	 */

	void setThumbnail ( iim::uint8 *buf, iim::uint32 _thumbnail_sz );
	/* set the thumbnail to be saved when the file is closed (has effect only if the file is being created) */

	herr_t addFinalInfo ( );
	/* add complete histogram information and the thumbnail */
};


/* Descriptor for reading subvolumes with direct access to datasets
 */
struct IMS_volume_descr_t {
	int n_chans;
	hid_t  vxl_type;
	int    vxl_nbytes;
	hid_t *datasets_id; //
};



IMS_HDF5_fdescr_t::IMS_HDF5_fdescr_t ( ) {
	fname = (char *) 0;
	file_id = (hid_t) -1;
	n_chans = (hsize_t) 0;
	chan_groups_id = (hid_t *) 0;
	n_timepoints = (hsize_t) 0;
	tp_groups_id = (hid_t *) 0;
	active_tp = -1; // invalid value
	n_res = (hsize_t) 0;
	res_groups_id = (hid_t *) 0;
	active_res = -1; // invalid value
	vxl_sizes = (vxl_size_t *) 0;
	vol_dims = (dims_t *) 0;	
	chunk_dims = (subdvsns_t *) 0;	
	vxl_nbytes = 0;
	vxl_type = -1;
	n_slices = (hsize_t ***) 0;
	olist = (IMS_obj_list_t *) 0;
	rootalist = (IMS_attr_list_t *) 0;

	hist         = (histogram_t ***)0;
	thumbnail    = (iim::uint8 *) 0;
	thumbnail_sz = 0;

	creating           = false;
}


IMS_HDF5_fdescr_t::IMS_HDF5_fdescr_t ( const char *_fname, bool loadstruct, int _vxl_nbytes, IMS_obj_list_t *obj_info, IMS_attr_list_t *root_attr_info, int maxstp, int maxres, int maxtps ) {
	if ( !iim::isFile(_fname) ) { // non existing file: create it empty
	//if ( (file_id = H5Fopen(_fname, H5F_ACC_RDWR, H5P_DEFAULT)) < 0 ) { // non existing file: create it empty

		file_id = H5Fcreate(_fname, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

		fname = new char[strlen(_fname)+1];
		strcpy(fname,_fname);
		n_chans = 0;
		chan_groups_id = new hid_t[maxstp];
		memset(chan_groups_id,0,(maxstp*sizeof(hid_t))); // initialize with invalid hid values
		n_timepoints = 0;
		tp_groups_id = new hid_t[maxtps];
		memset(tp_groups_id,0,(maxtps*sizeof(hid_t))); // initialize with invalid hid values
		n_res = 0;
		res_groups_id = new hid_t[maxres];
		memset(res_groups_id,0,(maxres*sizeof(hid_t))); // initialize with invalid hid values
		vxl_sizes = new vxl_size_t[maxres];
		vol_dims = new dims_t[maxres];	
		chunk_dims = new subdvsns_t[maxres];
		vxl_nbytes = _vxl_nbytes;

		active_res = active_tp = -1; // initialize with invalid values

		n_slices = new hsize_t **[maxres];
		memset(n_slices,0,maxres*sizeof(hsize_t **)); 
			
		olist = (IMS_obj_list_t *) 0;
		rootalist = (IMS_attr_list_t *) 0;

		hist = new histogram_t **[maxres];
		memset(hist,0,maxres*sizeof(histogram_t **)); 
		thumbnail    = (iim::uint8 *) 0;
		thumbnail_sz = 0;

		creating           = true;

		if ( !obj_info || !root_attr_info )
			throw iim::IOException(iim::strprintf("the file %s has to be created: a structure description and a root attribute list must be passed ",_fname).c_str(),__iim__current__function__);

		herr_t status;

		// creates in the file root the same attributes described by the attribute list attr_info
		status = create_attributes(file_id,root_attr_info);

		// creates in the file the same hierarchy of object described by the object list obj_info
		status = create_file_struct(file_id,obj_info);

		init_voxel_size(obj_info);

		delete root_attr_info;
		delete obj_info;
		obj_info_cnt = 0; // releasing obj_info (which has not been created) makes the counter negative
	}
	else { // existing file: initialize descriptor

		if ( (file_id = H5Fopen(_fname, H5F_ACC_RDWR, H5P_DEFAULT)) < 0 ) // non existing file
			throw iim::IOException(iim::strprintf("file %s does not exist",_fname).c_str(),__iim__current__function__);

		if ( obj_info )
			throw iim::IOException(iim::strprintf("the file %s already exists. no structure description should be passed ",_fname).c_str(),__iim__current__function__);
		if ( root_attr_info )
			throw iim::IOException(iim::strprintf("the file %s already exists. no root attributes should be passed ",_fname).c_str(),__iim__current__function__);

		fname = new char[strlen(_fname)+1];
		strcpy(fname,_fname);
		// set all pointers to 0 in case they are not allocated afterwards
		chan_groups_id = (hid_t *) 0;
		tp_groups_id = (hid_t *) 0;
		res_groups_id = (hid_t *) 0;
		vxl_sizes = (vxl_size_t *) 0;
		vol_dims = (dims_t *) 0;	
		chunk_dims = (subdvsns_t *) 0;	
		scan_root();

		if ( loadstruct ) {
			olist = getOLIST();
			rootalist = getROOTALIST();
		}
		else {
			olist = (IMS_obj_list_t *) 0;
			rootalist = (IMS_attr_list_t *) 0;
		}


		hist         = (histogram_t ***)0;
		thumbnail    = (iim::uint8 *) 0;
		thumbnail_sz = 0;

		creating           = false;
	}

	if ( vxl_nbytes == 1 )
		vxl_type = H5T_NATIVE_UCHAR;
	else if ( vxl_nbytes == 2 )
		vxl_type = H5T_NATIVE_USHORT;
	else
		throw iim::IOException(iim::strprintf("number of bytes per voxel not allowed (%d)",vxl_nbytes).c_str(),__iim__current__function__);
}


IMS_HDF5_fdescr_t::~IMS_HDF5_fdescr_t ( ) {
	if ( fname )
		delete fname;
	if ( chan_groups_id ) {
		for ( int s=0; s<n_chans; s++ )
			H5Gclose(chan_groups_id[s]);
		delete[] chan_groups_id;
	}
	if ( tp_groups_id ) {
		for ( int t=0; t<n_timepoints; t++ )
			H5Gclose(tp_groups_id[t]);
		delete[] tp_groups_id;
	}
	if ( res_groups_id ) {
		for ( int r=0; r<n_res; r++ )
			if ( res_groups_id[r] ) 
				H5Gclose(res_groups_id[r]);
		delete[] res_groups_id;
	}
	if ( vol_dims )
		delete[] vol_dims;
	if ( vxl_sizes )
		delete[] vxl_sizes;
	if ( chunk_dims )
		delete[] chunk_dims;
	if ( n_slices ) {
		for ( int r=0; r<n_res; r++ )
			if ( n_slices[r] ) {
				for ( int t=0; t<n_timepoints; t++ )
					if ( n_slices[r][t] )
						delete [] n_slices[r][t];
				delete[]  n_slices[r];
			}
		delete[] n_slices;
	}
	if ( olist )
		delete olist;

	if ( rootalist )
		delete rootalist;

	if ( obj_info_cnt )
		throw iim::IOException(iim::strprintf("there are still %d object info records not released",obj_info_cnt).c_str(),__iim__current__function__);

	if ( hist ) {
		for ( int r=0; r<n_res; r++ )
			if ( hist[r] ) {
				for ( int t=0; t<n_timepoints; t++ )
					if ( hist[r][t] )
						delete [] hist[r][t];
				delete[]  hist[r];
			}
		delete[] hist;
	}

	if ( thumbnail )
		delete thumbnail;

	H5Fclose(file_id);
}


herr_t IMS_HDF5_fdescr_t::get_string_attribute ( hid_t parent, const char *name, char *&value_str ) {

	herr_t status;

	hid_t attrid;
	hid_t typid;
	hid_t spaceid;
	hsize_t len;

	attrid = H5Aopen(parent,name, H5P_DEFAULT);
	typid = H5Aget_type(attrid);
	if ( H5Tget_class(typid) != H5T_STRING )
		throw iim::IOException(iim::strprintf("No char type: all attributes should be strings of characters").c_str(),__iim__current__function__);
	spaceid = H5Aget_space(attrid);
	len = H5Sget_simple_extent_npoints(spaceid);
	value_str = new char[len+1];
	status = H5Aread(attrid,typid,value_str);
	value_str[len] = '\0';
	H5Aclose(attrid);
	H5Tclose(typid);
	H5Sclose(spaceid);

	return status;
}

// return the list of object in group "/"
IMS_obj_list_t *IMS_HDF5_fdescr_t::getOLIST ( ) {

	hid_t root_group_id = H5Gopen(file_id,"/", H5P_DEFAULT);
	IMS_obj_list_t *olist = get_obj_list(root_group_id);
	H5Gclose(root_group_id);

	//IMS_obj_list_t::iterator ito, ito2;
	//IMS_attr_list_t::iterator ita;
	//for ( ito = olist->begin(); ito != olist->end(); ito++ ) {
	//	for ( ita = ito->second->alist->begin(); ita !=  ito->second->alist->end(); ita++ )
	//		printf("---> object = %s, attribute = %s \n",ito->first,ita->first);
	//	for ( ito2 = ito->second->olist->begin(); ito2 != ito->second->olist->end(); ito2++ ) {
	//		for ( ita = ito2->second->alist->begin(); ita !=  ito2->second->alist->end(); ita++ )
	//			printf("------> object = %s, attribute = %s \n",ito2->first,ita->first);
	//	}
	//}

	return olist;
}

// return the list of attributes of group "/"
IMS_attr_list_t *IMS_HDF5_fdescr_t::getROOTALIST ( ) {

	hid_t root_group_id = H5Gopen(file_id,"/", H5P_DEFAULT);
	IMS_attr_list_t *alist = get_root_attributes(root_group_id);
	H5Gclose(root_group_id);

	return alist;
}


void *IMS_HDF5_fdescr_t::extractOLIST ( ) {

	IMS_obj_list_t *tmp_olist = olist;
	olist = (IMS_obj_list_t *) 0;
	obj_info_cnt = 0; // couter is reset because ownership of the object list has been passed to caller

	return tmp_olist;
}


void *IMS_HDF5_fdescr_t::extractROOTALIST ( ) {

	IMS_attr_list_t *tmp_rootalist = rootalist;
	rootalist = (IMS_attr_list_t *) 0;
	obj_info_cnt = 0; // couter is reset because ownership of the object list has been passed to caller

	return tmp_rootalist;
}


void IMS_HDF5_fdescr_t::setVXL_SZ ( double szV, double szH, double szD ) {
	vxl_sizes[0][2] = szH;
	vxl_sizes[0][1] = szV;
	vxl_sizes[0][0] = szD;
}


void IMS_HDF5_fdescr_t::init_voxel_size ( IMS_obj_list_t *obj_info ) {

	std::string value_str;
	double ExtMax0;
	double ExtMax1;
	double ExtMax2;
	double ExtMin0;
	double ExtMin1;
	double ExtMin2;
	int X;
	int Y;
	int Z;

	// @FIXED by Alessandro on 2016-12-01: replaced "at()" (C++ 11 only) with "[]" operator
	value_str = (*(*(*obj_info)["DataSetInfo"].olist)["Image"].alist)["ExtMax0"];
	ExtMax0 = atof(value_str.c_str());
	value_str = (*(*(*obj_info)["DataSetInfo"].olist)["Image"].alist)["ExtMax1"];
	ExtMax1 = atof(value_str.c_str());
	value_str = (*(*(*obj_info)["DataSetInfo"].olist)["Image"].alist)["ExtMax2"];
	ExtMax2 = atof(value_str.c_str());
	value_str = (*(*(*obj_info)["DataSetInfo"].olist)["Image"].alist)["ExtMin0"];
	ExtMin0 = atof(value_str.c_str());
	value_str = (*(*(*obj_info)["DataSetInfo"].olist)["Image"].alist)["ExtMin1"];
	ExtMin1 = atof(value_str.c_str());
	value_str = (*(*(*obj_info)["DataSetInfo"].olist)["Image"].alist)["ExtMin2"];
	ExtMin2 = atof(value_str.c_str());
	value_str = (*(*(*obj_info)["DataSetInfo"].olist)["Image"].alist)["X"];
	X = atoi(value_str.c_str());
	value_str = (*(*(*obj_info)["DataSetInfo"].olist)["Image"].alist)["Y"];
	Y = atoi(value_str.c_str());
	value_str = (*(*(*obj_info)["DataSetInfo"].olist)["Image"].alist)["Z"];
	Z = atoi(value_str.c_str());

	vxl_sizes[0][2] = (ExtMax0 - ExtMin0) / X;
	vxl_sizes[0][1] = (ExtMax1 - ExtMin1) / Y;
	vxl_sizes[0][0] = (ExtMax2 - ExtMin2) / Z;
}



int IMS_HDF5_fdescr_t::addChan ( int s ) {
	std::string chan_dir;
	int n_digits = 2; // IMS uses 2 digits to number chans
	std::stringstream chan_ord;
	chan_ord.width(n_digits);
	chan_ord.fill('0');
	chan_ord << s;
    chan_dir = "/" + ("s" + chan_ord.str()); // chans names have the format sXX

    /*
     * Create the group for chan descriptions
     */
	hid_t group_id;
	hid_t dataspace_id;
	hid_t dataset_id;
	herr_t status;
	hsize_t dims[2];
	hsize_t chunk_dims[2];
	hid_t cparms;
    hsize_t maxdims[2] = {H5S_UNLIMITED, H5S_UNLIMITED};
	
	group_id = H5Gcreate2(file_id, chan_dir.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

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
	if ( n_chans == MAXSTP )
		throw iim::IOException(iim::strprintf("too much chans (n_chans = %d",n_chans).c_str(),__iim__current__function__);

	chan_groups_id[n_chans] = group_id;

	n_chans++;

	/* Close the group. */
	//status = H5Gclose(group_id);

	return s;
}


int IMS_HDF5_fdescr_t::addResolution ( int r, hsize_t dimV, hsize_t dimH, hsize_t dimD, int dimC, bool is_first ) {

	char value_str[33];
	herr_t status;

	hid_t grpid = H5Gopen(file_id,"DataSet", H5P_DEFAULT);

	sprintf(value_str,"%u",r);
	std::string resname = "ResolutionLevel " + std::string(value_str);
	res_groups_id[r] = H5Gcreate2(grpid,resname.c_str(),H5P_DEFAULT,H5P_DEFAULT,H5P_DEFAULT);
	n_res = std::max<int>((int)n_res,r+1); // resolutions are not necessarily added in deceasing order starting from the highest 

	H5Gclose(grpid);

	// set the voxel size
	if ( r != 0 ) {
		vxl_sizes[r][0] = vxl_sizes[0][0] * iim::powInt(2,r);
		vxl_sizes[r][1] = vxl_sizes[0][1] * iim::powInt(2,r);
		vxl_sizes[r][2] = vxl_sizes[0][2] * iim::powInt(2,r);
	}

	vol_dims[0][0] = dimD;
	vol_dims[0][1] = dimV;
	vol_dims[0][2] = dimH;

	vol_dims[r][0] = dimD / iim::powInt(2,r);
	vol_dims[r][1] = dimV / iim::powInt(2,r);
	vol_dims[r][2] = dimH / iim::powInt(2,r);

	// WARNING: chunk_dims have fixed size at all resolutions
	chunk_dims[r][0] = DEF_CHNK_DIM_Z; 
	chunk_dims[r][1] = DEF_CHNK_DIM_XY; 
	chunk_dims[r][2] = DEF_CHNK_DIM_XY; 

	if ( is_first ) {

		// initialize n_chans
		n_chans = dimC;

		// set /DataSetInfo/Image attributes if not yet defined
		grpid = H5Gopen(file_id,"/DataSetInfo/Image",H5P_DEFAULT);
		//hid_t aid;

		// according to HDF5 reference if the attributes already exist the attempt to create them leave things unchanged
		//if ( (aid = H5Aopen(grpid,"ExtMax0",H5P_DEFAULT)) < 0 ) {
			sprintf(value_str,"%.3f",vxl_sizes[0][2]*dimH);
			status = create_string_attribute(grpid,"ExtMax0",value_str);
		//}
		//else
		//	status = H5Aclose(aid);

		//if ( (aid = H5Aopen(grpid,"ExtMax1",H5P_DEFAULT)) < 0 ) {
			sprintf(value_str,"%.3f",vxl_sizes[0][1]*dimV);
			status = create_string_attribute(grpid,"ExtMax1",value_str);
		//}
		//else
		//	status = H5Aclose(aid);

		//if ( (aid = H5Aopen(grpid,"ExtMax2",H5P_DEFAULT)) < 0 ) {
			sprintf(value_str,"%.3f",vxl_sizes[0][0]*dimD);
			status = create_string_attribute(grpid,"ExtMax2",value_str);
		//}
		//else
		//	status = H5Aclose(aid);

		//if ( (aid = H5Aopen(grpid,"ExtMin0",H5P_DEFAULT)) < 0 ) {
			status = create_string_attribute(grpid,"ExtMin0","0");
		//}
		//else
		//	status = H5Aclose(aid);

		//if ( (aid = H5Aopen(grpid,"ExtMin1",H5P_DEFAULT)) < 0 ) {
			status = create_string_attribute(grpid,"ExtMin1","0");
		//}
		//else
		//	status = H5Aclose(aid);

		//if ( (aid = H5Aopen(grpid,"ExtMin2",H5P_DEFAULT)) < 0 ) {
			status = create_string_attribute(grpid,"ExtMin2","0");
		//}
		//else
		//	status = H5Aclose(aid);

		//if ( (aid = H5Aopen(grpid,"X",H5P_DEFAULT)) < 0 ) {
			sprintf(value_str,"%d",(int)dimH);
			status = create_string_attribute(grpid,"X",value_str);
		//}
		//else
		//	status = H5Aclose(aid);

		//if ( (aid = H5Aopen(grpid,"Y",H5P_DEFAULT)) < 0 ) {
			sprintf(value_str,"%d",(int)dimV);
			status = create_string_attribute(grpid,"Y",value_str);
		//}
		//else
		//	status = H5Aclose(aid);

		//if ( (aid = H5Aopen(grpid,"Z",H5P_DEFAULT)) < 0 ) {
			sprintf(value_str,"%d",(int)dimD);
			status = create_string_attribute(grpid,"Z",value_str);
		//}
		//else
		//	status = H5Aclose(aid);

		H5Gclose(grpid);
	}

	// initialize n_slices[r]
	n_slices[r] = new hsize_t *[MAXTPS];
	memset(n_slices[r],0,MAXTPS*sizeof(hsize_t *));
	for ( int t=0; t<n_timepoints; t++ ) {
		n_slices[r][t] = new hsize_t[MAXSTP];
		memset(n_slices[r][t],0,MAXSTP*sizeof(hsize_t));
	}

	// initialize hist[r]
	hist[r] = new histogram_t *[MAXTPS];
	memset(hist[r],0,MAXTPS*sizeof(histogram_t *));
	for ( int t=0; t<n_timepoints; t++ ) {
		hist[r][t] = new histogram_t[MAXSTP];
		memset(hist[r][t],0,MAXSTP*sizeof(histogram_t));
	}

	return r;
}

//int IMS_HDF5_fdescr_t::addResolution ( int r, float vxlszV, float vxlszH, float vxlszD, 
//									  hsize_t dimV, hsize_t dimH, hsize_t dimD, hsize_t chnk_dimV, hsize_t chnk_dimH, hsize_t chnk_dimD ) {
//
//	hid_t r_dataset_id;
//	hid_t s_dataset_id;
//	hsize_t size[2];
//	herr_t status;
//	hid_t filespace_id;
//	hsize_t offset[2];
//	hid_t dataspace_id;
//
//	// confrontare r con n_res (sono sempre uguali?)
//
//	// update descriptor
//	if ( n_res == MAXRES )
//		throw iim::IOException(iim::strprintf("too much resolutions (n_res = %d)",n_res).c_str(),__iim__current__function__);
//
//	vxl_sizes[n_res][0] = vxlszD;    
//	vxl_sizes[n_res][1] = vxlszV;    
//	vxl_sizes[n_res][2] = vxlszH;    
//	vol_dims[n_res][0] = dimD; 
//	vol_dims[n_res][1] = dimV; 
//	vol_dims[n_res][2] = dimH; 
//	chunk_dims[n_res][0] = (chnk_dimD==-1) ? DEF_CHNK_DIM : chnk_dimD; 
//	chunk_dims[n_res][1] = (chnk_dimV==-1) ? DEF_CHNK_DIM : chnk_dimV; 
//	chunk_dims[n_res][2] = (chnk_dimH==-1) ? DEF_CHNK_DIM : chnk_dimH; 
//
//	// Define memory space 1x3
//	size[0] = 1;
//	size[1] = 3;
//    dataspace_id = H5Screate_simple(2, size, NULL);
//
//	for ( int s=0; s<n_chans; s++ ) {
//		r_dataset_id = H5Dopen(chan_groups_id[s],"resolutions", H5P_DEFAULT);
//		s_dataset_id = H5Dopen(chan_groups_id[s],"subdivisions", H5P_DEFAULT);
//		// extends resolutions dataset
//		size[0]   = r+1;
//		size[1]   = 3;
//		status = H5Dset_extent(r_dataset_id, size);
//		status = H5Dset_extent(s_dataset_id, size);
//		// select hyperslab
//		offset[0] = r;
//		offset[1] = 0;
//		size[0]   = 1;
//		size[1]   = 3;
//		// Write the data to the hyperslab.
//		// resolutions
//		filespace_id = H5Dget_space(r_dataset_id);
//		status = H5Sselect_hyperslab(filespace_id, H5S_SELECT_SET, offset, NULL, size, NULL);
//		status = H5Dwrite(r_dataset_id, H5T_NATIVE_DOUBLE, dataspace_id, filespace_id,H5P_DEFAULT,vxl_sizes[n_res]);
//    	H5Sclose(filespace_id);
//		H5Dclose(r_dataset_id);
//		// subdivisions
//		filespace_id = H5Dget_space(s_dataset_id);
//		status = H5Sselect_hyperslab(filespace_id, H5S_SELECT_SET, offset, NULL, size, NULL);
//		status = H5Dwrite(s_dataset_id, H5T_NATIVE_INT, dataspace_id, filespace_id,H5P_DEFAULT,chunk_dims[n_res]);
//    	H5Sclose(filespace_id);
//		H5Dclose(s_dataset_id);
//	}
//
//    H5Sclose(dataspace_id);
//
//	n_res++;
//
//	return r;
//}



int IMS_HDF5_fdescr_t::addTimePoint ( int t, std::string params ) {

    hsize_t maxdims[3] = {H5S_UNLIMITED, H5S_UNLIMITED, H5S_UNLIMITED};

	hsize_t dims[3];
	hid_t dataspace_id;
	hid_t cparms;
	hid_t dataset_id;

	char value_str[33];
	char value2_str[33];
	herr_t status;
	hid_t tp_grpid;
	hid_t ch_grpid;
	std::string chname;

	sprintf(value_str,"%u",t);
	std::string tpname = "TimePoint " + std::string(value_str);

	int max_res = -1;
	for ( int r=0; r<n_res; r++ ) {
		if ( res_groups_id[r] ) {
			// set active resolution to the maximum one
			if ( max_res == -1 )
				max_res = r;

			// create timepoint at resolution r
			tp_grpid = H5Gcreate2(res_groups_id[r],tpname.c_str(),H5P_DEFAULT,H5P_DEFAULT,H5P_DEFAULT);

			// create channels at timepoint t at resolution r
			for ( int c=0; c<n_chans; c++ ) {
				sprintf(value2_str,"%u",c);
				chname = "Channel " + std::string(value2_str);
				ch_grpid = H5Gcreate2(tp_grpid,chname.c_str(),H5P_DEFAULT,H5P_DEFAULT,H5P_DEFAULT);

				sprintf(value_str,"%d",(int)vol_dims[r][2]);
				status = create_string_attribute(ch_grpid,"ImageSizeX",value_str);
				sprintf(value_str,"%d",(int)vol_dims[r][1]);
				status = create_string_attribute(ch_grpid,"ImageSizeY",value_str);
				sprintf(value_str,"%d",(int)vol_dims[r][0]);
				status = create_string_attribute(ch_grpid,"ImageSizeZ",value_str);

				// TODO: histogram attributes

				/* Create the data space for the Data datasets. */
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
				  
				status = H5Pset_deflate (cparms, 3); 
				*/

				/* Uncomment these lines to set SZIP Compression 
				
				szip_options_mask = H5_SZIP_NN_OPTION_MASK;
				szip_pixels_per_block = 16;
				status = H5Pset_szip (cparms, szip_options_mask, szip_pixels_per_block);
				*/
				
				/* Uncomment and modify these lines to generalize filter setting */

				H5Z_filter_t h5z_filter_ID; 
				std::vector<std::string> tokens;
				
				if ( params == "" ) {
					tokens.push_back("1");
					tokens.push_back("3");
				}
				else {
					iom::split(params,DELIMITATOR,tokens);
				}
				// extract the filter ID
				h5z_filter_ID = (H5Z_filter_t) atoi(tokens[0].c_str());
				
				// Set up the filter if any
				// Note the "optional" flag is necessary, as with the DEFLATE filter
				if (h5z_filter_ID != FILTER_NOTHING) {
					if (h5z_filter_ID == FILTER_GZIP) {						
						unsigned int compression_level;
						if ( tokens.size() > 1 )
							compression_level = (unsigned int)atoi(tokens[1].c_str());
						else
							compression_level = 3;
						status = H5Pset_deflate(cparms, compression_level);
					}
					else if (h5z_filter_ID == FILTER_SZIP) {
						unsigned int szip_options_mask;
						if ( tokens.size() > 1 )
							szip_options_mask = (unsigned int)atoi(tokens[1].c_str());
						else
							szip_options_mask = H5_SZIP_NN_OPTION_MASK;
						unsigned int szip_pixels_per_block;
						if ( tokens.size() > 2 )
							szip_pixels_per_block = (unsigned int)atoi(tokens[2].c_str());
						else
							szip_pixels_per_block = 16;
						status = H5Pset_szip (cparms, szip_options_mask, szip_pixels_per_block);
					}
					else {
						size_t cd_nelmts = tokens.size()-1;
						unsigned int *cd_values;
						// extract compressor parameters
						cd_values = new unsigned int[cd_nelmts];
						for ( int t=0; t<cd_nelmts; t++ )
							cd_values[t] = (unsigned int) atoi(tokens[t+1].c_str());
						status = H5Pset_filter(cparms, h5z_filter_ID, H5Z_FLAG_OPTIONAL, cd_nelmts, cd_values);
						delete []cd_values;
					}
				}
				/**/

				/* Create the cells dataset. */
				dataset_id = H5Dcreate(ch_grpid, "Data", vxl_type, dataspace_id, H5P_DEFAULT, cparms, H5P_DEFAULT);

				/* Write data */
				//status = H5Dwrite (dataset_id, H5T_NATIVE_SHORT, H5S_ALL, H5S_ALL, H5P_DEFAULT, buf);

				/* Terminate access to the parameters. */ 
				status = H5Pclose(cparms);

				/* Terminate access to the data space. */ 
				status = H5Sclose(dataspace_id);

				/* End access to the dataset and release resources used by it. */
				status = H5Dclose(dataset_id);

				// TODO: create Histogram dataset

				/* Close the subsubgroup. */
				status = H5Gclose(ch_grpid);
			}

			// 2017-06-27. Giulio. @ADDED The group id of the new timepoint must saved and left open only if active_res is not changed by the next call to getDATASETS_ID
			if ( active_res == max_res ) 
				tp_groups_id[t] = tp_grpid;
			else
				// the group must be closed if active_res will be changed by the next call to getDATASETS_ID
				H5Gclose(tp_grpid);
		}
	}

	n_timepoints++; // timepoins are added in order starting from timepoint 0' 

	// 2017-06-27. Giulio. @ADDED enable the addition of the histogram and other metadata
	creating = true;

	// allocate n_slices for timepoint t at each existing resolution 
	for ( int r=0; r<n_res; r++ ) { 
		if ( n_slices[r] ) { // resolution r exists
			n_slices[r][t] = new hsize_t [MAXTPS];
			memset(n_slices[r][t],0,MAXTPS*sizeof(hsize_t));
		}
		else { 
			// 2017-06-27. Giulio. If the time point is added to an exixsting file the n_slices structure must be recreated
			// initialize n_slices[r]
			n_slices[r] = new hsize_t *[MAXTPS];
			memset(n_slices[r],0,MAXTPS*sizeof(hsize_t *));
			for ( int t=0; t<n_timepoints; t++ ) {
				n_slices[r][t] = new hsize_t[MAXSTP];
				memset(n_slices[r][t],0,MAXSTP*sizeof(hsize_t));
			}
		}
	}

	// allocate hists for timepoint t at each existing resolution 
	// 2017-06-27. Giulio. If the time point is added to an exixsting file the hisy structure must be recreated from scratch
	if ( !hist ) {
		hist = new histogram_t **[MAXRES];
		memset(hist,0,MAXRES*sizeof(histogram_t **)); 
	}
	for ( int r=0; r<n_res; r++ ) { 
		if ( hist[r] ) { // resolution r exists
			hist[r][t] = new histogram_t [MAXTPS];
			memset(hist[r][t],0,MAXTPS*sizeof(histogram_t));
		}
		else {
			// 2017-06-27. Giulio. If the time point is added to an exixsting file the hisy structure must be recreated
			// initialize hist[r]
			hist[r] = new histogram_t *[MAXTPS];
			memset(hist[r],0,MAXTPS*sizeof(histogram_t *));
			for ( int t=0; t<n_timepoints; t++ ) {
				hist[r][t] = new histogram_t[MAXSTP];
				memset(hist[r][t],0,MAXSTP*sizeof(histogram_t));
			}
		}
	}

	// set the maximum resolution as current one and t as the current timepoint
	hid_t *dset_id = IMS_HDF5_fdescr_t::getDATASETS_ID ( t, max_res, false );

	// load dataset ids
	//for ( int c=0; c<n_chans; c++ ) {
	//	status = H5Dclose(dset_id[c]);
	//}
	//delete dset_id;


	//tp_groups_id[t] = tp_group_id;
	//if ( t == n_timepoints ) { // one more time point has been added
	//	n_slices[t] = new hsize_t *[MAXSTP];
	//	for ( int s=0; s<n_chans; s++ ) {
	//		n_slices[t][s] = new hsize_t[MAXRES];
	//		memset(n_slices[t][s],0,MAXRES*sizeof(hsize_t));
	//	}
	//	n_timepoints++;
	//}

	return t;
}


// private methods implementation

void IMS_HDF5_fdescr_t::scan_root ( ) {
	int i, j, k, m;
	ssize_t len;
	herr_t err;
	hsize_t nobj;
	int otype;
	hid_t grpid;
	hid_t subgrpid;
	hid_t dset_id;
	//hid_t dspace_id; 
	hid_t dtype_id;

	char group_name[MAX_NAME];
	char memb_name[MAX_NAME];
	char submemb_name[MAX_NAME];
	char subsubmemb_name[MAX_NAME];
	char subsubsubmemb_name[MAX_NAME];
	
	char *value_str;

	bool groupDataSetFound = false;

	hid_t root_group_id = H5Gopen(file_id,"/", H5P_DEFAULT);
	/*
	 * Information about the group:
     */
	len = H5Iget_name (root_group_id, group_name, MAX_NAME);

    /*
     *  Get all the members of the groups, one at a time.
     */
	err = H5Gget_num_objs(root_group_id, &nobj);
	
	n_chans        = 0;
	chan_groups_id = new hid_t[MAXSTP];	
	n_timepoints    = 0;
	tp_groups_id    = new hid_t[MAXTPS];	
	n_res           = 0;
	res_groups_id   = new hid_t[MAXRES];	
	n_slices        = new hsize_t **[MAXRES]; // allocate only the resolutions list
	memset(n_slices,0,MAXRES*sizeof(hsize_t **)); // and initialize to 0 to control deallocation in destructor

	for (i = 0; i < nobj; i++) {
        /*
         *  For each object in the group, get the name and
         *  what type of object it is.
		 */
		len = H5Gget_objname_by_idx(root_group_id, (hsize_t)i, memb_name, (size_t)MAX_NAME );
		otype =  H5Gget_objtype_by_idx(root_group_id, (size_t)i);

        /*
         * process each object according to its type
         */
		switch(otype) {
			case H5G_LINK:
				break;
			case H5G_GROUP:
				grpid = H5Gopen(root_group_id,memb_name, H5P_DEFAULT);
				if ( strcmp(memb_name,"DataSet") == 0 ) { // is the DataSet group
					groupDataSetFound = true;
					// get the number of resolutions
					err = H5Gget_num_objs(grpid, &n_res);
					vol_dims = new dims_t[n_res]; 
					chunk_dims = new subdvsns_t[n_res]; // 2017-06-27. Giulio. @ADDED in case an existing file is re-opened for update
					active_res = 0; // stores timepoints at resolution 0
					active_tp  = 0; // stores channels at timepoint 0
					for ( j = 0; j < n_res; j++ ) { // process resolution j
						// 2017-06-27. Giulio. @ADDED initialization of chunk_dims at all existing resolutions
						// WARNING: chunk_dims have fixed size at all resolutions
						chunk_dims[j][0] = DEF_CHNK_DIM_Z; 
						chunk_dims[j][1] = DEF_CHNK_DIM_XY; 
						chunk_dims[j][2] = DEF_CHNK_DIM_XY; 
						len = H5Gget_objname_by_idx(grpid, (hsize_t)j, submemb_name, (size_t)MAX_NAME );
						otype =  H5Gget_objtype_by_idx(grpid, (size_t)j);
						if ( otype == H5G_GROUP ) {
							// get the number of timepoints
							res_groups_id[j] = H5Gopen(grpid,submemb_name, H5P_DEFAULT);
							err = H5Gget_num_objs(res_groups_id[j], &n_timepoints);
							// allocates n_slices[j]
							n_slices[j] = new hsize_t *[MAXTPS];
							memset(n_slices[j],0,MAXTPS*sizeof(hsize_t *));
							for ( k = 0; k < n_timepoints; k++ ) { // process timepoint k at resolution j
								len = H5Gget_objname_by_idx(res_groups_id[j], (hsize_t)k, subsubmemb_name, (size_t)MAX_NAME );
								otype =  H5Gget_objtype_by_idx(res_groups_id[j], (size_t)k);
								if ( otype == H5G_GROUP ) {
									// get the number of chans
									if ( j == 0 ) { // open timepoint groups only at resolution 0
										tp_groups_id[k] = H5Gopen(res_groups_id[j],subsubmemb_name, H5P_DEFAULT);
										err = H5Gget_num_objs(tp_groups_id[k], &n_chans);
									}
									n_slices[j][k] = new hsize_t[MAXSTP];
									memset(n_slices[j][k],0,MAXSTP*sizeof(hsize_t));
									if ( j== 0 && k == 0 ) { // process only channels at timepoint 0
										for ( m = 0; m < n_chans; m++ ) { // process chan m of time point k at resolution j
											len = H5Gget_objname_by_idx(tp_groups_id[k], (hsize_t)m, subsubsubmemb_name, (size_t)MAX_NAME );
											otype =  H5Gget_objtype_by_idx(tp_groups_id[k], (size_t)m);
											if ( otype == H5G_GROUP ) {
												chan_groups_id[m] = H5Gopen(tp_groups_id[k],subsubsubmemb_name, H5P_DEFAULT);
												dset_id = H5Dopen(chan_groups_id[m],"Data",H5P_DEFAULT);

												hid_t dspace_id = H5Dget_space(dset_id);    
												err = H5Sget_simple_extent_dims(dspace_id, vol_dims[j], NULL);
												H5Sclose(dspace_id);

												// data size should be get from group attributes
												err = get_string_attribute(chan_groups_id[m],"ImageSizeX",value_str);
												vol_dims[j][2] = atoi(value_str);
												delete []value_str;


												err = get_string_attribute(chan_groups_id[m],"ImageSizeY",value_str);
												vol_dims[j][1] = atoi(value_str);
												delete []value_str;

												err = get_string_attribute(chan_groups_id[m],"ImageSizeZ",value_str);
												vol_dims[j][0] = atoi(value_str);
												delete []value_str;

												dtype_id = H5Dget_type(dset_id);     /* datatype handle */ 
												//class_id     = H5Tget_class(dtype_id);
												//if (_class == H5T_INTEGER) printf("Data set has INTEGER type \n");
												//order     = H5Tget_order(datatype);
												//if (order == H5T_ORDER_LE) printf("Little endian order \n");
												vxl_nbytes = (int) H5Tget_size(dtype_id);
													
												// close temporary object
												H5Tclose(dtype_id);
												//H5Sclose(dspace_id); // not used
												H5Dclose(dset_id);

												// WARNING: resolutions groups, timepoint groups at resolution 0, channel groups at resolution 0 and timepoint 0 are left open
											}
											else
												throw iim::IOException(iim::strprintf("No group object: channels must be a group object").c_str(),__iim__current__function__);
										} // end chan m
									}
								}
								else
									throw iim::IOException(iim::strprintf("No group object: timepoint must be a group objects").c_str(),__iim__current__function__);
							} // end timepoint k
						}
						else 
							throw iim::IOException(iim::strprintf("No group object: resolution must be a group object").c_str(),__iim__current__function__);
					} // end resolution j

				}
				else if ( strcmp(memb_name,"DataSetInfo") == 0 ) { // is the DataSetInfo group
					if ( !groupDataSetFound )
						throw iim::IOException(iim::strprintf("DataSetInfo group cannot be processed before DataSet group").c_str(),__iim__current__function__);

					double ExtMax0;
					double ExtMax1;
					double ExtMax2;
					double ExtMin0;
					double ExtMin1;
					double ExtMin2;
					int X;
					int Y;
					int Z;
					subgrpid = H5Gopen(grpid,"Image", H5P_DEFAULT);

					err = get_string_attribute(subgrpid,"ExtMax0",value_str);
					ExtMax0 = atof(value_str);
					delete []value_str;

					err = get_string_attribute(subgrpid,"ExtMax1",value_str);
					ExtMax1 = atof(value_str);
					delete []value_str;

					err = get_string_attribute(subgrpid,"ExtMax2",value_str);
					ExtMax2 = atof(value_str);
					delete []value_str;

					err = get_string_attribute(subgrpid,"ExtMin0",value_str);
					ExtMin0 = atof(value_str);
					delete []value_str;

					err = get_string_attribute(subgrpid,"ExtMin1",value_str);
					ExtMin1 = atof(value_str);
					delete []value_str;

					err = get_string_attribute(subgrpid,"ExtMin2",value_str);
					ExtMin2 = atof(value_str);
					delete []value_str;

					err = get_string_attribute(subgrpid,"X",value_str);
					X = atoi(value_str);
					delete []value_str;

					err = get_string_attribute(subgrpid,"Y",value_str);
					Y = atoi(value_str);
					delete []value_str;

					err = get_string_attribute(subgrpid,"Z",value_str);
					Z = atoi(value_str);
					delete []value_str;
					
					vxl_sizes = new vxl_size_t[n_res];
					// store only resolution 0 because it the only one available in DataSetInfo group
					vxl_sizes[0][2] = (ExtMax0 - ExtMin0) / X;
					vxl_sizes[0][1] = (ExtMax1 - ExtMin1) / Y;
					vxl_sizes[0][0] = (ExtMax2 - ExtMin2) / Z;

					H5Gclose(subgrpid);
				}
				else // it is another group do nothing now
					;

				// close group
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

	H5Gclose(root_group_id);
}


int IMS_HDF5_fdescr_t::writeHyperslab ( int tp, int s, int r, iim::uint8 *buf, hsize_t *dims_buf, hsize_t *hl_buf, hsize_t *hl_file ) {

	//hsize_t len;
	//char gname[MAX_NAME];

	//hid_t data_group_id;
	//hid_t data_subgroup_id;
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

	//// get name of chan to be written
	//len = H5Iget_name(chan_groups_id[s], gname, (size_t)MAX_NAME );
	//data_group_id = H5Gopen(tp_groups_id[tp],gname+1,H5P_DEFAULT);

	//// get group (resolution) of dataset
	//std::stringstream res;
	//res << r;
	//data_subgroup_id = H5Gopen(data_group_id, res.str().c_str(), H5P_DEFAULT);

	// get dataset
	hid_t *did = getDATASETS_ID(tp,r,false);
	dataset_id =  H5Dopen2(chan_groups_id[s],"Data",H5P_DEFAULT);

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
		throw iim::IOException(iim::strprintf("File hyperslab provided: only append operation is supported").c_str(),__iim__current__function__);
    else { // append slice
    	start_file[0] = n_slices[r][tp][s];
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

		n_slices[r][tp][s] += count_buf[0];
	}

	H5Tclose(datatype_id);
	H5Sclose(bufspace_id);
	H5Sclose(dataspace_id);
	H5Dclose(dataset_id);
	//H5Gclose(data_subgroup_id);
	//H5Gclose(data_group_id);

	return 0;
}


hid_t *IMS_HDF5_fdescr_t::getDATASETS_ID ( int tp, int r, bool get ) {

	ssize_t len;
	herr_t err;
	int otype;
	hid_t dset_id;
	//hid_t dspace_id;
	hid_t dtype_id;

	char memb_name[MAX_NAME];
	char submemb_name[MAX_NAME];

	char *value_str;

	if ( r != active_res ) { // resolution is not active: load timepoint groups at resolution r and channel info at resolution r and timepoint tp 
		// get the number of timepoints at resolution r
		err = H5Gget_num_objs(res_groups_id[r], &n_timepoints);
		for ( int k = 0; k < n_timepoints; k++ ) { // process timepoint k at resolution r
			len = H5Gget_objname_by_idx(res_groups_id[r], (hsize_t)k, memb_name, (size_t)MAX_NAME );
			otype =  H5Gget_objtype_by_idx(res_groups_id[r], (size_t)k);
			if ( otype == H5G_GROUP ) {
				// get the number of chans
				if ( tp_groups_id[k] > 0 ) 
					err = H5Gclose(tp_groups_id[k]); // close the timepoint of previous resolution
				tp_groups_id[k] = H5Gopen(res_groups_id[r],memb_name, H5P_DEFAULT);
				if ( k == tp ) { // process only channels at timepoint tp
					err = H5Gget_num_objs(tp_groups_id[k], &n_chans);
					for ( int m = 0; m < n_chans; m++ ) { // process chan m of time point k at resolution r
						len = H5Gget_objname_by_idx(tp_groups_id[k], (hsize_t)m, submemb_name, (size_t)MAX_NAME );
						otype =  H5Gget_objtype_by_idx(tp_groups_id[k], (size_t)m);
						if ( otype == H5G_GROUP ) {
							if ( chan_groups_id[m] > 0 )
								err = H5Gclose(chan_groups_id[m]); // close the channel of previous resolution
							chan_groups_id[m] = H5Gopen(tp_groups_id[k],submemb_name, H5P_DEFAULT);
							dset_id = H5Dopen(chan_groups_id[m],"Data",H5P_DEFAULT);

							// data size should be get from group attributes
							//dspace_id = H5Dget_space(dset_id);    
							//status = H5Sget_simple_extent_dims(dspace_id, vol_dims[r], NULL);

							err = get_string_attribute(chan_groups_id[m],"ImageSizeX",value_str);
							vol_dims[r][2] = atoi(value_str);
							delete []value_str;

							err = get_string_attribute(chan_groups_id[m],"ImageSizeY",value_str);
							vol_dims[r][1] = atoi(value_str);
							delete []value_str;

							err = get_string_attribute(chan_groups_id[m],"ImageSizeZ",value_str);
							vol_dims[r][0] = atoi(value_str);
							delete []value_str;

							dtype_id = H5Dget_type(dset_id);     /* datatype handle */ 
							//class_id     = H5Tget_class(dtype_id);
							//if (_class == H5T_INTEGER) printf("Data set has INTEGER type \n");
							//order     = H5Tget_order(datatype);
							//if (order == H5T_ORDER_LE) printf("Little endian order \n");
							vxl_nbytes = (int) H5Tget_size(dtype_id);
							
							// set vxl size at resolution r (vol_dims[0] is assumed to be defined when file is opened
							vxl_sizes[r][0] = vxl_sizes[r][0] * (vol_dims[0][0] / vol_dims[r][0]);
							vxl_sizes[r][1] = vxl_sizes[r][1] * (vol_dims[0][1] / vol_dims[r][1]);
							vxl_sizes[r][2] = vxl_sizes[r][2] * (vol_dims[0][2] / vol_dims[r][2]);
													
							// close temporary object
							H5Tclose(dtype_id);
							//H5Sclose(dspace_id);
							H5Dclose(dset_id);

							// WARNING: timepoint groups at resolution r, channel groups at resolution r and timepoint tp are left open
						}
						else
							throw iim::IOException(iim::strprintf("No group object: channels must be a group object").c_str(),__iim__current__function__);
					} // end chan m
					active_tp = tp; // tp is now the active timepoint
				}
			}
			else
				throw iim::IOException(iim::strprintf("No group object: timepoint must be a group objects").c_str(),__iim__current__function__);
		}
		active_res = r; // r is now the active resolution
	} 
	else if ( tp != active_tp ) { // timepoint is not active: load channel info at timepoint tp 
		// get the number of channels at timepoint tp
		err = H5Gget_num_objs(tp_groups_id[tp], &n_chans);
		for ( int m = 0; m < n_chans; m++ ) { // process chan m of timepoint tp at resolution r
			len = H5Gget_objname_by_idx(tp_groups_id[tp], (hsize_t)m, memb_name, (size_t)MAX_NAME );
			otype =  H5Gget_objtype_by_idx(tp_groups_id[tp], (size_t)m);
			if ( otype == H5G_GROUP ) {
				err = H5Gclose(chan_groups_id[m]); // close the channel of previous timepoint
				chan_groups_id[m] = H5Gopen(tp_groups_id[tp],memb_name, H5P_DEFAULT);
				dset_id = H5Dopen(chan_groups_id[m],"Data",H5P_DEFAULT);

				// data size should be get from group attributes
				//dspace_id = H5Dget_space(dset_id);    
				//status = H5Sget_simple_extent_dims(dspace_id, vol_dims[r], NULL);

				err = get_string_attribute(chan_groups_id[m],"ImageSizeX",value_str);
				vol_dims[r][2] = atoi(value_str);
				delete []value_str;

				err = get_string_attribute(chan_groups_id[m],"ImageSizeY",value_str);
				vol_dims[r][1] = atoi(value_str);
				delete []value_str;

				err = get_string_attribute(chan_groups_id[m],"ImageSizeZ",value_str);
				vol_dims[r][0] = atoi(value_str);
				delete []value_str;

				dtype_id = H5Dget_type(dset_id);     /* datatype handle */ 
				//class_id     = H5Tget_class(dtype_id);
				//if (_class == H5T_INTEGER) printf("Data set has INTEGER type \n");
				//order     = H5Tget_order(datatype);
				//if (order == H5T_ORDER_LE) printf("Little endian order \n");
				vxl_nbytes = (int) H5Tget_size(dtype_id);
				
				// set vxl size at resolution r (vol_dims[0] is assumed to be defined when file is opened
				vxl_sizes[r][0] = vxl_sizes[r][0] * (vol_dims[0][0] / vol_dims[r][0]);
				vxl_sizes[r][1] = vxl_sizes[r][1] * (vol_dims[0][1] / vol_dims[r][1]);
				vxl_sizes[r][2] = vxl_sizes[r][2] * (vol_dims[0][2] / vol_dims[r][2]);
													
				// close temporary object
				H5Tclose(dtype_id);
				//H5Sclose(dspace_id);
				H5Dclose(dset_id);

				// WARNING: channel groups at tesolution r and  timepoint tp are left open
			}
			else
				throw iim::IOException(iim::strprintf("No group object: channels must be a group object").c_str(),__iim__current__function__);
		}
		active_tp = tp; // tp is now the active timepoint
	} 

	if ( get ) {
		hid_t *dsets_id = new hid_t[n_chans];

		// load dataset ids
		for ( int m=0; m<n_chans; m++ ) {
			dsets_id[m] = H5Dopen2(chan_groups_id[m],"Data",H5P_DEFAULT);
		}

		return dsets_id;
	}
	else
		return (hid_t *) 0;
}


void IMS_HDF5_fdescr_t::setHistogram ( histogram_t *buf, int r, int ch, int tp ) {
	if ( creating ) {
		hist[r][tp][ch].hmin = buf->hmin;
		hist[r][tp][ch].hmax = buf->hmax;
		hist[r][tp][ch].hlen = buf->hlen;
		hist[r][tp][ch].hist = buf->hist;
		buf->hist = (iim::uint64 *) 0;
	}
}

	
void IMS_HDF5_fdescr_t::setThumbnail ( iim::uint8 *buf, iim::uint32 _thumbnail_sz ) {
	if ( creating ) {
		thumbnail = buf;
		thumbnail_sz = _thumbnail_sz;
	}
}


herr_t IMS_HDF5_fdescr_t::addFinalInfo ( ) {

	hid_t thumbnail_group_id;

    hsize_t maxdims[2] = {H5S_UNLIMITED, H5S_UNLIMITED};

	hsize_t dims[2];
	hid_t dataspace_id;
	hid_t cparms;
	hid_t dataset_id;

	char num_str[64]; // num to string conversion

	herr_t status;

	if ( !creating )
		return 0;

	for ( int r=0; r<n_res; r++ ) {
		if ( res_groups_id[r] ) {
			for ( int t=0; t<n_timepoints; t++ ) {
				getDATASETS_ID(t,r,false);
				for ( int c=0; c<n_chans; c++ ) {

					if ( hist[r][t][c].hlen ) { // 2017-06-27. Giulio. @ADDED if hlen is 0 the histogram has not been added and no metadata must be written

						/* add histogram attributes to channel group */
						sprintf(num_str,"%.2f",(double)hist[r][t][c].hmax);
						create_string_attribute(chan_groups_id[c],"HistogramMax",std::string(num_str).c_str());
						sprintf(num_str,"%.2f",(double)hist[r][t][c].hmin);
						create_string_attribute(chan_groups_id[c],"HistogramMin",std::string(num_str).c_str());

						/* Create the data space for the Data datasets. */
						dims[0] = hist[r][t][c].hlen;
						dataspace_id = H5Screate_simple(1, dims, maxdims);

						/* Create the parameter list for cells datasets. */
						cparms = H5Pcreate(H5P_DATASET_CREATE);
						status = H5Pset_chunk(cparms, 1, dims);
						status = H5Pset_fill_time(cparms,H5D_FILL_TIME_NEVER);

						/* Create the cells dataset. */
						dataset_id = H5Dcreate(chan_groups_id[c], "Histogram", H5T_NATIVE_ULLONG, dataspace_id, H5P_DEFAULT, cparms, H5P_DEFAULT);

						/* Write data */
						status = H5Dwrite (dataset_id, H5T_NATIVE_ULLONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, hist[r][t][c].hist);

						/* Terminate access to the parameters. */ 
						status = H5Pclose(cparms);

						/* Terminate access to the data space. */ 
						status = H5Sclose(dataspace_id);

						/* End access to the dataset and release resources used by it. */
						status = H5Dclose(dataset_id);

						/* chan_groups_id[c] shold not be closed */
						//status = H5Gclose(chan_groups_id[c]);
					}
				}
			}
		}
	}

	if ( thumbnail ) {
		thumbnail_group_id = H5Gopen(file_id,"/Thumbnail", H5P_DEFAULT);

		/* Create the data space for the Data datasets. */
		dims[0] = thumbnail_sz;
		dims[1] = thumbnail_sz * 4;
		dataspace_id = H5Screate_simple(2, dims, maxdims);

		/* Create the parameter list for cells datasets. */
		cparms = H5Pcreate(H5P_DATASET_CREATE);
		status = H5Pset_chunk(cparms, 2, dims);
	    status = H5Pset_fill_time(cparms,H5D_FILL_TIME_NEVER);

		/* Create the cells dataset. */
		dataset_id = H5Dcreate(thumbnail_group_id, "Data", H5T_NATIVE_UCHAR, dataspace_id, H5P_DEFAULT, cparms, H5P_DEFAULT);

		/* Write data */
		status = H5Dwrite (dataset_id, H5T_NATIVE_UCHAR, H5S_ALL, H5S_ALL, H5P_DEFAULT, thumbnail);

		/* Terminate access to the parameters. */ 
		status = H5Pclose(cparms);

		/* Terminate access to the data space. */ 
		status = H5Sclose(dataspace_id);

		/* End access to the dataset and release resources used by it. */
		status = H5Dclose(dataset_id);

		/* Close the subsubgroup. */
		status = H5Gclose(thumbnail_group_id);
	}

	return 0;
}

#endif // ENABLE_IMS_HDF5


/****************************************************************************
* HDF5 Manager implementation
****************************************************************************/

void IMS_HDF5init ( std::string fname, void *&descr, bool loadstruct, int vxl_nbytes, void *obj_info, void *root_attr_info ) throw (iim::IOException) {
#ifdef ENABLE_IMS_HDF5
	IMS_HDF5_fdescr_t *int_descr = new IMS_HDF5_fdescr_t(fname.c_str(),loadstruct,vxl_nbytes,(IMS_obj_list_t *)obj_info,(IMS_attr_list_t *)root_attr_info);
	descr = int_descr;
#else
	throw iim::IOException(iim::strprintf(
			"Support to IMS_HDF5 files not available: please verify there is are valid hdf5 static libs (hdf5 and szip) "
			"in ""3rdparty/libs"" directory and set the ""ENABLED_IMS_HDF5"" checkbox before configuring CMake project").c_str(),__iim__current__function__);
#endif
}

void *IMS_HDF5get_olist ( void *descr, std::string fname, int height, int width, int z_points, int n_chans, int n_timepoints, float abs_V, float abs_H ) throw (iim::IOException) {
#ifdef ENABLE_IMS_HDF5
	if ( descr )
		return ((IMS_HDF5_fdescr_t *) descr)->extractOLIST();
	else
		return (void *) build_std_filestruct(fname,height,width,z_points,n_chans,n_timepoints,abs_V,abs_H);
#else
	throw iim::IOException(iim::strprintf(
			"Support to IMS_HDF5 files not available: please verify there is are valid hdf5 static libs (hdf5 and szip) "
			"in ""3rdparty/libs"" directory and set the ""ENABLED_IMS_HDF5"" checkbox before configuring CMake project").c_str(),__iim__current__function__);
#endif
}

void *IMS_HDF5adjust_olist ( void *olist, std::string fname, int height, int width, int z_points, iim::uint32 *chans, int n_chans, float abs_V, float abs_H ) throw (iim::IOException) {
#ifdef ENABLE_IMS_HDF5
		return ((void *) adjust_obj_list((IMS_obj_list_t *)olist,fname,height,width,z_points,chans,n_chans,abs_V,abs_H));
#else
	throw iim::IOException(iim::strprintf(
			"Support to IMS_HDF5 files not available: please verify there is are valid hdf5 static libs (hdf5 and szip) "
			"in ""3rdparty/libs"" directory and set the ""ENABLED_IMS_HDF5"" checkbox before configuring CMake project").c_str(),__iim__current__function__);
#endif
}

void *IMS_HDF5get_rootalist ( void *descr ) throw (iim::IOException) {
#ifdef ENABLE_IMS_HDF5
	if ( descr )
		return ((IMS_HDF5_fdescr_t *) descr)->extractROOTALIST();
	else
		return (void *) build_std_rootattributes();
#else
	throw iim::IOException(iim::strprintf(
			"Support to IMS_HDF5 files not available: please verify there is are valid hdf5 static libs (hdf5 and szip) "
			"in ""3rdparty/libs"" directory and set the ""ENABLED_IMS_HDF5"" checkbox before configuring CMake project").c_str(),__iim__current__function__);
#endif
}

int IMS_HDF5n_resolutions ( void *descr ) throw (iim::IOException) { 
#ifdef ENABLE_IMS_HDF5
	return ((IMS_HDF5_fdescr_t *) descr)->getN_RES();
#else
	throw iim::IOException(iim::strprintf(
			"Support to IMS_HDF5 files not available: please verify there is are valid hdf5 static libs (hdf5 and szip) "
			"in ""3rdparty/libs"" directory and set the ""ENABLED_IMS_HDF5"" checkbox before configuring CMake project").c_str(),__iim__current__function__);
#endif
}


void IMS_HDF5set_histogram ( void *descr, histogram_t *buf, int r, int ch, int tp ) throw (iim::IOException) {
#ifdef ENABLE_IMS_HDF5
	((IMS_HDF5_fdescr_t *) descr)->setHistogram(buf,r,ch,tp);
#else
	throw iim::IOException(iim::strprintf(
			"Support to IMS_HDF5 files not available: please verify there is are valid hdf5 static libs (hdf5 and szip) "
			"in ""3rdparty/libs"" directory and set the ""ENABLED_IMS_HDF5"" checkbox before configuring CMake project").c_str(),__iim__current__function__);
#endif
}


void IMS_HDF5set_thumbnail ( void *descr, iim::uint8 *buf, iim::uint32 thumbnail_sz ) throw (iim::IOException) {
#ifdef ENABLE_IMS_HDF5
	((IMS_HDF5_fdescr_t *) descr)->setThumbnail(buf,thumbnail_sz);
#else
	throw iim::IOException(iim::strprintf(
			"Support to IMS_HDF5 files not available: please verify there is are valid hdf5 static libs (hdf5 and szip) "
			"in ""3rdparty/libs"" directory and set the ""ENABLED_IMS_HDF5"" checkbox before configuring CMake project").c_str(),__iim__current__function__);
#endif
}



void IMS_HDF5close ( void *descr ) throw (iim::IOException) {
#ifdef ENABLE_IMS_HDF5
	((IMS_HDF5_fdescr_t *) descr)->addFinalInfo();
	delete (IMS_HDF5_fdescr_t *) descr;
#else
	throw iim::IOException(iim::strprintf(
			"Support to IMS_HDF5 files not available: please verify there is are valid hdf5 static libs (hdf5 and szip) "
			"in ""3rdparty/libs"" directory and set the ""ENABLED_IMS_HDF5"" checkbox before configuring CMake project").c_str(),__iim__current__function__);
#endif
}


void IMS_HDF5setVxlSize ( void *descr, double szV, double szH, double szD ) throw (iim::IOException) {
#ifdef ENABLE_IMS_HDF5
	IMS_HDF5_fdescr_t *int_descr = (IMS_HDF5_fdescr_t *) descr;

	int_descr->setVXL_SZ(szV,szH,szD);
#else
	throw iim::IOException(iim::strprintf(
			"Support to IMS_HDF5 files not available: please verify there is are valid hdf5 static libs (hdf5 and szip) "
			"in ""3rdparty/libs"" directory and set the ""ENABLED_IMS_HDF5"" checkbox before configuring CMake project").c_str(),__iim__current__function__);
#endif
}


void IMS_HDF5getVxlSize ( void *descr, double &szV, double &szH, double &szD ) throw (iim::IOException) {
#ifdef ENABLE_IMS_HDF5
	IMS_HDF5_fdescr_t *int_descr = (IMS_HDF5_fdescr_t *) descr;

	szH = int_descr->getVXL_SZ(0)[2];
	szV = int_descr->getVXL_SZ(0)[1];
	szD = int_descr->getVXL_SZ(0)[0];
#else
	throw iim::IOException(iim::strprintf(
			"Support to IMS_HDF5 files not available: please verify there is are valid hdf5 static libs (hdf5 and szip) "
			"in ""3rdparty/libs"" directory and set the ""ENABLED_IMS_HDF5"" checkbox before configuring CMake project").c_str(),__iim__current__function__);
#endif
}


void IMS_HDF5addResolution ( void *file_descr, iim::sint64 height, iim::sint64 width, iim::sint64 depth, int nchans, int r, bool is_first ) throw (iim::IOException) {
#ifdef ENABLE_IMS_HDF5
	IMS_HDF5_fdescr_t *int_descr = (IMS_HDF5_fdescr_t *) file_descr;

	if ( int_descr->addResolution(r,(hsize_t)height, (hsize_t)width,(hsize_t)depth,nchans,is_first) != r )
		throw iim::IOException(iim::strprintf("cannot add resolution %d",r).c_str(),__iim__current__function__);
#else
	throw iim::IOException(iim::strprintf(
			"Support to IMS_HDF5 files not available: please verify there is are valid hdf5 static libs (hdf5 and szip) "
			"in ""3rdparty/libs"" directory and set the ""ENABLED_IMS_HDF5"" checkbox before configuring CMake project").c_str(),__iim__current__function__);
#endif
}


void IMS_HDF5addChans ( void *file_descr, iim::sint64 height, iim::sint64 width, iim::sint64 depth, 
				 float vxlszV, float vxlszH, float vxlszD, bool *res, int res_size, int chans, int block_height, int block_width, int block_depth ) throw (iim::IOException) {
#ifdef ENABLE_IMS_HDF5

	IMS_HDF5_fdescr_t *int_descr = (IMS_HDF5_fdescr_t *) file_descr;

	if ( int_descr->getN_CHANS() ) { // file already contains timepoints: check consistency of additional data
		throw iim::IOException(iim::strprintf("updating already existing files not supported yet").c_str(),__iim__current__function__);
	}
	else { // file is empty create chans descriptors
		for ( int c=0; c<chans; c++ )
			if ( int_descr->addChan(c) != c )
				throw iim::IOException(iim::strprintf("cannot add chan %d",c).c_str(),__iim__current__function__);
		//for ( int r=0; r<res_size; r++ )
		//	if ( res[r] )
		//		if ( int_descr->addResolution(r,vxlszV*iim::powInt(2,r),vxlszH*iim::powInt(2,r),vxlszD*iim::powInt(2,r),
		//					height/iim::powInt(2,r),width/iim::powInt(2,r),depth/iim::powInt(2,r),block_height, block_width, block_depth) != r )
		//			throw iim::IOException(iim::strprintf("cannot add resolution %d",r).c_str(),__iim__current__function__);
	}
#else
	throw iim::IOException(iim::strprintf(
			"Support to IMS_HDF5 files not available: please verify there is are valid hdf5 static libs (hdf5 and szip) "
			"in ""3rdparty/libs"" directory and set the ""ENABLED_IMS_HDF5"" checkbox before configuring CMake project").c_str(),__iim__current__function__);
#endif
}


void IMS_HDF5addTimepoint ( void *file_descr, int tp, std::string params ) throw (iim::IOException) {
#ifdef ENABLE_IMS_HDF5
	IMS_HDF5_fdescr_t *int_descr = (IMS_HDF5_fdescr_t *) file_descr;

	if ( int_descr->addTimePoint(tp,params) != tp )
		throw iim::IOException(iim::strprintf("cannot add time point %d",tp).c_str(),__iim__current__function__);
#else
	throw iim::IOException(iim::strprintf(
			"Support to IMS_HDF5 files not available: please verify there is are valid hdf5 static libs (hdf5 and szip) "
			"in ""3rdparty/libs"" directory and set the ""ENABLED_IMS_HDF5"" checkbox before configuring CMake project").c_str(),__iim__current__function__);
#endif
}


void IMS_HDF5writeHyperslab ( void *file_descr, iim::uint8 *buf, iim::sint64 *dims_buf, iim::sint64 *hl, int r, int s, int tp ) throw (iim::IOException) {
#ifdef ENABLE_IMS_HDF5
	IMS_HDF5_fdescr_t *int_descr = (IMS_HDF5_fdescr_t *) file_descr;

	if ( int_descr->writeHyperslab(tp,s,r,buf,(hsize_t *)dims_buf,(hsize_t *)hl) )
		throw iim::IOException(iim::strprintf("cannot add hyperslab: buffer size=(%d,%d,%d), hyperslab=[offset=(%d,%d,%d), stride=(%d,%d,%d), size=(%d,%d,%d), block=(%d,%d,%d)]",
									dims_buf[0],dims_buf[1],dims_buf[2],hl[0],hl[1],hl[2],hl[3],hl[4],hl[5],hl[6],hl[7],hl[8],hl[9],hl[10],hl[11]).c_str(),__iim__current__function__);
#else
	throw iim::IOException(iim::strprintf(
			"Support to IMS_HDF5 files not available: please verify there is are valid hdf5 static libs (hdf5 and szip) "
			"in ""3rdparty/libs"" directory and set the ""ENABLED_IMS_HDF5"" checkbox before configuring CMake project").c_str(),__iim__current__function__);
#endif
}



void IMS_HDF5getVolumeInfo ( void *descr, int tp, int res, void *&volume_descr, 
								float &VXL_1, float &VXL_2, float &VXL_3, 
								float &ORG_V, float &ORG_H, float &ORG_D, 
								iim::uint32 &DIM_V, iim::uint32 &DIM_H, iim::uint32 &DIM_D,
							    int &DIM_C, int &BYTESxCHAN, int &DIM_T, int &t0, int &t1 ) throw (iim::IOException) {
#ifdef ENABLE_IMS_HDF5

	
	IMS_HDF5_fdescr_t *int_descr = (IMS_HDF5_fdescr_t *) descr;

	IMS_volume_descr_t *int_volume_descr = new IMS_volume_descr_t;

	int_volume_descr->datasets_id = int_descr->getDATASETS_ID(tp,res);

	if ( res != int_descr->getACTIVE_RES() ) 
		throw iim::IOException(iim::strprintf(
			"resolution %d is not active (%d): only volume information of active resolution can be get",res, int_descr->getACTIVE_RES()).c_str(),__iim__current__function__);
	if ( tp != int_descr->getACTIVE_TP() ) 
		throw iim::IOException(iim::strprintf(
			"timepoint %d is not active (%d): only volume information of active timepoint can be get",tp, int_descr->getACTIVE_TP()).c_str(),__iim__current__function__);

	// in HDF5 dimensions are ordered as DVH
	VXL_1 = (float) int_descr->getVXL_SZ(res)[1];
	VXL_2 = (float) int_descr->getVXL_SZ(res)[2];
	VXL_3 = (float) int_descr->getVXL_SZ(res)[0];

	ORG_V = ORG_H = ORG_D = 0.0;

	DIM_V = (int) int_descr->getVOL_DIMS(res)[1];
	DIM_H = (int) int_descr->getVOL_DIMS(res)[2];
	DIM_D = (int) int_descr->getVOL_DIMS(res)[0];

	DIM_C = int_descr->getN_CHANS();
	BYTESxCHAN = int_descr->getVXL_NBYTES();

	DIM_T = int_descr->getN_TPS();
	t0 = t1 = 0;

	int_volume_descr->n_chans    = int_descr->getN_CHANS();
	int_volume_descr->vxl_type   = (int) int_descr->getVXL_TYPE();
	int_volume_descr->vxl_nbytes = int_descr->getVXL_NBYTES();

	volume_descr = int_volume_descr;
#else
	throw iim::IOException(iim::strprintf(
			"Support to IMS_HDF5 files not available: please verify there is are valid hdf5 static libs (hdf5 and szip) "
			"in ""3rdparty/libs"" directory and set the ""ENABLED_IMS_HDF5"" checkbox before configuring CMake project").c_str(),__iim__current__function__);
#endif
}


void IMS_HDF5getSubVolume ( void *descr, int V0, int V1, int H0, int H1, int D0, int D1, int chan, iim::uint8 *buf, int red_factor ) throw (iim::IOException) {
#ifdef ENABLE_IMS_HDF5

	IMS_volume_descr_t *int_volume_descr = (IMS_volume_descr_t *) descr;
	
	hsize_t start_buf[3];     
	hsize_t start_file[3]; 
	hsize_t dims_buf[3];
	herr_t status;

	dims_buf[0] = D1 - D0;
	dims_buf[1] = V1 - V0;
	dims_buf[2] = H1 - H0;
	
	/* Get filespace handle first. */
	hid_t filespace_id = H5Dget_space(int_volume_descr->datasets_id[chan]);    /* dataspace handle */
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
		status = H5Dread(int_volume_descr->datasets_id[chan],int_volume_descr->vxl_type,bufspace_id,filespace_id,H5P_DEFAULT,buf);
	}
	else if ( int_volume_descr->vxl_nbytes == 2 ) {
		
		if ( red_factor == 1 ) /* 2015-12-29. Giulio. @ADDED no conversion from 16 to 8 bits depth */ { 
			status = H5Dread(int_volume_descr->datasets_id[chan],int_volume_descr->vxl_type,bufspace_id,filespace_id,H5P_DEFAULT,buf);
		}
		else if ( red_factor == 2 ) { 
			iim::sint64 sbv_ch_dim = dims_buf[0] * dims_buf[1] * dims_buf[2];
			iim::uint8 *tempbuf = new iim::uint8[red_factor * sbv_ch_dim];
			status = H5Dread(int_volume_descr->datasets_id[chan],int_volume_descr->vxl_type,bufspace_id,filespace_id,H5P_DEFAULT,tempbuf);

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
			"Support to IMS_HDF5 files not available: please verify there is are valid hdf5 static libs (hdf5 and szip) "
			"in ""3rdparty/libs"" directory and set the ""ENABLED_IMS_HDF5"" checkbox before configuring CMake project").c_str(),__iim__current__function__);
#endif
}

void IMS_HDF5closeVolume ( void *descr ) throw (iim::IOException) {
#ifdef ENABLE_IMS_HDF5
	IMS_volume_descr_t *int_volume_descr = (IMS_volume_descr_t *) descr;
	if ( int_volume_descr->datasets_id )
		delete int_volume_descr->datasets_id;
	delete (IMS_volume_descr_t *) descr;
#else
	throw iim::IOException(iim::strprintf(
			"Support to IMS_HDF5 files not available: please verify there is are valid hdf5 static libs (hdf5 and szip) "
			"in ""3rdparty/libs"" directory and set the ""ENABLED_IMS_HDF5"" checkbox before configuring CMake project").c_str(),__iim__current__function__);
#endif
}

