// cell segmentation based on gradient vector field
// ref. 
// F. Long revised
// 20080902

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "./local_basic_c_fun/img_definition.h"
#include "./local_basic_c_fun/stackutil.h"

#include "FL_gvfCellSeg.h"
#include "FL_defType.h"
#include "FL_volimgProcLib.h"

//int x_size;
//int y_size;
//int z_size;
//int volume;
//V3DLONG *sz = 0;


typedef unsigned char byte;
 
typedef struct 
{
  float x;
  float y;
  float z;
} Fvector3d;

typedef struct 
{
  int x;
  int y;
  int z;
} Ivector3d;



int **Ialloc2d(int i_size,int j_size)
{
	int **array;
	int i;

	array=(int **) calloc(i_size,sizeof(int *));

	for(i=0;i<i_size;i++)
		array[i]=(int *) calloc(j_size,sizeof(int ));

	return(array);
}

void Ifree2d(int **array,int i_size)
{
	int i;

	for(i=0;i<i_size;i++)
		free(array[i]);

	free(array);
}


unsigned char ***UCalloc3d(int i_size,int j_size,int k_size)
{
	unsigned char ***array;
	int i,k;

	array=(unsigned char ***) calloc(k_size,sizeof(unsigned char **));

	for(k=0;k<k_size;k++)
		array[k]=(unsigned char **) calloc(i_size,sizeof(unsigned char *));

	for(k=0;k<k_size;k++)
	for(i=0;i<i_size;i++)
		array[k][i]=(unsigned char *) calloc(j_size,sizeof(unsigned char ));
	
	return(array);
}

void UCfree3d(unsigned char ***array,int k_size,int i_size)
{
	int k,i;

	for(k=0;k<k_size;k++)
	for(i=0;i<i_size;i++)
		free(array[k][i]);

	for(k=0;k<k_size;k++)
		free(array[k]);

	free(array);
}

float ***Falloc3d(int i_size,int j_size,int k_size)
{
	float ***array;
	int i,k;

	array=(float ***) calloc(k_size,sizeof(float **));

	for(k=0;k<k_size;k++)
		array[k]=(float **) calloc(i_size,sizeof(float *));

	for(k=0;k<k_size;k++)
    for(i=0;i<i_size;i++)
      array[k][i]=(float *) calloc(j_size,sizeof(float ));
	
	return(array);
}

void Ffree3d(float ***array,int k_size,int i_size)
{
	int k,i;

	for(k=0;k<k_size;k++)
    for(i=0;i<i_size;i++)
		free(array[k][i]);

	for(k=0;k<k_size;k++)
		free(array[k]);

	free(array);
}

Fvector3d ***Fvector3dalloc3d(int i_size,int j_size,int k_size)
{
	Fvector3d ***array;
	int i,k;

	array=(Fvector3d ***) calloc(k_size,sizeof(Fvector3d **));

	for(k=0;k<k_size;k++)
		array[k]=(Fvector3d **) calloc(i_size,sizeof(Fvector3d *));

	for(k=0;k<k_size;k++)
    for(i=0;i<i_size;i++)
		array[k][i]=(Fvector3d *) calloc(j_size,sizeof(Fvector3d ));
	
	return(array);
}

void Fvector3dfree3d(Fvector3d ***array,int k_size,int i_size)
{
	int k,i;

	for(k=0;k<k_size;k++)
    for(i=0;i<i_size;i++)
		free(array[k][i]);

	for(k=0;k<k_size;k++)
		free(array[k]);

	free(array);
}

int ***Ialloc3d(int i_size,int j_size,int k_size)
{
	int ***array;
	int i,k;

	array=(int ***) calloc(k_size,sizeof(int **));

	for(k=0;k<k_size;k++)
		array[k]=(int **) calloc(i_size,sizeof(int *));

	for(k=0;k<k_size;k++)
    for(i=0;i<i_size;i++)
      array[k][i]=(int *) calloc(j_size,sizeof(int ));
	
	return(array);
}

void Ifree3d(int ***array,int k_size,int i_size)
{
	int k,i;

	for(k=0;k<k_size;k++)
    for(i=0;i<i_size;i++)
		free(array[k][i]);

	for(k=0;k<k_size;k++)
		free(array[k]);

	free(array);
}

Ivector3d ***Ivector3dalloc3d(int i_size,int j_size,int k_size)
{
	Ivector3d ***array;
	int i,k;

	array=(Ivector3d ***) calloc(k_size,sizeof(Ivector3d **));

	for(k=0;k<k_size;k++)
		array[k]=(Ivector3d **) calloc(i_size,sizeof(Ivector3d *));

	for(k=0;k<k_size;k++)
    for(i=0;i<i_size;i++)
		array[k][i]=(Ivector3d *) calloc(j_size,sizeof(Ivector3d ));
	
	return(array);
}

void Ivector3dfree3d(Ivector3d ***array,int k_size,int i_size)
{
	int k,i;

	for(k=0;k<k_size;k++)
    for(i=0;i<i_size;i++)
		free(array[k][i]);

	for(k=0;k<k_size;k++)
		free(array[k]);

	free(array);
}


void read_image_byte(char *filename, float ***image, int x_size, int y_size, int z_size)
{
	int x,y,z;
	byte temp;
	FILE *fp;

	fp = fopen(filename,"rb");

	if(!fp)
	{
		printf("error: cannot open image file!\n");
		
		exit(1);
	}
	
	for(z=0;z<z_size;z++)
	for(x=0;x<x_size;x++)	
	for(y=0;y<y_size;y++)
	{
		fread(&temp,sizeof(byte),1,fp);
		image[z][x][y] = (float)temp;
	}

	fclose(fp);

	return;
}

void write_image_byte(char *filename, byte ***image, int x_size, int y_size, int z_size)
{
	int x,y,z;
	byte temp;

	FILE *fp;

	fp=fopen(filename,"wb");

	for(z=0;z<z_size;z++)
	for(x=0;x<x_size;x++)
	for(y=0;y<y_size;y++)
	{
		temp = image[z][x][y];

		fwrite(&temp,sizeof(byte),1,fp);
	}

	fclose(fp);

	return;
}

