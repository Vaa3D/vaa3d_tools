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
#include "SnakeOperation.h"
#include "../../../v3d_main/jba/newmat11/newmatap.h"
#include "../../../v3d_main/jba/newmat11/newmatio.h"

SnakeClass::SnakeClass(void)
{
	//Cu.SetN(1024);
	collision = 0;
	hit_boundary = false;
	tail_collision_snake_id = -1;
	head_collision_snake_id = -1;
}

SnakeClass SnakeClass::operator=(SnakeClass Snake)
{ 
   //Cu.SetN(1024);
   Cu = Snake.Cu;
   BranchPt = Snake.BranchPt;
   RootPt = Snake.RootPt;
   Ru = Snake.Ru;
   
   return *this;
}

void SnakeClass::SetImage(ImageOperation *I_Input)
{
   IM = I_Input;
}

void SnakeClass::Set_Seed_Point(PointList3D seeds)
{
   collision = 0;
   hit_boundary = false;
   //Cu.NP = 0;
   Cu.RemoveAllPts();
   Cu.AddPtList(seeds);
}

void SnakeClass::Set_Seed_Point(Point3D seed)
{
   collision = 0;
   hit_boundary = false;
   //Cu.NP = 0;
   Cu.RemoveAllPts();
   Cu.AddPt(seed);
}

void SnakeClass::Nail_Branch()
{
  //replace head or tail with new branch point
   for( int i = 0; i < BranchPt.GetSize(); i++ )
   {
	   BranchPt.Pt[i].check_out_of_range_3D(IM->SM,IM->SN,IM->SZ);
	   if( BranchPt.Pt[i].GetDistTo(Cu.GetFirstPt()) > BranchPt.Pt[i].GetDistTo(Cu.GetLastPt()) ) 
	   {
	      Cu.Pt[Cu.NP-1] = BranchPt.Pt[i];
	   }
	   else
	   {
	      Cu.Pt[0] = BranchPt.Pt[i];
	   }
   }
}

void SnakeClass::Branch_Adjustment()
{

   int iter_num = 10;
   typedef itk::VectorLinearInterpolateImageFunction< 
                       GradientImageType, float >  GradientInterpolatorType;

   GradientInterpolatorType::Pointer interpolator = GradientInterpolatorType::New();
   interpolator->SetInputImage(IM->IGVF);

   Point3D temp_pt;

   for( int i = 0; i < BranchPt.GetSize(); i++ )
   {
	 int j = 0;
     while( j < iter_num )
	 {
	    GradientImageType::IndexType index; 
	    BranchPt.Pt[i].check_out_of_range_3D(IM->SM,IM->SN,IM->SZ);
		index[0] = (BranchPt.Pt[i].x);
		index[1] = (BranchPt.Pt[i].y);
		index[2] = (BranchPt.Pt[i].z);
	    GradientPixelType gradient = interpolator->EvaluateAtIndex(index);
		temp_pt.x = gradient[0];
        temp_pt.y = gradient[1];
		temp_pt.z = gradient[2];
        BranchPt.Pt[i] = BranchPt.Pt[i] + temp_pt;
		j++;
	 }
   }

  Nail_Branch();
 
}


void SnakeClass::Expand_Seed_Point(int expand_distance)
{

  Point3D P1, P2;
  Point3D temp_p1, temp_p2;

  int SM = IM->SM;
  int SN = IM->SN;
  int SZ = IM->SZ;

  ImageType::IndexType index; 

  int max_intensity = 0;
  if( Cu.NP == 1 )
  {
     Cu.Pt[Cu.NP-1].check_out_of_range_3D(SM,SN,SZ);

	 for( int i = -1; i <= 0; i++ )
	 {
	   for( int j = -1; j <= 0; j++ )
	   {
	     for( int k = -1; k <= 0; k++ )
		 {
			int sum_intensity = 0;

		    temp_p1.x = Cu.Pt[Cu.NP-1].x + i;
            temp_p1.y = Cu.Pt[Cu.NP-1].y + j;
			temp_p1.z = Cu.Pt[Cu.NP-1].z + k;
			temp_p2.x = Cu.Pt[Cu.NP-1].x - i;
            temp_p2.y = Cu.Pt[Cu.NP-1].y - j;
			temp_p2.z = Cu.Pt[Cu.NP-1].z - k;
			temp_p1.check_out_of_range_3D(SM,SN,SZ);
			temp_p2.check_out_of_range_3D(SM,SN,SZ);

			index[0] = temp_p1.x;
			index[1] = temp_p1.y;
			index[2] = temp_p1.z;
			sum_intensity += IM->I->GetPixel(index);
			index[0] = temp_p2.x;
			index[1] = temp_p2.y;
			index[2] = temp_p2.z;
			sum_intensity += IM->I->GetPixel(index);

			if( sum_intensity > max_intensity )
			{
				max_intensity = sum_intensity;
				P1 = temp_p1;
				P2 = temp_p2;
			}
		 }
 	   }
	 }
    
	 Cu.AddTailPt(P1);
	 Cu.AddPt(P2);
  }
	
   /*int SM = IM->V1->GetLargestPossibleRegion().GetSize()[0];
   int SN = IM->V1->GetLargestPossibleRegion().GetSize()[1];
   int SZ = IM->V1->GetLargestPossibleRegion().GetSize()[2];
   //int expand_distance = 5;
   GradientImageType::IndexType index; 

   Point3D temp_pt;

  if( Cu.NP == 1 )
  {
    Cu.Pt[Cu.NP-1].check_out_of_range_3D(SM,SN,SZ);
	index[0] = (Cu.Pt[Cu.NP-1].x);
    index[1] = (Cu.Pt[Cu.NP-1].y);
	index[2] = (Cu.Pt[Cu.NP-1].z);
  
    GradientPixelType first_p_direction = IM->V1->GetPixel(index);
	temp_pt.x = first_p_direction[0];
    temp_pt.y = first_p_direction[1];
	temp_pt.z = first_p_direction[2];
	temp_pt = Cu.Pt[Cu.NP-1] - temp_pt * 2;
	Cu.AddTailPt(temp_pt);

    temp_pt.x = first_p_direction[0];
    temp_pt.y = first_p_direction[1];
	temp_pt.z = first_p_direction[2];
	temp_pt = Cu.Pt[Cu.NP-1] + temp_pt * 2;
	Cu.AddPt(temp_pt);

     for( int i = 0; i < Cu.GetSize(); i++ )
	 {
		 Cu.Pt[i].check_out_of_range_3D(SM,SN,SZ);
	 }
  }*/

   /*if(Cu.NP == 1)
   {
     for(int i = 1; i < expand_distance; i++)
	 {
	   Cu.Pt[Cu.NP-1].check_out_of_range_3D(SM,SN,SZ);
	   index[0] = (Cu.Pt[Cu.NP-1].x);
       index[1] = (Cu.Pt[Cu.NP-1].y);
	   index[2] = (Cu.Pt[Cu.NP-1].z);
	   GradientPixelType first_p_direction = IM->V1->GetPixel(index);
	   Point3D temp(first_p_direction[0],first_p_direction[1],first_p_direction[2]);
	   temp = Cu.Pt[Cu.NP-1] + temp * 1;
       Cu.AddPt(temp);
	 }
    
	 for( int i = 0; i < Cu.GetSize(); i++ )
	 {
		 Cu.Pt[i].check_out_of_range_3D(SM,SN,SZ);
	 }
   }*/

  //for(int j = 0; j < Cu.NP; j++)
  //{
  //   std::cout<< Cu.Pt[j].x<<","<<Cu.Pt[j].y<<","<<Cu.Pt[j].z<<std::endl;
  //}
}

bool SnakeClass::Check_Validity(float minimum_length, int snake_id, int automatic_merging)
{
   int SM = IM->SM;
   int SN = IM->SN;
   int SZ = IM->SZ;

   bool valid = true;
   Point3D temp_pt;

   float repeat_ratio = 0.5;
   int repeat_dist = 1;

   float background_percent = 0.5;

   //Point3D temp_pt;
   
   //check repeat tracing
  int label_tail = 0;
   int label_head = 0;

   int overlap_pt = 0;
   for( int i = 0; i < Cu.GetSize(); i++ )
   {

    LabelImageType::IndexType index;
	index[0] = Cu.Pt[i].x;
	index[1] = Cu.Pt[i].y;
	index[2] = Cu.Pt[i].z;
    bool overlap = false;
    for( int ix = -repeat_dist; ix <= repeat_dist; ix++ )
    {
     for( int iy = -repeat_dist; iy <= repeat_dist; iy++ )
	 {
	   for( int iz = -repeat_dist; iz <= repeat_dist; iz++ )
	   { 
          LabelImageType::IndexType new_index;
	      temp_pt.x = index[0] + ix;
          temp_pt.y = index[1] + iy;
		  temp_pt.z = index[2] + iz;
		  temp_pt.check_out_of_range_3D(SM,SN,SZ);
		  new_index[0] = temp_pt.x;
		  new_index[1] = temp_pt.y;
		  new_index[2] = temp_pt.z;
		  
          if( i == 0 )
		  {
		    int label_temp = IM->IL->GetPixel( new_index );
			if( label_temp != 0 )
		       label_tail = label_temp;
		  }
		  else if( i == Cu.GetSize() - 1 )
		  { 
		    int label_temp = IM->IL->GetPixel( new_index );
			if( label_temp != 0 )
		       label_head = label_temp;
		  }

		  if( IM->IL->GetPixel( new_index ) != 0 ) 
            overlap = true;
	   }
	 }
	}
	if( overlap )
     overlap_pt++;

   }


   //check for repeating and loop
   if( (float)overlap_pt/(float)Cu.GetSize() > repeat_ratio  || (label_tail == label_head && label_tail != 0) )
   {
	  //std::cout<<"repeating tracing"<<std::endl;
      valid = false;
	  return valid;     
   }

   //check length
   if( Cu.GetLength() <= minimum_length )
   {
	   //std::cout<<"less than minimum length, not connected"<<std::endl;
	   valid = false;
	   return valid;
   }


   return valid;
}

void SnakeClass::SetTracedSnakes(SnakeListClass *S)
{
   SnakeList = S;
}

