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
//#include "regression/regression_test2.h"


//#include <SQB/Core/RegTree.h>
//#include <SQB/Core/Utils.h>

//#include <SQB/Core/Booster.h>

//#include <SQB/Core/LineSearch.h>


//using namespace Eigen;
typedef Eigen::VectorXf VectorTypeFloat;
typedef Eigen::VectorXd VectorTypeDouble;
typedef Eigen::MatrixXd MatrixTypeDouble;
typedef Eigen::MatrixXf MatrixTypeFloat;


typedef SQB::TreeBoosterWeightsType  WeightsType;
typedef float   FeatsType;

typedef Eigen::Array<FeatsType, Eigen::Dynamic, Eigen::Dynamic>     gFeatArrayType;
typedef Eigen::Array<WeightsType, Eigen::Dynamic, 1>                gResponseArrayType;
typedef Eigen::Array<WeightsType, Eigen::Dynamic, 1>                gWeightsArrayType;


typedef float ImageScalarType;
typedef itk::Image< ImageScalarType, 3 >         ITKImageType;


typedef SQB::MatrixFeatureIndexList<gFeatArrayType>          MatrixFeatureIndexListType;
typedef SQB::MatrixSampleIndexList<gFeatArrayType>           MatrixSampleIndexListType;
typedef SQB::MatrixFeatureValueObject<gFeatArrayType>        MatrixFeatureValueObjectType;
typedef SQB::MatrixSingleResponseValueObject<gResponseArrayType>   MatrixClassifResponseValueObjectType;


typedef SQB::TreeBooster<
            MatrixSampleIndexListType,
            MatrixFeatureIndexListType,
            MatrixFeatureValueObjectType,
            MatrixClassifResponseValueObjectType >      TreeBoosterType;

using namespace std;
Q_EXPORT_PLUGIN2(SQBTree, SQBTreePlugin);

#define INF 1E9


//void callSQBTree_mex(int nlhs, void *plhs[], int nrhs, void *prhs[])
//{
//   sqb_entrance( nlhs, plhs,  nrhs, prhs);
//}

bool trainTubularityImage(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output);

bool testTubularityImage(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output);


//template<typename ImageType>
//typename ImageType::Pointer rawData2ItkImage(typename ImageType::PixelType *data1d, const long int  *in_sz);

//template <class T> void convolveV3D(T* data1d,
//                     V3DLONG *in_sz,
//                     float* &outimg);
//template<typename ImageType>
//void rawData2ItkImage(typename ImageType::PixelType *data1d);

//template<typename ImageType>
//typename ImageType::Pointer rawData2ItkImage(unsigned char *data1d,const long int *in_sz);
//void convolveV3D(typename ImageType::PixelType *data1d,V3DLONG *in_sz,float* &outimg);
void convolveV3D(unsigned char *data1d,V3DLONG *in_sz,float* &outimg, const unsigned int unit_bites,V3DPluginCallback2 &callback);

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
  //	vector<char*> infiles, inparas, outfiles;
  //	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
  //	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
  //	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

  if(func_name == tr("train"))
  {
      return trainTubularityImage(callback, input, output);

  }
  else if (func_name == tr("test")) // apply already trained classifier to an image and save results
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
    v3d_msg("To be implemented.");
  }
  else return false;

  return true;
}