void write_image_float(char *filename, float ***image, int x_size, int y_size, int z_size)
{
	int x,y,z;
	float temp;

	FILE *fp;

	fp=fopen(filename,"wb");

	for(z=0;z<z_size;z++)
	for(x=0;x<x_size;x++)
	for(y=0;y<y_size;y++)
	{
		temp = image[z][x][y];
		fwrite(&temp,sizeof(float),1,fp);
	}

	fclose(fp);

	return;
}

void write_image_int(char *filename, int ***image, int x_size, int y_size, int z_size)
{
	int x,y,z;
	int temp;

	FILE *fp;

	fp=fopen(filename,"wb");

	for(z=0;z<z_size;z++)
	for(x=0;x<x_size;x++)
	for(y=0;y<y_size;y++)
	{
		temp = image[z][x][y];
		fwrite(&temp,sizeof(int),1,fp);
	}

	fclose(fp);

	return;
}

void write_image_binary(char *filename, int ***image, int x_size, int y_size, int z_size)
{
	int x,y,z;
	unsigned char temp;

	FILE *fp;

	fp=fopen(filename,"wb");

	for(z=0;z<z_size;z++)
	for(x=0;x<x_size;x++)
	for(y=0;y<y_size;y++)
	{
		if( image[z][x][y]>0 )
			temp = 255;
		else
			temp = 0;

		fwrite(&temp,sizeof(unsigned char),1,fp);
	}

	fclose(fp);

	return;
}


void write_filed( char *filename, Fvector3d ***field, int x_size, int y_size, int z_size)
{
	int x, z;

	FILE *fp;

	fp=fopen(filename,"wb");

 	for(z=0;z<z_size;z++)
	for(x=0;x<x_size;x++)
	{
		fwrite(field[z][x],sizeof(Fvector3d),y_size,fp);
	}

	fclose(fp);

	return;
}


// smooth image
void smooth_image(float ***image, int ITER, int x_size, int y_size, int z_size)
{
	int x, y, z, i;

	float ***simage;

	simage = Falloc3d(x_size,y_size,z_size);

	for(i=0;i<ITER;i++)
	{
		for(z=0;z<z_size;z++)
		for(x=0;x<x_size;x++)
		for(y=0;y<y_size;y++)
			simage[z][x][y] = 0;

		for(z=1;z<z_size-1;z++)
		for(x=1;x<x_size-1;x++)
		for(y=1;y<y_size-1;y++)
		{
			simage[z][x][y] = 0.4*image[z][x][y] + \
				0.1*(image[z-1][x][y]+image[z+1][x][y]+image[z][x-1][y]+image[z][x+1][y]+image[z][x][y-1]+image[z][x][y+1]);
		}

		for(z=0;z<z_size;z++)
		for(x=0;x<x_size;x++)
		for(y=0;y<y_size;y++)
			image[z][x][y] = simage[z][x][y];
	}
	
	Ffree3d(simage,z_size,x_size);

	return;
}


//compute gradient image
void gradient(float ***image, Fvector3d ***grad, byte ***I, int x_size, int y_size, int z_size)
{
	int x,y,z;
	float maxgrad,tempgrad;
	int pos,neg;

	for(z=0;z<z_size;z++)
	for(x=0;x<x_size;x++)
	for(y=0;y<y_size;y++)
	{
		grad[z][x][y].x = 0;
		grad[z][x][y].y = 0;
		grad[z][x][y].z = 0;
		
		I[z][x][y] = 0;
	}
	
	printf("%d, %d, %d\n", z_size-1, x_size-1, y_size-1);
	
		
	for(z=1;z<z_size-1;z++)
	for(x=1;x<x_size-1;x++)
	for(y=1;y<y_size-1;y++)
	{
		grad[z][x][y].x = 0.5*(image[z][x+1][y] - image[z][x-1][y]); 
		grad[z][x][y].y = 0.5*(image[z][x][y+1] - image[z][x][y-1]);
		grad[z][x][y].z = 0.5*(image[z+1][x][y] - image[z-1][x][y]);
//		printf("%f, %f, %f\n", grad[z][x][y].x, grad[z][x][y].y, grad[z][x][y].z);
	}
	
	/*
	for(z=1;z<z_size-1;z++)
	for(x=1;x<x_size-1;x++)
	for(y=1;y<y_size-1;y++)
	{
		pos = 4*image[z][x+1][y] + 2*(image[z+1][x+1][y]+image[z-1][x+1][y]+image[z][x+1][y+1]+image[z][x+1][y-1]) + \
			(image[z+1][x+1][y+1] + image[z+1][x+1][y-1] + image[z-1][x+1][y+1] + image[z-1][x+1][y-1]);
		neg = 4*image[z][x-1][y] + 2*(image[z+1][x-1][y]+image[z-1][x-1][y]+image[z][x-1][y+1]+image[z][x-1][y-1]) + \
			(image[z+1][x-1][y+1] + image[z+1][x-1][y-1] + image[z-1][x-1][y+1] + image[z-1][x-1][y-1]);
 
		grad[z][x][y].x = pos - neg; 
		
		pos = 4*image[z][x][y+1] + 2*(image[z+1][x][y+1]+image[z-1][x][y+1]+image[z][x+1][y+1]+image[z][x-1][y+1]) + \
			(image[z+1][x+1][y+1]+image[z-1][x+1][y+1]+image[z+1][x-1][y+1]+image[z-1][x-1][y+1]);
		
		neg = 4*image[z][x][y-1] + 2*(image[z+1][x][y-1]+image[z-1][x][y-1]+image[z][x+1][y-1]+image[z][x-1][y-1]) + \
			(image[z+1][x+1][y-1]+image[z-1][x+1][y-1]+image[z+1][x-1][y-1]+image[z-1][x-1][y-1]);

		grad[z][x][y].y = pos - neg;
		
		pos = 4*image[z+1][x][y] + 2*(image[z+1][x+1][y]+image[z+1][x-1][y]+image[z+1][x][y+1]+image[z+1][x][y-1]) + \
			(image[z+1][x+1][y+1]+image[z+1][x-1][y+1]+image[z+1][x+1][y-1]+image[z+1][x-1][y-1]);

		neg = 4*image[z-1][x][y] + 2*(image[z-1][x+1][y]+image[z-1][x-1][y]+image[z-1][x][y+1]+image[z-1][x][y-1]) + \
			(image[z-1][x+1][y+1]+image[z-1][x-1][y+1]+image[z-1][x+1][y-1]+image[z-1][x-1][y-1]);

		grad[z][x][y].z = pos - neg;

	}
	*/
	
	maxgrad = -1000;

	for(z=0;z<z_size;z++)
	for(x=0;x<x_size;x++)
	for(y=0;y<y_size;y++)
	{
		tempgrad = grad[z][x][y].x*grad[z][x][y].x + grad[z][x][y].y*grad[z][x][y].y + grad[z][x][y].z*grad[z][x][y].z; 
		
		if(tempgrad > maxgrad)
			maxgrad = tempgrad;
	}
	
	maxgrad = sqrt(maxgrad);
	maxgrad = maxgrad + 0.000000001;
	  
	for(z=0;z<z_size;z++)
	for(x=0;x<x_size;x++)
	for(y=0;y<y_size;y++)
	{
		grad[z][x][y].x = grad[z][x][y].x / maxgrad;
		grad[z][x][y].y = grad[z][x][y].y / maxgrad;
		grad[z][x][y].z = grad[z][x][y].z / maxgrad;

		tempgrad = sqrt(grad[z][x][y].x*grad[z][x][y].x + grad[z][x][y].y*grad[z][x][y].y + grad[z][x][y].z*grad[z][x][y].z);

		if(tempgrad > 0.001)
			I[z][x][y] = 1;
	}


	return;
}


