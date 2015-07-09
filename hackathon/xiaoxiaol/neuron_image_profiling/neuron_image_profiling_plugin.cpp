/* neuron_image_profiling_plugin.cpp
 * This is a plugin for profiling image qualities for neuron tracing, via SNR and tubuliarty measures.
 * Given an input swc with labled neurite types (soma, dendrite and axon), generate dynamic range and SNR
 * 2015-7-6 : by Xiaoxiao Liu and Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "neuron_image_profiling_plugin.h"
#include "profile_swc.h"
#include <iostream>

using namespace std;
Q_EXPORT_PLUGIN2(neuron_image_profiling, image_profiling);
 
QStringList image_profiling::menulist() const
{
	return QStringList() 
		<<tr("profile_swc")
		<<tr("about");
}

QStringList image_profiling::funclist() const
{
	return QStringList()
		<<tr("profile_swc")
		<<tr("help");
}

void image_profiling::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("profile_swc"))
	{
		if (!	profile_swc_menu(callback, parent))
		{
                     std::cout << "error in profile_swc_menu() " << std::endl;
		}  
	}
	else 
	{
		printHelp(callback, parent);
	}
}

bool image_profiling::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("profile_swc"))
	{
                    profile_swc_func(callback,input, output);
	}
	else if (func_name == tr("help"))
	{
                printHelp(input, output);
	}
    else
        return false;

	return true;
}

