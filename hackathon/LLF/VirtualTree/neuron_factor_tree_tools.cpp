/*neuron_factor_tree_tools.cpp
 *funcs for neuron_factor_tree struct.
 */
#include "neuron_factor_tree_plugin.h"
#include <QtGlobal>

/////////////////////////////////////////////////////////////////////////////////
// Helper functions for branch count.

void calculate_branches(const NeuronTree &neuron, vector<V3DLONG> &branches)
{
	//calculate a node's branch num;
	V3DLONG neuron_siz = neuron.listNeuron.size();
	branches = vector<V3DLONG>(neuron_siz,0);

	for (V3DLONG i = 0; i < neuron_siz; i++)
	{
		if (neuron.listNeuron[i].pn < 0) continue;//root
		V3DLONG pid = neuron.hashNeuron.value(neuron.listNeuron[i].pn);//hash:node no.to list no.
		branches[pid]++;
	}
	cout<<"Calculate branches at "<<neuron_siz<<" nodes"<<endl;

}

void get_leaf_branch_nodes(const NeuronTree &neuron, vector<V3DLONG> branches, set<V3DLONG> &leaf_node_set, set<V3DLONG> &branch_node_set)
{
	V3DLONG neuron_siz = neuron.listNeuron.size();
	//find leaf nodes and branche nodes;
	for (V3DLONG i = 0; i < neuron_siz; i++)
	{
		if(branches[i]==0)
			leaf_node_set.insert(i);
		else if(branches[i]>1)
			branch_node_set.insert(i);
	}
	if (!branch_node_set.count(0)) branch_node_set.insert(0);//if root have only one branch, still insert it in the nft_node_set;

	cout<<"Find leaf nodes finish.The number of leaf nodes is "<<leaf_node_set.size()<<endl;
	cout<<"Find branch nodes finish.The number branch nodes is "<<branch_node_set.size()<<endl;

}

void get_nft_node_relation_map(const NeuronTree &neuron, vector<V3DLONG> branches, set<V3DLONG> nft_node_set, map<V3DLONG,V3DLONG> &nft_parent_map, map<V3DLONG,V3DLONG> &nft_child_map, map<V3DLONG, V3DLONG> &nft_sibling_map)
{
	//Initializae map;
	for(auto it = nft_node_set.begin(); it!=nft_node_set.end(); it++)
	{
		nft_child_map[*it] = -1;
		nft_parent_map[*it] = -1;
		nft_sibling_map[*it] = -1;
	}
	//renew values
	for(auto it = nft_node_set.begin(); it!=nft_node_set.end(); it++)
	{
		V3DLONG current = *it;
		//V3DLONG parent = neuron.hashNeuron.value(neuron.listNeuron[parent].pn);
		if(current==0) continue;//for nft root don't have a parent;
		V3DLONG parent = neuron.hashNeuron.value(neuron.listNeuron[current].pn);
		for (;parent!=0 && branches[parent]<=1;parent = neuron.hashNeuron.value(neuron.listNeuron[parent].pn));//go to a parent node;
		//if(parent==0) continue;//
		nft_parent_map[current] = parent;
		if(nft_child_map[parent] != -1)
		{
			auto sib = nft_child_map[parent];
			while (nft_sibling_map[sib] != -1) sib = nft_sibling_map[sib];//this could collect all the children node in siblings;
			nft_sibling_map[sib] = current;//Add current to the list's tail;
		}
		else
			nft_child_map[parent] = current;
	}

	cout<<"Get a nft relation map for nft nodes"<<endl;
}

//create a neuron_factor_treee from a neuron_tree;
NeuronFactorTree create_neuron_factor_tree(const NeuronTree &neuron, bool is_calculate_feature)
{
	vector<V3DLONG> branches;
	calculate_branches(neuron, branches);

	set<V3DLONG> leaf_node_set;
	set<V3DLONG> branch_node_set;
	get_leaf_branch_nodes(neuron, branches, leaf_node_set, branch_node_set);//find leaf nodes and branch nodes;

	set<V3DLONG> nft_node_set = branch_node_set;
	nft_node_set.insert(leaf_node_set.begin(), leaf_node_set.end());//use leaf nodes and branch nodes as nft nodes;

	map<V3DLONG, V3DLONG> nft_parent_map;//record each nft nodes' parent node;
	map<V3DLONG, V3DLONG> nft_child_map;//record each nft nodes' child node;
	map<V3DLONG, V3DLONG> nft_sibling_map;
	get_nft_node_relation_map(neuron, branches, nft_node_set, nft_parent_map, nft_child_map, nft_sibling_map);

	NeuronFactors nfs;//neuron factors map;
	NeuronFactor nft_root;//record tree root;

	vector<int> marked(neuron.listNeuron.size()+1, 0);//record how many child node is visited;
	queue<V3DLONG> q;
	for each (auto l in leaf_node_set)
	{
		q.push(l);
	}

	while (!q.empty()) {
		auto current = q.front();
		V3DLONG pa, ch, ch_sb, sib;
		
		pa = nft_parent_map.at(current);
		sib = nft_sibling_map.at(current);
		ch = nft_child_map.at(current);
		ch_sb = (ch != -1 ? nft_sibling_map.at(ch) : -1);
		NeuronFactor nf_current(1, neuron.listNeuron[current].type, current, neuron.listNeuron[current].x, neuron.listNeuron[current].y, neuron.listNeuron[current].z, pa, ch, ch_sb, sib);

		//sib = (nft_child_map.at(pa) == current ? nft_sibling_map.at(current) : nft_child_map.at(pa));
		if (is_calculate_feature && branches[current] != 0 && pa!=-1)//for leaf don,t calculate features;
		{
			sib = (nft_child_map.at(pa) == current ? nft_sibling_map.at(current) : nft_child_map.at(pa));
			//for feature caluculation, make sure sib is exsitted(SIB.sib = current, current.sib = -1 => curent.sib = SIB);
			calculate_features_at_node(neuron, nf_current, ch, ch_sb, sib, pa, nfs);//don,t calculate features at 0;
		}
		nfs[current] = nf_current;
		if (nft_parent_map[current] == -1) nft_root = nf_current;//the nft_root may NOT BE the neuron root(branches[root]==1), let the first branch be nft_root;

		if (pa != -1)
		{
			marked[pa] = marked[pa] + 1;
			if (marked[pa] == 2) q.push(pa);//if one node's two children is visited, means feature could calculated from it, enqueue;
		}

		q.pop();
	}
	//if (nft_root.label == -1) nft_root = NeuronFactor(1, 0, neuron.listNeuron[0].x, neuron.listNeuron[0].y, neuron.listNeuron[0].z, -1, nft_child_map[0], -1);//let root be nft_root;
	cout << "Get NeuronFactor Tree root at nft_node:" << nft_root.label << "." << endl;
	NeuronFactorTree nft = NeuronFactorTree(1, neuron.file.toStdString(), nft_root, nfs);
	cout << "NeuronFactor Tree constructed." << endl;

	return nft;
}

void save_neuron_factor_tree(const NeuronFactorTree nft, const QString path)
{
	/*Save nft
	 *saves XYZ coordinates for visualization
	 *folleowed by features in comments format
	 *so that nft could shown in vaa3d.
	 */
	cout<<"Saving NeuronFactor Tree."<<endl;
	FILE * fp;
	fp = fopen(path.toLatin1(), "w");
	cout<<"Open file :"<<qPrintable(path)<<endl;
	if (fp==NULL)
	{
		cout<<"ERROR! Can't open file "<<qPrintable(path)<<endl;
	}
	cout<<"neuron factor num is :"<<nft.neuron_factors.size()<<endl;
	//header
	fprintf(fp, "#neuronFactorTree.nft format, nft node start with id 0;\n"
				 "#to get the node id in neuronTree, use neuron.listNeuron[node];\n"
		         "#level:the level neuronfactortree is from;\n"
		         "#neuron path:the absolute path of swc file;\n"
				 "#nf info:V3DLONG id;label for visualization color(2 for red);float x, y, z;radius as 0.05;\n"
				 "#V3DLONG parent_id,child_id,child_sibling_id,sibling_id(-1 for don't exist);\n"
				 "#nf feature values:For root node and leaf node DONOT have any values;\n");
	//feature names
	fprintf(fp, "#FEATURES: fea_dist_c1 fea_dist_c2 fea_dist_soma fea_bif_angle_remote fea_path_len_1 fea_contraction_1 fea_diameter_avg_1 fea_area_avg_1 fea_surface_1 fea_volume_1 "
		"fea_path_len_2 fea_contraction_2 fea_diameter_avg_2 fea_area_avg_2 fea_surface_2 fea_volume_2 fea_bif_angle_local"
		/*" fea_tilt_angle_remote fea_tilt_angle_local fea_torque_remote fea_torque_local "
		"fea_child_ratio"*/
		" fea_direction_x fea_direction_y fea_direction_z fea_tips_num fea_tips_asymmetry\n");//define feature names;
	//save level
	fprintf(fp, "#LEVEL: %d\n", nft.level);
	//save swc path
	fprintf(fp, "#NEURONPATH: ");
	fprintf(fp, nft.neuron_path.c_str());
	fprintf(fp, "\n");
	//save the xyz coordinates in swc format, so that file could display in vaa3d; 
	for (auto it = nft.neuron_factors.begin(); it!=nft.neuron_factors.end();it++)
	{
		//cout<<it->second.label<<it->second.x<< it->second.y<<it->second.z<<it->second.parent<<endl;
		fprintf(fp, "%ld %d %5.3f %5.3f %5.3f %5.3f %ld %ld %ld %ld\n",
			it->second.label, it->second.type, it->second.x, it->second.y, it->second.z, 0.05, 
			it->second.parent, it->second.child, it->second.child_sibling, it->second.sibling);
			//it->second.label, it->second.x, it->second.y, it->second.z, it->second.parent, it->second.child, it->second.sibling);
		//print features
		fprintf(fp, "#VALUES:");
		for(int i=0; i<it->second.feature_vector.size(); i++)
		{
			fprintf(fp, " %5.3f", it->second.feature_vector[i]);
		}

		fprintf(fp, "\n");

	}
	cout<<"NeuronFactor Tree saved at path:"<<qPrintable(path)<<endl;
	fclose(fp);
}

NeuronFactorTree read_neuron_factor_tree(const QString path)
{
	cout<<"Reading NeuronFactor Tree."<<endl;

	NeuronFactorTree nft;
	QFile qf(path);
	if (! qf.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		v3d_msg(QString("open file at [%1] failed!").arg(path));
		return nft;
	}

	//nft members
	NeuronFactor neuron_factor_tree_root;
	NeuronFactors neuron_factors;

	QStringList feature_names;//record feature names in string;

	while (! qf.atEnd())
	{
		NeuronFactor nf;
		/************************************************************************/
		/*	nf members
			int level;
			V3DLONG label;
			float x, y, z;
			unordered_map<string,double> feature;
			vector<double> feature_vector;
			V3DLONG parent;
			V3DLONG child;
			V3DLONG child_sibling;
			V3DLONG sibling;                                                    */
		/************************************************************************/

		//nf features
		unordered_map<string,double> feature;
		vector<double> feature_vector;
		QStringList feature_values;//record feature values in string;

		char _buf[1000], *buf;
		qf.readLine(_buf, sizeof(_buf));
		for (buf=_buf; (*buf && *buf==' '); buf++); //skip space

		if (buf[0]=='\0')	continue;//empty
		if (buf[0]=='#')//headers
		{
			if (buf[1] == 'L'&&buf[2] == 'E'&&buf[3] == 'V'&&buf[4] == 'E'&&buf[5] == 'L'&&buf[6] == ':') nft.level = QString("%1").arg(buf + 8).toInt();//skip "";
			else if (buf[1] == 'N'&&buf[2] == 'E'&&buf[3] == 'U'&&buf[4] == 'R'&&buf[5] == 'O'&&buf[6] == 'N'&&
				buf[7] == 'P'&&buf[8] == 'A'&&buf[9] == 'T'&&buf[10] == 'H'&&buf[11] == ':')
				nft.neuron_path = QString("%1").arg(buf + 13).toStdString();//skip a blank;
			else if (buf[1]=='F'&&buf[2]=='E'&&buf[3]=='A'&&buf[4]=='T'&&buf[5]=='U'&&buf[6]=='R'&&buf[7]=='E'&&buf[8]=='S'&&buf[9]==':')
			{
				QString feature_name_str = QString("%1").arg(buf+10); 
				feature_names = feature_name_str.split(" ", QString::SkipEmptyParts);
			}
			continue;
		}

		//First line for nf members;
		QStringList msl = QString(buf).trimmed().split(" ",QString::SkipEmptyParts);//members string list;
		if (msl.size()==0)   continue;//empty string

		nf.level = nft.level;
		for (int i=0; i<msl.size(); i++)
		{
			msl[i].truncate(99);//in case it's too long;
			if (i==0) nf.label = msl[i].toLong();
			else if (i==1) nf.type = msl[i].toInt();
			else if (i==2) nf.x = msl[i].toFloat();
			else if (i==3) nf.y = msl[i].toFloat();
			else if (i==4) nf.z = msl[i].toFloat();
			//skip msl[5] which is for radius;
			else if (i==6) nf.parent = msl[i].toLong();
			else if (i==7) nf.child = msl[i].toLong();
			else if (i == 8) nf.child_sibling = msl[i].toLong();
			else nf.sibling = msl[i].toLong();
		}
		//Second line for faeture values;
		qf.readLine(_buf, sizeof(_buf));
		buf = _buf;
		if (buf[0] == '#'&&buf[1] == 'V'&&buf[2] == 'A'&&buf[3] == 'L'&&buf[4] == 'U'&&buf[5] == 'E'&&buf[6] == 'S'&&buf[7] == ':')
		{
			QString features_str = QString("%1").arg(buf + 8);
			feature_values = features_str.split(" ", QString::SkipEmptyParts);
			if (feature_values.size() != 1) {//for don't have any values, skip;
				for (int i = 0; i < feature_names.size(); i++)
				{
					feature[feature_names[i].toStdString()] = feature_values[i].toDouble();
					feature_vector.push_back(feature_values[i].toDouble());
					nf.feature = feature;
					nf.feature_vector = feature_vector;
				}
			}
		}
		neuron_factors[nf.label] = nf;
		if (nf.parent==-1) neuron_factor_tree_root = nf;
	}
	nft.neuron_factor_tree_root = neuron_factor_tree_root;
	nft.neuron_factors = neuron_factors;

	cout<<"Reading NeuronFactor Tree finish."<<endl;
	cout<<"NeuronFactor Tree size:"<<nft.neuron_factors.size()<<endl;

	return nft;
}