bool trainTubularityImage(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output)
{

    if (output.size() != 1) return false;

    vector<char*> *trainImagePaths = ((vector<char*> *)(input.at(0).p));

    unsigned int n_train_images = trainImagePaths->size()/2; //half inputs are images, second half are swc gt files
    if (n_train_images < 1){

        cout<<"No input train images!"<<endl;
        return false;
    }
    if (2*n_train_images !=  input.size()){
        cout<<"Number of input train images and swc files must be the same !"<<endl;
        return false;
    }

    unsigned int channel = 1; //only process first channel

    //// get features and gt
    for(unsigned int i_img =0; i_img<n_train_images; i_img++){

        char * train_img_file = trainImagePaths->at(i_img);
        char * swc_gt_file = trainImagePaths->at(i_img+n_train_images);

        Image4DSimple *train_img = callback.loadImage(train_img_file);
        if (!train_img || !train_img->valid())
        {
          v3d_msg("Fail to open the image file.", 0);
          return false;
        }

        V3DLONG in_sz[4];
        in_sz[0] = train_img->getXDim();
        in_sz[1] = train_img->getYDim();
        in_sz[2] = train_img->getZDim();
        in_sz[3] = train_img->getCDim();


        ImagePixelType pixel_type = train_img->getDatatype();

        ////convert image to itk format
        ITKImageType::Pointer train_img_ITK  =  ITKImageType::New();

        train_img_ITK =v3d2ItkImage<ITKImageType,ImageScalarType>(train_img,in_sz,channel);

        ////convert swc file to diastance gt image


    }

    cout<<"Starting Training!"<<endl;



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
    //    if(paras.size() >= 1) classifier_filename = (paras.at(0));
    //   if(paras.size() >= 2) Wy = atoi(paras.at(1));
    //   if(paras.size() >= 3) Wz = atoi(paras.at(2));
    //   if(paras.size() >= 4) c = atoi(paras.at(3));
    //   if(paras.size() >= 5) sigma = atof(paras.at(4));
  }

  vector<char*> *inputImagePaths = ((vector<char*> *)(input.at(0).p));
  vector<char*> *outputImagePaths = ((vector<char*> *)(output.at(0).p));

  if(inputImagePaths->size() != outputImagePaths->size()) {
    std::cerr << "The number of input images is different than the number of output images." << std::endl;
    return false;
  } else {
    std::cout << "inputImagePaths->size():  " << inputImagePaths->size() << std::endl
              << "outputImagePaths->size(): " << outputImagePaths->size() << std::endl;
  }

  for(unsigned int imgInd = 0; imgInd < inputImagePaths->size(); imgInd++) {
    char * inimg_file = inputImagePaths->at(imgInd);
    char * outimg_file = outputImagePaths->at(imgInd);
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
    convolveV3D((unsigned char *)inimg->getRawDataAtChannel(c), in_sz, outimg,unit_bites,callback);

    ////convert image to itk format
//    typedef unsigned char ImageScalarType;
   // typedef float ImageScalarType;
   // typedef itk::Image< ImageScalarType, 3 >         ITKImageType;

    ITKImageType::Pointer I  =  ITKImageType::New();
    I =v3d2ItkImage<ITKImageType,ImageScalarType>(inimg,in_sz,c);


//    typedef itk::Image< ImageScalarType, 3 >         ITKImageType;
//    ITKImageType::Pointer I  =  ITKImageType::New();

//    I =rawData2ItkImage<ITKImageType>((unsigned char *)inimg->getRawDataAtChannel(c),in_sz);

//   ////rescale in [0 1]
//     typedef itk::RescaleIntensityImageFilter< ITKImageType, ITKImageType > RescaleFilterType;
//    RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
//    rescaleFilter->SetInput(I);
//    rescaleFilter->SetOutputMinimum(0.0);
//    rescaleFilter->SetOutputMaximum(1.0);


    ////load filters
    const char *weight_file = "/cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/sep_conv2/filters/oof_fb_3d_scale_5_weigths_cpd_rank_25_rot_n1.txt";
    const char *sep_filters_file = "/cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/sep_conv2/filters/oof_fb_3d_scale_5_sep_cpd_rank_25_rot_n1.txt";

    MatrixTypeDouble weights = readMatrix(weight_file);
    MatrixTypeDouble sep_filters = readMatrix(sep_filters_file);

    std::cout << "Loaded weights ! max :" << weights.maxCoeff() <<std::endl;

    ////convolve image

   // template<typename ImageType, typename MatrixType, typename VectorType>
    const float scale_factor =1.0;
    MatrixTypeFloat sep_filters_float = sep_filters.cast<float>();
    MatrixTypeFloat weights_float = weights.cast<float>();
   // ITKImageType::Pointer rescaled_img = rescaleFilter->GetOutput();
    //rescaleFilter->Update();
      MatrixXf nonsep_features_all = convolveSepFilterBankComb<ITKImageType,MatrixXf,VectorTypeFloat>(I,sep_filters_float,weights_float, scale_factor);
  //  MatrixTypeFloat nonsep_features_all = convolveSepFilterBankComb<ITKImageType,MatrixTypeFloat,VectorTypeFloat>(I,sep_filters_float,weights_float, scale_factor);
//MatrixTypeFloat sep_features_all = convolveSepFilterBank<ITKImageType,MatrixTypeFloat,VectorTypeFloat>(I,sep_filters_float);

//convolve 3D image with separable filters (split eigen vector in 3 equal parts)
//template<typename ImageType, typename VectorType>
//Matrix3D<float> outxx;
// wholeConvolveSepFilterSplitVec<ITKImageType,VectorTypeFloat>( I, sep_filters_float.col(0),outxx);
//float s = 1.0;
//int t = dumbfun<float,int>(s);

//    MatrixTypeDouble feats_all =  convolveSepFilterBankComb<ITKImageType,MatrixTypeDouble,VectorTypeDouble>( I, sep_filters, weights, scale_factor );

    ////apply trained regressor
    const char *regressor_file = "TODO.cfg";

    libconfig::Config cfg;

    // Read the file. If there is an error, report it and exit.
    try
    {
      cfg.readFile(regressor_file);
    }
    catch(const libconfig::FileIOException &fioex)
    {
      std::cerr << "I/O error while reading file." << std::endl;
//      return(EXIT_FAILURE);
    }
    catch(const libconfig::ParseException &pex)
    {
      std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
                << " - " << pex.getError() << std::endl;
//      return(EXIT_FAILURE);
    }

    libconfig::Setting &root = cfg.getRoot();

    libconfig::Setting &regressor = root["regressor"];

    TreeBoosterType TB;
    TB.loadFromLibconfig(regressor);

    unsigned maxIters = TB.numWeakLearners();

    //need to convert format?
    //gFeatArrayType feats = Eigen::Map< const gFeatArrayType >( testFeaturesArray, testFeaturesRowsNo, testFeaturesColsNo );

    TreeBoosterType::ResponseArrayType newScores;
    TB.predict( TreeBoosterType::SampleListType(nonsep_features_all),
                TreeBoosterType::FeatureValueObjectType(nonsep_features_all),
                newScores,
                maxIters );


    ////save image
    cout<<"saving image"<<endl;
    // save image
    Image4DSimple outimg1;
    //  outimg1.setData((unsigned char *)inimg->getRawDataAtChannel(c), in_sz[0], in_sz[1], in_sz[2], 1, pixel_type);
    outimg1.setData((unsigned char *)outimg, in_sz[0], in_sz[1], in_sz[2], 1, V3D_FLOAT32);


    //   cout<<outimg_file<<endl;
    callback.saveImage(&outimg1, outimg_file);

    // if(inimg) {delete inimg; inimg =0;}
  }

  return true;


}




