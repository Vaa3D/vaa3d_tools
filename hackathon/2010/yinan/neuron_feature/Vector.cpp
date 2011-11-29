#include <iostream>
using namespace std;
#include "Vector.h"
#include <cmath>

Vector::Vector(double xx, double yy, double zz)
{

	x=xx;y=yy;z=zz;type=0;

}

Vector::Vector(){

	x=0;y=0;z=0;type=0;
}

Vector::Vector(Vector* a){
	if(a->getType()==0){
		type=0;
		x=a->getX();
		y=a->getY();
		z=a->getZ();
	} else {
		type=1;
		rho=a->getRho();
		azimut=a->getPhi();
		elevation=a->getTheta();
		torch=a->getTorch();
	}
}

void Vector::setPolar(double r, double a, double e,double t){
	rho=r;azimut=a;elevation=e;torch=t;type=1;	
};

double Vector::distance(Vector *b){
	
 	double disX=(this->getX() - b->getX());
	double disY=(this->getY() - b->getY());
	double disZ=(this->getZ() - b->getZ());

	return sqrt( disX*disX + disY*disY + disZ*disZ );

}

void Vector::polarToRelativeCartesian(double xx, double yy, double zz){
	if(type==1){
		type=0;
		//do the rotation around the axis
		azimut=azimut*cos(torch)+elevation*sin(torch);
		elevation=azimut*sin(torch)+elevation*cos(torch);

		x=xx+rho*cos(elevation)*cos(azimut);
		y=yy+rho*cos(elevation)*sin(azimut);
		z=zz+rho*sin(elevation);
	}
	

};
void Vector::cartesianToPolar(){
	if(type==0){
		type=1;
		rho=sqrt(x*x+y*y+z*z);
		//do the rotation around the axis
		azimut=atan(sqrt(x*x+y*y)/z);
		elevation=atan(y/x);

	}
	

};

double Vector::getTheta(){
		if(type==1)
			return elevation;
		
		if(z==0)
			if(x>=0){
				return 0;
			}else {
				return PI;
			}
		double theta=atan(z/x);
		if(x<0)
			return PI+theta;
		return theta;
}


double Vector::getPhi(){
		if(type==1)
			return azimut;
		double r=getRho();
		if (r==0)
			return 0;
		return acos(y/getRho());
}

double Vector::getTorch(){
		if(type==1)
			return torch;
		return 0;
		
}

void Vector::rotateP(Vector *B){
		double theta,phi;
		 theta=B->getTheta();
		 phi=B->getPhi();
	
	 
		rotateP(theta,phi);
}


void Vector::rotateAxis(Vector *a, double theta){
	  /* This function performs an axis/angle rotation. (x,y,z) is any 
		  vector on the axis. For greater speed, always use a unit vector 
		  (length = 1). In this version, we will assume an arbitrary 
		  length and normalize. */

	   double xA,yA,zA;
		xA=a->getX();
		yA=a->getY();
		zA=a->getZ();
		rotateAxis(xA,yA,zA,theta);
};

void Vector::rotateAxis(double xA,double yA, double zA, double theta){
	   double RotMat[12];
	  /* This function performs an axis/angle rotation. (x,y,z) is any 
		  vector on the axis. For greater speed, always use a unit vector 
		  (length = 1). In this version, we will assume an arbitrary 
		  length and normalize. */

	   double length;
	   double c,s,t,x1,y1,z1;

	   // normalize
	   length = sqrt(xA*xA + yA*yA + zA*zA);

	   // too close to 0, can't make a normalized vector
	   if (length < .000001)
		  return;

	   xA /= length;
	   yA /= length;
	   zA /= length;

	   // do the trig
	   c = cos(theta);
	   s = sin(theta);
	   t = 1-c;   

	   // build the rotation matrixA
	   RotMat[0] = t*xA*xA + c;
	   RotMat[1] = t*xA*yA - s*zA;
	   RotMat[2] = t*xA*zA + s*yA;
	   RotMat[3] = 0;
		x1=RotMat[0]*x+RotMat[1]*y+RotMat[2]*z;
	   RotMat[4] = t*xA*yA + s*zA;
	   RotMat[5] = t*yA*yA + c;
	   RotMat[6] = t*yA*zA - s*xA;
	   RotMat[7] = 0;
		y1=RotMat[4]*x+RotMat[5]*y+RotMat[6]*z;

	   RotMat[8] = t*xA*zA - s*yA;
	   RotMat[9] = t*yA*zA + s*xA;
	   RotMat[10] = t*zA*zA + c;
	   RotMat[11] = 0;
		z1=RotMat[8]*x+RotMat[9]*y+RotMat[10]*z;
		x=x1;y=y1;z=z1;

}

