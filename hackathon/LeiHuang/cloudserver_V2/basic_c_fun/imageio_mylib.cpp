/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).  
 * All rights reserved.
 */


/************
 ********* LICENSE NOTICE ************
 
 This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it. 
 
 You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.
 
 1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.
 
 2. You agree to appropriately cite this work in your related studies and publications.
 
 Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )
 
 Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )
 
 3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.
 
 4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.
 
 *************/




//100817. by PHC
//last change 2013-11-02 by PHC. add single slice reading function

#include <stdio.h>
#include <string.h>

#include "v3d_basicdatatype.h"
#include "v3d_message.h"
#include "imageio_mylib.h"

extern "C" {
#include "../common_lib/src_packages/mylib_tiff/image.h"
};

void freeMylibArray(Array * indata)
{
    if (indata)
    {
        Kill_Array(indata);
        indata=0;
    } //still need to free space
}

void freeMylibBundle(Layer_Bundle * indata)
{
	if (indata && indata->layers)
	{
		for (V3DLONG n=0; n<indata->num_layers; n++)
			if (indata->layers[n]) {Kill_Array(indata->layers[n]); indata->layers[n]=0;} //still need to free space
		
		//do NOT free the bundle pointer itself, as Gene used the static pointer for the bundle!
	}
}

int loadTif2StackMylib(char * filename, unsigned char * & img, V3DLONG * & sz, int & datatype, int &nbits)
{
	if (!filename) 
		return 1;
	
	//read data
	V3DLONG n; //n for the number of layers
	
	Layer_Bundle * indata = Read_Images(filename); //not use Read_Image as there could be multiple layers
	if (!indata || !indata->layers)
	{
		fprintf(stderr, "************* MYLIB Error MSG: [%s]\n", Image_Error());
		return 1;
	}
	else
	{
		for (n=0;n<indata->num_layers;n++)
			if (indata->layers[n] == NULL || indata->layers[n]->data == NULL)
			{
				fprintf(stderr, "************* MYLIB data structure corrupted. MYLIB error msg [%s]\n", Image_Error());
				freeMylibBundle(indata);
				return 1;
			}
	}
	
	//find the number of channels in the output data
	V3DLONG nchannels=0;
	for (n=0;n<indata->num_layers;n++)
	{
		if (indata->layers[n]->type != UINT8 && indata->layers[n]->type != UINT16 && indata->layers[n]->type != FLOAT32)
		{
			fprintf(stderr, "************* MYLIB returns a data type that is not oen of UINT8, UINT16, and FLOAT32. V3D cannot convert. Addition MYLIB error msg [%s].\n", Image_Error());
			freeMylibBundle(indata);
			return 1;
		}
		
		if (indata->layers[n]->ndims>4)
		{
			fprintf(stderr, "************* MYLIB returns a 5D or more-dimensional array. V3D cannot convert. Addition MYLIB error msg [%s].\n", Image_Error());
			freeMylibBundle(indata);
			return 1;
		}
		
		if (indata->layers[n]->ndims<4)
			nchannels += 1;
		else //==4
			nchannels += indata->layers[n]->dims[3]; 
		
		//also do a redundant verification of the datatype & dims (Gene indicated this not needed, but I still add in case an error which will cause a crash)
		if (n>0)
		{
			if (indata->layers[n]->type != indata->layers[n-1]->type || indata->layers[n]->scale != indata->layers[n-1]->scale || indata->layers[n]->kind != indata->layers[n-1]->kind)
			{
				fprintf(stderr, "************* MYLIB returns layers which have different datatypes/number-of-bits-per-pixels/kind. V3D cannot convert. Addition MYLIB error msg [%s].\n", Image_Error());
				freeMylibBundle(indata);
				return 1;
			}
			
			if (indata->layers[n]->ndims != indata->layers[n-1]->ndims)
			{
				fprintf(stderr, "************* MYLIB returns layers which have different # of dimensions. V3D cannot convert. Addition MYLIB error msg [%s].\n", Image_Error());
				fprintf(stderr, "************* layers %ld #dimensions = [%ld] and layer %ld #dimensions = [%ld] do NOT match.\n", n, (V3DLONG)(indata->layers[n]->ndims), n-1, (V3DLONG)(indata->layers[n-1]->ndims));
				freeMylibBundle(indata);
				return 1;
			}
			
			for (V3DLONG j=0;j<indata->layers[n]->ndims;j++)
			{
				if (indata->layers[n]->dims[j] != indata->layers[n-1]->dims[j])
				{
					fprintf(stderr, "************* MYLIB returns layers which have different dimensions (sizes). V3D cannot convert. Addition MYLIB error msg [%s].\n", Image_Error());
					fprintf(stderr, "************* layers %ld dimension %ld [%ld] and layer %ld dimension %ld [%ld] do NOT match.\n", n, j, (V3DLONG)(indata->layers[n]->dims[j]), n-1, j, (V3DLONG)(indata->layers[n-1]->dims[j]));
					freeMylibBundle(indata);
					return 1;
				}
			}
		}
	}
	
	//prepare the output buffer
	if (img)
	{
		printf("Warning: The pointer for 1d data storage is not empty. This pointer will be freed first and the  reallocated. \n");
		delete []img; img=0;
	}
	if (sz)
	{
		printf("Warning: The pointer for size is not empty. This pointer will be freed first and the  reallocated. \n");
		delete []sz; sz=0;
	}

	try
	{
		sz = new V3DLONG [4];
	}
	catch(...)
	{
		printf("Fail to alocate memory for the size variable.\n");
		return false;
	}
	
	//now copy data
	
	switch (indata->layers[0]->type)
	{
		case UINT8:		datatype=1; break;
		case UINT16:	datatype=2; break;
		case FLOAT32:	datatype=4; break;
		default:
			fprintf(stderr, "Unsupport data type detected in loadTif2StackMylib(). You should never see this message. Contact V3D developers!.\n");
			return 1;
	}
	
	nbits = indata->layers[0]->scale;
	
	V3DLONG i, unitsPerLayer=1;
	V3DLONG upper = (indata->layers[0]->ndims);
	if (upper>3) upper = 3; 
	for (i=0;i<upper;i++)
	{
		sz[i] = indata->layers[0]->dims[i];
		unitsPerLayer *= sz[i];
	}
	for (i=upper;i<3;i++)
	{
		sz[i] = 1;
		// since sz[i] is 1, then no need to multiply to totalunits
	}
	sz[3] = nchannels; 
	fprintf(stdout, "The dimensions of the image will be [%ld %ld %ld %ld].\n", sz[0], sz[1], sz[2], sz[3]);
	
	V3DLONG lengthPerLayers = unitsPerLayer * datatype;
	V3DLONG totallen = lengthPerLayers * sz[3];
	try
	{
		img = new unsigned char [totallen];
	}
	catch(...)
	{
		fprintf(stderr, "Fail to allocate memory in loadTif2StackMylib().");
		if (sz) {delete []sz; sz=0;}
		return 1;
	}
	
	unsigned char *img1;
	for (n=0, img1=img; n<indata->num_layers; n++)
	{
		printf("Now copy layer %ld's data...\n", n);
		memcpy(img1, (unsigned char*)indata->layers[n]->data, (V3DLONG)(indata->layers[n]->size)*datatype); //copy data
		img1 += (V3DLONG)(indata->layers[n]->size)*datatype;
	}

	//free space
	freeMylibBundle(indata);
	return 0;
}

