#include "BrainVesselCPR_filter.h"
#include <iostream>
using namespace std;

#define INF 1E9

void gaussian_filter(unsigned short int * data1d,
                     V3DLONG *in_sz,
                     unsigned int Wx,
                     unsigned int Wy,
                     unsigned int Wz,
                     unsigned int c,
                     double sigma,
                     float* &outimg)
{
    if (!data1d || !in_sz || in_sz[0]<=0 || in_sz[1]<=0 || in_sz[2]<=0 || in_sz[3]<=0 || outimg)
    {
        cout << (bool)(!data1d) << (bool)(!in_sz) << (bool)(in_sz[0]<=0) << (bool)(in_sz[1]<=0) << (bool)(in_sz[2]<=0) << (bool)(in_sz[3]<=0) << (bool)outimg << endl;
        v3d_msg("Invalid parameters to gaussian_filter().", 0);
        return;
    }

    if (outimg)
    {
        v3d_msg("Warning: you have supplied an non-empty output image pointer. This program will force to free it now. But you may want to double check.");
        delete []outimg;
        outimg = 0;
    }

     // for filter kernel
     double sigma_s2 = 0.5/(sigma*sigma); // 1/(2*sigma*sigma)
     double pi_sigma = 1.0/(sqrt(2*3.1415926)*sigma); // 1.0/(sqrt(2*pi)*sigma)

     float min_val = INF, max_val = 0;

     V3DLONG N = in_sz[0];
     V3DLONG M = in_sz[1];
     V3DLONG P = in_sz[2];
     V3DLONG sc = in_sz[3];
     V3DLONG pagesz = N*M*P;

     //filtering
     V3DLONG offset_init = (c-1)*pagesz;

     //declare temporary pointer
     float *pImage = new float [pagesz];
     if (!pImage)
     {
          printf("Fail to allocate memory.\n");
          return;
     }
     else
     {
          for(V3DLONG i=0; i<pagesz; i++)
               pImage[i] = data1d[i + offset_init];  //first channel data (red in V3D, green in ImageJ)
     }
       //Filtering
     //
     //   Filtering along x
     if(N<2)
     {
          //do nothing
     }
     else
     {
          //create Gaussian kernel
          float  *WeightsX = 0;
          WeightsX = new float [Wx];
          if (!WeightsX)
               return;

          float Half = (float)(Wx-1)/2.0;

          // Gaussian filter equation:
          // http://en.wikipedia.org/wiki/Gaussian_blur
       //   for (unsigned int Weight = 0; Weight < Half; ++Weight)
       //   {
       //        const float  x = Half* float (Weight) / float (Half);
      //         WeightsX[(int)Half - Weight] = WeightsX[(int)Half + Weight] = pi_sigma * exp(-x * x *sigma_s2); // Corresponding symmetric WeightsX
      //    }

          for (unsigned int Weight = 0; Weight <= Half; ++Weight)
          {
              const float  x = float(Weight)-Half;
              WeightsX[Weight] = WeightsX[Wx-Weight-1] = pi_sigma * exp(-(x * x *sigma_s2)); // Corresponding symmetric WeightsX
          }


          double k = 0.;
          for (unsigned int Weight = 0; Weight < Wx; ++Weight)
               k += WeightsX[Weight];

          for (unsigned int Weight = 0; Weight < Wx; ++Weight)
               WeightsX[Weight] /= k;

         printf("\n x dierction");

         for (unsigned int Weight = 0; Weight < Wx; ++Weight)
             printf("/n%f",WeightsX[Weight]);

          //   Allocate 1-D extension array
          float  *extension_bufferX = 0;
          extension_bufferX = new float [N + (Wx<<1)];

          unsigned int offset = Wx>>1;

          //	along x
          const float  *extStop = extension_bufferX + N + offset;

          for(V3DLONG iz = 0; iz < P; iz++)
          {
               for(V3DLONG iy = 0; iy < M; iy++)
               {
                    float  *extIter = extension_bufferX + Wx;
                    for(V3DLONG ix = 0; ix < N; ix++)
                    {
                         *(extIter++) = pImage[iz*M*N + iy*N + ix];
                    }

                    //   Extend image
                    const float  *const stop_line = extension_bufferX - 1;
                    float  *extLeft = extension_bufferX + Wx - 1;
                    const float  *arrLeft = extLeft + 2;
                    float  *extRight = extLeft + N + 1;
                    const float  *arrRight = extRight - 2;

                    while (extLeft > stop_line)
                    {
                         *(extLeft--) = *(arrLeft++);
                         *(extRight++) = *(arrRight--);

                    }

                    //	Filtering
                    extIter = extension_bufferX + offset;

                    float  *resIter = &(pImage[iz*M*N + iy*N]);

                    while (extIter < extStop)
                    {
                         double sum = 0.;
                         const float  *weightIter = WeightsX;
                         const float  *const End = WeightsX + Wx;
                         const float * arrIter = extIter;
                         while (weightIter < End)
                              sum += *(weightIter++) * float (*(arrIter++));
                         extIter++;
                         *(resIter++) = sum;

                         //for rescale
                         if(max_val<*arrIter) max_val = *arrIter;
                         if(min_val>*arrIter) min_val = *arrIter;


                    }

               }
          }
          //de-alloc
           if (WeightsX) {delete []WeightsX; WeightsX=0;}
           if (extension_bufferX) {delete []extension_bufferX; extension_bufferX=0;}

     }

     //   Filtering along y
     if(M<2)
     {
          //do nothing
     }
     else
     {
          //create Gaussian kernel
          float  *WeightsY = 0;
          WeightsY = new float [Wy];
          if (!WeightsY)
               return;

          float Half = (float)(Wy-1)/2.0;

          // Gaussian filter equation:
          // http://en.wikipedia.org/wiki/Gaussian_blur
         /* for (unsigned int Weight = 0; Weight < Half; ++Weight)
          {
               const float  y = Half* float (Weight) / float (Half);
               WeightsY[(int)Half - Weight] = WeightsY[(int)Half + Weight] = pi_sigma * exp(-y * y *sigma_s2); // Corresponding symmetric WeightsY
          }*/

          for (unsigned int Weight = 0; Weight <= Half; ++Weight)
          {
              const float  y = float(Weight)-Half;
              WeightsY[Weight] = WeightsY[Wy-Weight-1] = pi_sigma * exp(-(y * y *sigma_s2)); // Corresponding symmetric WeightsY
          }


          double k = 0.;
          for (unsigned int Weight = 0; Weight < Wy; ++Weight)
               k += WeightsY[Weight];

          for (unsigned int Weight = 0; Weight < Wy; ++Weight)
               WeightsY[Weight] /= k;

          //	along y
          float  *extension_bufferY = 0;
          extension_bufferY = new float [M + (Wy<<1)];

          unsigned int offset = Wy>>1;
          const float *extStop = extension_bufferY + M + offset;

          for(V3DLONG iz = 0; iz < P; iz++)
          {
               for(V3DLONG ix = 0; ix < N; ix++)
               {
                    float  *extIter = extension_bufferY + Wy;
                    for(V3DLONG iy = 0; iy < M; iy++)
                    {
                         *(extIter++) = pImage[iz*M*N + iy*N + ix];
                    }

                    //   Extend image
                    const float  *const stop_line = extension_bufferY - 1;
                    float  *extLeft = extension_bufferY + Wy - 1;
                    const float  *arrLeft = extLeft + 2;
                    float  *extRight = extLeft + M + 1;
                    const float  *arrRight = extRight - 2;

                    while (extLeft > stop_line)
                    {
                         *(extLeft--) = *(arrLeft++);
                         *(extRight++) = *(arrRight--);
                    }

                    //	Filtering
                    extIter = extension_bufferY + offset;

                    float  *resIter = &(pImage[iz*M*N + ix]);

                    while (extIter < extStop)
                    {
                         double sum = 0.;
                         const float  *weightIter = WeightsY;
                         const float  *const End = WeightsY + Wy;
                         const float * arrIter = extIter;
                         while (weightIter < End)
                              sum += *(weightIter++) * float (*(arrIter++));
                         extIter++;
                         *resIter = sum;
                         resIter += N;

                         //for rescale
                         if(max_val<*arrIter) max_val = *arrIter;
                         if(min_val>*arrIter) min_val = *arrIter;


                    }

               }
          }

          //de-alloc
          if (WeightsY) {delete []WeightsY; WeightsY=0;}
          if (extension_bufferY) {delete []extension_bufferY; extension_bufferY=0;}


     }

     //  Filtering  along z
     if(P<2)
     {
          //do nothing
     }
     else
     {
          //create Gaussian kernel
          float  *WeightsZ = 0;
          WeightsZ = new float [Wz];
          if (!WeightsZ)
               return;

          float Half = (float)(Wz-1)/2.0;

         /* for (unsigned int Weight = 1; Weight < Half; ++Weight)
          {
               const float  z = Half * float (Weight) / Half;
               WeightsZ[(int)Half - Weight] = WeightsZ[(int)Half + Weight] = pi_sigma * exp(-z * z * sigma_s2) ; // Corresponding symmetric WeightsZ
          }*/

          for (unsigned int Weight = 0; Weight <= Half; ++Weight)
          {
              const float  z = float(Weight)-Half;
              WeightsZ[Weight] = WeightsZ[Wz-Weight-1] = pi_sigma * exp(-(z * z *sigma_s2)); // Corresponding symmetric WeightsZ
          }


          double k = 0.;
          for (unsigned int Weight = 0; Weight < Wz; ++Weight)
               k += WeightsZ[Weight];

          for (unsigned int Weight = 0; Weight < Wz; ++Weight)
               WeightsZ[Weight] /= k;

          //	along z
          float  *extension_bufferZ = 0;
          extension_bufferZ = new float [P + (Wz<<1)];

          unsigned int offset = Wz>>1;
          const float *extStop = extension_bufferZ + P + offset;

          for(V3DLONG iy = 0; iy < M; iy++)
          {
               for(V3DLONG ix = 0; ix < N; ix++)
               {

                    float  *extIter = extension_bufferZ + Wz;
                    for(V3DLONG iz = 0; iz < P; iz++)
                    {
                         *(extIter++) = pImage[iz*M*N + iy*N + ix];
                    }

                    //   Extend image
                    const float  *const stop_line = extension_bufferZ - 1;
                    float  *extLeft = extension_bufferZ + Wz - 1;
                    const float  *arrLeft = extLeft + 2;
                    float  *extRight = extLeft + P + 1;
                    const float  *arrRight = extRight - 2;

                    while (extLeft > stop_line)
                    {
                         *(extLeft--) = *(arrLeft++);
                         *(extRight++) = *(arrRight--);
                    }

                    //	Filtering
                    extIter = extension_bufferZ + offset;

                    float  *resIter = &(pImage[iy*N + ix]);

                    while (extIter < extStop)
                    {
                         double sum = 0.;
                         const float  *weightIter = WeightsZ;
                         const float  *const End = WeightsZ + Wz;
                         const float * arrIter = extIter;
                         while (weightIter < End)
                              sum += *(weightIter++) * float (*(arrIter++));
                         extIter++;
                         *resIter = sum;
                         resIter += M*N;

                         //for rescale
                         if(max_val<*arrIter) max_val = *arrIter;
                         if(min_val>*arrIter) min_val = *arrIter;

                    }

               }
          }

          //de-alloc
          if (WeightsZ) {delete []WeightsZ; WeightsZ=0;}
          if (extension_bufferZ) {delete []extension_bufferZ; extension_bufferZ=0;}


     }

    outimg = pImage;


    return;
}
