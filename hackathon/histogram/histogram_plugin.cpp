/* histogram_plugin.cpp
 * Display histogram of the image
 * 2012-03-01 : by Jianlong Zhou
 */
 
#include "v3d_message.h"

#include "histogram_plugin.h"
#include "histogram_func.h"
 
Q_EXPORT_PLUGIN2(histogram, HistogramPlugin);
 
QStringList HistogramPlugin::menulist() const
{
	return QStringList() 
                <<tr("Histogram")
		<<tr("about");
}

QStringList HistogramPlugin::funclist() const
{
	return QStringList()
		<<tr("compute");
}

void HistogramPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
        if (menu_name == tr("Histogram"))
	{
		compute(callback,parent);
	}
	else
	{
		v3d_msg(tr("Display histogram of the image. "
			"Developed by Jianlong Zhou, 2012-03-01"));
	}
}

bool HistogramPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("compute"))
	{
		return compute(input, output);
	}
}

