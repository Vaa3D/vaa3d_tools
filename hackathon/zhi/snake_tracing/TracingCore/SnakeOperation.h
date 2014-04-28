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
#ifndef SNAKEOPERATION_H
#define SNAKEOPERATION_H

#include "ImageOperation.h"
#include "PointOperation.h"
#include "itkLinearInterpolateImageFunction.h"

bool isinf(float x);
bool isnan(float x);
//float norm_density(float x, float mu, float sigma);

class SnakeListClass;

class SnakeClass 
{
public:

    SnakeClass(void);
	SnakeClass operator=(SnakeClass Snake);

	PointList3D Cu;
	PointList3D Cu_Backup;
	PointList3D BranchPt;
	PointList3D RootPt;
	
	//vnl_vector<float> Ru;
	std::vector<float> Ru;
	std::vector<float> Ru1;

	//std::vector<float> Type;

	//PointList3D Probe;
	Point3D head_pt;
	Point3D tail_pt;

	int collision;
	int tail_collision_snake_id;
	int head_collision_snake_id;
    bool hit_boundary;

	ImageOperation *IM;
    
	SnakeListClass *SnakeList;

	void Branch_Adjustment();
	void Nail_Branch();

	void SetTracedSnakes(SnakeListClass *S);
	void SetImage(ImageOperation *I_Input);
	void Set_Seed_Point(PointList3D seeds);
    void Set_Seed_Point(Point3D seed);

	void Expand_Seed_Point(int expand_distance);

	void OpenSnake_Init_4D(float alpha, int ITER, float beta, float kappa, float gamma, int pt_distance);
	void OpenSnakeStretch_4D(float alpha, int ITER, int pt_distance, float beta, float kappa, float gamma, 
                                float stretchingRatio, int collision_dist, int minimum_length, 
								bool automatic_merging, int max_angle, bool freeze_body, int s_force, 
								int snake_id, int tracing_model, int coding_method, float sigma_ratio, int border);
	void OpenSnakeStretch_5D(float alpha, int ITER, int pt_distance, float beta, float kappa, float gamma, 
                                float stretchingRatio, int collision_dist, int minimum_length, 
								bool automatic_merging, int max_angle, bool freeze_body, int s_force, 
								int snake_id, int tracing_model, int coding_method, float sigma_ratio, int border);
	bool Check_Validity(float minimum_length, int snake_id, int automatic_merging);
	bool Check_Head_Collision(ImageType::IndexType in, int collision_dist, int minimum_length, bool automatic_merging, int max_angle, int snake_id);
	bool Check_Tail_Collision(ImageType::IndexType in, int collision_dist, int minimum_length, bool automatic_merging, int max_angle, int snake_id);
	bool Compute_Seed_Force(int head_tail, int distance);

	vnl_matrix<float> makeOpenA(float alpha, float beta, int N);
    vnl_matrix<float> matrix_inverse(vnl_matrix<float> A, int N);
};

class SnakeListClass
{
public:

    SnakeListClass(void);
	SnakeListClass operator= (SnakeListClass SnakeList);
    
    int NSnakes;
	//SnakeClass *Snakes;
	std::vector<SnakeClass> Snakes;
	std::vector<int> valid_list;

	PointList3D branch_points;
    
	ImageOperation *IM;

	void AddSnake(SnakeClass snake);
	void AddSnake_Coding(SnakeClass snake);
	void RemoveSnake(int idx);
	void RemoveAllSnakes();
	void SplitSnake(int idx_snake, int idx_pt);
	void MergeSnake(int idx1, int idx2, bool im_coding);
	void CreateBranch(int idx1, int idx2);
	SnakeClass GetSnake(int idx);
	void SetNSpace(int N);
	void SetImage(ImageOperation *I_Input);

};

struct SnakeTree
{
	Point3D root_point;
    PointList3D branch_point;
	std::vector<int> parent_list;
	PointList3D points;
	std::vector<float> Ru;
	std::vector<int> snake_id;
};

struct SnakeTree_SWC //snake tree class for loaded swc
{
	SnakeListClass Snakes;
	Point3D RootPt;
	float soma_radius;
	PointList3D BranchPt;
};

struct TreeFeature
{
	float SomaVolume;
	int N_Stems;
	float TotalLength;
	int N_Branches;
	int N_Bifs;
	int N_Tips;
	float A_Diameter;
	float A_SectionArea;
	float Volume;
	float Surface;
	float A_BranchOrder;
	float A_BifAngle;
};

#endif
