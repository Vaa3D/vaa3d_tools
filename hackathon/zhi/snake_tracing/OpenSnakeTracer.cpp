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

OpenSnakeTracer::OpenSnakeTracer()
{
   tracing = false;
   use_multi_threads = false;
   tracing_thread = new TracingThread;
   Old_NSnakes = 0;
}

void OpenSnakeTracer::Init()
{
	SnakeList.RemoveAllSnakes();
	//SnakeList.NSnakes = 0;
	//SnakeList.valid_list.fill(1);
	//SnakeList.SetNSpace(IM->SeedPt.GetSize());

	//remove all the existing branch points
	SnakeList.branch_points.RemoveAllPts();

	SnakeList.SetImage(IM);

	Old_NSnakes = 0;
}

void OpenSnakeTracer::RemoveSeeds_SN() //in case of batch tracing II, remove the seed covered by snakes growing from seed snakes first
{
	//if( SnakeList.NSnakes != 0 && IM->visit_label.sum() == 0 )
	if( SnakeList.NSnakes != 0 )
	{
	 for( unsigned int i = 0; i < IM->visit_label.size(); i++ )
     {
      if( IM->visit_label(i) == 0)
	  {
	    int NS = SnakeList.NSnakes;
	    for( int j = 0; j < NS ; j++ )
		{
			if( SnakeList.valid_list[j] == 0 )
				continue;
		   bool removal = false;
           for( int k = 0; k < SnakeList.Snakes[j].Cu.NP ; k++ )
		   {
			if( IM->SeedPt.Pt[i].GetDistTo( SnakeList.Snakes[j].Cu.Pt[k] ) <= options.remove_seed_range )
			{
			  IM->visit_label(i) = 1;
			  removal = true;
			  break;
			}
		   }

		   if( removal )
             break;
		}
	  }
	 }
	}

	std::cout<<"#Growed Seed Snakes:"<< SnakeList.NSnakes  << std::endl;
	std::cout<<"#Seeds Left:"<< IM->SeedPt.GetSize() - IM->visit_label.sum() << std::endl;
}

void OpenSnakeTracer::RemoveSeeds()
{
    //eliminate seed points covered by traced snakes
   if( SnakeList.NSnakes != 0 && Old_NSnakes != SnakeList.NSnakes )
   {
     int NS = SnakeList.NSnakes;
	for( int i = NS-1; i >= Old_NSnakes; i-- )
	{
     IM->ImRemove_RedSeeds(SnakeList.Snakes[i].Cu, SnakeList.Snakes[i].Ru);
	}

    for( unsigned int i = 0; i < IM->visit_label.size(); i++ )
    {
      if( IM->visit_label(i) == 0 && SnakeList.NSnakes != 0 )
	  {
	    int NS = SnakeList.NSnakes;
	    for( int j = 0; j < SnakeList.Snakes[ NS - 1 ].Cu.GetSize() ; j++ )
		{
			if( IM->SeedPt.Pt[i].GetDistTo( SnakeList.Snakes[ NS - 1 ].Cu.Pt[j] ) <= options.remove_seed_range )
			{
			  IM->visit_label(i) = 1;
			  break;
			}
		}
	  }
	}

    /*for( int i = NS-1; i >= Old_NSnakes; i-- )
	{
	 //add newly found seeds
	 IM->SeedPt.AddPtList(SnakeList.Snakes[i].Detect_Branches());
	 vnl_vector<int> new_visit_label(IM->SeedPt.NP);
	 new_visit_label.fill(0);
	 for( int i = 0; i < IM->visit_label.size(); i++ )
	 {
	   new_visit_label(i) = IM->visit_label(i);
	 }
	 IM->visit_label = new_visit_label;
	}*/
    

	Old_NSnakes = SnakeList.NSnakes;
   }
   
   //std::cout<<"#Seeds Left:"<< IM->SeedPt.GetSize() - IM->visit_label.sum() << std::endl;
  if(IM->SeedPt.GetSize() - IM->visit_label.sum() < 10)
   std::cout << "\r#Seeds Left:"<<"000"<<IM->SeedPt.GetSize() - IM->visit_label.sum();
  else if(IM->SeedPt.GetSize() - IM->visit_label.sum() < 100)
   std::cout << "\r#Seeds Left:"<<"00"<<IM->SeedPt.GetSize() - IM->visit_label.sum();
  else if(IM->SeedPt.GetSize() - IM->visit_label.sum() < 1000)
   std::cout << "\r#Seeds Left:"<<"0"<<IM->SeedPt.GetSize() - IM->visit_label.sum();
  else if(IM->SeedPt.GetSize() - IM->visit_label.sum() < 10000)
   std::cout << "\r#Seeds Left:"<<IM->SeedPt.GetSize() - IM->visit_label.sum();
}