void initialize_EDT(Fvector3d ***grad, byte ***I, Fvector3d ***f, Fvector3d ***u, int x_size, int y_size, int z_size)
{
	int x,y,z;
	
	for(z=0;z<z_size;z++)
	for(x=0;x<x_size;x++)
	for(y=0;y<y_size;y++)
	{
		f[z][x][y].x = 0;
		f[z][x][y].y = 0;
		f[z][x][y].z = 0;
	}

	for(z=1;z<z_size-1;z++)
	for(x=1;x<x_size-1;x++)
	for(y=1;y<y_size-1;y++)
	{
		if(I[z][x][y]==1)
		{
			f[z][x][y].x = x + grad[z][x][y].x;
			f[z][x][y].y = y + grad[z][x][y].y;
			f[z][x][y].z = z + grad[z][x][y].z;
		}
	}

	for(z=0;z<z_size;z++)
	for(x=0;x<x_size;x++)
	for(y=0;y<y_size;y++)
	{
		u[z][x][y].x = x;
		u[z][x][y].y = y;
		u[z][x][y].z = z;
	}

	return;
}


void initialize_GVF(Fvector3d ***grad, Fvector3d ***f, Fvector3d ***u, int x_size, int y_size, int z_size)
{
	int x,y,z;
	
	for(z=0;z<z_size;z++)
	for(x=0;x<x_size;x++)
	for(y=0;y<y_size;y++)
	{
		f[z][x][y].x = grad[z][x][y].x;
		f[z][x][y].y = grad[z][x][y].y;
		f[z][x][y].z = grad[z][x][y].z;

		u[z][x][y].x = 0.0;
		u[z][x][y].y = 0.0;
		u[z][x][y].z = 0.0;
	}

	return;
}


int warp_EDT(int ITER, float lambda, float mu, Fvector3d ***u, float omega, byte ***I, Fvector3d ***f, int x_size, int y_size, int z_size)
{
	int count=0,i,j,k;
	float temporary,resid,temp;

	while(count<ITER)
    {
	    for(k=1;k<z_size-1;k++)
		for(i=1;i<x_size-1;i++)
		for(j=1;j<y_size-1;j++)
	    {
			temp=(8*lambda+2*mu+I[k][i][j]);

			resid=-lambda*(u[k+1][i][j].x+u[k-1][i][j].x+u[k][i+1][j].x+u[k][i-1][j].x+u[k][i][j+1].x+u[k][i][j-1].x)+temp*u[k][i][j].x-f[k][i][j].x-(lambda+mu)*(u[k][i+1][j].x+u[k][i-1][j].x+u[k][i][j].y+u[k][i+1][j+1].y-u[k][i+1][j].y-u[k][i][j+1].y+u[k][i][j].z+u[k+1][i+1][j].z-u[k+1][i][j].z-u[k][i+1][j].z);
			temporary = omega*resid/temp;
			u[k][i][j].x -= temporary;
			
			resid=-lambda*(u[k+1][i][j].y+u[k-1][i][j].y+u[k][i+1][j].y+u[k][i-1][j].y+u[k][i][j+1].y+u[k][i][j-1].y)+temp*u[k][i][j].y-f[k][i][j].y-(lambda+mu)*(u[k][i][j+1].y+u[k][i][j-1].y+u[k][i][j].x+u[k][i+1][j+1].x-u[k][i+1][j].x-u[k][i][j+1].x+u[k][i][j].z+u[k+1][i][j+1].z-u[k+1][i][j].z-u[k][i][j+1].z);
			temporary=omega*resid/temp;
			u[k][i][j].y -=temporary;

			resid=-lambda*(u[k+1][i][j].z+u[k-1][i][j].z+u[k][i+1][j].z+u[k][i-1][j].z+u[k][i][j+1].z+u[k][i][j-1].z)+temp*u[k][i][j].z-f[k][i][j].z-(lambda+mu)*(u[k+1][i][j].z+u[k-1][i][j].z+u[k][i][j].x+u[k+1][i+1][j].x-u[k][i+1][j].x-u[k+1][i][j].x+u[k][i][j].y+u[k+1][i][j+1].y-u[k+1][i][j].y-u[k][i][j+1].y);
			temporary=omega*resid/temp;
			u[k][i][j].z -= temporary;

	    }
		
		count++;

		printf("iteration %d\n",count);
	} 

	for(k=1;k<z_size-1;k++)
	for(i=1;i<x_size-1;i++)
	for(j=1;j<y_size-1;j++)
	{	
		u[k][i][j].x -= i;
		u[k][i][j].y -= j;
		u[k][i][j].z -= k;
	}

  	return(count);
} 


