/* blastneuron_plugin.cpp
 * BlastNeuron plugin for rapid retrieve and alignment of neuron morphologies, also includes pre-processing, inverse projection and batch feature computation
 * 2016-06-03 : by Yinan Wan
 */

#include "pre_processing/pre_processing_main.h" 
#include "global_cmp/batch_compute_main.h" 
#include "global_cmp/global_retrieve_main.h" 
#include "tree_matching/local_alignment_main.h" 
#include "pointcloud_match/inverse_projection_main.h"
#include "v3d_message.h"
#include <vector>
#include "blastneuron_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(blastneuron, BlastNeuronPlugin);

QStringList BlastNeuronPlugin::menulist() const
{
	return QStringList() 
		<<tr("about");
}

QStringList BlastNeuronPlugin::funclist() const
{
	return QStringList()
		<<tr("global_retrieve")
		<<tr("local_alignment")
		<<tr("batch_compute")
		<<tr("pre_processing")
		<<tr("inverse_projection")
		<<tr("help");
}

void BlastNeuronPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	v3d_msg(tr("BlastNeuron plugin for rapid retrieve and alignment of neuron morphologies, also includes pre-processing, inverse projection and batch feature computation.\n\nDomenu is not supported yet. Please run the program from either of the following ways:\n1. Run indivisual function from dofunc: type \"vaa3d -x blastneuron -f help\" for detailed instruction\n2. Run a query search from the neuron feature database: try \"sh blastneuron_plugin.sh\" for detailed instruction\n\nDeveloped by Yinan Wan, 2016-06-03"));
}

bool BlastNeuronPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("global_retrieve"))
	{
		return(global_retrieve_main(input, output));
	}
	else if (func_name == tr("local_alignment"))
	{
		return(local_alignment_main(input, output));
	}
	else if (func_name == tr("batch_compute"))
	{
		return (batch_compute_main(input, output));
	}
	else if (func_name == tr("pre_processing"))
	{
		return (pre_processing_main(input, output));
	}
	else if (func_name == tr("inverse_projection"))
	{
		return (inverse_projection_main(input, output));
	}
	else
	{

		printf("\nBlastNeuron Plugin: rapid global retrieval and local alignent of 3D neuron morphologies. This plugin also includes pre-processing, inverse projection and batch feature computation. by Yinan Wan.\n\n");
		printf("Functions:\n");
		printf("\t global_retrieve         given a query neuron, retrieve morphologically similar neurons from a neuron feature database.\n");
		printf("\t local_alignment         point to point alignment of two morphologically similar neurosn.\n");
		printf("\t batch_compute           generate neuron feature database (.nfb) file first to speed up neuron retrieve\n");
		printf("\t invert_projection       find an optimal affine transform between neuron structures regardless of topology.\n\n");
		printf("Example: \n");
		printf("\t vaa3d -x blastneuron -f batch_compute -p \"#i mydatabase.ano #o features.nfb\"\n");
		printf("\t vaa3d -x blastneuron -f global_retrieve -p \"#d features.nfb #q query.swc #n 10 #m 1,2,4 #o retrieve.ano\"\n");
		printf("\t vaa3d -x blastneuron -f local_alignment -p \"#t target.swc #s subject.swc #o match.swc\"\n");
		printf("\t vaa3d -x blastneuron -f inverse_projection -p \"#i target.swc #o target_invp.swc\"\n\n");
		printf("Type vaa3d -x blastneuron -f <function_name> for instructions to run individual functions.\n");
		printf("Alternatively, you can run a query search from the neuron feature database with a bash script: try \"sh blastneuron_plugin.sh\" for detailed instruction\n\n");
		return true;
	}
}