NeuronFactorSequences serialization(const NeuronFactorTree& nft)
{ 
	NeuronFactor current = nft.neuron_factor_tree_root;
	NeuronFactorSequences nfseqs;
	NeuronFactorSequence sequence;
	sequence.push_back(current);
	while (1)
	{
		while (current.child!=-1)
		{
			current = nft.neuron_factors.at(current.child);
			sequence.push_back(current);
		}
		nfseqs.push_back(sequence);

		current = sequence.back();
		sequence.pop_back();//pop out last;

		for (;current.sibling==-1 && current.parent!=-1;current = nft.neuron_factors.at(current.parent))sequence.pop_back();//find a node which has a sibling
		if (current.sibling==-1 ) {break;}//means get to the root node, break;
		current = nft.neuron_factors.at(current.sibling);
		sequence.push_back(current);
	}
	//unordered_map<V3DLONG, bool> marked;
	//(nft.neuron_factors.begin(), nft.neuron_factors.end(), [&marked](const pair<int, NeuronFactor> p) {marked[p.first] = false; });
	
	cout<<"NeuronFactor Tree serializated."<<endl;
	cout<<"Got sequences:"<<nfseqs.size()<<endl;

	return nfseqs;
}

void save_neuron_factor_sequences(const NeuronFactorSequences &nfseqs, const string neuron_file_path, const QString path)
{
	/*
	 *.nfss files for neuronfactor sequences;
	 *in format:
	 *#FEATURES : feaeture names
	 *#LEVEL:
	 *#SEQ_SIZE:
	 *#ROOT : a seq begins
	 *#VALUES:
	 *#VALUES:
	 *...
	 *#LEAF : a seq ends
	 *...
	 */
	if (nfseqs.size() == 0) { cout<<"Sequences is empty!"; return; }
	cout<<"Saving NeuronFactor Sequences."<<endl;
	FILE * fp;
	fp = fopen(path.toLatin1(), "w");
	cout<<"Open file :"<<qPrintable(path)<<endl;
	if (fp==NULL)
	{
		cout<<"ERROR! Can't open file "<<qPrintable(path)<<endl;
	}
	cout<<"neuron sequence num is :"<<nfseqs.size()<<endl;
	//header for features
	fprintf(fp, "#FEATURES: fea_dist_c1 fea_dist_c2 fea_dist_soma fea_bif_angle_remote fea_path_len_1 fea_contraction_1 fea_diameter_avg_1 fea_area_avg_1 fea_surface_1 fea_volume_1 "
		"fea_path_len_2 fea_contraction_2 fea_diameter_avg_2 fea_area_avg_2 fea_surface_2 fea_volume_2 fea_bif_angle_local"
		/*" fea_tilt_angle_remote fea_tilt_angle_local fea_torque_remote fea_torque_local "
		"fea_child_ratio"*/
		" fea_direction_x fea_direction_y fea_direction_z fea_tips_num fea_tips_asymmetry\n");//define feature names;

	//save level
	fprintf(fp, "#LEVEL: %d\n", nfseqs[0].front().level);
	//save neuron file path
	fprintf(fp, "#NEURONPATH: ");
	fprintf(fp, neuron_file_path.c_str());
	fprintf(fp, "\n");
	//a block for a sequence; 
	for (auto seq = nfseqs.begin(); seq != nfseqs.end(); seq++)
	{
		fprintf(fp, "#SEQ_TYPE: %d\n",(*seq).rbegin()->type);// type;
		fprintf(fp, "#SEQ_SIZE: %d\n",(*seq).size());
		fprintf(fp, "#NODES:");
		for (auto nf = (*seq).begin(); nf != (*seq).end(); nf++)
		{
			fprintf(fp, " %d", nf->label);
		}
		fprintf(fp, "\n");
		//feature1,feature2,...featuren,
		for(auto nf = (*seq).begin(); nf != (*seq).end(); nf++)
		{
			if (nf->parent == -1) fprintf(fp, "#ROOT\n");//mark root;
			else if (nf->child == -1) fprintf(fp, "#LEAF\n");//mark leaf;
			else//print values;
			{
				fprintf(fp, "#VALUES:");
				auto f_vector = (*nf).feature_vector;
				for (int i = 0; i < f_vector.size(); i++)
				{
					double f = f_vector[i];
					fprintf(fp, " %5.3lf", f);
				}
				fprintf(fp, "\n");
			}
		}
		fprintf(fp, "\n");		
	}
	cout << "NeuronFactor Sequences saved at path:" << qPrintable(path) << endl;
	fclose(fp);
}

SomaBranch get_soma_branch(const NeuronTree &neuron)
{
	/*Input: a neuron file;
	 *Return a neuron's soma branch part;
	 */
	cout << "Get soma_branch begins." << endl;
	vector<V3DLONG> branches;
	calculate_branches(neuron, branches);
	auto getCoordxyz = [](const NeuronSWC node) { return Coordxyz(node.x, node.y, node.z); };//get coord from a NeuronSWC;

	V3DLONG soma_id;
	for each (auto node in neuron.listNeuron)
	{
		if (node.parent == -1)
		{
			soma_id = node.n;
			break;
		}
	}
	V3DLONG origin_node = neuron.hashNeuron.value(soma_id);//hash value for soma node;

	SomaBranch sb;
	vector<V3DLONG> child_branch_root_id;//tchild branches' root id, unhash;
	//record soma branch's nodes;
	for each (auto node in neuron.listNeuron)
	{
		if (branches[neuron.hashNeuron.value(node.n)] != 1 && node.n!= soma_id)
		{
			V3DLONG parent = neuron.hashNeuron.value(node.pn);
			for (; parent != 0 && branches[parent] <= 1; parent = neuron.hashNeuron.value(neuron.listNeuron[parent].pn));
			if (parent == origin_node)//node's nf parent is origin_node, collect those between node and origin_node;
			{
				sb.stem_type.push_back(node.type);
				sb.stem_end.push_back(node.pn);
				sb.stem_edge_coord.push_back(Coordxyz(node.x, node.y, node.z));
				//record a branch's edege coord so that child branch coord can use;
				child_branch_root_id.push_back(node.n);//get child branchs' root id;
				sb.stem_num++;
				//stem_id++;
				for (auto pa = neuron.hashNeuron.value(node.pn); pa != origin_node; pa = neuron.hashNeuron.value(neuron.listNeuron[pa].pn))
				{
					sb.soma_branch_nodes.push_back(neuron.listNeuron[pa]);
					//cout << neuron.listNeuron[pa].n << endl;
				}
				//cout << "one stem finish." << endl;
			}
		}
	}
	//put soma node;
	sb.soma_branch_nodes.push_back(neuron.listNeuron[origin_node]);
	cout << "Record soma branch nodes finish." << endl;
	cout << "Soma stem num : " << sb.stem_num << "." << endl;

	//record stem direction;
	
	for (int i = 0; i < sb.stem_num; i++)
	{
		auto stem_end = sb.stem_end[i];
		Coordxyz child_branch_end[2];
		Coordxyz child_branch_root;
		int child_branch_left_or_right = 0;
		if (stem_end != soma_id)
		{
			//cout << stem_end << endl;
			for each (auto node in neuron.listNeuron)
			{
				if (branches[neuron.hashNeuron.value(node.n)] != 1)
				{
					V3DLONG parent = neuron.hashNeuron.value(node.pn);
					for (; parent != 0 && branches[parent] <= 1; parent = neuron.hashNeuron.value(neuron.listNeuron[parent].pn));
					if (parent == neuron.hashNeuron.value(child_branch_root_id[i]))//node's nf parent is one child branch root, get a vector from root to it;
					{
						child_branch_root = getCoordxyz(neuron.listNeuron[parent]);
						child_branch_end[child_branch_left_or_right] = getCoordxyz(node);
						child_branch_left_or_right++;
					}
				}
			}
		}

		//calculate direction;
		if (child_branch_root.x == -1 && child_branch_root.y == -1 && child_branch_root.z == -1)//don't have a child branch;
		{
			sb.child_branch_direction.push_back(child_branch_root);//Fake direction;
		}
		else//have child branch;
		{
			Coordxyz child_direction;
			auto root2child1 = child_branch_end[0] - child_branch_root;
			auto root2child2 = child_branch_end[1] - child_branch_root;
			//child1_direction = (root2child1 / root2child1.length()) + (root2child2 / root2child2.length());//direction_1;
			child_direction = root2child1 + root2child2;//direction_1;
			sb.child_branch_direction.push_back(child_direction);
		}
	}
	cout << "Record soma stem direction finish." << endl;

	return sb;
}

void assemble_neuron_tree_from_deprecated(const QString gen_file_path)
{
	/*Input: .gen file, constituted by neuron factor origin sequences.
	 *format:
	 *node index, node origin swc file, node origin id, node parent index.
	 */
	QString neuron_tree_file = gen_file_path.mid(0, gen_file_path.lastIndexOf(".")) + ".swc";
	//collect soma branch;
	auto soma_branch = collect_neuron_soma_branch(gen_file_path);
	//collect branches;
	auto branches = collect_neuron_factor_infos(gen_file_path);
	vector<NeuronSWC> neuron = adjust_branches(soma_branch, branches);

	save_swc_file(neuron_tree_file, neuron);
	cout<<"Neuron swc file saved at ["<<qPrintable(neuron_tree_file)<<"].";
}

SomaBranch collect_neuron_soma_branch(const QString file_path)
{
	cout << qPrintable(file_path) << endl;
	QFile qf(file_path);
	SomaBranch sob;

	if (!qf.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		v3d_msg(QString("open file [%1] failed!").arg(file_path));
		return sob;
	}

	while (!qf.atEnd())
	{
		char buf[1000];
		qf.readLine(buf, sizeof(buf));
		//for stem num:
		if (buf[0] == '#' && buf[1] == 'S' && buf[2] == 'T' && buf[3] == 'E' && buf[4] == 'M' && buf[5] == 'N'&&buf[6] == 'U'&&buf[7] == 'M')
			sob.stem_num = QString("%1").arg(buf + 11).toInt();
		else if (buf[0] == '#' && buf[1] == 'S' && buf[2] == 'T' && buf[3] == 'E' && buf[4] == 'M' && buf[5] == 'E'&&buf[6] == 'N'&&buf[7] == 'D')
		{
			sob.stem_end.push_back(QString("%1").arg(buf + 11).toInt());// stem end index;
			qf.readLine(buf, sizeof(buf));
			sob.stem_type.push_back(QString("%1").arg(buf + 12).toInt());// stem type;
			qf.readLine(buf, sizeof(buf));
			double x = QString("%1").arg(buf + 18).toDouble();// stem coord x
			qf.readLine(buf, sizeof(buf));
			double y = QString("%1").arg(buf + 18).toDouble();// stem coord y
			qf.readLine(buf, sizeof(buf));
			double z = QString("%1").arg(buf + 18).toDouble();// stem coord z
			sob.stem_edge_coord.push_back(Coordxyz(x, y, z));

			qf.readLine(buf, sizeof(buf));
			double d_x = QString("%1").arg(buf + 18).toDouble();// stem direction x
			qf.readLine(buf, sizeof(buf));
			double d_y = QString("%1").arg(buf + 18).toDouble();// stem direction y
			qf.readLine(buf, sizeof(buf));
			double d_z = QString("%1").arg(buf + 18).toDouble();// stem direction z
			sob.child_branch_direction.push_back(Coordxyz(d_x, d_y, d_z));
		}
		else if (buf[0] == '#' && buf[1] == 'S' && buf[2] == 'O' && buf[3] == 'M' && buf[4] == 'A' && buf[5] == 'B'&&buf[6] == 'R')
		{
			QString features_str = QString("%1").arg(buf + 19);
			QStringList qsl = features_str.split(" ", QString::SkipEmptyParts);
			NeuronSWC nswc;
			nswc.n = qsl[0].toLongLong();
			nswc.type = qsl[1].toInt();
			nswc.x = qsl[2].toFloat();
			nswc.y = qsl[3].toFloat();
			nswc.z = qsl[4].toFloat();
			nswc.r = qsl[5].toFloat();
			nswc.pn = qsl[6].toLongLong();
			sob.soma_branch_nodes.push_back(nswc);
		}
	}
	cout << "Collect soma branch : "<< sob.soma_branch_nodes.size() <<"."<< endl;

	return sob;
}