void OpenSnakeTracer::Open_Curve_Snake_Tracing()
{
  if( !options.parallel_tracing )
  {
   int visit_label_sum = IM->visit_label.sum();

   current_seeds.RemoveAllPts();

   vnl_vector<int> seed_indices(6);
   seed_indices.fill(-1);
   //find the indices of seeds that are going to be fed to tracing threads

    for( unsigned int i = 0; i < IM->visit_label.size(); i++ )
    {
      if( IM->visit_label(i) == 0 )
	  {
		  seed_indices(0) = i;
		  //current_seeds.AddPt(IM->SeedPt.Pt[i]);
		  //IM->current_seed_idx = i;
		  //IM->visit_label(i) = 1;
		  break;
	  }
    }

   if( visit_label_sum != IM->SeedPt.GetSize() )
   {
      //Cast_Open_Snake_3D( current_seeds, false );
	   //Cast_Open_Snake_3D(IM->current_seed_idx);
	   Cast_Open_Snake_3D( seed_indices );
   }
  }
  else
  {
  
    tracing_thread->setParas( &SnakeList, IM->SeedPt, IM, options);

    tracing = true;

    tracing_thread->start();
	//std::cout<<"check point 1"<<std::endl;
  }

}
void OpenSnakeTracer::Cast_Open_Snake_3D( vnl_vector<int> seed_indices )
{

   tracing_thread->manual_seed = false;
   tracing_thread->setParas( &SnakeList, seed_indices(0), IM, options);

   //in case of going back to preprocessing step, stopped is ture and hence it needs to be reset
   if( SnakeList.NSnakes == 0 )
   {
	   tracing_thread->stopped = false;
   }
   tracing = true;
   tracing_thread->start();

}

void OpenSnakeTracer::Cast_Open_Snake_3D( PointList3D seeds, bool manual_seed )
{

   //int iter_num = 50;
  
 if( manual_seed )
 {
   //std::cout<<"manual seed"<<std::endl;
   int SM = IM->I->GetLargestPossibleRegion().GetSize()[0];
   int SN = IM->I->GetLargestPossibleRegion().GetSize()[1];
   int SZ = IM->I->GetLargestPossibleRegion().GetSize()[2];

   PointList3D temp_seeds;

  for( int j = 0; j < seeds.NP; j++ )
  {
    
   if( seeds.Pt[j].z != 0 )
   {
	   temp_seeds.AddPt(seeds.Pt[j]);
	   continue;
   }

   ProbImageType::IndexType index; 
   ImageType::IndexType index1;
   index[0] = seeds.Pt[j].x;
   index1[0] = seeds.Pt[j].x;
   index[1] = seeds.Pt[j].y;
   index1[1] = seeds.Pt[j].y;
   vnl_vector<float> saliency(SZ);
   saliency.fill(0);

   bool skeleton_point = false;

     for( int i = 0; i < SZ; i++ )
	 {
       
	   index[2] = i;
	   index1[2] = i;

	   /*if( IM->SBW->GetPixel( index1 ) == 1 )
	   {
		  seeds.Pt[j].z = i;
		  seeds.Pt[j].check_out_of_range_3D(SM,SN,SZ);
		  temp_seeds.AddPt(seeds.Pt[j]);
		  skeleton_point = true;
		  break;
	   }*/
	  
	  //if( IM->VBW->GetPixel( index1 ) == 1 )  
		  saliency(i) = IM->I->GetPixel( index );

	 }
   
     if( !skeleton_point )
	 {
	  int idx = saliency.arg_max();
	  
	  //if( saliency.max_value() == 0 && j > 0 )
	  //{
	  //	seeds.Pt[j].z = seeds.Pt[j-1].z;
      //  seeds.Pt[j].check_out_of_range_3D(SM,SN,SZ);
	  //break;
	  //}
	  seeds.Pt[j].z = idx;
	  seeds.Pt[j].check_out_of_range_3D(SM,SN,SZ);

      //if( saliency.arg_max() != 0 )
		  temp_seeds.AddPt(seeds.Pt[j]);
	 }
  }

    if( temp_seeds.NP == 2 )
	{
		seeds.RemoveAllPts();
		seeds.AddPt(temp_seeds.Pt[0]);
	}
	else
	{
        seeds = temp_seeds;
	}

	 tracing_thread->manual_seed = true;
 }
 else
 {
     tracing_thread->manual_seed = false;
 }
   
   tracing_thread->setParas( &SnakeList, seeds, IM, options);

   //in case of going back to preprocessing step, stopped is ture and hence it needs to be reset
   if( SnakeList.NSnakes == 0 )
   {
	   tracing_thread->stopped = false;
   }

   tracing = true;

   std::cout<<"Start the tracing......"<<std::endl;
   tracing_thread->start();


}

