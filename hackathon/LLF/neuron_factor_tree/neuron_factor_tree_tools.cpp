/*neuron_factor_tree_tools.cpp
 *funcs for neuron_factor_tree struct.
 */
#include "neuron_factor_tree_plugin.h"
#include <QtGlobal>

void calculate_branches(const NeuronTree &neuron, vector<V3DLONG> &branches)
{
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
						  //construct neuronfactors
	for (auto it = nft_node_set.begin(); it != nft_node_set.end(); it++)//So the constructed neuronFactor tree doesn't have a root which has just one child(don't in nft_node_set)
	{
		V3DLONG current = *it;
		unordered_map<string, double> feature;
		vector<double> feature_vector;
		//calculate features at branch node besides leaf and root;
		if (current != 0 && branches[current] != 0) calculate_features_at_node(neuron, current, nft_parent_map, nft_child_map, nft_sibling_map, feature, feature_vector);//calculate features at each nft node except soma node;
		NeuronFactor nf = NeuronFactor(1, current, neuron.listNeuron[current].x, neuron.listNeuron[current].y, neuron.listNeuron[current].z, feature, feature_vector, nft_parent_map[current], nft_child_map[current], nft_sibling_map[current]);
		nfs[current] = nf;
		if (nft_parent_map[current] == -1) nft_root = nf;
	}
	cout << "Get NeuronFactor Tree root at nft_node:" << nft_root.label << "." << endl;
	NeuronFactorTree nft = NeuronFactorTree(1, nft_root, nfs);
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
				 "#nf info:V3DLONG label;float x, y, z;unordered_map<string, double> feature;parent;child;sibling;\n"
				 "#nf feature values:For root node and leaf node DONOT have any values;\n");
	//feature names
	fprintf(fp, "#FEATURES:fea_dist_c1 fea_dist_c2 fea_dist_soma fea_bif_angle_remote fea_path_len_1 fea_contraction_1 fea_diameter_avg_1 fea_area_avg_1 fea_surface_1 fea_volume_1 "
		"fea_path_len_2 fea_contraction_2 fea_diameter_avg_2 fea_area_avg_2 fea_surface_2 fea_volume_2 fea_bif_angle_local fea_tilt_angle_remote fea_tilt_angle_local fea_torque_remote fea_torque_local\n");//define feature names;
	//save level
	fprintf(fp, "#LEVEL:%d\n", nft.level);
	//save the xyz coordinates in swc format, so that file could display in vaa3d; 
	for (auto it = nft.neuron_factors.begin(); it!=nft.neuron_factors.end();it++)
	{
		//cout<<it->second.label<<it->second.x<< it->second.y<<it->second.z<<it->second.parent<<endl;
		fprintf(fp, "%ld %d %5.3f %5.3f %5.3f %5.3f %ld %ld %ld\n",
			it->second.label, 2, it->second.x, it->second.y, it->second.z, 0.05, it->second.parent, it->second.child, it->second.sibling);
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
			if (buf[1]=='L'&&buf[2]=='E'&&buf[3]=='V'&&buf[4]=='E'&&buf[5]=='L'&&buf[6]==':') nft.level = QString("%1").arg(buf + 7).toInt();
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
	
	cout<<"NeuronFactor Tree serializated."<<endl;
	cout<<"Got sequences:"<<nfseqs.size()<<endl;

	return nfseqs;
}

void save_neuron_factor_sequences(const NeuronFactorSequences &nfseqs, const QString path)
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
		"fea_path_len_2 fea_contraction_2 fea_diameter_avg_2 fea_area_avg_2 fea_surface_2 fea_volume_2 fea_bif_angle_local fea_tilt_angle_remote fea_tilt_angle_local fea_torque_remote fea_torque_local\n");//define feature names;

	//save level
	fprintf(fp, "#LEVEL: %d\n", nfseqs[0].front().level);

	//a block for a sequence; 
	for (auto seq = nfseqs.begin(); seq != nfseqs.end(); seq++)
	{
		fprintf(fp, "#SEQ_SIZE: %d\n",(*seq).size());
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
					fprintf(fp, " %5.3f", f);
				}
				fprintf(fp, "\n");
			}
		}
		fprintf(fp, "\n");		
	}
	cout << "NeuronFactor Tree saved at path:" << qPrintable(path) << endl;
	fclose(fp);
}