//template<typename ImageType>
//void convolveV3D(typename ImageType::PixelType *data1d,V3DLONG *in_sz,float* &outimg){
void convolveV3D(unsigned char *data1d,V3DLONG *in_sz,float* &outimg, const unsigned int unit_bites,V3DPluginCallback2 &callback){

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



  I =rawData2ItkImage<ITKImageType>(data1d,in_sz);


  ITKImageType::SizeType size_image = I->GetLargestPossibleRegion().GetSize();
  std::cout << "size image " <<size_image <<std::endl;


  V3DLONG N = in_sz[0];
  V3DLONG M = in_sz[1];
  V3DLONG P = in_sz[2];
  V3DLONG sc = in_sz[3];
  V3DLONG pagesz = N*M*P;
  float *data1d_after_itk = new float [pagesz];

  for(int iz = 0; iz < in_sz[2]; iz++)
  {
    int offsetk = iz*in_sz[1]*in_sz[0];
    for(int iy = 0; iy < in_sz[1]; iy++)
    {
      int offsetj = iy*in_sz[0];
      for(int ix = 0; ix < in_sz[0]; ix++)
      {

        //  //                cout<< offsetk + offsetj + ix ;
        itk::Index<3> indexX;
        indexX[0] = ix;
        indexX[1] = iy;
        indexX[2] = iz;
        unsigned char PixelVaule =  I->GetPixel(indexX);//data1d[offsetk + offsetj + ix];

        data1d_after_itk[offsetk + offsetj + ix] = (float)PixelVaule;

        //cout<< ": " << indexX <<endl;
        //I->SetPixel(indexX, PixelVaule);
      }
    }

  }


  std::cout << "copied values back " <<std::endl;

  //OK  !
  //  Image4DSimple outimg_after_itk;
  //  outimg_after_itk.setData(data1d_after_itk, in_sz[0], in_sz[1], in_sz[2], 1, V3D_UINT8);
  //  cout<<"saving image itk copy"<<endl;
  //  char *  name_out_after_itk = "/cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/regression/cropped_N2_unit8_after_itk.v3draw";
  //  callback.saveImage(&outimg_after_itk, name_out_after_itk);
  //OK  !


  //unsigned char * inimg1d = p4DImage->getRawDataAtChannel(c);

  /*
        std::cout << "unit_bites " <<unit_bites <<std::endl;
       V3DLONG tb = in_sz[0]*in_sz[1]*in_sz[2]*unit_bites;
       float *pImage = new float [tb];


//V3DLONG pagesz = in_sz[0]*in_sz[1]*in_sz[2];
       for(V3DLONG i=0; i<pagesz; i++)
            pImage[i] = data1d[i];
*/

  outimg = data1d_after_itk;


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








