/*********************************************
 * file : run_main.cpp , 2012-03-05 by Hang Xiao
 *
 * The main parser for different commands
 *
 *********************************************/

#ifndef __RUN_MAIN_SRC__
#define __RUN_MAIN_SRC__

#include <iostream>
#include <fstream>
#include <string>
#include <sstream> 
#include <vector>

#include "stackutil.h"
#include "common_macro.h"
#include "basic_parser.h"
#include "swc2mask.h"
using namespace std;

int run_main(BasicParser &parser)
{
  INIT_IMAGE_VARIABLE;
  LOAD_FIRST_IMAGE;

	string cmd_name = parser.get_main_cmd();

	if(cmd_name == "-swc2mask")
    {
        if(parser.filelist.empty() || !parser.is_exist("-inswc")) return false;

        string inswc_file = parser.get_para("-inswc");
        vector<MyMarker*> inswc = readSWC_file(inswc_file);
        swc2mask(indata1d, outdata1d, inswc, in_sz[0], in_sz[1], in_sz[2]);
	}
	else return -1;

  SAVE_OUTPUT_IMAGE;
	return true;
}

bool main_usage(string cmd_name, ostream &out)
{
	if(cmd_name == "-swc2mask")
	{
		out<<"v3d_convert -swc2mask "<<endl;
		out<<endl;
	}
	else return false;

	return true;
}

bool main_demo(string cmd_name, ostream & out)
{
	if(cmd_name == "-swc2mask")
	{
		out<<"v3d_convert -swc2mask"<<endl;
	}
	else return false;

	return true;
}

#endif
