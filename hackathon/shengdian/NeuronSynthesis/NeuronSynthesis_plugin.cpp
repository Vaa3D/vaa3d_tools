/* NeuronSynthesis_plugin.cpp
 * This plugin is for neuron synthesis project.
 * 2021-5-31 : by Shengdian
 */
 
#include "v3d_message.h"
#include <vector>
#include "NeuronSynthesis_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(NeuronSynthesis, NeuronSynthesis);
 
QStringList NeuronSynthesis::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("about");
}

QStringList NeuronSynthesis::funclist() const
{
	return QStringList()
        <<tr("Tip_branch_pruning")
       <<tr("Three_bifurcation_processing")
      <<tr("Split_neuron_types")
       <<tr("To_topology_tree")
      <<tr("To_branches")
		<<tr("help");
}

void NeuronSynthesis::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("menu1"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This plugin is for neuron synthesis project.. "
			"Developed by Shengdian, 2021-5-31"));
	}
}

bool NeuronSynthesis::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("Tip_branch_pruning"))
	{
        /* designed by shengdian, 2021-05-31
         * ---
         * remove small tip_branches
         * --Usage--
         * input: swc or eswc file
         * input para list:
                    * 1. pruning   -----default: 20 pixels
                    * 2. three-bifruction processing
                    * 3. reorder index
                    * 4. save to eswc
         * output: processed swc file
        */
        string inswc_file;
        if(infiles.size()>=1) {inswc_file = infiles[0];}
        else {  printHelp(); return false;}
        //read para list
        int pruning_thre =(inparas.size()>=1)?atoi(inparas[0]):20;
        int threebp_flag=(inparas.size()>=2)?atoi(inparas[1]):1;
        int reorder=(inparas.size()>=3)?atoi(inparas[2]):1;
        int saveESWC=(inparas.size()>=4)?atoi(inparas[3]):0;

        string out_swc_file=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_pruning.swc");
        if(saveESWC)
            out_swc_file=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_pruning.eswc");
        //read swc
        NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));

        NeuronTree nt_3bp;nt_3bp.deepCopy(nt);
        if(threebp_flag)
        {
            //processing of three-bifruction
            nt_3bp.listNeuron.clear();nt_3bp.hashNeuron.clear();
            nt_3bp=three_bifurcation_processing(nt);
        }
        //pruning and reorder-Index of swc
        NeuronTree nt_out;nt_out.listNeuron.clear();nt_out.hashNeuron.clear();
        nt_out=tip_branch_pruning(nt_3bp,pruning_thre);
        if(reorder)
            nt_out=reindexNT(nt_out);
        //save to file
        if(saveESWC)
            writeESWC_file(QString::fromStdString(out_swc_file),nt_out);
        else
            writeSWC_file(QString::fromStdString(out_swc_file),nt_out);
	}
    else if (func_name == tr("Three_bifurcation_processing"))
    {
        /* designed by shengdian, 2021-06-02
         * ---
         * processing of three-bifurcation branches
         * --Usage--
         * input: swc or eswc file
         * output: processed swc file
        */
        string inswc_file;
        if(infiles.size()>=1) {inswc_file = infiles[0];}
        else {  printHelp(); return false;}
        //read para list
        int saveESWC=(inparas.size()>=1)?atoi(inparas[0]):0;
        string out_swc_file=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_3bp.swc");
        if(saveESWC)
            out_swc_file=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_3bp.eswc");
        //read swc
        NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
        NeuronTree nt_out;nt_out.listNeuron.clear();nt_out.hashNeuron.clear();
        nt_out=three_bifurcation_processing(nt);
        //save to file
        if(saveESWC)
            writeESWC_file(QString::fromStdString(out_swc_file),nt_out);
        else
            writeSWC_file(QString::fromStdString(out_swc_file),nt_out);
    }
    else if (func_name == tr("Split_neuron_types"))
    {
        /* designed by shengdian, 2021-06-02
         * ---
         * split neuron tree to different part, by node types
                 * type2: axon;
                 * type3: dendrite;
                 * type4: apical dendrite
                 * type3_4: whole dendrite
         * --Usage--
         * input: swc or eswc file
         * output: processed swc file
        */
        string inswc_file;
        if(infiles.size()>=1) {inswc_file = infiles[0];}
        else {  printHelp(); return false;}
        //read para list
        int saveESWC=(inparas.size()>=1)?atoi(inparas[0]):0;
        QString outpath=(outfiles.size()>=1)?outfiles[0]:"";
        split_neuron_type(QString::fromStdString(inswc_file),outpath,saveESWC);
    }
    else if (func_name == tr("To_topology_tree"))
    {
        /* designed by shengdian, 2021-05-31
         * ---
         * remove internodes, only soma-node, branch-nodes and tip-nodes are kept
         * --Usage--
         * input: swc or eswc file
         * input para list:
                    * 1. pruning
                    * 2. three-bifruction processing
                    * 3. reorder index
                    * 4. save to eswc
         * output: processed swc file
        */
        string inswc_file;
        if(infiles.size()>=1) {inswc_file = infiles[0];}
        else {  printHelp(); return false;}
        //read para list
        int pruning_thre =(inparas.size()>=1)?atoi(inparas[0]):20;
        int threebp_flag=(inparas.size()>=2)?atoi(inparas[1]):1;
        int reorder=(inparas.size()>=3)?atoi(inparas[2]):1;
        int saveESWC=(inparas.size()>=4)?atoi(inparas[3]):0;
        string out_swc_file=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_topo.swc");
        if(saveESWC)
            out_swc_file=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_topo.eswc");
        //read swc
        NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));

        NeuronTree nt_3bp;nt_3bp.deepCopy(nt);
        if(threebp_flag)
        {
            //processing of three-bifruction
            nt_3bp.listNeuron.clear();nt_3bp.hashNeuron.clear();
            nt_3bp=three_bifurcation_processing(nt);
        }
        NeuronTree nt_3bp_p;nt_3bp_p.deepCopy(nt_3bp);
        if(pruning_thre>0)
        {
            //pruning
            nt_3bp_p.listNeuron.clear();nt_3bp_p.hashNeuron.clear();
            nt_3bp_p=tip_branch_pruning(nt_3bp,pruning_thre);
        }

        NeuronTree nt_out;
        nt_out.listNeuron.clear();nt_out.hashNeuron.clear();
        /*processing*/
        nt_out=to_topology_tree(nt_3bp_p);
        //reorder index
        if(reorder)
            nt_out=reindexNT(nt_out);
        //save to file
        if(saveESWC)
            writeESWC_file(QString::fromStdString(out_swc_file),nt_out);
        else
            writeSWC_file(QString::fromStdString(out_swc_file),nt_out);
    }
    else if (func_name == tr("To_branches"))
    {
        /* designed by shengdian, 2021-06-01
         * ---
         * get specified features of a neuron tree
              * global features:
                    * name
                        *type2,type3,type4
                            * #branches,#tips,Max_branch_order,topoLength,pathLength
              * branch features:
                    * branch_id, parent_branch_id
                    * branch_topo_length,branch_path_length
                    * branch_topo_len_ratio, branch_path_len_ratio
                    * xoy_angle,yoz_angle
                    * tip_ratio, balance_ratio
                    * spoint(x,y,z),...,epoint(x,y,z)
         * --Usage--
         * input: swc or eswc file
         * output: <filename>.brs
        */

    }
	else if (func_name == tr("help"))
	{
        printHelp();
	}
	else return false;

	return true;
}

void printHelp()
{
    /*designed by shengdian, 2021-06-01
     *---
     * 1. remove small tip-branches
     * 2. swc to topological-tree
     * 3. get branches and its features for training
     * 4.
     * ---
    */
    qDebug()<<"               -----Usage_basic: function for neuron synthesis project----              ";
    qDebug()<<"1. vaa3d -x <libname> -f Tip_branch_pruning -i <input_swc> -p <pruning_threshold> <save eswc> -o <out_file_path>";
    qDebug()<<"2. vaa3d -x <libname> -f To_topology_tree -i <input_swc> -p <save eswc> -o <out_file_path>";
}
