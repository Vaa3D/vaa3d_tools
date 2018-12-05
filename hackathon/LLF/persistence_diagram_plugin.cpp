/* persistence_diagram_plugin.cpp
 * The main file of plugin persistence_diagram
 * 2018-11-27 by Longfei Li.
 */

#include "persistence_diagram_plugin.h"
#include <QtGlobal>

Q_EXPORT_PLUGIN2(persistence_diagram, PersistenceDiagramPlugin);


QStringList PersistenceDiagramPlugin::menulist() const
{
	return QStringList()
		<< tr("generate_persistence_diagram")
		<< tr("about");
}

QStringList PersistenceDiagramPlugin::funclist() const
{
	return QStringList()
		<< tr("help");
}

void PersistenceDiagramPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("generate_persistence_diagram"))
	{
		generate_persistence_diagram();
	}
	else if (menu_name == tr("about"))
	{
		v3d_msg(tr("PersistenceDiagram by Longfei Li!"));
	}
}

bool PersistenceDiagramPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback, QWidget * parent)
{
	if (func_name == tr("help"))
	{
		printHelp();
		return true;
	}
	return false;
}

void generate_persistence_diagram()
{
	QStringList filenames = QFileDialog::getOpenFileNames(0, 0, "", "Supported file (*.swc)" ";;Neuron structure(*.swc)", 0, 0);

	if (filenames.isEmpty())
	{
		v3d_msg("You don't open any SWC file!");
		return;
	}

	if(filenames.size()!=1)
	{
		v3d_msg("You've select too much swc files!");
		return;
	}

	BarCode barcode;

	NeuronTree neuron = readSWC_file(filenames[0]);
	V3DLONG neuron_siz = neuron.listNeuron.size();

	vector<V3DLONG> branches(neuron_siz,0);
	calculate_branches(neuron,branches);
	set<V3DLONG> leaf_node_set;
	set<V3DLONG> branch_node_set;
	//find leaf nodes and branche nodes;
	for (V3DLONG i = 0; i < neuron_siz; i++)
	{
		if(branches[i]==0)
			leaf_node_set.insert(i);
		else if(branches[i]==2)
			branch_node_set.insert(i);
	}
	cout<<"Find leaf nodes finish.The leaf node number is "<<leaf_node_set.size()<<endl;
	cout<<"Find branch nodes finish.The branch node number is "<<branch_node_set.size()<<endl;

	unordered_map<V3DLONG, VALUETYPE> values;//record values at each leaf or branch node;
	map<V3DLONG,V3DLONG> max_son_map;//record parent node's child node num which with max value;
	//calculate values at nodes in leaf set and branch set;
	for(auto it = leaf_node_set.begin();it!=leaf_node_set.end();it++)
	{
		V3DLONG current = *it;
		V3DLONG parent = current;
		values[current] = calculate_feature_at_node(neuron,current);//initialize the values by calculating features at each leaf node first;
		while (current!=0)//from one node to the root;
		{
			for (;branches[parent]<=1 && parent!=0;parent = neuron.hashNeuron.value(neuron.listNeuron[parent].pn));//got a branch
			if (values.find(parent)==values.end()||values[current]>=values[parent])
			{
				values[parent]=values[current];
				max_son_map[parent] = current;
				cout<<"Update node no."<<parent<<" max son branch node:"<<current<<" value:"<<values[parent]<<endl;
			}
			current = parent;
			parent = neuron.hashNeuron.value(neuron.listNeuron[parent].pn);
		}
	}

	set<V3DLONG> active_set(leaf_node_set);//use leaf nodes initializing active points set;
	set<V3DLONG> parent_set;//temporary put the parent nodes;
	set<V3DLONG> count_set(leaf_node_set);//count if all nodes actived;
	V3DLONG set_siz = leaf_node_set.size()+branch_node_set.size();
	while (count_set.size()!=set_siz)
	{
		cout<<"Enter loop."<<endl;
		cout<<count_set.size()<<" : "<<set_siz<<endl;
		parent_set.clear();
		for(auto it=active_set.begin();it!=active_set.end();it++)
		{
			if (*it==0)//for root;
			{
				continue;
			}
			V3DLONG parent_id;//get current node's parent node;

			for (parent_id = neuron.hashNeuron.value(neuron.listNeuron[*it].pn);branches[parent_id]<=1 && parent_id!=0;parent_id = neuron.hashNeuron.value(neuron.listNeuron[parent_id].pn));
			if(max_son_map[parent_id] != *it)
			{
				barcode_add(barcode,values[*it],calculate_feature_at_node(neuron,parent_id));
				parent_set.insert(parent_id);//HOW TO DEFINE ONE NODE IS ACTIVED???
			}
		}
		active_set.clear();
		active_set.insert(parent_set.begin(),parent_set.end());
		count_set.insert(parent_set.begin(),parent_set.end());
	}
	//add root barcode
	barcode_add(barcode,values[0],calculate_feature_at_node(neuron,0));

	QString neuron_path = neuron.file;
	QString output_path = neuron_path+".bcd";
	//save barcode
	barcode_save(output_path, neuron_path, barcode);
	construct_PD_from_barcode(barcode);
	/*for (int i=0;i<barcode.size();i++)
	{
		cout<<barcode[i].birth<<"---"<<barcode[i].death<<endl;
	}*/
}

