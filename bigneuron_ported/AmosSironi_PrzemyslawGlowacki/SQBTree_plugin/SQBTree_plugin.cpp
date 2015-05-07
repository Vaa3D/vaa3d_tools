/* SQBTree_plugin.cpp
 * a plugin for test
 * 2015-5-5 : by HP
 */
 
#include "v3d_message.h"
#include <vector>
#include "SQBTree_plugin.h"

#include "sqb_0.1/src/MatrixSQB/vaa3d_link.h"

////Amos
#include "sep_conv.h"
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

        /*
        ////Amos
        if (func_name == tr("test")) // apply already trained classifier to an image and save results
	{

            //read input image file
                Matrix3D<float> in_matrix;
                //bool load( const std::string &fName )
                bool loaded_image = in_matrix.load(infiles);
                if(!loaded_image){
                    //error
                    v3d_msg('ERROR: image %s not found\n',infiles);
                    return 0;
                }

            //read sep filters and weights
                 const char *sep_filters_file = inparas[0];
                 const char *weight_file = inparas[1];

                 MatrixXd weights = readMatrix(weight_file);
                 MatrixXd sep_filters = readMatrix(sep_filters_file);

            //convolve image with separable filters
                MatrixXf all_features =
*/
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("func2"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}



