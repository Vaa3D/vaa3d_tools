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
		<<tr("LCM_boost")
		<<tr("about");
}

QStringList LoadImageAndSWCPlugin::funclist() const
{
	return QStringList()
		<<tr("LCM_boost");
}

void LoadImageAndSWCPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("LCM_boost"))
	{
		Opencv_example(callback,parent);
	}
	else
	{
		v3d_msg(tr("This plugin will boost the performance of the existing methods and the result is stored in neuron_swc.swc"));
	}
}

bool LoadImageAndSWCPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("LCM_boost"))
	{
		return Opencv_example(callback, input, output);
	}


	if (func_name == tr("help"))
	{

		cout << "**** Usage of LCM_boost tracing **** " << endl;
		printf("vaa3d -x LCM_boost -f LCM_boost -i <inimg_file> -o <output_segmentation_file> <outswc_file>\n");

		printf("e.g.    vaa3d -x LCM_boost -f LCM_boost -i B4.v3draw -o tmp_tst.v3draw tmp_tst.swc \n");

		printf("inimg_file       The input image\n");

        printf("output_segmentation_file          The file name of output segmentation.\n");

        printf("outswc_file      The file name of output swc.\n");



	}

	if (func_name == tr("Batch"))
	{
		return Batch_Process(callback, input, output);
	}

}

