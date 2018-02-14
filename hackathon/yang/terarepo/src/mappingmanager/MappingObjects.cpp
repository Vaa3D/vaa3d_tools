/*
 * MappingObjects.cpp
 *
 * created September 3, 2012 by Giulio Iannello
 * changed May 02, 2016 by Pierangelo Afferni: added sub-class MappingView180 extending RotMappingViews
 */


/******************
*    CHANGELOG    *
*******************
* 2016-04-27 Pierangelo.      @CREATED
*/


# include "MappingObjects.h"

# include <math.h>

using namespace iim;


/****************************************************
 *
 *    RotMappingViews Methods
 *
 ****************************************************/

// compute the rotation matrix of an angle of theta radiants around rotation 
// axis rotaxis
//
//uxu = rotaxis.u' * rotaxis.u;
//ux  = ...
//    [      0        -rotaxis.u(3)   rotaxis.u(2) ; ...
//      rotaxis.u(3)       0         -rotaxis.u(1) ; ...
//     -rotaxis.u(2)   rotaxis.u(1)         0       ];
//rotmat = uxu + cos(theta) * (eye(3) - uxu) + sin(theta) * ux;
//
//rot_fcoords = (rotmat * (fpos - rotaxis.C)' + rotaxis.C')'

//[ x^2 + cos(theta)(1-x^2)         & xy - cos(theta)xy - sin(theta)z & xz - cos(theta)xz + sin(theta)y
//  yx - cos(theta)yx + sin(theta)z & y^2 + cos(theta)(1-y^2)         & yz - cos(theta)yz - sin(theta)x
//  zx - cos(theta)zx - sin(theta)y & zy - cos(theta)zy + sin(theta)x & z^2 + cos(theta)(1-z^2)          ]


RotMappingViews::RotMappingViews ( SpaceSize Axis, Point Center, double Angle ) {
	double uxu[3][3] = { { (Axis.val(0) * Axis.val(0)), (Axis.val(0) * Axis.val(1)), (Axis.val(0) * Axis.val(2)) },
						 { (Axis.val(1) * Axis.val(0)), (Axis.val(1) * Axis.val(1)), (Axis.val(1) * Axis.val(2)) },
						 { (Axis.val(2) * Axis.val(0)), (Axis.val(2) * Axis.val(1)), (Axis.val(2) * Axis.val(2)) } };
	double ux[3][3]  = { {            0,   -Axis.val(2),  Axis.val(1) },
						 {  Axis.val(2),              0, -Axis.val(0) },
						 { -Axis.val(1),    Axis.val(0),            0 } };
	rotMat[0][0] = myROUND(uxu[0][0] + cos(Angle) * (1 - uxu[0][0]) + sin(Angle) * ux[0][0],3.0);
	rotMat[0][1] = myROUND(uxu[0][1] + cos(Angle) * (  - uxu[0][1]) + sin(Angle) * ux[0][1],3.0);
	rotMat[0][2] = myROUND(uxu[0][2] + cos(Angle) * (  - uxu[0][2]) + sin(Angle) * ux[0][2],3.0);

	rotMat[1][0] = myROUND(uxu[1][0] + cos(Angle) * (  - uxu[1][0]) + sin(Angle) * ux[1][0],3.0);
	rotMat[1][1] = myROUND(uxu[1][1] + cos(Angle) * (1 - uxu[1][1]) + sin(Angle) * ux[1][1],3.0);
	rotMat[1][2] = myROUND(uxu[1][2] + cos(Angle) * (  - uxu[1][2]) + sin(Angle) * ux[1][2],3.0);

	rotMat[2][0] = myROUND(uxu[2][0] + cos(Angle) * (  - uxu[2][0]) + sin(Angle) * ux[2][0],3.0);
	rotMat[2][1] = myROUND(uxu[2][1] + cos(Angle) * (  - uxu[2][1]) + sin(Angle) * ux[2][1],3.0);
	rotMat[2][2] = myROUND(uxu[2][2] + cos(Angle) * (1 - uxu[2][2]) + sin(Angle) * ux[2][2],3.0);

	rotInvMat[0][0] = myROUND(uxu[0][0] + cos(-Angle) * (1 - uxu[0][0]) + sin(-Angle) * ux[0][0],3.0);
	rotInvMat[0][1] = myROUND(uxu[0][1] + cos(-Angle) * (  - uxu[0][1]) + sin(-Angle) * ux[0][1],3.0);
	rotInvMat[0][2] = myROUND(uxu[0][2] + cos(-Angle) * (  - uxu[0][2]) + sin(-Angle) * ux[0][2],3.0);

	rotInvMat[1][0] = myROUND(uxu[1][0] + cos(-Angle) * (  - uxu[1][0]) + sin(-Angle) * ux[1][0],3.0);
	rotInvMat[1][1] = myROUND(uxu[1][1] + cos(-Angle) * (1 - uxu[1][1]) + sin(-Angle) * ux[1][1],3.0);
	rotInvMat[1][2] = myROUND(uxu[1][2] + cos(-Angle) * (  - uxu[1][2]) + sin(-Angle) * ux[1][2],3.0);

	rotInvMat[2][0] = myROUND(uxu[2][0] + cos(-Angle) * (  - uxu[2][0]) + sin(-Angle) * ux[2][0],3.0);
	rotInvMat[2][1] = myROUND(uxu[2][1] + cos(-Angle) * (  - uxu[2][1]) + sin(-Angle) * ux[2][1],3.0);
	rotInvMat[2][2] = myROUND(uxu[2][2] + cos(-Angle) * (1 - uxu[2][2]) + sin(-Angle) * ux[2][2],3.0);

	rotAxis   = Axis;
	rotCenter = Center;
}


