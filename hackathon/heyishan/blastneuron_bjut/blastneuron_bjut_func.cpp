/*
 * blastneuron_func.cpp
 *
 * Created by He Yishan
 */

#include<iostream>
#include"blastneuron_bjut_func.h"

using namespace std;

bool pre_process_func(const V3DPluginArgList & input, V3DPluginArgList & output)
{
    cout << "This is a pre-processing for blastneuron."<<endl;
    resampling_main(input, output);
    return 1;
 }

void printHelp()
{
    cout<<"This is printHelp"<<endl;
}
