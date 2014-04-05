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

#include "OpenSnakeTracer.h"

void OpenSnakeTracer::Init()
{
	SnakeList.SetNSpace(IM->SeedPt.GetSize());
	SnakeList.SetImage(IM);
	seed_num = 0;
}

void OpenSnakeTracer::Open_Curve_Snake_Tracing()
{

   for( int i = 0; i < IM->SeedPt.GetSize(); i++)
   {
	   std::cout<<"#Seeds Left:"<< IM->SeedPt.GetSize() - i << std::endl;
	   Cast_Open_Snake_3D();
   }
   
}


void OpenSnakeTracer::Cast_Open_Snake_3D()
{

   float alpha = 0; 
   int iter_num = 50;
   int ITER = 5;

   int pt_distance = 2;

   float beta = 0.05;
   float kappa = 1;
   float gamma = 1;
   float stretchingRatio = 3;

   int collision_dist = 1;
   int minimum_length = 5;
   bool automatic_merging = 0;
   int max_angle = 99;
   bool freeze_body = 0;
   int s_force = 1;
   int snake_id;
   int tracing_model = 0;
   int coding_method = 0;
   float sigma_ratio = 1;
   int border = 0;

   Point3D seed = IM->SeedPt.Pt[seed_num];
   seed_num++;
   std::cout<<"#Seeds Left:"<< IM->SeedPt.GetSize() - seed_num << std::endl;

   SnakeClass snake;
   snake.Set_Seed_Point(seed);
   snake.SetImage(IM);
   snake.Expand_Seed_Point(3);

  int i = 0;
  while( i < iter_num )
  {
	  std::cout<<"iteration#"<<i<<std::endl;
      std::cout<<"zhi#"<<i<<std::endl;
    //  snake.OpenSnakeStretch_4D( alpha, ITER, pt_distance, beta, kappa, gamma, stretchingRatio,collision_dist,minimum_length,
    //                             automatic_merging,max_angle,freeze_body,s_force,snake_id,tracing_model,coding_method,sigma_ratio,border);
     // snake.OpenSnake_Init_4D(alpha,ITER,beta,kappa,gamma,pt_distance);
      i++;
  }
   
   SnakeList.AddSnake(snake);


}

void OpenSnakeTracer::SetImage(ImageOperation *I_Input)
{
   IM = I_Input;
}
   	


TracingThread::TracingThread()
: QThread()
{

}

void TracingThread::setSnake(SnakeClass *s)
{
	snake = s;
}

void TracingThread::setIterNum(int num)
{
	iter_num = num;
}

void TracingThread::run()
{

}
void TracingThread::suspend()
{
   ToSuspend = true;
}

void TracingThread::resume()
{
  if (!ToSuspend)
     return;
   ToSuspend = false;
   condition.wakeOne();
}

void TracingThread::stop()
{
   stopped = true;
}
