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
#include "PointOperation.h"
#include <map>

Point3D::Point3D(void)
{
	x = 0;
	y = 0;
	z = 0;
}

Point3D::Point3D(float X, float Y, float Z, int TAG, int INDEX)
{
	x = X;
	y = Y;
	z = Z;
	tag = TAG;
	index = INDEX;
}

Point3D Point3D::operator = (Vector3D V)
{
	x = V.x;
	y = V.y;
	z = V.z;
	return *this;
}		
Point3D	Point3D::operator + (Vector3D V)
{
	Point3D P;
	P.x = x + V.x;
	P.y = y + V.y;
	P.z = z + V.z;
	return P;
}

Point3D	Point3D::operator - (Vector3D V)
{
	Point3D P;
	P.x = x - V.x;
	P.y = y - V.y;
	P.z = z - V.z;
	return P;
}

Point3D Point3D::operator = (Point3D P)
{
	x = P.x;
	y = P.y;
	z = P.z;
	tag = P.tag;
	index = P.index;
	return *this;
}

Point3D Point3D::operator * (float Scalar)
{   
	Point3D P;
	P.x = x * Scalar;
	P.y = y * Scalar;
	P.z = z * Scalar;
	return P;
}

Point3D	Point3D::operator / (float Scalar)
{   
	Point3D P;
	P.x = x / Scalar;
	P.y = y / Scalar;
	P.z = z / Scalar;
	return P;
}

Point3D	Point3D::operator + (Point3D P2)
{
	Point3D P;
	P.x = x + P2.x;
	P.y = y + P2.y;
	P.z = z + P2.z;
	return P;
}

Point3D	Point3D::operator - (Point3D P2)
{
	Point3D P;
	P.x = x - P2.x;
	P.y = y - P2.y;
	P.z = z - P2.z;
	return P;
}

int Point3D::Inpolygon2D(PointList3D Contour)
{
	int		i, j, c = 0;

	for (i=0, j=Contour.GetSize()-1;  i<Contour.GetSize(); j=i++){
	 if ( ((Contour.GetPt(i).y>y) != (Contour.GetPt(j).y>y)) &&
	 (x < (Contour.GetPt(j).x-Contour.GetPt(i).x) * (y-Contour.GetPt(i).y) / (Contour.GetPt(j).y-Contour.GetPt(i).y) + Contour.GetPt(i).x) )
       c = !c;
	}

	return c;
}

bool Point3D::check_out_of_range_3D(int IM, int IN, int IZ)
{
   bool out_of_range = false;
   if( ceil(x) <= 0 )
   {
       x = 0;
	   out_of_range = true;
   }
   if( ceil(y) <= 0 )
   {
       y = 0;
	   out_of_range = true;
   }
   if( ceil(z) <= 0 )
   {
       z = 0;
	   out_of_range = true;
   }

   if( ceil(x) >= IM )
   {
       x = float(IM-1);
	   out_of_range = true;
   }
   if( ceil(y) >= IN )
   {
       y = float(IN-1);
	   out_of_range = true;
   }
   if( ceil(z) >= IZ )
   {
       z = float(IZ-1);
	   out_of_range = true;
   }

   return out_of_range;
}

bool Point3D::check_out_of_range_2D(int IM, int IN, int IZ)
{
   bool out_of_range = false;
   if( ceil(x) <= 0 )
   {
       x = 0;
	   out_of_range = true;
   }
   if( ceil(y) <= 0 )
   {
       y = 0;
	   out_of_range = true;
   }

   if( ceil(x) >= IM )
   {
       x = float(IM-1);
	   out_of_range = true;
   }
   if( ceil(y) >= IN )
   {
       y = float(IN-1);
	   out_of_range = true;
   }

   return out_of_range;
}

float		Point3D::GetDistTo2D(Point3D P)
{
	return sqrt(pow(P.x - x,2) + pow(P.y - y,2));
}


float		Point3D::GetDistTo(Point3D P)
{
	return sqrt(pow(P.x - x,2) + pow(P.y - y,2) + pow(P.z - z,2));
}

float		Point3D::GetDistTo(Point3D P, int Z_Weight)
{
	return sqrt(pow(P.x - x,2) + pow(P.y - y,2) + Z_Weight * pow(P.z - z,2));
}

void		Point3D::Print(void)
{
	std::cout<<"("<<x<<","<<y<<","<<z<<")"<<std::endl;
}