vector<vector<BranchInfo>> collect_neuron_factor_infos(const QString file_path)
{
	//From .gen file make up a Branch vector;
	cout << qPrintable(file_path) << endl;
	QFile qf(file_path);
	//SomaBranch sob;
	vector<vector<BranchInfo>> total_branch_vector;
	vector<BranchInfo> branch_vector;

	if (!qf.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		v3d_msg(QString("open file [%1] failed!").arg(file_path));
		return total_branch_vector;
	}
	while (!qf.atEnd())
	{
		char buf[1000];
		qf.readLine(buf, sizeof(buf));
		if (buf[0] != '#')
		{
			BranchInfo branch;

			QString str = QString("%1").arg(buf); 
			QStringList factor_infos = str.split(" ");

			if (factor_infos.size() == 1) continue;//ship empty line;

			int branch_id = factor_infos[0].toInt();
			QString origin_swc = factor_infos[1];
			int origin_node = factor_infos[2].toInt();
			int parent_branch_id = factor_infos[3].toInt();

			//collect branch nodes;
			NeuronTree neuron = readSWC_file(origin_swc);
			vector<V3DLONG> branches;
			calculate_branches(neuron, branches);
			origin_node = neuron.hashNeuron.value(origin_node);//from swc node id  (start with 1) to list id (start with 0).
			int left_or_right = 0;
			V3DLONG child_branch_root_id[2] = { -1, -1};//two child branches' root id, unhash;

			auto getCoordxyz = [](const NeuronSWC node) { return Coordxyz(node.x, node.y, node.z); };//get coord from a NeuronSWC;

			for each (auto node in neuron.listNeuron)
			{				
				if (branches[neuron.hashNeuron.value(node.n)] != 1)
				{
					V3DLONG parent = neuron.hashNeuron.value(node.pn);
					for (; parent != 0 && branches[parent] <= 1; parent = neuron.hashNeuron.value(neuron.listNeuron[parent].pn));
					if (parent == origin_node)//node's nf parent is origin_node, collect those between node and origin_node;
					{
						if (left_or_right >= 2) break;
						branch.branch_end[left_or_right] = node.pn;//two branch_end;
						branch.branch_edge_coord[left_or_right] = Coordxyz(node.x, node.y, node.z);
						//cout << branch.branch_edge_coord[left_or_right] << endl;
						//record a branch's edege coord so that child branch coord can use;
						child_branch_root_id[left_or_right] = node.n;//get child branchs' root id;
						left_or_right++;
						for (auto pa = neuron.hashNeuron.value(node.pn); pa != origin_node; pa = neuron.hashNeuron.value(neuron.listNeuron[pa].pn))
						{
							branch.branch_nodes.push_back(neuron.listNeuron[pa]);
							//cout << neuron.listNeuron[pa].n << endl;
						}
						//cout << "one stem finish." << endl;
					}
				}
			}
			branch.branch_nodes.push_back(neuron.listNeuron[origin_node]);//a branch finish, collect parent node also;
			cout << "Branch nodes collected." << endl;

			/*Update 2019-06-09, record child branchs' directions, so that could adjust generation branch's direction in future;*/
			//record a branch's child bracnch's direction vector;
			Coordxyz child_branch1_end[2];
			Coordxyz child_branch1_root;
			int child_branch1_left_or_right = 0;
			Coordxyz child_branch2_end[2];
			Coordxyz child_branch2_root;
			int child_branch2_left_or_right = 0;

			for each (auto node in neuron.listNeuron)
			{
				if (branches[neuron.hashNeuron.value(node.n)] != 1)
				{
					V3DLONG parent = neuron.hashNeuron.value(node.pn);
					for (; parent != 0 && branches[parent] <= 1; parent = neuron.hashNeuron.value(neuron.listNeuron[parent].pn));
					if (parent == neuron.hashNeuron.value(child_branch_root_id[0]))//node's nf parent is one child branch root, get a vector from root to it;
					{
						if (child_branch2_left_or_right >= 2) break;
						child_branch1_root = getCoordxyz(neuron.listNeuron[parent]);
						child_branch1_end[child_branch1_left_or_right] = getCoordxyz(node);
						child_branch1_left_or_right++;
					}
					else if (parent == neuron.hashNeuron.value(child_branch_root_id[1]))//another child branch root;
					{
						child_branch2_root = getCoordxyz(neuron.listNeuron[parent]);
						child_branch2_end[child_branch2_left_or_right] = getCoordxyz(node);
						child_branch2_left_or_right++;
					}
				}
			}

			Coordxyz child1_direction, child2_direction;
			auto root2child1 = child_branch1_end[0] - child_branch1_root;
			auto root2child2 = child_branch1_end[1] - child_branch1_root;
			//child1_direction = (root2child1 / root2child1.length()) + (root2child2 / root2child2.length());//direction_1;
			child1_direction = root2child1 + root2child2;//direction_1;

			root2child1 = child_branch2_end[0] - child_branch2_root;
			root2child2 = child_branch2_end[1] - child_branch2_root;
			child2_direction = root2child1 + root2child2;//direction_2;

			branch.child_branch_direction[0] = child1_direction;
			branch.child_branch_direction[1] = child2_direction;

			//For branch don't have a child branch; use vector from root to leaf node as direction;
			if (child_branch1_root.x == -1 && child_branch1_root.y == -1 && child_branch1_root.z == -1)
			{
				branch.child_branch_direction[0] = branch.branch_edge_coord[0]
					- getCoordxyz(branch.branch_nodes[branch.branch_nodes.size()-1]);
			}
			if (child_branch2_root.x == -1 && child_branch2_root.y == -1 && child_branch2_root.z == -1)
			{
				branch.child_branch_direction[1] = branch.branch_edge_coord[1]
					- getCoordxyz(branch.branch_nodes[branch.branch_nodes.size() - 1]);
			}
			//cout << "D1 : " << branch.child_branch_direction[0] << endl;
			//cout << "D2 : " << branch.child_branch_direction[1] << endl;
			cout << "Got child branch directions." << endl;

			//collect rest;
			branch.branch_id = branch_id;
			branch.branch_parent = parent_branch_id;
			//cout << "Branch info : " << branch.branch_id<<" "<<branch.branch_parent<<" "<<branch.branch_end[0]<<"-"<<branch.branch_end[1] << endl;

			//collect in vector;
			branch_vector.push_back(branch);
		}
		else if (buf[0] == '#' && buf[1] == 'G' && buf[2] == 'E' && buf[3] == 'N' && buf[4] == 'E'&& buf[5] == 'N'&& buf[6] == 'D')
		{
			total_branch_vector.push_back(branch_vector);
			branch_vector.clear();
		}
	}

	return total_branch_vector;
}

