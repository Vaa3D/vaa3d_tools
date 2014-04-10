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

#include "Register.h"


Register::Register()
{
   TG = NULL;
}

void Register::set_root(int in)
{
  if( TG )
  {
    root = in;

    TT = new TransformGraph[N];

    for( int i = 0; i < N; i++ )
	{
	  TT[i].level = 0;
	  TT[i].visited = 0;
	  for( int j = 0; j < N; j++ )
	  {
	    //if( TG[i*N+j].overlap != 0 )
		if( TG[i*N+j].overlap >= 0.03 && TG[i*N+j].overlap <= 0.97 )
		{
		  /*//also check if the two tiles are adjacent
		  bool adjacent = false;
		  PointList3D boundary;
		  boundary.AddPt(coordinates.Pt[j].x, coordinates.Pt[j].y, 0);
		  boundary.AddPt(coordinates.Pt[j].x + SM, coordinates.Pt[j].y, 0);
		  boundary.AddPt(coordinates.Pt[j].x + SM, coordinates.Pt[j].y + SN, 0);
		  boundary.AddPt(coordinates.Pt[j].x, coordinates.Pt[j].y + SN, 0);
          
		  Point3D corner1, corner2, corner3, corner4;
          corner1.x = coordinates.Pt[i].x;
		  corner1.y = coordinates.Pt[i].y;
		  corner1.z = 0;
		  if( corner1.Inpolygon2D(boundary) )
			  adjacent = true;

          corner2.x = coordinates.Pt[i].x + SM;
		  corner2.y = coordinates.Pt[i].y;
		  corner2.z = 0;
		  if( corner2.Inpolygon2D(boundary) )
			  adjacent = true;

		  corner3.x = coordinates.Pt[i].x + SM;
		  corner3.y = coordinates.Pt[i].y + SN;
		  corner3.z = 0;
		  if( corner3.Inpolygon2D(boundary) )
			  adjacent = true;

		  corner4.x = coordinates.Pt[i].x;
		  corner4.y = coordinates.Pt[i].y + SN;
		  corner4.z = 0;
		  if( corner4.Inpolygon2D(boundary) )
			  adjacent = true;
  
		  if( adjacent ) */
		   TT[i].connection.push_back(j);
		}
	  }
	}
    

	std::vector<int> current_list;
	std::vector<int> next_list;
	current_list.push_back(in);

	int lvl = 1;

    while( current_list.size() != 0 )
	{
		next_list.clear();
        
		lvl++;

		for( unsigned int i = 0 ; i < current_list.size(); i++ )
		{
		  int idx = current_list[i];
		  TT[idx].visited = 1;

          if( idx == root )
		  {
		    TT[idx].level = 1;
		    vnl_matrix<float> R_temp(3,3);
			R_temp.set_identity();
            TT[idx].R = R_temp;
            vnl_vector<float> T_temp(3);
			T_temp.fill(0);
			TT[idx].T = T_temp;
		  }
  
		  for( unsigned int j = 0; j < TT[idx].connection.size(); j++ )
		  {
		     int idx1 = TT[idx].connection[j];
			 if( TT[idx1].visited != 1 )
			 {
			    bool repeat = false;
			    for( unsigned int k = 0; k < current_list.size(); k++)
				{
				  if( idx1 == current_list[k] )
				  {
				    repeat = true;
					break;
				  }
				}
				if( !repeat )
				{
				 //check if there is repeat in next_list
				  bool next_repeat = false;
				  for( unsigned int k = 0; k < next_list.size(); k++ )
				  {
					if( idx1 == next_list[k] )
						next_repeat = true;
				  }
			      if( !next_repeat )
				  {
				    next_list.push_back(idx1);
				    TT[idx1].level = lvl;
				    TT[idx1].R = TT[idx].R * TG[idx1*N+idx].R;
				    TT[idx1].T = TT[idx].T + TG[idx1*N+idx].T;
		
					//std::cout<<"idx1-idx:"<<string_array[idx1]<<","<<string_array[idx]<<","<<TG[idx1*N+idx].overlap<<":"<<TG[idx1*N+idx].T<<", "<<TG[idx*N+idx1].T<<std::endl;
					//TT[idx1].R = TG[idx1*N+root].R;
					//TT[idx1].T = TG[idx1*N+root].T;
					//std::cout<<"idx1:"<<string_array[idx1]<<","<<"idx:"<<idx<<",  "<<TT[idx1].T<<"  "<<TG[idx1*N+root].T<<std::endl;
				  }
				}
			 }
		  }
		}
	

		current_list = next_list;
	   if( next_list.size() != 0 )
	   {
		std::cout<<"Level"<<lvl<<":";
		for( unsigned int k = 0; k < current_list.size(); k++ )
		{
			std::cout<<string_array[current_list[k]]<<",";
		}
		std::cout<<std::endl;
	   }
	}

	//clear all the visited label
	 for( int i = 0; i < N; i++ )
	 {
		 TT[i].visited = 0;
	 }
	
  }
}

