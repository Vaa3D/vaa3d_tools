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
#ifndef REGISTER_H
#define REGISTER_H

#include <tinyxml/tinyxml.h>
#include <string>
#include <vector>
#include <vnl/vnl_matrix.h>
#include <vnl/vnl_vector.h>
#include "../PointOperation.h"
#include "../SnakeOperation.h"

struct Transform
{
	vnl_matrix<float> R;
	vnl_vector<float> T;
	float overlap;
};

struct TransformGraph
{
    bool visited;
	int level;
	std::vector<int> connection;
	vnl_matrix<float> R;
	vnl_vector<float> T;
};

class Register
{
public: 
  Register();
  void set_root(int in);
  void set_n(int in, std::string *sin);
  void set_coordinates(PointList3D in, int in1, int in2);
  SnakeClass convert_global_snake(SnakeClass sin, int tile_num, int scale);
  SnakeClass convert_local_snake(SnakeClass sin, int tile_num, int scale);
  void eliminate_repeating(SnakeListClass *SnakeList, int tile_num, int shift_pixel, int min_length);
  void calculate_transform();
  void read_xml(std::string const & filename);
  void read_xml_node(TiXmlElement* parent_node);
  Transform *TG;
  TransformGraph *TT;
  std::string *string_array;
  SnakeListClass Seed_Snakes;
  std::vector<int> seed_snake_label;
  int N;
  int SM,SN;
private:
  int root;
  PointList3D coordinates;
  
};

#endif
