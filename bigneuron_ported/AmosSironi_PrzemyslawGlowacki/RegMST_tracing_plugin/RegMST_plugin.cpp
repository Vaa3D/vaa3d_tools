/* RegMST_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2015-6-23 : by Amos Sironi and Przemyslaw Glowacki
 */
 
#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"

#include "RegMST_plugin.h"


#include <math.h>

#include "../libs/sqb_0.1/src/MatrixSQB/vaa3d_link.h"

#include "../libs/regression/sep_conv.h"
#include "../libs/regression/sampling.h"
#include "../libs/regression/regressor.h"

#include <sys/stat.h>
#include <sys/types.h>


//#include "../../../vaa3d_tools/hackathon/zhi/neurontracing_mst/neurontracing_mst_plugin.h"
//#include "../../../vaa3d_tools/hackathon/zhi/neurontracing_mst/neurontracing_mst_plugin.cpp"
#include "../SQBTree_plugin/RegressionTubularityAC_plugin.h"
#include "mst_functions/neurontracing_mst_plugin.h"

//#include "basic_surf_objs.h"
//#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/v3dneuron_gd_tracing.h"
//#include "../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h""
//#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"
//#include "stackutil.h"
//#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/gd.h"


#ifndef __EXPORT_PLUGIN_H__
#define __EXPORT_PLUGIN_H__
Q_EXPORT_PLUGIN2(RegMST, RegMST);
#endif

using namespace std;

//parameters used by tubularity plugin
struct input_PARA
{
    QString inimg_file;
    V3DLONG channel;
    V3DLONG n_ac_iters;
    QList<QString> regressor_paths;
    V3DLONG Ws;
};

//parameters used by tracing plugin
//#ifndef __MST_PARA__
//#define __MST_PARA__
//struct MST_PARA
//{
//    QString inimg_file;
//    V3DLONG channel;
//    V3DLONG Ws;
//};
//#endif

void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);
 

//void autotrace_mst(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);
//template <class T> QList<NeuronSWC> seed_detection(T* data1d,
//                                      V3DLONG *in_sz,
//                                      unsigned int Ws,
//                                      unsigned int c,
//                                      double th);

//template <class T> T pow2(T a)
//{
//    return a*a;

//}
//NeuronTree post_process(NeuronTree nt);



QStringList RegMST::menulist() const
{
	return QStringList() 
		<<tr("tracing_menu")
		<<tr("about");
}

QStringList RegMST::funclist() const
{
	return QStringList()
		<<tr("tracing_func")
		<<tr("help");
}

void RegMST::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("tracing_menu"))
	{
        bool bmenu = true;
        input_PARA PARA;
        reconstruction_func(callback,parent,PARA,bmenu);

	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Amos Sironi and Przemyslaw Glowacki, 2015-6-23"));
	}
}

bool RegMST::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("tracing_func"))
	{
        bool bmenu = false;
        input_PARA PARA;

        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

        if(infiles.empty())
        {
            fprintf (stderr, "Need input image. \n");
            return false;
        }
        else
            PARA.inimg_file = infiles[0];
        int k=0;
        PARA.channel = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;
        PARA.n_ac_iters = (paras.size() >= k+1) ? atoi(paras[k]) : -1;  k++;
        for(unsigned int i_ac =0; i_ac < PARA.n_ac_iters+1; i_ac++,k++){
            PARA.regressor_paths.append(paras[k]);
        }
        PARA.Ws = (paras.size() >= k+1) ? atoi(paras[k]) : 10;  k++;

        reconstruction_func(callback,parent,PARA,bmenu);
	}
    else if (func_name == tr("help"))
    {

        ////HERE IS WHERE THE DEVELOPERS SHOULD UPDATE THE USAGE OF THE PLUGIN


		printf("**** Usage of RegMST tracing **** \n");
        printf("vaa3d -x RegMST -f tracing_func -i <inimg_file> -p <channel> <n_AC> <reg_path_1.cfg> ... <reg_path_n_AC.cfg> <window size>\n");
        printf("inimg_file                                The input image\n");
        printf("channel                                   Data channel for tracing. Start from 1 (default 1).\n");
        printf("n_AC                                      Number of autoncontext iterations to compute tubularity. 0 = no Auto-context; -1 = apply MST on original image (default -1)\n");
        printf("reg_path_1.cfg ... reg_path_n_AC.cfg      Path to regressors. Number of files must be equal to n_AC.\n");
        printf("window size                               Window size for seed detection in MST. (default 10)\n");

        printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");

	}
	else return false;

	return true;
}

