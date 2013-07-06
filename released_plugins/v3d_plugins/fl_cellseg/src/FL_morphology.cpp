/* morphology operations
 * F. Long
  define structural elements and morphology operations, dilate, erode, open, close, and fill
 * F. Long
 * combine FL_morphology_old.h and FL_morphology_old.cpp, for template, interface and implementation need to be in the same file
 */

#ifndef __FL_MORPHOLOGY_CPP__
#define __FL_MORPHOLOGY_CPP__

#include "FL_morphology.h"

Strel::Strel()
{

  its_offset = 0;
  its_num_neighbors = 0;
  its_array_sz = 0;
  its_num_dims  = 0;
//  its_mask = 0;
}

Strel::~Strel()
{

  if (its_offset) {delete [] its_offset; its_offset = 0;}
  if (its_array_sz)	{delete [] its_array_sz; its_array_sz = 0;}
  
}
  

// print the structrual element information, not right
void Strel::printStrelInfo()
{
  V3DLONG i, j;
  
//  for (i = 0; i < its_num_neighbors; i++) 
//  {
//	printf("[%d, %d, %d]\n", its_offset[i*its_num_dims], its_offset[i*its_num_dims+1],its_offset[i*its_num_dims+2]);
//	printf("[");
//	for (j=0; j<its_num_dims; j++)
//		printf("%d ", its_offset[i*its_num_dims+j]);
//	printf("]\n");	
//  }
  
  printf("Dimensions: %ld\n", its_num_dims);
  
  printf("Size: ["); 
  for (i = 0; i < its_num_dims; i++) 
  {
	printf("%ld ", its_array_sz[i]);
  }
  printf("]\n");
  
  printf("Number of elements: %ld\n", its_num_neighbors);
  
}
 
// create disk structural element
void Strel::createDiskStrel(const V3DLONG radius)
{


    V3DLONG len = radius * 2 + 1;
    V3DLONG len2 = radius*radius;
    V3DLONG i, j;
    V3DLONG count = -1;


//    its_offset = new V3DLONG [len*len];
    its_offset = new V3DLONG [len*len*2];
    
    if (its_offset==0)
        printf("allocate memory for its_offset fail.\n");

    its_array_sz = new V3DLONG [2];

    if (its_array_sz==0)
        printf("allocate memory for its_array_sz fail.\n");
    
    its_array_sz[0]=its_array_sz[1]= len;
    its_num_dims  = 2;

    //  printf("%d, %d\n", its_array_sz[0], its_array_sz[1]);


    for (j = -radius; j <= radius; j++) 
    {
      for (i = -radius; i <= radius; i++) 
      {
            if (i * i + j * j <= len2) 
            {
                its_offset[count++] = i;
                its_offset[count++] = j;
    //				se->mask[n] = 1;
    //				printf("[%d, %d]\n", i,j);
                
            }
      }
    }

    its_num_neighbors = (count+1)/its_num_dims;
  
}
 

// create ball structural element
void Strel::createBallStrel(const V3DLONG radius)
{
    
    V3DLONG len = radius * 2 + 1;
    V3DLONG len2 = radius*radius;
    V3DLONG i, j, k;
    V3DLONG count = -1;


//    its_offset = new V3DLONG [len*len*len];
    its_offset = new V3DLONG [len*len*len*3];
    
    if (its_offset==0)
        printf("allocate memory for its_offset fail.\n");
        
    its_array_sz = new V3DLONG [3];

    if (its_array_sz==0)
    {
        printf("allocate memory for its_array_sz fail.\n");
    }

    its_array_sz[0]=its_array_sz[1]=its_array_sz[2] = len;
    its_num_dims  = 3;

    for (k = -radius; k <= radius; k++)
    {
    for (j = -radius; j <= radius; j++) 
    {
      for (i = -radius; i <= radius; i++) 
      {
            if (i * i + j * j + k * k <= len2) 
            {
                count++;
                its_offset[count] = i;
                
                count++;
                its_offset[count] = j;
                
                count++;
                its_offset[count] = k;
    //				se->mask[n] = 1;
            }
      }
    }
    }

    its_num_neighbors = (count+1)/its_num_dims;

    printf("%ld, %ld, %ld\n", its_array_sz[0], its_array_sz[1], its_array_sz[2]);
    
    
}


