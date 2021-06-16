/* UtilityArbor_plugin.cpp
 * Generation of the utility arbors
 * 2020-9-24 : by Shengdian
 */
 
#include "v3d_message.h"
#include <vector>
#include "UtilityArbor_plugin.h"
#include "Utility_function.h"
using namespace std;
Q_EXPORT_PLUGIN2(UtilityArbor, UtilityArbor);
 
QStringList UtilityArbor::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("about");
}

QStringList UtilityArbor::funclist() const
{
    return QStringList()
            <<tr("GetNodeType")
           <<tr("GetNodeOrder")
          <<tr("GetNodeSubtree")
         <<tr("GetSubtreeLength")
        <<tr("GetNodeTips")
       <<tr("Pruning_SubTree")
		<<tr("help");
}

void UtilityArbor::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("menu1"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("Generation of the utility arbors. "
			"Developed by Shengdian, 2020-9-24"));
	}
}

bool UtilityArbor::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("GetNodeType"))
	{
        /* designed by shengdian, 2021-06-14
         * ---
         * Get the node type of neuron.
         * soma: >=3, branch: =2; tip: =0; internodes: =1
         * --Usage--
         * input: swc or eswc file
         * output: processed swc file
        */
        string inswc_file;
        if(infiles.size()>=1) {inswc_file = infiles[0];}
        else {  printHelp(); return false;}
        //read para list
        int to_radius=(inparas.size()>=1)?atoi(inparas[0]):0;

        string out_swc_file=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_with_node_type.swc");
        //read swc
        NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
        vector<int> ntype=getNodeType(nt);
        if(to_radius)
        {
            NeuronTree nt_out;nt_out.listNeuron.clear();nt_out.hashNeuron.clear();
            for (V3DLONG i=0;i<nt.listNeuron.size();i++)
            {
                NeuronSWC s = nt.listNeuron[i];
                s.radius=ntype[i];
                if(ntype[i]==0)
                    s.radius=1;
                else if(ntype[i]==1)
                    s.radius=0.2;
                nt_out.listNeuron.append(s);
                nt_out.hashNeuron.insert(s.n,nt_out.listNeuron.size()-1);
            }
            writeSWC_file(QString::fromStdString(out_swc_file),nt_out);
        }
	}
    else if (func_name == tr("GetNodeTips"))
    {
        /* designed by shengdian, 2021-06-15
         * ---
         * Get terminal number of every node
         * --Usage--
         * input: swc or eswc file
         * output: processed swc file
        */
        string inswc_file;
        if(infiles.size()>=1) {inswc_file = infiles[0];}
        else {  printHelp(); return false;}
        //read para list
        int to_file=(inparas.size()>=1)?atoi(inparas[0]):1;

        string out_swc_file=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_with_tips.swc");
        //read swc
        NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
        vector<int> ntips=getNodeTips(nt);

        if(to_file)
        {
            NeuronTree nt_out;nt_out.listNeuron.clear();nt_out.hashNeuron.clear();
            for (V3DLONG i=0;i<nt.listNeuron.size();i++)
            {
                NeuronSWC s = nt.listNeuron[i];
                s.radius=ntips[i];
                nt_out.listNeuron.append(s);
                nt_out.hashNeuron.insert(s.n,nt_out.listNeuron.size()-1);
            }
            writeSWC_file(QString::fromStdString(out_swc_file),nt_out);
        }
    }
    else if (func_name == tr("GetNodeOrder"))
	{
        /* designed by shengdian, 2021-06-15
         * ---
         * Get the node order or depth of neuron.
         * --Usage--
         * input: swc or eswc file
         * output: processed swc file
        */
        string inswc_file;
        if(infiles.size()>=1) {inswc_file = infiles[0];}
        else {  printHelp(); return false;}
        //read para list
        int to_file=(inparas.size()>=1)?atoi(inparas[0]):0;

        string out_swc_file=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_with_node_order.swc");
        //read swc
        NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
        vector<int> norder=getNodeOrder(nt);
        if(to_file)
        {
            NeuronTree nt_out;nt_out.listNeuron.clear();nt_out.hashNeuron.clear();
            for (V3DLONG i=0;i<nt.listNeuron.size();i++)
            {
                NeuronSWC s = nt.listNeuron[i];
                s.radius=1+norder[i]*0.2;
                nt_out.listNeuron.append(s);
                nt_out.hashNeuron.insert(s.n,nt_out.listNeuron.size()-1);
            }
            writeSWC_file(QString::fromStdString(out_swc_file),nt_out);
        }
	}
    else if (func_name == tr("GetNodeSubtree"))
    {
        /* designed by shengdian, 2021-06-15
         * ---
         * Get subtree of one node.
         * --Usage--
         * input: swc or eswc file
         * output: processed swc file
        */
        string inswc_file;
        if(infiles.size()>=1) {inswc_file = infiles[0];}
        else {  printHelp(); return false;}
        long nid=(inparas.size()>=1)?atoi(inparas[0]):1;
        int to_file=(inparas.size()>=2)?atoi(inparas[1]):1;
        string out_swc_file=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_subtree.swc");
        //read swc
        NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
        NeuronTree nt_out;nt_out.listNeuron.clear();nt_out.hashNeuron.clear();
        nt_out=getSubtree(nt,nid);
        nt_out=reindexNT(nt_out);
        if(to_file)
            writeSWC_file(QString::fromStdString(out_swc_file),nt_out);
    }
    else if (func_name == tr("GetSubtreeLength"))
    {
        /* designed by shengdian, 2021-06-15
         * ---
         * Get subtree length of one node.
         * --Usage--
         * input: swc or eswc file
         * output: processed swc file
        */
        string inswc_file;
        if(infiles.size()>=1) {inswc_file = infiles[0];}
        else {  printHelp(); return false;}
        long nid=(inparas.size()>=1)?atoi(inparas[0]):0;
        long normalized_max_size=(inparas.size()>=2)?atoi(inparas[1]):0;
        int to_file=(inparas.size()>=3)?atoi(inparas[2]):1;
        string out_swc_file=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_subtree_with_length.swc");
        //read swc
        NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
        if(!nid) //process to all
        {
            vector<double> n_subtree_len(nt.listNeuron.size(),0.0);
            n_subtree_len=get_node_subtree_len_v1(nt,normalized_max_size);
            NeuronTree nt_out;nt_out.listNeuron.clear();nt_out.hashNeuron.clear();
            for(V3DLONG i=0;i<nt.listNeuron.size();i++)
            {
                NeuronSWC s=nt.listNeuron[i];
                s.radius=n_subtree_len[i];
                nt_out.listNeuron.append(s);
                nt_out.hashNeuron.insert(s.n,nt_out.listNeuron.size()-1);
            }
            if(to_file)
                writeSWC_file(QString::fromStdString(out_swc_file),nt_out);
        }
        else
        {
            //process to one node
            double n_subtree_len=0.0;
            NeuronTree nt_subtree;nt_subtree.listNeuron.clear();nt_subtree.hashNeuron.clear();
            nt_subtree=getSubtree(nt,nid);
            nt_subtree=reindexNT(nt_subtree);
            //get length of a neuron tree
            n_subtree_len=get_nt_len(nt_subtree);
            cout<<"Node: "<<nid<<", subtree_len= "<<n_subtree_len<<endl;
        }
    }
    else if (func_name == tr("Pruning_SubTree"))
    {
        /* designed by shengdian, 2021-06-15
         * ---
         * pruning the node with small subtree in its length
         * --Usage--
         * input: swc or eswc file
         * output: processed swc file
        */
        string inswc_file;
        if(infiles.size()>=1) {inswc_file = infiles[0];}
        else {  printHelp(); return false;}
        int pruning_thre=(inparas.size()>=1)?atoi(inparas[0]):100;
        int to_file=(inparas.size()>=2)?atoi(inparas[1]):1;
        string out_swc_file=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_spruning.swc");
        //read swc
        NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
        NeuronTree nt_out;nt_out.listNeuron.clear();nt_out.hashNeuron.clear();
        nt_out=pruning_subtree(nt,pruning_thre);
        if(to_file)
            writeSWC_file(QString::fromStdString(out_swc_file),nt_out);
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
    qDebug()<<"               -----Usage_basic: function for analysis of branch----              ";
    qDebug()<<"                                               <libname>:UtilityArbor                                ";
    qDebug()<<"1. vaa3d -x <libname> -f GetNodeOrder -i <input_swc> -p <to_file> -o <out_file_path>";
    qDebug()<<"2. vaa3d -x <libname> -f GetNodeType -i <input_swc> -p <to_file> -o <out_file_path>";
    qDebug()<<"3. vaa3d -x <libname> -f GetNodeSubtree -i <input_swc> -p <node_index> <to_file> -o <out_file_path>";
    qDebug()<<"4. vaa3d -x <libname> -f GetSubtreeLength -i <input_swc> -p <node_index> <normalized_size> <to_file> -o <out_file_path>";
    qDebug()<<"5. vaa3d -x <libname> -f GetNodeTips -i <input_swc> -p <to_file> -o <out_file_path>";
    qDebug()<<"6. vaa3d -x <libname> -f Pruning_SubTree -i <input_swc> -p <pruning_length_thre> <to_file> -o <out_file_path>";
}