void Register::set_n(int in, std::string *sin)
{
	
  string_array = sin;
  N = in;
  std::cout<<"Number of Images:"<<N<<std::endl;
  TG = new Transform[N*N];
  
  vnl_matrix<float> R_temp(3,3);
  R_temp.fill(0);
  vnl_vector<float> T_temp(3);
  T_temp.fill(0);

  for( int i = 0; i < N; i++ )
  {
    for( int j = 0; j < N; j++ )
	{
	  TG[i*N+j].R = R_temp;
      TG[i*N+j].T = T_temp;
	  TG[i*N+j].overlap = 0;
	}
  }
}

void Register::set_coordinates(PointList3D in, int in1, int in2)
{
  coordinates = in;
  SM = in1;
  SN = in2;
}

SnakeClass Register::convert_global_snake(SnakeClass snake_in, int tile_num, int scale)
{
  bool affine = false;

  for( int i = 0; i < snake_in.Cu.NP; i++ )
  {
    
   if( affine )
   {
    //rotation
	vnl_vector<float> pt(3);
	pt(0) = snake_in.Cu.Pt[i].x * scale;
	pt(1) = snake_in.Cu.Pt[i].y * scale;
	pt(2) = snake_in.Cu.Pt[i].z;
	pt = TT[tile_num].R * pt;

	//translation
	snake_in.Cu.Pt[i].x = pt(0) + TT[tile_num].T[0] + coordinates.Pt[root].x;
	snake_in.Cu.Pt[i].y = pt(1) + TT[tile_num].T[1] + coordinates.Pt[root].y;
	snake_in.Cu.Pt[i].z = pt(2) + TT[tile_num].T[2];
	//sin.Cu.Pt[i].z = 0;
   }
   else
   {
    //sin.Cu.Pt[i].x = sin.Cu.Pt[i].x * scale + TT[tile_num].T[0] + coordinates.Pt[root].x;
    //sin.Cu.Pt[i].y = sin.Cu.Pt[i].y * scale + TT[tile_num].T[1] + coordinates.Pt[root].y;
	//sin.Cu.Pt[i].z = sin.Cu.Pt[i].z + TT[tile_num].T[2];
	snake_in.Cu.Pt[i].x = snake_in.Cu.Pt[i].x * scale + coordinates.Pt[tile_num].x;
    snake_in.Cu.Pt[i].y = snake_in.Cu.Pt[i].y * scale + coordinates.Pt[tile_num].y;
	//sin.Cu.Pt[i].z = 0;
   }
  }

  return snake_in;
}

