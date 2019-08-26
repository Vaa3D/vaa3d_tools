/* reconstruction_IO_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-10-29 : by Peng Xie
 */
 
#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"

#include "reconstruction_IO_plugin.h"
Q_EXPORT_PLUGIN2(reconstruction_IO, reconstruction_IO);

using namespace std;

void reconstruction_IO_help(){
    ////HERE IS WHERE THE DEVELOPERS SHOULD UPDATE THE USAGE OF THE PLUGIN


    printf("**** Usage of reconstruction_IO tracing **** \n");
    printf("vaa3d -x reconstruction_IO -f convert -i <inimg_file> -p <channel> <other parameters>\n");
    printf("inimg_file       The input image\n");
    printf("channel          Data channel for tracing. Start from 1 (default 1).\n");

    printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");

}
 
QStringList reconstruction_IO::menulist() const
{
	return QStringList() 
		<<tr("about");
}

QStringList reconstruction_IO::funclist() const
{
	return QStringList()
		<<tr("convert")
		<<tr("help");
}

void reconstruction_IO::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("about"))
    {
        reconstruction_IO_help();
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Peng Xie, 2018-10-29"));
	}
}

bool reconstruction_IO::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{

    vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
    vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
    vector<char*> * poutfiles = (output.size() >= 1) ? (vector<char*> *) output[0].p : 0;
    vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
    vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();
    vector<char*> outfiles = (poutfiles != 0) ? * poutfiles : vector<char*>();

    if(infiles.empty())
    {
        fprintf (stderr, "Need input image. \n");
        return false;
    }
    QString inputfile = QString(infiles.at(0));
    QString outputlabel;
    if (outfiles.size()>=1){
        outputlabel = outfiles.at(0);
    }
	if (func_name == tr("convert"))
	{
        if (paras.size()>=2){
            QString informat = QString(paras.at(0));
            QString outformat = QString(paras.at(1));
            convert_format(inputfile, outputlabel, informat, outformat);
        }
        else{
            fprintf (stderr, "Need input and output format. \n");
        }

	}
    else if (func_name == tr("scale"))
    {
        if (paras.size()==3){
            double xscale = atof(paras.at(0));
            double yscale = atof(paras.at(1));
            double zscale = atof(paras.at(2));
            scale(inputfile, outputlabel, xscale, yscale, zscale);
        }
        else{
            fprintf(stderr, "Need scale factors for xyz axes.");
        }
    }
    else if (func_name == tr("help"))
    {
        reconstruction_IO_help();

	}
	else return false;

	return true;
}


