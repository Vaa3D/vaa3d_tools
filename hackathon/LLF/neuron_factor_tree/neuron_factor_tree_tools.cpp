/*neuron_factor_tree_tools.cpp
 *funcs for neuron_factor_tree struct.
 */
#include "neuron_factor_tree_plugin.h"
#include <QtGlobal>

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
NeuronFactorTree create_neuron_factor_tree(const NeuronTree &neuron)
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
		NeuronFactor nf_current(1, current, neuron.listNeuron[current].x, neuron.listNeuron[current].y, neuron.listNeuron[current].z, pa, ch, ch_sb, sib);

		//sib = (nft_child_map.at(pa) == current ? nft_sibling_map.at(current) : nft_child_map.at(pa));
		if (branches[current] != 0 && pa!=-1)//for leaf don,t calculate features;
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
			it->second.label, 2, it->second.x, it->second.y, it->second.z, 0.05, 
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
			//skip msl[1] which just control color;
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

NeuronFactorSequences serialization(const NeuronFactorTree nft)
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

void assemble_neuron_tree_from(const QString gen_file_path)
{
	/*Input: .gen file, constituted by neuron factor origin sequences.
	 *format:
	 *node index, node origin swc file, node origin id, node parent index.
	 */
	QString neuron_tree_file = gen_file_path.mid(0, gen_file_path.lastIndexOf(".")) + ".swc";
	auto branches = collect_neuron_factor_infos(gen_file_path);
	vector<NeuronSWC> neuron = adjust_branches(branches);
	save_swc_file(neuron_tree_file, neuron);
	cout<<"Neuron swc file saved at ["<<qPrintable(neuron_tree_file)<<"].";
}