bool SnakeClass::Check_Head_Collision(ImageType::IndexType index, int collision_dist, int minimum_length, bool automatic_merging, int max_angle, int snake_id)
{
   int angle_th = max_angle;

   bool merging = false;
   if( collision == 2 || collision == 3 )
	   return merging;
  
   int SM = IM->I->GetLargestPossibleRegion().GetSize()[0];
   int SN = IM->I->GetLargestPossibleRegion().GetSize()[1];
   int SZ = IM->I->GetLargestPossibleRegion().GetSize()[2];
 
   bool overlap = false;
   Point3D temp_pt;
   //Point3D head_pt(index[0],index[1],index[2]);
   head_pt.x = index[0];
   head_pt.y = index[1];
   head_pt.z = index[2];
   float L3 = Cu.GetLength();

   for( int ix = -collision_dist; ix <= collision_dist; ix++ )
   {
     for( int iy = -collision_dist; iy <= collision_dist; iy++ )
	 {
	   for( int iz = -collision_dist; iz <= collision_dist; iz++ )
	   { 
          LabelImageType::IndexType new_index;
	      temp_pt.x = index[0] + ix;
          temp_pt.y = index[1] + iy;
		  temp_pt.z = index[2] + iz;
		  temp_pt.check_out_of_range_3D(SM,SN,SZ);
		  new_index[0] = temp_pt.x;
		  new_index[1] = temp_pt.y;
		  new_index[2] = temp_pt.z;


		 //check if the head is in soma region
	     if( IM->Centroid.NP != 0 )
		 {
		  int id_soma = IM->ISoma->GetPixel(new_index);
		  if( id_soma != 0 && Cu.NP > 3 && Cu.GetLength() > minimum_length )
		  {
		     Cu.AddPt( IM->Centroid.Pt[id_soma-1] );
			 Ru.push_back( 1 );
             //BranchPt.AddPt(Cu.Pt[Cu.NP-1]);
			 //std::cout<<"head grows into soma region..."<<std::endl;
			 RootPt.AddPt(Cu.Pt[Cu.NP-1]);

			   if( collision == 1 )
				 collision = 3;
			   else
				 collision = 2;
			   merging = true;
			   return merging;
		  }
		 }


		  int id = IM->IL->GetPixel( new_index );
	      if( id != 0 && id != snake_id )
		  {
		    //std::cout<<"head collision"<<std::endl;
			//int id = IM->IL->GetPixel( new_index );

			//find nearest point at the traced snake
			vnl_vector<float> dist_temp(SnakeList->Snakes[id-1].Cu.GetSize());
		    for( int i = 0; i < SnakeList->Snakes[id-1].Cu.GetSize(); i++ )
			{
			   dist_temp(i) =  head_pt.GetDistTo(SnakeList->Snakes[id-1].Cu.Pt[i]);
			}

			int pt_id = dist_temp.arg_min();


			if( automatic_merging )
		    {
            float L1 = SnakeList->Snakes[id-1].Cu.GetPartLength(pt_id,0);
		    float L2 = SnakeList->Snakes[id-1].Cu.GetPartLength(pt_id,1);
			
			if( pt_id != 0 && pt_id != SnakeList->Snakes[id-1].Cu.GetSize() - 1 )
			{
	        
			  //if( L1 > minimum_length && L2 > minimum_length && L3 > minimum_length )
			  if( L1 > minimum_length && L2 > minimum_length )
			  {

			     Cu.Pt[Cu.NP-1] = SnakeList->Snakes[id-1].Cu.Pt[pt_id];
				 //Ru[Cu.NP-1] = SnakeList->Snakes[id-1].Ru[pt_id];
				 //Ru[Cu.NP-1] = 0;
				 Ru[Cu.NP-1] = Ru[Cu.NP-2];
			     BranchPt.AddPt(Cu.Pt[Cu.NP-1]);
				 SnakeList->Snakes[id-1].BranchPt.AddPt(Cu.Pt[Cu.NP-1]);
			     //std::cout<<"head collision, branch point detected..."<<std::endl;
				 if( collision == 1 )
				  collision = 3;
			     else
				  collision = 2;
				 head_collision_snake_id = id-1;

			     return merging;
			  }
			  else if( L1 <= minimum_length && L2 > minimum_length && L3 > minimum_length )
			  {
			     
				 //PointList3D Cu_Temp;
				 Cu_Backup = Cu;
				 std::vector<float> Ru_Backup = Ru;

				 for( int im = pt_id; im < SnakeList->Snakes[id-1].Cu.GetSize(); im++ )
				 {
					 Cu.AddPt(SnakeList->Snakes[id-1].Cu.Pt[im]);
					 Ru.push_back(SnakeList->Snakes[id-1].Ru[im]);
				 }
				 
				 if( Cu.check_for_sharp_turn(angle_th) )
				 {
				   Cu = Cu_Backup;
				   Ru = Ru_Backup;

				   Cu.Pt[Cu.NP-1] = SnakeList->Snakes[id-1].Cu.Pt[pt_id];
				   //Ru[Cu.NP-1] = SnakeList->Snakes[id-1].Ru[pt_id];
                   //Ru[Cu.NP-1] = 0;
				   Ru[Cu.NP-1] = Ru[Cu.NP-2];
			       BranchPt.AddPt(Cu.Pt[Cu.NP-1]);
				   SnakeList->Snakes[id-1].BranchPt.AddPt(Cu.Pt[Cu.NP-1]);
			       //std::cout<<"head collision, branch point detected..."<<std::endl;
				   if( collision == 1 )
				    collision = 3;
			       else
				    collision = 2;
				   head_collision_snake_id = id-1;

				   return merging;
				 }
                 
				 //std::cout<<"head merging 1.................."<<std::endl;
				 SnakeList->RemoveSnake(id-1);
				 merging = true;
				 //std::cout<<"Ru size:"<<Ru.size()<<std::endl;
			     //std::cout<<"Cu size:"<<Cu.NP<<std::endl;
				 return merging; 
			  }
			  else if( L2 <= minimum_length && L1 > minimum_length && L3 > minimum_length )
			  {
                 
				 //PointList3D Cu_Temp;
				 Cu_Backup = Cu;
                 std::vector<float> Ru_Backup = Ru;

				 for( int im = pt_id; im >= 0; im-- )
				 {
					 Cu.AddPt(SnakeList->Snakes[id-1].Cu.Pt[im]);
					 Ru.push_back(SnakeList->Snakes[id-1].Ru[im]);
				 }

				 if( Cu.check_for_sharp_turn(angle_th) )
				 {
				   Cu = Cu_Backup;
				   Ru = Ru_Backup;

				   Cu.Pt[Cu.NP-1] = SnakeList->Snakes[id-1].Cu.Pt[pt_id];
				   //Ru[Cu.NP-1] = SnakeList->Snakes[id-1].Ru[pt_id];
				   //Ru[Cu.NP-1] = 0;
				   Ru[Cu.NP-1] = Ru[Cu.NP-2];
			       BranchPt.AddPt(Cu.Pt[Cu.NP-1]);
				   SnakeList->Snakes[id-1].BranchPt.AddPt(Cu.Pt[Cu.NP-1]);
			       //std::cout<<"head collision, branch point detected..."<<std::endl;
				   if( collision == 1 )
				    collision = 3;
			       else
				    collision = 2;
				   head_collision_snake_id = id-1;

				   return merging;
				 }
                 
				 //std::cout<<"head merging 2.................."<<std::endl;
				 SnakeList->RemoveSnake(id-1);
				 merging = true;
				 //std::cout<<"Ru size:"<<Ru.size()<<std::endl;
			     //std::cout<<"Cu size:"<<Cu.NP<<std::endl;
				 return merging;
			  }
			}
			else if( pt_id == 0 && L3 > minimum_length )
			{
               
			   //PointList3D Cu_Temp;
			   Cu_Backup = Cu;
			   std::vector<float> Ru_Backup = Ru;

			  // Cu.AddPtList(SnakeList->Snakes[id-1].Cu);
			   for( int im = 0; im < SnakeList->Snakes[id-1].Cu.NP; im++ )
			   {
			     Cu.AddPt(SnakeList->Snakes[id-1].Cu.Pt[im]);
			     Ru.push_back(SnakeList->Snakes[id-1].Ru[im]);
			   }

			    if( Cu.check_for_sharp_turn(angle_th) )
				 {
				   Cu = Cu_Backup;
				   Ru = Ru_Backup;

				   Cu.Pt[Cu.NP-1] = SnakeList->Snakes[id-1].Cu.Pt[pt_id];
				   //Ru[Cu.NP-1] = SnakeList->Snakes[id-1].Ru[pt_id];
				   Ru[Cu.NP-1] = Ru[Cu.NP-2];
			       BranchPt.AddPt(Cu.Pt[Cu.NP-1]);
				   SnakeList->Snakes[id-1].BranchPt.AddPt(Cu.Pt[Cu.NP-1]);
			       //std::cout<<"head collision, branch point detected..."<<std::endl;
				   if( collision == 1 )
				    collision = 3;
			       else
				    collision = 2;
				   head_collision_snake_id = id-1;

				   return merging;
				 }

			   //std::cout<<"head merging 3.................."<<std::endl;
			   SnakeList->RemoveSnake(id-1);
			   merging = true;
			   //std::cout<<"Ru size:"<<Ru.size()<<std::endl;
			   //std::cout<<"Cu size:"<<Cu.NP<<std::endl;
			   return merging;
			}
			else if( pt_id == SnakeList->Snakes[id-1].Cu.GetSize() - 1 && L3 > minimum_length )
			{
			   //SnakeList->Snakes[id-1].Cu.Flip();
                SnakeList->Snakes[id-1].Ru = SnakeList->Snakes[id-1].Cu.Flip_4D( SnakeList->Snakes[id-1].Ru );
			   //PointList3D Cu_Temp;
			   Cu_Backup = Cu;
			   std::vector<float> Ru_Backup = Ru;

			   //Cu.AddPtList(SnakeList->Snakes[id-1].Cu);
			   for( int im = 0; im < SnakeList->Snakes[id-1].Cu.NP; im++ )
			   {
			     Cu.AddPt(SnakeList->Snakes[id-1].Cu.Pt[im]);
			     Ru.push_back(SnakeList->Snakes[id-1].Ru[im]);
			   }

			    if( Cu.check_for_sharp_turn(angle_th) )
				 {
				   Cu = Cu_Backup;
				   Ru = Ru_Backup;

				   //SnakeList->Snakes[id-1].Cu.Flip();
                    SnakeList->Snakes[id-1].Ru = SnakeList->Snakes[id-1].Cu.Flip_4D( SnakeList->Snakes[id-1].Ru );

				   Cu.Pt[Cu.NP-1] = SnakeList->Snakes[id-1].Cu.Pt[pt_id];
				   //Ru[Cu.NP-1] = SnakeList->Snakes[id-1].Ru[pt_id];
				   //Ru[Cu.NP-1] = 0;
				   Ru[Cu.NP-1] = Ru[Cu.NP-2];
			       BranchPt.AddPt(Cu.Pt[Cu.NP-1]);
				   SnakeList->Snakes[id-1].BranchPt.AddPt(Cu.Pt[Cu.NP-1]);
			       //std::cout<<"head collision, branch point detected..."<<std::endl;
				   if( collision == 1 )
				    collision = 3;
			       else
				    collision = 2;
				   head_collision_snake_id = id-1;

				   return merging;
				 }

			   //std::cout<<"head merging 4.................."<<std::endl;
			   SnakeList->RemoveSnake(id-1);
			   merging = true;
			   //std::cout<<"Ru size:"<<Ru.size()<<std::endl;
			   //std::cout<<"Cu size:"<<Cu.NP<<std::endl;
			   return merging;
			}
			}
			else
			{
              Cu.Pt[Cu.NP-1] = SnakeList->Snakes[id-1].Cu.Pt[pt_id];
			  //Ru[Cu.NP-1] = SnakeList->Snakes[id-1].Ru[pt_id];
			  //Ru[Cu.NP-1] = 0;
			  Ru[Cu.NP-1] = Ru[Cu.NP-2];
			  BranchPt.AddPt(Cu.Pt[Cu.NP-1]);
              SnakeList->Snakes[id-1].BranchPt.AddPt(Cu.Pt[Cu.NP-1]);
			  merging = false;
			  //std::cout<<"head collision, branch point detected..."<<std::endl;
			  if( collision == 1 )
				collision = 3;
			  else
				collision = 2;
			  head_collision_snake_id = id-1;
			  return merging;
            }
		  }
	   }
	 }
   }
   return merging;
}

bool SnakeClass::Check_Tail_Collision(ImageType::IndexType index, int collision_dist, int minimum_length, bool automatic_merging, int max_angle, int snake_id)
{
   int angle_th = max_angle;
   bool merging = false;

   if( collision == 1 || collision == 3)
	   return merging;

   int SM = IM->I->GetLargestPossibleRegion().GetSize()[0];
   int SN = IM->I->GetLargestPossibleRegion().GetSize()[1];
   int SZ = IM->I->GetLargestPossibleRegion().GetSize()[2];
 
   bool overlap = false;
   Point3D temp_pt;
   //Point3D tail_pt(index[0],index[1],index[2]);
   tail_pt.x = index[0];
   tail_pt.y = index[1];
   tail_pt.z = index[2];

   float L3 = Cu.GetLength();

   for( int ix = -collision_dist; ix <= collision_dist; ix++ )
   {
     for( int iy = -collision_dist; iy <=collision_dist; iy++ )
	 {
	   for( int iz = -collision_dist; iz <=collision_dist; iz++ )
	   { 
          LabelImageType::IndexType new_index;

	      temp_pt.x = index[0] + ix;
          temp_pt.y = index[1] + iy;
		  temp_pt.z = index[2] + iz;
		  temp_pt.check_out_of_range_3D(SM,SN,SZ);
		  new_index[0] = temp_pt.x;
		  new_index[1] = temp_pt.y;
		  new_index[2] = temp_pt.z;

		 //check if the tail is in soma region
	     if( IM->Centroid.NP != 0 )
		 {
		  int id_soma = IM->ISoma->GetPixel(new_index);
		  if( id_soma != 0 && Cu.NP > 3 && Cu.GetLength() > minimum_length)
		  {
		     Cu.AddTailPt( IM->Centroid.Pt[id_soma-1] );
			 //Ru.push_back( 0 );
			 Ru.insert(Ru.begin(), 1);
			 //BranchPt.AddPt(Cu.Pt[0]);
			 //std::cout<<"tail grows into soma region..."<<std::endl;
             RootPt.AddPt(Cu.Pt[0]);

			   if( collision == 2 )
				 collision = 3;
			   else
				 collision = 1;
			   merging = true;
			   return merging;
		  }
		 }

		  int id = IM->IL->GetPixel( new_index );
	      if( id != 0 && id != snake_id )
		  {
			//int id = IM->IL->GetPixel( new_index );
			//find nearest point at the traced snake
			vnl_vector<float> dist_temp(SnakeList->Snakes[id-1].Cu.GetSize());
            //check if snake is removed
			if( SnakeList->valid_list[id-1] == 0 )
                 continue;

			overlap = true;
		    for( int i = 0; i < SnakeList->Snakes[id-1].Cu.GetSize(); i++ )
			{
			   dist_temp(i) =  tail_pt.GetDistTo(SnakeList->Snakes[id-1].Cu.Pt[i]);
			}

			int pt_id = dist_temp.arg_min();

		    if( automatic_merging )
			{
			float L1 = SnakeList->Snakes[id-1].Cu.GetPartLength(pt_id,0);
		    float L2 = SnakeList->Snakes[id-1].Cu.GetPartLength(pt_id,1);

			if( pt_id != 0 && pt_id != SnakeList->Snakes[id-1].Cu.GetSize() - 1 )
			{
	        
			  //if( L1 > minimum_length && L2 > minimum_length && L3 > minimum_length )
			  if( L1 > minimum_length && L2 > minimum_length )
			  {
	
			     Cu.Pt[0] = SnakeList->Snakes[id-1].Cu.Pt[pt_id];
				 //Ru[0] = SnakeList->Snakes[id-1].Ru[pt_id];
				 //Ru[0] = 0;
				 Ru[0] = Ru[1];
			     BranchPt.AddPt(Cu.Pt[0]);
				 SnakeList->Snakes[id-1].BranchPt.AddPt(Cu.Pt[0]);
			     //std::cout<<"tail collision, branch point detected..."<<std::endl;
				 if( collision == 2 )
				  collision = 3;
			     else
				  collision = 1;
				 tail_collision_snake_id = id-1;

			     return merging;
			  }
			  else if( L1 <= minimum_length && L2 > minimum_length && L3 > minimum_length )
			  {

				 //SnakeList->Snakes[id-1].Cu.Flip();
				  SnakeList->Snakes[id-1].Ru = SnakeList->Snakes[id-1].Cu.Flip_4D( SnakeList->Snakes[id-1].Ru );

                 //PointList3D Cu_Temp;
				 Cu_Backup = Cu;
				 std::vector<float> Ru_Backup = Ru;

				 for( int im = SnakeList->Snakes[id-1].Cu.GetSize() - pt_id; im >= 0; im-- )
				 {
					 Cu.AddTailPt(SnakeList->Snakes[id-1].Cu.Pt[im]);
                     Ru.insert ( Ru.begin() , SnakeList->Snakes[id-1].Ru[im] );
				 }

			    if( Cu.check_for_sharp_turn(angle_th) )
				 {
				   Cu = Cu_Backup;
				   Ru = Ru_Backup;

				   //SnakeList->Snakes[id-1].Cu.Flip();
				    SnakeList->Snakes[id-1].Ru = SnakeList->Snakes[id-1].Cu.Flip_4D( SnakeList->Snakes[id-1].Ru );

				   Cu.Pt[0] = SnakeList->Snakes[id-1].Cu.Pt[pt_id];
				   //Ru[0] = SnakeList->Snakes[id-1].Ru[pt_id];
				   //Ru[0] = 0;
				   Ru[0] = Ru[1];
			       BranchPt.AddPt(Cu.Pt[0]);
				   SnakeList->Snakes[id-1].BranchPt.AddPt(Cu.Pt[0]);
			       //std::cout<<"tail collision, branch point detected..."<<std::endl;
				   if( collision == 2 )
				    collision = 3;
			       else
				    collision = 1;
				   tail_collision_snake_id = id-1;

				   return merging;
				 }

				 //std::cout<<"tail merging 1.................."<<std::endl;
				 SnakeList->RemoveSnake(id-1);
				 merging = true;
				 return merging; 
			  }
			  else if( L1 > minimum_length && L2 <= minimum_length && L3 > minimum_length )
			  {
                 //PointList3D Cu_Temp;

				 Cu_Backup = Cu;
                 std::vector<float> Ru_Backup = Ru;

				 for( int im = pt_id; im >= 0; im-- )
				 {
					 Cu.AddTailPt(SnakeList->Snakes[id-1].Cu.Pt[im]);
                     Ru.insert ( Ru.begin() , SnakeList->Snakes[id-1].Ru[im] );
				 }

			    if( Cu.check_for_sharp_turn(angle_th) )
				 {
				   Cu = Cu_Backup;
				   Ru = Ru_Backup;

				   Cu.Pt[0] = SnakeList->Snakes[id-1].Cu.Pt[pt_id];
                   //Ru[0] = SnakeList->Snakes[id-1].Ru[pt_id];
				   //Ru[0] = 0;
				   Ru[0] = Ru[1];
				   //BranchPt.RemoveAllPts();
			       BranchPt.AddPt(Cu.Pt[0]);
				   SnakeList->Snakes[id-1].BranchPt.AddPt(Cu.Pt[0]);
			       //std::cout<<"tail collision, branch point detected..."<<std::endl;
				   if( collision == 2 )
				    collision = 3;
			       else
				    collision = 1;
				   tail_collision_snake_id = id-1;

				   return merging;
				 }
   
				 //std::cout<<"tail merging 2.................."<<std::endl;
				 SnakeList->RemoveSnake(id-1);
				 merging = true;
				 return merging;
			  }
			}
			else if( pt_id == 0 && L3 > minimum_length )
			{
			   //SnakeList->Snakes[id-1].Cu.Flip();
				 SnakeList->Snakes[id-1].Ru = SnakeList->Snakes[id-1].Cu.Flip_4D( SnakeList->Snakes[id-1].Ru );

			   //PointList3D Cu_Temp;

			   Cu_Backup = Cu;
			   std::vector<float> Ru_Backup = Ru;

			   //Cu.AddTailPtList(SnakeList->Snakes[id-1].Cu);
               for( int im = SnakeList->Snakes[id-1].Cu.GetSize()-1 ; im >=0; im-- )
			   {
			      Cu.AddTailPt(SnakeList->Snakes[id-1].Cu.Pt[im]);
                  Ru.insert ( Ru.begin() , SnakeList->Snakes[id-1].Ru[im] );
			   }

			    if( Cu.check_for_sharp_turn(angle_th) )
				{
				   Cu = Cu_Backup;
				   Ru = Ru_Backup;

				   //SnakeList->Snakes[id-1].Cu.Flip();
				    SnakeList->Snakes[id-1].Ru = SnakeList->Snakes[id-1].Cu.Flip_4D( SnakeList->Snakes[id-1].Ru );

				   Cu.Pt[0] = SnakeList->Snakes[id-1].Cu.Pt[pt_id];
                   //Ru[0] = SnakeList->Snakes[id-1].Ru[pt_id];
				   //Ru[0] = 0;
				   Ru[0] = Ru[1];
			       BranchPt.AddPt(Cu.Pt[0]);
				   SnakeList->Snakes[id-1].BranchPt.AddPt(Cu.Pt[0]);
			       //std::cout<<"tail collision, branch point detected..."<<std::endl;
				   if( collision == 2 )
				    collision = 3;
			       else
				    collision = 1;
				   tail_collision_snake_id = id-1;

				   return merging;
				}

			   //std::cout<<"tail merging 3.................."<<std::endl;
			   SnakeList->RemoveSnake(id-1);
			   merging = true;
			   return merging;
			}
			else if( pt_id == SnakeList->Snakes[id-1].Cu.GetSize() - 1 && L3 > minimum_length )
			{
	           //PointList3D Cu_Temp;
			   Cu_Backup = Cu;
			   std::vector<float> Ru_Backup = Ru;

			   //Cu.AddTailPtList(SnakeList->Snakes[id-1].Cu);
               for( int im = pt_id; im >=0; im-- )
			   {
			      Cu.AddTailPt(SnakeList->Snakes[id-1].Cu.Pt[im]);
                  Ru.insert ( Ru.begin() , SnakeList->Snakes[id-1].Ru[im] );
			   }

			    if( Cu.check_for_sharp_turn(angle_th) )
				{
				   Cu = Cu_Backup;
				   Ru = Ru_Backup;

				   Cu.Pt[0] = SnakeList->Snakes[id-1].Cu.Pt[pt_id];
                   //Ru[0] = SnakeList->Snakes[id-1].Ru[pt_id];
				   //Ru[0] = 0;
				   Ru[0] = Ru[1];
			       BranchPt.AddPt(Cu.Pt[0]);
				   SnakeList->Snakes[id-1].BranchPt.AddPt(Cu.Pt[0]);
			       //std::cout<<"tail collision, branch point detected..."<<std::endl;
				   if( collision == 2 )
				    collision = 3;
			       else
				    collision = 1;
				   tail_collision_snake_id = id-1;

				   return merging;
				}

			   //std::cout<<"tail merging 4.................."<<std::endl;
			   SnakeList->RemoveSnake(id-1);
			   merging = true;
			   return merging;
			}
			}
			else
			{
              Cu.Pt[0] = SnakeList->Snakes[id-1].Cu.Pt[pt_id];
			  //Ru[0] = SnakeList->Snakes[id-1].Ru[pt_id];
			  //Ru[0] = 0;
			  Ru[0] = Ru[1];
			  BranchPt.AddPt(Cu.Pt[0]);
              SnakeList->Snakes[id-1].BranchPt.AddPt(Cu.Pt[0]);
			  merging = false;
			  //std::cout<<"tail collision, branch point detected..."<<std::endl;
			   if( collision == 2 )
				 collision = 3;
			   else
				 collision = 1;
			   tail_collision_snake_id = id-1;
			   return merging;
			}
		  }
	   }
	 }
   }
   return merging;
}

