/* RegressionTubularityAC_plugin.cpp
 * a plugin for training and testing the regression tubularity approach [1].
 * 2015-5-5 : by Amos Sironi and Przemysław Głowacki
 *
 *
 * [1] A. Sironi, E. Türetken, V. Lepetit and P. Fua. Multiscale Centerline Detection, submitted to IEEE Transactions on Pattern Analysis and Machine Intelligence.
 */

#define myQDebug(...) printf (__VA_ARGS__)
#define myQFatal(...) do{ printf (__VA_ARGS__); exit(1); } while(0)

#include "v3d_message.h"
#include <vector>
#include "RegressionTubularityAC_plugin.h"
#include <math.h>

#include "../libs/sqb_0.1/src/MatrixSQB/vaa3d_link.h"

#include "../libs/regression/sep_conv.h"
#include "../libs/regression/sampling.h"
#include "../libs/regression/regressor.h"

#include <sys/stat.h>
#include <sys/types.h>

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


typedef float ImageScalarTypeFloat;
typedef unsigned char ImageScalarTypeUint8;
typedef itk::Image< ImageScalarTypeFloat, 3 >         ITKImageType;


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

Q_EXPORT_PLUGIN2(RegressionTubularityAC, RegressionTubularityACPlugin)

bool trainTubularityImage(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output);

bool resampleImage(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output);

QStringList RegressionTubularityACPlugin::menulist() const
{
  return QStringList()
      <<tr("test")
     <<tr("train")
    <<tr("about");
}

QStringList RegressionTubularityACPlugin::funclist() const
{
  return QStringList()
      <<tr("func1")
     <<tr("func2")
    <<tr("help");
}

void RegressionTubularityACPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
  if (menu_name == tr("test"))
  {
    v3d_msg("To be implemented. Please call this plugin from command line.\n\n "
            "Example call: ./vaa3d -x RegressionTubularityAC -f test   \n "
            "-i <Test_Img1.tif> <Test_Img2.tif> ... <Test_ImgN.tif> \n"
            "-o  <Tubularity_img1.raw> <Tubularity_Img2.raw> ... <Tubularity_ImgN.v3draw> \n"
            "-p  <Path_to_sep_filters_file_im.txt> <Path_to_weights_file_im.txt> <Path_to_sep_filters_file_ac.txt> <Path_to_seigths_file_ac.txt>\n"
            "    <Path_to_Regressor_ac_0.cfg> <Path_to_Regressor_ac_1.cfg> ... <Path_to_Regressor_ac_M.cfg>\n");
  }
  else if (menu_name == tr("train"))
  {
      v3d_msg("To be implemented. Please call this plugin from command line. \n\n"
              "Example call: ./vaa3d -x RegressionTubularityAC -f train \n"
              "-i <Train_Img1.tif> <Train_Img2.tif> ... <Train_ImgN.tif> \n"
              "-o <Output_Results_Dir> \n"
              "-p <GroundTruth_Img1.swc> <GroundTruth_Img2.swc> ... <GroundTruth_imgN.swc> \n"
              "   <Path_to_sep_filters_file_im.txt> <Path_to_weights_file_im.txt> <Path_to_sep_filters_file_ac.txt> <Path_to_seigths_file_ac.txt>\n"
              "   <N_Autocontext_Iters> <N_Train_Samples> <N_boosting_Iters> <Tree_Depth> <Shrinkage_factor> <m_try>\n");
  }
  else
  {
    v3d_msg(tr("A plugin for training and testing a tubularity regressor [1]. \n"
               "You can call this plugin from command line. See README.txt file for more details.\n\n"
               ""
               "[1] A. Sironi, E. Turetken, V. Lepetit and P. Fua. Multiscale Centerline Detection, submitted to IEEE Transactions on Pattern Analysis and Machine Intelligence."));
  }
}

bool RegressionTubularityACPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{

  if(func_name == tr("train")) //train a regressor
  {
      return trainTubularityImage(callback, input, output);

  }
  else if (func_name == tr("test")) // apply already trained regressor to an image and save results
  {

    return testTubularityImage(callback, input, output);

  }
  else if (func_name == tr("resample"))
  {
    return resampleImage(callback, input, output);
  }
  else if (func_name == tr("help"))
  {
    v3d_msg("To be implemented.");
  }
  else return false;

  return true;
}


////filter banks to compute features (use hand-crafter filters)
////image features
////const char *weight_file_im ="../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/data/filter_banks/oof_fb_3d_scale_1_2_3_5_8_size_21_weigths_cpd_rank_49.txt";
////const char *sep_filters_file_im ="../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/data/filter_banks/oof_fb_3d_scale_1_2_3_5_8_size_21_sep_cpd_rank_49.txt";
////auto-context features
//const char *weight_file_ac = "../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/data/filter_banks/proto_filter_AC_lap_633_822_weigths_cpd_rank_49.txt";
//const char *sep_filters_file_ac = "../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/data/filter_banks/proto_filter_AC_lap_633_822_sep_cpd_rank_49.txt";

////inv filters
////const char *weight_file_ac ="../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/data/filter_banks/oof_fb_3d_scale_1_2_3_5_8_size_21_weigths_cpd_rank_49.txt";
////const char *sep_filters_file_ac ="../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/data/filter_banks/oof_fb_3d_scale_1_2_3_5_8_size_21_sep_cpd_rank_49.txt";
//////auto-context features
////const char *weight_file_im = "../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/data/filter_banks/proto_filter_AC_lap_633_822_weigths_cpd_rank_49.txt";
////const char *sep_filters_file_im = "../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/data/filter_banks/proto_filter_AC_lap_633_822_sep_cpd_rank_49.txt";


//const char *sep_filters_file_im = "../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/data/filter_banks/oof_fb_3d_scale_1_2_3_5_size_13_sep_cpd_rank_49.txt";
//const char *weight_file_im = "../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/data/filter_banks/oof_fb_3d_scale_1_2_3_5_size_13_weigths_cpd_rank_49.txt";

//crop images train/test
//bool crop_images = false;
bool crop_images = true;
double uniform_thresh = 0.1; //to crop images
bool transpose_swc_y = false; // if true transpose Y axis when reading swc gt file
double max_pixels_predict = 0.5*1e8; //if image too large try to downsample it (otherwise might go out of memory)


