/* resample_swc_plugin.cpp
 * This is a plugin to resample neuron swc subject to a fixed step length.
 * 2012-03-02 : by Yinan Wan
 * 2014-10-02 : Hanbo Chen added adaptive threshold
 */
 
#include "v3d_message.h"

#include "resample_swc_plugin.h"
#include "resample_swc_func.h"
 
Q_EXPORT_PLUGIN2(resample_swc, ResamplePlugin);
 
QStringList ResamplePlugin::menulist() const
{
	return QStringList() 
        <<tr("resample")
        <<tr("adaptive downsample")
		<<tr("about");
}

QStringList ResamplePlugin::funclist() const
{
	return QStringList()
		<<tr("resample_swc")
		<<tr("resample_swc_adaptive")
		<<tr("help");
}

void ResamplePlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("resample (fixed step)"))
	{
		resample_swc(callback,parent);
	}
	else if (menu_name == tr("adaptive downsample"))
	{
		resample_swc_adaptive(callback,parent);
	}
	else
	{
		v3d_msg(tr("This is a plugin to resample neuron swc subject to a fixed step length or adaptively downsample neuron swc based on curveness and radius change.\n"
			"Developed by Yinan Wan, 2012-03-02\n"
			"Updated by Hanbo Chen, 2014-10-02"));
	}
}

bool ResamplePlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("resample_swc"))
	{
		return resample_swc(input, output);
	}else if (func_name == tr("resample_swc_adaptive"))
	{
		return resample_swc(input, output);
	}
	else if (func_name == tr("TOOLBOXresample"))
	{
		resample_swc_toolbox(input);
		return true;
	}
	if (func_name == tr("help"))
	{
		printf("\n\n(version 1.0) Resample points in a swc file subject to a fixed step length. Developed by Yinan Wan 2012-03-02\n");
		printf("usage:\n");
		printf("\t-f <function_name>:     resample_swc\n");
		printf("\t-i <input_file_name>:   input .swc\n");
		printf("\t-o <output_file_name>:  (not required) resampled swc file. DEFAULT: 'inputName_resampled.swc'\n");
		printf("\t-p <step_length>:       step length for resampling.\n");
		printf("Demo: v3d -x resample_swc -f resample_swc -i test.swc -o test_resampled.swc -p 1\n\n");
	}
	return false;
}