SnakeClass Register::convert_local_snake(SnakeClass snake_in, int tile_num, int scale)
{
  bool affine = false;

  for( int i = 0; i < snake_in.Cu.NP; i++ )
  {
    
   if( affine )
   {
    //rotation
	vnl_vector<float> pt(3);
	pt(0) = snake_in.Cu.Pt[i].x * scale;
	pt(1) = snake_in.Cu.Pt[i].y * scale;
	pt(2) = snake_in.Cu.Pt[i].z;
	pt = TT[tile_num].R * pt;

	//translation
	snake_in.Cu.Pt[i].x = pt(0) - TT[tile_num].T[0] - coordinates.Pt[root].x;
	snake_in.Cu.Pt[i].y = pt(1) - TT[tile_num].T[1] - coordinates.Pt[root].y;
	snake_in.Cu.Pt[i].z = pt(2) - TT[tile_num].T[2];
	//sin.Cu.Pt[i].z = 0;
   }
   else
   {
    //sin.Cu.Pt[i].x = sin.Cu.Pt[i].x * scale + TT[tile_num].T[0] + coordinates.Pt[root].x;
    //sin.Cu.Pt[i].y = sin.Cu.Pt[i].y * scale + TT[tile_num].T[1] + coordinates.Pt[root].y;
	//sin.Cu.Pt[i].z = sin.Cu.Pt[i].z + TT[tile_num].T[2];
	snake_in.Cu.Pt[i].x = (snake_in.Cu.Pt[i].x - coordinates.Pt[tile_num].x)/scale;
    snake_in.Cu.Pt[i].y = (snake_in.Cu.Pt[i].y - coordinates.Pt[tile_num].y)/scale;
	//sin.Cu.Pt[i].z = 0;
   }
  }
  return snake_in;
}

