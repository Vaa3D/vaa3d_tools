/*
The Mathematics of the 3D Rotation Matrix: Source Code 
Diana Gruber 
Note: This is NOT THE SOURCE CODE TO FASTGRAPH. This is just some C++ code I wrote over the weekend to illustrate the ideas in my paper The Mathematics of the 3D Rotation Matrix. You may use this code however you see fit. 
Recently I have been benchmarking this code against Fastgraph. Fastgraph is written in assembly language, and takes advantage of the speed boost available using concurrent floating point operations. There are 3 floating point registers. Vectors consist of 3 floats. You can do 3 floating point operations at the same time. Get it? 

So far, Fastgraph beats this code by 200%-500%, depending on such things as what compiler switches you apply. There are further optimizations that can be applied to this C++ code. In many cases you don't need to do a complete matrix multiply, for example. Since the first 3 columns of the transform matrix are identical to the rotation matrix, you can take a shortcut and just calculate the 4th column.

Source code here is presented "as is". Use it at your own risk. Please report any errors to dgruber@fastgraph.com. Don't flame me, just report the errors. 
*/


//---------------------------------------------------------------------------
/* 3D.h: declarations for 3D rotation/translation class 3D.cpp */

#ifndef _3DH
#define _3DH

//---------------------------------------------------------------------------
class _3D
{
private:

   // internal vectors and matrices
   double WorldUp[3];
   double ViewMoveMatrix[12];
   double ViewRotationMatrix[12];
   double WorldTransform[12];
   double ObjectMoveMatrix[12];
   double ObjectRotationMatrix[12];
   double ObjectTransform[12];
   double CombinedTransform[12];

   // helper functions
   void MatrixMultiply(double *A, double *B, double *C);
   void MoveFill(double *A, double Cx, double Cy, double Cz);
   void RotateFill(double *A, double Cx, double Cy, double Cz);

public:
 
   double pi;

   _3D(void); // constructor

   // 3D functions
   void _3DAxisAngle (double x, double y, double z, double theta);
   void _3DAxisAngleObject (double x, double y, double z, double theta);
   void _3DGetView (double *x1, double *y1, double *z1, double *x2, double *y2, double *z2);
   int  _3DLookAt (double x1, double y1, double z1, double x2, double y2, double z2);
   void _3DPOV(double Cx, double Cy, double Cz, double xAngle, double yAngle, double zAngle);
   void _3DRoll(double theta);
   void _3DRollObject(double theta);
   void _3DRotateRight(double theta);
   void _3DRotateRightObject(double theta);
   void _3DRotateUp(double theta);
   void _3DRotateUpObject(double theta);
   void _3DMove(double x, double y, double z);
   void _3DMoveObject(double x, double y, double z);
   void _3DMoveForward(double n);
   void _3DMoveForwardObject(double n);
   void _3DMoveRight(double n);
   void _3DMoveRightObject(double n);
   void _3DMoveUp(double n);
   void _3DMoveUpObject(double n);
   void _3DSetObject(double Ox, double Oy, double Oz, int xAngle, int yAngle, int zAngle);
   void _3DUpVector(double x, double y, double z);

	void _3DView (double *x1, double *y1, double *z1, double *x2, double *y2, double *z2);

};
#endif

/*

            Absolute Moves
            --------------
View                             Object
----                             ------
_3DMove()                        _3DMoveObject


            Relative Moves
            --------------
View                             Object
----                             ------
 _3DMoveForward()                _3DMoveForwardObject()
 _3DMoveRight()                  _3DMoveRightObject()
 _3DMoveUp()                     _3DMoveUpObject()


            Absolute Rotations
            ------------------
View                             Object
----                             ------
_3DAxis/Angle()                  _3DAxis/AngleObject()


            Relative Rotations (roll, pitch, yaw)
            -------------------------------------
View                             Object
----                             ------
_3DRoll()                        _3DRollObject()
_3DRotateRight()                 _3DRotateRightObject()
_3DRotateUp()                    _3DRotateUpObject


            Absolute Move + Absolute Rotation using Euler Angles
            -----------------------------------------
View                             Object
----                             ------
_3DPOV()                         _3DSetObject()


            Absolute Move + Absolute Rotation using Look At vector
            -----------------------------------------
View                             Object
----                             ------
_3DLookAt()                       ?

*/

 