void construct_PD_from_barcode(BarCode barcode)
{

}

void printHelp() 
{
	cout << "We are in dofunc->printHelp() now." << endl;

	return;
}

void calculate_branches(NeuronTree neuron, vector<V3DLONG> &branches)
{
	V3DLONG siz = neuron.listNeuron.size();
	for (V3DLONG i = 0; i < siz; i++)
	{
		if (neuron.listNeuron[i].pn < 0) continue;//root
		V3DLONG pid = neuron.hashNeuron.value(neuron.listNeuron[i].pn);//hash:node no.to list no.
		branches[pid]++;
	}
}

void find_leaf_nodes(NeuronTree neuron,set<V3DLONG> &leaf_node_set)
{
	V3DLONG siz = neuron.listNeuron.size();
	vector<V3DLONG> branches(siz,0); 
	for (V3DLONG i = 0; i < siz; i++)
	{
		if (neuron.listNeuron[i].pn < 0) continue;//root
		V3DLONG pid = neuron.hashNeuron.value(neuron.listNeuron[i].pn);//hash:node no.to list no.
		branches[pid]++;
	}

	for (V3DLONG i = 0; i < siz; i++)
	{
		if(branches[i]==0)
			leaf_node_set.insert(i);
	}
}

double calculate_feature_at_node(NeuronTree neuron,V3DLONG node_id)
{
	VALUETYPE v = DIST(neuron.listNeuron[node_id],neuron.listNeuron[0]);
	cout<<"Calculated feature at node "<<node_id<<" : "<<v<<endl;
	//the feature we're calculating here is radial distance from root(like defined in paper);
	return v;
}

void barcode_add(BarCode &barcode,VALUETYPE value_begin,VALUETYPE value_end)
{
	LifeTime lf;
	if(value_begin<=value_end)
	{
		lf.birth=value_begin;
		lf.death=value_end;
	}
	else
	{
		lf.death=value_begin;
		lf.birth=value_end;
	}
	cout<<"Add barcode : "<<lf.birth<<"---"<<lf.death<<endl;
	barcode.push_back(lf);
}

void barcode_save(QString output_path,QString neuron_path, BarCode barcode)
{
	cout<<"Save barcode begin"<<endl;
	FILE * fp;
	fp = fopen(output_path.toLatin1(), "w");
	cout<<"Open file :"<<qPrintable(output_path)<<endl;
	if (fp==NULL)
	{
		cout<<"ERROR! can't open file "<<qPrintable(output_path)<<endl;
	}
	fprintf(fp,"#neuron:      %s\n",qPrintable(neuron_path));
	fprintf(fp,"#barcode size:          %d\n",barcode.size());
	fprintf(fp,"#birth\tdeath\n");

	for (int i=0;i<barcode.size();i++)
	{
		fprintf(fp,"%lf",barcode[i].birth);
		fprintf(fp,"\t");
		fprintf(fp,"%lf",barcode[i].death);
		fprintf(fp,"\n");
	}
	cout<<"Barcode saved at "<<qPrintable(output_path)<<endl;
	fclose(fp);
}