void Register::eliminate_repeating(SnakeListClass *SnakeList, int tile_num, int shift_pixel, int min_length)
{
  //int shift_pixel = 0;
  Seed_Snakes.RemoveAllSnakes();
  seed_snake_label.clear();
  //shift_pixel = 1;

  

  std::cout<<"coordinates.Pt[tile_num]:"<<coordinates.Pt[tile_num].x<<","<<coordinates.Pt[tile_num].y<<","<<coordinates.Pt[tile_num].z<<std::endl;
  std::cout<<"TT[tile_num].T + coordinates.Pt[root]:"<<TT[tile_num].T[0] + coordinates.Pt[root].x<<","<<TT[tile_num].T[1] + coordinates.Pt[root].y<<std::endl;
  std::cout<<"SnakeList->NSnakes:"<<SnakeList->NSnakes<<std::endl;
  for( int i = 0; i < SnakeList->NSnakes; i++ )
  {

	  SnakeClass temp_snake;

	  if( SnakeList->valid_list[i] == 0 )
		  continue;

      int idx = 0;
	  if( SnakeList->Snakes[i].Cu.GetFirstPt().x - TT[tile_num].T[0] - coordinates.Pt[root].x < SM - shift_pixel && 
		  SnakeList->Snakes[i].Cu.GetFirstPt().x - TT[tile_num].T[0] - coordinates.Pt[root].x >= shift_pixel &&
		  SnakeList->Snakes[i].Cu.GetFirstPt().y - TT[tile_num].T[1] - coordinates.Pt[root].y < SN - shift_pixel &&
		  SnakeList->Snakes[i].Cu.GetFirstPt().y - TT[tile_num].T[1] - coordinates.Pt[root].y >= shift_pixel )
	  {
	     idx = 1;
	     while( SnakeList->Snakes[i].Cu.Pt[idx].x - TT[tile_num].T[0] - coordinates.Pt[root].x < SM - shift_pixel && 
		  SnakeList->Snakes[i].Cu.Pt[idx].x - TT[tile_num].T[0] - coordinates.Pt[root].x >= shift_pixel &&
		  SnakeList->Snakes[i].Cu.Pt[idx].y - TT[tile_num].T[1] - coordinates.Pt[root].y < SN - shift_pixel &&
		  SnakeList->Snakes[i].Cu.Pt[idx].y - TT[tile_num].T[1] - coordinates.Pt[root].y >= shift_pixel )
		 {
		   idx++;
	       
		   //in case all the snake is in the overlapping region
		   if( idx >= SnakeList->Snakes[i].Cu.NP )
		   {
			   idx = SnakeList->Snakes[i].Cu.NP - 1; 
			   break;
		   }
		 }

		 //add eliminated part to seed snake
		 seed_snake_label.push_back(i);
		 //Seed_Snakes.NSnakes++;
		   //Seed_Snakes.Snakes[Seed_Snakes.NSnakes-1].Cu.NP = 0;
		 //Seed_Snakes.Snakes[Seed_Snakes.NSnakes-1].Cu.RemoveAllPts();

	     for( int k = 0; k < idx; k++ )
		 {
		   //Seed_Snakes.Snakes[Seed_Snakes.NSnakes-1].Cu.AddPt(SnakeList->Snakes[i].Cu.Pt[k]);
			 temp_snake.Cu.AddPt(SnakeList->Snakes[i].Cu.Pt[k]);
			 temp_snake.Ru.push_back(SnakeList->Snakes[i].Ru[k]);
		 }

		 Seed_Snakes.AddSnake(temp_snake);

		 //remove the first idx points
		 SnakeList->Snakes[i].Cu.Pt.erase(SnakeList->Snakes[i].Cu.Pt.begin(), SnakeList->Snakes[i].Cu.Pt.begin() + idx);
		 SnakeList->Snakes[i].Cu.NP = SnakeList->Snakes[i].Cu.Pt.size();
		 SnakeList->Snakes[i].Ru.erase(SnakeList->Snakes[i].Ru.begin(), SnakeList->Snakes[i].Ru.begin() + idx);
		 
		 //for( int k = idx; k < SnakeList->Snakes[i].Cu.NP; k++ )
		 //{
		 //  SnakeList->Snakes[i].Cu.Pt[k-idx] = SnakeList->Snakes[i].Cu.Pt[k];
		 //}
		 //SnakeList->Snakes[i].Cu.NP -= idx;
		 //SnakeList->Snakes[i].Cu.Resize(SnakeList->Snakes[i].Cu.NP - idx);
	  }
	  else if( SnakeList->Snakes[i].Cu.GetLastPt().x - TT[tile_num].T[0] - coordinates.Pt[root].x < SM - shift_pixel && 
		  SnakeList->Snakes[i].Cu.GetLastPt().x - TT[tile_num].T[0] - coordinates.Pt[root].x >= shift_pixel &&
		  SnakeList->Snakes[i].Cu.GetLastPt().y - TT[tile_num].T[1] - coordinates.Pt[root].y < SN - shift_pixel &&
		  SnakeList->Snakes[i].Cu.GetLastPt().y - TT[tile_num].T[1] - coordinates.Pt[root].y >= shift_pixel )
	  {
	  	 idx = SnakeList->Snakes[i].Cu.NP - 2;
		 int minus_value = 0;
	     while( SnakeList->Snakes[i].Cu.Pt[idx].x - TT[tile_num].T[0] - coordinates.Pt[root].x < SM - shift_pixel && 
		  SnakeList->Snakes[i].Cu.Pt[idx].x - TT[tile_num].T[0] - coordinates.Pt[root].x >= shift_pixel &&
		  SnakeList->Snakes[i].Cu.Pt[idx].y - TT[tile_num].T[1] - coordinates.Pt[root].y < SN - shift_pixel &&
		  SnakeList->Snakes[i].Cu.Pt[idx].y - TT[tile_num].T[1] - coordinates.Pt[root].y >= shift_pixel )
		 {
		   idx--;
		   //SnakeList->Snakes[i].Cu.NP--;
		   minus_value++;
		   //in case all the snake is in the overlapping region
		   if( idx < 0 )
		   {
			   idx = 0; 
			   break;
		   }
		 }

		 //add eliminated part to seed snake
		 seed_snake_label.push_back(i);
		 //Seed_Snakes.NSnakes++;
		  //Seed_Snakes.Snakes[Seed_Snakes.NSnakes-1].Cu.NP = 0;
		 //Seed_Snakes.Snakes[Seed_Snakes.NSnakes-1].Cu.RemoveAllPts();

		 //std::cout<<"idx:"<<idx<<std::endl;
		 //std::cout<<"SnakeList->Snakes[i].Cu.NP:"<<SnakeList->Snakes[i].Cu.NP<<std::endl;
		 //std::cout<<"SnakeList->Snakes[i].Ru.size():"<<SnakeList->Snakes[i].Ru.size()<<std::endl;
	     for( int k = idx; k < SnakeList->Snakes[i].Cu.NP; k++ )
		 {
		   //Seed_Snakes.Snakes[Seed_Snakes.NSnakes-1].Cu.AddPt(SnakeList->Snakes[i].Cu.Pt[k]);
			 temp_snake.Cu.AddPt(SnakeList->Snakes[i].Cu.Pt[k]);
			 temp_snake.Ru.push_back(SnakeList->Snakes[i].Ru[k]);
		 }

		 Seed_Snakes.AddSnake(temp_snake);

		 //SnakeList->Snakes[i].Cu.NP -= minus_value;
		 SnakeList->Snakes[i].Cu.Resize(SnakeList->Snakes[i].Cu.NP - minus_value);
		 SnakeList->Snakes[i].Ru.resize(SnakeList->Snakes[i].Ru.size() - minus_value);

	  }
	  else
	  {
		  continue;
	  }
    
	  //std::cout<<"SnakeList->Snakes[i].Cu.NP:"<<SnakeList->Snakes[i].Cu.NP<<std::endl;
	  //std::cout<<"SnakeList->Snakes[i].Ru:"<<SnakeList->Snakes[i].Ru.size()<<std::endl;
	  //remove invalid snakes
	  if( SnakeList->Snakes[i].Cu.GetLength() <= min_length || SnakeList->Snakes[i].Cu.NP < 3 )
		  SnakeList->valid_list[i] = 0;
  }
}