bool RotMappingViews::map ( Point fPoint, Point *tPoint ) {
	SpaceSize rel_fPoint = getMOVE_INFO(rotCenter,fPoint);
	tPoint->setval( 0, 
		rotMat[0][0]*rel_fPoint.val(0) + rotMat[0][1]*rel_fPoint.val(1) + rotMat[0][2]*rel_fPoint.val(2) + rotCenter.val(0) ); 
	tPoint->setval( 1, -
		rotMat[1][0]*rel_fPoint.val(0) + rotMat[1][1]*rel_fPoint.val(1) + rotMat[1][2]*rel_fPoint.val(2) + rotCenter.val(1) ); 
	tPoint->setval( 2, 
		rotMat[2][0]*rel_fPoint.val(0) + rotMat[2][1]*rel_fPoint.val(1) + rotMat[2][2]*rel_fPoint.val(2) + rotCenter.val(2)); 
	return true;
}


bool RotMappingViews::mapback ( Point tPoint, Point *fPoint ) {
	SpaceSize rel_tPoint = getMOVE_INFO(rotCenter,tPoint);
	fPoint->setval( 0, 
		rotInvMat[0][0]*rel_tPoint.val(0) + rotInvMat[0][1]*rel_tPoint.val(1) + rotInvMat[0][2]*rel_tPoint.val(2) + rotCenter.val(0) ); 
	fPoint->setval( 1, 
		rotInvMat[1][0]*rel_tPoint.val(0) + rotInvMat[1][1]*rel_tPoint.val(1) + rotInvMat[1][2]*rel_tPoint.val(2) + rotCenter.val(1) ); 
	fPoint->setval( 2, 
		rotInvMat[2][0]*rel_tPoint.val(0) + rotInvMat[2][1]*rel_tPoint.val(1) + rotInvMat[2][2]*rel_tPoint.val(2) + rotCenter.val(2)); 
	return true;
}


