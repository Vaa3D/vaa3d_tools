#ifndef __RUN_MAIN_H__
#define __RUN_MAIN_H__
#include "basic_parser.h"

Attribute main_attrs[] = {
	{"+swc2mask",0},{"-inswc",1}
};

bool main_usage(string cmd_name, ostream &out);
bool main_demo(string cmd_name, ostream &out);
int  run_main(BasicParser &parser);

#endif
