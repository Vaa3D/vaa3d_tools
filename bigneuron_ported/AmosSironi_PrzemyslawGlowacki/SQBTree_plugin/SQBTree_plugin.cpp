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
#include <math.h>

#include "sqb_0.1/src/MatrixSQB/vaa3d_link.h"

#include "regression/sep_conv.h"
#include "regression/sampling.h"
#include "regression/regressor.h"
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

//#define INF 1E9


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

    //sample call: ./vaa3d -x SQBTree -f train -i /cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/regression/cropped_N2_unit8.tif -o /cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/regression/regressor_path_DEBUG.cfg -p TODO.swc
    //sample call: ./vaa3d -x SQBTree -f train -i /cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/regression/cropped_N2_unit8.tif -o /cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/regression/regressor_path_DEBUG_long.cfg -p TODO.swc


     cout<<"Reading Input Files and Parameters."<<endl;
    if (output.size() != 1) return false;
    // vector<char*> *regressor_output_file = ((vector<char*> *)(output.at(0).p));
     char * regressor_output_file = ((vector<char*> *)(output.at(0).p))->at(0);

    vector<char*> *trainImagePaths = ((vector<char*> *)(input.at(0).p)); //train images

    unsigned int n_train_images = trainImagePaths->size(); //

   // cout<< "Number input train images= " <<n_train_images<<endl;
   // cout<< "name train image 1= " <<trainImagePaths->at(0)<<endl;
    if (n_train_images < 1){

        cout<<"No input train images!"<<endl;
        return false;
    }
//    if (2*n_train_images !=  input.size()){
//        cout<<"Number of input train images and swc files must be the same !"<<endl;
//        return false;
//    }

    //first n_train_images parameters are swc files
    vector<char*> *parameters = ((vector<char*> *)(input.at(1).p));
        if(parameters->size() < n_train_images){
            cout<<"Number of input train images and swc files must be the same !"<<endl;
            return false;
        }

        vector<char*> trainGtSwcPaths(n_train_images);
        for(unsigned int i_swc = 0; i_swc<n_train_images; i_swc++){
            trainGtSwcPaths.at(i_swc) =  parameters->at(i_swc);
        }
          //  classifier_filename = (paras.at(0));

    ////boost parameters (TODO pass as arguments)
    const unsigned max_boost_iters = 250;
    const unsigned max_depth_wl_tree = 2;
    char * loss_type = "squaredloss";
    const double shrink_factor = 0.1;
    unsigned int m_try =200;


    /// TODO: pass n_samples_tot and n_neg_samples_tot as parameter
    unsigned int n_pos_samples_tot =100000;
    n_pos_samples_tot = 2*(n_pos_samples_tot/2); //ensure it is even
    unsigned int n_neg_samples_tot =100000;
    n_neg_samples_tot = 2*(n_neg_samples_tot/2); //ensure it is even

    unsigned int n_samples_tot =n_pos_samples_tot+ n_neg_samples_tot;

    ////get filters to compute features
    /// TODO: pass path as parameter
  //  const char *weight_file = "/cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/sep_conv2/filters/oof_fb_3d_scale_5_weigths_cpd_rank_25_rot_n1.txt";
  //  const char *sep_filters_file = "/cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/sep_conv2/filters/oof_fb_3d_scale_5_sep_cpd_rank_25_rot_n1.txt";