void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu)
{
    unsigned char* data1d = 0;
    V3DLONG N,M,P,sc,c;
    V3DLONG in_sz[4];
    if(bmenu)
    {
        v3dhandle curwin = callback.currentImageWindow();
        if (!curwin)
        {
            QMessageBox::information(0, "", "You don't have any image open in the main window.");
            return;
        }

        Image4DSimple* p4DImage = callback.getImage(curwin);

        if (!p4DImage)
        {
            QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
            return;
        }


        data1d = p4DImage->getRawData();
        N = p4DImage->getXDim();
        M = p4DImage->getYDim();
        P = p4DImage->getZDim();
        sc = p4DImage->getCDim();

        bool ok1;

        if(sc==1)
        {
            c=1;
            ok1=true;
        }
        else
        {
            c = QInputDialog::getInteger(parent, "Channel",
                                             "Enter channel NO:",
                                             1, 1, sc, 1, &ok1);
        }

        if(!ok1)
            return;

        in_sz[0] = N;
        in_sz[1] = M;
        in_sz[2] = P;
        in_sz[3] = sc;


        PARA.inimg_file = p4DImage->getFileName();
    }
    else
    {
        int datatype = 0;
        if (!simple_loadimage_wrapper(callback,PARA.inimg_file.toStdString().c_str(), data1d, in_sz, datatype))
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",PARA.inimg_file.toStdString().c_str());
            return;
        }
        if(PARA.channel < 1 || PARA.channel > in_sz[3])
        {
            fprintf (stderr, "Invalid channel number. \n");
            return;
        }
        N = in_sz[0];
        M = in_sz[1];
        P = in_sz[2];
        sc = in_sz[3];
        c = PARA.channel;
    }

    //main neuron reconstruction code

    //// THIS IS WHERE THE DEVELOPERS SHOULD ADD THEIR OWN NEURON TRACING CODE

    std::cout << "size image input : " << N <<" "<< M << " " << P <<std::endl;


 V3DPluginArgList  input;
 V3DPluginArgItem name_input_v3d;
 char *buf_input = new char[PARA.inimg_file.toStdString().size()+1];
 strcpy(buf_input, PARA.inimg_file.toStdString().c_str());
 vector<char*> *buf_input_vec= new vector<char*>[1];
buf_input_vec->push_back(buf_input);
 name_input_v3d.p =buf_input_vec;
 input.append(name_input_v3d);

  std::cout << "PARA.inimg_file.toStdString().c_str():  " << PARA.inimg_file.toStdString().c_str()<< std::endl;
 std::cout << "buf input:  " << buf_input<< std::endl;
// std::cout << "name_input_v3d.p:  " << name_para_v3d.p<< std::endl;
 std::cout << "input.at(0).p:   " << (char*)input.at(0).p<< std::endl;

//V3DPluginArgItem name_input_v3d;//(PARA.inimg_file);
//name_input_v3d =
//input.at(0).p = 0;//(char *)PARA.inimg_file.toStdString().c_str();

V3DPluginArgList  output;
V3DPluginArgItem name_output_v3d;

QString out_tubularity_filename = PARA.inimg_file + "_tubularity.v3draw";
char *buf_out = new char[out_tubularity_filename.toStdString().size()+1];
strcpy(buf_out, out_tubularity_filename.toStdString().c_str());
vector<char*> *buf_out_vec= new vector<char*>[1];
buf_out_vec->push_back(buf_out);
name_output_v3d.p =buf_out_vec;
output.append(name_output_v3d);