void OpenSnakeTracer::SetImage(ImageOperation *I_Input)
{
   IM = I_Input;
   //visit_label.set_size( IM->SeedPt.GetSize() );
   //visit_label.fill(0);
}

void OpenSnakeTracer::setParas(int pt_distance, float gamma, float stretchingRatio, float minimum_length, 
							   int collision_dist, int remove_seed_range, int deform_iter, bool automatic_merging, 
							   int max_angle, bool freeze_body, int s_force, int tracing_model, bool parallel_tracing, int coding_method, float sigma_ratio, int border)
{
   options.alpha = 0.05;
   options.stretch_iter = 5;
   options.pt_distance = pt_distance;
   options.beta = 0.05;
   options.kappa = 1;
   options.gamma = gamma;
   options.stretchingRatio = stretchingRatio;
   //options.struggle_dist = 2;
   options.struggle_dist = 0.01;
   options.struggle_th = 3;
   options.minimum_length = minimum_length;
   options.iter_num = 100;
   options.remove_seed_range = remove_seed_range;
   options.deform_iter = deform_iter;
   options.collision_dist = collision_dist;
   options.automatic_merging = automatic_merging;
   options.max_angle = max_angle;
   options.freeze_body = freeze_body;
   options.s_force = s_force;
   options.tracing_model = tracing_model;
   options.parallel_tracing = parallel_tracing;
   options.coding_method = coding_method + 1;
   options.sigma_ratio = sigma_ratio;
   options.border = border;
}
   
void OpenSnakeTracer::Refine_Branch_Point()
{
	for( int i = 0; i < SnakeList.NSnakes; i++ )
	{
		if( SnakeList.valid_list[i] == 1 )
	      SnakeList.Snakes[i].Branch_Adjustment();
	}

}

TracingThread::TracingThread()
{
   stopped = false;
   ToSuspend = false;
}

void TracingThread::setParas(SnakeListClass *sl, int idx, ImageOperation *im, OptionsStruct io)
{
	Snakes = sl;
	//snake = s;
	//seeds = s;
	current_seed_index = idx;
	IM = im;
	options = io;
}
void TracingThread::setParas(SnakeListClass *sl, PointList3D s, ImageOperation *im, OptionsStruct io)
{
	Snakes = sl;
	//snake = s;
    seeds = s;
	//current_seed_index = idx;
	IM = im;
	options = io;
}