void Register::calculate_transform()
{
   PointList3D pt_rec;
   for( int i = 0; i < coordinates.NP; i++ )
   {
     for( int j = i; j < coordinates.NP; j++ )
	 {
		TG[i*N+j].R.set_identity();
		TG[i*N+j].T(0) = coordinates.Pt[i].x - coordinates.Pt[j].x;
		TG[i*N+j].T(1) = coordinates.Pt[i].y - coordinates.Pt[j].y;
		TG[i*N+j].T(2) = 0;

		//decide if the two tiles are overlapping
		pt_rec.RemoveAllPts();
		pt_rec.AddPt( coordinates.Pt[j].x, coordinates.Pt[j].y, 0);
		pt_rec.AddPt( coordinates.Pt[j].x + SM, coordinates.Pt[j].y, 0);
		pt_rec.AddPt( coordinates.Pt[j].x + SM, coordinates.Pt[j].y + SN, 0);
        pt_rec.AddPt( coordinates.Pt[j].x, coordinates.Pt[j].y + SN, 0);

		bool overlap = false;

		Point3D temp_pt;
		temp_pt.x = coordinates.Pt[i].x;
		temp_pt.y = coordinates.Pt[i].y;
		temp_pt.z = 0;
		if( temp_pt.Inpolygon2D(pt_rec) )
			overlap = true;
		temp_pt.x = coordinates.Pt[i].x + SM;
		temp_pt.y = coordinates.Pt[i].y;
		temp_pt.z = 0;
		if( temp_pt.Inpolygon2D(pt_rec) )
			overlap = true;
		temp_pt.x = coordinates.Pt[i].x + SM;
		temp_pt.y = coordinates.Pt[i].y + SN;
		temp_pt.z = 0;
		if( temp_pt.Inpolygon2D(pt_rec) )
			overlap = true;
		temp_pt.x = coordinates.Pt[i].x;
		temp_pt.y = coordinates.Pt[i].y + SN;
		temp_pt.z = 0;
		if( temp_pt.Inpolygon2D(pt_rec) )
			overlap = true;

		if( !overlap )
			continue;
		
		float overlapping = 0;
	    //estimate the overlapping
		for( int k = coordinates.Pt[i].x; k < coordinates.Pt[i].x + SM; k++ )
		{
		  for( int z = coordinates.Pt[i].y; z < coordinates.Pt[i].y + SN; z++ )
		  {
		    if( k <= coordinates.Pt[j].x + SM && k >= coordinates.Pt[j].x && z <= coordinates.Pt[j].y + SN && z >= coordinates.Pt[j].y )
			{
			   overlapping++;
			}
		  }
		}
		overlapping /= (float)(SM*SN);
		TG[i*N+j].overlap = overlapping;

        TG[j*N+i].R = TG[i*N+j].R.transpose();
	    TG[j*N+i].T = TG[i*N+j].T * -1;
        TG[j*N+i].overlap = TG[i*N+j].overlap;
	 }
   }
}

