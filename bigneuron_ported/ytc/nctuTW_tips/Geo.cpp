#include "Geo.h"
#include <cmath>

bool VectorCordinate::operator == (const VectorCordinate V)
{
	if( x == V.x && y == V.y && z == V.z) return true;
	else return false;
}

VectorCordinate VectorCordinate::operator - (const struct _Vector_Coordinate V) const
{
		VectorCordinate tempV;
		tempV.x = x - V.x;
		tempV.y = y - V.y;
		tempV.z = z - V.z;

		return tempV;

}
VectorCordinate VectorCordinate::operator * (double s) 
{
		VectorCordinate tempV;
		tempV.x = s * x;
		tempV.y = s * y;
		tempV.z = s * z;

		return tempV;

}

void VectorCordinate::operator /= (double ratio) 
{
	x /= ratio; y /= ratio; z /= ratio;
}

void VectorCordinate::operator *= (double ratio )
{
	x *= ratio; y *= ratio; z *= ratio;
}

double L2_Norm(const VectorCordinate Vector)
{
	return  sqrt(Vector.x * Vector.x + Vector.y * Vector.y + Vector.z * Vector.z); 
}

double L2_Norm(const VectorCordinate S_Vector, const VectorCordinate E_Vector)
{
	double Dif_x, Dif_y, Dif_z;

	Dif_x = S_Vector.x - E_Vector.x;
	Dif_y = S_Vector.y - E_Vector.y;
	Dif_z = S_Vector.z - E_Vector.z;

	return sqrt(Dif_x * Dif_x + Dif_y * Dif_y + Dif_z * Dif_z); 
}

double Point_Line_Disatnce(const GeneralLine Line, VectorCordinate Point)
{
	VectorCordinate DifVector;
	DifVector = Point - Line.RefPoint;
	double DifVectorLength = L2_Norm(DifVector);
	
	if(DifVectorLength == 0 ) return 0.0;

	DifVector.x /= DifVectorLength;
	DifVector.y /= DifVectorLength;
	DifVector.z /= DifVectorLength;
	
	double LineVecLength = L2_Norm(Line.LineVector);

	if(LineVecLength == 0.0) 
	{
		std::cout<<"T_T\n";
		exit(1);
	}
	VectorCordinate UnitLineVec = Line.LineVector;
	UnitLineVec.x /= LineVecLength;
	UnitLineVec.y /= LineVecLength;
	UnitLineVec.z /= LineVecLength;

	double Cosin = DifVector.x * UnitLineVec.x + DifVector.y * UnitLineVec.y + DifVector.z * UnitLineVec.z;

	return DifVectorLength * sqrt( 1.0 - Cosin * Cosin );

}

double Point_LineSeg_Distance(const LineSegment LineSeg, VectorCordinate Point)
{
	VectorCordinate DifVector;
	DifVector = Point - LineSeg.S_Point;
	double DifVectorLength = L2_Norm(DifVector);

	DifVector.x /= DifVectorLength;
	DifVector.y /= DifVectorLength;
	DifVector.z /= DifVectorLength;
	
	double Cosin = DifVector.x * LineSeg.LineVector.x + DifVector.y * LineSeg.LineVector.y + DifVector.z * LineSeg.LineVector.z;	
	
//	std::cout<<"cosin: "<<Cosin<<'\n';

	if( DifVectorLength * Cosin  > 0 && DifVectorLength * Cosin  < 1)
//		|| LineSeg.S_Point.x + DifVectorLength * Cosin * DifVector.x < LineSeg.S_Point.x && LineSeg.S_Point.x + DifVectorLength * Cosin * DifVector.x > LineSeg.E_Point.x)
		return DifVectorLength * sqrt( 1.0 - Cosin * Cosin );
	else
	{
		if( L2_Norm(Point, LineSeg.S_Point) < L2_Norm(Point, LineSeg.E_Point) ) return L2_Norm(Point, LineSeg.S_Point);
		else return L2_Norm(Point, LineSeg.E_Point);
	}
/*	if( LineSeg.S_Point.x + DifVectorLength * Cosin * DifVector.x > LineSeg.S_Point.x && LineSeg.S_Point.x + DifVectorLength * Cosin * DifVector.x < LineSeg.E_Point.x
		|| LineSeg.S_Point.x + DifVectorLength * Cosin * DifVector.x < LineSeg.S_Point.x && LineSeg.S_Point.x + DifVectorLength * Cosin * DifVector.x > LineSeg.E_Point.x)
		return DifVectorLength * sqrt( 1.0 - Cosin * Cosin );
	else
	{
		if( L2_Norm(Point, LineSeg.S_Point) < L2_Norm(Point, LineSeg.E_Point) ) return L2_Norm(Point, LineSeg.S_Point);
		else return L2_Norm(Point, LineSeg.E_Point);
	}
*/
}