int warp_GVF(int ITER, float mu, Fvector3d ***u, Fvector3d ***f, int x_size, int y_size, int z_size)
{
	int count=0,i,j,k;
	
	Fvector3d ***tmpu;
	
	float *mag;
	
	tmpu = Fvector3dalloc3d(x_size,y_size,z_size);

	mag=(float *)calloc(x_size*y_size*z_size,sizeof(float));
	
	for(k=0;k<z_size;k++)
	for(i=0;i<x_size;i++)
	for(j=0;j<y_size;j++)
		mag[k*x_size*y_size+i*y_size+j]=f[k][i][j].x*f[k][i][j].x+f[k][i][j].y*f[k][i][j].y+f[k][i][j].z*f[k][i][j].z;
	
	while(count<ITER)
    {   
    	for(k=1;k<z_size-1;k++)
		for(i=1;i<x_size-1;i++)
		for(j=1;j<y_size-1;j++)
	    {
			tmpu[k][i][j].x=u[k][i][j].x+mu*(u[k+1][i][j].x+u[k-1][i][j].x+u[k][i+1][j].x+u[k][i-1][j].x+u[k][i][j+1].x+u[k][i][j-1].x-6*u[k][i][j].x)-(u[k][i][j].x-f[k][i][j].x)*mag[k*x_size*y_size+i*y_size+j];
			tmpu[k][i][j].y=u[k][i][j].y+mu*(u[k+1][i][j].y+u[k-1][i][j].y+u[k][i+1][j].y+u[k][i-1][j].y+u[k][i][j+1].y+u[k][i][j-1].y-6*u[k][i][j].y)-(u[k][i][j].y-f[k][i][j].y)*mag[k*x_size*y_size+i*y_size+j];
			tmpu[k][i][j].z=u[k][i][j].z+mu*(u[k+1][i][j].z+u[k-1][i][j].z+u[k][i+1][j].z+u[k][i-1][j].z+u[k][i][j+1].z+u[k][i][j-1].z-6*u[k][i][j].z)-(u[k][i][j].z-f[k][i][j].z)*mag[k*x_size*y_size+i*y_size+j];
	    }

		for(k=1;k<z_size-1;k++)
		for(i=1;i<x_size-1;i++)
		for(j=1;j<y_size-1;j++)
	    {
			u[k][i][j].x=tmpu[k][i][j].x;
			u[k][i][j].y=tmpu[k][i][j].y;
			u[k][i][j].z=tmpu[k][i][j].z;
	    }
		
		count++;
      
		printf("iteration: %d\n",count);
    } 
	
	free(mag);

	Fvector3dfree3d( tmpu, z_size, x_size);
	
	return(count);
} 


void normalize(Fvector3d ***u, Fvector3d ***nu, int x_size, int y_size, int z_size)
{
	int x,y,z;
	float tempgrad;

	for(z=1;z<z_size-1;z++)
	for(x=1;x<x_size-1;x++)
	for(y=1;y<y_size-1;y++)
	{
		tempgrad = sqrt(u[z][x][y].x*u[z][x][y].x + u[z][x][y].y*u[z][x][y].y + u[z][x][y].z*u[z][x][y].z + 0.000000001);
	  
		nu[z][x][y].x = u[z][x][y].x/tempgrad;
		nu[z][x][y].y = u[z][x][y].y/tempgrad;
		nu[z][x][y].z = u[z][x][y].z/tempgrad;
	}
	
	return;
}