void Register::read_xml(std::string const & filename)
{
  TiXmlDocument doc;
    
  //Parse the resource
  if ( !doc.LoadFile( filename.c_str() ) ) {
	  std::cout<<"Unable to load XML File"<<std::endl;
    return ;
  }

  /*Get the root element node */
  TiXmlElement* root_element = doc.RootElement();
  const char* docname = root_element->Value();
  if ( strcmp( docname, "Pairwise_Registration" ) != 0 &&
       strcmp( docname, "Joint_Registration" ) != 0 ) {
		   std::cout<<"Incorrect XML root Element "<<std::endl;
    return;
  }

  //Read the parameters
  TiXmlElement* node = root_element->FirstChildElement();
 
  for( ; node; node = node->NextSiblingElement())
  {
   read_xml_node( node );
  }
 
}

void Register::read_xml_node(TiXmlElement* parent_node)
{
  std::string from_image_id_;
  std::string to_image_id_;

  int ratio = 1;

  TiXmlElement* cur_node =  parent_node->FirstChildElement();

  int id1,id2;
  id1 = -1;
  id2 = -1;

  for ( ; cur_node; cur_node = cur_node->NextSiblingElement()) {

    const char * value = cur_node->Value();
   

    // from_image_id
    if (strcmp( value, "from_image_ID" ) == 0 ) {
      from_image_id_ = cur_node->GetText();
      for( int i = 0; i < N; i++ )
	  {
		  //if(strcmp( from_image_id_.c_str(), string_array[i].c_str() ) == 0 )
		  //std::cout<<from_image_id_.substr(0,6).c_str()<<","<<string_array[i].substr(0,6).c_str()<<std::endl;
		  if(strcmp( from_image_id_.substr(0,6).c_str(), string_array[i].substr(0,6).c_str() ) == 0 )
		  {
			id1 = i;
		  }
	  }
      continue;
    }
    
    // to_image_id
    if (strcmp( value, "to_image_ID" ) == 0) {
      to_image_id_ = cur_node->GetText();
      for( int i = 0; i < N; i++ )
	  {
		  //std::cout<<to_image_id_.substr(0,6).c_str()<<","<<string_array[i].substr(0,6).c_str()<<std::endl;
		  if(strcmp( to_image_id_.substr(0,6).c_str(), string_array[i].substr(0,6).c_str() ) == 0 )
		  {
			  id2 = i;
		  }
	  }
      continue;
    }


	if( id1 == -1 || id2 == -1 )
	  return;

    // overlapping
    if (strcmp( value, "overlap_percentage" ) == 0 ) {
		std::stringstream( cur_node->GetText() ) >> TG[id1*N+id2].overlap;
     continue;
    }

    // parameters
    if ( strcmp( value, "parameters") == 0 ) {
      std::stringstream( cur_node->Attribute("a00") ) >> TG[id1*N+id2].R(0,0);
      std::stringstream( cur_node->Attribute("a01") ) >> TG[id1*N+id2].R(0,1);
      std::stringstream( cur_node->Attribute("a02") ) >> TG[id1*N+id2].R(0,2);
      std::stringstream( cur_node->Attribute("a10") ) >> TG[id1*N+id2].R(1,0);
      std::stringstream( cur_node->Attribute("a11") ) >> TG[id1*N+id2].R(1,1);
      std::stringstream( cur_node->Attribute("a12") ) >> TG[id1*N+id2].R(1,2);
      std::stringstream( cur_node->Attribute("a20") ) >> TG[id1*N+id2].R(2,0);
      std::stringstream( cur_node->Attribute("a21") ) >> TG[id1*N+id2].R(2,1);
      std::stringstream( cur_node->Attribute("a22") ) >> TG[id1*N+id2].R(2,2);
      std::stringstream( cur_node->Attribute("tx") ) >> TG[id1*N+id2].T(0);
      std::stringstream( cur_node->Attribute("ty") ) >> TG[id1*N+id2].T(1);
      std::stringstream( cur_node->Attribute("tz") ) >> TG[id1*N+id2].T(2);

	  TG[id1*N+id2].T(0) *= ratio;
	  TG[id1*N+id2].T(1) *= ratio;

	  TG[id2*N+id1].R = TG[id1*N+id2].R.transpose();
	  TG[id2*N+id1].T = TG[id1*N+id2].T * -1;
      TG[id2*N+id1].overlap = TG[id1*N+id2].overlap;

      continue;
	} 
    
  }
}