// testcuda1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <time.h>

#include <stdio.h>
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "iostream"

#include<omp.h>
using namespace std;
extern "C"
	cudaError_t addWithCuda(unsigned short *c, const unsigned short *a, const unsigned short *b, unsigned int size,int blockNums,int threadNums);

void addWithCpp(unsigned short *c, const unsigned short *a, const unsigned short *b, unsigned int size){
	for(int i=0;i<size;i++){
		for(int j=0;j<60;j++){
			c[i]=a[i]*b[i];
		}
	}
}
int main()
{
	int cpu_threads=12;
	omp_set_num_threads(cpu_threads); 
	clock_t start_for=clock();
#pragma omp parallel for
	for (int tmp=0;tmp<cpu_threads;tmp++){
		/*const int arraySize = 5;
		const int a[arraySize] = { 1, 2, 3, 4, 5 };
		const int b[arraySize] = { 10, 20, 30, 40, 50 };*/
		cout << "start" << endl;
		const int arraySize=512*512*512;
		unsigned short *a=(unsigned short*)malloc(sizeof(unsigned short)*arraySize);
		cout << "malloc(sizeof(int)*arraySize);" << endl;
		unsigned short *b=(unsigned short*)malloc(sizeof(unsigned short)*arraySize);
		cout << "malloc(sizeof(int)*arraySize);" << endl;
		unsigned short *c=(unsigned short*)malloc(sizeof(unsigned short)*arraySize);
		cout << "malloc(sizeof(int)*arraySize);" << endl;
		for(int i=0;i<arraySize;i++){
			a[i]=unsigned short(1);
			b[i]=unsigned short(1);
			c[i]=unsigned short(1);
		}
		cout << sizeof(unsigned short) << " " << unsigned short(1) << endl;
		cout << "malloc over" << endl;
	
		clock_t  clockBegin, clockEnd;
		clockBegin = clock();
		addWithCpp(c, a, b, arraySize);
		clockEnd=clock();
		cout << "cpu: " << clockEnd-clockBegin << "ms" << endl;


		{
			clockBegin = clock();
			// Add vectors in parallel.
			cudaError_t cudaStatus=cudaError_t(0);
			cudaStatus = addWithCuda(c, a, b, arraySize,512*2*128,512*2);
			cout << "gpu:" << clock()-clockBegin << "ms" << endl;

			if (cudaStatus != cudaSuccess) {
				fprintf(stderr, "addWithCuda failed!");
				//return 1;
			}

			printf("{1,2,3,4,5} + {10,20,30,40,50} = {%d,%d,%d,%d,%d}\n",
				c[0], c[1], c[2], c[512*512*512-2], c[512*512*512-2]);
			printf("cuda工程中调用cpp成功！\n");

			// cudaDeviceReset must be called before exiting in order for profiling and
			// tracing tools such as Nsight and Visual Profiler to show complete traces.
			cudaStatus = cudaDeviceReset();
			if (cudaStatus != cudaSuccess) {
				fprintf(stderr, "cudaDeviceReset failed!");
				//return 1;
			}
		}
		/*for(int i=0;i<arraySize;i++){
			cout << c[i] << endl;
		}*/
		free(a);
		free(b);
		free(c);
	}
	cout << "all_time: " << clock()-start_for << endl;

	getchar(); //here we want the console to hold for a while
	return 0;
}
