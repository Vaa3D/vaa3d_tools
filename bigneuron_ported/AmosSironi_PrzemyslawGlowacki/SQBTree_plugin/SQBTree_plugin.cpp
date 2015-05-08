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
          if(paras.size() >= 1) classifier_filename = atoi(paras.at(0));
       //   if(paras.size() >= 2) Wy = atoi(paras.at(1));
       //   if(paras.size() >= 3) Wz = atoi(paras.at(2));
       //   if(paras.size() >= 4) c = atoi(paras.at(3));
       //   if(paras.size() >= 5) sigma = atof(paras.at(4));
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



