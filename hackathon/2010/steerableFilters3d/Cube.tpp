
#include "Cube.h"
#include "polynomial.h"
#include "utils.h"

#include <math.h>
#include <vector>

using namespace std;

//#define M_PI 3.141592653589793

template <class T>
Cube<T>::Cube(T* data, long _depth, long _height, long _width)
{
   width   = _width;
   height  = _height;
   depth   = _depth;

   if(data != 0)
      voxels_origin = (T*) data;
   else {
      long long int size = (long long int)this->width*
      (long long int)this->height*(long long int)this->depth;
      voxels_origin = (T*)calloc(size,sizeof(T));
   }

   voxels = (T***)malloc(depth*sizeof(T**));

   //Initializes the pointer structure to acces quickly to the voxels
   for(int z = 0; z < depth; z++)
   {
      voxels[z] = (T**)malloc(height*sizeof(T*));
      for(int j = 0; j < height; j++){
         //long long int pos = (long long int)z*(long long int)width*(long long int)height +
         //(long long int)j*(long long int)width;
         voxels[z][j]=(T*)&voxels_origin[(long long int)z*(long long int)width*(long long int)height +
         (long long int)j*(long long int)width];
      }
   }
}
/*
template <class T>
Cube<float>*  Cube<T>::create_blank_cube(string name, bool reflectToFile)
{
  string vl = ".vl";
  string nfo = ".nfo";

  //if(fileExists(this->directory + name + nfo) &&
  //   fileExists(this->directory + name + vl))
  //  return new Cube<float>(this->directory + name + nfo);

  Cube<float>* toReturn = new Cube<float>(0,depth,height,width);
  //toReturn->height = height;
  //toReturn->depth  = depth;
  //toReturn->width  = width;
  //toReturn->directory = directory;
  toReturn->type = "float";
  //toReturn->filenameVoxelData = name + vl;
  //if(reflectToFile){
  //  toReturn->save_parameters(this->directory + name + nfo);
  //  toReturn->create_volume_file(this->directory + name + vl);
  //}
  //toReturn->load_volume_data(this->directory + name + vl, reflectToFile);

  return toReturn;
}
*/

template <class T>
Cube<T>::~Cube()
{
  for(int z = 0; z < depth; z++){
    free(voxels[z]);
  }
  free(voxels);
}

template <class T>
T Cube<T>::at(int x, int y, int z) {return voxels[z][y][x];}

template <class T>
void Cube<T>::put(int x, int y, int z, T value) {voxels[z][y][x] = value;}


template <class T>
void Cube<T>::calculate_derivative
(int nx, int ny, int nz,
 float sigma_x, float sigma_y, float sigma_z,
 Cube<float>* output, Cube<float>* tmp)
{
  vector< float > mask_x = Cube::gaussian_mask(nx, sigma_x, true);
  vector< float > mask_y = Cube::gaussian_mask(ny, sigma_y, true);
  vector< float > mask_z = Cube::gaussian_mask(nz, sigma_z, true);

  this->convolve_horizontally(mask_x, output, true);
  output->convolve_vertically(mask_y, tmp,    true);
  tmp->convolve_depth(        mask_z, output, true);
}


template <class T>
void Cube<T>::convolve_horizontally(vector< float >& mask, Cube< float >* output, bool use_borders)
{
  //assert(mask.size() > 0);

  //T* original_line;
  //float* result_line;

  int mask_side = mask.size()/2;
  int mask_size = mask.size();
  printf("Cube<T>::convolve_horizontally [");

  int printLimit = max(1,(int)(depth/20));
  #ifdef WITH_OPENMP
  #pragma omp parallel for
  #endif
  for(int z = 0; z < depth; z++){
    int x,q;
    float result;
    for(int y = 0; y < height; y++)
      {

        // Beginning of the line
        for(x = 0; x < mask_size; x++){
          result = 0;
          for(q = -mask_side; q <=mask_side; q++){
            if(x+q<0)
              result+=this->at(0,y,z)*mask[mask_side + q];
            else
              result += this->at(x+q,y,z)*mask[mask_side + q];
          }
          output->put(x,y,z,result);
        }

       //Middle of the line
        for(x = mask_size; x <= width-mask_size-1; x++)
          {
            result = 0;
            for(q = -mask_side; q <=mask_side; q++)
              result += this->at(x+q,y,z)*mask[mask_side + q];
            output->put(x,y,z,result);
          }

        //End of the line
        for(x = width-mask_size; x < width; x++){
          result = 0;
          for(q = -mask_side; q <=mask_side; q++){
            if(x+q >= width)
              result+=this->at(width-1,y,z)*mask[mask_side + q];
            else
              result += this->at(x+q,y,z)*mask[mask_side + q];
          }
          output->put(x,y,z,result);
        }
      }
    if(z%printLimit==0)
      printf("#");fflush(stdout);
  }
  printf("]\n");
}

