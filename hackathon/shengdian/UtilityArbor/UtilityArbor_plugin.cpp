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
            <<tr("GetArbor")
          <<tr("GetNodeSubtree")
         <<tr("GetSubtreeLength")
       <<tr("Pruning_SubTree")
      <<tr("GetMainPath")
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

    if (func_name == tr("GetNodeSubtree"))
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
        NeuronTree nt_out;
        nt_out=getSubtree(nt,nid);
        nt_out=reindexNT(nt_out);
        if(to_file)
            writeSWC_file(QString::fromStdString(out_swc_file),nt_out);
    }
    else if(func_name==tr("GetArbor"))
    {
        /*get arbor of a neuron:
         * arbor_type=2 for axonal arbor
         * arbor_type=3 for type3 dendrite arbor
         * arbor_type=4 for apical dendrite arbor
         * arbor_type=5 for type3 and type4 dendrite arbor
         * arbor_type=6 for distal axonal arbor
         * arbor_type=7 for local axonal arbor
        */
        string inswc_file;
        if(infiles.size()>=1) {inswc_file = infiles[0];}
        else {  printHelp(); return false;}

        int arbor_type=(inparas.size()>=1)?atoi(inparas[0]):1;
        NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
        NeuronTree nt_out;
        if(arbor_type>=2&&arbor_type<=5)
            get_arbor(nt,nt_out,arbor_type);
        else if(arbor_type==1){
            NeuronTree nt_axon,nt_den,nt_apicalden,nt_wholeden;
            string out_axon=inswc_file + "_axon.eswc";
            get_arbor(nt,nt_axon,2);
            if(nt_axon.listNeuron.size()>0)
                writeESWC_file(QString::fromStdString(out_axon),nt_axon);
            else
                cout<<"Axonal arbor is not exist in file"<<endl;
            string out_den=inswc_file + "_dendrite.eswc";
            get_arbor(nt,nt_den,3);
            if(nt_den.listNeuron.size()>0)
                writeESWC_file(QString::fromStdString(out_den),nt_den);
            else
                cout<<"Dendritic arbor is not exist in file"<<endl;
            string out_apical=inswc_file + "_apical.eswc";
            get_arbor(nt,nt_apicalden,4);
            if(nt_apicalden.listNeuron.size()>0)
                writeESWC_file(QString::fromStdString(out_apical),nt_apicalden);
            else
                cout<<"Apical dendritic arbor is not exist in file"<<endl;
            string out_all_den=inswc_file + "_alldendrite.eswc";
            get_arbor(nt,nt_wholeden,5);
            if(nt_wholeden.listNeuron.size()>0)
                writeESWC_file(QString::fromStdString(out_all_den),nt_wholeden);
            else
                cout<<"Apical dendritic  arbor is not exist in file"<<endl;
            return true;
        }
        else if(arbor_type==6||arbor_type==7){
            cout<<"in getting the cluster arbor of axon"<<endl;
            float r_thre=(inparas.size()>=2)?atof(inparas[1]):0.6;
            get_axonarbor(nt,nt_out,arbor_type,r_thre);
            string out_swc_file=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_distal.eswc");
            if(arbor_type==7)
                out_swc_file=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_local.eswc");
            if(nt_out.listNeuron.size()>0)
                writeESWC_file(QString::fromStdString(out_swc_file),nt_out);
            else
                cout<<"Request axonal arbor is not exist in file"<<endl;
            return true;
        }
        else
            return false;
        string out_swc_file;
        switch (arbor_type) {
        case 2:
            out_swc_file=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_axon.eswc");
            break;
        case 3:
            out_swc_file=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_dendrite.eswc");
            break;
        case 4:
            out_swc_file=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_apical.eswc");
            break;
        case 5:
            out_swc_file=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_alldendrite.eswc");
            break;
        default:
            break;
        }
        if(nt_out.listNeuron.size()>0)
            writeESWC_file(QString::fromStdString(out_swc_file),nt_out);
        else
            cout<<"Request arbor is not exist with type="<<arbor_type<<endl;
    }
    else if (func_name == tr("GetMainPath"))
    {
        /* designed by shengdian, 2021-12-02 */
        string inswc_file;
        if(infiles.size()>=1) {inswc_file = infiles[0];}
        else {  printHelp(); return false;}
        string out_swc_file=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_mp.eswc");
        float len_thre=(inparas.size()>=1)?atof(inparas[0]):0.5;
        float tip_thre=(inparas.size()>=2)?atof(inparas[1]):0.2;
        //read swc
        NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
        NeuronTree nt_out;
        if(axon_main_path(nt,nt_out,len_thre,tip_thre)){
            cout<<"main path nodes="<<nt_out.listNeuron.size()<<endl;
            nt_out=reindexNT(nt_out);
            writeESWC_file(QString::fromStdString(out_swc_file),nt_out);
        }
        else
        {
            cout<<"Main path is not found"<<endl;
            return false;
        }
        return true;
    }
    else if (func_name == tr("GetCluster"))
    {
        /* designed by shengdian, 2021-11-28 */
        string inswc_file;
        if(infiles.size()>=1) {inswc_file = infiles[0];}
        else {  printHelp(); return false;}
        string out_swc_file=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_subtree.eswc");
        float r_thre=(inparas.size()>=1)?atof(inparas[0]):0.6;
        float order_thre=(inparas.size()>=2)?atof(inparas[1]):0.0;
        //read swc
        NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
        NeuronTree nt_out;
        float max_r=0; float max_level=0;
        for(V3DLONG i=0;i<nt.listNeuron.size();i++){
            max_r=(nt.listNeuron.at(i).r>max_r)?nt.listNeuron.at(i).r:max_r;
            max_level=(nt.listNeuron.at(i).level>max_level)?nt.listNeuron.at(i).level:max_level;
        }
        vector<int> ntype(nt.listNeuron.size(),0);
        getNodeType(nt,ntype);

        //all the candicate nodes
        cout<<"radius thre="<<r_thre*max_r<<endl;
        cout<<"order thre="<<order_thre*max_level<<endl;
        V3DLONG nid_index=0;
        QList<V3DLONG> rnids;
        float loop_times=0.0;
        while(rnids.size()==0){
            float order_step_up=0.02;
            for(V3DLONG i=0;i<nt.listNeuron.size();i++)
            {
                NeuronSWC s=nt.listNeuron.at(i);
                if(s.type==2
                        &&s.r>=(r_thre-loop_times*0.05)*max_r
                        &&s.level>=(order_thre-loop_times*order_step_up)*max_level
                       &&ntype.at(i)>=2)
                    rnids.append(i);
            }
            loop_times++;
        }
        cout<<"candicated nodes="<<rnids.size()<<endl;
        nid_index=rnids.at(0);
        //find the minimum radius
        for(V3DLONG i=0;i<rnids.size();i++)
            if(nt.listNeuron.at(rnids.at(i)).r<nt.listNeuron.at(nid_index).r)
                nid_index=rnids.at(i);
        cout<<"subtree index="<<nid_index<<endl;
        nt_out=getSubtree(nt,nt.listNeuron.at(nid_index).n);
        nt_out=reindexNT(nt_out);
        //rescale
        for(V3DLONG i=0;i<nt_out.listNeuron.size();i++)
            nt_out.listNeuron[i].r=1;

        writeESWC_file(QString::fromStdString(out_swc_file),nt_out);
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
        float axon_ratio=(inparas.size()>=3)?atof(inparas[2]):1.0;
        float other_ratio=(inparas.size()>=4)?atof(inparas[3]):0.0;
        string out_swc_file=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_subtree_with_length.eswc");
        //read swc
        NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
        if(!nid) //process to all
        {
            vector<double> n_subtree_len(nt.listNeuron.size(),0.0);
            n_subtree_len=get_node_subtree_len_v1(nt,normalized_max_size,axon_ratio,other_ratio);
            vector<int> norder(nt.listNeuron.size(),0);
            getNodeOrder(nt,norder);
            NeuronTree nt_out;
            for(V3DLONG i=0;i<nt.listNeuron.size();i++)
            {
                NeuronSWC s=nt.listNeuron[i];
                s.radius=n_subtree_len[i];
                s.level=norder.at(i);
                nt_out.listNeuron.append(s);
                nt_out.hashNeuron.insert(s.n,nt_out.listNeuron.size()-1);
            }
            writeESWC_file(QString::fromStdString(out_swc_file),nt_out);
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
        float pruning_thre=(inparas.size()>=1)?atof(inparas[0]):0.1;
        int pruning_times=(inparas.size()>=1)?atoi(inparas[1]):2;
        string out_swc_file=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_spruning.eswc");
        //read swc
        NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));

        NeuronTree nt_iter,nt_out;
        int this_p=0;
        while(this_p<pruning_times){
            if(pruning_subtree(nt,nt_iter,pruning_thre)){
                nt.listNeuron.clear();
                nt.hashNeuron.clear();
                nt.deepCopy(nt_iter);
            }
            else
                break;
            this_p++;
        }
        if(this_p>=1)
            nt_out.deepCopy(nt);
        else
            nt_out.deepCopy(nt_iter);
        if(nt_out.listNeuron.size()>0)
            writeESWC_file(QString::fromStdString(out_swc_file),nt_out);
        return true;
    }
    else if (func_name == tr("Arbor_topo"))
    {
        /* designed by shengdian, 2021-12-02 */
        string inswc_file;
        if(infiles.size()>=1) {inswc_file = infiles[0];}
        else {  printHelp(); return false;}
        string out_swc_file=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_topo.eswc");
        //read swc
        NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
        NeuronTree nt_out;
        if(arbor_topo(nt,nt_out)){
            nt_out=reindexNT(nt_out);
            writeESWC_file(QString::fromStdString(out_swc_file),nt_out);
        }
        else
        {
            cout<<"Fail to convert to topo tree"<<endl;
            return false;
        }
        return true;
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
    qDebug()<<"3. vaa3d -x <libname> -f GetNodeSubtree -i <input_swc> -p <node_index> <to_file> -o <out_file_path>";
    qDebug()<<"4. vaa3d -x <libname> -f GetSubtreeLength -i <input_swc> -p <node_index> <normalized_size> <to_file> -o <out_file_path>";
    qDebug()<<"6. vaa3d -x <libname> -f Pruning_SubTree -i <input_swc> -p <pruning_length_thre> <to_file> -o <out_file_path>";
}