bool SnakeClass::Compute_Seed_Force(int head_tail, int distance)
{
	bool seed_force = false;
	Vector3D SForce;

   if( head_tail == 0 )
   {
    vnl_vector<float> dist_temp(IM->SeedPt.NP);
	dist_temp.fill(10000);

	SForce.x = Cu.Pt[0].x - Cu.Pt[2].x;
    SForce.y = Cu.Pt[0].x - Cu.Pt[2].y;
	SForce.z = Cu.Pt[0].x - Cu.Pt[2].z;

    for( int i = 0; i < IM->SeedPt.NP; i++ )
    {
	   if( IM->visit_label(i) == 1 )
		   continue;
	   
	   //if( Cu.Pt[0].GetDistTo(IM->SeedPt.Pt[i]) <= distance )
	   //{
	     if( ((IM->SeedPt.Pt[i].x - Cu.Pt[0].x) * SForce.x + (IM->SeedPt.Pt[i].y - Cu.Pt[0].y) * SForce.y + (IM->SeedPt.Pt[i].z - Cu.Pt[0].z) * SForce.z) > 0 )
		 {
		    //seed_force = true;
			//Cu.Pt[0] = IM->SeedPt.Pt[i];
			//break;
           dist_temp(i) = Cu.Pt[0].GetDistTo(IM->SeedPt.Pt[i]);
		 }
	   //}
    }
	
	if( dist_temp.min_value() <= distance )
	{
		seed_force = true;
	    Cu.Pt[0] = IM->SeedPt.Pt[dist_temp.arg_min()];
	}
   }
   else
   {
    vnl_vector<float> dist_temp(IM->SeedPt.NP);
	dist_temp.fill(10000);

   SForce.x = Cu.Pt[Cu.NP-1].x - Cu.Pt[Cu.NP-3].x;
    SForce.y = Cu.Pt[Cu.NP-1].x - Cu.Pt[Cu.NP-3].y;
	SForce.z = Cu.Pt[Cu.NP-1].x - Cu.Pt[Cu.NP-3].z;

    for( int i = 0; i < IM->SeedPt.NP; i++ )
    {
	   if( IM->visit_label(i) == 1 )
		   continue;
	   
	   //if( Cu.Pt[Cu.NP-1].GetDistTo(IM->SeedPt.Pt[i]) <= distance )
	   //{
	     if( ((IM->SeedPt.Pt[i].x - Cu.Pt[Cu.NP-1].x) * SForce.x + (IM->SeedPt.Pt[i].y - Cu.Pt[Cu.NP-1].y) * SForce.y + (IM->SeedPt.Pt[i].z - Cu.Pt[Cu.NP-1].z) * SForce.z) > 0 )
		 {
		    //seed_force = true;
			//Cu.Pt[Cu.NP-1] = IM->SeedPt.Pt[i];
			//break;
			 dist_temp(i) = Cu.Pt[Cu.NP-1].GetDistTo(IM->SeedPt.Pt[i]);
		 }
	   //}
    }
	if( dist_temp.min_value() <= distance)
	{
	  seed_force = true;
      Cu.Pt[Cu.NP-1] = IM->SeedPt.Pt[dist_temp.arg_min()];
	}
   }

   return seed_force;
}

void SnakeClass::OpenSnake_Init_4D(float alpha, int ITER, float beta, float kappa, float gamma, int pt_distance)
{
  //deform the 3 starting point and roughly estimate the radii

  float pi = 3.1415926;
  int m = 8;
   typedef itk::NearestNeighborInterpolateImageFunction< 
                       ImageType, float>  InterpolatorType1;

   InterpolatorType1::Pointer I_interpolator = InterpolatorType1::New();
   I_interpolator->SetInputImage(IM->I);

   int SM = IM->SM;
   int SN = IM->SN;
   int SZ = IM->SZ;

   int N = Cu.GetSize();

   vnl_matrix<float> A = makeOpenA(alpha, beta, N);
   vnl_matrix<float> I(N,N);
   I.set_identity();
   vnl_matrix<float> invAI = vnl_matrix_inverse<float>( A + I * gamma);
   vnl_vector<float> vnl_Ru(N);
   vnl_Ru.fill(0);


   for( int j = 0; j < N; j++ )
   {
     vnl_Ru(j) = Ru[j];
   }

   std::cout<<"this is zhi"<<std::endl;

  for( int iter = 0; iter < ITER; iter++ )
  {
	 vnl_vector<float> mfx(N);
	 vnl_vector<float> mfy(N);
	 vnl_vector<float> mfz(N);
	 vnl_vector<float> mfr(N);
     mfx.fill(0);
     mfy.fill(0);
	 mfz.fill(0);
	 mfr.fill(0);

	 vnl_vector<float> x(N);
     vnl_vector<float> y(N);
	 vnl_vector<float> z(N);

	 Vector3D v1,v2,v3, vtemp;
	 Point3D temp_r_pt;
	 vnl_matrix<float> H(3,3);
	 H.fill(0);

	 for( int j = 0; j < Cu.GetSize(); j++ )
	 {
		x(j) = Cu.GetPt(j).x;
		y(j) = Cu.GetPt(j).y;
        z(j) = Cu.GetPt(j).z;

        GradientImageType::IndexType index; 
        index[0] = (x(j));
		index[1] = (y(j));
		index[2] = (z(j));

	   //compute the radius force
	   if( j == 0 )
	   {
	      v1.x = Cu.Pt[0].x - Cu.Pt[1].x;
          v1.y = Cu.Pt[0].y - Cu.Pt[1].y;
          v1.z = Cu.Pt[0].z - Cu.Pt[1].z;
          v1.ConvertUnit();
	   }
	   else
	   {
	   	  v1.x = Cu.Pt[j].x - Cu.Pt[j-1].x;
          v1.y = Cu.Pt[j].y - Cu.Pt[j-1].y;
          v1.z = Cu.Pt[j].z - Cu.Pt[j-1].z;
          v1.ConvertUnit();
	   }

       v2.x = -v1.z;
       v2.y = 0;
       v2.z = v1.x;
       v2.ConvertUnit();
       v3.x = 1;
       v3.y = -(pow(v1.x,2) + pow(v1.z,2))/(v1.x*v1.y + std::numeric_limits<float>::epsilon());
       v3.z = v1.z/(v1.x + std::numeric_limits<float>::epsilon());
       v3.ConvertUnit();

	   float force_r = 0;
	   vnl_vector<float> force(3);
	   force.fill(0);
      
	   float force_r_region = 0;
	   vnl_vector<float> force_region(3);
	   force_region.fill(0);

	    for( int k = 0; k < m; k++ )
	    {
	      float theta = (2 * pi * k)/m;
	      vtemp.x = v2.x * cos(theta) + v3.x * sin(theta);
	      vtemp.y = v2.y * cos(theta) + v3.y * sin(theta);
		  vtemp.z = v2.z * cos(theta) + v3.z * sin(theta);
		  vtemp.ConvertUnit();

		  vnl_vector<float> oj(3);
		  oj(0) = vtemp.x;
		  oj(1) = vtemp.y;
		  oj(2) = vtemp.z;

		  temp_r_pt.x = x(j) + vnl_Ru(j) * vtemp.x;
		  temp_r_pt.y = y(j) + vnl_Ru(j) * vtemp.y;
		  temp_r_pt.z = z(j) + vnl_Ru(j) * vtemp.z;

          if( isinf( temp_r_pt.x ) || isinf( temp_r_pt.y ) || isinf( temp_r_pt.z ) )
		  {
		    continue;
		  }
		
		  if( temp_r_pt.check_out_of_range_3D(IM->SM,IM->SN,IM->SZ) )
			continue;

          ProbImageType::IndexType temp_index; 
          temp_index[0] = temp_r_pt.x;
          temp_index[1] = temp_r_pt.y;
		  temp_index[2] = temp_r_pt.z;

           float Ic = I_interpolator->EvaluateAtIndex(temp_index);
		   //force_region += oj * log(fabs(Ic - IM->u1)/(fabs(Ic - IM->u2)+std::numeric_limits<float>::epsilon()) + std::numeric_limits<float>::epsilon());
		   //force_r_region += log(fabs(Ic - IM->u1)/(fabs(Ic - IM->u2)+std::numeric_limits<float>::epsilon()) + std::numeric_limits<float>::epsilon());
           float prob1 = norm_density(Ic, IM->u1, IM->sigma1);
		   float prob2 = norm_density(Ic, IM->u2, IM->sigma2);
		   float sum_prob = prob1 + prob2;
		   prob1 /= sum_prob;
		   prob2 /= sum_prob;
		   float eps = std::numeric_limits<float>::epsilon();
		   force_region += oj * log(MAX_snake(prob1/MAX_snake(prob2,eps),eps)) * -1;
		   force_r_region += log(MAX_snake(prob1/MAX_snake(prob2,eps),eps)) * -1;
		}
        
		float mag2 = MAX_snake(sqrt(pow(force_region[0],2) + pow(force_region[1],2) + pow(force_region[2],2) + pow(force_r_region,2)),std::numeric_limits<float>::epsilon());
	    force_region /= mag2;
        force_r_region /= mag2;

		mfx(j) = force_region(0);
		mfy(j) = force_region(1);
		mfz(j) = force_region(2);
		mfr(j) = force_r_region;
	 }

    x = (invAI * ( x * gamma - mfx));
    y = (invAI * ( y * gamma - mfy));
    z = (invAI * ( z * gamma - mfz));
	vnl_Ru = (invAI * ( vnl_Ru * gamma  - mfr));

    for( unsigned int k = 0; k < x.size(); k++ )
	{
	    Cu.Pt[k].x = x(k);
        Cu.Pt[k].y = y(k);
		Cu.Pt[k].z = z(k);
		Cu.Pt[k].check_out_of_range_3D(SM,SN,SZ);
	}
  }

  //resampling
   vnl_Ru = Cu.curveinterp_4D((float)pt_distance, vnl_Ru);
   Ru.clear();
   for( int i = 0; i < vnl_Ru.size(); i++ )
   {
     if( vnl_Ru(i) < 0)
	 	 vnl_Ru(i) = 0;

     Ru.push_back(vnl_Ru(i));
   }
}