bool RotMappingViews::map_ss(SpaceSize feature_ss, SpaceSize *target_ss) {
	target_ss->setval( 0,
		rotMat[0][0]*feature_ss.val(0) + rotMat[0][1]*feature_ss.val(1) + rotMat[0][2]*feature_ss.val(2) );
		//myABS(rotMat[0][0])*feature_ss.val(0) + myABS(rotMat[0][1])*feature_ss.val(1) + myABS(rotMat[0][2])*feature_ss.val(2) );
	target_ss->setval( 1,
		rotMat[1][0]*feature_ss.val(0) + rotMat[1][1]*feature_ss.val(1) + rotMat[1][2]*feature_ss.val(2) );
		//myABS(rotMat[1][0])*feature_ss.val(0) + myABS(rotMat[1][1])*feature_ss.val(1) + myABS(rotMat[1][2])*feature_ss.val(2) );
	target_ss->setval( 2,
		rotMat[2][0]*feature_ss.val(0) + rotMat[2][1]*feature_ss.val(1) + rotMat[2][2]*feature_ss.val(2) );
		//myABS(rotMat[2][0])*feature_ss.val(0) + myABS(rotMat[2][1])*feature_ss.val(1) + myABS(rotMat[2][2])*feature_ss.val(2) );
	return true;
}


bool RotMappingViews::mapback_ss ( SpaceSize target_ss, SpaceSize *feature_ss ) {
	feature_ss->setval( 0, 
		rotInvMat[0][0]*target_ss.val(0) + rotInvMat[0][1]*target_ss.val(1) + rotInvMat[0][2]*target_ss.val(2) ); 
		//myABS(rotInvMat[0][0])*target_ss.val(0) + myABS(rotInvMat[0][1])*target_ss.val(1) + myABS(rotInvMat[0][2])*target_ss.val(2) ); 
	feature_ss->setval( 1, 
		rotInvMat[1][0]*target_ss.val(0) + rotInvMat[1][1]*target_ss.val(1) + rotInvMat[1][2]*target_ss.val(2) ); 
		//myABS(rotInvMat[1][0])*target_ss.val(0) + myABS(rotInvMat[1][1])*target_ss.val(1) + myABS(rotInvMat[1][2])*target_ss.val(2) ); 
	feature_ss->setval( 2, 
		rotInvMat[2][0]*target_ss.val(0) + rotInvMat[2][1]*target_ss.val(1) + rotInvMat[2][2]*target_ss.val(2) ); 
		//myABS(rotInvMat[2][0])*target_ss.val(0) + myABS(rotInvMat[2][1])*target_ss.val(1) + myABS(rotInvMat[2][2])*target_ss.val(2) ); 
	return true;
}


iim::real32 * RotMappingViews::mapbuffer(iim::real32 * buf, int dimV, int dimH, int dimD) {
	return 0;
}

iim::uint8 * RotMappingViews::mapbuffer(iim::uint8 * buf, int dimV, int dimH, int dimD, int channels, int bytes_x_chan) {
	return 0;
}

void RotMappingViews::mapdims ( int dimf_V, int dimf_H, int dimf_D, int *dimt_V, int *dimt_H, int *dimt_D ) {
}


/****************************************************
 *
 *    MappingView180 Methods
 *
 ****************************************************/
//rotmat = uxu - (eye(3) - uxu);

MappingView180::MappingView180(SpaceSize Axis, Point Center )
{

	int i,j,k;

	if  	(Axis.val(0)==1.0 && Axis.val(1)==0.0 && Axis.val(2)==0.0)
			{i=1; j=-1; k=-1;}

	else if ((int)Axis.val(0)==0 && (int)Axis.val(1)==1 && (int)Axis.val(2)==0)
			{i=-1; j=1; k=-1;}

	else if ((int)Axis.val(0)==0 && (int)Axis.val(1)==0 && (int)Axis.val(2)==1)
			{i=-1; j=-1; k=1;			}

	else
		{
		char errMsg[2000];
		sprintf(errMsg, "in MappeingView180::MappeingView180(...): %f %f %f Axis are not supported.", Axis.y(), Axis.x(), Axis.z());
		throw IOException(errMsg);
		}; // signal error


	rotMat[0][0] = i;
	rotMat[0][1] = 0;
	rotMat[0][2] = 0;

	rotMat[1][0] = 0;
	rotMat[1][1] = j;
	rotMat[1][2] = 0;

	rotMat[2][0] = 0;
	rotMat[2][1] = 0;
	rotMat[2][2] = k;

	rotInvMat[0][0] = i;
	rotInvMat[0][1] = 0;
	rotInvMat[0][2] = 0;

	rotInvMat[1][0] = 0;
	rotInvMat[1][1] = j;
	rotInvMat[1][2] = 0;

	rotInvMat[2][0] = 0;
	rotInvMat[2][1] = 0;
	rotInvMat[2][2] = k;

	rotAxis   = Axis;
	rotCenter = Center;
}


