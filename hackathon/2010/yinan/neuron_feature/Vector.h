#ifndef VECTOR_H
#define VECTOR_H
#include <math.h>
#define PI 3.14159265359

class Vector {
protected:    // Make these variables available to the subclasses
	
	int type;
   double x;
   double y;
   double z;   
   double rho;
   
   double azimut;
   
   double elevation;
   
   double torch;
   
public:
	Vector();
	~Vector(){};
	Vector(Vector* a);
	Vector(double x, double y, double z);

	double distance(Vector* b);
	void setPolar(double r, double a, double e,double t);

	void setX(double v){x=v;}
	void setY(double v){y=v;}
	void setZ(double v){z=v;}
	double getX(){return x;}
	double getY(){return y;}
	double getZ(){return z;}	
	int getType(){return type;}
	//polar coord
	double getRho(){
		if(type==0)
			return sqrt(x*x+y*y+z*z);
		return rho;
	};

	void polarToRelativeCartesian(double x, double y, double z);
	void cartesianToPolar();
	double getTheta();
	double getPhi();
	double getTorch();
	void setTheta(double the){if(type==1) elevation=the;};
	void setPhi(double phi){if(type==1) azimut=phi;}
	void setTorch(double t){if(type==1) torch=t;}

	double anglePlane(Vector * a1, Vector* a2,Vector * Perp);
	void set(double x,double y, double z);
	double scalar(Vector * orig,Vector *A,Vector * B);
	double scalar(Vector * orig1,Vector *A, Vector * orig2,Vector * B);
	void crossProduct(Vector * o,Vector * a, Vector *b);
	void crossProduct(Vector * o,Vector * a,Vector * o1, Vector *b1);
	double angle(Vector* A,  Vector* B,  Vector* C);
	double angle(Vector* A,  Vector* B);
	//result in radiants
	double angleR(Vector* A,  Vector* B);
	double angle(Vector * O,Vector* A, Vector* Q, Vector* B);
	void rotate(double alpha, double beta, double gamma);
	void rotateP(double thetaP, double phiP);
	void rotateAxis(double xA,double yA, double zA, double theta);
	void rotateAxis(Vector *a, double theta);
	void rotateP(Vector *B);
}
;
#endif