void SnakeClass::OpenSnakeStretch_5D(float alpha, int ITER, int pt_distance, float beta, float kappa, float gamma, 
                                float stretchingRatio, int collision_dist, int minimum_length, 
								bool automatic_merging, int max_angle, bool freeze_body, int s_force, int snake_id, 
								int tracing_model, int coding_method, float sigma_ratio, int border)
{

   int hit_boundary_dist = border;
   float pi = 3.1415926;
   int m = 8;
   int band_width = 0;

   float eps = std::numeric_limits<float>::epsilon();

   float stretchingRatio_head = stretchingRatio;
   float stretchingRatio_tail = stretchingRatio;

   //when collision happened for both tail and head, stop stretching
   if( collision == 3)
	   return;

   typedef itk::VectorLinearInterpolateImageFunction< 
                       GradientImageType, float >  GradientInterpolatorType;
   GradientInterpolatorType::Pointer interpolator = GradientInterpolatorType::New();
   interpolator->SetInputImage(IM->IGVF);
   /*GradientInterpolatorType::Pointer interpolator_V1 = GradientInterpolatorType::New();
   interpolator_V1->SetInputImage(IM->V1);*/

   typedef itk::LinearInterpolateImageFunction< 
                       ProbImageType, float>  InterpolatorType;

   typedef itk::LinearInterpolateImageFunction< 
                       ImageType, float>  InterpolatorType1;

   typedef itk::LinearInterpolateImageFunction< 
                       LabelImageType, float>  InterpolatorType2;

   InterpolatorType1::Pointer I_interpolator = InterpolatorType1::New();
   I_interpolator->SetInputImage(IM->I);

   InterpolatorType2::Pointer IL_interpolator = InterpolatorType2::New();
   IL_interpolator->SetInputImage(IM->IL);

   int SM = IM->SM;
   int SN = IM->SN;
   int SZ = IM->SZ;
  
   int N = Cu.GetSize();
   vnl_matrix<float> A = makeOpenA(alpha, beta, N);
   vnl_matrix<float> I(N,N);
   I.set_identity();
   vnl_matrix<float> invAI = vnl_matrix_inverse<float>( A + I * gamma);

   vnl_vector<float> vnl_Ru(N);
   vnl_Ru.fill(0);
   vnl_vector<float> vnl_Ru1(N);
   vnl_Ru1.fill(0);
   for( int j = 0; j < N; j++ )
   {
     vnl_Ru(j) = Ru[j];
	 vnl_Ru1(j) = Ru1[j];
   }
   

   //evolve 4D snake
   for( int iter = 0; iter < ITER; iter++ )
   {
     vnl_vector<float> vfx(N);
     vnl_vector<float> vfy(N);
	 vnl_vector<float> vfz(N);
 
     vnl_vector<float> x(N);
     vnl_vector<float> y(N);
	 vnl_vector<float> z(N);
	 
	 vnl_vector<float> mfx(N);
	 vnl_vector<float> mfy(N);
	 vnl_vector<float> mfz(N);
	 vnl_vector<float> mfr(N);
	 vnl_vector<float> mfr1(N);

	 vfx.fill(0);
	 vfy.fill(0);
	 vfz.fill(0);

	 mfx.fill(0);
     mfy.fill(0);
	 mfz.fill(0);
	 mfr.fill(0);
	 mfr1.fill(0);

	 //GradientImageType::IndexType head_index; 
	 //GradientImageType::IndexType tail_index; 
	 GradientInterpolatorType::ContinuousIndexType head_index;
     GradientInterpolatorType::ContinuousIndexType tail_index;

	 Vector3D v1,v2,v3, vtemp;
	 Point3D temp_r_pt;

	 for( int j = 0; j < Cu.GetSize(); j++ )
	 {
		x(j) = Cu.GetPt(j).x;
		y(j) = Cu.GetPt(j).y;
        z(j) = Cu.GetPt(j).z;


        //GradientImageType::IndexType index; 
		GradientInterpolatorType::ContinuousIndexType index;
        index[0] = (x(j));
		index[1] = (y(j));
		index[2] = (z(j));

	   if( j == 0 )
	   {
	     tail_index[0] = Cu.GetPt(j).x;
		 tail_index[1] = Cu.GetPt(j).y;
		 tail_index[2] = Cu.GetPt(j).z;

		 /*//project field flow to the normal plane
		 GradientPixelType gvf = interpolator->EvaluateAtIndex(index);
         float dot_product = gvf[0] * evfx(j) + gvf[1] * evfy(j) + gvf[2] * evfz(j);
         vfx(j) = gvf[0] - dot_product * evfx(j);
         vfy(j) = gvf[1] - dot_product * evfy(j);;
	 	 vfz(j) = gvf[2] - dot_product * evfz(j);;
         continue; */
	   }
	   if( j == Cu.GetSize() - 1 )
	   {
	   	 head_index[0] = Cu.GetPt(j).x;
		 head_index[1] = Cu.GetPt(j).y;
		 head_index[2] = Cu.GetPt(j).z;


		 /*//project field flow to the normal plane
		 GradientPixelType gvf = interpolator->EvaluateAtIndex(index);
         float dot_product = gvf[0] * evfx(j) + gvf[1] * evfy(j) + gvf[2] * evfz(j);
         vfx(j) = gvf[0] - dot_product * evfx(j);
         vfy(j) = gvf[1] - dot_product * evfy(j);;
	 	 vfz(j) = gvf[2] - dot_product * evfz(j);;
         continue;*/
	   }

	   //std::cout<<"check point 0"<<std::endl;
	   //compute the radius force
	   if( j == 0 )
	   {
	      v1.x = Cu.Pt[0].x - Cu.Pt[1].x;
          v1.y = Cu.Pt[0].y - Cu.Pt[1].y;
          v1.z = Cu.Pt[0].z - Cu.Pt[1].z;
          v1.ConvertUnit();
	   }
	   else
	   {
	   	  v1.x = Cu.Pt[j].x - Cu.Pt[j-1].x;
          v1.y = Cu.Pt[j].y - Cu.Pt[j-1].y;
          v1.z = Cu.Pt[j].z - Cu.Pt[j-1].z;
          v1.ConvertUnit();
	   }

       /*v2.x = -v1.z;
       v2.y = 0;
       v2.z = v1.x;
       v2.ConvertUnit();
       v3.x = 1;
       v3.y = -(pow(v1.x,2) + pow(v1.z,2))/(v1.x*v1.y + std::numeric_limits<float>::epsilon());
       v3.z = v1.z/(v1.x + std::numeric_limits<float>::epsilon());
       v3.ConvertUnit();*/

	   v2.x = -1 * v1.x * v1.z;
	   v2.y = v1.y * v1.z;
	   v2.z = pow(v1.x,2) + pow(v1.y,2);
	   v2.ConvertUnit();
	   v3.x = -1 * v1.y;
	   v3.y = v1.x;
	   v3.z = 0;
	   v3.ConvertUnit();
      
	   float force_r_region = 0;
	   float force_r_region1 = 0;
	   vnl_vector<float> force_region(3);
	   force_region.fill(0);

	  //if( vnl_Ru(j) >= 0.1 )
	  if( tracing_model != 0 )
	  {
	   if( IM->SZ != 1 )
	   {
	    for( int k = 0; k < m; k++ )
	    {
	      float theta = (2 * pi * k)/m;
	      vtemp.x = vnl_Ru1(j) * v2.x * cos(theta) + vnl_Ru(j) * v3.x * sin(theta);
	      vtemp.y = vnl_Ru1(j) * v2.y * cos(theta) + vnl_Ru(j) * v3.y * sin(theta);
		  vtemp.z = vnl_Ru1(j) * v2.z * cos(theta) + vnl_Ru(j) * v3.z * sin(theta);

		  temp_r_pt.x = x(j) + vtemp.x;
		  temp_r_pt.y = y(j) + vtemp.y;
		  temp_r_pt.z = z(j) + vtemp.z;

		  vnl_vector<float> oj(3);

		  vtemp.x = vnl_Ru(j) * v2.x * cos(theta) + vnl_Ru1(j) * v3.x * sin(theta);
	      vtemp.y = vnl_Ru(j) * v2.y * cos(theta) + vnl_Ru1(j) * v3.y * sin(theta);
		  vtemp.z = vnl_Ru(j) * v2.z * cos(theta) + vnl_Ru1(j) * v3.z * sin(theta);
          vtemp.ConvertUnit();

		  oj(0) = vtemp.x;
		  oj(1) = vtemp.y;
		  oj(2) = vtemp.z;

          //if( isinf( temp_r_pt.x ) || isinf( temp_r_pt.y ) || isinf( temp_r_pt.z ) )
		  //{
		  //  continue;
		  //}
		
		  if( temp_r_pt.check_out_of_range_3D(IM->SM,IM->SN,IM->SZ) )
			continue;

          //ProbImageType::IndexType temp_index; 
		  GradientInterpolatorType::ContinuousIndexType temp_index;
          temp_index[0] = temp_r_pt.x;
          temp_index[1] = temp_r_pt.y;
		  temp_index[2] = temp_r_pt.z;

		  /*GradientPixelType gvf = interpolator->EvaluateAtIndex(temp_index);
          vnl_vector<float> nj(3);
		  nj(0) = gvf[0];
		  nj(1) = gvf[1];
		  nj(2) = gvf[2];*/

		   float Ic = I_interpolator->EvaluateAtContinuousIndex(temp_index);
		   float ILc = IL_interpolator->EvaluateAtContinuousIndex(temp_index);
           if( ILc != 0 )
             continue;

		   //force_region += oj * log(fabs(Ic - IM->u1)/(fabs(Ic - IM->u2)+std::numeric_limits<float>::epsilon()) + std::numeric_limits<float>::epsilon());
		   //force_r_region += log(fabs(Ic - IM->u1)/(fabs(Ic - IM->u2)+std::numeric_limits<float>::epsilon()) + std::numeric_limits<float>::epsilon());
           float prob1 = norm_density(Ic, IM->u1, IM->sigma1);
		   float prob2 = norm_density(Ic, IM->u2, IM->sigma2);
		   /*float sum_prob = prob1 + prob2;
		   prob1 /= sum_prob;
		   prob2 /= sum_prob;*/
		   force_region += oj * log(MAX_snake(prob1/MAX_snake(prob2,eps),eps)) * -1;
		   force_r_region += (float)1/(float)m * log(MAX_snake(prob1/MAX_snake(prob2,eps),eps)) * sin(theta) * ( v3.x * oj(0) + v3.y * oj(1) + v3.z * oj(2)) * -1;
		   force_r_region1 += (float)1/(float)m * log(MAX_snake(prob1/MAX_snake(prob2,eps),eps)) * cos(theta) * ( v2.x * oj(0) + v2.y * oj(1) + v2.z * oj(2)) * -1;
		}

		//force_r_region /= m;
        
		//float mag3 = MAX(sqrt(pow(force_region[0],2) + pow(force_region[1],2) + pow(force_region[2],2)),std::numeric_limits<float>::epsilon());
		float mag2 = MAX_snake(sqrt(pow(force_region[0],2) + pow(force_region[1],2) + pow(force_region[2],2) + pow(force_r_region,2) + pow(force_r_region1,2)),std::numeric_limits<float>::epsilon());

		force_region /= mag2;
        force_r_region /= mag2;
		force_r_region1 /= mag2;
		//std::cout<<"force_region:"<<force_region(0)<<","<<force_region(1)<<","<<force_region(2)<<std::endl;
		mfx(j) = force_region(0);
		mfy(j) = force_region(1);
		mfz(j) = force_region(2);
		mfr(j) = force_r_region;
		mfr1(j) = force_r_region1;
	   }
	   else  //2D Tracing
	   {
	    for( int k = -1; k <= 1; k+=2 )
	    {
	      vtemp.x = k * -1 * v1.y;
	      vtemp.y = k * 1 * v1.x;
		  vtemp.z = 0;
		  vtemp.ConvertUnit();

		  vnl_vector<float> oj(3);
		  oj(0) = vtemp.x;
		  oj(1) = vtemp.y;
		  oj(2) = vtemp.z;

		  temp_r_pt.x = x(j) + vnl_Ru(j) * vtemp.x;
		  temp_r_pt.y = y(j) + vnl_Ru(j) * vtemp.y;
		  temp_r_pt.z = 0;
		
		  if( temp_r_pt.check_out_of_range_2D(IM->SM,IM->SN,IM->SZ) )
		  {
              continue;
		  }
		  	
          //ProbImageType::IndexType temp_index; 
		  GradientInterpolatorType::ContinuousIndexType temp_index;
          temp_index[0] = temp_r_pt.x;
          temp_index[1] = temp_r_pt.y;
		  temp_index[2] = temp_r_pt.z;

		   float Ic = I_interpolator->EvaluateAtContinuousIndex(temp_index);
		   float ILc = IL_interpolator->EvaluateAtContinuousIndex(temp_index);
           if( ILc != 0 )
             continue;
           float prob1 = norm_density(Ic, IM->u1, IM->sigma1);
		   float prob2 = norm_density(Ic, IM->u2, IM->sigma2);

		   force_region += oj * log(MAX_snake(prob1/MAX_snake(prob2,eps),eps)) * -1;
		   force_r_region += log(MAX_snake(prob1/MAX_snake(prob2,eps),eps)) * -1;
		 
		}

		float mag2 = MAX_snake(sqrt(pow(force_region[0],2) + pow(force_region[1],2) + pow(force_r_region,2)),std::numeric_limits<float>::epsilon());
		force_region /= mag2;
        force_r_region /= mag2;
		mfx(j) = force_region(0);
		mfy(j) = force_region(1);
		mfr(j) = force_r_region;
	 
	   }
	  }

	   //if( freeze_body )
	   if( tracing_model == 1 ) 
	   {
	   	//if( j >= Cu.GetSize() - N_Active || j <= N_Active ) 
	    if( j == 0 || j == Cu.GetSize()-1 )
	    {
		 GradientPixelType gvf = interpolator->EvaluateAtContinuousIndex(index);
         vfx(j) = gvf[0];
         vfy(j) = gvf[1];
	 	 vfz(j) = gvf[2];
	    }
	   }
	   else
	   {
	     GradientPixelType gvf = interpolator->EvaluateAtContinuousIndex(index);
         vfx(j) = gvf[0];
         vfy(j) = gvf[1];
	 	 vfz(j) = gvf[2];
	   }

	 }

     //normalize the radius forces
	 //mfr.normalize();
	 //std::cout<<"Ru1:";
	// for(int iii = 0; iii < mfr.size(); iii++)
	//	 std::cout<<mfr(iii);
	// std::cout<<std::endl;
	 //mfr = ((mfr - mfr.min_value())/(mfr.max_value()-mfr.min_value()+std::numeric_limits<float>::epsilon()) - 0.5);
	 //mfr /= (mfr.max_value() + std::numeric_limits<double>::epsilon());

	 /*std::cout<<"max_value:"<<mfr.max_value()<<","<<mfr.min_value()<<","<<std::numeric_limits<double>::epsilon()<<std::endl;

	  std::cout<<"mfr:";
     for(int iii = 0; iii < mfr.size(); iii++)
	 	 std::cout<<mfr(iii)<<",";
	 std::cout<<std::endl;*/


	 //automate the selection of stretching force
      float Ic_h = I_interpolator->EvaluateAtContinuousIndex(head_index);
     float prob1_h = norm_density(Ic_h, IM->u1, IM->sigma1);
	 float prob2_h = norm_density(Ic_h, IM->u2, IM->sigma2);
	 stretchingRatio_head = log(MAX_snake(prob1_h/MAX_snake(prob2_h,eps),eps));

	 if( stretchingRatio_head < stretchingRatio )
		 stretchingRatio_head = stretchingRatio;
	 //std::cout<<"stretchingRatio_head:"<<stretchingRatio_head<<std::endl;

     float Ic_t = I_interpolator->EvaluateAtContinuousIndex(tail_index);
     float prob1_t = norm_density(Ic_t, IM->u1, IM->sigma1);
	 float prob2_t = norm_density(Ic_t, IM->u2, IM->sigma2);
	 stretchingRatio_tail = log(MAX_snake(prob1_t/MAX_snake(prob2_t,eps),eps));
	 //std::cout<<"stretchingRatio_tail:"<<stretchingRatio_tail<<std::endl;
	 if( stretchingRatio_tail < stretchingRatio )
		 stretchingRatio_tail = stretchingRatio;

	 Vector3D tailForce( (x(0) - x(2)), (y(0) - y(2)), (z(0) - z(2)) );
	 tailForce.ConvertUnit();

	Vector3D tForce;
       tForce = tailForce;

	 int end = x.size() - 1;
     Vector3D headForce( (x(end) - x(end-2)), (y(end) - y(end-2)), (z(end) - z(end-2)) );
	 headForce.ConvertUnit();
     

	Vector3D hForce;
       hForce = headForce;
	

	 //check for tail leakage and self-intersection
	LabelImageType::IndexType tail_index_temp;
	tail_index_temp[0] = tail_index[0];
    tail_index_temp[1] = tail_index[1];
	tail_index_temp[2] = tail_index[2];
	if( IM->IL_Tracing->GetPixel(tail_index_temp) == 1 )
	 {
	     tForce.x = 0;
		 tForce.y = 0;
		 tForce.z = 0;
		 mfx(0) = 0;
		 mfy(0) = 0;
		 mfz(0) = 0;
		 mfr(0) = 0;
		 mfr1(0) = 0;
	 }

	 //if( tracing_model == 2 || tracing_model == 3 )
	 if( 1 )
	 {
	     /*//extend the tail index with its radius
		 GradientInterpolatorType::ContinuousIndexType temp_index;
		 temp_index[0] = tail_index[0] + tailForce.x * vnl_Ru(0);
		 temp_index[1] = tail_index[1] + tailForce.y * vnl_Ru(0);
		 temp_index[2] = tail_index[2] + tailForce.z * vnl_Ru(0);
		 if( ceil((float)temp_index[0]) >= SM || ceil((float)temp_index[1]) >= SN || ceil((float)temp_index[2]) >= SZ
		 || ceil((float)temp_index[0]) < 0 || ceil((float)tail_index[1]) < 0 || ceil((float)temp_index[2]) < 0 )
	     {
			 temp_index = tail_index;
		 }*/
		
	    float IT =  I_interpolator->EvaluateAtContinuousIndex(tail_index);

		bool leakage = false;
		if( IM->u2 + sigma_ratio * IM->sigma2 >  IM->u1 )
		{
           leakage =  norm_density(IT, IM->u1, IM->sigma1) < norm_density(IT, IM->u2, IM->sigma2);
		}
		else
		{   
		   leakage = IT <= IM->u2 + sigma_ratio * IM->sigma2;
		}
		if( leakage)
		{
	     tForce.x = 0;
		 tForce.y = 0;
		 tForce.z = 0;
		 mfx(0) = 0;
		 mfy(0) = 0;
		 mfz(0) = 0;
		 mfr(0) = 0;
		 mfr1(0) = 0;
		 //vfx(0) = 0;
		 //vfy(0) = 0;
		 //vfz(0) = 0;
		}
	 }

     //check for tail collision
	 int draw_force_tail = 1;
	
	 bool tail_merging = Check_Tail_Collision(tail_index_temp,collision_dist,minimum_length,automatic_merging,max_angle,snake_id);

	 if( tail_merging )
		return;

	 if( collision == 1 || collision == 3 )
	 {
		x(0) = Cu.Pt[0].x;
		y(0) = Cu.Pt[0].y;
		z(0) = Cu.Pt[0].z;
		tail_index[0] = x(0);
		tail_index[1] = y(0);
		tail_index[2] = z(0);
		vnl_Ru(0) = Ru[0];
	    draw_force_tail = 0;
	 }

	 //check for boundary condition
	 int boundary_tail = 0;
	if( IM->SZ == 1 )
	{
	 if( ceil((float)tail_index[0]) >= SM-hit_boundary_dist || ceil((float)tail_index[1]) >= SN-hit_boundary_dist
		 || ceil((float)tail_index[0]) < hit_boundary_dist || ceil((float)tail_index[1]) < hit_boundary_dist )
	 {
	    draw_force_tail = 0;
	    boundary_tail = 1;
	 }
	}
	else
	{
	 if( ceil((float)tail_index[0]) >= SM-hit_boundary_dist || ceil((float)tail_index[1]) >= SN-hit_boundary_dist || ceil((float)tail_index[2]) >= SZ-hit_boundary_dist
		 || ceil((float)tail_index[0]) < hit_boundary_dist || ceil((float)tail_index[1]) < hit_boundary_dist || ceil((float)tail_index[2]) < hit_boundary_dist )
	 {
	    draw_force_tail = 0;
	    boundary_tail = 1;
	 }
	}

     //check for head leakage and self_intersection
	 LabelImageType::IndexType head_index_temp;
	 head_index_temp[0] = head_index[0];
     head_index_temp[1] = head_index[1];
	 head_index_temp[2] = head_index[2];
	 if( IM->IL_Tracing->GetPixel(head_index_temp) == 1)
	 {
	 	hForce.x = 0;
		hForce.y = 0;
		hForce.z = 0;
		mfx(end) = 0;
		mfy(end) = 0;
		mfz(end) = 0;
		mfr(end) = 0;
		mfr1(end) = 0;
	 }

	 //if( tracing_model == 2 || tracing_model == 3 )
	 if( 1 ) 
	 {
		/*//extend the tail index with its radius
		GradientInterpolatorType::ContinuousIndexType temp_index;
		temp_index[0] = head_index[0] + headForce.x * vnl_Ru(end);
		temp_index[1] = head_index[1] + headForce.y * vnl_Ru(end);
		temp_index[2] = head_index[2] + headForce.z * vnl_Ru(end);
		 if( ceil((float)temp_index[0]) >= SM || ceil((float)temp_index[1]) >= SN || ceil((float)temp_index[2]) >= SZ
		 || ceil((float)temp_index[0]) < 0 || ceil((float)tail_index[1]) < 0 || ceil((float)temp_index[2]) < 0 )
	     {
			 temp_index = head_index;
		 }*/

	    float IH =  I_interpolator->EvaluateAtContinuousIndex(head_index);

		bool leakage = false;
		if( IM->u2 + sigma_ratio * IM->sigma2 >  IM->u1 )
		{
           leakage =  norm_density(IH, IM->u1, IM->sigma1) < norm_density(IH, IM->u2, IM->sigma2);
		}
		else
		{   
		   leakage = IH <= IM->u2 + sigma_ratio * IM->sigma2;
		}
		if( leakage)
		{
		 hForce.x = 0;
		 hForce.y = 0;
		 hForce.z = 0;
		 mfx(end) = 0;
		 mfy(end) = 0;
		 mfz(end) = 0;
		 mfr(end) = 0;
		 mfr1(end) = 0;
		 //vfx(end) = 0;
		 //vfy(end) = 0;
	     //vfz(end) = 0;
		}
	 }

	 //check for head collision
	 int draw_force_head = 1;

     bool head_merging = Check_Head_Collision(head_index_temp,collision_dist,minimum_length,automatic_merging,max_angle,snake_id);

	 if( head_merging )
		return;

	 if( collision == 2 || collision == 3 )
	 {
	    x(end) = Cu.Pt[end].x;
		y(end) = Cu.Pt[end].y;
		z(end) = Cu.Pt[end].z;
	    head_index[0] = x(end);
		head_index[1] = y(end);
		head_index[2] = z(end);
		vnl_Ru(end) = Ru[end];
	    draw_force_head = 0;
	 }

	 //check for boundary condition
	 int boundary_head = 0;
    if( IM->SZ == 1 )
	{
	 if( ceil((float)head_index[0]) >= SM-hit_boundary_dist || ceil((float)head_index[1]) >= SN-hit_boundary_dist
		 || ceil((float)head_index[0]) < hit_boundary_dist || ceil((float)head_index[1]) < hit_boundary_dist )
	 {
	    draw_force_head = 0;
	    boundary_head = 1;
	 }
	}
	else
	{
	 if( ceil((float)head_index[0]) >= SM-hit_boundary_dist || ceil((float)head_index[1]) >= SN-hit_boundary_dist || ceil((float)head_index[2]) >= SZ-hit_boundary_dist
		 || ceil((float)head_index[0]) < hit_boundary_dist || ceil((float)head_index[1]) < hit_boundary_dist || ceil((float)head_index[2]) < hit_boundary_dist )
	 {
	    draw_force_head = 0;
	    boundary_head = 1;
	 }
	}

    /*if( vnl_Ru(0) < 0.5 )
	{
		tForce.x = 0;
	    tForce.y = 0;
		tForce.z = 0;
	}
	if( vnl_Ru(end) < 0.5 )
	{
		hForce.x = 0;
	    hForce.y = 0;
		hForce.z = 0;
	} */


    vfx(0) = (vfx(0) + stretchingRatio * tForce.x) * draw_force_tail;
    vfy(0) = (vfy(0) + stretchingRatio * tForce.y) * draw_force_tail;
    vfz(0) = (vfz(0) + stretchingRatio * tForce.z) * draw_force_tail;  
    mfx(0) = mfx(0) * draw_force_tail;
    mfy(0) = mfy(0) * draw_force_tail;
	mfz(0) = mfz(0) * draw_force_tail;
	mfr(0) = mfr(0) * draw_force_tail;
    mfr1(0) = mfr1(0) * draw_force_tail;

    vfx(end) = (vfx(end) + stretchingRatio * hForce.x) * draw_force_head;
    vfy(end) = (vfy(end) + stretchingRatio * hForce.y) * draw_force_head;
    vfz(end) = (vfz(end) + stretchingRatio * hForce.z) * draw_force_head;
    mfx(end) = mfx(end) * draw_force_head;
    mfy(end) = mfy(end) * draw_force_head;
	mfz(end) = mfz(end) * draw_force_head;
	mfr(end) = mfr(end) * draw_force_head;
    mfr1(end) = mfr1(end) * draw_force_head;

	//freeze the radius when collision occurs
	float r_head, r_tail, r_head1, r_tail1;
	r_head = r_tail = r_tail1 = r_head1 = 0;
	if(collision == 1 || collision == 3)
	{
	   r_tail = vnl_Ru(0);
	   r_tail1 = vnl_Ru1(0);
	}
	else if( collision == 2 || collision == 3)
	{
	   r_head = vnl_Ru(end);
	   r_head1 = vnl_Ru1(end);
	}

    x = (invAI * ( x * gamma +  vfx - mfx));
    y = (invAI * ( y * gamma +  vfy - mfy));

   if( IM->SZ != 1)
    z = (invAI * ( z * gamma +  vfz - mfz));

   if( tracing_model != 0 )
   {
	vnl_Ru = (invAI * ( vnl_Ru * gamma  - mfr));
	vnl_Ru1 = (invAI * ( vnl_Ru1 * gamma  - mfr1));
   }

	if(collision == 1 || collision == 3)
	{
	   x(0) = tail_index[0];
	   y(0) = tail_index[1];
	   z(0) = tail_index[2];
	   vnl_Ru(0) = r_tail;
	   vnl_Ru1(0) = r_tail1;
	}
	else if( collision == 2 || collision == 3)
	{
	   x(end) = head_index[0];
	   y(end) = head_index[1];
	   z(end) = head_index[2];
	   vnl_Ru(end) = r_head;
	   vnl_Ru1(end) = r_head1;
	}
	
	for( unsigned int k = 0; k < x.size(); k++ )
	{
		//freeze tail or head part when collision happens
	  if( freeze_body )
	  {
	   if( collision == 1 && k < N/2 )
		   continue;
	   if( collision == 2 && k > N/2 )
		   continue;
	  }

	    Cu.Pt[k].x = x(k);
        Cu.Pt[k].y = y(k);
		Cu.Pt[k].z = z(k);
		Cu.Pt[k].check_out_of_range_3D(SM,SN,SZ);
	}

     if( boundary_head == 1 && boundary_tail == 1)
	 {
	   hit_boundary = true;
	   break;
	 }
  
  } 
   
   //check for NaN
   for( int i = 0; i < Cu.GetSize(); i++ )
   {
     if( isnan(Cu.Pt[i].x) || isnan(Cu.Pt[i].y) || isnan(Cu.Pt[i].z) )
		 return;
   }

   /*PointList3D Cu_temp;
   Ru.clear();

   for( int i = 0; i < vnl_Ru.size(); i++ )
   {
      if( vnl_Ru(i) > 0 )
	  {
	    Cu_temp.AddPt(Cu.Pt[i]);
		Ru.push_back(vnl_Ru(i));
	  }
   }

   Cu = Cu_temp;

   if( Cu.NP < 3 )
   {
        //Ru.clear();
		//for( int i = 0; i < vnl_Ru.size(); i++ )
		//	Ru.push_back(vnl_Ru(i));
	   return;
   }


   Ru = Cu.curveinterp_4D((float)pt_distance,Ru);*/

   //resampling
   Cu_Backup = Cu;
   vnl_Ru = Cu.curveinterp_4D((float)pt_distance, vnl_Ru);
   vnl_Ru1 = Cu_Backup.curveinterp_4D((float)pt_distance, vnl_Ru1);
   Ru.clear();
   Ru1.clear();
  
   for( int i = 0; i < vnl_Ru.size(); i++ )
   {
     if( vnl_Ru(i) < 0)
	 	 vnl_Ru(i) = 1;
	 Ru.push_back(vnl_Ru(i));
	 if( vnl_Ru1(i) < 0)
	 	 vnl_Ru1(i) = 1;
	 Ru1.push_back(vnl_Ru1(i));
   }
   //std::cout<<"size"<<Cu.NP<<","<<Ru.size()<<std::endl;

}