//adjust the id of collected neuronSWC;
vector<NeuronSWC> adjust_branches(SomaBranch soma_branch, vector<vector<BranchInfo>> total_branches)
{
	/*Input : structured NeuronSWC, vector<Branch> <- Branch <- NeuronSWC;
	 *Return : a battery of index-adjusted NeuronSWC vector<NeuronSWC>;
	 */
	vector<NeuronSWC> ret_vec;

	int index = 1;//global index;
	int parent = -1;//current branch's parent swc node e.g. one branch end node in parent branch;
	Coordxyz parent_edge_coord;//current branch's start coord e.g. parent branch's edge coord;
	auto getCoordxyz = [](const NeuronSWC node) { return Coordxyz(node.x, node.y, node.z); };//get coord from a NeuronSWC;
	auto adjust_coord = [](NeuronSWC &node, const Coordxyz new_coord) 
	{node.x = new_coord.x, node.y = new_coord.y, node.z = new_coord.z; };//adjust NeuronSWC's coord with a Coordxyz;
	auto dist = [](const Coordxyz c1, const Coordxyz c2)->float { auto sub = c2 - c1; return sqrt(sub.x*sub.x + sub.y*sub.y + sub.z*sub.z); };

	//Adjust soma branch ids;
	auto branch_root_node = soma_branch.soma_branch_nodes[soma_branch.soma_branch_nodes.size() - 1];//soma branch's root node;
	int parent_in_soma_old = branch_root_node.n;//the parent id in branch before adjust;
	int parent_in_soma_new = 1;//the parent id in branch after adjust;

	for (int i = (soma_branch.soma_branch_nodes.size() - 1); i >= 0; i--)//nodes in branch, stores in leaf->...->leaf_another->...->root;
	{
		auto &curr_node_in_soma = soma_branch.soma_branch_nodes[i];
		//cout << "curr:" << curr_node_in_soma.n << endl;
		//cout << "pa:" << curr_node_in_soma.parent << endl;
		for (int m = 0; m < soma_branch.stem_num; m++)
		{
			if (curr_node_in_soma.n == soma_branch.stem_end[m])
			{
				soma_branch.stem_end[m] = index;
				//cout << soma_branch.stem_end[m] << endl;
			}
		}
		//cout << "Old:" << curr_node.n << ", Parent" << curr_node.parent <<endl;
		curr_node_in_soma.n = index;
		if (curr_node_in_soma.parent == parent_in_soma_old)
			curr_node_in_soma.parent = parent_in_soma_new;//if parent is branch_root, renew it to new index of branch_root;
		else
			curr_node_in_soma.parent = parent;//else going down;
		//cout << "New:" << curr_node.n << ", Parent" << curr_node.parent << endl;
		parent = index++;
	}
	//push soma_branch in;
	for each (auto node in soma_branch.soma_branch_nodes)
		ret_vec.push_back(node);
	//return ret_vec;

	//adjust branches;
	for (int s = 0; s < soma_branch.stem_num; s++)//for each stem end;
	{
		cout << "stem: " << s << " begins." << endl;
		int stem_type = soma_branch.stem_type[s];
		if (soma_branch.child_branch_direction[s].x == -1 && soma_branch.child_branch_direction[s].y == -1 && soma_branch.child_branch_direction[s].z == -1)
			continue;
		auto branches = total_branches[s];//get a stem to add branch;
		// Leaf count sum up.
		cout << branches.size() << endl;
		for (int i = branches.size()-1; i >=0; i--)
		{
			auto &branch = branches[i];

			int parent_stem;

			if (branch.branch_parent == -1)//first branch, don't have parent branch;
			{
				parent = soma_branch.stem_end[s];
				parent_edge_coord = soma_branch.stem_edge_coord[s];
				//cout << "Seq parent : " << parent << endl;

			}
			else
			{
				//current branch's parent branch;
				auto &parent_branch = branches[branch.branch_parent];//the current branch's parent branch;
				if (parent_branch.left_or_right == -1)//First child branch to choose parent stem.
				{
					//std::cout << "first" << std::endl;
					auto len_stem_1 = dist(parent_branch.branch_edge_coord[0], getCoordxyz(parent_branch.branch_nodes[parent_branch.branch_nodes.size() - 1]));
					auto len_stem_2 = dist(parent_branch.branch_edge_coord[1], getCoordxyz(parent_branch.branch_nodes[parent_branch.branch_nodes.size() - 1]));
					parent_stem = (len_stem_1 > len_stem_2 ? 0 : 1);
					parent_branch.left_or_right = (parent_stem == 0 ? 1 : 0);
				}
				else
				{
					//std::cout << "second" << std::endl;

					parent_stem = parent_branch.left_or_right;
					parent_branch.left_or_right = -1;
				}

				//parent_branch.stem_leaf_count[parent_stem] = branch.stem_leaf_count[0] + branch.stem_leaf_count[1];// Sum up stem leaf count.
				if (branch.stem_weight_count[0] == 0.f)
				{
					branch.stem_weight_count[0] = dist(branch.branch_edge_coord[0], getCoordxyz(branch.branch_nodes[branch.branch_nodes.size() - 1]));
				}
				if (branch.stem_weight_count[1] == 0.f)
				{
					branch.stem_weight_count[1] = dist(branch.branch_edge_coord[1], getCoordxyz(branch.branch_nodes[branch.branch_nodes.size() - 1]));
				}
				parent_branch.stem_weight_count[parent_stem] = branch.stem_weight_count[0] + branch.stem_weight_count[1];// Sum up stem leaf count.
				//cout << branch.stem_leaf_count[0] + branch.stem_leaf_count[1] << endl;
				//cout << "Branch parent : " << parent << endl;
			}

		}

		for (int i = 0; i < branches.size(); i++)
		{
			auto &branch = branches[i];

			int parent_stem;

			if (branch.branch_parent == -1)//first branch, don't have parent branch;
			{
				parent = soma_branch.stem_end[s];
				parent_edge_coord = soma_branch.stem_edge_coord[s];
				//cout << "Seq parent : " << parent << endl;

			}
			else
			{
				//current branch's parent branch;
				auto &parent_branch = branches[branch.branch_parent];//the current branch's parent branch;
				if (parent_branch.left_or_right == -1)//First child branch to choose parent stem.
				{
					//std::cout << "first" << std::endl;
					auto len_stem_1 = dist(parent_branch.branch_edge_coord[0], getCoordxyz(branch.branch_nodes[branch.branch_nodes.size() - 1]));
					auto len_stem_2 = dist(parent_branch.branch_edge_coord[1], getCoordxyz(branch.branch_nodes[branch.branch_nodes.size() - 1]));
					parent_stem = (len_stem_1 > len_stem_2 ? 0 : 1);
					parent_branch.left_or_right = (parent_stem == 0 ? 1 : 0);
				}
				else
				{
					//std::cout << "second" << std::endl;

					parent_stem = parent_branch.left_or_right;
				}
				parent = parent_branch.branch_end[parent_stem];
				parent_edge_coord = parent_branch.branch_edge_coord[parent_stem];//the current branch's start coord, record in parent branch;

				//parent_branch.stem_leaf_count[parent_stem] = branch.stem_leaf_count[0] + branch.stem_leaf_count[1];// Sum up stem leaf count.

				//parent = parent_branch.branch_end[parent_branch.left_or_right];//the current branch's parent branch's end node index;
				//parent_edge_coord = parent_branch.branch_edge_coord[parent_branch.left_or_right];//the current branch's start coord, record in parent branch;
				//parent_branch.left_or_right = (parent_branch.left_or_right) + 1;
				//cout << "Branch parent : " << parent << endl;

			}

			//////////////////////////////////////////////////////////////////////////////
			//Adjust ids;
			//the current branch's root 
			auto branch_root_node = branch.branch_nodes[branch.branch_nodes.size() - 1];//current branch's root node;
			int parent_in_branch_old = branch_root_node.n;//the parent id in branch before adjust;
			int parent_in_branch_new = index;//the parent id in branch after adjust;
			//cout << branch_root_node.n << endl;

			for (int i = (branch.branch_nodes.size() - 1); i >= 0; i--)//nodes in branch, stores in leaf->...->leaf_another->...->root;
			{
				auto &curr_node = branch.branch_nodes[i];
				if (curr_node.n == branch.branch_end[0])
					branch.branch_end[0] = index;
				if (curr_node.n == branch.branch_end[1])
					branch.branch_end[1] = index;
				//cout << "Old:" << curr_node.n << ", Parent" << curr_node.parent <<endl;
				curr_node.n = index;
				if (curr_node.parent == parent_in_branch_old)
					curr_node.parent = parent_in_branch_new;//if parent is branch_root, renew it to new index of branch_root;
				else
					curr_node.parent = parent;//else going down;
				//cout << "New:" << curr_node.n << ", Parent" << curr_node.parent << endl;
				curr_node.type = stem_type;// also adjust type;
				parent = index++;
			}
			cout << "Index adjusted." << endl;

			//cout << "Branch end point" << branch.branch_end[0] << "," << branch.branch_end[1] << endl;


			//////////////////////////////////////////////////////////////////////////////////
			//Adjust coords;
			//a shifting vector;
			Coordxyz shifting = getCoordxyz(branch_root_node) - parent_edge_coord;

			for (int i = (branch.branch_nodes.size() - 1); i >= 0; i--)//nodes in branch, stores in leaf->...->leaf_another->...->root;
			{
				//cout << "Old coord:" <<getCoordxyz(branch.branch_nodes[i]) << endl;
				auto new_coord = getCoordxyz(branch.branch_nodes[i]) - shifting;//coord to adjust;

				adjust_coord(branch.branch_nodes[i], new_coord);//adjust it;

				//cout << "New coord:" << getCoordxyz(branch.branch_nodes[i]) << endl;
			}
			//branch_edge coord adjust;
			branch.branch_edge_coord[0] = branch.branch_edge_coord[0] - shifting;
			branch.branch_edge_coord[1] = branch.branch_edge_coord[1] - shifting;


			////////////////////////////////////////////////////////////////////////////////////
			/*Update 2019-06-09*/
			//Adjust branch direction;
			branch_root_node = branch.branch_nodes[branch.branch_nodes.size() - 1];//get current branch's root node, adjusted;
			auto coord_root = getCoordxyz(branch_root_node);//current bracnch's root coordinante;

			//Get vector from root to children;
			auto root2child1 = branch.branch_edge_coord[0] - coord_root;//A vector from root to child1;
			auto root2child2 = branch.branch_edge_coord[1] - coord_root;//A vector from root to child1;

			//uniformazition;
			root2child1 = root2child1 / root2child1.length();
			root2child2 = root2child2 / root2child2.length();

			//current branch's direction;
			//auto branch_direction = root2child1 + root2child2;//set a branch's direction as middle of two vectors;

			auto branch_direction = ((branch.stem_weight_count[0]* root2child2 + branch.stem_weight_count[1]* root2child1)/
				(branch.stem_weight_count[0]+branch.stem_weight_count[1]));//set a branch's direction as middle of two vectors;
			//auto branch_direction = 0.8*root2child2+0.2*root2child1;//set a branch's direction as middle of two vectors;
			cout << branch_direction << endl;
			//branch_direction = branch_direction / branch_direction.length();//uniformation;
			cout << "Get branch direction." << endl;

			//parent branch;
			//cout << branch.branch_parent << endl;
			Coordxyz branch_direction_new;
			if (branch.branch_parent == -1)//first branch in sequence, use soma_branch stem direction;
			{
				branch_direction_new = soma_branch.child_branch_direction[s];
			}
			else
			{
				auto &parent_branch = branches[branch.branch_parent];//the current branch's parent branch;
				branch_direction_new = parent_branch.child_branch_direction[parent_stem];//current branch's target direction;
				//parent_branch.left_or_right = (parent_branch.left_or_right) + 1;//renew left_or_right flag;
			}
			
			//branch_direction_new = branch_direction_new / branch_direction_new.length();//uniformation;
			cout << "Get target branch direction." << endl;

			///////////////////////////////////////////////////////////
			// Rotate

			const double PI = 3.1415926;
			auto angle_of = [PI](const Coordxyz v1, const Coordxyz v2)->double { double angle = acos((v1.x * v2.x + v1.y * v2.y + v1.z * v2.z) / v1.length() / v2.length()) * 180 / PI; return angle; };

			///////////////////////////////////////////////////////////
			// Rotate with Z.

			//projection to XOY;
			auto branch_direction_p = branch_direction;
			branch_direction_p.z = 0.0;
			auto branch_direction_new_p = branch_direction_new;
			branch_direction_new_p.z = 0.0;

			auto angle = angle_of(branch_direction_p, branch_direction_new_p);//the two vectors' angle;
			if (branch_direction_p.cross_product(branch_direction_new_p).z < 0)//need anticlockwise rotate;
				angle = 360 - angle;
			auto angle_ = angle * PI / 180.0;//radian angle;
			auto sin_angle = sin(angle_);
			auto cos_angle = cos(angle_);
			//cout << "Rotate Z angle calculated." << endl;

			//move branch root to O(0, 0, 0)
			auto shiftting_from_o2root = coord_root - Coordxyz(0.0, 0.0, 0.0);//shiffting;
			auto coord_after_rotate_theta_with_Z = [sin_angle, cos_angle](const Coordxyz coord)->Coordxyz
			{
				auto coord_x_rotated = (coord.x * cos_angle) - (coord.y * sin_angle);//rotate with Z-axis;
				auto coord_y_rotated = (coord.x * sin_angle) + (coord.y * cos_angle);//rotate with Z-axis;
				auto coord_z_rotated = coord.z;//rotate with Z-axis;
				auto coord_after_rotate = Coordxyz(coord_x_rotated, coord_y_rotated, coord_z_rotated);
				return coord_after_rotate;
			};
			for (int i = 0; i < branch.branch_nodes.size(); i++)
			{
				//cout << "Before rotate : " << getCoordxyz(branch.branch_nodes[i]) << endl;
				auto coord_after_shiftting = getCoordxyz(branch.branch_nodes[i]) - shiftting_from_o2root;//target position;
				auto coord_after_rotate = coord_after_rotate_theta_with_Z(coord_after_shiftting);
				//cout << "After rotate : " << coord_after_rotate << endl;
				auto coord_target = coord_after_rotate + shiftting_from_o2root;
				adjust_coord(branch.branch_nodes[i], coord_target);
			}

			//adjust branch_edge_coord also;
			auto new_edge_coord_0 = coord_after_rotate_theta_with_Z(branch.branch_edge_coord[0] - shiftting_from_o2root) + shiftting_from_o2root;
			branch.branch_edge_coord[0] = new_edge_coord_0;
			auto new_edge_coord_1 = coord_after_rotate_theta_with_Z(branch.branch_edge_coord[1] - shiftting_from_o2root) + shiftting_from_o2root;
			branch.branch_edge_coord[1] = new_edge_coord_1;

			//Adjust child_branch_direction also;
			branch.child_branch_direction[0] = coord_after_rotate_theta_with_Z(branch.child_branch_direction[0]);
			branch.child_branch_direction[1] = coord_after_rotate_theta_with_Z(branch.child_branch_direction[1]);

			branch_direction = coord_after_rotate_theta_with_Z(branch_direction);

			///////////////////////////////////////////
			// Rotate with Y.

			branch_direction_p = branch_direction;
			branch_direction_p.y = 0.0;
			branch_direction_new_p = branch_direction_new;
			branch_direction_new_p.y = 0.0;

			angle = angle_of(branch_direction_p, branch_direction_new_p);//the two vectors' angle;
			if (branch_direction_p.cross_product(branch_direction_new_p).y < 0)//need anticlockwise rotate;
				angle = 360 - angle;
			angle_ = angle * PI / 180.0;//radian angle;
			sin_angle = sin(angle_);
			cos_angle = cos(angle_);

			//cout << "Rotate Y angle calculated." << endl;

			//move branch root to O(0, 0, 0)
			auto coord_after_rotate_theta_with_Y = [sin_angle, cos_angle](const Coordxyz coord)->Coordxyz
			{
				auto coord_x_rotated = (coord.x * cos_angle) + (coord.z * sin_angle);//rotate with Z-axis;
				auto coord_y_rotated = coord.y;//rotate with Z-axis;
				auto coord_z_rotated = (-coord.x * sin_angle) + (coord.z * cos_angle);//rotate with Z-axis;
				auto coord_after_rotate = Coordxyz(coord_x_rotated, coord_y_rotated, coord_z_rotated);
				return coord_after_rotate;
			};
			for (int i = 0; i < branch.branch_nodes.size(); i++)
			{
				auto coord_after_shiftting = getCoordxyz(branch.branch_nodes[i]) - shiftting_from_o2root;//target position;
				auto coord_after_rotate = coord_after_rotate_theta_with_Y(coord_after_shiftting);
				auto coord_target = coord_after_rotate + shiftting_from_o2root;
				adjust_coord(branch.branch_nodes[i], coord_target);
			}

			//adjust branch_edge_coord also;
			new_edge_coord_0 = coord_after_rotate_theta_with_Y(branch.branch_edge_coord[0] - shiftting_from_o2root) + shiftting_from_o2root;
			branch.branch_edge_coord[0] = new_edge_coord_0;
			new_edge_coord_1 = coord_after_rotate_theta_with_Y(branch.branch_edge_coord[1] - shiftting_from_o2root) + shiftting_from_o2root;
			branch.branch_edge_coord[1] = new_edge_coord_1;

			//Adjust child_branch_direction also;
			branch.child_branch_direction[0] = coord_after_rotate_theta_with_Y(branch.child_branch_direction[0]);
			branch.child_branch_direction[1] = coord_after_rotate_theta_with_Y(branch.child_branch_direction[1]);

			branch_direction = coord_after_rotate_theta_with_Y(branch_direction);

			///////////////////////////////////////////
			// Rotate with X.

			branch_direction_p = branch_direction;
			branch_direction_p.x = 0.0;
			branch_direction_new_p = branch_direction_new;
			branch_direction_new_p.x = 0.0;

			angle = angle_of(branch_direction_p, branch_direction_new_p);//the two vectors' angle;
			if (branch_direction_p.cross_product(branch_direction_new_p).x < 0)//need anticlockwise rotate;
				angle = 360 - angle;
			angle_ = angle * PI / 180.0;//radian angle;
			sin_angle = sin(angle_);
			cos_angle = cos(angle_);

			//cout << "Rotate X angle calculated." << endl;

			//move branch root to O(0, 0, 0)
			auto coord_after_rotate_theta_with_X = [sin_angle, cos_angle](const Coordxyz coord)->Coordxyz
			{
				auto coord_x_rotated = coord.x;//rotate with Z-axis;
				auto coord_y_rotated = (coord.y * cos_angle) - (coord.z * sin_angle);//rotate with Z-axis;
				auto coord_z_rotated = (coord.y * sin_angle) + (coord.z * cos_angle);
				auto coord_after_rotate = Coordxyz(coord_x_rotated, coord_y_rotated, coord_z_rotated);
				return coord_after_rotate;
			};
			for (int i = 0; i < branch.branch_nodes.size(); i++)
			{
				auto coord_after_shiftting = getCoordxyz(branch.branch_nodes[i]) - shiftting_from_o2root;//target position;
				auto coord_after_rotate = coord_after_rotate_theta_with_X(coord_after_shiftting);
				auto coord_target = coord_after_rotate + shiftting_from_o2root;
				adjust_coord(branch.branch_nodes[i], coord_target);
			}
			cout << "Branch direction adjusted." << endl;

			//adjust branch_edge_coord also;
			new_edge_coord_0 = coord_after_rotate_theta_with_X(branch.branch_edge_coord[0] - shiftting_from_o2root) + shiftting_from_o2root;
			branch.branch_edge_coord[0] = new_edge_coord_0;
			new_edge_coord_1 = coord_after_rotate_theta_with_X(branch.branch_edge_coord[1] - shiftting_from_o2root) + shiftting_from_o2root;
			branch.branch_edge_coord[1] = new_edge_coord_1;

			//Adjust child_branch_direction also;
			branch.child_branch_direction[0] = coord_after_rotate_theta_with_X(branch.child_branch_direction[0]);
			branch.child_branch_direction[1] = coord_after_rotate_theta_with_X(branch.child_branch_direction[1]);

			cout << "Coordinate adjusted." << endl;

		}
		for (int i = 0; i < branches.size(); i++)
		{
			auto &branch = branches[i];
			for each (auto node in branch.branch_nodes)
			{
				node.type = stem_type;
				ret_vec.push_back(node);
			}
		}
	}
	
	cout << "Node adjusted." << endl;

	return ret_vec;
}

