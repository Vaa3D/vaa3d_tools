#include "region_match_plugin.h"
#include "make_consensus.h"
#include "neuron_utilities/sort_swc.h"
#include <stdio.h>
#define VOID 1000000000
#define min(a,b) (a)<(b)?(a):(b)
#define max(a,b) (a)>(b)?(a):(b)

struct Boundary
{
    double minx;
    double miny;
    double minz;
    double maxx;
    double maxy;
    double maxz;
};

void make_consensus(const NeuronTree & nt, NeuronTree & pattern, NeuronTree & mk, V3DPluginCallback2 &callback)
{
    V3DLONG pattern_size = pattern.listNeuron.size();
    V3DLONG nt_size = nt.listNeuron.size();
    vector<Boundary> v_boundary;
    //vector<NeuronTree> trees();
    if(pattern_size==0 || nt_size==0)
    {
        cout<<"no input"<<endl;
        return;
    }
    cout<<"size="<<pattern_size<<endl;
    Boundary temp;
    for(V3DLONG i=0;i<pattern_size;i++)
    {

       if(pattern.listNeuron[i].pn<0)
       {
            temp.minx=VOID; temp.miny=VOID;temp.minz=VOID;
            temp.maxx=0;temp.maxy=0;temp.maxz=0;
        }
       temp.minx=min(pattern.listNeuron[i].x,temp.minx);
       temp.miny=min(pattern.listNeuron[i].y,temp.miny);
       temp.minz=min(pattern.listNeuron[i].z,temp.minz);
       temp.maxx=max(pattern.listNeuron[i].x,temp.maxx);
       temp.maxy=max(pattern.listNeuron[i].y,temp.maxy);
       temp.maxz=max(pattern.listNeuron[i].z,temp.maxz);

       if(i==pattern_size-2)
       {
           temp.minx=min(pattern.listNeuron[i+1].x,temp.minx);
           temp.miny=min(pattern.listNeuron[i+1].y,temp.miny);
           temp.minz=min(pattern.listNeuron[i+1].z,temp.minz);
           temp.maxx=max(pattern.listNeuron[i+1].x,temp.maxx);
           temp.maxy=max(pattern.listNeuron[i+1].y,temp.maxy);
           temp.maxz=max(pattern.listNeuron[i+1].z,temp.maxz);
           v_boundary.push_back(temp);
           i++;
       }
       else if(pattern.listNeuron[i+1].pn==-1)
       {
           v_boundary.push_back(temp);
       }
    }
    vector<NeuronTree> trees(v_boundary.size());

   for(V3DLONG i=0;i<nt.listNeuron.size();i++)
   {
       NeuronSWC curr = nt.listNeuron[i];
       for(int j=0; j<v_boundary.size();j++)
       {
           Boundary b=v_boundary[j];
           if(curr.x>b.minx && curr.y>b.miny && curr.z>b.minz &&curr.x<b.maxx && curr.y<b.maxy&&curr.z<b.maxz)
           {
               trees[j].listNeuron.push_back(curr);
               break;
           }
       }
   }
   vector<QString> file_list;
   vector<QString> file_list_bn;
   for(int i=0; i<trees.size();i++)
   {
       NeuronTree sorted_tree;
       if(!SortSWC(trees[i].listNeuron,sorted_tree.listNeuron,VOID,3)) return;
       char buf[10];
       string num_i;
       sprintf(buf, "%d", i+1);
       num_i = buf;
       QString savename = "pattern_" + QString::fromStdString(num_i) + ".swc";
       QString savename_bn = "pattern_bn_" + QString::fromStdString(num_i) + ".swc";
       file_list.push_back(savename);
       file_list_bn.push_back(savename_bn);
       writeSWC_file(savename,sorted_tree);
   }

   V3DPluginArgItem arg;
   V3DPluginArgList input_bn;
   V3DPluginArgList output_bn;

   // call blastneuron for pre_processing
   for(int i=0; i<trees.size();i++)
   {
      input_bn.clear();
       QString tree_file=file_list[i];
       QString out_file=file_list_bn[i];
       QString para_nt="#i "+tree_file +" #o "+ out_file +" #l 1 #s 1 #t 2 #r 0";
       cout<<para_nt.toStdString().c_str()<<endl;

       arg.type="random";vector<char*> arg_input_bn;
       string fileName_Qstring(para_nt.toStdString());char* fileName_string = new char[fileName_Qstring.length() +1]; strcpy(fileName_string, fileName_Qstring.c_str());

       arg_input_bn.push_back(fileName_string);

       arg.p = (void*) & arg_input_bn; input_bn<< arg;
       arg.type="random";vector<char*> arg_bn_para; arg_bn_para.push_back(fileName_string);arg.p = (void *) & arg_bn_para; input_bn << arg;

       QString plugin_name_bn = "blastneuron/libblastneuron"; //Need change
       QString func_name_bn = "pre_processing";
       callback.callPluginFunc(plugin_name_bn,func_name_bn,input_bn,output_bn);
       arg_input_bn.clear();

   }

    // call consensus
   cout<<"Example: v3d -x consensus_swc -f consensus_swc -i myfolder/*.swc -o consensus.eswc -p 3 5 0\n"<<endl;


//       V3DPluginArgItem arg2;
       V3DPluginArgList input_consensus;
       V3DPluginArgList output_consensus;
       QString consensus_result = "consensus.swc";
       arg.type = "random";vector<char*> arg_input_consensus;
       for(int i=0;i<file_list_bn.size();i++)
       {
           QString files_name = file_list_bn[i];
           string fileName_Qstring(files_name.toStdString());char* fileName_string =  new char[fileName_Qstring.length() + 1]; strcpy(fileName_string, fileName_Qstring.c_str());
           arg_input_consensus.push_back(fileName_string);
       }
       arg.p = (void *) & arg_input_consensus;
       input_consensus<< arg;

       arg.type = "random";vector<char*> arg_consensus_para; arg_consensus_para.push_back("3");arg_consensus_para.push_back("5");arg_consensus_para.push_back("0");
       arg.p = (void *) & arg_consensus_para; input_consensus << arg;

       arg.type = "random";vector<char*> arg_output;
       string fileName_Qstring2(consensus_result.toStdString());char* fileName_string2 =  new char[fileName_Qstring2.length() + 1]; strcpy(fileName_string2, fileName_Qstring2.c_str());
       arg_output.push_back(fileName_string2); arg.p = (void *) & arg_output; output_consensus<< arg;

       QString full_plugin_name_consensus = "consensus_swc";
       QString func_name_consensus = "consensus_swc";
       callback.callPluginFunc(full_plugin_name_consensus,func_name_consensus,input_consensus,output_consensus);

       mk = readSWC_file(consensus_result);


}
