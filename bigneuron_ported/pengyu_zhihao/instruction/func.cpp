// Sample code for calling Matlab function in C++
// Zhihao Zheng 10/2015
// zhihaozh@brandeis.edu
#include "func.h"
#include "libvaa_port_test.h"
#include <iostream>

mxClassID get_mxClass(int unit_bytes){
	 
	switch (unit_bytes)
	{
	case 1:	return mxUINT8_CLASS;
	case 2: return mxUINT16_CLASS;
	case 4: return mxSINGLE_CLASS;
	case 8: return mxINT64_CLASS;
	default:
		std::cerr << "Error: Unknown Unit Bytes " << unit_bytes << std::endl;
		break;
	}
}

bool* shared_lib_func(unsigned char* raw, V3DLONG total_bytes, V3DLONG unit_bytes, V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG t,int paran, double* para,std::string fileDir){
	
	std::cerr << "Matlab Part Start." << std::endl;

	// Initialize the MATLAB Compiler Runtime global state
	/* For windows
    if (!mclInitializeApplication(NULL, 0))
	{
		std::cerr << "Could not initialize the application properly." << mclGetLastErrorMessage() << std::endl;
		return -1;
	}*/
	// Initialize the Vigenere library
	if (!libvaa_port_testInitialize())
	{
		std::cerr << "Could not initialize the library properly." << mclGetLastErrorMessage() <<std::endl;
		return -1;
	}
	std::cout << "Intialization Finished" << std::endl;
	mxArray *mx_output = NULL;
	size_t raw_dim[1] = {x*y*z*t};
	size_t dim[1] = { 1 };
	size_t dim_para[1];
	dim_para[0] = paran;

	mxArray *mx_raw, *mx_unit_bytes, *mx_x, *mx_y, *mx_z, *mx_t, *mx_para,*mx_fileDir;

	unsigned char *dynamic_raw = (unsigned char *)mxCalloc(total_bytes, sizeof(UINT8_T));
	for (V3DLONG i = 0; i < total_bytes; i++){
		dynamic_raw[i] = raw[i];
	}
	mx_raw = mxCreateUninitNumericArray(1, raw_dim, get_mxClass(unit_bytes),mxREAL);
	mxSetData(mx_raw, dynamic_raw);
	
	V3DLONG *dynamic_unit_bytes = (V3DLONG *)mxCalloc(1, sizeof(V3DLONG));
	dynamic_unit_bytes[0] = unit_bytes;
	mx_unit_bytes = mxCreateUninitNumericArray(1, dim, get_mxClass(8), mxREAL);
	mxSetData(mx_unit_bytes, dynamic_unit_bytes);

	V3DLONG *dynamic_x = (V3DLONG *)mxCalloc(1, sizeof(V3DLONG));
	dynamic_x[0] = x;
	mx_x = mxCreateUninitNumericArray(1, dim, get_mxClass(8), mxREAL);
	mxSetData(mx_x, dynamic_x);

	V3DLONG *dynamic_y = (V3DLONG *)mxCalloc(1, sizeof(V3DLONG));
	dynamic_y[0] = y;
	mx_y = mxCreateUninitNumericArray(1, dim, get_mxClass(8), mxREAL);
	mxSetData(mx_y, dynamic_y);

	V3DLONG *dynamic_z = (V3DLONG *)mxCalloc(1, sizeof(V3DLONG));
	dynamic_z[0] = z;
	mx_z = mxCreateUninitNumericArray(1, dim, get_mxClass(8), mxREAL);
	mxSetData(mx_z, dynamic_z);

	V3DLONG *dynamic_t = (V3DLONG *)mxCalloc(1, sizeof(V3DLONG));
	dynamic_t[0] = t;
	mx_t = mxCreateUninitNumericArray(1, dim, get_mxClass(8), mxREAL);
	mxSetData(mx_t, dynamic_t);

	double *dynamic_para = (double *)mxCalloc(paran, sizeof(double));
	for (int i = 0; i < paran; i++){
		dynamic_para[i] = para[i];
	}
	mx_para = mxCreateUninitNumericArray(1, dim_para, mxDOUBLE_CLASS, mxREAL);
	mxSetData(mx_para, dynamic_para);

    mx_fileDir = mxCreateString(fileDir.c_str());
	std::cout << "Assignment Finished\n";
    //Call matlab function
	mlfVaa3d_trace3D(1, &mx_output, mx_raw, mx_unit_bytes, mx_x, mx_y, mx_z, mx_t, mx_para,mx_fileDir);
    
	// Shut down the library and the application global state.
    
	bool* output = mxGetLogicals(mx_output);
	
	mxDestroyArray(mx_raw);
	std::cout << "free mx raw\n";

	mxDestroyArray(mx_unit_bytes);
	std::cout << "free mx unit\n";
	mxDestroyArray(mx_x);
	std::cout << "free mx x\n";
	mxDestroyArray(mx_y);
	std::cout << "free mx y\n";
	mxDestroyArray(mx_z);
	std::cout << "free mx z\n";
	mxDestroyArray(mx_t);
	std::cout << "free mx t\n";
	mxDestroyArray(mx_para);
	std::cout << "free mx para\n";
	libvaa_port_testTerminate();
	//For windows
    //mclTerminateApplication();
	std::cout << "Matlab Part Finished." << std::endl;
	return output;
}