//create cuboid structural element
void Strel::creatRectangleStrel(const V3DLONG *rectangle_sz)
{
	V3DLONG len_x = (rectangle_sz[0]-1)/2;
	V3DLONG len_y = (rectangle_sz[1]-1)/2;
	V3DLONG len2 = rectangle_sz[0]*rectangle_sz[1];
	V3DLONG i, j;
	V3DLONG count = -1;


	its_offset = new V3DLONG [len2*2];
	its_array_sz = new V3DLONG [2];
	its_array_sz[0] = rectangle_sz[0];
	its_array_sz[1]= rectangle_sz[0];
	its_num_dims  = 2;

	for (j = -len_y; j <= len_y; j++) 
	{
	  for (i = -len_x; i <= len_x; i++) 
	  {
			its_offset[count++] = i;
			its_offset[count++] = j;
				
	//				se->mask[n] = 1;
	  }
	}
 
   its_num_neighbors = count;
}

//create cuboid structural element
void Strel::creatCuboidStrel(const V3DLONG *cuboid_sz)
{
	V3DLONG len_x = (cuboid_sz[0]-1)/2;
	V3DLONG len_y = (cuboid_sz[1]-1)/2;
	V3DLONG len_z = (cuboid_sz[2]-1)/2;  
	V3DLONG len2 = cuboid_sz[0]*cuboid_sz[1]*cuboid_sz[2];
	V3DLONG i, j, k;
	V3DLONG count = -1;


	its_offset = new V3DLONG [len2*3];
	its_array_sz = new V3DLONG [3];
	its_array_sz[0] = cuboid_sz[0];
	its_array_sz[1] = cuboid_sz[1];
	its_array_sz[2] = cuboid_sz[2];
	its_num_dims  = 3;

	for (k = -len_y; k <= len_y; k++) 
	{
		for (j = -len_y; j <= len_y; j++) 
		{
		  for (i = -len_x; i <= len_x; i++) 
		  {
				its_offset[count++] = i;
				its_offset[count++] = j;
				its_offset[count++] = k;
				
	//				se->mask[n] = 1;
		  }
		}
	}
 
  its_num_neighbors = count;
}

Neighborhood * Strel::convertStrelToNeighbor()
{
    
    
	Neighborhood * nh = new Neighborhood;
	V3DLONG i,j;
	
	if (!nh)
	{
		fprintf(stderr, "nh must be NULL\n");
		return NULL;
	}
	
    
//	for (j=0; j<its_num_dims; j++)
//    {
//		printf("*****j= %d\n", j);
//		printf("*****its_array_sz = %d\n", its_array_sz[j]);
//    }
    
    
//	nh = new Neighborhood(se->its_mask, se->its_array_sz, se->its_num_dims, NH_CENTER_MIDDLE_ROUNDUP);
	nh->its_num_neighbors = its_num_neighbors;
	nh->its_num_dims = its_num_dims;
	
	nh->its_array_coords = new V3DLONG [nh->its_num_neighbors * nh-> its_num_dims];
	nh->its_array_sz = new V3DLONG [nh->its_num_dims];
	 
	for(i=0; i<nh->its_num_neighbors; i++)
		for (j=0; j<nh->its_num_dims; j++)
				nh->its_array_coords[i * nh-> its_num_dims+j] = its_offset[i * nh-> its_num_dims+j];
	
	for (j=0; j<nh->its_num_dims; j++)
    {
//		printf("j= %d\n", j);
//		printf("its_array_sz = %d\n", its_array_sz[j]);
        
        nh->its_array_sz[j] = its_array_sz[j];
//		printf("j= %d, nh->its_array_sz = %d\n", j, nh->its_array_sz[j]);
        
        
    }
	
	return nh;
			
}


#endif