Vector3D::Vector3D(void)
{
	x = 0;
	y = 0;
	z = 0;
}

Vector3D::Vector3D(float X, float Y, float Z)
{
	x = X;
	y = Y;
	z = Z;
}

Vector3D::Vector3D(Point3D P1,Point3D P2)
{
	SetEndPoints(P1,P2);
}
Vector3D	Vector3D::operator = (Vector3D V)
{
	x = V.x;
	y = V.y;
	z = V.z;
	return *this;
}

Vector3D	Vector3D::operator + (Vector3D V)
{
	Vector3D V1;
	V1.x = x + V.x;
	V1.y = y + V.y;
	V1.z = z + V.z;
	return V1;
}

Vector3D	Vector3D::operator - (Vector3D V)
{
	Vector3D V1;
	V1.x = x - V.x;
	V1.y = y - V.y;
	V1.z = z - V.z;
	return V1;
}

Vector3D	Vector3D::operator + (Point3D P)
{
	Vector3D V1;
	V1.x = x + P.x;
	V1.y = y + P.y;
	V1.z = z + P.z;
	return V1;
}

Vector3D	Vector3D::operator - (Point3D P)
{
	Vector3D V1;
	V1.x = x - P.x;
	V1.y = y - P.y;
	V1.z = z - P.y;
	return V1;
}

Vector3D	Vector3D::operator * (float Scaler)
{
	Vector3D V1;
	V1.x = x*Scaler;
	V1.y = y*Scaler;
	V1.z = z*Scaler;
	return V1;
}

Vector3D	Vector3D::operator / (float Scaler)
{
	Vector3D V1;
	V1.x = x/Scaler;
	V1.y = y/Scaler;
	V1.z = z/Scaler;
	return V1;
}

void	Vector3D::SetEndPoints(Point3D P1,Point3D P2)
{
	x = P2.x - P1.x;
	y = P2.y - P1.y;
	z = P2.z - P1.z;
}

void	   Vector3D::SetAngle(float theta)
{
}

void	   Vector3D::SetMagnitude(float Mag)
{
}

float	   Vector3D::GetMagnitude(void)
{
	return sqrt(pow(x,2)+pow(y,2)+pow(z,2));
}

float      Vector3D::GetAngle1()
{
	float theta = atan(z/sqrt(pow(x,2)+pow(y,2)));
    return theta;
}

float      Vector3D::GetAngle2()
{
    float phi = atan(y/x);
	return phi;
}


float	   Vector3D::GetAngleTo(Vector3D V)
{
	float theta = acos(GetUnit().x*V.GetUnit().x + GetUnit().y*V.GetUnit().y + GetUnit().z*V.GetUnit().z);

	//if(GetXProduct(V)<0)
	//{
	//	theta = - theta;
	//}
	//
	return theta;
}

float Vector3D::GetDProduct(Vector3D V)
{
	return x*V.x + y*V.y + z*V.z;
}



Vector3D Vector3D::GetUnit(void)
{
	Vector3D U;

	float Mag = GetMagnitude();
   if( Mag != 0 )
   {
	U.x = x/( Mag + std::numeric_limits<double>::epsilon() );
	U.y = y/( Mag + std::numeric_limits<double>::epsilon() );
	U.z = z/( Mag + std::numeric_limits<double>::epsilon() );
   }
   else
   {
    U.x = 0;
	U.y = 0;
	U.z = 0;
   }

	return U;
}

void Vector3D::ConvertUnit(void)
{

	float Mag = GetMagnitude();
   if( Mag != 0 )
   {
	x = x/( Mag + std::numeric_limits<float>::epsilon() );
	y = y/( Mag + std::numeric_limits<float>::epsilon() );
	z = z/( Mag + std::numeric_limits<float>::epsilon() );
   }
   else
   {
    x = 0;
	y = 0;
	z = 0;
   }
}

PointList3D::PointList3D(void)
{ 
	//Pt = new Point3D[1024];
	Pt.clear();
	NP = 0;
}

/*PointList3D::PointList3D(int N)
{ 
	Pt = new Point3D[N];
	NP = 0;
}*/

/*void PointList3D::SetN(int N)
{
	Pt = new Point3D[N];
	NP = 0;
}*/

PointList3D::PointList3D(vnl_vector<float> VX, vnl_vector<float> VY, vnl_vector<float> VZ)
{
   //Pt = new Point3D[1024];
   NP = 0;
   for(unsigned int i = 0; i<VX.size(); i++)
   {
     AddPt(VX(i), VY(i), VZ(i));
   }
}

