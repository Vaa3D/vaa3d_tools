#include "ioV3dUtils.h"


void rescaleForDisplay(double* input, double* output, int numVoxels, ImagePixelType dataType)
{
	int maxValType;
	switch (dataType)
	{
		case V3D_UINT8:
    		maxValType = 255;
    	break;
    	case V3D_UINT16:
    			maxValType = 65535;
    	break;
    	case V3D_FLOAT32:
    		maxValType = 3.402823466E+38;
    	break;
    	case V3D_UNKNOWN:
    	default:
			maxValType = 255;
	}
	
	double max = 0;
	double min = 0;
	for (int i = 0; i < numVoxels; i++)
	{
 		max = (max < input[i]) ? input[i] : max;	
 		min = (min > input[i]) ? input[i] : min;	
	}
	for (int i = 0; i < numVoxels; i++) {
		output[i] = round(maxValType * (input[i] - min) / (max - min));
	}
}

unsigned char* doubleArrayToCharArray(double* data1dD, int numVoxels, ImagePixelType dataType)
{
  	unsigned char* dataOut1d;
    
	// output the processed image
    switch(dataType)
    {
    	case V3D_UINT8:
    	{
    	dataOut1d = new unsigned char[numVoxels];
    	uint8* tmp1 = (uint8*) dataOut1d;
    	for(V3DLONG i = 0; i < numVoxels; i++)
    	{
    		tmp1[i] = (uint8) round(data1dD[i]);
    	}
    	}
    	break;
    	case V3D_UINT16:
    	{
    	dataOut1d = new unsigned char[2*numVoxels];
    	uint16* tmp2 = (uint16*) dataOut1d;
    	for(V3DLONG i = 0; i < numVoxels; i++)
    	{
    		tmp2[i] = (uint16) round(data1dD[i]);
    	}
    	}
    	break;
    	case V3D_FLOAT32:
    	{
    	dataOut1d = new unsigned char[4*numVoxels];
    	float32* tmp3 = (float32*) dataOut1d;
    	for(V3DLONG i = 0; i < numVoxels; i++)
    	{
    		tmp3[i] = (float32) data1dD[i];
    	}
    	}
    	break;
    	case V3D_UNKNOWN:
    	default:
			return NULL;
    }
    
    return dataOut1d;
}

double* channelToDoubleArray(Image4DSimple* inputImage, int channel)
{
    unsigned char* data1d = inputImage->getRawData();//get raw data
    //V3DLONG totalpxls = p4DImage->getTotalBytes();
    V3DLONG pagesz = inputImage->getTotalUnitNumberPerChannel();
	//get dims
    V3DLONG szx = inputImage->getXDim();
    V3DLONG szy = inputImage->getYDim();
    V3DLONG szz = inputImage->getZDim();
    V3DLONG sc = inputImage->getCDim();
    V3DLONG N = szx * szy * szz;

	//convert first channel to double values
    double* data1dD = new double[N]; //(double*)malloc(sizeof(double)*N*M*P);
	int offset = (channel-1)*N;
	switch(inputImage->getDatatype())
    {
    	case V3D_UINT8:
    	{
    	uint8* tmp = (uint8*) data1d;
    	for(V3DLONG i = 0; i < pagesz; i++)
    	{
    		data1dD[i] = tmp[i+offset];
    	}
    	}
    	break;
    	case V3D_UINT16:
    	{
    	uint16* tmp1 = (uint16*) data1d;
    	for(V3DLONG i = 0; i < pagesz; i++)
    	{
    		data1dD[i] = tmp1[i+offset];
    	}
    	}
    	break;
    	case V3D_FLOAT32:
    	{
    	float32* tmp2 = (float32*) data1d;
    	for(V3DLONG i = 0; i < pagesz; i++)
    	{
    		data1dD[i] = tmp2[i+offset];
    	}
    	}
    	break;
    	case V3D_UNKNOWN:
    	default:
        delete [] data1dD;
			return NULL;
    }
    return data1dD;
}