const char *weight_file ="/cvlabdata1/cvlab/datasets_amos/data3D/filter_banks/learned/weights_join_learned_gauss_21_rank_80.txt";
    const char *sep_filters_file ="/cvlabdata1/cvlab/datasets_amos/data3D/filter_banks/learned/sep_join_learned_gauss_21_rank_80.txt";

    MatrixTypeDouble weights = readMatrix(weight_file);
    MatrixTypeDouble sep_filters = readMatrix(sep_filters_file);
    MatrixTypeFloat sep_filters_float = sep_filters.cast<float>();
    MatrixTypeFloat weights_float = weights.cast<float>();

    unsigned int n_features_tot = weights_float.cols();//for now just number of non-sep filters (TODO: add context features)
    const float scale_factor =1.0; //no rescale filter
    unsigned int channel = 1; //only process first channel of image

    //m_try = (unsigned int)(sqrt(weights_float.cols()));


    // cout<<"Computing Features!"<<endl;
    //// get gt, compute features and random samples
    unsigned int n_pos_samples_per_image;
    unsigned int n_neg_samples_per_image;
    unsigned int collected_pos_samples = 0;
    unsigned int collected_neg_samples = 0;

    MatrixTypeFloat all_samples_features = MatrixTypeFloat::Zero(n_samples_tot,n_features_tot) ;
    VectorTypeFloat all_samples_gt = VectorTypeFloat::Zero(n_samples_tot) ;

    MatrixTypeFloat sampled_features_image;
    VectorTypeFloat sampled_gt_vector_image;
    for(unsigned int i_img =0; i_img<n_train_images; i_img++){

        cout<<"Processing Train Image "<< i_img+1<< "/"<< n_train_images << endl;

        if(i_img<n_train_images-1)
        {
            n_pos_samples_per_image = n_pos_samples_tot/n_train_images;
            n_neg_samples_per_image = n_neg_samples_tot/n_train_images;
        }
        else
        {
            n_pos_samples_per_image = n_pos_samples_tot - collected_pos_samples;
            n_neg_samples_per_image = n_neg_samples_tot - collected_neg_samples;
        }


     //   cout<<"n samples pos img "<< n_pos_samples_per_image << endl;
     //   cout<<"n samples neg img"<< n_neg_samples_per_image << endl;

        char * train_img_file = trainImagePaths->at(i_img);
        cout<<"image file "<< train_img_file << endl;

        char * swc_gt_file = trainGtSwcPaths.at(i_img);
        cout<<"swc file "<< swc_gt_file << endl;

        Image4DSimple *train_img = callback.loadImage(train_img_file);
        if (!train_img || !train_img->valid())
        {
          v3d_msg("Fail to open the image file.", 0);
          return false;
        }

        V3DLONG train_img_size[4];
        train_img_size[0] = train_img->getXDim();
        train_img_size[1] = train_img->getYDim();
        train_img_size[2] = train_img->getZDim();
        train_img_size[3] = train_img->getCDim();

        V3DLONG n_pixels = train_img_size[0]*train_img_size[1]*train_img_size[2];

        ImagePixelType pixel_type = train_img->getDatatype();

        ////convert image to itk format
        ITKImageType::Pointer train_img_ITK  =  ITKImageType::New();

        train_img_ITK =v3d2ItkImage<ITKImageType,ImageScalarType>(train_img,train_img_size,channel);

         cout<<"Loaded and converted image!"<<endl;

        ////TODO ! convert swc file to distance gt image
        ITKImageType::Pointer train_gt_ITK  =  ITKImageType::New();
        train_gt_ITK =swc2ItkImage<ITKImageType,ImageScalarType>(swc_gt_file,train_img_size);//for now return null poinyter !

        //for now load gt
        char * train_gt_file = "/cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/regression/cropped_N2_scaled_exp_dist_gt.tif";
        cout<<"gt file "<< train_gt_file << endl;
        Image4DSimple *train_gt = callback.loadImage(train_gt_file);
        if (!train_gt || !train_gt->valid())
        {
          v3d_msg("Fail to open the gt image file.", 0);
          return false;
        }
train_gt_ITK = v3d2ItkImage<ITKImageType,ImageScalarType>(train_gt,train_img_size,channel);

        //convert gt to vector
        VectorTypeFloat train_gt_vector = itkImage2EigenVector<ITKImageType,VectorTypeFloat>(train_gt_ITK,n_pixels,n_pixels);

        train_gt_vector = train_gt_vector*409.5;

   //     cout << "min gt: "<< train_gt_vector.minCoeff() << " max gt: " << train_gt_vector.maxCoeff()  << endl;

        //for debug
//        VectorTypeFloat train_gt_vector = VectorTypeFloat::Zero(n_pixels);
//        for(unsigned int i = 0; i<n_pixels/2; i++){
//train_gt_vector.row(i) << 100.0;
//        }
//std::cout << train_gt_vector << "\n\n";

        ////compute features (TODO add context features)
        cout<<"Computing Features..."<<endl;
        MatrixTypeFloat features_image = computeFeaturesSepComb<ITKImageType,MatrixTypeFloat,VectorTypeFloat>(train_img_ITK,sep_filters_float,weights_float, scale_factor);
        cout<<"Computing Features...Done!"<<endl;

cout<<"Rows Features: "<<features_image.rows()<<" Cols Features: "<<features_image.cols() <<endl;
cout<< "n pixels: "<< n_pixels << endl;


        ////random sampling
        cout<<"Getting Samples..."<<endl;
        bool got_samples = getTrainSamplesFeaturesAndGt<MatrixTypeFloat,VectorTypeFloat>(features_image,train_gt_vector,sampled_features_image, sampled_gt_vector_image,n_pos_samples_per_image,n_neg_samples_per_image);

        if(!got_samples){
            cout<< "Problems encountered while getting samples ! " << endl;
            return false;
        }
cout<<"Getting Samples...Done!"<<endl;


   //     cout <<all_samples_features.middleRows(i_img*(collected_pos_samples+collected_neg_samples), n_pos_samples_per_image+n_neg_samples_per_image).rows() << " "<<  all_samples_features.middleRows(i_img*(collected_pos_samples+collected_neg_samples), n_pos_samples_per_image+n_neg_samples_per_image -1).cols()  << endl;
   //     cout << sampled_features_image.rows() << " " << sampled_features_image.cols() << endl;

        all_samples_features.middleRows(i_img*(collected_pos_samples+collected_neg_samples), n_pos_samples_per_image+n_neg_samples_per_image ) = sampled_features_image;//P.middleRows(i, rows) <=> P(i+1:i+rows, :)
        all_samples_gt.middleRows(i_img*(collected_pos_samples+collected_neg_samples), n_pos_samples_per_image+n_neg_samples_per_image ) = sampled_gt_vector_image;

    //    std::cout << sampled_gt_vector_image << "\n\n";

        collected_pos_samples += n_pos_samples_per_image;
        collected_neg_samples += n_neg_samples_per_image;
    }

    cout<<"Processed all Train Images "<< endl;
    cout<<"Collected " << collected_pos_samples << " Pos samples and " <<collected_neg_samples << " negative samples."<< endl;

    //clean up: delete train_img_ITK, train_gt_ITK, train_gt_vector,features_image,weights, sep_filters

    cout<<"Starting Training..."<<endl;


