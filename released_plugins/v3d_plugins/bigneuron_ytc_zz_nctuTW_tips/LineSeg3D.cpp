#include <math.h>
#include <stdio.h>
#include "LineSeg3D.h"

//Point p1,p2; //Line segment is from p1 to p2
//double xb,yb,xe,ye; //Line segment is from (xb,yb) to (xe,ye)
//double m;    //slope
//double a,b;  //ax+by=1;

LineSeg::LineSeg()
{
  Point tmp;
  a_point = 1;
  p1 = tmp; p2 = tmp;
  xb = 0; yb = 0; zb = 0;
  xe = 0; ye = 0; ze = 0;
  //  slope = 0;
  //  a=b=c=0;
}

LineSeg::LineSeg(Point p, Point q)
{
  p1 = p; p2 = q;
  xb = p.X(); yb = p.Y(); zb = p.Z();
  xe = q.X(); ye = q.Y(); ze = q.Z();

  if (p==q) {a_point=1;}
  else a_point = 0;
  
}

LineSeg::LineSeg(double x1, double y1, double z1, double x2, double y2, double z2)
{
  p1.X(x1); p1.Y(y1); p1.Z(z1);
  p2.X(x2); p2.Y(y2); p2.Z(z1);
  xb = x1; yb = y1; zb = z1;
  xe = x2; ye = y2; ze = z2;

  if (p1==p2) {a_point=1;}
  else a_point =0;
}

LineSeg::LineSeg (const LineSeg& l)
{
  p1 = l.p1; p2 = l.p2;
  xb = p1.X(); yb = p1.Y(); zb = p1.Z();
  xe = p2.X(); ye = p2.Y(); ze = p2.Z();

  if (p1==p2) {a_point =1;}
  else a_point = 0;
}

LineSeg & LineSeg::operator = (const LineSeg & l)
{
  if (this != &l){
    p1 = l.p1; p2 = l.p2;
    xb = p1.X(); yb = p1.Y(); zb = p1.Z();
    xe = p2.X(); ye = p2.Y(); ze = p2.Z();

    if (p1==p2) {a_point=1;}
    else a_point = 0;
    return *this;
  }
}

int LineSeg::operator == (const LineSeg l) const
{
  return (p1==l.p1 && p2 == l.p2);
}

int LineSeg::operator != (const LineSeg l) const
{
  return (p1 !=l.p1 || p2 != l.p2);
}


  //utility functions

// define a line segment by two Points
void LineSeg::P1P2(Point p, Point q)
{
  p1 = p; p2 = q;
  xb = p1.X(); yb = p1.Y(); zb = p1.Z();
  xe = p2.X(); ye = p2.Y(); ze = p2.Z();

  if (p1==p2) {a_point=1;}
  else a_point = 0;
}

// defined a line segment by a pair of coordiantes
void LineSeg::XY(double x1, double y1, double z1, double x2, double y2, double z2)
{
  xb = x1; yb = y1; zb = z1; xe = x2; ye = y2; ze = z2;
  p1.X(xb); p1.Y(yb); p1.Z(zb);
  p2.X(xe); p2.Y(ye); p2.Z(ze);

  if (p1==p2) {a_point = 1;}
  else a_point = 0;
}

Point LineSeg::P1()
{
  return p1;
}

Point LineSeg::P2()
{
  return p2;
}

double LineSeg::Xb()
{return xb;}

double LineSeg::Xe()
{return xe;}

double LineSeg::Yb()
{return yb;}

double LineSeg::Ye()
{return ye;}

double LineSeg::Zb()
{return zb;}

double LineSeg::Ze()
{return ze;}

void LineSeg::Print()
{
  printf ("from (xb %f yb %f)",xb,yb);
  printf ("to (xe %f ye %f)\n",xe,ye);
  //  printf ("slope = %f\n",slope);
  //  printf ("line equation is %f X+%f Y = %f \n",a,b,c);
}
//void Print(FILE );

//Other functions
double LineSeg::Length()
{
  return (sqrt((xb-xe)*(xb-xe) + (yb-ye)*(yb-ye) + (zb-ze)*(zb-ze)));
}

// distance between a line and a point p 
// if the perpendicular line droped from p is on the line segment
//   return the distance from p to the line
// otherwise return the distance from an end point to p
double LineSeg::Distance(Point p)
{
  double x,y,z,d1,d2;
  double alpha;
  double tmp;
  Point q;

  if (a_point) return 999999.9;
  else {
    x = xe-xb; y = ye-yb; z = ze-zb;
    tmp = (x*x + y*y + z*z);
    if (tmp<0.000001) printf ("%d \n",a_point);
    alpha = (p.X()*x+p.Y()*y+p.Z()*z-xb*x-yb*y-zb*z)/tmp;
    // printf ("alpha %f\n ",alpha);
    q.X(xb+alpha*x);
    q.Y(yb+alpha*y);
    q.Z(zb+alpha*z);
    d1 = q.Distance(p);
    d2 = p.Distance(q);
    //    printf ("distance %f %f\n",d1,d2);
    return d1;
  }
}