//save a created neuron(as a vector of NeuronSWC);
void save_swc_file(const QString swc_file, const vector<NeuronSWC> neuron)
{
	/*From a series of NeuronSWC save a swc file*/
	FILE * fp = fopen(swc_file.toLatin1(), "wt");
	fprintf(fp, "##n,type,x,y,z,radius,parent\n");
	NeuronSWC * p_pt = 0;
	for (int i = 0; i < neuron.size(); i++)
	{
		p_pt = (NeuronSWC *)(&(neuron.at(i)));
		fprintf(fp, "%ld %d %5.3f %5.3f %5.3f %5.3f %ld\n",
			p_pt->n, p_pt->type, p_pt->x, p_pt->y, p_pt->z, p_pt->r, p_pt->pn);
	}
	cout << "Neuron saved." << endl;
	fclose(fp);
}

//save a soma branch file;
void save_soma_branch(const QString file_path, const SomaBranch soma_branch)
{
	//save a soma branch;
	//header;
	FILE * fp = fopen(file_path.toLatin1(), "wt");
	fprintf(fp, "##Soma Branch infos:\n");
	fprintf(fp, "#STEMNUM : %d\n", soma_branch.stem_num);
	for (int i = 0; i < soma_branch.stem_num; i++)
	{
		fprintf(fp, "#STEMEND : %lld\n", soma_branch.stem_end[i]);
		fprintf(fp, "#STEMTYPE : %d\n", soma_branch.stem_type[i]);
		fprintf(fp, "#STEMEDGECOORDX : %5.3lf\n", soma_branch.stem_edge_coord[i].x);
		fprintf(fp, "#STEMEDGECOORDY : %5.3lf\n", soma_branch.stem_edge_coord[i].y);
		fprintf(fp, "#STEMEDGECOORDZ : %5.3lf\n", soma_branch.stem_edge_coord[i].z);
		fprintf(fp, "#STEMDIRECTIONX : %5.3lf\n", soma_branch.child_branch_direction[i].x);
		fprintf(fp, "#STEMDIRECTIONY : %5.3lf\n", soma_branch.child_branch_direction[i].y);
		fprintf(fp, "#STEMDIRECTIONZ : %5.3lf\n", soma_branch.child_branch_direction[i].z);
	}
	//nodes;
	fprintf(fp, "#NEURONNODES : n,type,x,y,z,radius,parent\n");
	NeuronSWC * p_pt = 0;
	for (int i = 0; i < soma_branch.soma_branch_nodes.size(); i++)
	{
		p_pt = (NeuronSWC *)(&(soma_branch.soma_branch_nodes.at(i)));
		fprintf(fp, "#SOMABRANCHNODES : %ld %d %5.3f %5.3f %5.3f %5.3f %ld\n",
			p_pt->n, p_pt->type, p_pt->x, p_pt->y, p_pt->z, p_pt->r, p_pt->pn);
	}

	cout << "Soma branch saved." << endl;
	fclose(fp);

}

//////////////////////////////////////////////////////////////////////////////////////
// Branches producting.

void collect_branches(const NeuronTree& neuronTree, const int id, branch_map_t& branch_map, branch_sequence_vector& sequences)
{
	////////////////////////////////////////////////////////////////////////////
	// Build a branch dataset and branch sequence dataset for one neuron. 
	//
	// Each branch is reprented by tuple<neuron id, branch root node id>.
	// branch_map : save all branches.
	// sequences : save every branch sequence of a neuron.

	vector<V3DLONG> branch_count;
	calculate_branches(neuronTree, branch_count);

	set<V3DLONG> leaf_node_set;
	set<V3DLONG> branch_node_set;
	get_leaf_branch_nodes(neuronTree, branch_count, leaf_node_set, branch_node_set);//find leaf nodes and branch nodes;

	set<V3DLONG> nft_node_set = branch_node_set;
	nft_node_set.insert(leaf_node_set.begin(), leaf_node_set.end());//use leaf nodes and branch nodes as nft nodes;

	// Relations.
	map<V3DLONG, V3DLONG> nft_parent_map;//record each nft nodes' parent node;
	map<V3DLONG, V3DLONG> nft_child_map;//record each nft nodes' child node;
	map<V3DLONG, V3DLONG> nft_sibling_map;// If a leaf and branch have a same root, let leaf and branch's parent as siblings.

	for (auto it = nft_node_set.begin(); it != nft_node_set.end(); it++)
	{
		nft_child_map[*it] = -1;
		nft_parent_map[*it] = -1;
		nft_sibling_map[*it] = -1;
	}

	for (auto it = nft_node_set.begin(); it != nft_node_set.end(); it++)
	{
		V3DLONG current = *it;
		//V3DLONG parent = neuronTree.hashNeuron.value(neuronTree.listNeuron[parent].pn);
		if (current == 0) continue;//for nft root don't have a parent;
		V3DLONG parent = neuronTree.hashNeuron.value(neuronTree.listNeuron[current].pn);
		for (; parent != 0 && branch_count[parent] <= 1; parent = neuronTree.hashNeuron.value(neuronTree.listNeuron[parent].pn));//go to a parent node;
		if(parent==0) continue;// Don't calculate sibling for soma branches.
		//current = (branch_count[current] != 2 ? current : neuronTree.hashNeuron.value(neuronTree.listNeuron[current].pn));// Leaf or branch.
		if (nft_child_map[parent] != -1)
		{
			auto sib = nft_child_map[parent];
			//while (nft_sibling_map[sib] != -1) sib = nft_sibling_map[sib];//this could collect all the children node in siblings;
			nft_sibling_map[sib] = current;//Add current to the list's tail;
			nft_sibling_map[current] = sib;//Add current to the list's tail;
		}
		else
		{
			nft_child_map[parent] = current;
		}
	}

	auto get_xyz = [](const NeuronSWC& n) { return Coordxyz(n.x, n.y, n.z); };
	
	for (auto leaf_it = leaf_node_set.begin(); leaf_it != leaf_node_set.end(); ++leaf_it)
	{
		//std::vector<tuple<int, V3DLONG>> seq;// A seq collect each branch's root id.
		std::vector<string> seq;// A seq collect each branch's root id.
		seq.clear();
		//seq.push_back(make_tuple(id, -1));// A leaf node marker.
		seq.push_back(string("#"));// A leaf node marker.

		auto current = *leaf_it;

		while (true)// For a sequence.
		{
			V3DLONG sib = nft_sibling_map[current];
			V3DLONG root = neuronTree.hashNeuron.value(neuronTree.listNeuron[current].pn);
			for (; root != 0 && branch_count[root] <= 1; root = neuronTree.hashNeuron.value(neuronTree.listNeuron[root].pn));// Get a branch's root node.

			if (neuronTree.listNeuron[root].pn == -1)// If is soma branch finisg this sequence.
			{
				break;
			}

			//seq.push_back(make_tuple(id, root));// Collect a branch in sequnce.
			seq.push_back(to_string(long long(id))+":"+to_string(root));// Collect a branch in sequnce.

			Stem stem;
			stem.clear();

			// Collect one stem.
			auto coord = get_xyz(neuronTree.listNeuron.at(current));
			stem.push_back(coord);
			current = neuronTree.hashNeuron.value(neuronTree.listNeuron[current].pn);

			for (; branch_count[current] != 2; current = neuronTree.hashNeuron.value(neuronTree.listNeuron[current].pn))
			{
				auto coord = get_xyz(neuronTree.listNeuron.at(current));
				stem.push_back(coord);
			}
			// And root.
			stem.push_back(get_xyz(neuronTree.listNeuron.at(root)));

			//¡¡Collect branch id.
			//auto it = branch_map.find(make_tuple(id, root));
			auto it = branch_map.find(to_string(long long(id)) + ":" + to_string(root));
			if (it == branch_map.end())// Branch do not exist, push in this stem as half branch.
			{
				Branch branch(stem, neuronTree.listNeuron.at(current).type);
				Stem another_stem;
				another_stem.clear();

				auto coord = get_xyz(neuronTree.listNeuron.at(sib));
				another_stem.push_back(coord);
				sib = neuronTree.hashNeuron.value(neuronTree.listNeuron[sib].pn);

				for (; branch_count[sib] < 2; sib = neuronTree.hashNeuron.value(neuronTree.listNeuron[sib].pn))
				{
					auto coord = get_xyz(neuronTree.listNeuron.at(sib));
					another_stem.push_back(coord);
					//cout << sib << endl;
				}
				another_stem.push_back(get_xyz(neuronTree.listNeuron.at(root)));

				branch.append(another_stem);

				//branch_map[make_tuple(id, root)] = branch;// Branch map from <id, root node id in neuron> to branch.
				branch_map[to_string(long long(id)) + ":" + to_string(root)] = branch;// Branch map from <id, root node id in neuron> to branch.
			}

			current = neuronTree.hashNeuron.value(neuronTree.listNeuron[current].pn);// To upper branch.
		}
		
		if (seq.size() > 1)// Sequence bigger than one node(fake leaf node).
			sequences.push_back(seq);
	}

	cout << branch_map.size() << endl;
	//cout << sequences.size() << endl;
	//cout << leaf_node_set.size() << endl;

}