template <class T>
void Cube<T>::convolve_vertically(vector< float >& mask, Cube<float>* output, bool use_borders)
{
  //assert(mask.size() > 0);
  int mask_side = mask.size()/2;
  int mask_size = mask.size();

  printf("Cube<T>::convolve_vertically [");
  int printLimit = max(1,(int)(depth/20));
  #ifdef WITH_OPENMP
  #pragma omp parallel for
  #endif
  for(int z = 0; z < depth; z++){
    float result = 0;
    int q = 0;
    int y = 0;
    for(int x = 0; x < width; x++)
      {

        //Beginning of the line
        for(y = 0; y < mask_size; y++){
          result = 0;
          for(q = -mask_side; q <=mask_side; q++){
            if(y+q<0)
              result+=this->at(x,0,z)*mask[mask_side + q];
            else
              result += this->at(x,y+q,z)*mask[mask_side + q];
          }
          output->put(x,y,z,result);
        }

        //Middle of the line
        for(y = mask_size; y <= height-mask_size-1; y++)
          {
            result = 0;
            for(q = -mask_side; q <=mask_side; q++)
              result += this->at(x,y+q,z)*mask[mask_side + q];
            output->put(x,y,z,result);
          }

        //End of the line
        for(y = height-mask_size; y < height; y++){
          result = 0;
          for(q = -mask_side; q <=mask_side; q++){
            if(y+q >= height)
              result+=this->at(x,height-1,z)*mask[mask_side + q];
            else
              result += this->at(x,y+q,z)*mask[mask_side + q];
          }
          output->put(x,y,z,result);
        }
      }
    if(z%printLimit==0)
      printf("#");fflush(stdout);
  }
  printf("]\n");
}

template <class T>
void Cube<T>::convolve_depth(vector< float >& mask, Cube<float>* output, bool use_borders)
{
  //assert(mask.size() > 0);
  int mask_side = mask.size()/2;
  int mask_size = mask.size();

  printf("Cube<T>::convolve_depth [");
  int printLimit = max(1,(int)(height/20));
  #ifdef WITH_OPENMP
  #pragma omp parallel for
  #endif
  for(int y = 0; y < height; y++){
    float result = 0;
    int q = 0;
    int z = 0;
    for(int x = 0; x < width; x++){

      for(z = 0; z < mask_size; z++){
        result = 0;
        for(q = -mask_side; q <=mask_side; q++){
          if(z+q<0)
            result+=this->at(x,y,0)*mask[mask_side + q];
          else if (z+q >= depth)
            result+=this->at(x,y,depth-1)*mask[mask_side + q];
          else
            result += this->at(x,y,z+q)*mask[mask_side + q];
        }
        output->put(x,y,z,result);
      }

      for(z = mask_size; z <depth - mask_size; z++){
        result = 0;
        for(q = -mask_side; q <=mask_side; q++)
          result += this->at(x,y,z+q)*mask[mask_side + q];
        output->put(x,y,z,result);
      }

      for(z = depth-mask_size; z < depth; z++){
        result = 0;
        for(q = -mask_side; q <=mask_side; q++){
          if(z+q >= depth)
            result+=this->at(x,y,depth-1)*mask[mask_side + q];
          else if(z+q<0)
            result+=this->at(x,y,0)*mask[mask_side + q];
          else
            result += this->at(x,y,z+q)*mask[mask_side + q];
        }
        output->put(x,y,z,result);
      }
    }
    if((y%printLimit)==0){
      printf("#");fflush(stdout);}
  }
  printf("]\n");
}

template <class T>
vector<float> Cube<T>::gaussian_mask(int order, float sigma, bool limit_value)
{
  //First we get the polynomial that would multiply the gaussian kernel.
  vector< double > coeffs(1);
  coeffs[0]=1.0;
  vector< double > coeffs_d(2);
  coeffs_d[0] = 0.0;
  coeffs_d[1] = -1/(sigma*sigma);

  Polynomial* otr = new Polynomial(coeffs_d);
  Polynomial* p = new Polynomial(coeffs);

  for(int i = 0; i < order; i++){
    //p = p' - x/sigma p;
    Polynomial* p_dev = p->derivative();
    Polynomial* p_otr = p->multiply_polynomial(otr);
//     delete p;
    p = p_dev->add_polynomial(p_otr);
  }
  // p->print();

  //This calculates half of the mask. It would be symetric or antysimetric.
  vector< double > value(1);
  double norm_coeff = 1.0/(sqrt((double)2.0*M_PI)*sigma);
  value[0] = norm_coeff*p->evaluate(0);
  int i = 1;
  do{
    value.push_back(norm_coeff*
                    p->evaluate(i)*
                    exp(-double(i*i)/(2.0*sigma*sigma)));
    i = i+1;
//     value.push_back(p->evaluate(i++));
  }
  while( (exp(-double(i*i)/(2.0*sigma*sigma)) > 1e-3)  && (value.size() <= 50)*limit_value ||
         (1-limit_value)*(value.size() <= 50)
       );
//    while( (value.size() <= 100));

//   p->print();
//   for(int i = 0; i < value.size(); i++)
//     printf("%f ", value[i]);
//   printf("\n");

  float simmetry;
  if( order%2 != 0)
    simmetry = -1;
  else
    simmetry = 1;

  vector<float> mask(2*value.size()-1);
  mask[value.size()-1]= value[0];
  for(int i = 1; i < value.size(); i++){
    mask[value.size()-1+i] = value[i];
    mask[value.size()-1-i] = simmetry*value[i];
  }
  return mask;
}
