/* SQBTree_plugin.cpp
 * a plugin for test
 * 2015-5-5 : by HP
 */
 
#define myQDebug(...) printf (__VA_ARGS__)
#define myQFatal(...) do{ printf (__VA_ARGS__); exit(1); } while(0)

#include "v3d_message.h"
#include <vector>
#include "SQBTree_plugin.h"

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


//void callSQBTree_mex(int nlhs, void *plhs[], int nrhs, void *prhs[])
//{
//   sqb_entrance( nlhs, plhs,  nrhs, prhs);
//}


bool testTubularityImage(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output);
//template<typename ImageType>
//typename ImageType::Pointer Imcreate(typename ImageType::PixelType *data1d, const long int  *in_sz);

//template <class T> void convolveV3D(T* data1d,
//                     V3DLONG *in_sz,
//                     float* &outimg);
//template<typename ImageType>
//void Imcreate(typename ImageType::PixelType *data1d);

template<typename ImageType>
typename ImageType::Pointer Imcreate(float *data1d,const long int *in_sz);
//void convolveV3D(typename ImageType::PixelType *data1d,V3DLONG *in_sz,float* &outimg);
void convolveV3D(float *data1d,V3DLONG *in_sz,float* &outimg);

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
		v3d_msg("To be implemented.");
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

   //  switch (inimg->getDatatype())
    // {
    //      case V3D_UINT8: convolveV3D(inimg->getRawData(), in_sz, outimg); break;
     //     case V3D_UINT16: convolveV3D((unsigned short int*)(inimg->getRawData()), in_sz, outimg); break;
      //    case V3D_FLOAT32: convolveV3D((float *)(inimg->getRawData()), in_sz, outimg); break;
       //   default:
     //          v3d_msg("Invalid datatype in convolveV3D.", 0);
      //         if (inimg) {delete inimg; inimg=0;}
     //          return false;
    // }

   //  typedef itk::Image<unsigned char, 3> ImageTypeUINT8;
//     typedef itk::Image<float, 3> ImageTypeFloat;
//     switch (inimg->getDatatype())
//     {
//           case V3D_UINT8: convolveV3D<ImageTypeUINT8>(inimg->getRawData(), in_sz, outimg); break;
//          // case V3D_UINT16: typedef itk::Image<V3D_UINT16, 3> ImageType;; break;
//           case V3D_FLOAT32: convolveV3D<ImageTypeFloat>((float *)inimg->getRawData(), in_sz, outimg);  break;
//     default:
//                   v3d_msg("Invalid datatype in convolveV3D.", 0);
//                   if (inimg) {delete inimg; inimg=0;}
//                   return false;
//     }


    convolveV3D((float *)inimg->getRawData(), in_sz, outimg);


     // save image
     Image4DSimple outimg1;
     outimg1.setData((unsigned char *)outimg, in_sz[0], in_sz[1], in_sz[2], 1, V3D_FLOAT32);

     callback.saveImage(&outimg1, outimg_file);

     if(inimg) {delete inimg; inimg =0;}

     return true;
}


//template<typename ImageType>
//void convolveV3D(typename ImageType::PixelType *data1d,V3DLONG *in_sz,float* &outimg){
void convolveV3D(float *data1d,V3DLONG *in_sz,float* &outimg){

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

    typedef float ImageScalarType;
    typedef itk::Image< ImageScalarType, 3 >         ITKImageType;
    typedef itk::Image<float, 3> ImageType;
   ITKImageType::Pointer I  =  ITKImageType::New();
   I =Imcreate<ITKImageType>(data1d,in_sz);


       ITKImageType::SizeType size_image = I->GetLargestPossibleRegion().GetSize();
       std::cout << "size image " <<size_image <<std::endl;


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
    return;
}





template<typename ImageType>
//void Imcreate(ImageType::PixelType *data1d){
typename ImageType::Pointer Imcreate(float *data1d,const long int *in_sz){
//typename ImageType::Pointer Imcreate(typename ImageType::PixelType *data1d,const long int *in_sz){

    //typedef itk::Image<signed int, 3> ImageType;
    unsigned int SN = in_sz[0];
   unsigned int  SM = in_sz[1];
   unsigned int  SZ = in_sz[2];

    typename ImageType::Pointer I  = ImageType::New();
    typename ImageType::SizeType size;
    size[0] = SN;
    size[1] = SM;
    size[2] = SZ;

    typename ImageType::IndexType idx;
    idx.Fill(0);
    typename ImageType::RegionType region;
    region.SetSize( size );
    region.SetIndex( idx );

    I->SetRegions(region);
    I->Allocate();
    I->FillBuffer(0);

    for(int iz = 0; iz < SZ; iz++)
      {
          int offsetk = iz*SM*SN;
          for(int iy = 0; iy < SM; iy++)
          {
              int offsetj = iy*SN;
              for(int ix = 0; ix < SN; ix++)
              {

                  float PixelVaule =  data1d[offsetk + offsetj + ix];
                  itk::Index<3> indexX;
                  indexX[0] = ix;
                  indexX[1] = iy;
                  indexX[2] = iz;
                  I->SetPixel(indexX, PixelVaule);
              }
          }

      }
    return I;

}






