/* boundary_gd_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-7-2 : by DZC
 */
 
#include "v3d_message.h"
#include <vector>
#include "boundary_gd_func.h"
#include "boundary_gd_plugin.h"

Q_EXPORT_PLUGIN2(boundary_gd, boundary_gd);

using namespace std;


 
QStringList boundary_gd::menulist() const
{
	return QStringList() 
		<<tr("boundary_gd")
		<<tr("about");
}

QStringList boundary_gd::funclist() const
{
	return QStringList()
		<<tr("boundary_gd")
		<<tr("help");
}

void boundary_gd::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("boundary_gd"))
	{
        bool bmenu = true;
        input_PARA PARA;
        reconstruction_func(callback,parent,PARA,bmenu);

	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by DZC, 2019-7-2"));
	}
}

bool boundary_gd::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("boundary_gd"))
	{
        bool bmenu = false;
        input_PARA PARA;

        vector<char*> infiles, inparas, outfiles;
        if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
        if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
        if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

        if(infiles.empty())
        {
            fprintf (stderr, "Need input image. \n");
            return false;
        }
        else
            PARA.inimg_file = infiles[0];
        int k=0;
        PARA.channel = (inparas.size() >= k+1) ? atoi(inparas[k]) : 1;  k++;
        //reconstruction_func(callback,parent,PARA,bmenu);
        // extract points on boundary
        PARA.fore_thresh=20;
        PARA.outimg_file=outfiles[0];
        PARA.search_radius=10;
        extract_boundary_points(callback,PARA);





	}
    else if (func_name == tr("help"))
    {

        printf("**** Usage of boundary_gd tracing **** \n");
		printf("vaa3d -x boundary_gd -f boundary_gd -i <inimg_file> -p <channel> <other parameters>\n");
        printf("inimg_file       The input image\n");
        printf("channel          Data channel for tracing. Start from 1 (default 1).\n");

        printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");

	}
	else return false;

	return true;
}


