/*
 * MappingObjects.h
 *
 * created September 3, 2012 by Giulio Iannello
 * changed May 02, 2016 by Pierangelo Afferni: added sub-class MappingView180 extending RotMappingViews
 */


/******************
*    CHANGELOG    *
*******************
* 2016-04-27 Pierangelo.      @CREATED
*/


#ifndef MAPPING_OBJECTS_H
#define MAPPING_OBJECTS_H

# include "../imagemanager/IM_config.h"
# include "spaceObjects.h"
# include "reg_utils.h"

typedef double RotMat_t[3][3]; // moved here from IM_config.h

/* MappingViews: 
 * virtual class for any kind of mapping between positioned volumes
 *
 * mapping is from a 'feature' volume to a 'target' volume 
 */
class MappingViews {

public:
	MappingViews() {}

	// destructor
	virtual ~MappingViews ( ) { }

	virtual bool map ( Point fPoint, Point *tPoint ) = 0;
	/* if the point fPoint from feature space can be mapped to target space
	 * returns the corresponding point in target space in *tPoint and the 
	 * result is true, otherwise the result is false
	 */

	virtual bool mapback ( Point tPoint, Point *fPoint ) = 0;
	/* if the point tPoint from target space can be mapped to feature space
	 * returns the corresponding point in feature space in *fPoint and the 
	 * result is true, otherwise the result is false
	 */

	virtual bool map_ss ( SpaceSize feature_ss, SpaceSize *target_ss ) = 0;
	/* if the space size (segment) feature_ss from feature space can be mapped
	 * to target space, return the transformed space size in *target_ss and
	 * the result is true, otherwise the result is false
	 */

	virtual bool mapback_ss ( SpaceSize target_ss, SpaceSize *feature_ss ) = 0;
	/* if the space size (segment) target_ss from target space can be mapped back 
	 * to feature space, return the transformed space size in *feature_ss and 
	 * the result is true, otherwise the result is false
	 */

	virtual iim::real32 * mapbuffer(iim::real32 * buf, int dimV, int dimH, int dimD) = 0;
	/* Returns a copy of data in buf transformed according to this mapping
	 */

 	virtual iim::uint8 * mapbuffer(iim::uint8 * buf, int dimV, int dimH, int dimD, int channels, int bytes_x_chan) = 0;

	virtual void mapdims ( int dimf_V, int dimf_H, int dimf_D, int *dimt_V, int *dimt_H, int *dimt_D ) = 0;
	/* maps the extensions of the feature volume to the ones of the target volume 
	 * the mapped extensions are computed in such a way that all useful voxels of the feature volume
	 * can be accessed through the target volume
	 * some voxels in the target volume may be empty since they correspond to voxels of the feature one
	 * that do not exist
	 */
};


/* mapping between positioned volumes corresponding to a rigid rotation
 */
class RotMappingViews : public MappingViews {
protected:
	SpaceSize rotAxis;   // versors in feature space of rotation axis
	Point     rotCenter; // absolute coordinates in feature space of rotation center
	RotMat_t  rotMat;    // rotation matrix from feature to target
	RotMat_t  rotInvMat; // rotation matrix from target to feature

	RotMappingViews () {}

public:

	// constructors
	RotMappingViews ( SpaceSize Axis, Point Center, double Angle );

	RotMappingViews ( SpaceSize _rotAxis, Point _rotCenter, RotMat_t & _rotMat, RotMat_t & _rotInvMat ) {
		rotAxis=_rotAxis;
		rotCenter=_rotCenter;
		for (int i=0; i<3; i++)
			for (int j=0; j<3; j++) {
				rotMat[i][j]=_rotMat[i][j];
				rotInvMat[i][j]=_rotInvMat[i][j];
			}
	}

	// destructor
	~RotMappingViews ( ) { }

	bool map ( Point fPoint, Point *tPoint );
	/* maps point from feature to target
	 */

	bool mapback ( Point tPoint, Point *fPoint );
	/* maps back point from target to feature
	 */

	bool map_ss ( SpaceSize feature_ss, SpaceSize *target_ss );
	/* maps a space size (segment) from feature to target using rotation matrix
	 */

	bool mapback_ss ( SpaceSize target_ss, SpaceSize *feature_ss );
	/* maps back a space size (segment) from target to feature using invrotation matrix
	 */

	iim::real32 * mapbuffer(iim::real32 * buf, int dimV, int dimH, int dimD);

 	iim::uint8 * mapbuffer(iim::uint8 * buf, int dimV, int dimH, int dimD, int channels, int bytes_x_chan);

	void mapdims ( int dimf_V, int dimf_H, int dimf_D, int *dimt_V, int *dimt_H, int *dimt_D );

	// getters
	SpaceSize getU ( )         { return rotAxis; }
	Point     getC ( )         { return rotCenter; }
	//RotMat_t *getROTMAT ( )    { return &rotMat; }     // non si può passare un array in questo modo
	//RotMat_t *getROTINVMAT ( ) { return &rotInvMat; }  // non si può passare un array in questo modo
};


/* mapping between positioned volumes corresponding to a rigid rotation of 180 degree */

class MappingView180 : public RotMappingViews {
private:
	int mapInd(int fI, int fJ, int fK, int dimV, int dimH, int dimD);
	// map from feature indices to target buffer index (returned)

public:

	// constructors
	MappingView180 ( SpaceSize Axis, Point Center );

	// destructor
	~MappingView180 ( ) { }

 	iim::real32 * mapbuffer(iim::real32 * buf, int dimV, int dimH, int dimD);
	// Returns a copy of data in buf transformed according to this mapping

 	iim::uint8 * mapbuffer(iim::uint8 * buf, int dimV, int dimH, int dimD, int channels, int bytes_x_chan);

	void mapdims ( int dimf_V, int dimf_H, int dimf_D, int *dimt_V, int *dimt_H, int *dimt_D );

};

# endif