void find_mode(Fvector3d ***u, Ivector3d ***mode, int x_size, int y_size, int z_size)
{
	int x,y,z;
	int nextx,nexty,nextz,nnextx,nnexty,nnextz;
	int n,count;
	float threshold = 0.3, threshold2 = 0.1;
	float max;

	byte ***visit;
	byte ***edge;
	int **pass;

	visit = UCalloc3d(x_size,y_size,z_size);
	edge = UCalloc3d(x_size,y_size,z_size);
	
	int volume = x_size*y_size*z_size;
	pass = Ialloc2d(volume/3, 3);
	
	for(z=0;z<z_size;z++)
	for(x=0;x<x_size;x++)
	for(y=0;y<y_size;y++)
	{
		visit[z][x][y] = 0;
		edge[z][x][y] = 0;
		
		mode[z][x][y].x = -1;
		mode[z][x][y].y = -1;
		mode[z][x][y].z = -1;
	}
	
	for(x=0;x<volume/3;x++)
	{
		pass[x][0] = 0;
		pass[x][1] = 0;
		pass[x][2] = 0;
	}
	
	for(z=1;z<z_size-1;z++)
	for(x=1;x<x_size-1;x++)
	for(y=1;y<y_size-1;y++)
	{
		if(visit[z][x][y]==0)
		{
			count = -1;
			
			max = -10000;

			nextx = x;	
			nexty = y;
			nextz = z;

			if(u[nextz][nextx][nexty].x>=0)
				nnextx = nextx + (int)(u[nextz][nextx][nexty].x + 0.5);
			else
				nnextx = nextx + (int)(u[nextz][nextx][nexty].x - 0.5);

			if(u[nextz][nextx][nexty].y>=0)
				nnexty = nexty + (int)(u[nextz][nextx][nexty].y + 0.5);
			else
				nnexty = nexty + (int)(u[nextz][nextx][nexty].y - 0.5);

			if(u[nextz][nextx][nexty].z>=0)
				nnextz = nextz + (int)(u[nextz][nextx][nexty].z + 0.5);
			else
				nnextz = nextz + (int)(u[nextz][nextx][nexty].z - 0.5);

		
			while(nnextx>0 && nnexty>0 && nnextz>0 && nnextx<x_size && nnexty<y_size && nnextz<z_size \
				&& u[nextz][nextx][nexty].x*u[nnextz][nnextx][nnexty].x + \
				u[nextz][nextx][nexty].y*u[nnextz][nnextx][nnexty].y + \
				u[nextz][nextx][nexty].z*u[nnextz][nnextx][nnexty].z>=threshold \
				&& fabs(u[nextz][nextx][nexty].x) + fabs(u[nextz][nextx][nexty].y) + fabs(u[nextz][nextx][nexty].z) > 0.5 \
				&& (nnextx!=nextx || nnexty!=nexty || nnextz!=nextz))
			{
				count++;
				
				pass[count][0] = nextx;
				pass[count][1] = nexty;
				pass[count][2] = nextz;

				nextx = nnextx;
				nexty = nnexty;
				nextz = nnextz;

				if(u[nextz][nextx][nexty].x>=0)
					nnextx = nextx + (int)(u[nextz][nextx][nexty].x + 0.5);
				else
					nnextx = nextx + (int)(u[nextz][nextx][nexty].x - 0.5);

				if(u[nextz][nextx][nexty].y>=0)
					nnexty = nexty + (int)(u[nextz][nextx][nexty].y + 0.5);
				else
					nnexty = nexty + (int)(u[nextz][nextx][nexty].y - 0.5);

				if(u[nextz][nextx][nexty].z>=0)
					nnextz = nextz + (int)(u[nextz][nextx][nexty].z + 0.5);
				else
					nnextz = nextz + (int)(u[nextz][nextx][nexty].z - 0.5);

				if (count > 65535)
					break;
			}
			
			if(nextx>0 && nexty>0 && nextz>0 && nextx<x_size && nexty<y_size && nextz<z_size)
			{
				mode[z][x][y].x = nextx;
				mode[z][x][y].y = nexty;
				mode[z][x][y].z = nextz;
			}
			else
			{
				mode[z][x][y].x = -1;
				mode[z][x][y].y = -1;
				mode[z][x][y].z = -1;
			}
				
			visit[z][x][y]=1;
			
			if(count>=0)
			{
				for(n=0;n<=count;n++)
				{
					if(visit[pass[count][2]][pass[n][0]][pass[n][1]]==0)
					{
						mode[pass[count][2]][pass[n][0]][pass[n][1]].x = nextx;
						mode[pass[count][2]][pass[n][0]][pass[n][1]].y = nexty;
						mode[pass[count][2]][pass[n][0]][pass[n][1]].z = nextz;

						visit[pass[count][2]][pass[n][0]][pass[n][1]] = 1;
					}
				}
			}						

		}
	}
	

	Ifree2d(pass,volume/3);

	UCfree3d(visit,z_size,x_size);
	UCfree3d(edge,z_size,x_size);
	

	return;
}



void fill(int x, int y, int z, int label, Ivector3d ***mode, int ***visit, int *modePointCounts, int **indexTable, int fuse_th, int x_size, int y_size, int z_size)
{

	int i, j, k;
	int neighx, neighy, neighz;
	int index=0;
	float mode_dist;
	int neighborsFound;

	indexTable[0][0] = x;
	indexTable[0][1] = y;
	indexTable[0][2] = z;

	modePointCounts[label]++;

	while(1)
	{
		neighborsFound	= 0;

		for(k=-1;k<=1;k++)
		for(i=-1;i<=1;i++)
		for(j=-1;j<=1;j++)
		{
			neighx = x + i;
			neighy = y + j;
			neighz = z + k;
			
			if((neighx>=0)&&(neighy>=0)&&(neighz>=0) \
				&&(neighx<x_size)&&(neighy<y_size)&&(neighz<z_size) \
				&& visit[neighz][neighx][neighy]<0) 
			{
				mode_dist = sqrt(double((mode[z][x][y].x-mode[neighz][neighx][neighy].x)*(mode[z][x][y].x-mode[neighz][neighx][neighy].x) \
					+ (mode[z][x][y].y-mode[neighz][neighx][neighy].y)*(mode[z][x][y].y-mode[neighz][neighx][neighy].y) \
					+ (mode[z][x][y].z-mode[neighz][neighx][neighy].z)*(mode[z][x][y].z-mode[neighz][neighx][neighy].z)));
				
				if(mode_dist <= fuse_th)
				{
					visit[neighz][neighx][neighy] = label;

					modePointCounts[label]++;
					
					++index;
					indexTable[index][0] = neighx;
					indexTable[index][1] = neighy;
					indexTable[index][2] = neighz;

					neighborsFound = 1;
				}

			}
		}

		if(neighborsFound)
		{
			x = indexTable[index][0];
			y = indexTable[index][1];
			z = indexTable[index][2];
		}
		else if (index > 1)
		{
			--index;
			x = indexTable[index][0];
			y = indexTable[index][1];
			z = indexTable[index][2];
		}
		else
			break; 
	}

	return;
}


void fuse_mode(Ivector3d ***mode, int ***visit, float fuse_th, int x_size, int y_size, int z_size)
{
	int x, y, z;
	int label;
	
	int **indexTable;
	int *modePointCounts;
	int volume = x_size*y_size*z_size;

	indexTable = Ialloc2d(volume,3);

	modePointCounts = (int *) malloc(sizeof(int)*volume);

	for(z=0;z<z_size;z++)
	for(x=0;x<x_size;x++)
	for(y=0;y<y_size;y++)
	{
		visit[z][x][y] = -1;
	}


	for(x=0;x<volume/3;x++)
	{
		modePointCounts[x] = 0;

		indexTable[x][0] = 0;
		indexTable[x][1] = 0;
		indexTable[x][2] = 0;
	}

	label = -1;
	for(z=0;z<z_size;z++)
	{
		for(x=0;x<x_size;x++)
		{
			for(y=0;y<y_size;y++)
			{
				if(visit[z][x][y]<0)
				{
					visit[z][x][y] = ++label;

					fill( x, y, z, label, mode, visit, modePointCounts, indexTable, fuse_th, x_size, y_size, z_size);
				}
			}
		}

	}
	
	free(modePointCounts);

	Ifree2d(indexTable,volume);

	return;
}