PointList3D PointList3D::operator=(PointList3D PInput)
{  
   Pt.clear();
   NP = 0;
   for(int i = 0; i < PInput.GetSize(); i++)
   { 
	   AddPt(PInput.Pt[i]);
   }
   return *this;
}

void	PointList3D::AddPt(Point3D P)
{
	//Pt[NP].x = P.x;
	//Pt[NP].y = P.y;
	//Pt[NP].z = P.z;
	Pt.push_back(P);
	NP++;
}


void PointList3D::AddPt(float x,float y,float z, int tag, int index)
{
	temp_pt.x = x;
	temp_pt.y = y;
	temp_pt.z = z;
	temp_pt.tag = tag;
	temp_pt.index = index;
	Pt.push_back(temp_pt);
	NP++;
}

void    PointList3D::AddPtList(PointList3D pl)
{
    for( int i = 0; i < pl.GetSize(); i++ )
	{
		AddPt(pl.Pt[i]);
	}
}

void	PointList3D::AddTailPtList(PointList3D pl)
{ 
	for(int i = 0; i < pl.GetSize(); i++)
	{
		AddTailPt(pl.Pt[i]);
	}
    /*for( int i = 0; i < NP; i++ )
	{
		pl.AddPt(Pt[i]);
	}
	Pt = pl.Pt;
	NP = pl.NP;*/
}

void	PointList3D::AddTailPt(Point3D P)
{
   Pt.insert(Pt.begin(), P);
   NP++;
   /*for(int i = NP-1; i>=0; i--)
   {
     Pt[i+1].x = Pt[i].x;
     Pt[i+1].y = Pt[i].y;
	 Pt[i+1].z = Pt[i].z;
   }
	Pt[0].x = P.x;
	Pt[0].y = P.y;
	Pt[0].z = P.z;
	NP++;*/
}

void	PointList3D::AddTailPt(float x,float y,float z)
{
	temp_pt.x = x;
	temp_pt.y = y;
	temp_pt.z = z;
	Pt.insert(Pt.begin(),temp_pt);
	NP++;
   /*for(int i = NP-1; i>=0; i--)
   {
     Pt[i+1].x = Pt[i].x;
     Pt[i+1].y = Pt[i].y;
	 Pt[i+1].z = Pt[i].z;
   }
	Pt[0].x = x;
	Pt[0].y = y;
	Pt[0].z = z;
	NP++;*/
}

void PointList3D::MergePtList(PointList3D P)
{
    for(int i=0; i<P.GetSize(); i++)
	{
	  AddPt(P.GetPt(i));
	}
}

void	PointList3D::SetPt(int i, float x, float y, float z)
{
	Pt[i].x = x;
	Pt[i].y = y;
	Pt[i].z = z;
}

void	PointList3D::SetPt(int i,Point3D P)
{
	Pt[i] = P;
}

int		PointList3D::GetSize()
{
	return NP;
}

Point3D PointList3D::GetPt(int i)
{
	return Pt[i];
}


float	PointList3D::GetLength(void)
{
	int		m = NP;
	int		i;
	float	Length = 0;
	for(i = 1 ; i<m; i++)
	{
		Length = Length + Pt[i].GetDistTo(Pt[i-1]);
	}
	return Length;
}

float	PointList3D::GetPartLength(int id, int direct)
{
	int		m = NP;
	int		i;
	float	Length = 0;

  if( direct == 0 )
  {
    for(i = 1; i<=id; i++)
	{
		Length = Length + Pt[i].GetDistTo(Pt[i-1]);
	}
  }
  else
  {
    for(i = id + 1; i<m; i++)
	{
		Length = Length + Pt[i].GetDistTo(Pt[i-1]);
	}
  }
	return Length;
}

vnl_vector<float> PointList3D::GetCumLength(void)
{  
	int		m = NP;
	vnl_vector<float> CumDist(m);
	CumDist.fill(0);
	//CumDist(0) = 0;
	//int		i;
	//float	Length = 0;
	for(int i = 1; i<m; i++)
	{
		CumDist(i) = CumDist(i-1) + Pt[i].GetDistTo(Pt[i-1]);
		//Length = Length + GetPt(i).GetDistTo(GetPt(i-1));
	}
	return CumDist;
}


float	PointList3D::GetPieceLength()
{
    return GetLength()/(NP - 1);
}