void calculate_features_at_node(const NeuronTree neuron, const V3DLONG current, const map<V3DLONG,V3DLONG> nft_pa_map, const map<V3DLONG,V3DLONG> nft_ch_map, const map<V3DLONG,V3DLONG> nft_sib_map,unordered_map<string,double> &feature, vector<double> &feature_vector)
{
	cout << "Calculating features at node:" << current << endl;
	V3DLONG pa = nft_pa_map.at(current);
	V3DLONG ch = nft_ch_map.at(current);

	NeuronSWC node_cur = neuron.listNeuron[current];
	NeuronSWC soma = neuron.listNeuron[0];
	NeuronSWC node_pa, node_ch;//Let the node_ch defines a closer node, if not, swap latter;
	node_pa = neuron.listNeuron[pa];
	node_ch = neuron.listNeuron[ch];

	//Lambda func to get a neuron SWC's xyz coordinates;calculate dist;calculate angle;
	const double PI = 3.1415926;
	auto xyz = [](const decltype(node_cur) node) { return Coordxyz(node.x, node.y, node.z); };
	auto dist = [](const Coordxyz c1, const Coordxyz c2)->float { auto sub = c2 - c1; return sqrt(sub.x*sub.x + sub.y*sub.y + sub.z*sub.z); };
	auto angle = [PI](const Coordxyz v1, const Coordxyz v2) {return acos((v1.x * v2.x + v1.y * v2.y + v1.z * v2.z) / sqrt(v1.x * v1.x + v1.y * v1.y + v1.z * v1.z) / sqrt(v2.x * v2.x + v2.y * v2.y + v2.z * v2.z))*180/PI; };
	auto acute_angle = [PI](const Coordxyz v1, const Coordxyz v2)->double { double angle = acos((v1.x * v2.x + v1.y * v2.y + v1.z * v2.z) / sqrt(v1.x * v1.x + v1.y * v1.y + v1.z * v1.z) / sqrt(v2.x * v2.x + v2.y * v2.y + v2.z * v2.z)) * 180 / PI; return angle > 90.0 ? (180.0 - angle) : angle; };
	//Features
	double fea_dist_c1;//Feature:euc_dist between cur and it's nearer child;
	double fea_dist_c2;//Feature:euc_dist between cur and it's further child;
	double fea_dist_soma;//Feature:euc_dist between cur and soma;
	double fea_bif_angle_remote;//Feature:bifurcation angle among current and two nft children nodes; 
	double fea_path_len_1;//Feature:the path between current and the closer nft child node;
	double fea_contraction_1;//Feature:ratio between euc dist of a branch and its path len;
	double fea_diameter_avg_1;//Feature:average diameter among a branch;
	double fea_area_avg_1;//Fature:average area among a branch;
	double fea_surface_1;//Feature:surface area of a branch;
	double fea_volume_1;//Feature:volume of a branch;
	double fea_path_len_2;
	double fea_contraction_2;
	double fea_diameter_avg_2;
	double fea_area_avg_2;
	double fea_surface_2;
	double fea_volume_2;
	double fea_bif_angle_local;//Feature:bifurication angle among current and two direct children nodes;
	double fea_tilt_angle_remote;
	double fea_tilt_angle_local;
	//Features only calculate when nft parent is a branch
	double fea_torque_remote = 0.0;//Feature:angle between two bifu planes(remote);
	double fea_torque_local = 0.0;//Feature:angle between two bifu planes(local);

	//features between node:dist and angle remote;
	auto node_ch_s = neuron.listNeuron[nft_sib_map.at(ch)];//Another child;
	auto coord = xyz(node_cur);
	auto coord_c1 = xyz(node_ch);
	auto coord_c2 = xyz(node_ch_s);
	if (dist(coord, coord_c1) > dist(coord, coord_c2))
	{
		fea_dist_c1 = dist(coord, coord_c2);//Let fea_dist_c1 record the nearer dist;
		fea_dist_c2 = dist(coord, coord_c1);

		auto t_node = node_ch;
		node_ch = node_ch_s;
		node_ch_s = t_node;//Swap to let node_ch defines the nearer child;
	}
	else
	{
		fea_dist_c1 = dist(coord, coord_c1);
		fea_dist_c2 = dist(coord, coord_c2);
	}
	fea_dist_soma = dist(xyz(soma), xyz(node_cur));//Feature:euc_dist between cur and soma;
	fea_bif_angle_remote = angle(coord_c1 - coord, coord_c2 - coord);//Feature:bifurcation angle among current and two nft children nodes; 

	//statistics to count in the recursives;
	double path_len = 0.0;
	int stems_num = 0;
	double total_diameter = 0.0;//total use stems_num to get average;
	double total_area = 0.0;
	double surface = 0.0;
	double volume = 0.0;

	//A lambda func to recursely calculate features;from nft child node to direct child node;
	auto recursion = [&path_len, &stems_num, &total_diameter, &total_area, &surface, &volume, neuron, current, dist, xyz ,PI](int &lo_ch) 
	{
		//After lambda func, lo_ch will be got, features on the path from nft child to current will be collected;
		path_len = total_diameter = total_area = surface = volume = 0.0; stems_num = 0;
		int curr = lo_ch;
		for (auto p_id = neuron.hashNeuron.value(neuron.listNeuron[curr].pn); curr != current; curr = p_id, p_id = neuron.hashNeuron.value(neuron.listNeuron[p_id].pn))
		{
			auto len = dist(xyz(neuron.listNeuron[p_id]), xyz(neuron.listNeuron[curr]));
			auto area = PI * neuron.listNeuron[p_id].radius * neuron.listNeuron[p_id].radius;
			path_len += len;//Recursively get a path length from child to current node;
			total_diameter += (neuron.listNeuron[p_id].radius * 2);
			total_area += area;
			surface += (neuron.listNeuron[p_id].radius * 2 * len);//Recursively get a stem's surface area;
			volume += area * len;
			stems_num++;
			lo_ch = curr;
		}
	};

	auto lo_ch = neuron.hashNeuron.value(node_ch.n);//Set the start node(current's nft child node);
	auto lo_ch_s = neuron.hashNeuron.value(node_ch_s.n);//lo_ch_s node_ch_s' ancestor;

	//Recursively collect features;
	recursion(lo_ch);
	fea_path_len_1 = path_len;//Feature:the path between current and the closer nft child node;
	fea_contraction_1 = fea_dist_c1/fea_path_len_1;//Feature:ratio between euc dist of a branch and its path len;
	fea_diameter_avg_1 = total_diameter / stems_num;//Feature:average diameter among a branch;
	fea_area_avg_1 = total_area / stems_num;//Fature:average area among a branch;
	fea_surface_1 = surface;//Feature:surface area of a branch;
	fea_volume_1 = volume;//Feature:volume of a branch;
	auto coord_c1_lo = xyz(neuron.listNeuron[lo_ch]);//direct child node's coordinates;

	recursion(lo_ch_s);
	fea_path_len_2 = path_len;
	fea_contraction_2 = fea_dist_c2 / fea_path_len_2;
	fea_diameter_avg_2 = total_diameter / stems_num;
	fea_area_avg_2 = total_area / stems_num;
	fea_surface_2 = surface;
	fea_volume_2 = volume;
	auto coord_c2_lo = xyz(neuron.listNeuron[lo_ch_s]);

	fea_bif_angle_local = angle(coord_c1_lo - coord, coord_c2_lo - coord);//Feature:bifurication angle among current and two direct children nodes;

	//Tilt
	auto u = coord - xyz(neuron.listNeuron[nft_pa_map.at(current)]);//A vector from current's nft parent to current;
	auto v = (coord_c1 - coord).cross_product(coord_c2 - coord);//A vector being vertical with this branch(remote);
	auto p = u - (v * (u*v) / v.length() / v.length());//Vector u's projection on the branch plane(remote);
	fea_tilt_angle_remote = 0.5 * fea_bif_angle_remote - min(acute_angle(p, (coord_c1 - coord)), acute_angle(p, (coord_c2 - coord)));
	fea_tilt_angle_remote = abs(fea_tilt_angle_remote);

	u = coord - xyz(neuron.listNeuron[neuron.hashNeuron.value(neuron.listNeuron[current].pn)]);//A vector from current's parent to current;
	v = (coord_c1_lo - coord).cross_product(coord_c2_lo - coord);//A vector being vertical with this branch(local);
	p = v * (u*v) / v.length() / v.length() - u;//Vector u's projection on the branch plane(local);
	fea_tilt_angle_local = 0.5 * fea_bif_angle_local - min(acute_angle(p, (coord_c1_lo - coord)), acute_angle(p, (coord_c2_lo - coord)));
	fea_tilt_angle_local = abs(fea_tilt_angle_local);

	//Torque only calculate when nft parent is a branch
	if (nft_pa_map.at(current) != 0) {//if nft parent is not root, means parent node is branch node;
		auto d1 = (coord_c1 - coord).cross_product(coord_c2 - coord);//A vector being vertical with this branch(remote);
		auto pa = nft_pa_map.at(current);
		auto sib = (nft_ch_map.at(pa) == current ? nft_sib_map.at(current) : nft_ch_map.at(pa));
		auto d2 = (coord - xyz(neuron.listNeuron[pa])).cross_product(xyz(neuron.listNeuron[sib]) - xyz(neuron.listNeuron[pa]));
		fea_torque_remote = acute_angle(d1, d2);//Feature:angle between two bifu planes(remote);

		d1 = (coord_c1_lo - coord).cross_product(coord_c2_lo - coord);//A vector being vertical with this branch(local);
		pa = nft_pa_map.at(current);
		sib = (nft_ch_map.at(pa) == current ? nft_sib_map.at(current) : nft_ch_map.at(pa));
		for (; neuron.hashNeuron.value(neuron.listNeuron[sib].pn) != pa; sib = neuron.hashNeuron.value(neuron.listNeuron[sib].pn));//Get parent's local child;
		auto pa_ch_lo = current;
		for (; neuron.hashNeuron.value(neuron.listNeuron[pa_ch_lo].pn) != pa; pa_ch_lo = neuron.hashNeuron.value(neuron.listNeuron[pa_ch_lo].pn));//Get parent's local child;
		d2 = (xyz(neuron.listNeuron[pa_ch_lo]) - xyz(neuron.listNeuron[pa])).cross_product(xyz(neuron.listNeuron[sib]) - xyz(neuron.listNeuron[pa]));
		fea_torque_local = acute_angle(d1, d2);//Feature:angle between two bifu planes(local);

	}

	//collect features;
	feature_vector.push_back(fea_dist_c1);
	feature_vector.push_back(fea_dist_c2);
	feature_vector.push_back(fea_dist_soma);
	feature_vector.push_back(fea_bif_angle_remote);
	feature_vector.push_back(fea_path_len_1);
	feature_vector.push_back(fea_contraction_1);
	feature_vector.push_back(fea_diameter_avg_1);
	feature_vector.push_back(fea_area_avg_1);
	feature_vector.push_back(fea_surface_1);
	feature_vector.push_back(fea_volume_1);
	feature_vector.push_back(fea_path_len_2);
	feature_vector.push_back(fea_contraction_2);
	feature_vector.push_back(fea_diameter_avg_2);
	feature_vector.push_back(fea_area_avg_2);
	feature_vector.push_back(fea_surface_2);
	feature_vector.push_back(fea_volume_2);
	feature_vector.push_back(fea_bif_angle_local);
	feature_vector.push_back(fea_tilt_angle_remote);
	feature_vector.push_back(fea_tilt_angle_local);
	feature_vector.push_back(fea_torque_remote);
	feature_vector.push_back(fea_torque_local);

	feature["fea_dist_c1"] = fea_dist_c1;
	feature["fea_dist_c2"] = fea_dist_c2;
	feature["fea_dist_soma"] = fea_dist_soma;
	feature["fea_bif_angle_remote"] = fea_bif_angle_remote;
	feature["fea_path_len_1"] = fea_path_len_1;
	feature["fea_contraction_1"] = fea_contraction_1;
	feature["fea_diameter_avg_1"] = fea_diameter_avg_1;
	feature["fea_area_avg_1"] = fea_area_avg_1;
	feature["fea_surface_1"] = fea_surface_1;
	feature["fea_volume_1"] = fea_volume_1;
	feature["fea_path_len_2"] = fea_path_len_2;
	feature["fea_contraction_2"] = fea_contraction_2;
	feature["fea_diameter_avg_2"] = fea_diameter_avg_2;
	feature["fea_area_avg_2"] = fea_area_avg_2;
	feature["fea_surface_2"] = fea_surface_2;
	feature["fea_volume_2"] = fea_volume_2;
	feature["fea_bif_angle_local"] = fea_bif_angle_local;
	feature["fea_tilt_angle_remote"] = fea_tilt_angle_remote;
	feature["fea_tilt_angle_local"] = fea_tilt_angle_local;
	feature["fea_torque_remote"] = fea_torque_remote;
	feature["fea_torque_local"] = fea_torque_local;

}

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