void find_edge(int ***label, byte ***edge, int x_size, int y_size, int z_size)
{
	int x,y,z;

	for(z=0;z<z_size;z++)
	for(x=0;x<x_size;x++)
	for(y=0;y<y_size;y++)
	{
		edge[z][x][y]=0;
	}

	for(z=1;z<z_size-1;z++)
	for(x=1;x<x_size-1;x++)
	for(y=1;y<y_size-1;y++)
	{
		if((label[z][x][y]!=label[z][x+1][y] || label[z][x][y]!=label[z][x-1][y] \
			|| label[z][x][y]!=label[z][x][y+1] || label[z][x][y]!=label[z][x][y-1] \
			|| label[z][x][y]!=label[z+1][x][y] || label[z][x][y]!=label[z-1][x][y]) && label[z][x][y]>0)
			edge[z][x][y] = 255;
		else
			edge[z][x][y] = 0;
	}
   
	return;
}


int otsu(float *hist)
{
	int i, threshold;
	float s,ut,uk,wk,max;
	float b,q;

	s = 0.0;
	for(i=0;i<256;i++)
		s += hist[i];

	for(i=0;i<256;i++)
		hist[i] = hist[i]/s;

	ut = 0.0;
	for(i=0;i<256;i++)
		ut = ut + i*hist[i];

	uk = 0.0; wk = 0.0; max = 0.0;
	for(i=0;i<256;i++)
	{
		wk = wk + hist[i];
		uk = uk + i*hist[i];
		
		b = ut * wk - uk;
		q = wk * (1.0-wk);

		if(q<0.00000001) continue;
		
		b = b*b/q;

		if(b > max)
		{
			max = b;
			threshold = i;
		}
	}

	return threshold;
}


void local_thresh(float ***image, int ***label, int ***result, float globalCoefficient, int x_size, int y_size, int z_size)
{
	int x, y, z;
	int count, n, i;
	int globalThreshold = 0;
	
	int **imhist;
	float *hist;
	int *threshold;
	
	count = 0;
	for(z=0;z<z_size;z++)
	for(x=0;x<x_size;x++)
	for(y=0;y<y_size;y++)
	{
		if(label[z][x][y]>count)
		{
			count = label[z][x][y];
		}
	}
	count = count + 1;

	imhist = Ialloc2d(count,256);

	hist = (float *)malloc(sizeof(float)*256);

	for(i=0;i<256;i++)
	{
		hist[i] = 0;
	}
	
	for(z=0;z<z_size;z++)
	for(x=0;x<x_size;x++)
	for(y=0;y<y_size;y++)
	{
		if(label[z][x][y]>0)
			imhist[label[z][x][y]][(int)image[z][x][y]]++;
		
		hist[(int)image[z][x][y]]++;
	}

	globalThreshold = otsu(hist);


	threshold = (int *)malloc(sizeof(int)*(count));
	
	for(n=0;n<count;n++)
	{
		for(i=0;i<256;i++)
		{
			hist[i] = (float)imhist[n][i];
		}

		threshold[n] = otsu(hist);
	}
	
	free(hist);

	for(z=0;z<z_size;z++)
	for(x=0;x<x_size;x++)
	for(y=0;y<y_size;y++)
	{
		if(label[z][x][y]>=0 && image[z][x][y]>=0.8*threshold[label[z][x][y]] && image[z][x][y]>=globalCoefficient*globalThreshold)
			result[z][x][y] =  label[z][x][y]+1;
		else
			result[z][x][y] = 0;
	}


	Ifree2d(imhist,count);

	free(threshold);

	return;
}


void connective_analysis(int x, int y, int z, int label, int ***visit, int ***input, int *modePointCounts, int **indexTable, int x_size, int y_size, int z_size)
{
	int i,j,k;
	int neighx,neighy,neighz;
	int index=0;
	int neighborsFound;
	
	indexTable[0][0] = x;
	indexTable[0][1] = y;
	indexTable[0][2] = z;

	modePointCounts[label]++;

	while(1)
	{
		neighborsFound	= 0;

		for(k=-1;k<=1;k++)
		for(i=-1;i<=1;i++)
		for(j=-1;j<=1;j++)
		{
			neighx = x + i;
			neighy = y + j;
			neighz = z + k;
			
			if( (neighx>=0)&&(neighy>=0)&&(neighz>=0)&&(neighx<x_size)&&(neighy<y_size)&&(neighz<z_size)\
				&& visit[neighz][neighx][neighy]<0 && input[neighz][neighx][neighy]>0) 
			{
				if(input[z][x][y] == input[neighz][neighx][neighy])
				{
					visit[neighz][neighx][neighy] = label;

					modePointCounts[label]++;
					
					++index;

					indexTable[index][0] = neighx;
					indexTable[index][1] = neighy;
					indexTable[index][2] = neighz;

					neighborsFound = 1;
				}
			}
		}

		if(neighborsFound)
		{
			x = indexTable[index][0];
			y = indexTable[index][1];
			z = indexTable[index][2];
		}
		else if (index > 1)
		{
			--index;
			x = indexTable[index][0];
			y = indexTable[index][1];
			z = indexTable[index][2];
		}
		else
			break; 
	}


	return;
}


