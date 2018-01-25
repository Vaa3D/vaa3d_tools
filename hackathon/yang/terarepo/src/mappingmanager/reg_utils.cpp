/*
 * reg_utils.cpp
 *
 * created August 29, 2012 by Giulio Iannello
 * changed May 02, 2016 by Pierangelo Afferni
 */

# include "reg_utils.h"

SpaceSize getMOVE_INFO ( Point fromPoint, Point toPoint ) {
	SpaceSize move_info;
	move_info.setx(toPoint.x() - fromPoint.x());
	move_info.sety(toPoint.y() - fromPoint.y());
	move_info.setz(toPoint.z() - fromPoint.z());
	return move_info;
}

void reorder_VERTICES ( Point *p0, Point *p1, SpaceSize vxlsz ) {
	COORDSTYPE tmp;
	SpaceSize diff = getMOVE_INFO((*p0),(*p1));
	if ( (vxlsz.x() * diff.x()) < 0 ) {
		tmp = p0->x();
		p0->setx(p1->x());
		p1->setx(tmp);
	}
	if ( (vxlsz.y() * diff.y()) < 0 ) {
		tmp = p0->y();
		p0->setx(p1->y());
		p1->sety(tmp);
	}
	if ( (vxlsz.z() * diff.z()) < 0 ) {
		tmp = p0->z();
		p0->setz(p1->z());
		p1->setz(tmp);
	}
}

bool intersect_VOLUMES ( Point p00, Point p01, Point p10, Point p11, SpaceSize vxlsz, Point *p20, Point *p21 ) {
	SpaceSize dummy(1,1,1);
	reorder_VERTICES(&p00,&p01,dummy); // reorder vertices of subvolume with respect to the natural reference system
	reorder_VERTICES(&p10,&p11,dummy); // reorder vertices of subvolume with respect to the natural reference system
	// compute intersection
	p20->setx(myMAX(p00.x(),p10.x()));
	p20->sety(myMAX(p00.y(),p10.y()));
	p20->setz(myMAX(p00.z(),p10.z()));
	p21->setx(myMIN(p01.x(),p11.x()));
	p21->sety(myMIN(p01.y(),p11.y()));
	p21->setz(myMIN(p01.z(),p11.z()));
	if ( isempty_VOLUME((*p20),(*p21),dummy) ) { // domanda perché confronto con dummy=(1,1,1) ?
		// empty volume: set both vertices to (0,0,0)
		p20->setvals(0,0,0);
		p21->setvals(0,0,0);
		return false;
	}
	else {
		// not empty volume: reorder vertices of subvolume with respect to the image reference system
		reorder_VERTICES(p20,p21,vxlsz);
		return true;
	}
}

bool isempty_VOLUME ( Point p0, Point p1, SpaceSize vxlsz ) {
	if ( (vxlsz.x() * (p1.x() - p0.x()) < MIN_VOLUME_DIMENSION) ||
		 (vxlsz.y() * (p1.y() - p0.y()) < MIN_VOLUME_DIMENSION) ||
		 (vxlsz.z() * (p1.z() - p0.z()) < MIN_VOLUME_DIMENSION)    )
		return true;
	else
		return false;
}