void branch_data_normalization(branch_map_t & branch_map, branch_sequence_vector& sequences, int& shift_neuron_id)
{
	// Normalize branch by normalize two stems respectively.
	for (auto it = branch_map.begin(); it != branch_map.end(); ++it)
	{
		////////////////////////////////////////////////
		// Check branch size to dump empty branch.

		auto& branch = (*it).second;
		auto& stem_1 = (*it).second.stems[0];
		auto& stem_2 = (*it).second.stems[1];
		if (stem_1.size() < 2 || stem_2.size() < 2)
		{
			if (stem_1.size() < 2)
				cout << "first" << endl;
			else
			{
				cout << "second" << endl;

			}
			(*it).second = Branch();// let it be empty, so will not be saved later.
			continue;
		}

		///////////////////////////////////////////////
		// Shift branch start point to (0, 0, 0).

		auto root_coord = stem_1[stem_1.size() - 1];
		for (int i = 0; i < stem_1.size(); ++i)
		{
			stem_1.at(i) -= root_coord;
		}
		for (int i = 0; i < stem_2.size(); ++i)
		{
			stem_2.at(i) -= root_coord;
		}

		///////////////////////////////////////////////
		// Normalize stem diretion to (1, 0, 0)

		///////////////////////////////////////////
		// Rotate functions.
		auto coord_after_rotate_theta_with_X = [](const Coordxyz& coord, float angle_)->Coordxyz&
		{
			auto sin_angle = sin(angle_);
			auto cos_angle = cos(angle_);

			auto coord_x_rotated = coord.x;
			auto coord_y_rotated = (coord.y * cos_angle) - (coord.z * sin_angle);
			auto coord_z_rotated = (coord.y * sin_angle) + (coord.z * cos_angle);

			return Coordxyz(coord_x_rotated, coord_y_rotated, coord_z_rotated);;
		};
		auto coord_after_rotate_theta_with_Y = [](const Coordxyz& coord, float angle_)->Coordxyz&
		{
			auto sin_angle = sin(angle_);
			auto cos_angle = cos(angle_);

			auto coord_x_rotated = coord.x * cos_angle + coord.z * sin_angle;
			auto coord_y_rotated = coord.y;
			auto coord_z_rotated = (-coord.x * sin_angle) + (coord.z * cos_angle);

			return Coordxyz(coord_x_rotated, coord_y_rotated, coord_z_rotated);
		};
		auto coord_after_rotate_theta_with_Z = [](const Coordxyz& coord, float angle_)->Coordxyz&
		{
			auto sin_angle = sin(angle_);
			auto cos_angle = cos(angle_);

			auto coord_x_rotated = (coord.x * cos_angle) - (coord.y * sin_angle);
			auto coord_y_rotated = (coord.x * sin_angle) + (coord.y * cos_angle);
			auto coord_z_rotated = coord.z;

			return Coordxyz(coord_x_rotated, coord_y_rotated, coord_z_rotated);
		};

		const double PI = 3.1415926;
		auto angle_of = [PI](const Coordxyz v1, const Coordxyz v2)->double
		{
			if (v1.length() < DBL_MIN || v2.length() < DBL_MIN)
				return 0.0;
			// Angle from v1 to v2.
			double angle = acos((v1.x * v2.x + v1.y * v2.y + v1.z * v2.z) / v1.length() / v2.length()) * 180 / PI;

			return angle;
		};

		// Calculate angle of two stems.

		auto stem_direction_remote_1 = stem_1[0];// Stem direction : from root to first leaf.
		auto stem_direction_remote_2 = stem_2[0];// Stem direction : from root to first leaf.
		branch.angle = angle_of(stem_direction_remote_1, stem_direction_remote_2);

		//////////////////////////////////////////////////////////////////
		// Rotate each stem to (1, 0, 0).

		auto rotate_stem = [=](Stem& stem) {

			auto stem_direction = stem[0];
			// calculate angle of rotation with z.
			auto _xy_projection = stem_direction;
			_xy_projection.z = 0.0;
			auto _rotate_target = Coordxyz(1.0, 0.0, 0.0);// Target direction.

			auto _z_rotate_angle = angle_of(_xy_projection.normalize(), _rotate_target.normalize());
			if (_xy_projection.normalize().cross_product(_rotate_target.normalize()).z < 0)//need anticlockwise rotate(right hand).
				_z_rotate_angle = 360 - _z_rotate_angle;

			auto _z_rotate_angle_radian = _z_rotate_angle * PI / 180.0;

			// rotate with z.
			for (int i = 0; i < stem.size(); ++i)
			{
				stem[i] = coord_after_rotate_theta_with_Z(stem[i], _z_rotate_angle_radian);
			}

			// calculate angle of rotation with y.
			auto _xz_projection = coord_after_rotate_theta_with_Z(stem_direction, _z_rotate_angle_radian);
			_xz_projection.y = 0.0;// y value should already be 0.0.

			auto _y_rotate_angle = angle_of(_xz_projection.normalize(), _rotate_target.normalize());
			if (_xz_projection.normalize().cross_product(_rotate_target.normalize()).y < 0)//need anticlockwise rotate(right hand).
				_y_rotate_angle = 360 - _y_rotate_angle;

			auto _y_rotate_angle_radian = _y_rotate_angle * PI / 180.0;

			// rotate with y.
			for (int i = 0; i < stem.size(); ++i)
			{
				stem[i] = coord_after_rotate_theta_with_Y(stem[i], _y_rotate_angle_radian);
			}

		};

		rotate_stem(stem_1);
		rotate_stem(stem_2);

		// Now stem direction is (1, 0, 0).

		// Record scale then scale to 1.
		branch.stm_scale_1 = stem_1[0].length();
		branch.stm_scale_2 = stem_2[0].length();

		for (int i = 0; i < stem_1.size(); ++i)
		{
			stem_1.at(i) = stem_1.at(i) / branch.stm_scale_1;
		}
		for (int i = 0; i < stem_2.size(); ++i)
		{
			stem_2.at(i) = stem_2.at(i) / branch.stm_scale_2;
		}
	}
}

void branch_data_normalization_(branch_map_t & branch_map, branch_sequence_vector& sequences, int& shift_neuron_id)
{
	branch_map_t branch_map_new;// A map for newly cutting-created branches.

	// Normalize branch by normalize two stems respectively.
	for (auto it = branch_map.begin(); it != branch_map.end(); ++it)
	{
		////////////////////////////////////////////////
		// Check branch size to dump empty branch.

		auto& stem_1 = (*it).second.stems[0];
		auto& stem_2 = (*it).second.stems[1];
		if (stem_1.size() < 2 || stem_2.size() < 2)
		{
			(*it).second = Branch();// let it be empty, so will not be saved later.
			continue;
		}

		///////////////////////////////////////////////
		// Shift branch start point to (0, 0, 0).

		auto root_coord = stem_1[stem_1.size() - 1];
		for (int i = 0; i < stem_1.size(); ++i)
		{
			stem_1.at(i) -= root_coord;
			//branch_coords.at(i) += Coordxyz(1.f, 1.f, 1.f);
		}
		for (int i = 0; i < stem_2.size(); ++i)
		{
			stem_2.at(i) -= root_coord;
			//branch_coords.at(i) += Coordxyz(1.f, 1.f, 1.f);
		}

		///////////////////////////////////////////////
		// Normalize branch diretion to (1, 0, 0)

		normalize_branch_direction((*it).second, Coordxyz(1.0, 0, 0));

	}
}