int MappingView180::mapInd(int fI, int fJ, int fK, int dimV, int dimH, int dimD) {
	int i,j,k;

	if  	((int)rotAxis.val(0)==1 && (int)rotAxis.val(1)==0 && (int)rotAxis.val(2)==0)
			{i=fI; j=dimH-fJ-1; k=dimD-fK-1;}

	else if ((int)rotAxis.val(0)==0 && (int)rotAxis.val(1)==1 && (int)rotAxis.val(2)==0)
			{i=dimV-fI-1; j=fJ; k=dimD-fK-1;}

	else if ((int)rotAxis.val(0)==0 && (int)rotAxis.val(1)==0 && (int)rotAxis.val(2)==1)
			{i=dimV-fI-1; j=dimH-fJ-1; k=fK;}

	else
		{
		char errMsg[2000];
		sprintf(errMsg, "in MappeingView180::MappeingView180(...): %f %f %f rotAxis are not supported.", rotAxis.y(), rotAxis.x(), rotAxis.z());
		throw IOException(errMsg);
		}; // signal error

	int tInd=(j + i*dimH + k*dimH*dimV);
	return tInd;
}


iim::real32 * MappingView180::mapbuffer(iim::real32 * buf, int dimV, int dimH, int dimD) {
	iim::real32*outbuf = new iim::real32[dimV * dimH * dimD];
	int tInd;  // target buffer index
	int fInd;  // feature buffer index

	for (int k=0; k<dimD; k++)
		for (int i=0; i<dimV; i++)
			for (int j=0; j<dimH; j++){
				fInd=j + i*dimH + k*dimH*dimV;
				tInd=mapInd(i,j,k, dimV, dimH, dimD);
				outbuf[tInd] = buf[fInd];
			}

	return outbuf;
}


iim::uint8 * MappingView180::mapbuffer(iim::uint8 * buf, int dimV, int dimH, int dimD, int channels, int bytes_x_chan) {
	iim::sint64 chan_size = dimV * dimH * dimD;
	iim::uint8 *outbuf = new iim::uint8[chan_size * channels * bytes_x_chan];
	int tInd;  // target buffer index
	int fInd;  // feature buffer index

	if ( bytes_x_chan == 1 ) {
		iim::uint8 *ptrbuf = outbuf;
		for ( int c=0; c<channels; c++, ptrbuf+=chan_size, buf+=chan_size ) {
			for (int k=0; k<dimD; k++)
				for (int i=0; i<dimV; i++)
					for (int j=0; j<dimH; j++){
						fInd=j + i*dimH + k*dimH*dimV;
						tInd=mapInd(i,j,k, dimV, dimH, dimD);
						ptrbuf[tInd] = buf[fInd];
					}
		}
	}
	else if ( bytes_x_chan == 2 ) {
		iim::uint16 *ptrbuf = (iim::uint16 *) outbuf;
		for ( int c=0; c<channels; c++, ptrbuf+=chan_size, buf+=(chan_size*bytes_x_chan) ) {
			for (int k=0; k<dimD; k++)
				for (int i=0; i<dimV; i++)
					for (int j=0; j<dimH; j++){
						fInd=j + i*dimH + k*dimH*dimV;
						tInd=mapInd(i,j,k, dimV, dimH, dimD);
						ptrbuf[tInd] = ((iim::uint16 *) buf)[fInd];
					}
		}
	}
	else {
		;
	}

	return outbuf;
}


void MappingView180::mapdims ( int dimf_V, int dimf_H, int dimf_D, int *dimt_V, int *dimt_H, int *dimt_D ) {
	*dimt_V = dimf_V;
	*dimt_H = dimf_H;
	*dimt_D = dimf_D;
}