void	PointList3D::RemoveFirstPt()
{
	Pt.erase(Pt.begin());
	NP--;
}


void	PointList3D::RemovePt(void)
{
	Pt.pop_back();
	NP--;
}

void PointList3D::RemovePt(int index)
{

	Pt.erase( Pt.begin() + index );
	NP = Pt.size();
   /*if( index == 0 )
   {
     for( int i = 0; i < NP; i++ )
	 {
	  Pt[i] = Pt[i+1];
	 }
	  NP--;
   }
   else if( index == NP-1 )
   {
      NP--;
   }
   else
   {
     for( int i = index; i < NP; i++ )
	 {
	  Pt[i] = Pt[i+1];
	 }
	  NP--;
   }*/
 
}

void PointList3D::RemoveAllPts(void)
{
	Pt.clear();
	NP = 0;
}

void PointList3D::Resize(int in)
{
    Pt.resize(in);
	NP = Pt.size();
}

Point3D	PointList3D::GetFirstPt(void)
{
	return Pt[0];
}
Point3D	PointList3D::GetMiddlePt(void)
{
	return Pt[NP/2];
}
Point3D	PointList3D::GetLastPt(void)
{
	return Pt[NP-1];
}

float   PointList3D::GetDistTo(PointList3D P)
{
	float sum = 0;
	for(int i=0; i<GetSize(); i++)
	{
	  sum += pow(Pt[i].x - P.Pt[i].x, 2) + pow(Pt[i].y - P.Pt[i].y, 2) + pow(Pt[i].z - P.Pt[i].z, 2);
	}
	return sqrt(sum);
}

bool PointList3D::check_for_sharp_turn(float angle_th)
{
  int point_dist = 2;
  bool sharp_turn = false;
  float angle = 0;
  Vector3D V1,V2;

  for( int i = point_dist; i < NP-point_dist; i++ )
  {
    V1.x = Pt[i+point_dist].x - Pt[i].x;
    V1.y = Pt[i+point_dist].y - Pt[i].y;
    V1.z = Pt[i+point_dist].z - Pt[i].z;
    V2.x = Pt[i].x - Pt[i-point_dist].x;
    V2.y = Pt[i].y - Pt[i-point_dist].y;
    V2.z = Pt[i].z - Pt[i-point_dist].z;

	 float angle_temp = V1.GetAngleTo(V2) * 360/(2*3.1415926);
	 if( angle_temp > angle)
		angle = angle_temp;
  }
  if( angle > angle_th )
	  sharp_turn = true;

  return sharp_turn;
} 

void PointList3D::Flip(void)
{
  vnl_vector<float> tempx(NP);
  vnl_vector<float> tempy(NP);
  vnl_vector<float> tempz(NP);
  for(int i=0; i<NP; i++)
  {
    tempx(i) = GetPt(i).x;
	tempy(i) = GetPt(i).y;
	tempz(i) = GetPt(i).z;
  }
  tempx.flip();
  tempy.flip();
  tempz.flip();
  for(int i=0; i<NP; i++)
  {
     SetPt(i, tempx(i),tempy(i),tempz(i));
  }
}

std::vector<float> PointList3D::Flip_4D(std::vector<float> Ru)
{ 
  std::vector<float> new_Ru;

  vnl_vector<float> tempx(NP);
  vnl_vector<float> tempy(NP);
  vnl_vector<float> tempz(NP);
  for(int i=0; i<NP; i++)
  {
    tempx(i) = GetPt(i).x;
	tempy(i) = GetPt(i).y;
	tempz(i) = GetPt(i).z;

	new_Ru.push_back(Ru[NP-1-i]);
  }
  tempx.flip();
  tempy.flip();
  tempz.flip();
  for(int i=0; i<NP; i++)
  {
     SetPt(i, tempx(i),tempy(i),tempz(i));
  }

  return new_Ru;
}

void PointList3D::curveinterp_3D(int N)
{
 //if( N > 1000 )
 //{
 //   N = 1000;
 // }

  if( N > 3 )
  {
    vnl_vector<float> d;
    d = GetCumLength();
    vnl_vector<float> Xi;
    vnl_vector<float> Yi;
    vnl_vector<float> Zi;
	vnl_vector<float> X(NP);
	vnl_vector<float> Y(NP);
	vnl_vector<float> Z(NP);
    
	for(int i=0; i<NP; i++)
	{
	  X(i) = Pt[i].x;
	  Y(i) = Pt[i].y;
	  Z(i) = Pt[i].z;
	}

    Xi = Interp1(d, X, N);
    Yi = Interp1(d, Y, N);  
	Zi = Interp1(d, Z, N);

	RemoveAllPts();

	for(int k =0; k<N; k++)
    {
     AddPt(Xi(k),Yi(k),Zi(k));
    }
  }
}