void normalize_branch_direction(Branch& branch, const Coordxyz& branch_target_direction)
{
	//////////////////////////////////////////////////
	// Rotate branch diretion to (1, 0, 0)
	// 1. calculate branch normal(plus-Z value).
	// 2. rotate branch normal to (0, 0, 1), (x_rotate -> y_rotate).
	// 3. rotate to (1, 0, 0), (z_rotate).

	auto& stem_1 = branch.stems[0];
	auto& stem_2 = branch.stems[1];

	///////////////////////////////////////////
	// Rotate functions.
	auto coord_after_rotate_theta_with_X = [](const Coordxyz& coord, float angle_)->Coordxyz&
	{
		auto sin_angle = sin(angle_);
		auto cos_angle = cos(angle_);

		auto coord_x_rotated = coord.x;
		auto coord_y_rotated = (coord.y * cos_angle) - (coord.z * sin_angle);
		auto coord_z_rotated = (coord.y * sin_angle) + (coord.z * cos_angle);

		return Coordxyz(coord_x_rotated, coord_y_rotated, coord_z_rotated);;
	};
	auto coord_after_rotate_theta_with_Y = [](const Coordxyz& coord, float angle_)->Coordxyz&
	{
		auto sin_angle = sin(angle_);
		auto cos_angle = cos(angle_);

		auto coord_x_rotated = coord.x * cos_angle + coord.z * sin_angle;
		auto coord_y_rotated = coord.y;
		auto coord_z_rotated = (-coord.x * sin_angle) + (coord.z * cos_angle);

		return Coordxyz(coord_x_rotated, coord_y_rotated, coord_z_rotated);
	};
	auto coord_after_rotate_theta_with_Z = [](const Coordxyz& coord, float angle_)->Coordxyz&
	{
		auto sin_angle = sin(angle_);
		auto cos_angle = cos(angle_);

		auto coord_x_rotated = (coord.x * cos_angle) - (coord.y * sin_angle);
		auto coord_y_rotated = (coord.x * sin_angle) + (coord.y * cos_angle);
		auto coord_z_rotated = coord.z;

		return Coordxyz(coord_x_rotated, coord_y_rotated, coord_z_rotated);
	};

	const double PI = 3.1415926;
	auto angle_of = [PI](const Coordxyz v1, const Coordxyz v2)->double
	{
		if (v1.length() < DBL_MIN || v2.length() < DBL_MIN)
			return 0.0;
		// Angle from v1 to v2.
		double angle = acos((v1.x * v2.x + v1.y * v2.y + v1.z * v2.z) / v1.length() / v2.length()) * 180 / PI;
		if (v1.cross_product(v2).z < 0)//need anticlockwise rotate(right hand).
			angle = 360 - angle;

		return angle;
	};

	//////////////////////////////////////////////////////////////////
	// Rotate branch normal to (0, 0, 1), so that branch lies on XOY.

	auto stem_direction_remote_1 = stem_1[0];// Stem direction : from root to first leaf.
	auto stem_direction_remote_2 = stem_2[0];// Stem direction : from root to first leaf.
	auto branch_normal = stem_direction_remote_1.cross_product(stem_direction_remote_2);// Cross product.

	//Let stem_1 is on clockwise direction of stem_2(below after rotation).
	if (branch_normal.z < 0)
	{
		branch_normal = -branch_normal;
		std::swap(stem_1, stem_2);
	}

	// calculate angle of rotation with x.
	auto _yz_projection = branch_normal;
	_yz_projection.x = 0.0;
	auto _rotate_target = Coordxyz(0.0, 0.0, 1.0);// Target direction.

	auto _x_rotate_angle = angle_of(_yz_projection.normalize(), _rotate_target.normalize());
	auto _x_rotate_angle_radian = _x_rotate_angle * PI / 180.0;

	// rotate with x.
	for (int i = 0; i < stem_1.size(); ++i)
	{
		stem_1[i] = coord_after_rotate_theta_with_X(stem_1[i], _x_rotate_angle_radian);
	}
	for (int i = 0; i < stem_2.size(); ++i)
	{
		stem_2[i] = coord_after_rotate_theta_with_X(stem_2[i], _x_rotate_angle_radian);
	}

	// calculate angle of rotation with y.
	auto _xz_projection = coord_after_rotate_theta_with_X(branch_normal, _x_rotate_angle_radian);
	_xz_projection.y = 0.0;// y value should already be 0.0.

	auto _y_rotate_angle = angle_of(_xz_projection.normalize(), _rotate_target.normalize());
	auto _y_rotate_angle_radian = _y_rotate_angle * PI / 180.0;

	// rotate with y.
	for (int i = 0; i < stem_1.size(); ++i)
	{
		stem_1[i] = coord_after_rotate_theta_with_Y(stem_1[i], _x_rotate_angle_radian);
	}
	for (int i = 0; i < stem_2.size(); ++i)
	{
		stem_2[i] = coord_after_rotate_theta_with_Y(stem_2[i], _x_rotate_angle_radian);
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// Rotate branch direction to (1, 0, 0).

	// Use local direction as branch direction.
	auto stem_direction_1 = stem_1[stem_1.size() - 2];// Stem direction : from root to child node.
	auto stem_direction_2 = stem_2[stem_2.size() - 2];// Stem direction : from root to child node.
	auto branch_direction_p = stem_direction_1 + stem_direction_2;
	branch_direction_p.z = 0.0;// already equals 0.

	//auto branch_target_direction = Coordxyz(1.0, 0.0, 0.0);

	auto _z_rotate_angle = angle_of(branch_direction_p, branch_target_direction);
	auto _z_rotate_angle_radian = _z_rotate_angle * PI / 180.0;


	//cout << "Branch direction adjusted." << endl;

	// Rotate two stems.
	for (int i = 0; i < stem_1.size(); ++i)
	{
		//cout << "Before rotate : " << getCoordxyz(branch.branch_nodes[i]) << endl;
		auto coord_after_rotate = coord_after_rotate_theta_with_Z(stem_1[i], _z_rotate_angle_radian);
		stem_1[i] = coord_after_rotate;
		//cout << "After rotate : " << coord_after_rotate << endl;
	}
	for (int i = 0; i < stem_2.size(); ++i)
	{
		//cout << "Before rotate : " << getCoordxyz(branch.branch_nodes[i]) << endl;
		auto coord_after_rotate = coord_after_rotate_theta_with_Z(stem_2[i], _z_rotate_angle_radian);
		stem_2[i] = coord_after_rotate;

		//cout << "After rotate : " << coord_after_rotate << endl;
	}
}

bool create_single_branch_code(const Branch & branch, Branch& created_branch, const int globalID)
{
	int count_stem_1, count_stem_2;
	count_stem_1 = branch.stems[0].size();
	count_stem_2 = branch.stems[1].size();

	// Interpolate values when units num is smaller than STEM_SAMPLE_POINT_NUM.
	auto interpolate_stem = [](Stem& interpolated_stem, Stem stem)
	{
		//cout << stem.size() << endl;

		int count = STEM_SAMPLE_POINT_NUM - stem.size();// Total interpolation count.
		//cout << count << endl;
		while (true)
		{
			interpolated_stem.clear();
			size_t j = 1;
			for (; j < stem.size(); j++)
			{
				interpolated_stem.push_back(stem.at(j - 1));
				interpolated_stem.push_back((stem.at(j) + stem.at(j - 1))/2.0f);
				count--;
				if(count==0)
					break;
			}
			if (count == 0)// Finished in for loop, collect all rest.
			{
				for (size_t k = j; k < stem.size(); k++)// collect rest.
				{
					interpolated_stem.push_back(stem.at(k));
				}
				break;
			}
			else
			{
				interpolated_stem.push_back(stem.at(stem.size() - 1));// If not collecte finish in for-loop, should also push the last one in stem, start next round.
				std::swap(interpolated_stem, stem);// Use interpolated stem to interpolate in next round.
			}
		}

		//if(interpolated_stem.size()!=20)
		//	cout << "interpolated to " << interpolated_stem.size() << endl;

	};

	auto down_sample_stem = [](Stem& down_sampled_stem, Stem stem)
	{
		down_sampled_stem.clear();
		auto step = stem.size() / STEM_SAMPLE_POINT_NUM;
		auto rest = stem.size() % STEM_SAMPLE_POINT_NUM;
		for (size_t i = 0; i < (STEM_SAMPLE_POINT_NUM - rest); ++i)
		{
			down_sampled_stem.push_back(stem.at(i*step));
		}
		down_sampled_stem.insert(down_sampled_stem.end(), stem.end() - rest, stem.end());

		//if(down_sampled_stem.size()!=20)
		//	cout << "down sampled to " << down_sampled_stem.size() << endl;

	};

	// For stem 1.
	Stem stem_1;
	if (count_stem_1 < STEM_SAMPLE_POINT_NUM)
	{
		interpolate_stem(stem_1, branch.stems[0]);
	}
	else if(count_stem_1 > STEM_SAMPLE_POINT_NUM)
	{
		down_sample_stem(stem_1, branch.stems[0]);
	}
	else
	{
		stem_1 = branch.stems[0];
	}

	// For stem 2.
	Stem stem_2;
	if (count_stem_2 < STEM_SAMPLE_POINT_NUM)
	{
		interpolate_stem(stem_2, branch.stems[1]);
	}
	else if (count_stem_2 > STEM_SAMPLE_POINT_NUM)
	{
		down_sample_stem(stem_2, branch.stems[1]);
	}
	else
	{
		stem_2 = branch.stems[1];
	}

	created_branch.stems[0] = std::move(stem_1);
	created_branch.stems[1] = std::move(stem_2);

	created_branch.stm_scale_1 = branch.stm_scale_1;
	created_branch.stm_scale_2 = branch.stm_scale_2;
	created_branch.angle = branch.angle;

	return true;
}

bool read_branch_from_br(const QString & br_file_path, Branch & branch)
{
	QFile qf(br_file_path);	

	if (!qf.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		v3d_msg(QString("open file [%1] failed!").arg(br_file_path));
		return false;
	}

	char buf[1000];

	// X-main axis version.
	//Stem stem;
	//stem.resize(20);// Get places for coord values.

	//// stem_1.
	//qf.readLine(buf, sizeof(buf));// skip root line.
	//stem[19] = std::move(Coordxyz(0.f, 0.f, 0.f));// root as 0, 0, 0.

	//float pre_x = 0.f;// x values for step, so accumulate up.
	//for (size_t i = 1; i < 20; i++)
	//{
	//	qf.readLine(buf, sizeof(buf));

	//	auto xyz_s = QString("%1").arg(buf).split(' ');
	//	Coordxyz xyz(xyz_s.at(0).toFloat() + pre_x, xyz_s.at(1).toFloat(), xyz_s.at(2).toFloat());
	//	pre_x += xyz_s.at(0).toFloat();
	//	stem[19 - i] = xyz;
	//}
	//branch.stems[0].swap(stem);
	////cout << branch.stems[0].size() << endl;

	//stem.resize(20);
	//stem[19] = std::move(Coordxyz(0.f, 0.f, 0.f));// root as 0, 0, 0.
	//qf.readLine(buf, sizeof(buf));// skip root line.
	//pre_x = 0.f;
	//for (size_t i = 1; i < 20; i++)
	//{
	//	qf.readLine(buf, sizeof(buf));

	//	auto xyz_s = QString("%1").arg(buf).split(' ');
	//	Coordxyz xyz(xyz_s.at(0).toFloat() + pre_x, xyz_s.at(1).toFloat(), xyz_s.at(2).toFloat());
	//	pre_x += xyz_s.at(0).toFloat();
	//	stem[19 - i] = xyz;
	//}
	//branch.append(stem);

	Stem stem;
	stem.resize(20);// Get places for coord values.

	// stem_1.
	qf.readLine(buf, sizeof(buf));// skip root line.
	stem[19] = std::move(Coordxyz(0.f, 0.f, 0.f));// root as 0, 0, 0.

	float pre_x = 0.f;// x values for step, so accumulate up.
	float pre_y = 0.f;// x values for step, so accumulate up.
	float pre_z = 0.f;// x values for step, so accumulate up.
	for (size_t i = 1; i < 20; i++)
	{
		qf.readLine(buf, sizeof(buf));

		auto xyz_s = QString("%1").arg(buf).split(' ');
		//Coordxyz xyz(xyz_s.at(0).toFloat() + pre_x, xyz_s.at(1).toFloat() + pre_y, xyz_s.at(2).toFloat() + pre_z);
		Coordxyz xyz(xyz_s.at(0).toFloat(), xyz_s.at(1).toFloat(), xyz_s.at(2).toFloat());

		pre_x += xyz_s.at(0).toFloat();
		pre_y += xyz_s.at(1).toFloat();
		pre_z += xyz_s.at(2).toFloat();
		stem[19 - i] = xyz;
	}
	branch.stems[0].swap(stem);
	//cout << branch.stems[0].size() << endl;

	//stem.resize(20);
	//stem[19] = std::move(Coordxyz(0.f, 0.f, 0.f));// root as 0, 0, 0.
	//qf.readLine(buf, sizeof(buf));// skip root line.
	//pre_x = 0.f;
	//pre_y = 0.f;
	//pre_z = 0.f;
	//for (size_t i = 1; i < 20; i++)
	//{
	//	qf.readLine(buf, sizeof(buf));

	//	auto xyz_s = QString("%1").arg(buf).split(' ');
	//	Coordxyz xyz(xyz_s.at(0).toFloat() + pre_x, xyz_s.at(1).toFloat() + pre_y, xyz_s.at(2).toFloat() + pre_z);
	//	pre_x += xyz_s.at(0).toFloat();
	//	pre_y += xyz_s.at(1).toFloat();
	//	pre_z += xyz_s.at(2).toFloat();
	//	stem[19 - i] = xyz;
	//}
	//branch.append(stem);

	return true;
}

bool save_single_branch_picture(const Branch & branch, const QString& file_path)
{
	///////////////////////////////////////////////////////////////////////////////
	// First calculate branch size.

	int MAX_X, MAX_Y;
	int MIN_X, MIN_Y;
	MAX_X = MAX_Y = INT_MIN;
	MIN_X = MIN_Y = INT_MAX;
	// For two stems.
	for (int i = 0; i < branch.stems[0].size(); ++i)
	{
		// record branch boundary.
		auto& cur = branch.stems[0].at(i);
		if (cur.x > MAX_X)
			MAX_X = cur.x;
		else if (cur.x < MIN_X)
			MIN_X = cur.x;
		if (cur.y > MAX_Y)
			MAX_Y = cur.y;
		else if (cur.y < MIN_Y)
			MIN_Y = cur.y;
	}
	for (int i = 0; i < branch.stems[1].size(); ++i)
	{

		// record branch boundary.
		auto& cur = branch.stems[1].at(i);
		if (cur.x > MAX_X)
			MAX_X = cur.x;
		else if (cur.x < MIN_X)
			MIN_X = cur.x;
		if (cur.y > MAX_Y)
			MAX_Y = cur.y;
		else if (cur.y < MIN_Y)
			MIN_Y = cur.y;
	}

	int x_size = (MAX_X > -MIN_X ? MAX_X : -MIN_X);
	//x_size = 2 * x_size + 1;
	int y_size = (MAX_Y > -MIN_Y ? MAX_Y : -MIN_Y);
	//y_size = 2 * y_size + 1;

	auto x_scale = float(BRANCH_SIZE / x_size);
	auto y_scale = float(BRANCH_SIZE / y_size);

	//cout << "X size : " << x_size << endl;
	//cout << "Y size : " << y_size << endl;

	///////////////////////////////////////////////////////////////////////////////
	// Create bmp file for visualization.

	if (!create_branch_bmp(branch, x_scale, y_scale, file_path))
	{
		cout << "Create branch bmp failed." << endl;
		return false;
	}
} 

bool save_single_branch_swc(const Branch & branch, const QString & file_path)
{
	/*From a series of NeuronSWC save a swc file*/
	FILE * fp = fopen(file_path.toLatin1(), "wt");
	fprintf(fp, "##n,type,x,y,z,radius,parent\n");
	int id = 1;
	for (int i = 0; i < branch.stems[0].size(); i++)
	{
		auto cur = branch.stems[0].at(i);
		fprintf(fp, "%ld %d %5.3f %5.3f %5.3f %5.3f %ld\n",
			id, 1, cur.x, cur.y, cur.z, 0.5, (i!= 19 ? id+1 : -1));
		++id;
	}
	for (int i = 0; i < branch.stems[1].size(); i++)
	{
		auto cur = branch.stems[1].at(i);
		fprintf(fp, "%ld %d %5.3f %5.3f %5.3f %5.3f %ld\n",
			id, 1, cur.x, cur.y, cur.z, 0.5, (i != 19 ? id+1 : -1));
		++id;
	}
	cout << "Neuron saved." << endl;
	fclose(fp);
	return true;
}

bool create_branch_bmp(const Branch & branch, float x_scale, float y_scale, const QString & bmp_file)
{
	// Define BMP Size
	const int height = BRANCH_SIZE * 2 + 1;
	const int width = BRANCH_SIZE * 2 + 1;
	const int size = height * width * 4;
	double x, y;
	int index;

	// Part.1 Create Bitmap File Header
	BITMAPFILEHEADER fileHeader;

	fileHeader.bfType = 0x4D42;
	fileHeader.bfReserved1 = 0;
	fileHeader.bfReserved2 = 0;
	fileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + size;
	fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	// Part.2 Create Bitmap Info Header
	BITMAPINFOHEADER bitmapHeader = { 0 };

	bitmapHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapHeader.biHeight = height;
	bitmapHeader.biWidth = width;
	bitmapHeader.biPlanes = 3;
	bitmapHeader.biBitCount = 32;// 4 bytes
	bitmapHeader.biSizeImage = size;
	bitmapHeader.biCompression = 0; //BI_RGB

									// Part.3 Create Data
	BYTE *bits = (BYTE *)malloc(size);

	// Clear
	memset(bits, 0xFF, size);

	for (int i = 0; i < branch.stems[0].size(); ++i)
	{
		int x = int(branch.stems[0].at(i).x * x_scale) + BRANCH_SIZE;
		int y = int(branch.stems[0].at(i).y * y_scale) + BRANCH_SIZE;
		//cout << x << " " << y << endl;
		if (x < width && x>0 && y > 0 && y < height)
		{
			index = (int)y * width * 4 + (int)x * 4;
			float z = branch.stems[0].at(i).z;
			char* ptr = (char*)&z;

			// RGBA saves a float value.
			bits[index + 0] = *ptr;
			bits[index + 1] = *(ptr + 1);
			bits[index + 2] = *(ptr + 2);
			bits[index + 3] = *(ptr + 3);
		}

	}

	// Write to file
	FILE *output = fopen((bmp_file+"_1.bmp").toLatin1(), "wb");
	if (output == NULL)
	{
		printf("Cannot open file!\n");
		return false;
	}
	else
	{
		fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, output);
		fwrite(&bitmapHeader, sizeof(BITMAPINFOHEADER), 1, output);
		fwrite(bits, size, 1, output);
		fclose(output);
	}

	memset(bits, 0xFF, size);

	for (int i = 0; i < branch.stems[1].size(); ++i)
	{
		int x = int(branch.stems[1].at(i).x * x_scale) + BRANCH_SIZE;
		int y = int(branch.stems[1].at(i).y * y_scale) + BRANCH_SIZE;
		//cout << x << " " << y << endl;
		if (x < width && x>0 && y > 0 && y < height)
		{
			index = (int)y * width * 4 + (int)x * 4;
			float z = branch.stems[1].at(i).z;
			char* ptr = (char*)&z;

			// RGBA saves a float value.
			bits[index + 0] = *ptr;
			bits[index + 1] = *(ptr + 1);
			bits[index + 2] = *(ptr + 2);
			bits[index + 3] = *(ptr + 3);

		}

	}

	// Write to file
	output = fopen((bmp_file + "_2.bmp").toLatin1(), "wb");
	if (output == NULL)
	{
		printf("Cannot open file!\n");
		return false;
	}
	else
	{
		fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, output);
		fwrite(&bitmapHeader, sizeof(BITMAPINFOHEADER), 1, output);
		fwrite(bits, size, 1, output);
		fclose(output);
	}

	return true;
}

