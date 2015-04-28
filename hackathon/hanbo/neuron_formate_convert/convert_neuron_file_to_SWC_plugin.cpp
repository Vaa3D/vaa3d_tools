/* convert_neuron_file_to_SWC_plugin.cpp
 * The plugin will convert neuron file to swc format. It is based on the code of L-Measure, Sridevi Polavaram. Imported by Hanbo Chen.
 * 2015-4-27 : by Hanbo Chen
 */
 
#include "v3d_message.h"
#include <vector>
#include "convert_neuron_file_to_SWC_plugin.h"
#include "Lm/Neuron.h"
#include "asc_to_swc.h"
using namespace std;
Q_EXPORT_PLUGIN2(convert_neuron_file_to_SWC, neuronConverter);
 
QStringList neuronConverter::menulist() const
{
	return QStringList() 
		<<tr("convert_to_swc")
		<<tr("about");
}

QStringList neuronConverter::funclist() const
{
	return QStringList()
		<<tr("convert_to_swc")
		<<tr("help");
}

void neuronConverter::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("convert_to_swc"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("The plugin will convert neuron file to swc format. It is based on the code of L-Measure, Sridevi Polavaram. Imported by Hanbo Chen.. "
			"Developed by Hanbo Chen, 2015-4-27"));
	}
}

bool neuronConverter::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("convert_to_swc"))
	{
        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> * poutfiles = (output.size() >= 1) ? (vector<char*> *) output[0].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();
        vector<char*> outfiles = (poutfiles != 0) ? * poutfiles : vector<char*>();

        if(paras.empty())
        {
            fprintf (stderr, "Need input image. \n");
            printHelp();
            return false;
        }

        if(outfiles.empty())
        {
            fprintf (stderr, "Need output image. \n");
            printHelp();
            return false;
        }

        char * fname_in = paras[0];
        char * fname_out = outfiles[0];

        doLoadAndSave(fname_in, fname_out);
    }
	else if (func_name == tr("help"))
	{
        printHelp();
	}
	else return false;

	return true;
}

bool doLoadAndSave(char* fname_open, char* fname_save)
{
//    qDebug()<<"loading the file";
//    Neuron neuronfunc(fname_open);
//    qDebug()<<"saving the file";
//    neuronfunc.printSWC(fname_save);
    asc_to_swc asc2swc(fname_open, fname_save);

}

void printHelp()
{
    qDebug()<<"vaa3d -x libname -f convert_to_swc -i input_neuron_file -o output.swc";
}
