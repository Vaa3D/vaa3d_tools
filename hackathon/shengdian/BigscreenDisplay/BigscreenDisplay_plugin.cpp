/* BigscreenDisplay_plugin.cpp
 * Use this plugin to control the bigscreen.
 * 2018-10-15 : by Shengdian Jiang
 */
 
#include "v3d_message.h"
#include <vector>
#include "BigscreenDisplay_plugin.h"
#include "BigScreenDispaly_func.h"

using namespace std;
Q_EXPORT_PLUGIN2(BigscreenDisplay, BigscreenDisplay);
 
QStringList BigscreenDisplay::menulist() const
{
	return QStringList() 
        <<tr("Bigscreen Display")
		<<tr("about");
}

QStringList BigscreenDisplay::funclist() const
{
	return QStringList()
        <<tr("BigscreenDisplay")
		<<tr("help");
}

void BigscreenDisplay::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("Bigscreen Display"))
    {
        bigscreenDisplay(callback,parent);
    }
	else
	{
        v3d_msg(tr("Using this plugin to control the bigscreen.. "
			"Developed by Shengdian Jiang, 2018-10-15"));
	}
}

bool BigscreenDisplay::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    vector<char*> inparas;
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(input.size() != 2)
    {
        printf("Please specify both input folder and the number of new finished Neurons.\n");
        printHelpForBigScreenUsage();
        return false;
    }
    if (inparas.size()!=2)
    {
        printf("Please specify only one parameter - the number of new finished Neurons.\n");
        printHelpForBigScreenUsage();
        return false;
    }

    int updateInterval = atof(inparas.at(0));

    if (func_name == tr("help"))
    {
        printHelpForBigScreenUsage();
    }
    else if(func_name==tr("BigScreenDisplay"))
    {
        MethodFunForBigScreenDisplay(callback,parent,input);
        int updateCount=0;

        while(true)
        {
            //sleep(updateInterval);
            if(updateCount==updateInterval)
            {
                MethodFunForUpdateSWCDispaly(callback,parent,input);
                updateCount=0;
            }
            ZmovieMaker(callback,parent,input);
            updateCount++;
        }

    }
    else return false;

    return true;
}

