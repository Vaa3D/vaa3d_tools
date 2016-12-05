/*
 * blastneuron_func.cpp
 *
 * Created by He Yishan
 */

#include<iostream>
#include"blastneuron_bjut_func.h"
#include<vector>
#include"v3d_message.h"
#include"v3d_interface.h"
#include"resample_func.h"
using namespace std;

bool blast_bjut_func(const V3DPluginArgList & input, V3DPluginArgList & output)
{
    cout << "This is blast_bjut_menu."<<endl;
    resampling_main(input, output);
    sort_main();
     blastneuron_main();
    return 1;
 }

void sort_main()
{
    cout<<"This is sort_menu."<<endl;
    return;
}
void blastneuron_main()
{
    cout<<"This is blastneuron menu."<<endl;
}
void printHelp()
{
    cout<<"This is printHelp"<<endl;
}
