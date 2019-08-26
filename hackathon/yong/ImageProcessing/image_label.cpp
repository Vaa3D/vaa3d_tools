#include "image_label.h"
#include <iostream>
#include <fstream>
#include <string>

//using namespace std;
//using namespace Eigen;

//#define MAXBUFSIZE  ((int) 1e6)
//typedef Eigen::VectorXf VectorTypeFloat;
//typedef Eigen::VectorXd VectorTypeDouble;
//typedef Eigen::MatrixXd MatrixTypeDouble;
//typedef Eigen::MatrixXf MatrixTypeFloat;

//////functions declarations
/////
//// read text file and store it in Eigen matrix
//MatrixTypeDouble readMatrix(const char *filename);


bool image_label(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA)
{
    return true;
}

bool crop_images = true;
double uniform_thresh = 0.1; //to crop images
bool transpose_swc_y = false; // if true transpose Y axis when reading swc gt file
double max_pixels_predict = 0.5*1e8; //if image too large try to downsample it (otherwise might go out of memory)

bool image_label(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent,input_PARA &PARA)
{
//    vector<char*> infiles, inparas, outfiles;
//    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
//    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
//    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

//    QString fileOpenName = infiles[0];
//    NeuronTree nt = readSWC_file(fileOpenName);
//    cout<<"SwcName"<<fileOpenName.toStdString()<<endl;
//    cout<<"nt.listNeuron = "<<nt.listNeuron.size()<<endl;

//    QString inimage = infiles[1];
//    cout<<"inimageName :"<<inimage.toStdString()<<endl;

//    unsigned char * data1d = 0;
//    V3DLONG in_sz[4];
//    int dataType;
//    if(!simple_loadimage_wrapper(callback, inimage.toStdString().c_str(), data1d, in_sz, dataType))
//    {
//        cerr<<"load image "<<inimage.toStdString()<<" error!"<<endl;
//        return false;
//    }

//    Image4DSimple* p4DImage = new Image4DSimple;
//    p4DImage->setData((unsigned char*)data1d, in_sz[0], in_sz[1], in_sz[2], 1, V3D_UINT8);


//    cout<<"Reading Input Files and Parameters."<<endl;
//   if (output.size() != 1) return false;
//    char * regressor_output_dir = ((vector<char*> *)(output.at(0).p))->at(0);

//    int created_dir = mkdir(regressor_output_dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

//    if(created_dir<0){
//        std::cout << "Could not create output directory (directory may already exist): " << regressor_output_dir << endl;
//                   return false;

//    }
//    unsigned int str_length = (unsigned)strlen(regressor_output_dir);
//   vector<char*> *trainImagePaths = ((vector<char*> *)(input.at(0).p)); //train images

//   unsigned int n_train_images = trainImagePaths->size(); //

//   if (n_train_images < 1){

//       cout<<"No input train images!"<<endl;
//       return false;
//   }

//   //first n_train_images parameters are swc files
//   vector<char*> *parameters = ((vector<char*> *)(input.at(1).p));
//       if(parameters->size() < n_train_images){
//           cout<<"Number of input train images and swc files must be the same !"<<endl;
//           return false;
//       }

//       //get swc filenames
//       vector<char*> trainGtSwcPaths(n_train_images);
//       for(unsigned int i_swc = 0; i_swc<n_train_images; i_swc++){
//           trainGtSwcPaths.at(i_swc) =  parameters->at(i_swc);
//       }

//       ////filters parameters
//       //const char *weight_file_ac = "../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/data/filter_banks/proto_filter_AC_lap_633_822_weigths_cpd_rank_49.txt";
//       //const char *sep_filters_file_ac = "../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/data/filter_banks/proto_filter_AC_lap_633_822_sep_cpd_rank_49.txt";
//       //const char *sep_filters_file_im = "../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/data/filter_banks/oof_fb_3d_scale_1_2_3_5_size_13_sep_cpd_rank_49.txt";
//       //const char *weight_file_im = "../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/data/filter_banks/oof_fb_3d_scale_1_2_3_5_size_13_weigths_cpd_rank_49.txt";

//       const char *sep_filters_file_im = parameters->at(n_train_images);
//       const char *weight_file_im = parameters->at(n_train_images+1);
//       const char *sep_filters_file_ac = parameters->at(n_train_images+2);
//       const char *weight_file_ac = parameters->at(n_train_images+3);



//       ////Auto-Context params
//       unsigned int n_ac_iters = 1;
//       if(parameters->size()>n_train_images+4){
//           n_ac_iters = atoi(parameters->at(n_train_images+4));
//       }
//       unsigned int n_pos_samples_tot =50000;
//       unsigned int n_neg_samples_tot =50000;
//       unsigned int n_samples_tot =n_pos_samples_tot+ n_neg_samples_tot;


//       if(parameters->size()>n_train_images+1+4){
//           n_samples_tot= atoi(parameters->at(n_train_images+1+4));
//           n_samples_tot = 2*(n_samples_tot/2);//ensure it is even
//           n_pos_samples_tot = n_samples_tot/2;
//           n_neg_samples_tot = n_samples_tot/2;
//       }

//   ////boosting parameters
//    unsigned max_boost_iters = 200;
//    unsigned max_depth_wl_tree = 2;
//    double shrink_factor = 0.1;
//   unsigned int m_try =0;
//   if(parameters->size()>n_train_images+2+4){
//       max_boost_iters = atoi(parameters->at(n_train_images+2+4));
//   }
//   if(parameters->size()>n_train_images+3+4){
//       max_depth_wl_tree = atoi(parameters->at(n_train_images+3+4));
//   }
//   if(parameters->size()>n_train_images+4+4){
//       shrink_factor = atof(parameters->at(n_train_images+4+4));
//   }if(parameters->size()>n_train_images+5+4){
//       m_try = atoi(parameters->at(n_train_images+5+4));
//   }
//   char * loss_type = "squaredloss";

//   ////get filters to compute features

//   MatrixTypeDouble weights = readMatrix(weight_file_im);
//   MatrixTypeDouble sep_filters = readMatrix(sep_filters_file_im);
//   MatrixTypeFloat sep_filters_float = sep_filters.cast<float>();
//   MatrixTypeFloat weights_float = weights.cast<float>();


//   MatrixTypeDouble weights_ac = readMatrix(weight_file_ac);
//   MatrixTypeDouble sep_filters_ac = readMatrix(sep_filters_file_ac);
//   MatrixTypeFloat sep_filters_float_ac = sep_filters_ac.cast<float>();
//   MatrixTypeFloat weights_float_ac = weights_ac.cast<float>();

//   unsigned int n_features_tot = weights_float.cols();//for now just number of non-sep filters (TODO: add context features)
//   const float scale_factor =1.0; //no rescale filter
//   unsigned int channel = 1; //only process first channel of image

//   if(m_try>weights_float.cols()){
//       std:cout << "m_try larger than number of features. Using all features" << std::endl;
//           m_try  = weights_float.cols();
//   }

//   MatrixTypeFloat * features_image = new MatrixTypeFloat[n_train_images];
//   VectorTypeFloat * train_gt_vector = new VectorTypeFloat[n_train_images];
//   ITKImageType::SizeType * size_img = new ITKImageType::SizeType[n_train_images];

//       for(unsigned int i_img =0; i_img<n_train_images; i_img++){

//           cout<<"Processing Train Image "<< i_img+1<< "/"<< n_train_images << endl;

//           char * train_img_file = trainImagePaths->at(i_img);
//           cout<<"image file "<< train_img_file << endl;


//           Image4DSimple *train_img = callback.loadImage(train_img_file);
//           if (!train_img || !train_img->valid())
//           {
//             v3d_msg("Fail to open the image file.", 0);
//             return false;
//           }

//           V3DLONG train_img_size[4];
//           train_img_size[0] = train_img->getXDim();
//           train_img_size[1] = train_img->getYDim();
//           train_img_size[2] = train_img->getZDim();
//           train_img_size[3] = train_img->getCDim();

//           ImagePixelType pixel_type = train_img->getDatatype();

//           ////convert image to itk format
//           ITKImageType::Pointer train_img_ITK  =  ITKImageType::New();

//           train_img_ITK =v3d2ItkImage<ITKImageType,ImageScalarTypeUint8>(train_img,train_img_size,channel);


//           char * swc_gt_file = trainGtSwcPaths.at(i_img);





//           //check if input is .swc file, otherwise assumes it is radial gt image and try to load it
//           std::string swc_gt_file_string(swc_gt_file);
//           std::string gt_extension = swc_gt_file_string.substr(swc_gt_file_string.length()-4,swc_gt_file_string.length());

//           ITKImageType::Pointer train_gt_radial_ITK  =  ITKImageType::New();
//           if(gt_extension.find(".swc")!=std::string::npos){
//               cout<<"loading swc file "<< swc_gt_file << endl;
//               //// convert swc file to distance gt image
//               train_gt_radial_ITK =swc2ItkImage<ITKImageType,ImageScalarTypeFloat>(swc_gt_file,train_img_size,transpose_swc_y);


//           }else{
//               cout<<"Extension of ground truth is:  "<< gt_extension << endl;
//               cout<<"trying to load it as radial gt.  " << endl;
//                Image4DSimple *train_radial_gt = callback.loadImage(swc_gt_file);
//                if (!train_radial_gt || !train_radial_gt->valid())
//                {
//                  v3d_msg("Fail to open the radial gt image file.", 0);
//                  return false;
//                }
//                 train_gt_radial_ITK = v3d2ItkImage<ITKImageType,ImageScalarTypeUint8>(train_radial_gt,train_img_size,channel);


//           }



//           //save radial gt and image to check size and  axis
//                       //radial gt
//                //       Image4DSimple *train_radial_gt = callback.loadImage("../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/regression/cropped_N2_scaled_radial_gt_uint8.tif");
//                //       callback.saveImage(train_radial_gt, "../../vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/temp_results/swc_gt_debug.v3draw");

//                       //debug: save radial gt
//                       Image4DSimple radial_img_debug = itk2v3dImage<ITKImageType>(train_gt_radial_ITK);
//                       std::string name_radial_gt_temp = swc_gt_file_string + "_radial_gt.v3draw";
//                       char *name_radial_gt_temp_char = new char[name_radial_gt_temp.length() + 1];
//                       strcpy(name_radial_gt_temp_char, name_radial_gt_temp.c_str());
//                       callback.saveImage(&radial_img_debug, name_radial_gt_temp_char);
//                       delete [] name_radial_gt_temp_char;





}