bool trainTubularityImage(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output)
{

     cout<<"Reading Input Files and Parameters."<<endl;
    if (output.size() != 1) return false;
     char * regressor_output_dir = ((vector<char*> *)(output.at(0).p))->at(0);

     int created_dir = mkdir(regressor_output_dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

     if(created_dir<0){
         std::cout << "Could not create output directory (directory may already exist): " << regressor_output_dir << endl;
                    return false;

     }
     unsigned int str_length = (unsigned)strlen(regressor_output_dir);
    vector<char*> *trainImagePaths = ((vector<char*> *)(input.at(0).p)); //train images

    unsigned int n_train_images = trainImagePaths->size(); //

    if (n_train_images < 1){

        cout<<"No input train images!"<<endl;
        return false;
    }

    //first n_train_images parameters are swc files
    vector<char*> *parameters = ((vector<char*> *)(input.at(1).p));
        if(parameters->size() < n_train_images){
            cout<<"Number of input train images and swc files must be the same !"<<endl;
            return false;
        }

        //get swc filenames
        vector<char*> trainGtSwcPaths(n_train_images);
        for(unsigned int i_swc = 0; i_swc<n_train_images; i_swc++){
            trainGtSwcPaths.at(i_swc) =  parameters->at(i_swc);
        }

        ////filters parameters
        //const char *weight_file_ac = "../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/data/filter_banks/proto_filter_AC_lap_633_822_weigths_cpd_rank_49.txt";
        //const char *sep_filters_file_ac = "../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/data/filter_banks/proto_filter_AC_lap_633_822_sep_cpd_rank_49.txt";
        //const char *sep_filters_file_im = "../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/data/filter_banks/oof_fb_3d_scale_1_2_3_5_size_13_sep_cpd_rank_49.txt";
        //const char *weight_file_im = "../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/data/filter_banks/oof_fb_3d_scale_1_2_3_5_size_13_weigths_cpd_rank_49.txt";

        const char *sep_filters_file_im = parameters->at(n_train_images);
        const char *weight_file_im = parameters->at(n_train_images+1);
        const char *sep_filters_file_ac = parameters->at(n_train_images+2);
        const char *weight_file_ac = parameters->at(n_train_images+3);



        ////Auto-Context params
        unsigned int n_ac_iters = 1;
        if(parameters->size()>n_train_images+4){
            n_ac_iters = atoi(parameters->at(n_train_images+4));
        }
        unsigned int n_pos_samples_tot =50000;
        unsigned int n_neg_samples_tot =50000;
        unsigned int n_samples_tot =n_pos_samples_tot+ n_neg_samples_tot;


        if(parameters->size()>n_train_images+1+4){
            n_samples_tot= atoi(parameters->at(n_train_images+1+4));
            n_samples_tot = 2*(n_samples_tot/2);//ensure it is even
            n_pos_samples_tot = n_samples_tot/2;
            n_neg_samples_tot = n_samples_tot/2;
        }

    ////boosting parameters
     unsigned max_boost_iters = 200;
     unsigned max_depth_wl_tree = 2;
     double shrink_factor = 0.1;
    unsigned int m_try =0;
    if(parameters->size()>n_train_images+2+4){
        max_boost_iters = atoi(parameters->at(n_train_images+2+4));
    }
    if(parameters->size()>n_train_images+3+4){
        max_depth_wl_tree = atoi(parameters->at(n_train_images+3+4));
    }
    if(parameters->size()>n_train_images+4+4){
        shrink_factor = atof(parameters->at(n_train_images+4+4));
    }if(parameters->size()>n_train_images+5+4){
        m_try = atoi(parameters->at(n_train_images+5+4));
    }
    char * loss_type = "squaredloss";




    ////get filters to compute features

    MatrixTypeDouble weights = readMatrix(weight_file_im);
    MatrixTypeDouble sep_filters = readMatrix(sep_filters_file_im);
    MatrixTypeFloat sep_filters_float = sep_filters.cast<float>();
    MatrixTypeFloat weights_float = weights.cast<float>();


    MatrixTypeDouble weights_ac = readMatrix(weight_file_ac);
    MatrixTypeDouble sep_filters_ac = readMatrix(sep_filters_file_ac);
    MatrixTypeFloat sep_filters_float_ac = sep_filters_ac.cast<float>();
    MatrixTypeFloat weights_float_ac = weights_ac.cast<float>();

    unsigned int n_features_tot = weights_float.cols();//for now just number of non-sep filters (TODO: add context features)
    const float scale_factor =1.0; //no rescale filter
    unsigned int channel = 1; //only process first channel of image

    if(m_try>weights_float.cols()){
        std:cout << "m_try larger than number of features. Using all features" << std::endl;
            m_try  = weights_float.cols();
    }

    MatrixTypeFloat * features_image = new MatrixTypeFloat[n_train_images];
    VectorTypeFloat * train_gt_vector = new VectorTypeFloat[n_train_images];
    ITKImageType::SizeType * size_img = new ITKImageType::SizeType[n_train_images];

        for(unsigned int i_img =0; i_img<n_train_images; i_img++){

            cout<<"Processing Train Image "<< i_img+1<< "/"<< n_train_images << endl;

            char * train_img_file = trainImagePaths->at(i_img);
            cout<<"image file "<< train_img_file << endl;


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

            ImagePixelType pixel_type = train_img->getDatatype();

            ////convert image to itk format
            ITKImageType::Pointer train_img_ITK  =  ITKImageType::New();

            train_img_ITK =v3d2ItkImage<ITKImageType,ImageScalarTypeUint8>(train_img,train_img_size,channel);


            char * swc_gt_file = trainGtSwcPaths.at(i_img);





            //check if input is .swc file, otherwise assumes it is radial gt image and try to load it
            std::string swc_gt_file_string(swc_gt_file);
            std::string gt_extension = swc_gt_file_string.substr(swc_gt_file_string.length()-4,swc_gt_file_string.length());

            ITKImageType::Pointer train_gt_radial_ITK  =  ITKImageType::New();
            if(gt_extension.find(".swc")!=std::string::npos){
                cout<<"loading swc file "<< swc_gt_file << endl;
                //// convert swc file to distance gt image
                train_gt_radial_ITK =swc2ItkImage<ITKImageType,ImageScalarTypeFloat>(swc_gt_file,train_img_size,transpose_swc_y);


            }else{
                cout<<"Extension of ground truth is:  "<< gt_extension << endl;
                cout<<"trying to load it as radial gt.  " << endl;
                 Image4DSimple *train_radial_gt = callback.loadImage(swc_gt_file);
                 if (!train_radial_gt || !train_radial_gt->valid())
                 {
                   v3d_msg("Fail to open the radial gt image file.", 0);
                   return false;
                 }
                  train_gt_radial_ITK = v3d2ItkImage<ITKImageType,ImageScalarTypeUint8>(train_radial_gt,train_img_size,channel);


            }



            //save radial gt and image to check size and  axis
                        //radial gt
                 //       Image4DSimple *train_radial_gt = callback.loadImage("../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/regression/cropped_N2_scaled_radial_gt_uint8.tif");
                 //       callback.saveImage(train_radial_gt, "../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/temp_results/swc_gt_debug.v3draw");

                        //debug: save radial gt
                        Image4DSimple radial_img_debug = itk2v3dImage<ITKImageType>(train_gt_radial_ITK);
                        std::string name_radial_gt_temp = swc_gt_file_string + "_radial_gt.v3draw";
                        char *name_radial_gt_temp_char = new char[name_radial_gt_temp.length() + 1];
                        strcpy(name_radial_gt_temp_char, name_radial_gt_temp.c_str());
                        callback.saveImage(&radial_img_debug, name_radial_gt_temp_char);
                        delete [] name_radial_gt_temp_char;



//                        //size input image
//                        std::cout << "v3d input X: " << train_img->getXDim() << " Y: " << train_img->getYDim() << " z: " << train_img->getZDim() << std::endl;
//                     //   std::cout << "v3d input resX: " << train_img->getRezX() << " Y: " << train_img->getRezY() << " z: " << train_img->getRezZ() << std::endl;
//                       //size input itk
//                        ITKImageType::SizeType size_temp_img = train_img_ITK->GetLargestPossibleRegion().GetSize();
//                        std::cout << "itk Rad X: " << size_temp_img[0] << " Y: " << size_temp_img[1] <<  " z: " << size_temp_img[2]  << std::endl;
//                //        std::cout << "v3d input resX: " << train_img->getRezX() << " Y: " << train_img->getRezY() << "z: " << train_img->getRezZ() << std::endl;
//                        //size itk radial
//                        ITKImageType::SizeType size_temp = train_gt_radial_ITK->GetLargestPossibleRegion().GetSize();
//                        std::cout << "itk Rad X: " << size_temp[0] << " Y: " << size_temp[1] << " z: " << size_temp[2]  << std::endl;
//                        //size v3d radial
//                        std::cout << "v3d Rad X: " << radial_img_debug.getXDim() << " Y: " << radial_img_debug.getYDim() << " z: " << radial_img_debug.getZDim() << std::endl;
//                 //       std::cout << "v3d Rad resX: " << radial_img_debug.getRezX() << " Y: " << radial_img_debug.getRezY() << " z: " << radial_img_debug.getRezZ() << std::endl;



            // crop images and gt
            long int *crop_start_idxs ;
            long int *in_sz_cropped;
            ITKImageType::Pointer train_img_ITK_cropped =  ITKImageType::New();
            ITKImageType::Pointer train_gt_radial_ITK_cropped  =  ITKImageType::New();
            if(crop_images){
                cout<<"cropping image..." << endl << std::flush;

                crop_start_idxs = new long int[3];
                in_sz_cropped = new long int[4];

                train_img_ITK_cropped = cropItkImageUniformBackground<ITKImageType>(train_img_ITK,crop_start_idxs,in_sz_cropped,uniform_thresh);

                train_gt_radial_ITK_cropped = cropItkImage<ITKImageType>(train_gt_radial_ITK,  crop_start_idxs,  in_sz_cropped);

                size_img[i_img][0] =in_sz_cropped[0];
                size_img[i_img][1] =in_sz_cropped[1];
                size_img[i_img][2] =in_sz_cropped[2];
                cout<<"cropping image...Done." << endl << std::flush;

                cout<<"size image cropped: " << size_img[i_img][0]<< " ;"<<size_img[i_img][1]<<  " ;"<<size_img[i_img][2]<<endl << std::flush;


            }else{

                train_img_ITK_cropped = train_img_ITK;
                train_gt_radial_ITK_cropped = train_gt_radial_ITK;
                size_img[i_img][0] =train_img_size[0];
                size_img[i_img][1] =train_img_size[1];
                size_img[i_img][2] =train_img_size[2];
                crop_start_idxs= new long int[3]; crop_start_idxs[0] = 0;  crop_start_idxs[1] = 0;  crop_start_idxs[2] = 0;//  crop_start_idxs[3] = 0;
                in_sz_cropped = train_img_size;

            }

            V3DLONG n_pixels = size_img[i_img][0]*size_img[i_img][1]*size_img[i_img][2];


              cout << "Computing distance gt...  "<< endl << std::flush;
            // train_gt_ITK = radialGt2ExpDistGt<ITKImageType>(train_gt_radial_ITK,thresh_distance,scales,scale_toll);
             float thresh_distance = 21.0;
             ITKImageType::Pointer  train_gt_ITK_computed = binaryGt2ExpDistGt<ITKImageType>(train_gt_radial_ITK_cropped,thresh_distance);
             cout << "Computing distance gt... Done."<< endl<< std::flush;

           train_gt_vector[i_img]  = itkImage2EigenVector<ITKImageType,VectorTypeFloat>(train_gt_ITK_computed,n_pixels,n_pixels);

            ////compute features image (TODO add context features)
            cout<<"Computing Features..."<<endl<< std::flush;
            computeFeaturesSepComb<ITKImageType,MatrixTypeFloat,VectorTypeFloat>(features_image[i_img],train_img_ITK_cropped,sep_filters_float,weights_float, scale_factor);
            cout<<"Computing Features...Done!"<<endl<< std::flush;

 //   cout<<"Rows Features: "<<features_image[i_img].rows()<<" Cols Features: "<<features_image[i_img].cols() <<endl;
    //cout<< "n pixels: "<< n_pixels << endl;


        }
        cout<<"Processed all Train Images "<< endl;

  cout<<"Starting Training..."<<endl;
        //Auto-Context iterations
  MatrixTypeFloat features_image_and_ac;
  MatrixTypeFloat *features_ac = new MatrixTypeFloat[n_train_images];
    for(unsigned int i_ac=0; i_ac <= n_ac_iters; i_ac++){
        cout<<"Auto-Context Iter "<< i_ac<< "/" <<n_ac_iters<< endl;


        if(i_ac>0){
            n_features_tot = features_image[0].cols()+features_ac[0].cols();
        }

        cout << "Tot number of features available for training Regressor: " <<n_features_tot << endl;

        // get samples
        ////random sampling
        ///  //// get gt, compute features and random samples
        unsigned int n_pos_samples_per_image;
        unsigned int n_neg_samples_per_image;
        unsigned int collected_pos_samples = 0;
        unsigned int collected_neg_samples = 0;

        MatrixTypeFloat all_samples_features = MatrixTypeFloat::Zero(n_samples_tot,n_features_tot) ;
        VectorTypeFloat all_samples_gt = VectorTypeFloat::Zero(n_samples_tot) ;

        MatrixTypeFloat sampled_features_image;
        VectorTypeFloat sampled_gt_vector_image;
        ///
        cout<<"Getting Samples..."<<endl << std::flush;

        for(unsigned int i_img =0; i_img<n_train_images; i_img++){



                if(i_ac>0){

                    features_image_and_ac =  MatrixTypeFloat::Zero(features_image[i_img].rows(),n_features_tot) ;
                    features_image_and_ac.middleCols(0,features_image[0].cols()) = features_image[i_img];
                    features_image_and_ac.middleCols(features_image[0].cols(),features_ac[0].cols()) = features_ac[i_img];

                }else{
                    features_image_and_ac = features_image[i_img];
                }
           //     cout << "size feats all: " <<features_image_and_ac.rows()<<" "<<features_image_and_ac.cols() << endl;


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

                bool got_samples = getTrainSamplesFeaturesAndGt<MatrixTypeFloat,VectorTypeFloat>(features_image_and_ac,train_gt_vector[i_img],sampled_features_image, sampled_gt_vector_image,n_pos_samples_per_image,n_neg_samples_per_image);

                if(!got_samples){
                    cout<< "Problems encountered while getting samples ! " << endl;
                    return false;
                }

                all_samples_features.middleRows((collected_pos_samples+collected_neg_samples), n_pos_samples_per_image+n_neg_samples_per_image ) = sampled_features_image;//P.middleRows(i, rows) <=> P(i+1:i+rows, :)
                all_samples_gt.middleRows((collected_pos_samples+collected_neg_samples), n_pos_samples_per_image+n_neg_samples_per_image ) = sampled_gt_vector_image;


                collected_pos_samples += n_pos_samples_per_image;
                collected_neg_samples += n_neg_samples_per_image;

        }//end sampling image
         cout<<"Getting Samples...Done!"<<endl;
         cout<<"Collected " << collected_pos_samples << " positive samples and " <<collected_neg_samples << " negative samples."<< endl<< std::flush;

         //write matrix features
//        if(i_ac>0){
//cout<<"writing matrix ac1 :" << endl;
//            writeMatrix<MatrixTypeFloat>("/cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/regression/results/feats_AC1.txt",all_samples_features.cast<float>());
//            writeMatrix<MatrixTypeDouble>("/cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/regression/results/gt_AC1.txt",all_samples_gt.cast<double>());

//        }else{

//             cout<<"writing matrix :" << endl;
//            writeMatrix<MatrixTypeFloat>("/cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/regression/results/feats_AC0.txt",all_samples_features.cast<float>());
//            writeMatrix<MatrixTypeDouble>("/cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/regression/results/gt_AC0.txt",all_samples_gt.cast<double>());

//        }


    char regressor_output_file[str_length+50];
    sprintf (regressor_output_file, "%s/Regressor_ac_%i.cfg",regressor_output_dir,i_ac);

////Training
        trainRegressor(all_samples_features.cast<float>(),all_samples_gt.cast<double>(),regressor_output_file,loss_type,max_boost_iters,max_depth_wl_tree,shrink_factor,m_try);


        if(i_ac<n_ac_iters && n_ac_iters>0){

            std::cout << "Computing Features Auto-Context..." << std::endl << std::flush;
            for(unsigned int i_img =0; i_img<n_train_images; i_img++){

                cout<<"...Train Image "<< i_img+1<< "/"<< n_train_images << "..."<<endl<< std::flush;


                if(i_ac>0){

                    //features_image_and_ac << features_image[i_img], features_ac[i_img];
                    features_image_and_ac =  MatrixTypeFloat::Zero(features_image[i_img].rows(),n_features_tot) ;
                    features_image_and_ac.middleCols(0,features_image[0].cols()) = features_image[i_img];
                    features_image_and_ac.middleCols(features_image[0].cols(),features_ac[0].cols()) = features_ac[i_img];

                }else{
                    features_image_and_ac = features_image[i_img];
                }

                //predict
                TreeBoosterType::ResponseArrayType predScore;
                predictRegressor(regressor_output_file,features_image_and_ac,predScore);

                VectorTypeFloat predScoreFloat = predScore.cast<float>();
                ITKImageType::Pointer predImg = eigenVector2itkImage<ITKImageType,VectorTypeFloat>(predScoreFloat,size_img[i_img]);

                //compute features autocontext
                cout<<"Computing Features Auto-Context..."<<endl<< std::flush;
                computeFeaturesSepComb<ITKImageType,MatrixTypeFloat,VectorTypeFloat>(features_ac[i_img],predImg,sep_filters_float_ac,weights_float_ac, scale_factor);
                cout<<"Computing Features Auto-Context...Done."<<endl<< std::flush;

//                if(i_ac>0){
//        cout<<"writing matrix ac1 :" << endl;
//                    writeMatrix<MatrixTypeFloat>("/cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/regression/results/feats_AC1_all.txt",features_image_and_ac.cast<float>());
//                //    writeMatrix<MatrixTypeDouble>("/cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/regression/results/gt_AC1.txt",all_samples_gt.cast<double>());

//                }else{

//                     cout<<"writing matrix :" << endl;
//                    writeMatrix<MatrixTypeFloat>("/cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/regression/results/feats_AC0_all.txt",features_image_and_ac.cast<float>());
//                    //writeMatrix<MatrixTypeDouble>("/cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/regression/results/gt_AC0.txt",all_samples_gt.cast<double>());

//                }


            }

            std::cout << "Computing Features Auto-Context...Done." << std::endl<< std::flush;

        }

    }//end AC iters

    cout<<"Training Done!"<<endl;
    cout<<"Training Terminated Normally"<<endl;
    return true;
}




bool testTubularityImage(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output)
{


  if (output.size() != 1) return false;
  const char *regressor_filename;

  vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
        if(paras.size() < 1) return false;

  vector<char*> *inputImagePaths = ((vector<char*> *)(input.at(0).p));
  vector<char*> *outputImagePaths = ((vector<char*> *)(output.at(0).p));

  if(inputImagePaths->size() != outputImagePaths->size()) {
    std::cerr << "The number of input images is different than the number of output images." << std::endl;
    return false;
  } else {
   // do nothing
  }

  unsigned int n_ac_iters = paras.size()-1-4;
  bool multiscale_pred = false; //todo: pass as arguments
//  bool multiscale_pred = true; //todo: pass as arguments

  const char *sep_filters_file_im = (paras.at(0));//"../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/data/filter_banks/oof_fb_3d_scale_1_2_3_5_size_13_sep_cpd_rank_49.txt";
  const char *weight_file_im = (paras.at(1));//"../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/data/filter_banks/oof_fb_3d_scale_1_2_3_5_size_13_weigths_cpd_rank_49.txt";

  const char *sep_filters_file_ac = (paras.at(2));// "../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/data/filter_banks/proto_filter_AC_lap_633_822_weigths_cpd_rank_49.txt";
  const char *weight_file_ac = (paras.at(3));//"../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/data/filter_banks/proto_filter_AC_lap_633_822_sep_cpd_rank_49.txt";



//  std::cout << "sep filt img: " << sep_filters_file_im << std::endl;
//  std::cout << "wei filt img: " << weight_file_im << std::endl;
//  std::cout << "sep filt ac: " << sep_filters_file_ac << std::endl;
//  std::cout << "wei filt ac: " << weight_file_ac << std::endl;


  for(unsigned int imgInd = 0; imgInd < inputImagePaths->size(); imgInd++) {
    char * inimg_file = inputImagePaths->at(imgInd);
    char * outimg_file = outputImagePaths->at(imgInd);

    cout<<"Predicting Test Image "<< imgInd+1<< "/"<< inputImagePaths->size() << "..."<<endl;
    cout<<"input file: "<<inimg_file<<endl;
    cout<<"output file: "<<outimg_file<<endl;


    Image4DSimple *inimg = callback.loadImage(inimg_file);
    if (!inimg || !inimg->valid())
    {
      v3d_msg("Fail to open the image file.", 0);
      return false;
    }

    //input
    float* outimg = 0; //no need to delete it later as the Image4DSimple variable "outimg" will do the job

    V3DLONG in_sz[4];
    in_sz[0] = inimg->getXDim();
    in_sz[1] = inimg->getYDim();
    in_sz[2] = inimg->getZDim();
    in_sz[3] = inimg->getCDim();
    ITKImageType::SizeType  size_img;

    size_img[0] = in_sz[0]; size_img[1] = in_sz[1]; size_img[2] = in_sz[2];

    ImagePixelType pixel_type = inimg->getDatatype();
    V3DLONG n_pixels = in_sz[0]*in_sz[1]*in_sz[2];

    unsigned int c = 1;
    unsigned int unit_bites = inimg->getUnitBytes();


    ////convert image to itk format
    ITKImageType::Pointer I  =  ITKImageType::New();
    I =v3d2ItkImage<ITKImageType,ImageScalarTypeUint8>(inimg,in_sz,c);


    //bool crop_images = false;
   // bool crop_images = true;
    ITKImageType::Pointer I_cropped ;
    long int *crop_start_idxs ;
    long int *in_sz_cropped;
    V3DLONG n_pixels_cropped;
    Image4DSimple *inimg_cropped;
    if(crop_images){

        cout << "Cropping image... " <<endl << std::flush;

        //I_cropped =  ITKImageType::New();
        crop_start_idxs = new long int[3];
        in_sz_cropped = new long int[4];
        I_cropped = cropItkImageUniformBackground<ITKImageType>(I,crop_start_idxs,in_sz_cropped,uniform_thresh);
         n_pixels_cropped = in_sz_cropped[0]*in_sz_cropped[1]*in_sz_cropped[2];
         inimg_cropped = 0;
         cout << "Cropping image...Done. " <<endl << std::flush;


         cout<<"size image cropped: " << in_sz_cropped[0]<< " ;"<<in_sz_cropped[1]<<  " ;"<<in_sz_cropped[2]<<endl << std::flush;


         //TODO: what if image cropped is empty?

         if(in_sz_cropped[0]*in_sz_cropped[1]*in_sz_cropped[2]<1){

             cout << "Cropped image is empty! " <<endl;

             Image4DSimple outimg_final_emty;
             unsigned char* out_data_zero = new unsigned char[n_pixels];
             for(unsigned int i_pix = 0; i_pix < n_pixels; i_pix++){
                      out_data_zero[i_pix] = 0;
             }

             outimg_final_emty.setData((unsigned char *)(out_data_zero), in_sz[0], in_sz[1], in_sz[2], 1, V3D_UINT8);
             callback.saveImage(&outimg_final_emty, outimg_file);
             cout << "Saved final result to: " << outimg_file<<endl;
             return true;

         }


       //  std::cout << "image origin" << I_cropped->GetOrigin() <<std::endl;

  //       Image4DSimple input_cropped_image = itk2v3dImage<ITKImageType>(I_cropped);
  //       callback.saveImage(&input_cropped_image, "../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/trained_models/OPF_debug/large_image_cropped.v3draw");


    }else{
        I_cropped = I;
        n_pixels_cropped = n_pixels;
        crop_start_idxs= new long int[3]; crop_start_idxs[0] = 0;  crop_start_idxs[1] = 0;  crop_start_idxs[2] = 0;//  crop_start_idxs[3] = 0;
        in_sz_cropped = in_sz;
        inimg_cropped = inimg;
    }

  //  return true;

    ////load filters
    MatrixTypeDouble weights = readMatrix(weight_file_im);
    MatrixTypeDouble sep_filters = readMatrix(sep_filters_file_im);


    MatrixTypeDouble weights_ac = readMatrix(weight_file_ac); //weights_ac = weights_ac/255.0;
    MatrixTypeDouble sep_filters_ac = readMatrix(sep_filters_file_ac);
    MatrixTypeFloat sep_filters_float_ac = sep_filters_ac.cast<float>();
    MatrixTypeFloat weights_float_ac = weights_ac.cast<float>();



    unsigned int n_img_features = weights.cols();
    unsigned int n_ac_features = weights_ac.cols();

    const float scale_factor =1.0;
    MatrixTypeFloat sep_filters_float = sep_filters.cast<float>();
    MatrixTypeFloat weights_float = weights.cast<float>();

    TreeBoosterType::ResponseArrayType finalScores = TreeBoosterType::ResponseArrayType::Zero(n_pixels_cropped);

    ITKImageType::Pointer pred_img_itk;
    ITKImageType::Pointer predImg_scaled_original_size ;
     Image4DSimple outimg1;


     ITKImageType::Pointer predImg;

    TreeBoosterType::ResponseArrayType newScores_original_size;
    TreeBoosterType::ResponseArrayType newScores;

    VectorTypeFloat scales;
    ITKImageType::Pointer I_resized =  ITKImageType::New();
    if(multiscale_pred){
         scales = VectorTypeFloat::Zero(3);
        scales << 0.5 ,1, 2 ;
        if(crop_images){
            std::cout << "cropping image with multiscale prediction not yet implemented ! " <<std::endl;
            return false;
        }

    }else{
         scales = VectorTypeFloat::Ones(1);
    }


    double *dfactor_down = new double[3];
    unsigned int n_scales = scales.size();
    for(unsigned int i_scale=0; i_scale<n_scales; i_scale++){

        std::cout << "Predicting scale:" << i_scale+1 <<"/"<< n_scales<<std::endl;

        V3DLONG *out_sz_res= new V3DLONG[4];

        if(scales(i_scale) ==1){
           // out_sz_res[0] = in_sz[0];  out_sz_res[1] = in_sz[1];  out_sz_res[2] = in_sz[2]; out_sz_res[3] = 1;
            out_sz_res[0] = in_sz_cropped[0];  out_sz_res[1] = in_sz_cropped[1];  out_sz_res[2] = in_sz_cropped[2]; out_sz_res[3] = 1;
            //I_resized = I;
            I_resized = I_cropped;
        }else{


            double *dfactor = new double[3];
            dfactor[0] = scales(i_scale); dfactor[1] = scales(i_scale);  dfactor[2] =scales(i_scale);
            //I_resized = resize_image_v3d<ITKImageType,unsigned char>(inimg,dfactor,out_sz_res);
            I_resized = resize_image_v3d<ITKImageType,unsigned char>(inimg_cropped,dfactor,out_sz_res);


        }

        ITKImageType::SizeType  size_img_scaled;
        size_img_scaled = I_resized->GetLargestPossibleRegion().GetSize();
        V3DLONG n_pixels_scaleed = size_img_scaled[0]*size_img_scaled[1]*size_img_scaled[2];

        std::cout << "size img cropped: " << size_img_scaled << std::endl << std::flush;


        bool downsampled = false;
        if(n_pixels_scaleed>max_pixels_predict){

            downsampled = true;

            Image4DSimple img_cropped_v3d = itk2v3dImage<ITKImageType>(I_resized);

            V3DLONG *out_sz_down= new V3DLONG[4];

         //   double dfactor =  ceil(std::pow(((double)(n_pixels_scaleed)/(max_pixels_predict)),1/3));

            double dfactor = 2.0;
            std::cout << "Image too large! trying to dowsample it. Factor: " << dfactor<< std::endl << std::flush;


            while(n_pixels_scaleed/(dfactor*dfactor*dfactor) > max_pixels_predict){

                dfactor = dfactor*2.0;
                std::cout << "Image too large! trying to dowsample it. Factor: " << dfactor<< std::endl << std::flush;

            }

//            if(dfactor<2.0){
//                dfactor = 2.0;

//            }



            dfactor_down[0] =dfactor; dfactor_down[1] = dfactor;  dfactor_down[2] =dfactor;

            I_resized = downsample_image_v3d<ITKImageType,float>(&img_cropped_v3d,dfactor_down,out_sz_down);

            size_img_scaled[0]=out_sz_down[0];
            size_img_scaled[1]=out_sz_down[1];
            size_img_scaled[2]=out_sz_down[2];
            n_pixels_scaleed = size_img_scaled[0]*size_img_scaled[1]*size_img_scaled[2];

            std::cout << "size img downsampled: " << size_img_scaled << std::endl << std::flush;


////debug
//        Image4DSimple img_down_v3d_debug = itk2v3dImage<ITKImageType>(I_resized);
//            char outimg_file_debug [500];

//                sprintf (outimg_file_debug, "../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/temp_results/image_downsampled.v3draw");

//                cout << "saving debug results: " << outimg_file_debug<<endl;
//              callback.saveImage(&img_down_v3d_debug, outimg_file_debug);

////
        }

        //// DEBUG
/*
        V3DLONG *out_sz_res_debug= new V3DLONG[4];
        double *dfactor_debig = new double[3];
        dfactor_debig[0] = 2.0; dfactor_debig[1] = 2.0;  dfactor_debig[2] =2.0;
        ITKImageType::Pointer I_down_debug = downsample_image_v3d<ITKImageType,unsigned char>(inimg,dfactor_debig,out_sz_res_debug);

        std::cout << "size img down: " << out_sz_res_debug << std::endl << std::flush;

        Image4DSimple img_down_v3d_debug = itk2v3dImage<ITKImageType>(I_down_debug);

        char outimg_file_debug [500];

            sprintf (outimg_file_debug, "../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/temp_results/image_downsampled.v3draw");

            cout << "saving debug results: " << outimg_file_debug<<endl;
          callback.saveImage(&img_down_v3d_debug, outimg_file_debug);

          //upsample back

          V3DLONG *out_sz_res_debug_up= new V3DLONG[4];
          double *dfactor_debig_up = new double[3];
          dfactor_debig_up[0] = dfactor_debig[0]; dfactor_debig_up[1] = dfactor_debig[0];  dfactor_debig_up[2] =dfactor_debig[0];
           ITKImageType::Pointer I_up_debug = upsample_image_v3d<ITKImageType,float>(&img_down_v3d_debug,dfactor_debig_up,out_sz_res_debug_up);


           Image4DSimple img_down_v3d_debug_up = itk2v3dImage<ITKImageType>(I_up_debug);

           char outimg_file_debug_up [500];

               sprintf (outimg_file_debug_up, "../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/temp_results/image_upsampled.v3draw");

               cout << "saving debug results: " << outimg_file_debug_up<<endl;
             callback.saveImage(&img_down_v3d_debug_up, outimg_file_debug_up);

        ////

*/

        MatrixTypeFloat nonsep_features_all;
   std::cout << "Computing features..."<<std::endl<< std::flush;
    computeFeaturesSepComb<ITKImageType,MatrixTypeFloat,VectorTypeFloat>(nonsep_features_all,I_resized,sep_filters_float,weights_float, scale_factor);
std::cout << "Computing features...Done."<<std::endl<< std::flush;


//VectorTypeFloat temp_feat = nonsep_features_all.col(0);
//Image4DSimple temp_feat_cropped;
//unsigned char* temp_feat_cropped_vec = new unsigned char[n_pixels_cropped];
//for(unsigned int i_pix = 0; i_pix < n_pixels_cropped; i_pix++){
//             temp_feat_cropped_vec[i_pix] = (float)temp_feat(i_pix);
//    }
//temp_feat_cropped.setData((unsigned char *)(temp_feat_cropped_vec), in_sz_cropped[0], in_sz_cropped[1], in_sz_cropped[2], 1, V3D_FLOAT32);
// callback.saveImage(&temp_feat_cropped, "../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/trained_models/BF_debug2/temp_feat_cropped.v3draw");



      for(unsigned int i_ac=0; i_ac<=n_ac_iters; i_ac++){

           cout<<"Auto-Context Iter "<< i_ac<< "/" <<n_ac_iters<< endl;
          regressor_filename = (paras.at(i_ac+4));

          //MatrixTypeFloat features_image_and_ac;
//          if(i_ac>0){
//              features_image_and_ac =  MatrixTypeFloat::Zero(nonsep_features_all.rows(),nonsep_features_all.cols()+features_ac.cols()) ;
//              features_image_and_ac.middleCols(0,nonsep_features_all.cols()) = nonsep_features_all;
//              features_image_and_ac.middleCols(nonsep_features_all.cols(),features_ac.cols()) = features_ac;

//          }else{
//              features_image_and_ac = nonsep_features_all;

//          }

          cout<<"applying regressor:  "<<  regressor_filename<< endl<< std::flush;
           predictRegressor(regressor_filename,nonsep_features_all,newScores);
            predImg = eigenVector2itkImage<ITKImageType,VectorTypeFloat>(newScores.cast<float>(),size_img_scaled);

            if(i_ac<n_ac_iters && n_ac_iters>0){


                if(i_ac<1){
                    nonsep_features_all.conservativeResize(nonsep_features_all.rows(), n_img_features+ n_ac_features);
                }


                //compute features auto-context
                std::cout << "Computing features Auto-context..."<<std::endl<< std::flush;
                MatrixTypeFloat temp_features_ac;
                computeFeaturesSepComb<ITKImageType,MatrixTypeFloat,VectorTypeFloat>(temp_features_ac,predImg,sep_filters_float_ac,weights_float_ac, scale_factor);
                nonsep_features_all.middleCols(n_img_features,n_ac_features) = temp_features_ac;
                temp_features_ac.resize(0,0);
                std::cout << "Computing features Auto-context...Done."<<std::endl<< std::flush;

            }//end compute features auto-context


            if(i_ac==n_ac_iters){

                if(scales(i_scale) ==1 && !downsampled){
                    newScores_original_size = newScores;
                }else if(downsampled){

                    std::cout << "Upsampling score back." <<std::endl;


                        ITKImageType::SizeType size_down_crop;
                        size_down_crop[0] = size_img_scaled[0]; size_down_crop[1] = size_img_scaled[1]; size_down_crop[2] = size_img_scaled[2];
                       pred_img_itk =  eigenVector2itkImage<ITKImageType,TreeBoosterType::ResponseArrayType>(newScores, size_down_crop);
                       Image4DSimple pred_img_v3d =  itk2v3dImage<ITKImageType>(pred_img_itk);


                       V3DLONG *out_sz_res_up= new V3DLONG[3];
                        out_sz_res_up[0] = in_sz_cropped[0];
                        out_sz_res_up[1] = in_sz_cropped[1];
                        out_sz_res_up[2] = in_sz_cropped[2];
//                       double *dfactor_up = new double[3];
//                       dfactor_up[0] = dfactor_down[0]; dfactor_up[1] = dfactor_down[0];  dfactor_up[2] =dfactor_down[0];



                     //  predImg_scaled_original_size = upsample_image_v3d<ITKImageType,float>(&pred_img_v3d,dfactor_up,out_sz_res_up);

                      predImg_scaled_original_size = upsample_image_v3d_size<ITKImageType,float>(&pred_img_v3d,out_sz_res_up);

                      newScores_original_size = itkImage2EigenVector<ITKImageType,TreeBoosterType::ResponseArrayType>(predImg_scaled_original_size,n_pixels_cropped,n_pixels_cropped);
                       std::cout << "Resampling output Done." <<std::endl;

                        std::cout << "size resampled image: " <<out_sz_res_up[0] <<out_sz_res_up[1] <<out_sz_res_up[2]<<std::endl;



                        ////debug

//                        Image4DSimple img_down_v3d_up = itk2v3dImage<ITKImageType>(predImg_scaled_original_size);
//                            char outimg_file_debug_up [500];

//                                sprintf (outimg_file_debug_up, "../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/temp_results/image_upsampled.v3draw");

//                                cout << "saving debug results: " << outimg_file_debug_up<<endl;
//                              callback.saveImage(&img_down_v3d_up, outimg_file_debug_up);

//                              char outimg_file_debug_up_before [500];

//                                  sprintf (outimg_file_debug_up_before, "../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/temp_results/image_before_upsampled.v3draw");

//                                  cout << "saving debug results: " << outimg_file_debug_up_before<<endl;
//                                callback.saveImage(&pred_img_v3d, outimg_file_debug_up_before);

                        ////


                }else{

                    std::cout << "Resampling output to orignal size" <<std::endl;

                    V3DLONG *out_sz_back = new V3DLONG[4];
                double *scale_factor_inv = new double[3];
//                 scale_factor_inv[0] = ((double)in_sz[0])/((double)out_sz_res[0]);
//                 scale_factor_inv[1] = ((double)in_sz[1])/((double)out_sz_res[1]);
//                 scale_factor_inv[2] = ((double)in_sz[2])/((double)out_sz_res[2]);
                 scale_factor_inv[0] = ((double)in_sz_cropped[0])/((double)out_sz_res[0]);
                 scale_factor_inv[1] = ((double)in_sz_cropped[1])/((double)out_sz_res[1]);
                 scale_factor_inv[2] = ((double)in_sz_cropped[2])/((double)out_sz_res[2]);


                    ITKImageType::SizeType size_res; size_res[0] = out_sz_res[0]; size_res[1] = out_sz_res[1]; size_res[2] = out_sz_res[2];
                   pred_img_itk =  eigenVector2itkImage<ITKImageType,TreeBoosterType::ResponseArrayType>(newScores, size_res);
                   Image4DSimple pred_img_v3d =  itk2v3dImage<ITKImageType>(pred_img_itk);

                    predImg_scaled_original_size = resize_image_v3d<ITKImageType,float>(&pred_img_v3d,scale_factor_inv,out_sz_back);

                   newScores_original_size = itkImage2EigenVector<ITKImageType,TreeBoosterType::ResponseArrayType>(predImg_scaled_original_size,n_pixels_cropped,n_pixels_cropped);
                    std::cout << "Resampling output Done." <<std::endl;
                }

               finalScores +=newScores_original_size;


            }

                ////save image
                cout<<"saving image"<<endl;

                //copy data to save with v3d format
               // V3DLONG n_pixels = in_sz[0]*in_sz[1]*in_sz[2];
               float* out_data_copy = new float[n_pixels_scaleed];
             for(unsigned int i_pix = 0; i_pix < n_pixels_scaleed; i_pix++){
                    out_data_copy[i_pix] = (float)newScores(i_pix);
               }


         outimg1.setData((unsigned char *)(out_data_copy), size_img_scaled[0], size_img_scaled[1], size_img_scaled[2], 1, V3D_FLOAT32);



                char outimg_file_ac [500];

                    sprintf (outimg_file_ac, "../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/temp_results/score_img_%i_ac_%i_scale_%i.v3draw",imgInd,i_ac,i_scale);

                    cout << "saving temp results: " << outimg_file_ac<<endl;
                  callback.saveImage(&outimg1, outimg_file_ac);


  }// end auto-context for loop




  }//end scale for loop

    finalScores /= n_scales;





  //  std::cout << "max score: "<< finalScores.maxCoeff() << "min score: "<< finalScores.minCoeff()    << endl;

    cout << "saving final result to: " << outimg_file<<endl;
    Image4DSimple outimg_final;

    //save double
//    float* out_data_copy_final = new float[n_pixels];
//    for(unsigned int i_pix = 0; i_pix < n_pixels; i_pix++){
//             out_data_copy_final[i_pix] = (float)finalScores(i_pix);
//    }

//     outimg_final.setData((unsigned char *)(out_data_copy_final), in_sz[0], in_sz[1], in_sz[2], 1, V3D_FLOAT32);
//     callback.saveImage(&outimg_final, outimg_file);



    //save uint8
    //set values smaller than 0 to 0
    for(unsigned int i_pix = 0; i_pix < n_pixels_cropped; i_pix++){
        if(finalScores(i_pix)<0.0){
                finalScores(i_pix) = 0.0;
        }
    }
    //normalize in [0,255]
    finalScores = 255.0*(finalScores - finalScores.minCoeff())/( finalScores.maxCoeff() - finalScores.minCoeff() );
    unsigned char* out_data_copy_final = new unsigned char[n_pixels];



//    Image4DSimple outimg_final_cropped;
//    unsigned char* out_data_copy_final_cropped = new unsigned char[n_pixels_cropped];
//    for(unsigned int i_pix = 0; i_pix < n_pixels_cropped; i_pix++){
//                 out_data_copy_final_cropped[i_pix] = (unsigned char)finalScores(i_pix);
//        }
//    outimg_final_cropped.setData((unsigned char *)(out_data_copy_final_cropped), in_sz_cropped[0], in_sz_cropped[1], in_sz_cropped[2], 1, V3D_UINT8);

//     callback.saveImage(&outimg_final_cropped, "../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/trained_models/BF_debug2/cropped_N2_uint8_tubularity_cropped_cropped.v3draw");


//    for(unsigned int i_pix = 0; i_pix < n_pixels; i_pix++){
//             out_data_copy_final[i_pix] = (unsigned char)finalScores(i_pix);
//    }

   //  unsigned int r,c,s,h,w,d;
   //  r = crop_start_idxs[0]; c = crop_start_idxs[1]; s = crop_start_idxs[2]; // start indeces to crop
   //   h = in_sz_cropped[0]; w = in_sz_cropped[1]; d = in_sz_cropped[2]; // size region to crop
    unsigned int i_pix =0;
    for(int iz = 0; iz < in_sz[2]; iz++)
    {
      int offsetk = iz*in_sz[1]*in_sz[0];
      for(int iy = 0; iy < in_sz[1]; iy++)
      {
        int offsetj = iy*in_sz[0];
        for(int ix = 0; ix < in_sz[0]; ix++)
        {

            //check if index is in cropped image, otherwise set tubularity to 0
            bool is_in_bb = (ix >=crop_start_idxs[0] && ix<crop_start_idxs[0]+in_sz_cropped[0])
                        && (iy >=crop_start_idxs[1] && iy<crop_start_idxs[1]+in_sz_cropped[1])
                        && (iz >=crop_start_idxs[2] && iz<crop_start_idxs[2]+in_sz_cropped[2]);
            if(is_in_bb){
                 out_data_copy_final[offsetk + offsetj + ix] = (unsigned char)finalScores(i_pix);
                 i_pix++;
            }else{
                out_data_copy_final[offsetk + offsetj + ix] = 0;
            }

          //T PixelVaule =  data1d[offsetk + offsetj + ix];
          //itk::Index<3> indexX;
          //indexX[0] = ix;
          //indexX[1] = iy;
          //indexX[2] = iz;
          //I->SetPixel(indexX, PixelVaule);
        }
      }

    }


    outimg_final.setData((unsigned char *)(out_data_copy_final), in_sz[0], in_sz[1], in_sz[2], 1, V3D_UINT8);

     callback.saveImage(&outimg_final, outimg_file);

 cout << "Saved final result to: " << outimg_file<<endl;

  }// end image for loop

   cout<<"Testing Terminated Normally"<<endl;
  return true;


}// end test function