double Point_LineSeg_Distance(const LineSegment LineSeg, VectorCordinate Point, VectorCordinate& ProjectionPoint)
{
	VectorCordinate DifVector;
	DifVector = Point - LineSeg.S_Point;
	double DifVectorLength = L2_Norm(DifVector);

	DifVector.x /= DifVectorLength;
	DifVector.y /= DifVectorLength;
	DifVector.z /= DifVectorLength;
	
	double Cosin = DifVector.x * LineSeg.LineVector.x + DifVector.y * LineSeg.LineVector.y + DifVector.z * LineSeg.LineVector.z;	
	
//	std::cout<<"cosin: "<<Cosin<<'\n';

/*	if( LineSeg.S_Point.x + DifVectorLength * Cosin * DifVector.x > LineSeg.S_Point.x && LineSeg.S_Point.x + DifVectorLength * Cosin * DifVector.x < LineSeg.E_Point.x
		|| LineSeg.S_Point.x + DifVectorLength * Cosin * DifVector.x < LineSeg.S_Point.x && LineSeg.S_Point.x + DifVectorLength * Cosin * DifVector.x > LineSeg.E_Point.x)
*/
	if( DifVectorLength * Cosin  > 0 && DifVectorLength * Cosin  < 1)
	{
		ProjectionPoint.x = LineSeg.S_Point.x + DifVectorLength * Cosin * DifVector.x;
		ProjectionPoint.y = LineSeg.S_Point.y + DifVectorLength * Cosin * DifVector.y;
		ProjectionPoint.z = LineSeg.S_Point.z + DifVectorLength * Cosin * DifVector.z;

		return DifVectorLength * sqrt( 1.0 - Cosin * Cosin );
	}
	else
	{
		if( L2_Norm(Point, LineSeg.S_Point) < L2_Norm(Point, LineSeg.E_Point) )
		{
			ProjectionPoint = LineSeg.S_Point;
			return L2_Norm(Point, LineSeg.S_Point);
		}
		else
		{
			ProjectionPoint = LineSeg.E_Point;
			return L2_Norm(Point, LineSeg.E_Point);
		}
	}

}
double Angles_Between_3D_Vectors(const VectorCordinate UnitVector2, const VectorCordinate Vector2)
{
	double UnitLength = L2_Norm(UnitVector2);
	double Length = L2_Norm(Vector2);

	VectorCordinate Vector = Vector2;
	VectorCordinate UnitVector = UnitVector2;

	Vector.x /= Length; Vector.y /= Length; Vector.z /= Length;
	UnitVector.x /= UnitLength; UnitVector.y /= UnitLength; UnitVector.z /= UnitLength;

	double Cosin = Vector.x * UnitVector.x + Vector.y * UnitVector.y + Vector.z * UnitVector.z;
	double tan =  ( sqrt(1 - Cosin * Cosin) ) / Cosin;

	return atan(tan);

}

bool Predicate_3DSphere(const VectorCordinate& P, const Sphere_3D& S)  //if P is inside the S return true
{
	return ( L2_Norm(P - S.Center) <= S.Radius );
}

double InnerProduct3(const VectorCordinate v1, const VectorCordinate v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}