int loadTif2StackMylib_slice(char * filename, unsigned char * & img, V3DLONG * & sz, int & datatype, int &nbits, V3DLONG zsliceno)
{
    if (!filename)
        return 1;

//    V3DLONG n = get_Tiff_Depth_mylib(filename);

    //read data

//commented for now to avoid builing error for some systems. by PHC 20131105    
  Layer_Bundle * indata0=0;
//Layer_Bundle * indata0 = read_One_Tiff_ZSlice(filename, zsliceno);
    //if (!indata0 || !indata0->layers)
        return 1;

    Array * indata = indata0->layers[0];
    if (!indata)
    {
        fprintf(stderr, "************* MYLIB Error MSG: [%s]\n", Image_Error());
        return 1;
    }

    //find the number of channels in the output data
    V3DLONG nchannels=0;
    {
        if (indata->type != UINT8 && indata->type != UINT16 && indata->type != FLOAT32)
        {
            fprintf(stderr, "************* MYLIB returns a data type that is not oen of UINT8, UINT16, and FLOAT32. V3D cannot convert. Addition MYLIB error msg [%s].\n", Image_Error());
            freeMylibArray(indata);
            return 1;
        }

        if (indata->ndims>4)
        {
            fprintf(stderr, "************* MYLIB returns a 5D or more-dimensional array. V3D cannot convert. Addition MYLIB error msg [%s].\n", Image_Error());
            freeMylibArray(indata);
            return 1;
        }

        if (indata->ndims < 4)
            nchannels += 1;
        else //==4
            nchannels += indata->dims[3];

        //also do a redundant verification of the datatype & dims (Gene indicated this not needed, but I still add in case an error which will cause a crash)
    }

    //prepare the output buffer
    if (img)
    {
        printf("Warning: The pointer for 1d data storage is not empty. This pointer will be freed first and the  reallocated. \n");
        delete []img; img=0;
    }
    if (sz)
    {
        printf("Warning: The pointer for size is not empty. This pointer will be freed first and the  reallocated. \n");
        delete []sz; sz=0;
    }

    try
    {
        sz = new V3DLONG [4];
    }
    catch(...)
    {
        printf("Fail to alocate memory for the size variable.\n");
        return false;
    }

    //now copy data

    switch (indata->type)
    {
        case UINT8:		datatype=1; break;
        case UINT16:	datatype=2; break;
        case FLOAT32:	datatype=4; break;
        default:
            fprintf(stderr, "Unsupport data type detected in loadTif2StackMylib_slice(). You should never see this message. Contact V3D developers!.\n");
            return 1;
    }

    nbits = indata->scale;

    V3DLONG i, unitsPerLayer=1;
    V3DLONG upper = (indata->ndims);
    if (upper>3) upper = 3;
    for (i=0;i<upper;i++)
    {
        sz[i] = indata->dims[i];
        unitsPerLayer *= sz[i];
    }
    for (i=upper;i<3;i++)
    {
        sz[i] = 1;
        // since sz[i] is 1, then no need to multiply to totalunits
    }
    sz[3] = nchannels;
    fprintf(stdout, "The dimensions of the image will be [%ld %ld %ld %ld].\n", sz[0], sz[1], sz[2], sz[3]);

    V3DLONG lengthPerLayers = unitsPerLayer * datatype;
    V3DLONG totallen = lengthPerLayers * sz[3];
    try
    {
        img = new unsigned char [totallen];
    }
    catch(...)
    {
        fprintf(stderr, "Fail to allocate memory in loadTif2StackMylib_slice().");
        if (sz) {delete []sz; sz=0;}
        return 1;
    }

    {
        printf("Now copy slice %ld's data...\n", zsliceno);
        memcpy(img, (unsigned char*)indata->data, (V3DLONG)(indata->size)*datatype); //copy data
    }

    //free space
    freeMylibArray(indata);
    return 0;
}

int loadTif2StackMylib(char * filename, unsigned char * & img, V3DLONG * & sz, int & datatype, int & nbits, int chan_id_to_load) //overload for convenience to read only 1 channel
{
	return 0;
}
int saveStack2TifMylib(const char * filename, const unsigned char * img, const V3DLONG * sz, int datatype)
{
	return 0;
}