// delete small connective components
void prune_component(int ***input, int minRegion, int x_size, int y_size, int z_size)
{
	int x,y,z;
	int label, count;
	int max;

	int ***visit;

	int *modePointCounts;

	int **indexTable;

	int  volume = x_size*y_size*z_size;
	
	indexTable = Ialloc2d(volume,3);

	visit = Ialloc3d(x_size,y_size,z_size);

//	modePointCounts = malloc(sizeof(int)*volume);
	modePointCounts = (int *) malloc(sizeof(int)*volume); // FL, 20080502

	for(z=0;z<z_size;z++)
	for(x=0;x<x_size;x++)
	for(y=0;y<y_size;y++)
	{
		visit[z][x][y] = -1;
	}

	for(x=0;x<volume;x++)
	{
		modePointCounts[x]=0;

		indexTable[x][0] = 0;
		indexTable[x][1] = 0;
		indexTable[x][2] = 0;
	}
	
	label = -1;
	for(z=0;z<z_size;z++)
	{
		for(x=0;x<x_size;x++)
		for(y=0;y<y_size;y++)
		{
			if(visit[z][x][y]<0 && input[z][x][y]>0)
			{
				visit[z][x][y] = ++label;

				connective_analysis( x, y, z, label, visit, input, modePointCounts, indexTable, x_size, y_size, z_size);
			}
		}
	}

	count = 0;
	for(z=0;z<z_size;z++)
	for(x=0;x<x_size;x++)
	for(y=0;y<y_size;y++)
	{
		if(visit[z][x][y]>=0 && modePointCounts[visit[z][x][y]]>minRegion)
		{
			input[z][x][y] =  visit[z][x][y] + 1;
		}
		else
			input[z][x][y] = 0;
	}

	// relabel image
	for(z=0;z<z_size;z++)
	for(x=0;x<x_size;x++)
	for(y=0;y<y_size;y++)
	{
		visit[z][x][y] = -1;
	}

	for(x=0;x<volume;x++)
	{
		modePointCounts[x]=0;

		indexTable[x][0] = 0;
		indexTable[x][1] = 0;
		indexTable[x][2] = 0;
	}

	
	label = 0;
	for(z=0;z<z_size;z++)
	{
		for(x=0;x<x_size;x++)
		for(y=0;y<y_size;y++)
		{
			if(visit[z][x][y]<0 && input[z][x][y]>0)
			{
				visit[z][x][y] = ++label;

				connective_analysis( x, y, z, label, visit, input, modePointCounts, indexTable, x_size, y_size, z_size);
			}
		}
	}


	max = 0;
	for(z=0;z<z_size;z++)
	for(x=0;x<x_size;x++)
	for(y=0;y<y_size;y++)
	{	
		if(visit[z][x][y] > 0)
		{
			input[z][x][y] =  visit[z][x][y];
		}
		else
		{
			input[z][x][y] = 0;
		}
		
		if(visit[z][x][y]>max)
		{
			max = visit[z][x][y];
		}
	}
	
	printf("There are %d segmented objects.\n", max);

	free(modePointCounts); 
	
	Ifree2d(indexTable, volume);

	Ifree3d(visit, z_size, x_size);

	return;

} 


// majority filter
void majority_filter(int ***image, int times, int x_size, int y_size, int z_size)
{
	int x, y, z, t;
	int xx, yy, zz;
	int max, temp_max;
	int xm, ym, zm;

	int ***temp_image;
	int *hist;

	temp_image = Ialloc3d(x_size,y_size,z_size);

	max = 0;
	for(z=0;z<z_size;z++)
	for(x=0;x<x_size;x++)
	for(y=0;y<y_size;y++)
	{
		temp_image[z][x][y] = image[z][x][y];

		if(temp_image[z][x][y]>max)
			max = temp_image[z][x][y]; 
	}

	hist = (int *)malloc(sizeof(int)*(max+1));

	for(x=0;x<max+1;x++)
		hist[x] = 0;

	for(t=0; t<times; t++)
	{
		for(z=0;z<z_size;z++)
		for(x=0;x<x_size;x++)
		for(y=0;y<y_size;y++)
		{
			temp_image[z][x][y] = image[z][x][y];
		}

		for(z=1; z<z_size-1; z++)
		for(x=1; x<x_size-1; x++)
		for(y=1; y<y_size-1; y++)
		{
			for(zz=-1; zz<2; zz++)
			for(xx=-1; xx<2; xx++)
			for(yy=-1; yy<2; yy++)
			{
				hist[temp_image[z+zz][x+xx][y+yy]]++;
			}

			temp_max = 0;
			for(zz=-1; zz<2; zz++)
			for(xx=-1; xx<2; xx++)
			for(yy=-1; yy<2; yy++)
			{
				if (hist[temp_image[z+zz][x+xx][y+yy]]>temp_max)
				{
					temp_max = hist[temp_image[z+zz][x+xx][y+yy]];
					xm = x + xx;
					ym = y + yy;
					zm = z + zz;
				}
				hist[temp_image[z+zz][x+xx][y+yy]] = 0;
			}
			
			image[z][x][y] = temp_image[zm][xm][ym];
		}
	}

	free(hist);
	
	Ifree3d(temp_image,z_size,x_size);

	return;
}


// compute divergence and flux
void divergence(Fvector3d ***u, float ***div, float ***flux, int x_size, int y_size, int z_size)
{
	int i,j,k,ii,jj,kk;
	float nii,njj,nkk,total;

	for(k=0;k<z_size;k++)
	for(i=0;i<x_size;i++)
	for(j=0;j<y_size;j++)
	{
		div[k][i][j] = 0;
		flux[k][i][j] = 0;
	}

	//diveregnce
	for(k=2;k<z_size-2;k++)
	for(i=2;i<x_size-2;i++)
	for(j=2;j<y_size-2;j++)
	{
	    div[k][i][j] =  -0.5*((u[k][i+1][j].x - u[k][i-1][j].x) + (u[k][i][j+1].y - u[k][i][j-1].y) + (u[k+1][i][j].z - u[k-1][i][j].z)) ; 

		for(kk=-1;kk<=1;kk++)
		for(ii=-1;ii<=1;ii++)
		for(jj=-1;jj<=1;jj++)
		{
			total = sqrt(double(ii*ii + jj*jj + kk*kk));

			if(total>0)
			{
				nii = ii/total;
				njj = jj/total;
				nkk = kk/total;
			}
			else
			{
				nii = 0; njj = 0; nkk = 0;
			}
			
			flux[k][i][j] -= (nii*u[k+kk][i+ii][j+jj].x + njj*u[k+kk][i+ii][j+jj].y + nkk*u[k+kk][i+ii][j+jj].z);
		}
	}

	return;
}