void SnakeClass::OpenSnakeStretch_4D(float alpha, int ITER, int pt_distance, float beta, float kappa, float gamma, 
                                float stretchingRatio, int collision_dist, int minimum_length, 
								bool automatic_merging, int max_angle, bool freeze_body, int s_force, int snake_id, 
								int tracing_model, int coding_method, float sigma_ratio, int border)
{

   int hit_boundary_dist = border;
   float pi = 3.1415926;
   int m = 8;
   int band_width = 0;

   float eps = std::numeric_limits<float>::epsilon();

   float stretchingRatio_head = stretchingRatio;
   float stretchingRatio_tail = stretchingRatio;

   //when collision happened for both tail and head, stop stretching
   if( collision == 3)
	   return;

   typedef itk::VectorLinearInterpolateImageFunction< 
                       GradientImageType, float >  GradientInterpolatorType;
   GradientInterpolatorType::Pointer interpolator = GradientInterpolatorType::New();
   interpolator->SetInputImage(IM->IGVF);
   /*GradientInterpolatorType::Pointer interpolator_V1 = GradientInterpolatorType::New();
   interpolator_V1->SetInputImage(IM->V1);*/

   typedef itk::LinearInterpolateImageFunction< 
                       ProbImageType, float>  InterpolatorType;

   typedef itk::LinearInterpolateImageFunction< 
                       ImageType, float>  InterpolatorType1;

   typedef itk::LinearInterpolateImageFunction< 
                       LabelImageType, float>  InterpolatorType2;

   InterpolatorType1::Pointer I_interpolator = InterpolatorType1::New();
   I_interpolator->SetInputImage(IM->I);

   InterpolatorType2::Pointer IL_interpolator = InterpolatorType2::New();
   IL_interpolator->SetInputImage(IM->IL);

   int SM = IM->SM;
   int SN = IM->SN;
   int SZ = IM->SZ;
  
   int N = Cu.GetSize();
   vnl_matrix<float> A = makeOpenA(alpha, beta, N);
   vnl_matrix<float> I(N,N);
   I.set_identity();
   vnl_matrix<float> invAI = matrix_inverse( A + I * gamma, N);



   vnl_vector<float> vnl_Ru(N);
   vnl_Ru.fill(0);
   for( int j = 0; j < N; j++ )
   {
     vnl_Ru(j) = Ru[j];
   }

   //evolve 4D snake
   for( int iter = 0; iter < ITER; iter++ )
   {
     vnl_vector<float> vfx(N);
     vnl_vector<float> vfy(N);
     vnl_vector<float> vfz(N);

     vnl_vector<float> x(N);
     vnl_vector<float> y(N);
     vnl_vector<float> z(N);

     vnl_vector<float> mfx(N);
     vnl_vector<float> mfy(N);
     vnl_vector<float> mfz(N);
     vnl_vector<float> mfr(N);

     vfx.fill(0);
     vfy.fill(0);
     vfz.fill(0);

     mfx.fill(0);
     mfy.fill(0);
     mfz.fill(0);
     mfr.fill(0);

     //GradientImageType::IndexType head_index;
     //GradientImageType::IndexType tail_index;
     GradientInterpolatorType::ContinuousIndexType head_index;
     GradientInterpolatorType::ContinuousIndexType tail_index;

     Vector3D v1,v2,v3, vtemp;
     Point3D temp_r_pt;

     for( int j = 0; j < Cu.GetSize(); j++ )
     {
        x(j) = Cu.GetPt(j).x;
        y(j) = Cu.GetPt(j).y;
        z(j) = Cu.GetPt(j).z;


        //GradientImageType::IndexType index;
        GradientInterpolatorType::ContinuousIndexType index;
        index[0] = (x(j));
        index[1] = (y(j));
        index[2] = (z(j));

       if( j == 0 )
       {
         tail_index[0] = Cu.GetPt(j).x;
         tail_index[1] = Cu.GetPt(j).y;
         tail_index[2] = Cu.GetPt(j).z;

         /*//project field flow to the normal plane
         GradientPixelType gvf = interpolator->EvaluateAtIndex(index);
         float dot_product = gvf[0] * evfx(j) + gvf[1] * evfy(j) + gvf[2] * evfz(j);
         vfx(j) = gvf[0] - dot_product * evfx(j);
         vfy(j) = gvf[1] - dot_product * evfy(j);;
         vfz(j) = gvf[2] - dot_product * evfz(j);;
         continue; */
       }
       if( j == Cu.GetSize() - 1 )
       {
         head_index[0] = Cu.GetPt(j).x;
         head_index[1] = Cu.GetPt(j).y;
         head_index[2] = Cu.GetPt(j).z;


         /*//project field flow to the normal plane
         GradientPixelType gvf = interpolator->EvaluateAtIndex(index);
         float dot_product = gvf[0] * evfx(j) + gvf[1] * evfy(j) + gvf[2] * evfz(j);
         vfx(j) = gvf[0] - dot_product * evfx(j);
         vfy(j) = gvf[1] - dot_product * evfy(j);;
         vfz(j) = gvf[2] - dot_product * evfz(j);;
         continue;*/
       }

       //std::cout<<"check point 0"<<std::endl;
       //compute the radius force
       if( j == 0 )
       {
          v1.x = Cu.Pt[0].x - Cu.Pt[1].x;
          v1.y = Cu.Pt[0].y - Cu.Pt[1].y;
          v1.z = Cu.Pt[0].z - Cu.Pt[1].z;
          v1.ConvertUnit();
       }
       else
       {
          v1.x = Cu.Pt[j].x - Cu.Pt[j-1].x;
          v1.y = Cu.Pt[j].y - Cu.Pt[j-1].y;
          v1.z = Cu.Pt[j].z - Cu.Pt[j-1].z;
          v1.ConvertUnit();
       }

       /*v2.x = -v1.z;
       v2.y = 0;
       v2.z = v1.x;
       v2.ConvertUnit();
       v3.x = 1;
       v3.y = -(pow(v1.x,2) + pow(v1.z,2))/(v1.x*v1.y + std::numeric_limits<float>::epsilon());
       v3.z = v1.z/(v1.x + std::numeric_limits<float>::epsilon());
       v3.ConvertUnit();*/

       v2.x = -1 * v1.x * v1.z;
       v2.y = -1 * v1.y * v1.z;
       v2.z = pow(v1.x,2) + pow(v1.y,2);
       v2.ConvertUnit();
       v3.x = -1 * v1.y;
       v3.y = v1.x;
       v3.z = 0;
       v3.ConvertUnit();

       float force_r_region = 0;
       vnl_vector<float> force_region(3);
       force_region.fill(0);

      //if( vnl_Ru(j) >= 0.1 )
      if( tracing_model != 0 )
      {
       if( IM->SZ != 1 )
       {
        for( int k = 0; k < m; k++ )
        {
          float theta = (2 * pi * k)/m;
          vtemp.x = vnl_Ru(j) * v2.x * cos(theta) + vnl_Ru(j) * v3.x * sin(theta);
          vtemp.y = vnl_Ru(j) * v2.y * cos(theta) + vnl_Ru(j) * v3.y * sin(theta);
          vtemp.z = vnl_Ru(j) * v2.z * cos(theta) + vnl_Ru(j) * v3.z * sin(theta);

          temp_r_pt.x = x(j) + vtemp.x;
          temp_r_pt.y = y(j) + vtemp.y;
          temp_r_pt.z = z(j) + vtemp.z;

          vnl_vector<float> oj(3);

          vtemp.ConvertUnit();

          oj(0) = vtemp.x;
          oj(1) = vtemp.y;
          oj(2) = vtemp.z;

          //if( isinf( temp_r_pt.x ) || isinf( temp_r_pt.y ) || isinf( temp_r_pt.z ) )
          //{
          //  continue;
          //}

          if( temp_r_pt.check_out_of_range_3D(IM->SM,IM->SN,IM->SZ) )
            continue;

          //ProbImageType::IndexType temp_index;
          GradientInterpolatorType::ContinuousIndexType temp_index;
          temp_index[0] = temp_r_pt.x;
          temp_index[1] = temp_r_pt.y;
          temp_index[2] = temp_r_pt.z;

          /*GradientPixelType gvf = interpolator->EvaluateAtIndex(temp_index);
          vnl_vector<float> nj(3);
          nj(0) = gvf[0];
          nj(1) = gvf[1];
          nj(2) = gvf[2];*/

           /*float Ic = 0.0;
           for( int band_width = -1; band_width <= 1; band_width++ )
           {
               GradientInterpolatorType::ContinuousIndexType temp_index1;
               temp_index1[0] = x(j) + (vnl_Ru(j) + band_width) * ( v2.x * cos(theta) +  v3.x * sin(theta) );
               temp_index1[1] = y(j) + (vnl_Ru(j) + band_width) * ( v2.y * cos(theta) +  v3.y * sin(theta) );
               temp_index1[2] = z(j) + (vnl_Ru(j) + band_width) * ( v2.z * cos(theta) +  v3.z * sin(theta) );
               Ic += I_interpolator->EvaluateAtContinuousIndex(temp_index1)/3;
           }*/

           float Ic = I_interpolator->EvaluateAtContinuousIndex(temp_index);

           float ILc = IL_interpolator->EvaluateAtContinuousIndex(temp_index);
           //if( ILc != 0 )
           //  continue;
           if( ILc != 0 )
               Ic = IM->u2;

           //force_region += oj * log(fabs(Ic - IM->u1)/(fabs(Ic - IM->u2)+std::numeric_limits<float>::epsilon()) + std::numeric_limits<float>::epsilon());
           //force_r_region += log(fabs(Ic - IM->u1)/(fabs(Ic - IM->u2)+std::numeric_limits<float>::epsilon()) + std::numeric_limits<float>::epsilon());
           float prob1 = norm_density(Ic, IM->u1, IM->sigma1);
           float prob2 = norm_density(Ic, IM->u2, IM->sigma2);
           /*float sum_prob = prob1 + prob2;
           prob1 /= sum_prob;
           prob2 /= sum_prob;*/
           force_region += oj * log(MAX_snake(prob1/MAX_snake(prob2,eps),eps)) * -1;
           force_r_region += (float)1/(float)m * log(MAX_snake(prob1/MAX_snake(prob2,eps),eps)) * -1;
        }

        //force_r_region /= m;

        //float mag3 = MAX(sqrt(pow(force_region[0],2) + pow(force_region[1],2) + pow(force_region[2],2)),std::numeric_limits<float>::epsilon());
        float mag2 = MAX_snake(sqrt(pow(force_region[0],2) + pow(force_region[1],2) + pow(force_region[2],2) + pow(force_r_region,2)),std::numeric_limits<float>::epsilon());

        force_region /= mag2;
        force_r_region /= mag2;
        //std::cout<<"force_region:"<<force_region(0)<<","<<force_region(1)<<","<<force_region(2)<<std::endl;
        mfx(j) = force_region(0);
        mfy(j) = force_region(1);
        mfz(j) = force_region(2);
        mfr(j) = force_r_region;
       }
       else  //2D Tracing
       {
        for( int k = -1; k <= 1; k+=2 )
        {
          vtemp.x = k * -1 * v1.y;
          vtemp.y = k * 1 * v1.x;
          vtemp.z = 0;
          vtemp.ConvertUnit();

          vnl_vector<float> oj(3);
          oj(0) = vtemp.x;
          oj(1) = vtemp.y;
          oj(2) = vtemp.z;

          temp_r_pt.x = x(j) + vnl_Ru(j) * vtemp.x;
          temp_r_pt.y = y(j) + vnl_Ru(j) * vtemp.y;
          temp_r_pt.z = 0;

          if( temp_r_pt.check_out_of_range_2D(IM->SM,IM->SN,IM->SZ) )
          {
              continue;
          }

          //ProbImageType::IndexType temp_index;
          GradientInterpolatorType::ContinuousIndexType temp_index;
          temp_index[0] = temp_r_pt.x;
          temp_index[1] = temp_r_pt.y;
          temp_index[2] = temp_r_pt.z;

           float Ic = I_interpolator->EvaluateAtContinuousIndex(temp_index);
           float ILc = IL_interpolator->EvaluateAtContinuousIndex(temp_index);
           if( ILc != 0 )
             continue;
           float prob1 = norm_density(Ic, IM->u1, IM->sigma1);
           float prob2 = norm_density(Ic, IM->u2, IM->sigma2);

           force_region += oj * log(MAX_snake(prob1/MAX_snake(prob2,eps),eps)) * -1;
           force_r_region += log(MAX_snake(prob1/MAX_snake(prob2,eps),eps)) * -1;

        }

        float mag2 = MAX_snake(sqrt(pow(force_region[0],2) + pow(force_region[1],2) + pow(force_r_region,2)),std::numeric_limits<float>::epsilon());
        force_region /= mag2;
        force_r_region /= mag2;
        mfx(j) = force_region(0);
        mfy(j) = force_region(1);
        mfr(j) = force_r_region;

       }
      }

       //if( freeze_body )
       if( tracing_model == 1 )
       {
        //if( j >= Cu.GetSize() - N_Active || j <= N_Active )
        if( j == 0 || j == Cu.GetSize()-1 )
        {
         GradientPixelType gvf = interpolator->EvaluateAtContinuousIndex(index);
         vfx(j) = gvf[0];
         vfy(j) = gvf[1];
         vfz(j) = gvf[2];
        }
       }
       else
       {
         GradientPixelType gvf = interpolator->EvaluateAtContinuousIndex(index);
         vfx(j) = gvf[0];
         vfy(j) = gvf[1];
         vfz(j) = gvf[2];
       }

     }

     //normalize the radius forces
     //mfr.normalize();
     //std::cout<<"Ru1:";
    // for(int iii = 0; iii < mfr.size(); iii++)
    //	 std::cout<<mfr(iii);
    // std::cout<<std::endl;
     //mfr = ((mfr - mfr.min_value())/(mfr.max_value()-mfr.min_value()+std::numeric_limits<float>::epsilon()) - 0.5);
     //mfr /= (mfr.max_value() + std::numeric_limits<double>::epsilon());

     /*std::cout<<"max_value:"<<mfr.max_value()<<","<<mfr.min_value()<<","<<std::numeric_limits<double>::epsilon()<<std::endl;

      std::cout<<"mfr:";
     for(int iii = 0; iii < mfr.size(); iii++)
         std::cout<<mfr(iii)<<",";
     std::cout<<std::endl;*/


     //automate the selection of stretching force
      float Ic_h = I_interpolator->EvaluateAtContinuousIndex(head_index);
     float prob1_h = norm_density(Ic_h, IM->u1, IM->sigma1);
     float prob2_h = norm_density(Ic_h, IM->u2, IM->sigma2);
     stretchingRatio_head = log(MAX_snake(prob1_h/MAX_snake(prob2_h,eps),eps));

     if( stretchingRatio_head < stretchingRatio )
         stretchingRatio_head = stretchingRatio;
     //std::cout<<"stretchingRatio_head:"<<stretchingRatio_head<<std::endl;

     float Ic_t = I_interpolator->EvaluateAtContinuousIndex(tail_index);
     float prob1_t = norm_density(Ic_t, IM->u1, IM->sigma1);
     float prob2_t = norm_density(Ic_t, IM->u2, IM->sigma2);
     stretchingRatio_tail = log(MAX_snake(prob1_t/MAX_snake(prob2_t,eps),eps));
     if( stretchingRatio_tail < stretchingRatio )
         stretchingRatio_tail = stretchingRatio;
     //std::cout<<"stretchingRatio_tail:"<<stretchingRatio_tail<<std::endl;

     Vector3D tailForce( (x(0) - x(2)), (y(0) - y(2)), (z(0) - z(2)) );
     tailForce.ConvertUnit();

    Vector3D tForce;
       tForce = tailForce;

     int end = x.size() - 1;
     Vector3D headForce( (x(end) - x(end-2)), (y(end) - y(end-2)), (z(end) - z(end-2)) );
     headForce.ConvertUnit();


    Vector3D hForce;
       hForce = headForce;


     //check for tail leakage and self-intersection
    LabelImageType::IndexType tail_index_temp;
    tail_index_temp[0] = tail_index[0];
    tail_index_temp[1] = tail_index[1];
    tail_index_temp[2] = tail_index[2];
    if( IM->IL_Tracing->GetPixel(tail_index_temp) == 1 )
     {
         tForce.x = 0;
         tForce.y = 0;
         tForce.z = 0;
         mfx(0) = 0;
         mfy(0) = 0;
         mfz(0) = 0;
         mfr(0) = 0;
     }

     //if( tracing_model == 2 || tracing_model == 3 )
     if( 1 )
     {
         /*//extend the tail index with its radius
         GradientInterpolatorType::ContinuousIndexType temp_index;
         temp_index[0] = tail_index[0] + tailForce.x * vnl_Ru(0);
         temp_index[1] = tail_index[1] + tailForce.y * vnl_Ru(0);
         temp_index[2] = tail_index[2] + tailForce.z * vnl_Ru(0);
         if( ceil((float)temp_index[0]) >= SM || ceil((float)temp_index[1]) >= SN || ceil((float)temp_index[2]) >= SZ
         || ceil((float)temp_index[0]) < 0 || ceil((float)tail_index[1]) < 0 || ceil((float)temp_index[2]) < 0 )
         {
             temp_index = tail_index;
         }*/

        float IT =  I_interpolator->EvaluateAtContinuousIndex(tail_index);

        bool leakage = false;
        if( IM->u2 + sigma_ratio * IM->sigma2 >  IM->u1 )
        {
           leakage =  norm_density(IT, IM->u1, IM->sigma1) < norm_density(IT, IM->u2, IM->sigma2);
        }
        else
        {
           leakage = IT <= IM->u2 + sigma_ratio * IM->sigma2;
        }
        if( leakage)
        {
         tForce.x = 0;
         tForce.y = 0;
         tForce.z = 0;
         mfx(0) = 0;
         mfy(0) = 0;
         mfz(0) = 0;
         mfr(0) = 0;
         //vfx(0) = 0;
         //vfy(0) = 0;
         //vfz(0) = 0;
        }
     }

     //check for tail collision
     int draw_force_tail = 1;

     bool tail_merging = Check_Tail_Collision(tail_index_temp,collision_dist,minimum_length,automatic_merging,max_angle,snake_id);

     if( tail_merging )
        return;

     if( collision == 1 || collision == 3 )
     {
        x(0) = Cu.Pt[0].x;
        y(0) = Cu.Pt[0].y;
        z(0) = Cu.Pt[0].z;
        tail_index[0] = x(0);
        tail_index[1] = y(0);
        tail_index[2] = z(0);
        vnl_Ru(0) = Ru[0];
        draw_force_tail = 0;
     }

     //check for boundary condition
     int boundary_tail = 0;
    if( IM->SZ == 1 )
    {
     if( ceil((float)tail_index[0]) >= SM-hit_boundary_dist || ceil((float)tail_index[1]) >= SN-hit_boundary_dist
         || ceil((float)tail_index[0]) < hit_boundary_dist || ceil((float)tail_index[1]) < hit_boundary_dist )
     {
        draw_force_tail = 0;
        boundary_tail = 1;
     }
    }
    else
    {
     if( ceil((float)tail_index[0]) >= SM-hit_boundary_dist || ceil((float)tail_index[1]) >= SN-hit_boundary_dist || ceil((float)tail_index[2]) >= SZ-hit_boundary_dist
         || ceil((float)tail_index[0]) < hit_boundary_dist || ceil((float)tail_index[1]) < hit_boundary_dist || ceil((float)tail_index[2]) < hit_boundary_dist )
     {
        draw_force_tail = 0;
        boundary_tail = 1;
     }
    }

     //check for head leakage and self_intersection
     LabelImageType::IndexType head_index_temp;
     head_index_temp[0] = head_index[0];
     head_index_temp[1] = head_index[1];
     head_index_temp[2] = head_index[2];
     if( IM->IL_Tracing->GetPixel(head_index_temp) == 1)
     {
        hForce.x = 0;
        hForce.y = 0;
        hForce.z = 0;
        mfx(end) = 0;
        mfy(end) = 0;
        mfz(end) = 0;
        mfr(end) = 0;
     }

     //if( tracing_model == 2 || tracing_model == 3 )
     if( 1 )
     {
        /*//extend the tail index with its radius
        GradientInterpolatorType::ContinuousIndexType temp_index;
        temp_index[0] = head_index[0] + headForce.x * vnl_Ru(end);
        temp_index[1] = head_index[1] + headForce.y * vnl_Ru(end);
        temp_index[2] = head_index[2] + headForce.z * vnl_Ru(end);
         if( ceil((float)temp_index[0]) >= SM || ceil((float)temp_index[1]) >= SN || ceil((float)temp_index[2]) >= SZ
         || ceil((float)temp_index[0]) < 0 || ceil((float)tail_index[1]) < 0 || ceil((float)temp_index[2]) < 0 )
         {
             temp_index = head_index;
         }*/

        float IH =  I_interpolator->EvaluateAtContinuousIndex(head_index);

        bool leakage = false;
        if( IM->u2 + sigma_ratio * IM->sigma2 >  IM->u1 )
        {
           leakage =  norm_density(IH, IM->u1, IM->sigma1) < norm_density(IH, IM->u2, IM->sigma2);
        }
        else
        {
           leakage = IH <= IM->u2 + sigma_ratio * IM->sigma2;
        }
        if( leakage)
        {
         hForce.x = 0;
         hForce.y = 0;
         hForce.z = 0;
         mfx(end) = 0;
         mfy(end) = 0;
         mfz(end) = 0;
         mfr(end) = 0;
         //vfx(end) = 0;
         //vfy(end) = 0;
         //vfz(end) = 0;
        }
     }

     //check for head collision
     int draw_force_head = 1;

     bool head_merging = Check_Head_Collision(head_index_temp,collision_dist,minimum_length,automatic_merging,max_angle,snake_id);

     if( head_merging )
        return;

     if( collision == 2 || collision == 3 )
     {
        x(end) = Cu.Pt[end].x;
        y(end) = Cu.Pt[end].y;
        z(end) = Cu.Pt[end].z;
        head_index[0] = x(end);
        head_index[1] = y(end);
        head_index[2] = z(end);
        vnl_Ru(end) = Ru[end];
        draw_force_head = 0;
     }

     //check for boundary condition
     int boundary_head = 0;
    if( IM->SZ == 1 )
    {
     if( ceil((float)head_index[0]) >= SM-hit_boundary_dist || ceil((float)head_index[1]) >= SN-hit_boundary_dist
         || ceil((float)head_index[0]) < hit_boundary_dist || ceil((float)head_index[1]) < hit_boundary_dist )
     {
        draw_force_head = 0;
        boundary_head = 1;
     }
    }
    else
    {
     if( ceil((float)head_index[0]) >= SM-hit_boundary_dist || ceil((float)head_index[1]) >= SN-hit_boundary_dist || ceil((float)head_index[2]) >= SZ-hit_boundary_dist
         || ceil((float)head_index[0]) < hit_boundary_dist || ceil((float)head_index[1]) < hit_boundary_dist || ceil((float)head_index[2]) < hit_boundary_dist )
     {
        draw_force_head = 0;
        boundary_head = 1;
     }
    }

    /*if( vnl_Ru(0) < 0.5 )
    {
        tForce.x = 0;
        tForce.y = 0;
        tForce.z = 0;
    }
    if( vnl_Ru(end) < 0.5 )
    {
        hForce.x = 0;
        hForce.y = 0;
        hForce.z = 0;
    } */


    vfx(0) = (vfx(0) + stretchingRatio * tForce.x) * draw_force_tail;
    vfy(0) = (vfy(0) + stretchingRatio * tForce.y) * draw_force_tail;
    vfz(0) = (vfz(0) + stretchingRatio * tForce.z) * draw_force_tail;
    mfx(0) = mfx(0) * draw_force_tail;
    mfy(0) = mfy(0) * draw_force_tail;
    mfz(0) = mfz(0) * draw_force_tail;
    mfr(0) = mfr(0) * draw_force_tail;

    vfx(end) = (vfx(end) + stretchingRatio * hForce.x) * draw_force_head;
    vfy(end) = (vfy(end) + stretchingRatio * hForce.y) * draw_force_head;
    vfz(end) = (vfz(end) + stretchingRatio * hForce.z) * draw_force_head;
    mfx(end) = mfx(end) * draw_force_head;
    mfy(end) = mfy(end) * draw_force_head;
    mfz(end) = mfz(end) * draw_force_head;
    mfr(end) = mfr(end) * draw_force_head;

    //freeze the radius when collision occurs
    float r_head, r_tail;
    r_head = r_tail = 0;
    if(collision == 1 || collision == 3)
    {
       r_tail = vnl_Ru(0);
    }
    else if( collision == 2 || collision == 3)
    {
       r_head = vnl_Ru(end);
    }

    x = (invAI * ( x * gamma +  vfx - mfx));
    y = (invAI * ( y * gamma +  vfy - mfy));

   if( IM->SZ != 1)
    z = (invAI * ( z * gamma +  vfz - mfz));

   if( tracing_model != 0 )
   {
    vnl_Ru = (invAI * ( vnl_Ru * gamma  - mfr));
   }

    if(collision == 1 || collision == 3)
    {
       x(0) = tail_index[0];
       y(0) = tail_index[1];
       z(0) = tail_index[2];
       vnl_Ru(0) = r_tail;
    }
    else if( collision == 2 || collision == 3)
    {
       x(end) = head_index[0];
       y(end) = head_index[1];
       z(end) = head_index[2];
       vnl_Ru(end) = r_head;
    }

    for( unsigned int k = 0; k < x.size(); k++ )
    {
        //freeze tail or head part when collision happens
      if( freeze_body )
      {
       if( collision == 1 && k < N/2 )
           continue;
       if( collision == 2 && k > N/2 )
           continue;
      }

        Cu.Pt[k].x = x(k);
        Cu.Pt[k].y = y(k);
        Cu.Pt[k].z = z(k);
        Cu.Pt[k].check_out_of_range_3D(SM,SN,SZ);
    }

     if( boundary_head == 1 && boundary_tail == 1)
     {
       hit_boundary = true;
       break;
     }

  }

   //check for NaN
   for( int i = 0; i < Cu.GetSize(); i++ )
   {
     if( isnan(Cu.Pt[i].x) || isnan(Cu.Pt[i].y) || isnan(Cu.Pt[i].z) )
         return;
   }

   /*PointList3D Cu_temp;
   Ru.clear();

   for( int i = 0; i < vnl_Ru.size(); i++ )
   {
      if( vnl_Ru(i) > 0 )
      {
        Cu_temp.AddPt(Cu.Pt[i]);
        Ru.push_back(vnl_Ru(i));
      }
   }

   Cu = Cu_temp;

   if( Cu.NP < 3 )
   {
        //Ru.clear();
        //for( int i = 0; i < vnl_Ru.size(); i++ )
        //	Ru.push_back(vnl_Ru(i));
       return;
   }


   Ru = Cu.curveinterp_4D((float)pt_distance,Ru);*/

   //resampling
   vnl_Ru = Cu.curveinterp_4D((float)pt_distance, vnl_Ru);
   Ru.clear();

   for( int i = 0; i < vnl_Ru.size(); i++ )
   {
     if( vnl_Ru(i) < 0)
         vnl_Ru(i) = 1;
     Ru.push_back(vnl_Ru(i));
   }
   //std::cout<<"size"<<Cu.NP<<","<<Ru.size()<<std::endl;

}