//char buf_para_array[PARA.n_ac_iters+1];
//char **buf_para_point = new char*[PARA.n_ac_iters+1];

vector<char*> *buf_para_vec= new vector<char*>[PARA.n_ac_iters+1];

for(unsigned int i_ac = 0; i_ac < PARA.n_ac_iters+1; i_ac++){

    char *buf_para = new char[PARA.regressor_paths.at(i_ac).toStdString().size()+1];
    strcpy(buf_para, PARA.regressor_paths.at(i_ac).toStdString().c_str());
   // buf_para_array[i_ac] =*buf_para;
   // buf_para_point[i_ac] = buf_para;
    buf_para_vec->push_back(buf_para);



//   strcpy(&buf_para_array[i_ac], PARA.regressor_paths.at(i_ac).toStdString().c_str());

     std::cout << "PARA.regressor_paths.at(i_ac).toStdString().c_str():  " << PARA.regressor_paths.at(i_ac).toStdString().c_str()<< std::endl;
     std::cout << "buf_para:  " << buf_para<< std::endl;
   //  std::cout << "buf_para_array[i_ac]:  " << buf_para_array[i_ac]<< std::endl;
   //    std::cout << "buf_para_point[i_ac]:  " << buf_para_point[i_ac]<< std::endl;
     std::cout << "buf_para_vec.at(i_ac):  " << buf_para_vec->at(i_ac)<< std::endl;
    // std::cout << "input.at(i_ac+1).p:   " << (char * )input.at(i_ac+1).p<< std::endl;

}
//vector<char*> *params_array = ((vector<char*> *) buf_para_point);
//std::cout << "params size:  " <<params_array->size()<<std::endl;

std::cout << "params size:  " <<buf_para_vec->size()<<std::endl;


//std::cout << "copied Params " <<std::endl << std::flush;

if( (PARA.n_ac_iters+1)>0){
    V3DPluginArgItem name_para_v3d;
   // name_para_v3d.p =buf_para_array;
   // name_para_v3d.p =buf_para_point;
     name_para_v3d.p =buf_para_vec;
    input.append(name_para_v3d);
}
std::cout << "appended to input   " <<std::endl << std::flush;



MST_PARA mst_PARA;
mst_PARA.Ws = PARA.Ws;
mst_PARA.channel = PARA.channel;

if(!(PARA.n_ac_iters<0)){//compute regression tubularity

    std::cout << "calling tubularity plugin  " <<std::endl << std::flush;

//    vector<char*> *inputImagePaths = ((vector<char*> *)(input.at(0).p));
//    vector<char*> *outputImagePaths = ((vector<char*> *)(output.at(0).p));
//  std::cout << "input size:  " <<inputImagePaths->size()<< std::endl;
//   std::cout << "out size:  " <<outputImagePaths->size()<< std::endl;



    testTubularityImage(callback, input,  output);

    mst_PARA.inimg_file = out_tubularity_filename;

}else{//use original image for tracing
    mst_PARA.inimg_file = PARA.inimg_file;

    std::cout << "no ac iter:  " <<PARA.n_ac_iters<<std::endl<< std::flush;;

}

//now call tracing function

std::cout << "calling MST tracing:  " <<std::endl<< std::flush;;
    autotrace_mst(callback,parent,mst_PARA,bmenu);

//std::cout << "saving output  " <<std::endl << std::flush;
//    //Output
//    NeuronTree nt;

//	QString swc_name = PARA.inimg_file + "_RegMST.swc";
//	nt.name = "RegMST";

//    writeSWC_file(swc_name.toStdString().c_str(),nt);
//    if(!bmenu)
//    {
//      //  std::cout << "free memory  " <<std::endl << std::flush;
//        if(data1d) {delete []data1d; data1d = 0;}
//    }

//    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(swc_name.toStdString().c_str()),bmenu);

    return;
}