void Vector::rotateP(double thetaP, double phiP){
		double r=getRho();
		double theta=getTheta();
		double phi=getPhi();

		if(x>0){
			theta+=thetaP;
		}else {
			theta-=thetaP;
		}
		if(x>0){
			phi+=phiP;
		} else {
			phi-=phiP;
		}
		
		double a=r*sin(phi);
		double t=cos(theta);
		double t1=sin(theta);
		x=r*sin(phi)*cos(theta);
		z=r*sin(phi)*sin(theta);
		y=r*cos(phi);

}
	//rotate along x, y, z axis
void Vector::rotate(double alpha, double beta, double gamma){
		static double cx=0,sx=0,cy=0,sy=0,cz=0,sz=0;

			cx=cos(alpha);sx=sin(alpha);
			cy=cos(beta);sy=sin(beta);
			cz=cos(gamma);sz=sin(gamma);
		
		double x1,y1,z1;
		x1=x,y1=y;z1=z;

		x1=x*cy*cz+y*cy*sz-z*sy;
		y1=x*(sx*sy*cz-cx*sz)+y*(sx*sy*sz+cx*cz)+z*sx*cy;
		z1=x*(cx*sy*cz+sx*sz)+y*((cx*sy*sz-sx*cz))+z*(cx*cy);
		x=x1;y=y1;z=z1;

};


	//angle betweenthe vector OA and QB 
double Vector::angle(Vector * O,Vector* A, Vector* Q, Vector* B){
		double ax,ay,az,bx,by,bz,ox,oy,oz,qx,qy,qz;
		
		ox=O->getX();oy=O->getY();oz=O->getZ();
		ax=A->getX();ay=A->getY();az=A->getZ();
		qx=Q->getX();qy=Q->getY();qz=Q->getZ();
		bx=B->getX();by=B->getY();bz=B->getZ();

		Vector * B1=new Vector(bx-qx+ox,by-qy+oy,bz-qz+oz);
		return O->angle(A,B1);
		//--------------------------------------------
		//reposition of QB in OB
		bx=bx-qx+ox;
		by=by-qy+oy;
		bz=bz-qz+oz;

		
		double div=(sqrt((ax-ox)*(ax-ox)+(ay-oy)*(ay-oy)+(az-oz)*(az-oz))*sqrt((bx-ox)*(bx-ox)+(by-oy)*(by-oy)+(bz-oz)*(bz-oz)));
		if (div==0) return 0;
		return 180.0/PI*acos( ((ax-ox)*(bx-ox)+(ay-oy)*(by-oy)+(az-oz)*(bz-oz))/div );
		
	
}
	
	//angle betweenthe vector OA and OB
	// angle always between 0-180!!!!
double Vector::angle(Vector* A,  Vector* B){

		return 180.0/PI*angleR(A,B);
}
double Vector::angleR(Vector* A,  Vector* B){
		double ax,ay,az,bx,by,bz,ox,oy,oz;
		
		ox=x;oy=y;oz=z;
		ax=A->getX();ay=A->getY();az=A->getZ();
		bx=B->getX();by=B->getY();bz=B->getZ();

		double div=(sqrt((ax-ox)*(ax-ox)+(ay-oy)*(ay-oy)+(az-oz)*(az-oz))*sqrt((bx-ox)*(bx-ox)+(by-oy)*(by-oy)+(bz-oz)*(bz-oz)));
		double rad=acos( ((ax-ox)*(bx-ox)+(ay-oy)*(by-oy)+(az-oz)*(bz-oz))/div );
		if (div==0) return 0;
		return rad;
		
	
}
	//angle betweenthe vector OA and OB with perpendicular oC
	//full rotation angle 0-360!!!
	//origin in 0,0,0!!!
double Vector::angle(Vector* A,  Vector* B,  Vector* C){
		return A->angle(B,C);
		double ax,ay,az,bx,by,bz,ox,oy,oz,cx,cy,cz;
		
		ox=0;oy=0;oz=0;
		ax=A->getX();ay=A->getY();az=A->getZ();
		bx=B->getX();by=B->getY();bz=B->getZ();
		cx=C->getX();cy=C->getY();cz=C->getZ();

		double div=(sqrt((ax-ox)*(ax-ox)+(ay-oy)*(ay-oy)+(az-oz)*(az-oz))*sqrt((bx-ox)*(bx-ox)+(by-oy)*(by-oy)+(bz-oz)*(bz-oz)));
		double rad=acos( ((ax-ox)*(bx-ox)+(ay-oy)*(by-oy)+(az-oz)*(bz-oz))/div );

		
		Vector * d =new Vector();
		Vector *Orig =new Vector();
		d->crossProduct(new Vector(),A,B);
		double aa=Orig->angle(d,C);
		double aaa=A->angle(B,C);
	
		if (div==0) return 0;
		return 180.0/PI*rad;
		
	
}
	//the vector will contain the cross product of OAxO1B -> result is in OC
