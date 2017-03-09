/* blastneuron_bjut_plugin.cpp
 * Compare neuron constructions using blastneuron method.
 * 2016-11-30 : by He Yishan
 */
 
#include "v3d_message.h"
#include "blastneuron_bjut_plugin.h"
#include "blastneuron_bjut_func.h"
#include "overlap/overlap_gold.h"
#include "overlap/pre_overlap.h"
#include "prune_alignment.h"
using namespace std;
Q_EXPORT_PLUGIN2(blastneuron_bjut, blastneuron_bjutPlugin);
 
QStringList blastneuron_bjutPlugin::menulist() const
{
	return QStringList() 
        <<tr("pre-process")
        <<tr("apply_blastneuron")
        <<tr("prune_alignment")
		<<tr("about");
}

QStringList blastneuron_bjutPlugin::funclist() const
{
	return QStringList()
        <<tr("pre-process")
        <<tr("apply_blastneuron")
        <<tr("prune_alignment")
        <<tr("pre_overlap")
        <<tr("overlap_gold")
		<<tr("help");
}

void blastneuron_bjutPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("pre-process"))
    {
        v3d_msg("Domenu is not supported yet.");
    }
    else if (menu_name == tr("apply_blastneuron"))
	{
        v3d_msg("Domenu is not supported yet.");
	}
    else if (menu_name == tr("prune_alignment"))
    {
        v3d_msg("Domenu is not supported yet.");
    }
    else{
		v3d_msg(tr("Compare neuron constructions using blastneuron method.. "
			"Developed by He Yishan, 2016-11-30"));
	}
}

bool blastneuron_bjutPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("pre_process"))
	{
        return pre_process_func(input,output);
	}
    else if (func_name == tr("apply_blastneuron"))
    {
             return blastneuron_main(input,output);
    }
    else if (func_name == tr("prune_alignment"))
    {
        return (prune_alignment(input, output));
    }
    else if (func_name == tr("overlap_gold"))
    {
        return(overlap_gold(input,output,callback));
    }
    else if (func_name == tr("pre_overlap"))
    {
        return(pre_overlap(input,output,callback));
    }
    else
    {
        printf("\nBlastneuron_bjut Plugin: local alignent of 3D neuron morphologies. Includes pre-processing and post-processing. \n\n");
        printf("Functions:\n");
        printf("pre_process              do resample an sort function,where -p is resample step.\n");
        printf("apply_blastneuron   do local_alignment function.\n");
        printf("prune_alignment      prune the connection between two far aligned points,-p is distance threshold.\n");
        printf("pre_overlap               if use method_mean, need do this first.\n");
        printf("overlap_gold             find overlap from local alignment results, p1 is method, p2 is dist_para, p3 is meth_para, p4 is prune short tree pare.");
        printf("Example:\n");
        printf("vaa3d -x blastneuron_bjut -f pre_process -i input.swc goldstandard.swc -o result.swc -p 1\n");
        printf("vaa3d -x blastneuron_bjut -f apply_blastneuron -i input1.swc input2.swc -o result.swc \n");
        printf("vaa3d -x blastneuron_bjut -f prune_alignment -i input.swc -o result.swc -p 20\n");
        printf("vaa3d -x blastneuron_bjut -f pre_overlap -i input.swc raw_img -o result.swc\n");
        printf("vaa3d -x blastneuron_bjut -f overlap_gold -i 'swcfolder'  'rawimg' -o result.swc -p 0 3 6 3");
        return true;
    }
}

