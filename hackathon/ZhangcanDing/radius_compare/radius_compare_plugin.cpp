/* radius_compare_plugin.cpp
 * This is a plugin to compare radius, input two swc and the node with big difference will be highlighted.
 * 2019-4-12 : by ZhangcanDing
 */
 
#include "v3d_message.h"
#include <vector>
#include "radius_compare_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(radius_compare, radius_compare);
 
QStringList radius_compare::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList radius_compare::funclist() const
{
	return QStringList()
		<<tr("radius_compare")
		<<tr("func2")
		<<tr("help");
}

void radius_compare::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
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
		v3d_msg(tr("This is a plugin to compare radius, input two swc and the node with big difference will be highlighted.. "
			"Developed by ZhangcanDing, 2019-4-12"));
	}
}

bool radius_compare::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("radius_compare"))
	{
        if(infiles.size()!=2)
        {   fprintf(stderr, "Need input two swc. \n");
            return false;
        }
        if(outfiles.empty())
        {
            fprintf(stderr,"please specify one output swc \n");
            return false;

        }

        NeuronTree nt0= readSWC_file(infiles[0]);
        NeuronTree nt1= readSWC_file(infiles[1]);
        NeuronTree nt2=swc_radius_compare(nt0,nt1);
        writeSWC_file(outfiles[0],nt2);

	}
    else if (func_name == tr("type_compare"))
	{
        if(infiles.size()!=2)
        {   fprintf(stderr, "Need input two swc. \n");
            return false;
        }
        if(outfiles.empty())
        {
            fprintf(stderr,"please specify one output swc \n");
            return false;

        }

        NeuronTree nt0= readSWC_file(infiles[0]);
        NeuronTree nt1= readSWC_file(infiles[1]);
        NeuronTree nt2=swc_radius_compare(nt0,nt1);
        writeSWC_file(outfiles[0],nt2);
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}


NeuronTree swc_radius_compare(NeuronTree nt0, NeuronTree nt1)
{
    NeuronTree result= nt0;
    QList<NeuronSWC> list0= nt0.listNeuron;
    QList<NeuronSWC> list1= nt1.listNeuron;

    for(int i=0; i<list0.size();i++)
    {

        double diff;
        double max_r= MAX(list0[i].radius,list1[i].radius);
        diff=ABS(list0[i].radius-list1[i].radius)/max_r;
        if (diff>0.7)
        {
            result.listNeuron[i].type=7;
        }


    }
    return result;




}