vnl_matrix<float> SnakeClass::makeOpenA(float alpha, float beta, int N)
{
   vnl_vector<float> Alpha(N);
   Alpha.fill(alpha);
   vnl_vector<float> Beta(N);
   Beta.fill(beta);
   Beta(0) = 0;
   Beta(Beta.size()-1) = 0;

   vnl_matrix<float> A(N,N);
   A.fill(0);
  
   for( int i = 0; i < N; i++ )
   {
     int iplus1 = i + 1;
	 int iplus2 = i + 2;
	 int iminus1 = i - 1;
	 int iminus2 = i - 2;

	 if( iminus1 <= -1 ) {iminus1 = -1 * iminus1;}
	 if( iminus2 <= -1 ) {iminus2 = -1 * iminus2;}
     if( iplus1 > N-1 ) {iplus1 = 2 * (N-1) - iplus1;}
     if( iplus2 > N-1 ) {iplus2 = 2 * (N-1) - iplus2;}

     A(i, iminus2) = A(i, iminus2) + Beta(iminus1);
     A(i, iminus1) = A(i, iminus1) - 2 * Beta(iminus1) - 2 * Beta(i) - Alpha(i);
     A(i, i) = A(i,i) + Alpha(i) + Alpha(iplus1) + 4 * Beta(i) + Beta(iminus1) + Beta(iplus1);
     A(i, iplus1) = A(i, iplus1) - Alpha(iplus1) - 2 * Beta(i) - 2 * Beta(iplus1);
     A(i, iplus2) = A(i, iplus2) + Beta(iplus1);
   }

  return A;
}

 vnl_matrix<float> SnakeClass::matrix_inverse(vnl_matrix<float> A, int N)
 {

     vnl_matrix<float> invAI(N,N);
     Matrix A_temp(N,N);
     float *a;
     a = new float[N*N];
     int d = 0;
     for(int i = 0; i < N; i++)
     {
        for(int j = 0; j < N; j++)
        {
            a[d] = A(i,j);
            d++;
         }
      }
      A_temp << a;

     if(a) {delete []a; a =0;}

     Matrix B_temp(N,N);
     B_temp = A_temp.i();

     for(int i = 0; i < N; i++)
     {
        for(int j = 0; j < N; j++)
        {
            invAI(i,j) = B_temp.element(i,j);
         }
      }

     return invAI;
 }