vnl_vector<float> PointList3D::curveinterp_4D(float unit_dist, vnl_vector<float> Ru)
{
  int N = ceil(GetLength()/unit_dist);
  vnl_vector<float> Ri;
  Ri = Ru;

  if( N > 1000 )
  {
    N = 1000;
  }

  if( N > 3 )
  {
    vnl_vector<float> d;
    d = GetCumLength();
    vnl_vector<float> Xi;
    vnl_vector<float> Yi;
    vnl_vector<float> Zi;
	vnl_vector<float> X(NP);
	vnl_vector<float> Y(NP);
	vnl_vector<float> Z(NP);
    
	for(int i=0; i<NP; i++)
	{
	  X(i) = Pt[i].x;
	  Y(i) = Pt[i].y;
	  Z(i) = Pt[i].z;
	}

    Xi = Interp1(d, X, N);
    Yi = Interp1(d, Y, N);  
	Zi = Interp1(d, Z, N);
	Ri = Interp1(d, Ru, N);

	RemoveAllPts();

	for(int k =0; k<N; k++)
    {
     AddPt(Xi(k),Yi(k),Zi(k));
    }
  }

  return Ri;
}

std::vector<float> PointList3D::curveinterp_4D(float unit_dist, std::vector<float> Ru)
{
  int N = ceil(GetLength()/unit_dist);
  vnl_vector<float> Ru1(Ru.size());
  vnl_vector<float> Ru2;
  for( int i = 0; i < Ru.size(); i++ )
	  Ru1(i) = Ru[i];

  Ru2 = Ru1;

  if( N > 1000 )
  {
    N = 1000;
  }

  if( N > 3 )
  {
    vnl_vector<float> d;
    d = GetCumLength();
    vnl_vector<float> Xi;
    vnl_vector<float> Yi;
    vnl_vector<float> Zi;
	vnl_vector<float> X(NP);
	vnl_vector<float> Y(NP);
	vnl_vector<float> Z(NP);
    
	for(int i=0; i<NP; i++)
	{
	  X(i) = Pt[i].x;
	  Y(i) = Pt[i].y;
	  Z(i) = Pt[i].z;
	}

    Xi = Interp1(d, X, N);
    Yi = Interp1(d, Y, N);  
	Zi = Interp1(d, Z, N);
	Ru2 = Interp1(d, Ru1, N);

	RemoveAllPts();

	for(int k =0; k<N; k++)
    {
     AddPt(Xi(k),Yi(k),Zi(k));
    }
  }

  Ru.clear();
  for( int i = 0; i < Ru2.size(); i++ )
	  Ru.push_back(Ru2(i));

  return Ru;
}



void PointList3D::curveinterp_3D(float unit_dist)
{
  int N = ceil(GetLength()/unit_dist);

  if( N > 1000 )
  {
    N = 1000;
  }

  if( N > 3 )
  {
    vnl_vector<float> d;
    d = GetCumLength();
    vnl_vector<float> Xi;
    vnl_vector<float> Yi;
    vnl_vector<float> Zi;
	vnl_vector<float> X(NP);
	vnl_vector<float> Y(NP);
	vnl_vector<float> Z(NP);
    
	for(int i=0; i<NP; i++)
	{
	  X(i) = Pt[i].x;
	  Y(i) = Pt[i].y;
	  Z(i) = Pt[i].z;
	}

    Xi = Interp1(d, X, N);
    Yi = Interp1(d, Y, N);  
	Zi = Interp1(d, Z, N);

	RemoveAllPts();

	for(int k =0; k<N; k++)
    {
     AddPt(Xi(k),Yi(k),Zi(k));
    }
  }
}


