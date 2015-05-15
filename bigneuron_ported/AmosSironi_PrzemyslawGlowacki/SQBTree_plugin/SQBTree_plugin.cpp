/* SQBTree_plugin.cpp
 * a plugin for test
 * 2015-5-5 : by HP
 */
 
#define myQDebug(...) printf (__VA_ARGS__)
#define myQFatal(...) do{ printf (__VA_ARGS__); exit(1); } while(0)

#include "v3d_message.h"
#include <vector>
#include "SQBTree_plugin.h"
#include "sqb_trees.h"


#include "sqb_0.1/src/MatrixSQB/vaa3d_link.h"

#include "regression/sep_conv.h"
#include "regression/regression_test2.h"
//using namespace Eigen;
//typedef Eigen::VectorXf VectorTypeFloat;
//typedef Eigen::VectorXd VectorTypeDouble;
//typedef Eigen::MatrixXd MatrixTypeDouble;
//typedef Eigen::MatrixXf MatrixTypeFloat;


using namespace std;
Q_EXPORT_PLUGIN2(SQBTree, SQBTreePlugin);

#define INF 1E9


//void callSQBTree_mex(int nlhs, void *plhs[], int nrhs, void *prhs[])
//{
//   sqb_entrance( nlhs, plhs,  nrhs, prhs);
//}


bool testTubularityImage(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output);


bool processImage(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output);
template <class T> void gaussian_filter(T* data1d,
                                        V3DLONG *in_sz,
                                        unsigned int Wx,
                                        unsigned int Wy,
                                        unsigned int Wz,
                                        unsigned int c,
                                        double sigma,
                                        float* &outimg);


//template<typename ImageType>
//typename ImageType::Pointer Imcreate(typename ImageType::PixelType *data1d, const long int  *in_sz);

//template <class T> void convolveV3D(T* data1d,
//                     V3DLONG *in_sz,
//                     float* &outimg);
//template<typename ImageType>
//void Imcreate(typename ImageType::PixelType *data1d);

template<typename ImageType>
typename ImageType::Pointer Imcreate(unsigned char *data1d,const long int *in_sz);
//void convolveV3D(typename ImageType::PixelType *data1d,V3DLONG *in_sz,float* &outimg);
void convolveV3D(unsigned char *data1d,V3DLONG *in_sz,float* &outimg, const unsigned int unit_bites);

QStringList SQBTreePlugin::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList SQBTreePlugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void SQBTreePlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("menu1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("a plugin for test. "
			"Developed by HP, 2015-5-5"));
	}
}

bool SQBTreePlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

        if (func_name == tr("test")) // apply already trained classifier to an image and save results
	{
        //const char *input_filename = "/cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/regression/cropped_N2.tif";
        //   testConvolve(input_filename);



            return testTubularityImage(callback, input, output);


        //v3d_msg("To be implemented.");
	}
	else if (func_name == tr("func2"))
	{
    mockTrainAndTest();
    v3d_msg("Hahahahahahahahahahahaha!.");
	}
	else if (func_name == tr("help"))
	{
        v3d_msg("To be implemented jewfwefjewfjiopew.");
	}
	else return false;

	return true;
}