void TracingThread::start()
{

   if( !manual_seed ) 
   {
	   seeds.RemoveAllPts();
	   seeds.AddPt(IM->SeedPt.Pt[current_seed_index]);
   }

   if( stopped )
	   return;

   if( seeds.NP == 0 )
   {
	   return;
   }

if( !options.parallel_tracing )
{
   //SnakeClass *snake;
   //snake = new SnakeClass;
	temp_snake.Set_Seed_Point(seeds);
    temp_snake.BranchPt.RemoveAllPts();
    temp_snake.SetImage(IM);
    temp_snake.SetTracedSnakes(Snakes);
	temp_snake.collision = 0;

   //seed expansion for initializing the tracing
 if( temp_snake.Cu.GetSize() < 3 )
 {
     temp_snake.Expand_Seed_Point(3);
 }
 else
 {
	temp_snake.Cu.curveinterp_3D(options.pt_distance);
	//initialize the radius
    temp_snake.Ru.clear();
    for( int j = 0; j < temp_snake.Cu.GetSize(); j++ )
    {
		temp_snake.Cu.Pt[j].Print();
        temp_snake.Ru.push_back(1);
    }
 }

  int i = 0;
  int struggle_label = 0;
  bool invalid = false;

  ImageType::IndexType idx;
  idx[0] = temp_snake.Cu.GetMiddlePt().x;
  idx[1] = temp_snake.Cu.GetMiddlePt().y;
  idx[2] = temp_snake.Cu.GetMiddlePt().z;

  //initialize the radius
  temp_snake.Ru.clear();
  temp_snake.Ru1.clear();

  for( int j = 0; j < temp_snake.Cu.GetSize(); j++ )
  {
    temp_snake.Ru.push_back(2);
	//temp_snake.Ru1.push_back(2);
  }

  if( options.tracing_model == 3 )
  {
   for( int j = 0; j < temp_snake.Cu.GetSize(); j++ )
   {
	temp_snake.Ru1.push_back(2);
   } 
  }
   
  while( i < options.iter_num && struggle_label <= options.struggle_th && !temp_snake.hit_boundary )
  {
	  float old_dist = temp_snake.Cu.GetLength();

	  if( options.tracing_model == 3 )
	   temp_snake.OpenSnakeStretch_5D( options.alpha, options.stretch_iter, options.pt_distance, options.beta, options.kappa, options.gamma,
	  	  options.stretchingRatio, options.collision_dist, options.minimum_length, options.automatic_merging, options.max_angle, options.freeze_body, 
		  options.s_force, 0, options.tracing_model, options.coding_method, options.sigma_ratio, options.border);
	  else
	   temp_snake.OpenSnakeStretch_4D( options.alpha, options.stretch_iter, options.pt_distance, options.beta, options.kappa, options.gamma,
	  	  options.stretchingRatio, options.collision_dist, options.minimum_length, options.automatic_merging, options.max_angle, options.freeze_body, 
		  options.s_force, 0, options.tracing_model, options.coding_method, options.sigma_ratio, options.border);

	  if( temp_snake.Cu.NP < 3 )
	  {
	     invalid = true;
		 break;
	  }

	  float new_dist = temp_snake.Cu.GetLength();

	  if( new_dist > old_dist * ( 1 - options.struggle_dist ) && new_dist < old_dist * ( 1 + options.struggle_dist ) )
	     struggle_label++;
	  else
	     struggle_label = 0;

	  if( temp_snake.Cu.GetLength() > options.minimum_length )
	  {
		 IM->ImCoding( temp_snake.Cu, temp_snake.Ru, 1, true );
	  }

	  i++;

  }


  if( !invalid )
  {
    if( manual_seed )
	{
	  if( temp_snake.Cu.GetLength() > options.minimum_length )
	  {
		if( temp_snake.BranchPt.NP != 0 )
			Snakes->branch_points.AddPtList(temp_snake.BranchPt);

	   temp_snake.Nail_Branch();

	   Snakes->AddSnake_Coding(temp_snake);

	   IM->ImRefresh_TracingImage();

   	   IM->ImComputeForegroundModel(temp_snake.Cu, temp_snake.Ru);
	  }
	  else
	  {
	     invalid = true;
	  }
	}
	else
	{
	  if( temp_snake.Check_Validity( options.minimum_length, 0, options.automatic_merging ) )
	  {
		if( temp_snake.BranchPt.NP != 0 )
			Snakes->branch_points.AddPtList(temp_snake.BranchPt);

	   temp_snake.Nail_Branch();

	   Snakes->AddSnake_Coding(temp_snake);

	   IM->ImRefresh_TracingImage();

   	   IM->ImComputeForegroundModel(temp_snake.Cu, temp_snake.Ru);
	  }
	  else
	  {
	     invalid = true;
	  }
	}
	
  }

   if( !manual_seed )
   {
    //IM->visit_label[IM->current_seed_idx] = 1;
      IM->visit_label[current_seed_index] = 1;
   }


   
}
else //Parallel Tracing
{
}

}