vnl_vector<float> PointList3D::Interp1(vnl_vector<float> X, vnl_vector<float> Y, int Ns)
{
  int N = X.size();

  vnl_vector<float> Xi(Ns);
  vnl_vector<float> Yi(Ns);
  for( int i = 0; i < Ns; i++)
  {
	  Xi(i) = X.max_value() * float(i)/float(Ns-1);
  }
  Yi.fill(0);
  Yi(0) = Y(0);
  Yi(Ns-1) = Y(N-1);

  int k = 1;

  for( int i = 0; i < N - 1; i++ )
  {
     for( int j = k; j < Ns - 1; j++ )
	 {  
	    if( Xi(j) == X(i) )
		{
		  Yi(j) = Y(i);
		}

	    if( Xi(j) > X(i) && Xi(j) < X(i+1))
		{
		  k = j + 1;
		  Yi(j) = Y(i) + (Y(i+1) - Y(i))/(X(i+1) - X(i)) * ( Xi(j) - X(i));
		}
		
	 }
  }

  return Yi;
}


float PointList3D::CubicNPSpline(float t, int n)
{
  float value;
  float s = t - n;
  if (s >= -2 && s < -1)
    value = (pow((2+s),3))/6;
  else if (s >= -1 && s < 0)
    value = (4 - 6*pow(s,2) - 3*pow(s,3))/6;
  else  if (s >= 0 && s < 1)
    value = (4 - 6*pow(s,2) + 3*pow(s,3))/6;
  else  if (s >= 1 && s <= 2)  
    value = (pow((2-s),3))/6;
  else
    value = 0;
  return value;
}

void PointList3D::Print()
{
 for(int i = 0; i< GetSize(); i++)
 {
	 Pt[i].Print();
 }
}

void PointList3D::Print_PointList(char *filename)
{
std::ofstream myfile (filename);
if (myfile.is_open())
{
for(int i = 0; i< GetSize(); i++)
{
  myfile << Pt[i].x;
  myfile <<"\t";
  myfile << Pt[i].y;
  myfile <<"\t";
  myfile << Pt[i].z;
  myfile <<"\n";
}
myfile.close();
}
}

// For point queue  tag : -1 removed, 0 initialized, >0 queue id;
void PointList3D::BuildNeighbourList()
{
	int NQ = 0;
	std::vector<int> size;
	vnl_vector<unsigned char> mark(NP);
	std::map<int,int> ptTagMap;
	mark.fill(0);
	for( int i = 0; i < NP; i++)
	{
		if( mark[i] == 0)
		{
			NQ++;
			int tag = Pt[i].tag;
			int count = 1;
			mark[i] = 1;
			ptTagMap.insert(std::pair<int,int>(tag, NQ));
			for( int j = 0; j < NP; j++)
			{
				if( Pt[j].tag == tag && mark[j] == 0)
				{
					mark[j] = 1;
					count++;
				}
			}
			size.push_back(count);
		}
	}

	std::cout<< "Queue Size: "<< NQ<<std::endl;
	vnl_vector<int> Index( NQ);
	int sum = 0;
	for( int i = 0; i < NQ - 1; i++)
	{
		Index[i] = sum;
		sum += size[i];
	}
	Index[NQ - 1] = sum;

	std::vector<Point3D> TmpPt;
	TmpPt.resize( NP);
	for( int i = 0; i < NP; i++)
	{
		int tmpTag = Pt[i].tag;
		int newTag = ptTagMap[tmpTag];
		std::cout<< newTag<<"\t";
		int ind = Index[ newTag - 1];
		Pt[i].tag = newTag;
		TmpPt[ ind] = Pt[i];
		Index[ newTag - 1] += 1;
	}
	std::cout<<std::endl;

	Pt = TmpPt;

	std::ofstream ofs("SeedPointList.txt");
	for( int i = 0; i < NP; i++)
	{
		ofs<< Pt[i].x<<"\t"<< Pt[i].y<<"\t"<< Pt[i].z<<"\t"<< Pt[i].tag<<"\t"<<Pt[i].index<<std::endl;
	}
	ofs.close();
}

void PointList3D::RemovePtTag( int tag)
{
	bool bRemoved = false;
	for( int i = 0; i < NP; i++)
	{
		Point3D point = Pt[i];
		if( Pt[i].tag == tag)
		{
			Pt[i].tag = -1;
			bRemoved = true;
		}
	}
}

void PointList3D::GetFirstPointOfEachQueue( PointList3D &pointList)
{
	int tmpTag = -1;
	for( int i = 0; i < NP; i++)
	{
		if( Pt[i].tag != -1 && Pt[i].tag != tmpTag)
		{
			pointList.AddPt(Pt[i]);
			tmpTag = Pt[i].tag;
			Pt[i].tag = -1;   // delete the point
		}
	}
}