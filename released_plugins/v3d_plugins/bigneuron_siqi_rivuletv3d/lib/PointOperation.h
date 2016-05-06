/*=========================================================================
Copyright 2009 Rensselaer Polytechnic Institute
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. 
=========================================================================*/

/*=========================================================================

  Program:   Farsight Biological Image Segmentation and Visualization Toolkit
  Language:  C++
  Date:      $Date:  $
  Version:   $Revision: 0.00 $

=========================================================================*/
#ifndef PointOPERATION_H
#define PointOPERATION_H

#include <iostream>
#include <fstream>
#include <vector>
#include "math.h"
#include <vnl/vnl_matrix.h>
#include <vnl/vnl_vector.h>
#include <vnl/algo/vnl_matrix_inverse.h>
#include "itkNumericTraits.h"
//#include "ImageOperation.h"
extern const double pi;


class Point3D;
class Vector3D;
class PointList3D;

class Point3D
{
public:
	int index;
	float x;
	float y;
	float z;
	int tag;

	Point3D(void);
	Point3D(float X, float Y, float Z, int TAG = 0, int INDEX = 0);
	Point3D operator = (Vector3D V);
	Point3D operator + (Vector3D V);
	Point3D operator + (Point3D P);
	Point3D operator - (Point3D P);
	Point3D operator * (float Scalar);
    Point3D operator / (float Scalar);
	Point3D operator-(Vector3D V);
	bool check_out_of_range_3D(int IM, int IN, int IZ);
	bool check_out_of_range_2D(int IM, int IN, int IZ);
	int Inpolygon2D(PointList3D Contour);
	Point3D  operator = (Point3D P);
	float GetDistTo(Point3D P);
    float GetDistTo2D(Point3D P);
	float GetDistTo(Point3D P, int Z_Weight);
	void Print(void);

};

class Vector3D
{
public:
	float x;
	float y;
	float z;
	Vector3D(void);
	Vector3D(float NewX, float NewY, float NewZ);
	Vector3D(Point3D P1, Point3D P2);

	Vector3D	operator=(Vector3D V);
	Vector3D	operator+(Vector3D V);
	Vector3D  operator-(Vector3D V);
	Vector3D	operator+(Point3D P);
	Vector3D	operator-(Point3D P);
	Vector3D	operator*(float Scaler);
	Vector3D	operator/(float Scaler);
	void SetEndPoints(Point3D P1, Point3D P2);
	void SetAngle(float theta);
	void SetMagnitude(float Mag);
	float GetMagnitude(void);
	float GetAngleTo(Vector3D V);
	float GetAngle1(void);
	float GetAngle2(void);
	float GetDProduct(Vector3D V);
	Vector3D GetUnit(void);
	void ConvertUnit(void);
};

class PointList3D
{
public:
	//Point3D *Pt;
	PointList3D(void);
	//PointList3D(int N);

	Point3D temp_pt;

    PointList3D operator=(PointList3D P);

	PointList3D(vnl_vector<float> VX, vnl_vector<float> VY, vnl_vector<float>);

	//void SetN(int N);

	void AddPt(Point3D P);
	void AddPt(float x, float y, float z, int tag = 0, int index = 0);
	void AddPtList(PointList3D pl);

	void AddTailPt(Point3D P);
	void AddTailPt(float x, float y, float z);
	void AddTailPtList(PointList3D pl);

	void MergePtList(PointList3D P);
	void RemovePt(void);
	void RemovePt(int index);
	void RemoveFirstPt();
	void RemoveAllPts();
	void Resize(int in);
	void SetPt(int i, float x, float y, float z);
	void SetPt(int i, Point3D P);
	void Flip();
    std::vector<float> Flip_4D(std::vector<float> Ru);

	/// initialize the queue
	void BuildNeighbourList(); 

	/// remove the point from queue
	void RemovePtTag( int tag); 

	/// get first point in each queue and get a new pointList
	void GetFirstPointOfEachQueue( PointList3D &pointList);

	float GetLength(void);
	float GetPartLength(int id, int direct);
	float GetPieceLength(void);
	vnl_vector<float> GetCumLength(void);
	int	GetSize(void);
	Point3D GetPt(int i);
	Point3D GetFirstPt(void);
	Point3D GetMiddlePt(void);
	Point3D GetLastPt(void);
	float GetDistTo(PointList3D P);
	void  Print_PointList(char *filename);
    void  Print();

	bool check_for_sharp_turn(float angle_th);

	void curveinterp_3D(float unit_dist);
	vnl_vector<float> curveinterp_4D(float unit_dist, vnl_vector<float> Ru);
	std::vector<float> curveinterp_4D(float unit_dist, std::vector<float> Ru);
	void curveinterp_3D(int new_np);
	vnl_vector<float> Interp1(vnl_vector<float> X, vnl_vector<float> Y, int Ns);
	float CubicNPSpline(float t, int n);
	void Cast_Open_Snake(Point3D sp);

	std::vector<Point3D> Pt;
	int	NP;
};

bool IsEven(int n);
bool IsOdd(int n);

#endif