bool gvfCellSeg(Vol3DSimple <unsigned char> *img3d, Vol3DSimple <unsigned short int> *outimg3d, const gvfsegPara & segpara)
{

	int diffusionIteration = segpara.diffusionIteration;
	int fusionThreshold = segpara.fusionThreshold;
	int minRegion = segpara.minRegion;
	float sigma = segpara.sigma;

	float lambda = 0.2, mu=0.1, omega=1.0;
//	int smoothIteration = 3;
	float globalCoefficient = 0.9;
	
	Fvector3d ***u;
	Fvector3d ***nu;
	Fvector3d ***f;
	Fvector3d ***grad;
	
	Ivector3d ***mode;

	float ***image;

	int ***visit;
	int ***result;

	byte ***I;
	byte ***edge;

	unsigned char ***indata3d = img3d->getData3dHandle();
	unsigned short int ***outdata3d = outimg3d->getData3dHandle();

	int sz[3];
	sz[0] = (int)img3d->sz0();
	sz[1] = (int)img3d->sz1();
	sz[2] = (int)img3d->sz2();
	
	int y_size = sz[0];
	int x_size = sz[1];
	int z_size = sz[2];
	
	int k, j,i;

//	int x_size = (int)img3d->sz0();
//	int y_size = (int)img3d->sz1();
//	int z_size = (int)img3d->sz2();
	
	int volume = x_size*y_size*z_size;
	int smoothIteration = (int)(3*segpara.sigma + 0.5);

	image = Falloc3d(x_size,y_size,z_size);    
	I = UCalloc3d(x_size,y_size,z_size);
	grad = Fvector3dalloc3d(x_size,y_size,z_size);

	for (k=0;k<sz[2]; k++)
	{
		for (j=0; j<sz[1]; j++)
		{
			for (i=0; i<sz[0]; i++)
			{
				image[k][j][i] = (float)(indata3d[k][j][i]);
			}
		}
	}
	
	printf("\nSmoothing image.\n");	
	smooth_image(image, smoothIteration, x_size, y_size, z_size);
		
	printf("Computing gradient vector field.\n");
	gradient(image, grad, I, x_size, y_size, z_size);

//	for (k=0;k<sz[2]; k++)
//	{
//		for (j=0; j<sz[1]; j++)
//		{
//			for (i=0; i<sz[0]; i++)
//			{
//				printf("%f, %f, %f\n", grad[k][j][i].x, grad[k][j][i].y, grad[k][j][i].z);
//			}
//		}
//	}

	printf("Initializing gradient vector field.\n");
	f = Fvector3dalloc3d(x_size,y_size,z_size);
	u = Fvector3dalloc3d(x_size,y_size,z_size);
	//initialize_EDT( grad, I, f, u);
	initialize_GVF( grad, f, u, x_size, y_size, z_size);
	Fvector3dfree3d(grad,z_size,x_size);

	printf("Diffusing gradient vector field.\n");
	//warp_EDT( diffusionIteration, lambda, mu, u, omega, I, f);
	warp_GVF(diffusionIteration, mu, u, f, x_size, y_size, z_size);
	Fvector3dfree3d(f,z_size,x_size);
	UCfree3d(I,z_size,x_size);
	
	nu = Fvector3dalloc3d(x_size,y_size,z_size);
	normalize(u, nu, x_size, y_size, z_size);
	Fvector3dfree3d(u,z_size,x_size);

//	float ***vv;
//	vv = Falloc3d(x_size,y_size,z_size);	
//	
//	for (k=0; k< z_size; k++)
//		for (j=0; j<y_size; j++)
//			for (i=0; i<x_size; i++)
//				vv[k][j][i]= nu[k][j][i].x;
//	sprintf(fnameOut,"%s_grad_x.raw",fname);
//	write_image_float(fnameOut, vv);
//	
//	for (k=0; k< z_size; k++)
//		for (j=0; j<y_size; j++)
//			for (i=0; i<x_size; i++)
//				vv[k][j][i]= nu[k][j][i].y;
//	sprintf(fnameOut,"%s_grad_y.raw",fname);
//	write_image_float(fnameOut, vv);
//	
//	for (k=0; k< z_size; k++)
//		for (j=0; j<y_size; j++)
//			for (i=0; i<x_size; i++)
//				vv[k][j][i]= nu[k][j][i].z;
//	sprintf(fnameOut,"%s_grad_z.raw",fname);
//	write_image_float(fnameOut, vv);
	
	mode = Ivector3dalloc3d(x_size,y_size,z_size);
	find_mode( nu, mode, x_size, y_size, z_size);
	Fvector3dfree3d(nu,z_size,x_size);

	printf("Tracking gradient flow field.\n");
	visit = Ialloc3d(x_size,y_size,z_size);
	fuse_mode( mode, visit, fusionThreshold, x_size, y_size, z_size);
	Ivector3dfree3d(mode,z_size,x_size);
	
//	for (k=0;k<sz[2]; k++)
//	{
//		for (j=0; j<sz[1]; j++)
//		{
//			for (i=0; i<sz[0]; i++)
//			{
//				printf("%d\n",visit[k][j][i]);
//			}
//		}
//	}
	

	edge = UCalloc3d(x_size,y_size,z_size);
	majority_filter(visit, 3, x_size, y_size, z_size);
	find_edge(visit, edge, x_size, y_size, z_size);

	printf("Local thresholding.\n");
	result = Ialloc3d(x_size,y_size,z_size);
	local_thresh(image, visit, result, globalCoefficient, x_size, y_size, z_size);

	majority_filter(result, 3, x_size, y_size, z_size);

	printf("Pruning small regions\n");
	prune_component(result, minRegion, x_size, y_size, z_size);
//	sprintf(fnameOut,"%s_segmask.raw",fname);
//	write_image_int(fnameOut, result);
	
	find_edge(result,edge, x_size, y_size, z_size);
//	sprintf(fnameOut,"%s_segedge.raw",fname);
//	write_image_byte(fnameOut,edge);

//	copy to output

	for (k=0;k<sz[2]; k++)
	{
		for (j=0; j<sz[1]; j++)
		{
			for (i=0; i<sz[0]; i++)
			{
				outdata3d[k][j][i] = (unsigned short int)(result[k][j][i]);
//				printf("%d\n",visit[k][j][i]);
			}
		}
	}
//
//	//free space
//	UCfree3d(edge,z_size,x_size);
	Ffree3d(image,z_size,x_size);
	Ifree3d(visit,z_size,x_size);
	Ifree3d(result,z_size,x_size);
//	free(fname);

	return true;
}