SnakeListClass::SnakeListClass(void)
{
     NSnakes = 0;
	 //only supports 1024 snakes
	 //Snakes = new SnakeClass[4000];
	 //valid_list.set_size(4000);
	 //valid_list.fill(1);
}

void SnakeListClass::SetNSpace(int N)
{
     NSnakes = 0;
	 //Snakes = new SnakeClass[N];
	 Snakes.resize(N);
	 //valid_list.set_size(N);
	 //valid_list.fill(1);
	 valid_list.resize(N);
	 for(int i = 0; i < valid_list.size(); i++)
	    valid_list[i] = 1;
}

SnakeListClass SnakeListClass::operator=(SnakeListClass SnakeList)
{ 
	 NSnakes = 0;
	 for( int i = 0; i<SnakeList.NSnakes; i++)
	 {
		 AddSnake(SnakeList.Snakes[i]);
	 }
	 
	 valid_list = SnakeList.valid_list;
     branch_points = SnakeList.branch_points;
	 return *this;
}

void SnakeListClass::RemoveSnake(int idx)
{
	 valid_list[idx] = 0;
     IM->ImCoding( Snakes[idx].Cu, Snakes[idx].Ru, 0, false );
	 //also remove the branch point from the list
	 if( Snakes[idx].BranchPt.NP != 0 )
	 {
	   for( int i = 0; i < branch_points.NP; i++ )
	   {
	     for( int j = 0; j < Snakes[idx].BranchPt.NP; j++ )
		 {
		   if( branch_points.Pt[i].x == Snakes[idx].BranchPt.Pt[j].x && branch_points.Pt[i].y == Snakes[idx].BranchPt.Pt[j].y && branch_points.Pt[i].z == Snakes[idx].BranchPt.Pt[j].z)
		   {
			   branch_points.RemovePt(i);
			   break;
		   }
		 }
	   }
	 }
}

