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
  Program:   Open Snake Tracing System
  Autohr:    Yu Wang
  Email: wangy15@rpi.edu
  Language:  C++
  Date:      $Date:  $
  Version:   $Revision: 0.00 $
=========================================================================*/

#ifndef OPENSNAKETRACER_H
#define OPENSNAKETRACER_H

#include "itkNumericTraits.h"
#include <vnl/vnl_matrix.h>
#include <vnl/vnl_vector.h>
#include <vnl/algo/vnl_matrix_inverse.h>
#include <QTextEdit>
#include "TracingCore/SnakeOperation.h"
#include "qthread.h"
#include "qmutex.h"
#include "qwaitcondition.h"

struct OptionsStruct
{
   float alpha; 
   int stretch_iter;
   int pt_distance;
   float beta;
   float kappa;
   float gamma;
   float stretchingRatio;
   float struggle_dist;
   float struggle_th;
   float minimum_length;
   float iter_num;
   float remove_seed_range;
   float deform_iter;
   int collision_dist;
   bool automatic_merging;
   int max_angle;
   bool freeze_body;
   int s_force;
   int tracing_model;
   bool parallel_tracing;
   int coding_method;
   float sigma_ratio;
   int border;
};

class TracingThread
{

 public:

    TracingThread();
    
	void setParas(SnakeListClass *sl, int idx, ImageOperation *im, OptionsStruct io);
    void setParas(SnakeListClass *sl, PointList3D seeds, ImageOperation *im, OptionsStruct io);

    void start();
	
    volatile bool stopped;
	bool ToSuspend;

	PointList3D seeds;
	int current_seed_index;
	SnakeListClass *Snakes;

	SnakeClass temp_snake;
	bool valid;
	
	ImageOperation *IM;
    OptionsStruct options;

	QWaitCondition condition;
	QMutex mutex;

	bool manual_seed;
    bool parallel_tracing;

 private:

};

class OpenSnakeTracer
{
public:
   
	OpenSnakeTracer();
    SnakeListClass SnakeList;
	
	void Init();
	void setParas(int pt_distance, float gamma, float stretchingRatio, float minimum_length, int collision_dist, 
		          int remove_seed_range, int deform_iter, bool automatic_merging, int max_angle, bool freeze_body, 
				  int s_force, int tracing_model, bool parallel_tracing, 
				  int coding_method, float sigma_ratio, int border);
	void SetImage(ImageOperation *I_Input);
	void Open_Curve_Snake_Tracing();
	void Cast_Open_Snake_3D(PointList3D seeds, bool manual_seed);
	void Cast_Open_Snake_3D(vnl_vector<int> seed_indices);
	void RemoveSeeds_SN();
	void RemoveSeeds();

	void Refine_Branch_Point();
	//Functions for GUI
	QTextEdit *log_viewer;

    OptionsStruct options;

	int Old_NSnakes;
	bool tracing;

	PointList3D current_seeds;

	ImageOperation *IM;

    bool use_multi_threads;

	//thread for controlling snake stretching and dynamic displaying
    TracingThread *tracing_thread;
};


#endif
