// openmp_test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "iostream"
#include <omp.h>
#include <ctime>
#include "first.h"
using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	cout << "hello world" << endl;
	int n=100;
	clock_t start,end;
	start = clock();

	omp_set_num_threads(12);
	#pragma omp parallel for 
	for (int i=0;i<n;i++){
		for (int j=0;j<n;j++){
			printf("i = %d, I am Thread %d\n", i, omp_get_thread_num());
		}
	}
	end = clock();
	double dur = (double)(end - start);
	cout << dur/CLOCKS_PER_SEC << endl;
	fun1();
	return 0;
}

void fun1(){
	std::cout << "main" << endl;
}