/* sv_trace_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-7-3 : by DZC_ZX
 */
 
#include "v3d_message.h"
#include <vector>
#include "sv_trace_plugin.h"
#include "sv_trace_func.h"

#include <iostream>

using namespace std;
Q_EXPORT_PLUGIN2(sv_trace, sc_trace);
 
QStringList sc_trace::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList sc_trace::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void sc_trace::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("menu1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by DZC_ZX, 2019-7-3"));
	}
}

bool sc_trace::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
        //if(infiles.size()<)
        QString img= infiles[0];
        unsigned char* pdata=0;
        V3DLONG sz[4];
        int datatype;
        simple_loadimage_wrapper(callback, img.toStdString().c_str(),pdata,sz,datatype);

        sv_tracer sv;
        vector<superpoint> V_superpoint;
        vector <superpoint> V_real_superpoints;
        double thres=30.0;
        sv.init_superpoints(V_superpoint, pdata,sz,thres);
        cout<<"initial: "<<V_superpoint.size()<<endl;
        sv.init_real_points(V_superpoint,V_real_superpoints,pdata,sz,thres);

        cout<<"superpoints size:"<<V_superpoint.size()<<endl;

        cout<<"realpoints size:"<<V_real_superpoints.size()<<endl;
        for(int i=0;i<V_real_superpoints.size();++i)
        {
            //cout<<"r: "<<V_real_superpoints[i].rx<<" intensity: "<<V_real_superpoints[i].intensity<<endl;
        }


        QString markerfile=outfiles[0];
        sv.writeSuperpoints(markerfile,V_real_superpoints);





	}
	else if (func_name == tr("func2"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