void SnakeListClass::RemoveAllSnakes()
{
	 Snakes.clear();
     valid_list.clear();
	 NSnakes = 0;
}

void SnakeListClass::SplitSnake(int idx_snake, int idx_pt)
{
     SnakeClass temp;

 	 for( int i = idx_pt-1; i < Snakes[idx_snake].Cu.GetSize(); i++ )
	 {
	 	temp.Cu.AddPt( Snakes[idx_snake].Cu.Pt[i] );
		temp.Ru.push_back( Snakes[idx_snake].Ru[i] );
	 }

     //IM->ImCoding( temp.Cu, temp.Ru, NSnakes+1, false );
	
	 if( temp.Cu.NP >= 3 )
	 {
	  AddSnake_Coding(temp);
	 }

	 //Snakes[idx_snake].Cu.NP = idx_pt;
	 Snakes[idx_snake].Cu.Resize(idx_pt);
	 Snakes[idx_snake].Ru.resize(idx_pt);

	 //Snakes[idx_snake].BranchPt.NP = 0;
	 Snakes[idx_snake].BranchPt.RemoveAllPts();
	
	 if( Snakes[idx_snake].Cu.NP < 3 )
		valid_list[idx_snake] = 0;
	 
}

void SnakeListClass::CreateBranch(int idx1, int idx2)
{
     vnl_vector<float> dist_temp(4);
	 Snakes[idx1].BranchPt.RemoveAllPts();
	 Snakes[idx2].BranchPt.RemoveAllPts();
     
	 vnl_vector<float> dist_temp_h2(Snakes[idx2].Cu.NP);
	 vnl_vector<float> dist_temp_t2(Snakes[idx2].Cu.NP);
	 for( int i = 0; i < Snakes[idx2].Cu.NP; i++ )
	 {
	   dist_temp_h2(i) = Snakes[idx1].Cu.GetLastPt().GetDistTo( Snakes[idx2].Cu.Pt[i] );
	   dist_temp_t2(i) = Snakes[idx1].Cu.GetFirstPt().GetDistTo( Snakes[idx2].Cu.Pt[i] );
	 }
	 dist_temp(0) = dist_temp_h2.min_value();
     dist_temp(1) = dist_temp_t2.min_value();

	 vnl_vector<float> dist_temp_h1(Snakes[idx1].Cu.NP);
	 vnl_vector<float> dist_temp_t1(Snakes[idx1].Cu.NP);
	 for( int i = 0; i < Snakes[idx1].Cu.NP; i++ )
	 {
	   dist_temp_h1(i) = Snakes[idx2].Cu.GetLastPt().GetDistTo( Snakes[idx1].Cu.Pt[i] );
	   dist_temp_t1(i) = Snakes[idx2].Cu.GetFirstPt().GetDistTo( Snakes[idx1].Cu.Pt[i] );
	 }

	 dist_temp(2) = dist_temp_h1.min_value();
     dist_temp(3) = dist_temp_t1.min_value();

	 int idx = dist_temp.arg_min();

	 if( idx == 0 )
	 {
	   Snakes[idx1].Cu.AddPt( Snakes[idx2].Cu.Pt[dist_temp_h2.arg_min()] );
	   Snakes[idx1].Ru.push_back( Snakes[idx2].Ru[dist_temp_h2.arg_min()] );
	   Snakes[idx1].BranchPt.AddPt( Snakes[idx2].Cu.Pt[dist_temp_h2.arg_min()] );
	   Snakes[idx1].Cu.curveinterp_3D(Snakes[idx1].Cu.NP);
	 }
	 else if( idx == 1 )
	 {
	   Snakes[idx1].Cu.AddTailPt( Snakes[idx2].Cu.Pt[dist_temp_t2.arg_min()] );
	   //Snakes[idx1].Ru.push_back( Snakes[idx2].Ru[dist_temp_h2.arg_min()] );
       Snakes[idx1].Ru.insert ( Snakes[idx1].Ru.begin() , Snakes[idx2].Ru[dist_temp_h2.arg_min()] );

	   Snakes[idx1].BranchPt.AddPt( Snakes[idx2].Cu.Pt[dist_temp_t2.arg_min()] );
	   Snakes[idx1].Cu.curveinterp_3D(Snakes[idx1].Cu.NP);
	 }
	 else if( idx == 2 )
	 {
       Snakes[idx2].Cu.AddPt( Snakes[idx1].Cu.Pt[dist_temp_h1.arg_min()] );
	   Snakes[idx2].Ru.push_back( Snakes[idx1].Ru[dist_temp_h1.arg_min()] );
	   Snakes[idx2].BranchPt.AddPt( Snakes[idx1].Cu.Pt[dist_temp_h1.arg_min()] );
	   Snakes[idx2].Cu.curveinterp_3D(Snakes[idx2].Cu.NP);
	 }
	 else
	 {
	   Snakes[idx2].Cu.AddTailPt( Snakes[idx1].Cu.Pt[dist_temp_t1.arg_min()] );
	   //Snakes[idx2].Ru.push_back( Snakes[idx1].Ru[dist_temp_h1.arg_min()] );
	   Snakes[idx2].Ru.insert ( Snakes[idx2].Ru.begin() , Snakes[idx1].Ru[dist_temp_h1.arg_min()] );
	   Snakes[idx2].BranchPt.AddPt( Snakes[idx1].Cu.Pt[dist_temp_t1.arg_min()] );
	   Snakes[idx2].Cu.curveinterp_3D(Snakes[idx2].Cu.NP);
	 }
	 
}

void SnakeListClass::MergeSnake(int idx1, int idx2, bool im_coding)
{
     vnl_vector<float> dist_temp(4);

	 //Snakes[idx1].BranchPt.NP = 0;
	 //Snakes[idx2].BranchPt.NP = 0;
	
	 //also remove the branch point from the list
	 if( Snakes[idx1].BranchPt.NP != 0 )
	 {
	   for( int i = 0; i < branch_points.NP; i++ )
	   {
	     for( int j = 0; j < Snakes[idx1].BranchPt.NP; j++ )
		 {
		   if( branch_points.Pt[i].x == Snakes[idx1].BranchPt.Pt[j].x && branch_points.Pt[i].y == Snakes[idx1].BranchPt.Pt[j].y && branch_points.Pt[i].z == Snakes[idx1].BranchPt.Pt[j].z)
		   {
			   branch_points.RemovePt(i);
			   break;
		   }
		 }
	   }
	 }
	//also remove the branch point from the list
	 if( Snakes[idx2].BranchPt.NP != 0 )
	 {
	   for( int i = 0; i < branch_points.NP; i++ )
	   {
	     for( int j = 0; j < Snakes[idx2].BranchPt.NP; j++ )
		 {
		   if( branch_points.Pt[i].x == Snakes[idx2].BranchPt.Pt[j].x && branch_points.Pt[i].y == Snakes[idx2].BranchPt.Pt[j].y && branch_points.Pt[i].z == Snakes[idx2].BranchPt.Pt[j].z)
		   {
			   branch_points.RemovePt(i);
			   break;
		   }
		 }
	   }
	 }


	 Snakes[idx1].BranchPt.RemoveAllPts();
	 Snakes[idx2].BranchPt.RemoveAllPts();

	 dist_temp(0) = Snakes[idx1].Cu.GetLastPt().GetDistTo( Snakes[idx2].Cu.GetLastPt() );
	 dist_temp(1) = Snakes[idx1].Cu.GetLastPt().GetDistTo( Snakes[idx2].Cu.GetFirstPt() );
	 dist_temp(2) = Snakes[idx1].Cu.GetFirstPt().GetDistTo( Snakes[idx2].Cu.GetLastPt() );
	 dist_temp(3) = Snakes[idx1].Cu.GetFirstPt().GetDistTo( Snakes[idx2].Cu.GetFirstPt() );

	 int idx = dist_temp.arg_min();

     if( idx == 0 )
	 {
	   //find closest point
	   vnl_vector<float> dist_temp1( Snakes[idx2].Cu.GetSize() );

	   for( int i = 0; i < Snakes[idx2].Cu.GetSize(); i++ )
	   {
		   dist_temp1(i) = Snakes[idx1].Cu.GetLastPt().GetDistTo( Snakes[idx2].Cu.Pt[i] );
	   }
	   int pt_idx = dist_temp1.arg_min();
	   
	   Snakes[idx1].Cu.RemovePt(); //remove the last point
	   Snakes[idx1].Ru.pop_back();

	   //transfer points
	   for( int i = pt_idx; i >= 0; i-- )
	   {
		   Snakes[idx1].Cu.AddPt( Snakes[idx2].Cu.Pt[i] );
		   Snakes[idx1].Ru.push_back( Snakes[idx2].Ru[i] );
	   }
	 }
	 else if( idx == 1 )
	 {
	   //find closest point
	   vnl_vector<float> dist_temp1( Snakes[idx2].Cu.GetSize() );

	   for( int i = 0; i < Snakes[idx2].Cu.GetSize(); i++ )
	   {
		   dist_temp1(i) = Snakes[idx1].Cu.GetLastPt().GetDistTo( Snakes[idx2].Cu.Pt[i] );
	   }
	   int pt_idx = dist_temp1.arg_min();
	   
	   Snakes[idx1].Cu.RemovePt(); //remove the last point
	   Snakes[idx1].Ru.pop_back();

	   //transfer points
       for( int i = pt_idx; i < Snakes[idx2].Cu.GetSize(); i++ )
	   {
		   Snakes[idx1].Cu.AddPt( Snakes[idx2].Cu.Pt[i] );
		   Snakes[idx1].Ru.push_back( Snakes[idx2].Ru[i] );
	   }
	 }
	 else if( idx == 2 )
	 {
	   //find closest point
	   vnl_vector<float> dist_temp1( Snakes[idx2].Cu.GetSize() );

	   for( int i = 0; i < Snakes[idx2].Cu.GetSize(); i++ )
	   {
		   dist_temp1(i) = Snakes[idx1].Cu.GetFirstPt().GetDistTo( Snakes[idx2].Cu.Pt[i] );
	   }
	   int pt_idx = dist_temp1.arg_min();
	   
	   //Snakes[idx1].Cu.Flip();
	   //Snakes[idx1].Cu.RemovePt(); //remove the last point
       //Snakes[idx1].Cu.Flip();
	   Snakes[idx1].Cu.RemoveFirstPt();
	   Snakes[idx1].Ru.erase(Snakes[idx1].Ru.begin());

	   //transfer points
       for( int i = pt_idx; i >= 0; i-- )
	   {
		   Snakes[idx1].Cu.AddTailPt( Snakes[idx2].Cu.Pt[i] );
		   //Snakes[idx1].Ru.push_back( Snakes[idx2].Ru[i] );
		   //std::vector<float>::iterator it;
		   //it = Snakes[idx1].Ru.begin();
           Snakes[idx1].Ru.insert ( Snakes[idx1].Ru.begin() , Snakes[idx2].Ru[i] );
	   }
	 }
	 else if( idx == 3 )
	 {
	   //find closest point
	   vnl_vector<float> dist_temp1( Snakes[idx2].Cu.GetSize() );

	   for( int i = 0; i < Snakes[idx2].Cu.GetSize(); i++ )
	   {
		   dist_temp1(i) = Snakes[idx1].Cu.GetFirstPt().GetDistTo( Snakes[idx2].Cu.Pt[i] );
	   }
	   int pt_idx = dist_temp1.arg_min();
	   
	   //Snakes[idx1].Cu.Flip();
	   //Snakes[idx1].Cu.RemovePt(); //remove the last point
       //Snakes[idx1].Cu.Flip();
	   Snakes[idx1].Cu.RemoveFirstPt();
       Snakes[idx1].Ru.erase(Snakes[idx1].Ru.begin());

	   //transfer points
       for( int i = pt_idx; i < Snakes[idx2].Cu.GetSize(); i++ )
	   {
		   Snakes[idx1].Cu.AddTailPt( Snakes[idx2].Cu.Pt[i] );
		   //Snakes[idx1].Ru.push_back( Snakes[idx2].Ru[i] );
		   //std::vector<float>::iterator it;
		   //it = Snakes[idx1].Ru.begin();
           Snakes[idx1].Ru.insert ( Snakes[idx1].Ru.begin() , Snakes[idx2].Ru[i] );
	   }
	 }
	 valid_list[idx2] = 0;

	 if( im_coding )
	 {
	  IM->ImCoding( Snakes[idx1].Cu, Snakes[idx1].Ru, idx1+1, false );
	 }

	  //for( int i = 0; i < Snakes[idx1].Cu.GetSize(); i++ )
	  // {
	  //   std::cout<<"Snakes[idx1].Cu:"<<Snakes[idx1].Cu.Pt[i].x<<","<<Snakes[idx1].Cu.Pt[i].y<<","<<Snakes[idx1].Cu.Pt[i].z<<std::endl;
	  //}
}

SnakeClass SnakeListClass::GetSnake(int idx)
{   
     return Snakes[idx];
}

void SnakeListClass::AddSnake(SnakeClass snake)
{
     //Snakes[NSnakes] = snake;
	 //Snakes[NSnakes].SetImage(IM);
	 //NSnakes++;
	Snakes.push_back(snake);
	valid_list.push_back(1);
	NSnakes = Snakes.size();
	
}
void SnakeListClass::AddSnake_Coding(SnakeClass snake)
{
     //Snakes[NSnakes] = snake;
	 //Snakes[NSnakes].SetImage(IM);
	 //NSnakes++;
    IM->ImCoding( snake.Cu, snake.Ru, NSnakes+1, false );
	Snakes.push_back(snake);
	valid_list.push_back(1);
	NSnakes = Snakes.size();
	
}

void SnakeListClass::SetImage(ImageOperation *I_Input)
{
     IM = I_Input;
}

bool isnan( float x ) 
{ 
	return (x) != (x); 
};

bool isinf( float x )
{
    return std::numeric_limits<float>::has_infinity &&
           x == std::numeric_limits<float>::infinity() ;
}