bool testTubularityImage(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output)
{


    /*
    cout<<"Welcome to Gaussian filter"<<endl;
    if (output.size() != 1) return false;

    unsigned int Wx=7, Wy=7, Wz=3, c=1;
     float sigma = 1.0;
     if (input.size()>=2)
     {
          vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
          if(paras.size() >= 1) Wx = atoi(paras.at(0));
          if(paras.size() >= 2) Wy = atoi(paras.at(1));
          if(paras.size() >= 3) Wz = atoi(paras.at(2));
          if(paras.size() >= 4) c = atoi(paras.at(3));
          if(paras.size() >= 5) sigma = atof(paras.at(4));
    }

    char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
    cout<<"Wx = "<<Wx<<endl;
     cout<<"Wy = "<<Wy<<endl;
    cout<<"Wz = "<<Wz<<endl;
     cout<<"c = "<<c<<endl;
     cout<<"sigma = "<<sigma<<endl;
    cout<<"inimg_file = "<<inimg_file<<endl;
    cout<<"outimg_file = "<<outimg_file<<endl;

     double sigma_s2 = 0.5/(sigma*sigma);

    Image4DSimple *inimg = callback.loadImage(inimg_file);
    if (!inimg || !inimg->valid())
    {
        v3d_msg("Fail to open the image file.", 0);
        return false;
    }

     if(c > inimg->getCDim())// check the input channel number range
     {
          v3d_msg("The input channel number is out of real channel range.\n", 0 );
          return false;
     }

    //input
     float* outimg = 0; //no need to delete it later as the Image4DSimple variable "outimg" will do the job

     V3DLONG in_sz[4];
     in_sz[0] = inimg->getXDim();
     in_sz[1] = inimg->getYDim();
     in_sz[2] = inimg->getZDim();
     in_sz[3] = inimg->getCDim();

     switch (inimg->getDatatype())
     {
          case V3D_UINT8: gaussian_filter(inimg->getRawData(), in_sz, Wx, Wy, Wz, c, sigma, outimg); break;
          case V3D_UINT16: gaussian_filter((unsigned short int*)(inimg->getRawData()), in_sz, Wx, Wy, Wz, c, sigma, outimg); break;
          case V3D_FLOAT32: gaussian_filter((float *)(inimg->getRawData()), in_sz, Wx, Wy, Wz, c, sigma, outimg); break;
          default:
               v3d_msg("Invalid datatype in Gaussian fileter.", 0);
               if (inimg) {delete inimg; inimg=0;}
               return false;
     }

     // save image
     Image4DSimple outimg1;
     outimg1.setData((unsigned char *)outimg, in_sz[0], in_sz[1], in_sz[2], 1, V3D_FLOAT32);

     callback.saveImage(&outimg1, outimg_file);

     if(inimg) {delete inimg; inimg =0;}

     return true;

    */




    cout<<"Welcome this plugin"<<endl;
    if (output.size() != 1) return false;

 //   unsigned int Wx=7, Wy=7, Wz=3, c=1;
  //   float sigma = 1.0;
  //  input_filename = "/cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/regression/cropped_N2.tif";
     if (input.size()>=2)
     {
          vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
      //    if(paras.size() >= 1) classifier_filename = atoi(paras.at(0));
       //   if(paras.size() >= 2) Wy = atoi(paras.at(1));
       //   if(paras.size() >= 3) Wz = atoi(paras.at(2));
       //   if(paras.size() >= 4) c = atoi(paras.at(3));
       //   if(paras.size() >= 5) sigma = atof(paras.at(4));
    }

    char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
//    cout<<"Wx = "<<Wx<<endl;
//     cout<<"Wy = "<<Wy<<endl;
//    cout<<"Wz = "<<Wz<<endl;
//     cout<<"c = "<<c<<endl;
//     cout<<"sigma = "<<sigma<<endl;
//
    cout<<"inimg_file = "<<inimg_file<<endl;
    cout<<"outimg_file = "<<outimg_file<<endl;


    Image4DSimple *inimg = callback.loadImage(inimg_file);
    if (!inimg || !inimg->valid())
    {
        v3d_msg("Fail to open the image file.", 0);
        return false;
    }

    cout<<"Loaded image"<<endl;

    //input
     float* outimg = 0; //no need to delete it later as the Image4DSimple variable "outimg" will do the job

     V3DLONG in_sz[4];
     in_sz[0] = inimg->getXDim();
     in_sz[1] = inimg->getYDim();
     in_sz[2] = inimg->getZDim();
     in_sz[3] = inimg->getCDim();


 //    cout<<"size image"<< in_sz[0] <<in_sz[1] <<in_sz[2] <<in_sz[3] << endl;

     ImagePixelType pixel_type = inimg->getDatatype();
 //    cout<<"calling conv3D"<<endl;

 //   convolveV3D((float *)inimg->getRawData(), in_sz, outimg);
     unsigned int c = 1;
     unsigned int unit_bites = inimg->getUnitBytes();


     //cout<<"N bites per pixel:  "<<unit_bites<< endl;

//callback.saveImage(inimg, outimg_file);
     //convolveV3D((unsigned char *)inimg->getRawDataAtChannel(c), in_sz, outimg,unit_bites);


 cout<<"saving image"<<endl;
     // save image
     Image4DSimple outimg1;
     outimg1.setData((unsigned char *)inimg->getRawDataAtChannel(c), in_sz[0], in_sz[1], in_sz[2], 1, pixel_type);


  //   cout<<outimg_file<<endl;
     callback.saveImage(&outimg1, outimg_file);

    // if(inimg) {delete inimg; inimg =0;}

     return true;


}