//the ^oc is compared with respect to ^a
//##ModelId=3F6DA234001A
void Vector::crossProduct(Vector * o,Vector * a,Vector * o1, Vector *b){

		setX( (a->getY() - o->getY())*(b->getZ() - o1->getZ()) - (a->getZ() - o->getZ())*(b->getY() - o1->getY()) );
		setY( (a->getZ() - o->getZ())*(b->getX() - o1->getX()) - (a->getX() - o->getX())*(b->getZ() - o1->getZ()) );
		setZ( (a->getX() - o->getX())*(b->getY() - o1->getY()) - (a->getY() - o->getY())*(b->getX() - o1->getX()) );
	
}	


void Vector::crossProduct(Vector * o,Vector * a, Vector *b){

		crossProduct(o,a,o,b);


	
}
// reimplemented the code as a.b instead of |a||b|cos@ during helix
//##ModelId=3F6DA23303CE
double Vector::scalar(Vector * orig1,Vector *A,Vector * orig2,Vector * B){

		double ax,ay,az,bx,by,bz,ox,oy,oz,qx,qy,qz;
		ox=orig1->getX();oy=orig1->getY();oz=orig1->getZ();
		ax=A->getX();ay=A->getY();az=A->getZ();
		qx=orig2->getX();qy=orig2->getY();qz=orig2->getZ();
		bx=B->getX();by=B->getY();bz=B->getZ();
	
		//reposition of B with respect to orig1 as to have a common point for both A and B
		bx=bx-qx+ox;
		by=by-qy+oy;
		bz=bz-qz+oz;
	
		return ((ax-ox)*(bx-ox)+(ay-oy)*(by-oy)+(az-oz)*(bz-oz));
		
}

//##ModelId=3F6DA23303CA
double Vector::scalar(Vector * orig,Vector *A,Vector * B){
	
		return scalar (orig, A, orig, B);
}

void Vector::set(double x,double y, double z){
		this->x=x;
		this->y=y;
		this->z=z;
}

	//return the angle between a1, a2 when those vectors are projected on a plane perpendiculat to Perp
double Vector::anglePlane(Vector * a1, Vector* a2,Vector * Perp){
		Vector *bb=a1;
		double a,b,c,x1,x2,y1,y2,z1,z2,u,x3,y3,z3,tmp;
		a=Perp->getX();b=Perp->getY();c=Perp->getZ();
		tmp=1000;
		a*=tmp;b*=tmp;c*=tmp;
		
		x1=bb->getX();y1=bb->getY();z1=bb->getZ();
		bb=Perp;
		//point 2 is from vector 1 adding the perpendicular
		x2=x1+bb->getX();y2=y1+bb->getY();z2=z1+bb->getZ();
		
		u=(a*(x1-x2)+b*(y1-y2)+c*(z1-z2));
		u=(a*x1+b*y1+c*z1)/u;
		//intersect of bb on the plane is:
		x3=x1+u*(x2-x1);
		y3=y1+u*(y2-y1);
		z3=z1+u*(z2-z1);
	
		// create virtual projection of vector a1 on plane p
		Vector *v1=new Vector(x3,y3,z3);

		//repeat process to get projection of a2 on p
		bb=a2;
		
		a=Perp->getX();b=Perp->getY();c=Perp->getZ();
		tmp=1000;
		a*=tmp;b*=tmp;c*=tmp;
		// point 1 is the end of the vector to extract is projection on the plane
		x1=bb->getX();y1=bb->getY();z1=bb->getZ();
		bb=Perp;
		//point 2 is from vector 1 adding the perpendicular
		x2=x1+bb->getX();y2=y1+bb->getY();z2=z1+bb->getZ();
		
		u=(a*(x1-x2)+b*(y1-y2)+c*(z1-z2));
		u=(a*x1+b*y1+c*z1)/u;
		//intersect of bb on the plane is:
		x3=x1+u*(x2-x1);
		y3=y1+u*(y2-y1);
		z3=z1+u*(z2-z1);

		// create virtual projection of vector a2 on plane p
		Vector *v2=new Vector(x3,y3,z3);

		double ang=angle(v1,v2);
		delete v1;
		delete v2;

		
		return ang;

}
;