vector<Branch> collect_neuron_factor_infos(const QString file_path)
{
	cout << qPrintable(file_path) << endl;
	QFile qf(file_path);
	vector<Branch> branch_vector;

	if (!qf.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		v3d_msg(QString("open file [%1] failed!").arg(file_path));
		return branch_vector;
	}
	while (!qf.atEnd())
	{
		char buf[1000];
		qf.readLine(buf, sizeof(buf));

		if (buf[0] != '#')
		{
			Branch branch;

			QString str = QString("%1").arg(buf); 
			QStringList factor_infos = str.split(" ");

			if (factor_infos.size() == 1) continue;//ship empty line;

			int branch_id = factor_infos[0].toInt();
			QString origin_swc = factor_infos[1];
			int origin_node = factor_infos[2].toInt();
			int parent_branch_id = factor_infos[3].toInt();

			if (origin_node == 0)
			{
				branch_vector.push_back(branch);//for root and leaf node, put a fake branch to keep vector index right; 
				continue;//skip root and leaf;
			}

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
	}

	return branch_vector;
}

//adjust the id of collected neuronSWC;
vector<NeuronSWC> adjust_branches(vector<Branch> branches)
{
	/*Input : structured NeuronSWC, vector<Branch> <- Branch <- NeuronSWC;
	 *Return : a battery of index-adjusted NeuronSWC vector<NeuronSWC>;
	 */
	vector<NeuronSWC> ret_vec;
	int index = 1;//global index;
	int parent;//current branch's parent swc node e.g. one branch end node in parent branch;
	Coordxyz parent_edge_coord;//current branch's start coord e.g. parent branch's edge coord;
	auto getCoordxyz = [](const NeuronSWC node) { return Coordxyz(node.x, node.y, node.z); };//get coord from a NeuronSWC;
	auto adjust_coord = [](NeuronSWC &node, const Coordxyz new_coord) 
	{node.x = new_coord.x, node.y = new_coord.y, node.z = new_coord.z; };//adjust NeuronSWC's coord with a Coordxyz;
	
	for(int i = 0; i < branches.size();i++)
	{
		auto &branch = branches[i];
		//cout << branch.branch_parent << endl;
		//cout << branch.branch_id << endl;
		if (branch.branch_id == -1) continue;//skip fake branch;

		if (branch.branch_parent == -1)//first branch, don't have parent branch;
		{
			parent = -1;
			parent_edge_coord = Coordxyz(0.0, 0.0, 0.0);
		}
		else
		{
			//current branch's parent branch;
			auto &parent_branch = branches[branch.branch_parent];//the current branch's parent branch;
			parent = parent_branch.branch_end[parent_branch.left_or_right];//the current branch's parent branch's end node index;
			parent_edge_coord = parent_branch.branch_edge_coord[parent_branch.left_or_right];//the current branch's start coord, record in parent branch;
			//parent_branch.left_or_right = (parent_branch.left_or_right) + 1;
		}

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
			parent = index++;
		}
		cout << "Index adjusted." << endl;

		//cout << "Branch end point" << branch.branch_end[0] << "," << branch.branch_end[1] << endl;

		if (branch.branch_parent == -1)//For first branch, don't need coords and direction adjust;
		{
			for each (auto node in branch.branch_nodes)
				ret_vec.push_back(node);
			continue;
		}

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

		
		/*Update 2019-06-09*/
		//Adjust branch direction;
		branch_root_node = branch.branch_nodes[branch.branch_nodes.size() - 1];//get current branch's root node, adjusted;
		auto coord_root = getCoordxyz(branch_root_node);//current bracnch's root coordinante;

		//Get vector from root to children;
		auto root2child1 = branch.branch_edge_coord[0] - coord_root;//A vector from root to child1;
		auto root2child2 = branch.branch_edge_coord[1] - coord_root;//A vector from root to child1;
		//uniformazition;
		/*
		root2child1 = root2child1 / root2child1.length();
		root2child2 = root2child2 / root2child2.length();*/

		//current branch's direction;
		auto branch_direction = root2child1 + root2child2;//set a branch's direction as middle of two vectors;
		//branch_direction = branch_direction / branch_direction.length();//uniformation;
		cout << "Get branch direction." << endl;

		//parent branch;
		//cout << branch.branch_parent << endl;
		auto &parent_branch = branches[branch.branch_parent];//the current branch's parent branch;
		auto branch_direction_new = parent_branch.child_branch_direction[parent_branch.left_or_right];//current branch's target direction;
		//branch_direction_new = branch_direction_new / branch_direction_new.length();//uniformation;
		cout << "Get target branch direction." << endl;

		/*Func : adjust_direction(direction, &parent_branch, &branch)*/
		//projection to XOY;
		auto branch_direction_p = branch_direction;
		branch_direction_p.z = 0.0;
		auto branch_direction_new_p = branch_direction_new;
		branch_direction_new_p.z = 0.0;

		const double PI = 3.1415926;
		auto angle_of = [PI](const Coordxyz v1, const Coordxyz v2)->double { double angle = acos((v1.x * v2.x + v1.y * v2.y + v1.z * v2.z) / v1.length() / v2.length()) * 180 / PI; return angle; };
		//cout << branch_direction_p << endl;
		//cout << branch_direction_new_p << endl;
		auto angle = angle_of(branch_direction_p, branch_direction_new_p);//the two vectors' angle;
		if (branch_direction_p.cross_product(branch_direction_new_p).z < 0)//need anticlockwise rotate;
			angle = 360-angle;
		//cout << "angle : " << angle << endl;
		auto angle_ = angle * PI / 180.0;//radian angle;
		//cout << "angle_ : " << angle_ << endl;
		auto sin_angle = sin(angle_);
		auto cos_angle = cos(angle_);
		//cout << sin_angle << " " << cos_angle << endl;
		cout << "Rotate angle calculated." << endl;

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
		for (int i =0; i < branch.branch_nodes.size(); i++)
		{
			//cout << "Before rotate : " << getCoordxyz(branch.branch_nodes[i]) << endl;
			auto coord_after_shiftting = getCoordxyz(branch.branch_nodes[i]) - shiftting_from_o2root;//target position;
			auto coord_after_rotate = coord_after_rotate_theta_with_Z(coord_after_shiftting);
			//cout << "After rotate : " << coord_after_rotate << endl;
			auto coord_target = coord_after_rotate + shiftting_from_o2root;
			adjust_coord(branch.branch_nodes[i], coord_target);
		}
		cout << "Branch direction adjusted." << endl;

		//adjust branch_edge_coord also;
		auto new_edge_coord_0 = coord_after_rotate_theta_with_Z(branch.branch_edge_coord[0] - shiftting_from_o2root) + shiftting_from_o2root;
		branch.branch_edge_coord[0] = new_edge_coord_0;
		auto new_edge_coord_1 = coord_after_rotate_theta_with_Z(branch.branch_edge_coord[1] - shiftting_from_o2root) + shiftting_from_o2root;
		branch.branch_edge_coord[1] = new_edge_coord_1;
		
		//Adjust child_branch_direction also;
		branch.child_branch_direction[0] = coord_after_rotate_theta_with_Z(branch.child_branch_direction[0]);
		branch.child_branch_direction[1] = coord_after_rotate_theta_with_Z(branch.child_branch_direction[1]);
		//cout << branch.child_branch_direction[0] << endl;
		//cout << branch.child_branch_direction[1] << endl;

		parent_branch.left_or_right = (parent_branch.left_or_right) + 1;//renew left_or_right flag;

		//cout << "Edge coord:" << branch.branch_edge_coord[0] << endl;
		//cout << "Edge coord:" << branch.branch_edge_coord[1] << endl;
		cout << "Coordinate adjusted." << endl;

		for each (auto node in branch.branch_nodes)
			ret_vec.push_back(node);

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
			p_pt->n, /*p_pt->type*/3, p_pt->x, p_pt->y, p_pt->z, p_pt->r, p_pt->pn);
	}
	cout << "Neuron saved." << endl;
	fclose(fp);
}


//USELESS func : collect_nfss
//collect a nfss file:file_path:*.nfss -> fp:features.txt
void collect_nfss(const QString file_path, FILE * fp, int &sequence_total_count, const QString class_name)
{
	fprintf(fp, qPrintable("#CLASS:" + class_name + "\n"));//start to collect one classs into file, marked by #CLASS;
	QFile qf(file_path);
	if (!qf.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		v3d_msg(QString("open file [%1] failed!").arg(file_path));
	}
	while (!qf.atEnd())
	{
		char buf[1000];
		qf.readLine(buf, sizeof(buf));

		while (buf[0] == '#'&&buf[1] == 'V'&&buf[2] == 'A'&&buf[3] == 'L'&&buf[4] == 'U'&&buf[5] == 'E'&&buf[6] == 'S'&&buf[7] == ':')
		{
			QString feature_str = QString("%1").arg(buf + 9);//skip a blank;
			fprintf(fp, qPrintable(feature_str));
			qf.readLine(buf, sizeof(buf));
		}
		if (buf[0] == '#'&&buf[1] == 'L'&&buf[2] == 'E'&&buf[3] == 'A'&&buf[4] == 'F')
		{
			fprintf(fp, "\n"); sequence_total_count++;
		}
		continue;
	}
}