bool processImage(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output)
{
    cout<<"Welcome to Gaussian filter"<<endl;
    if (output.size() != 1) return false;

    unsigned int Wx=7, Wy=7, Wz=3, c=1;
     float sigma = 1.0;
     if (input.size()>=2)
     {
          vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
          if(paras.size() >= 1) Wx = atoi(paras.at(0));
          if(paras.size() >= 2) Wy = atoi(paras.at(1));
          if(paras.size() >= 3) Wz = atoi(paras.at(2));
          if(paras.size() >= 4) c = atoi(paras.at(3));
          if(paras.size() >= 5) sigma = atof(paras.at(4));
    }

    char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
    cout<<"Wx = "<<Wx<<endl;
     cout<<"Wy = "<<Wy<<endl;
    cout<<"Wz = "<<Wz<<endl;
     cout<<"c = "<<c<<endl;
     cout<<"sigma = "<<sigma<<endl;
    cout<<"inimg_file = "<<inimg_file<<endl;
    cout<<"outimg_file = "<<outimg_file<<endl;

     double sigma_s2 = 0.5/(sigma*sigma);

    Image4DSimple *inimg = callback.loadImage(inimg_file);
    if (!inimg || !inimg->valid())
    {
        v3d_msg("Fail to open the image file.", 0);
        return false;
    }

     if(c > inimg->getCDim())// check the input channel number range
     {
          v3d_msg("The input channel number is out of real channel range.\n", 0 );
          return false;
     }

    //input
     float* outimg = 0; //no need to delete it later as the Image4DSimple variable "outimg" will do the job

     V3DLONG in_sz[4];
     in_sz[0] = inimg->getXDim();
     in_sz[1] = inimg->getYDim();
     in_sz[2] = inimg->getZDim();
     in_sz[3] = inimg->getCDim();

     switch (inimg->getDatatype())
     {
          case V3D_UINT8: gaussian_filter(inimg->getRawData(), in_sz, Wx, Wy, Wz, c, sigma, outimg); break;
          case V3D_UINT16: gaussian_filter((unsigned short int*)(inimg->getRawData()), in_sz, Wx, Wy, Wz, c, sigma, outimg); break;
          case V3D_FLOAT32: gaussian_filter((float *)(inimg->getRawData()), in_sz, Wx, Wy, Wz, c, sigma, outimg); break;
          default:
               v3d_msg("Invalid datatype in Gaussian fileter.", 0);
               if (inimg) {delete inimg; inimg=0;}
               return false;
     }

     // save image
     Image4DSimple outimg1;
     outimg1.setData((unsigned char *)outimg, in_sz[0], in_sz[1], in_sz[2], 1, V3D_FLOAT32);

     callback.saveImage(&outimg1, outimg_file);

     if(inimg) {delete inimg; inimg =0;}

     return true;
}