//cout<<"Max features: "<< all_samples_features.cast<float>().maxCoeff() <<"Min features: "<< all_samples_features.cast<float>().minCoeff()  << endl;
//cout<<"Max gt: "<< all_samples_gt.cast<double>().maxCoeff() <<"Min gt: "<< all_samples_gt.cast<double>().minCoeff()  << endl;
    //SQB wants double labels and float features
    trainRegressor(all_samples_features.cast<float>(),all_samples_gt.cast<double>(),regressor_output_file,loss_type,max_boost_iters,max_depth_wl_tree,shrink_factor,m_try);
   cout<<"Training Done!"<<endl;


    //debug
   // MatrixTypeFloat all_samples_features_debug = MatrixTypeFloat::Random(200,5);
   // VectorTypeFloat all_samples_gt_debug  = VectorTypeFloat::Random(200);
   // trainRegressor(all_samples_features_debug.cast<float>(),all_samples_gt_debug.cast<double>(),regressor_output_file,loss_type,max_boost_iters,max_depth_wl_tree);





    cout<<"Training Terminated Normally"<<endl;
    return true;
}









bool testTubularityImage(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output)
{

   //sample call ./vaa3d -x SQBTree -f test -i /cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/regression/cropped_N2_unit8.tif -o /cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/regression/cropped_predicted_DEBUG.v3draw -p /cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/regression/regressor_path_DEBUG.cfg
//sample call ./vaa3d -x SQBTree -f test -i /cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/regression/cropped_N2_unit8.tif -o /cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/regression/cropped_predicted_DEBUG.v3draw -p /cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/regression/regressor_path_DEBUG_long.cfg


  cout<<"Welcome this plugin"<<endl;
  if (output.size() != 1) return false;

  //   unsigned int Wx=7, Wy=7, Wz=3, c=1;
  //   float sigma = 1.0;
  //  input_filename = "/cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/regression/cropped_N2.tif";
  const char *regressor_filename;
  if (input.size()>=2)
  {
    vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
        if(paras.size() < 1) return false;
        regressor_filename = (paras.at(0));
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
   // const char *weight_file = "/cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/sep_conv2/filters/oof_fb_3d_scale_5_weigths_cpd_rank_25_rot_n1.txt";
   // const char *sep_filters_file = "/cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/sep_conv2/filters/oof_fb_3d_scale_5_sep_cpd_rank_25_rot_n1.txt";
    const char *weight_file ="/cvlabdata1/cvlab/datasets_amos/data3D/filter_banks/learned/weights_join_learned_gauss_21_rank_80.txt";
        const char *sep_filters_file ="/cvlabdata1/cvlab/datasets_amos/data3D/filter_banks/learned/sep_join_learned_gauss_21_rank_80.txt";

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
      MatrixTypeFloat nonsep_features_all = convolveSepFilterBankComb<ITKImageType,MatrixTypeFloat,VectorTypeFloat>(I,sep_filters_float,weights_float, scale_factor);
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
    ///
   std::cout << "Loading Regressor...";
   // const char *regressor_file = "TODO.cfg";

    libconfig::Config cfg;

    // Read the file. If there is an error, report it and exit.
    try
    {
      cfg.readFile(regressor_filename);
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
    std::cout<< "Done!"<< std::endl; //loaded regressor

    //need to convert format?
    //gFeatArrayType feats = Eigen::Map< const gFeatArrayType >( testFeaturesArray, testFeaturesRowsNo, testFeaturesColsNo );


    std::cout << "Predicting...";
    TreeBoosterType::ResponseArrayType newScores;
    newScores = TreeBoosterType::ResponseArrayType::Zero(nonsep_features_all.rows());

//    for (unsigned i=0; i < 10; i++) {
//      std::cout << newScores.coeff(i) << std::endl;
//    }


//    MatrixTypeFloat nonsep_features_all_debug = MatrixTypeFloat::Zero(nonsep_features_all.rows(),nonsep_features_all.cols());
//    MatrixTypeFloat nonsep_features_all_debug = MatrixTypeFloat::Random(20,5);
//        for (unsigned i=0; i < 10; i++) {
//          std::cout << nonsep_features_all_debug.coeff(i) << std::endl;
//    //      outMatrix.data()[i] = newScores.coeff(i);
//        }

    TB.predict( TreeBoosterType::SampleListType(nonsep_features_all),
                TreeBoosterType::FeatureValueObjectType(nonsep_features_all),
                newScores,
                maxIters );
    std::cout<< "Done!"<< std::endl;

//    std::cout << " "<<std::endl;
//    for (unsigned i=0; i <20; i++) {
//      std::cout << newScores.coeff(i) << std::endl;
////      outMatrix.data()[i] = newScores.coeff(i);
//    }

    ////save image
    cout<<"saving image"<<endl;
    // save image
    Image4DSimple outimg1;

    //VectorTypeFloat newScoresFloat = newScores.cast<float>();
    //VectorTypeFloat newScoresUchar = newScores.cast<unsigned char>();


    cout << "min pred: "<< newScores.minCoeff() << " max predic: " << newScores.maxCoeff()  << endl;
    //cout << "min pred int: "<< newScoresUchar.minCoeff() << " max predic int: " << newScoresUchar.maxCoeff()  << endl;


    //copy data to save with v3d format
    V3DLONG n_pixels = in_sz[0]*in_sz[1]*in_sz[2];
   float* out_data_copy = new float[n_pixels];
   for(unsigned int i_pix = 0; i_pix < n_pixels; i_pix++){
        out_data_copy[i_pix] = (float)newScores(i_pix);
   }




   // float* resultC = newScoresFloat.data();
 //   float* resultC ;
//Map<VectorTypeFloat>( resultC, newScores.rows() ) =   newScoresFloat;
    //cout << "num feat: " << nonsep_features_all.rows()<<  endl;
    //cout << "num data: " << newScores.rows()<<  endl;
    //cout << "num out: " << in_sz[0]*in_sz[1]*in_sz[2]<<  endl;

  //  Map<MatrixTypeDouble>( resultC, newScores.rows(), newScores.cols() ) =   newScores;
    //  outimg1.setData((unsigned char *)inimg->getRawDataAtChannel(c), in_sz[0], in_sz[1], in_sz[2], 1, pixel_type);
    //outimg1.setData((unsigned char *)outimg, in_sz[0], in_sz[1], in_sz[2], 1, V3D_FLOAT32);
    outimg1.setData((unsigned char *)(out_data_copy), in_sz[0], in_sz[1], in_sz[2], 1, V3D_FLOAT32); //this gives seg fault


    //   cout<<outimg_file<<endl;
    callback.saveImage(&outimg1, outimg_file);

    // if(inimg) {delete inimg; inimg =0;}
  }

   cout<<"Testing Terminated Normally"<<endl;
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