bool save_global_branch_dataset(const QString path, branch_map_t& branch_map, branch_sequence_vector& sequences)
{
	if (branch_map.size() == 0) { cout << "Branch dataset is empty!"; return false; }
	if (sequences.size() == 0) { cout << "Sequences is empty!"; return false; }
	FILE * fp;
	fp = fopen(path.toLatin1(), "w");
	cout << "Open file :" << qPrintable(path) << endl;
	if (fp == NULL)
	{
		cout << "ERROR! Can't open file " << qPrintable(path) << endl;
		return false;
	}

	cout << "Saving branches begin." << endl;

	branch_id_map_t branch_id_map;// map every branch id(neuron id + branch root node id) to a global index. 
	int id = 0;// Set each with branch with a id.

	// For each branch; 
	for (auto it = branch_map.begin(); it != branch_map.end(); ++it)
	{
		if ((*it).second.stems[0].size() == 0 || (*it).second.stems[1].size() == 0)
		{
			cout << "empty" << endl;
			//cout << get<0>((*it).first)<<":"<<get<1>((*it).first) << endl;
			continue;// Empty skip.
		}
		branch_id_map.insert(make_pair((*it).first, id));
		//fprintf(fp, "#BRANCH: %d %d\n", get<0>((*it).first), get<1>((*it).first));
		fprintf(fp, "#BRANCH: %s\n", (*it).first);
		fprintf(fp, "#ID: %d\n", id);
		Branch& branch = (*it).second;
		fprintf(fp, "#STMSCALE: %f\n", branch.stm_scale_1);
		for (int i = branch.stems[0].size() - 1; i >= 0; --i)// Reverse to save as root to leaf.
		{
			fprintf(fp, "#COORDS: %lf %lf %lf\n", branch.stems[0].at(i).x, branch.stems[0].at(i).y, branch.stems[0].at(i).z);
		}
		fprintf(fp, "####\n");// Stem finish.
		fprintf(fp, "#STMSCALE: %f\n", branch.stm_scale_2);
		for (int i = branch.stems[1].size() - 1; i >= 0; --i)// Reverse to save as root to leaf.
		{
			fprintf(fp, "#COORDS: %lf %lf %lf\n", branch.stems[1].at(i).x, branch.stems[1].at(i).y, branch.stems[1].at(i).z);
		}
		fprintf(fp, "####\n");// Another stem finish.

		id++;

	}
	cout << "total neuron branch num is :" << id << endl;

	cout << "Saving branch sequences begin." << endl;
	cout << "total neuron sequence num is :" << sequences.size() << endl;

	for (int i = 0; i<sequences.size();i++)
	{
		//fprintf(fp, "#SEQ %d\n", branch_map.find(sequences[i].at(sequences[i].size()-1))->second.type);
		fprintf(fp, "#SEQ %d\n", branch_map.find(sequences[i].at(sequences[i].size()-1))->second.type);
		for (int j = 0; j < sequences[i].size(); j++)
		{
			auto d = branch_id_map.find(sequences[i].at(j));
			if (d == branch_id_map.end())
			{
				//cout << (sequences[i].at(j)) << endl;
				fprintf(fp, "#BID %d\n", -1);
			}
			else
				fprintf(fp, "#BID %d\n", (*d).second);
		}
	}

	cout << "Branch dataset saved at path:" << qPrintable(path) << endl;
	fclose(fp);

	return true;
}

bool save_global_branch_code(const QString path, branch_map_t & branch_map, branch_sequence_vector& sequences)
{
	if (branch_map.size() == 0) { cout << "Branch dataset is empty!"; return false; }
	if (sequences.size() == 0) { cout << "Sequences is empty!"; return false; }
	FILE * fp;
	cout << qPrintable(path) << endl;
	fp = fopen(path.toLatin1(), "w");
	cout << "Open file :" << qPrintable(path) << endl;
	if (fp == NULL)
	{
		cout << "ERROR! Can't open file " << qPrintable(path) << endl;
		return false;
	}

	cout << "Saving branche codes begin." << endl;

	int id = 0;// Set each with branch with a id.

	Branch cur;
	// For each branch; 
	for (auto it = branch_map.begin(); it != branch_map.end(); ++it)
	{
		//cout << id << endl;
		if ((*it).second.stems[0].size() == 0 || (*it).second.stems[1].size() == 0)
		{
			continue;// Empty skip.
		}

		if (!create_single_branch_code((*it).second, cur, id))
			cout << "create branch code Error for branch : [" << id <<"]." << endl;

		fprintf(fp, "#BRANCH: %d\n", id);
		fprintf(fp, "#ANGLE: %f\n", cur.angle);
		fprintf(fp, "#STEM: %f\n", cur.stm_scale_1);
		double cur_x, cur_y, cur_z;
		double pre_x = cur.stems[0][cur.stems[0].size() - 1].x;// Coords is fliped in vector.
		double pre_y = cur.stems[0][cur.stems[0].size() - 1].y;// Coords is fliped in vector.
		double pre_z = cur.stems[0][cur.stems[0].size() - 1].z;// Coords is fliped in vector.
		for (int i = cur.stems[0].size() - 1; i >= 0; --i)
		{
			cur_x = cur.stems[0][i].x;
			cur_y = cur.stems[0][i].y;
			cur_z = cur.stems[0][i].z;
			//fprintf(fp, "#DATA: %lf %lf %lf\n", cur_x - pre_x, cur_y - pre_y, cur_z - pre_z);// DATA format : x_step, y coord, z coord.
			fprintf(fp, "#DATA: %lf %lf %lf\n", cur_x, cur_y, cur_z);// DATA format : x_step, y coord, z coord.

			pre_x = cur_x;
			pre_y = cur_y;
			pre_z = cur_z;
		}
		fprintf(fp, "#ENDSTEM\n");

		fprintf(fp, "#STEM: %f\n", cur.stm_scale_2);

		pre_x = cur.stems[1][cur.stems[1].size() - 1].x;
		pre_y = cur.stems[1][cur.stems[1].size() - 1].y;
		pre_z = cur.stems[1][cur.stems[1].size() - 1].z;
		for (int i = cur.stems[1].size() - 1; i >= 0; --i)
		{
			cur_x = cur.stems[1][i].x;
			cur_y = cur.stems[1][i].y;
			cur_z = cur.stems[1][i].z;
			//fprintf(fp, "#DATA: %lf %lf %lf\n", cur_x - pre_x, cur_y - pre_y, cur_z - pre_z);
			fprintf(fp, "#DATA: %lf %lf %lf\n", cur_x, cur_y, cur_z);
			pre_x = cur_x;
			pre_y = cur_y;
			pre_z = cur_z;
		}

		fprintf(fp, "#ENDSTEM\n");
		fprintf(fp, "#BRANCHEND\n");


		id++;
	}

	cout << "Branch codes saved at path:" << qPrintable(path) << endl;

	fclose(fp);

	return true;
}

void assemble_neuron_tree_from(const QString gen_file_path)
{
	QString neuron_tree_file = gen_file_path.mid(0, gen_file_path.lastIndexOf(".")) + ".swc";

	auto soma_branch = collect_neuron_soma_branch(gen_file_path);

	std::vector<vector<BranchInfo>> binfo_vec = collect_neuron_branches(gen_file_path);
	vector<NeuronSWC> neuron = adjust_branches(soma_branch, binfo_vec);

	save_swc_file(neuron_tree_file, neuron);
	cout << "Neuron swc file saved at [" << qPrintable(neuron_tree_file) << "].";
}

std::vector<vector<BranchInfo>> collect_neuron_branches(const QString file_path)
{
	//From .gen file make up a Branch vector;
	cout << qPrintable(file_path) << endl;
	QFile qf(file_path);
	//SomaBranch sob;
	vector<vector<BranchInfo>> total_branch_vector;
	vector<BranchInfo> branch_vector;

	if (!qf.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		v3d_msg(QString("open file [%1] failed!").arg(file_path));
		return total_branch_vector;
	}
	while (!qf.atEnd())
	{
		char buf[2000];
		qf.readLine(buf, sizeof(buf));
		if (buf[0] != '#')
		{
			BranchInfo branch;

			QString str = QString("%1").arg(buf);
			QStringList factor_infos = str.split(" ");

			if (factor_infos.size() == 1) continue;//ship empty line;

			int branch_id = factor_infos[0].toInt();
			int parent_branch_id = factor_infos[1].toInt();

			float x_coord = 0.f;
			for (size_t i = 0; i < 19; i++)
			{
				NeuronSWC node;
				if (i == 0)
				{
					node.x = 0.f;
					node.y = 0.f;
					node.z = 0.f;
				}
				else
				{
					node.x = factor_infos[i * 3 + 2].toFloat();
					node.y = factor_infos[i * 3 + 3].toFloat();
					node.z = factor_infos[i * 3 + 4].toFloat();
				}

				node.n = i;
				node.r = 0.5f;
				node.parent = i - 1;
				branch.branch_nodes.push_back(node);
			}
			// Last one as edge coord.
			branch.branch_edge_coord[0] = Coordxyz(factor_infos[19 * 3 + 2].toFloat(), factor_infos[19 * 3 + 3].toFloat(), factor_infos[19 * 3 + 4].toFloat());
			branch.branch_end[0] = 18;// The branch end node id is 18.

			x_coord = 0.f;
			for (size_t i = 21; i < 39; i++)//20 is root.
			{
				NeuronSWC node;
				node.n = i;
				x_coord = factor_infos[i * 3 + 2].toFloat();
				node.x = x_coord;
				node.y = factor_infos[i * 3 + 3].toFloat();
				node.z = factor_infos[i * 3 + 4].toFloat();
				node.r = 0.5f;
				node.parent = (i==21 ? 0 : i - 1);
				branch.branch_nodes.push_back(node);
			}
			branch.branch_edge_coord[1] = Coordxyz(factor_infos[39 * 3 + 2].toFloat(), factor_infos[39 * 3 + 3].toFloat(), factor_infos[39 * 3 + 4].toFloat());
			branch.branch_end[1] = 38;

			auto getCoordxyz = [](const NeuronSWC node) { return Coordxyz(node.x, node.y, node.z); };
			//Child branch direction;
			//branch.child_branch_direction[0] = branch.branch_edge_coord[0] - getCoordxyz(branch.branch_nodes[18]);
			//branch.child_branch_direction[1] = branch.branch_edge_coord[1] - getCoordxyz(branch.branch_nodes[branch.branch_nodes.size()-1]);// The branch have 37 nodes and 2 edge coords.
			branch.child_branch_direction[0] = branch.branch_edge_coord[0];
			branch.child_branch_direction[1] = branch.branch_edge_coord[1];// The branch have 37 nodes and 2 edge coords.

			// branch nodes stores in reverse order : leaf_1 -> leaf_0 -> root.
			std::reverse(branch.branch_nodes.begin(), branch.branch_nodes.end());

			cout << "Branch collected." << endl;

			//collect rest;
			branch.branch_id = branch_id;
			branch.branch_parent = parent_branch_id;

			//collect in vector;
			branch_vector.push_back(branch);
		}
		else if (buf[0] == '#' && buf[1] == 'G' && buf[2] == 'E' && buf[3] == 'N' && buf[4] == 'E'&& buf[5] == 'N'&& buf[6] == 'D')
		{
			total_branch_vector.push_back(branch_vector);
			branch_vector.clear();
		}
	}

	return total_branch_vector;
}