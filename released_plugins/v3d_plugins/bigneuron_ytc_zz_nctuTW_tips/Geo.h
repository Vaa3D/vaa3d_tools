/*
P.C Lee cis dept. NCTU
*/

#ifndef _GEO_H
#define _GEO_H
#include <iostream>

typedef struct _Vector_Coordinate
{

	_Vector_Coordinate(double i, double j, double k): x(i), y(j), z(k){};
	_Vector_Coordinate():x(0.0), y(0.0), z(0.0){};
	_Vector_Coordinate(const _Vector_Coordinate& rhs)
	{
		x = rhs.x; y = rhs.y; z = rhs.z;
	}

	double x, y, z;

//	friend std::ifstream& operator >> (std::ifstream& infile, struct _Vector_Cordinate V);
	bool operator == (const struct _Vector_Coordinate V);
	struct _Vector_Coordinate operator - (const struct _Vector_Coordinate V) const;
	void operator /= (double ratio) ;
	void operator *= (double ratio );
	
	struct _Vector_Coordinate operator* ( double s );

	inline _Vector_Coordinate operator+= ( const _Vector_Coordinate& rhs )
	{
		x += rhs.x; y += rhs.y; z += rhs.z;
		return *this;
	}

	inline _Vector_Coordinate operator+ (const _Vector_Coordinate& rhs)
	{
		return (*this += rhs); 
	}

    _Vector_Coordinate& operator= (const _Vector_Coordinate& rhs)
	{
		x = rhs.x; 
		y = rhs.y;
		z = rhs.z;

		return *this;
	}

	friend std::istream& operator>> ( std::istream& is, _Vector_Coordinate& Vec)
	{
		is>>Vec.x>>Vec.y>>Vec.z;
		return is;
	}

	friend std::ostream& operator<< (std::ostream& os, _Vector_Coordinate& Vec)
	{
		os<<Vec.x<<'\t'<<Vec.y<<'\t'<<Vec.z;
		return os;
	}
}VectorCordinate;

typedef struct _Line
{
	_Line( VectorCordinate Vec, VectorCordinate RefPt) {LineVector = Vec; RefPoint = RefPt;}
	VectorCordinate LineVector;
	VectorCordinate RefPoint;
}GeneralLine;

typedef struct _Line_Segment
{
	VectorCordinate LineVector;
	VectorCordinate S_Point;
	VectorCordinate E_Point;
}LineSegment;

typedef struct _3DSphere
{
	_3DSphere(const VectorCordinate vec, double r) : Center(vec), Radius(r) {}
	_3DSphere():Radius(.0) {}
	
	VectorCordinate Center;
	double Radius;
}Sphere_3D;

bool Predicate_3DSphere(const VectorCordinate& P, const Sphere_3D& S);
double L2_Norm(const VectorCordinate Vector);  
double L2_Norm(const VectorCordinate S_Vector, const VectorCordinate E_Vector);
double Point_Line_Disatnce(const GeneralLine Line, const VectorCordinate Point);
double Point_LineSeg_Distance(const LineSegment LineSeg, VectorCordinate Point);
double Point_LineSeg_Distance(const LineSegment LineSeg, VectorCordinate Point, VectorCordinate& ProjectionPoint);
//double Angles_Between_3D_Vectors(const VectorCordinate Vector1, const VectorCordinate Vector2, const VectorCordinate RefPoint ,double& alpha);
double Angles_Between_3D_Vectors(const VectorCordinate Vector1, const VectorCordinate Vector2);

double InnerProduct3(const VectorCordinate v1, const VectorCordinate v2);
#endif