template <class T> void gaussian_filter(T* data1d,
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













//template<typename ImageType>
//void convolveV3D(typename ImageType::PixelType *data1d,V3DLONG *in_sz,float* &outimg){
void convolveV3D(unsigned char *data1d,V3DLONG *in_sz,float* &outimg, const unsigned int unit_bites){

//template <class T> void convolveV3D(T* data1d,
//                     V3DLONG *in_sz,
//                     float* &outimg)
//{
    if (!data1d || !in_sz || in_sz[0]<=0 || in_sz[1]<=0 || in_sz[2]<=0 || in_sz[3]<=0 || outimg)
    {
        v3d_msg("Invalid parameters to gaussian_filter().", 0);
        return;
    }

    if (outimg)
    {
        v3d_msg("Warning: you have supplied an non-empty output image pointer. This program will force to free it now. But you may want to double check.");
        delete []outimg;
        outimg = 0;
    }

    typedef unsigned char ImageScalarType;
    typedef itk::Image< ImageScalarType, 3 >         ITKImageType;
    //typedef itk::Image<unsigned char, 3> ImageType;
   ITKImageType::Pointer I  =  ITKImageType::New();

   cout<<"creating itk like img"<<endl;



   I =Imcreate<ITKImageType>(data1d,in_sz);


       ITKImageType::SizeType size_image = I->GetLargestPossibleRegion().GetSize();
       std::cout << "size image " <<size_image <<std::endl;



       //unsigned char * inimg1d = p4DImage->getRawDataAtChannel(c);


        std::cout << "unit_bites " <<unit_bites <<std::endl;
       V3DLONG tb = in_sz[0]*in_sz[1]*in_sz[2]*unit_bites;
       float *pImage = new float [tb];


V3DLONG pagesz = in_sz[0]*in_sz[1]*in_sz[2];
       for(V3DLONG i=0; i<pagesz; i++)
            pImage[i] = data1d[i];

       outimg = pImage;


    /*

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




    outimg = pImage;

*/
   // return;
}





template<typename ImageType>
//void Imcreate(ImageType::PixelType *data1d){
typename ImageType::Pointer Imcreate(unsigned char *data1d,const long int *in_sz){
//typename ImageType::Pointer Imcreate(typename ImageType::PixelType *data1d,const long int *in_sz){

    //typedef itk::Image<signed int, 3> ImageType;
    unsigned int SN = in_sz[0];
   unsigned int  SM = in_sz[1];
   unsigned int  SZ = in_sz[2];

   // typename ImageType::Pointer I  = ImageType::New();
    typename ImageType::SizeType size;
    size[0] = SN;
    size[1] = SM;
    size[2] = SZ;

    cout<<"size    "<<size <<endl;


    typename ImageType::RegionType region;
   typename  ImageType::IndexType start;
     start[0] = 0;
     start[1] = 0;
     start[2] = 0;


     region.SetSize(size);
     region.SetIndex(start);

     typename ImageType::Pointer I = ImageType::New();
     I->SetRegions(region);
     I->Allocate();



//    typename ImageType::IndexType idx;
//    idx.Fill(0);
//    typename ImageType::RegionType region;
//    region.SetSize( size );
//    region.SetIndex( idx );

//    I->SetRegions(region);
//    I->Allocate();
//    I->FillBuffer(0);

//    I->Update();

    cout<<"allocating image  " <<endl;


  //  ImageType::RegionType region;
  // / region.SetSize(regionSize);
  //  region.SetIndex(regionIndex);


//    itk::ImageRegionIterator<ImageType> imageIterator(I,region);
//    unsigned int idx_lin= 0;
//    while(!imageIterator.IsAtEnd())
//        {
//            // Get the value of the current pixel
//            //unsigned char val = imageIterator.Get();
//            //std::cout << (int)val << std::endl;

//            // Set the current pixel to white
//            unsigned char PixelVaule =  data1d[idx_lin];
//            if(PixelVaule == NULL){
//                cout<< "NULL POINTER !: " <<idx_lin  << endl;
//            }
//            // PixelVaule = 0.0;
//            imageIterator.Set(PixelVaule);

//            ++imageIterator;
//            ++idx_lin;
//        }


    for(int iz = 0; iz < SZ; iz++)
      {
          int offsetk = iz*SM*SN;
          for(int iy = 0; iy < SM; iy++)
          {
              int offsetj = iy*SN;
              for(int ix = 0; ix < SN; ix++)
              {

//  //                cout<< offsetk + offsetj + ix ;

                  unsigned char PixelVaule =  data1d[offsetk + offsetj + ix];
                  itk::Index<3> indexX;
                  indexX[0] = ix;
                  indexX[1] = iy;
                  indexX[2] = iz;
//cout<< ": " << indexX <<endl;
                  I->SetPixel(indexX, PixelVaule);
              }
          }

      }


    cout<<"allocated image  "<<size <<endl;

    return I;

}






