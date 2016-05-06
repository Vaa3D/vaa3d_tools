/*
Y.T Ching	cis dept. NCTU
*/
#if !defined(__Point_h)
#define __Point_h 1

const int Dim=3;

class Point {
 protected:
  double P[Dim];

 public:
  Point();
  Point(double,double,double);
  Point (double *);
  Point (const Point &);

  //overload operator
  Point & operator = (const Point &);
  Point operator + (const Point) const;
  Point operator - (const Point) const;
  Point operator * (const double) const;
  double operator * (const Point) const;
  Point operator ^ (const Point) const; //Overload ^ to cross product.
  int operator == (const Point) const;
  int operator != (const Point) const;

  //utility functions
  void X(double);
  void Y(double);
  void Z(double);
  double X();
  double Y();
  double Z();
  void Print();
  //void Print(FILE );

  double Distance (Point p);

  //Other functions
  double Length();
  Point Normalize();

};
#endif
