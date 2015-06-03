#include <math.h>
#include <stdio.h>
#include "Point3D.h"

Point::Point()
{
  for (int i=0;i<Dim;i++) P[i]=0;
}

Point::Point(double x, double y, double z=0.0)
{
  P[0]=x; P[1]=y; P[2]=z;
}

Point::Point(double v[])
{
  for (int i=0;i<Dim;i++) P[i] = v[i];
}

Point::Point(const Point & p)
{
  for (int i=0;i<Dim;i++) P[i] = p.P[i];
}

Point & Point::operator = (const Point & p) 
{
  if (this != &p) for (int i=0;i<Dim;i++) P[i] = p.P[i];
  return *this;
}

Point Point::operator + (const Point p) const
{
  Point tmp;
  for (int i=0;i<Dim;i++) tmp.P[i] = P[i]+p.P[i];
  return tmp;
}

Point Point::operator - (const Point p) const
{
  Point tmp;
  for (int i=0;i<Dim;i++) tmp.P[i] = P[i]-p.P[i];
  return tmp;
}


Point Point::operator * (const double k) const 
{
  Point tmp;
  for (int i=0;i<Dim;i++) tmp.P[i] = P[i]*k;
  return tmp;
}

double Point::operator * (const Point p) const
{
  double tmp=0;
  for (int i=0;i<Dim;i++) tmp = tmp + P[i]*p.P[i];
  return tmp;
}

Point Point::operator ^ (const Point p) const //Overload ^ to cross product.
{
  Point tmp;
  tmp.P[0] = P[1]*p.P[2]-P[2]*p.P[1];
  tmp.P[1] = P[2]*p.P[0]-P[0]*p.P[2];
  tmp.P[2] = P[0]*p.P[1]-P[1]*p.P[0];
  return tmp;
}

int Point::operator == (const Point p) const
{
  return (P[0]==p.P[0] && P[1]==p.P[1] && P[2]==p.P[2]);
}

int Point::operator != (const Point p) const
{
  return (P[0]!=p.P[0] || P[1]!=p.P[1] || P[2]!=p.P[2]);
}

//utility functions
void Point::X(double x)
{
  P[0] = x;
}

void Point::Y(double y)
{
  P[1] = y;
}

void Point::Z(double z)
{
  P[2] = z;
}

double Point::X()
{
  return P[0];
}

double Point::Y()
{
  return P[1];
}

double Point::Z()
{
  return P[2];
}

void Point::Print()
{
  printf ("(%g, %g, %g)\n",P[0],P[1],P[2]);
}

//void Point::Print(FILE  fp)
//{
//  fprintf (fp,"%f %f %f",P[0],P[1],P[2]);
//}

//Other functions
double Point::Length()
{
  return sqrt(P[0]*P[0]+P[1]*P[1]+P[2]*P[2]);
}

Point Point::Normalize()
{
  Point tmp;
  double length = sqrt(P[0]*P[0]+P[1]*P[1]+P[2]*P[2]);
  for (int i=0;i<Dim;i++) tmp.P[i] = P[i] / length;
  return tmp;
}

double Point::Distance (Point p)
{
  double tmp=0;
  tmp = tmp + (P[0]-p.X())*(P[0]-p.X());
  tmp = tmp + (P[1]-p.Y())*(P[1]-p.Y());
  tmp = tmp + (P[2]-p.Z())*(P[2]-p.Z());
  return sqrt (tmp);
}
