/* Opencv_example_plugin.cpp
 * This plugin will load image and swc in domenu and dofunc
 * 2012-02-21 : by Hang Xiao
 */

#include "v3d_message.h"

#include "LCM_boost_plugin.h"
#include "LCM_boost_func.h"

Q_EXPORT_PLUGIN2(Opencv_example, LoadImageAndSWCPlugin);

QStringList LoadImageAndSWCPlugin::menulist() const
{
	return QStringList()
		<<tr("LCM_boost(MultiScale Ehancement based)")
		<<tr("LCM_boost(Fast Marching based)")
		<<tr("LCM_boost(mostVesselTracer based)")
		<<tr("LCM_boost(neuTube based)")
		<<tr("LCM_boost(SimpleTracing based)")
		<<tr("LCM_boost(APP2 based)")
		<<tr("LCM_boost(APP1 based)")
		<<tr("LCM_boost(fastmarching_spanningtree based)")
		<<tr("LCM_boost(NeuroGPSTree based)")
		<<tr("LCM_boost(neurontracing_mst based)")
		<<tr("about");
}

QStringList LoadImageAndSWCPlugin::funclist() const
{
	return QStringList()
		<<tr("LCM_boost");
}

void LoadImageAndSWCPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{



  //  cout << menu_name.toStdString() << endl;

    //   cin.get();

	if (menu_name == tr("LCM_boost(MultiScale Ehancement based)"))
	{
		(callback,parent,1,0);
	}

	if (menu_name == tr("LCM_boost(Fast Marching based)"))
	{
		General_Boost1(callback,parent,2,0);
	}

	if (menu_name == tr("LCM_boost(mostVesselTracer based)"))
	{
		General_Boost1(callback,parent,3,1);
	}

	if (menu_name == tr("LCM_boost(neuTube based)"))
	{
		General_Boost1(callback,parent,3,2);
	}

	if (menu_name == tr("LCM_boost(SimpleTracing based)"))
	{
		General_Boost1(callback,parent,3,3);
	}

	if (menu_name == tr("LCM_boost(APP2 based)"))
	{
		General_Boost1(callback,parent,3,4);
	}

	if (menu_name == tr("LCM_boost(APP1 based)"))
	{
		General_Boost1(callback,parent,3,5);
	}

	if (menu_name == tr("LCM_boost(fastmarching_spanningtree based)"))
	{
		General_Boost1(callback,parent,3,6);
	}

	if (menu_name == tr("LCM_boost(NeuroGPSTree based)"))
	{
		General_Boost1(callback,parent,3,7);
	}


	if (menu_name == tr("LCM_boost(neurontracing_mst based)"))
	{
		General_Boost1(callback,parent,3,8);
	}


	if(menu_name == tr("about"))
	{
		v3d_msg(tr("This plugin will boost the performance of the existing methods"));
	}
}

bool LoadImageAndSWCPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("LCM_boost"))
	{
		return General_Boost1(callback,input,output,1,0);
	}

	if (func_name == tr("Batch"))
	{
		return Batch_Process(callback, input, output);
	}

    if (func_name == tr("PreTraining"))
	{

		return Batch_Base_Method(callback, input, output);
	}

    if (func_name == tr("TrainLCM"))
	{

		return train_LCM(callback, input, output);
	}


    if (func_name == tr("BatchTest"))
	{

       Batch_Test1(callback, input, output,1,0);

       return Batch_Test1(callback, input, output,2,0);

     //  return Batch_Test2000(callback, input, output,1,0);
	}

    if (func_name == tr("BatchTest2000"))
	{

		return Batch_Test2000(callback, input, output,1,0);
	}




//    if (func_name == tr("BatchTest1"))
//	{

//		return Batch_Test1(callback, input, output);
//	}


	if (func_name == tr("LCM_boost_1"))
	{
		return General_Boost1(callback,input,output,1,0);
	}

	if (func_name == tr("LCM_boost_2"))
	{
		return General_Boost1(callback,input,output,2,0);
	}

	if (func_name == tr("LCM_boost_3"))
	{
		return General_Boost1(callback,input,output,3,1);
	}

	if (func_name == tr("LCM_boost_4"))
	{
		return General_Boost1(callback,input,output,3,2);
	}

	if (func_name == tr("LCM_boost_5"))
	{
		return General_Boost1(callback,input,output,3,3);
	}

	if (func_name == tr("LCM_boost_6"))
	{
		return General_Boost1(callback,input,output,3,4);
	}

	if (func_name == tr("LCM_boost_7"))
	{
		return General_Boost1(callback,input,output,3,5);
	}

	if (func_name == tr("LCM_boost_8"))
	{
		return General_Boost1(callback,input,output,3,6);
	}

	if (func_name == tr("LCM_boost_9"))
	{
		return General_Boost1(callback,input,output,3,7);
	}

	if (func_name == tr("LCM_boost_10"))
	{
		return General_Boost1(callback,input,output,3,8);
	}

		if (func_name == tr("help"))
	{

		cout << "**** Usage of LCM_boost tracing **** " << endl;
		printf("./vaa3d -x LCM_boost -f LCM_boost -i <inimg_file> -o  <outswc_file>\n");

		printf("e.g.    ./vaa3d -x LCM_boost -f LCM_boost -i B4.v3draw -o tmp_tst.swc \n");

		printf("inimg_file       The input image\n");

        printf("outswc_file      The file name of output swc.\n");

        printf("The default base method is multiScale enhancement. Users could also denote alternative base method \n");

        printf(" The function name LCM_boost_x (x = 1,.., 10) correponds to different base method.\n");

        printf(" The function name LCM_boost_1 corresponds to multiScale enhancement base method (default). \n");

        printf(" The function name LCM_boost_2 corresponds to fast marching base method. \n");

        printf(" The function name LCM_boost_3 corresponds to mostVesselTracer base method. \n");

        printf(" The function name LCM_boost_4 corresponds to neuTube base method. \n");

        printf(" The function name LCM_boost_5 corresponds to SimpleTracing base method. \n");

        printf(" The function name LCM_boost_6 corresponds to APP2 base method. \n");

        printf(" The function name LCM_boost_7 corresponds to APP1 base method. \n");

        printf(" The function name LCM_boost_8 corresponds to fastmarching_spanningtree base method. \n");

        printf(" The function name LCM_boost_9 corresponds to NeuroGPSTree base method. \n");

        printf(" The function name LCM_boost_10 corresponds to neurontracing_mst base method. \n");

	}



}




