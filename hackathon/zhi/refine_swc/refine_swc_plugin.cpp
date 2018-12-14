/* refine_swc_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-12-6 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "refine_swc_plugin.h"
#include "refine_swc_funcs.h"

#include "neuron_format_converter.h"
#include "../AllenNeuron_postprocessing/sort_swc_IVSCC.h"


using namespace std;
Q_EXPORT_PLUGIN2(refine_swc, refine_swc);

QStringList refine_swc::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList refine_swc::funclist() const
{
	return QStringList()
		<<tr("refine")
		<<tr("func2")
		<<tr("help");
}

void refine_swc::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
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
			"Developed by Zhi Zhou, 2018-12-6"));
	}
}

bool refine_swc::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("refine"))
	{
        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> * poutfiles = (output.size() >= 1) ? (vector<char*> *) output[0].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();
        vector<char*> outfiles = (poutfiles != 0) ? * poutfiles : vector<char*>();

        if(infiles.size()<2)
        {
            fprintf (stderr, "Need input terafly image path and swc. \n");
            return false;
        }

        if(outfiles.empty())
        {
            fprintf (stderr, "Need output file name. \n");
            return false;
        }

        int k=0;
        int max_length = (paras.size() >= k+1) ? atof(paras[k]) : 200;  k++;

        QList<ImageMarker> break_points;
        NeuronTree nt = readSWC_file(QString(infiles[1]));

        NeuronTree nt2 = SortSWC_pipeline(nt.listNeuron,VOID, 0);
        NeuronTree nt2_broken = breakSWC(nt2,max_length, break_points);


        V_NeuronSWC_list nt2_decomposed = NeuronTree__2__V_NeuronSWC_list(nt2_broken);
        NeuronTree nt2_new = V_NeuronSWC_list__2__NeuronTree(nt2_decomposed);
        NeuronTree nt2_refined = refineSWCTerafly(callback,infiles[0],nt2_new);
        NeuronTree nt2_sorted = SortSWC_pipeline(nt2_refined.listNeuron,VOID, 0);
        NeuronTree nt2_smoothed = smoothSWCTerafly(callback,infiles[0],nt2_sorted,break_points);
        NeuronTree nt2_smoothed_sorted = SortSWC_pipeline(nt2_smoothed.listNeuron,VOID, 0);

        writeESWC_file(QString(outfiles[0]),nt2_smoothed_sorted);

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
