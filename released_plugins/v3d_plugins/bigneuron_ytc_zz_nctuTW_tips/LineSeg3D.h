/*
Y.T. Ching cis dept. NCTU
*/
#if !defined(__LineSeg_h)
#define __LineSeg_h 1
#include "Point3D.h"

class LineSeg{
 private:
  Point p1,p2; //Line segment is from p1 to p2
  double xb,yb,zb,xe,ye,ze; //Line segment is from (xb,yb,zb) to (xe,ye,ze)
  //  double slope;    //slope
  //  double a,b,c;  //ax+by=c;
  int a_point;

 public:
  LineSeg();
  LineSeg(Point p, Point q);
  LineSeg(double x1, double y1, double z1, double x2, double y2, double z2);
  LineSeg(const LineSeg&);

  //overload operator
  LineSeg & operator = (const LineSeg &);
  int operator == (const LineSeg) const;
  int operator != (const LineSeg) const;

  //utility functions
  void P1P2(Point,Point);
  void XY(double, double, double, double, double, double);
  Point P1();
  Point P2();
  double Xb();
  double Yb();
  double Zb();
  double Xe();
  double Ye();
  double Ze();
  void Print();
  //void Print(FILE );

  //Other functions
  double Length();
  double Distance(Point p);
  //  double Slope();
  double Angle(LineSeg);
};
#endif
