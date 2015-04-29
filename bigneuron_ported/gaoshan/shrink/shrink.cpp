#include <math.h>
#include <iostream>
#include "v3d_interface.h"
#include "shrink_3dmorph.h"
#include "shrink_math.h"
using namespace std;

bool shrink_tracing(unsigned char* input_bw, long input_width, long input_height, long input_layer, const int kernel_sz, NeuronTree *output_tree)
{
    NeuronSWC TempNode;
    CKernelMP erode_kernel,dilate_kernel;
    long TotalByte = input_width*input_height*input_layer;
    double temp_divide = kernel_sz/2;
    const int anchor = floor(temp_divide);
    if(!shrink_create_kernel3D(kernel_sz,kernel_sz,kernel_sz,anchor,anchor,anchor,1,&erode_kernel))
    {
        return false;
    }
    if(!shrink_create_kernel3D(kernel_sz,kernel_sz,kernel_sz,anchor,anchor,anchor,1,&dilate_kernel))
    {
        return false;
    }
    unsigned char* ori = new unsigned char[TotalByte];
    if(!ori)
    {
        cout<<"Error Allocating Memory"<<endl;
        return false;
    }
    unsigned char* eroded = new unsigned char[TotalByte];
    if(!eroded)
    {
        cout<<"Error Allocating Memory"<<endl;
        return false;
    }
    unsigned char* dilated = new unsigned char[TotalByte];
    if(!dilated)
    {
        cout<<"Error Allocating Memory"<<endl;
        return false;
    }
    unsigned char* NewNodes = new unsigned char[TotalByte];
    if(!NewNodes)
    {
        cout<<"Error Allocating Memory"<<endl;
        return false;
    }
    memcpy(ori,input_bw,TotalByte);
    long RadiusNum = 0;
    long NodeNum = 0;
    while (shrink_sum(ori,TotalByte))
    {
        RadiusNum+=anchor;
        shrink_erosion3D(ori,input_width,input_height,input_layer,erode_kernel,eroded);
        shrink_dilation3D(eroded,input_width,input_height,input_layer,dilate_kernel,dilated);
        if(RadiusNum>1)
        {
            for (int i=0;i<TotalByte;i++)
            {
                NewNodes[i]=ori[i]>dilated[i]?1:0;
            }
            for (int i=0;i<input_width;i++)
            {
                for (int j=0;j<input_height;j++)
                {
                    for (int k=0;k<input_layer;k++)
                    {
                        long index=input_width*input_height*k+input_width*j+i;
                        if (NewNodes[index])
                        {
                            TempNode.n = NodeNum;
                            TempNode.r = RadiusNum;
                            TempNode.x = i;
                            TempNode.y = j;
                            TempNode.z = k;
                            output_tree->listNeuron.append(TempNode);
                            NodeNum++;
                        }
                    }
                }
            }
        }
        unsigned char* temp_pointer = ori;
        ori = eroded;
        eroded = temp_pointer;
    }
    return true;
}
