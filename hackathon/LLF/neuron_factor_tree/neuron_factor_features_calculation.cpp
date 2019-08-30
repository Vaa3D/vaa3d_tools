#include "neuron_factor_tree_plugin.h"

void calculate_features_at_node(const NeuronTree neuron, NeuronFactor &current, const V3DLONG id_child, const V3DLONG id_ch_sibling, const V3DLONG id_sibling, const V3DLONG id_parent, const NeuronFactors nfs)
{
	cout << "Calculating features at node:" << current.label << endl;
	NeuronSWC node_cur = neuron.listNeuron[current.label];
	NeuronSWC soma = neuron.listNeuron[0];
	NeuronSWC node_ch, node_ch_s;//Let the node_ch defines a closer node, if not, swap latter;
	node_ch = neuron.listNeuron[id_child];
	node_ch_s = neuron.listNeuron[id_ch_sibling];

	//Lambda func to get a neuron SWC's xyz coordinates;calculate dist;calculate angle;
	const double PI = 3.1415926;
	auto xyz = [](const decltype(node_cur) node) { return Coordxyz(node.x, node.y, node.z); };
	auto dist = [](const Coordxyz c1, const Coordxyz c2)->float { auto sub = c2 - c1; return sqrt(sub.x*sub.x + sub.y*sub.y + sub.z*sub.z); };
	auto angle = [PI](const Coordxyz v1, const Coordxyz v2) {return acos((v1.x * v2.x + v1.y * v2.y + v1.z * v2.z) / sqrt(v1.x * v1.x + v1.y * v1.y + v1.z * v1.z) / sqrt(v2.x * v2.x + v2.y * v2.y + v2.z * v2.z)) * 180 / PI; };
	auto acute_angle = [PI](const Coordxyz v1, const Coordxyz v2)->double { double angle = acos((v1.x * v2.x + v1.y * v2.y + v1.z * v2.z) / sqrt(v1.x * v1.x + v1.y * v1.y + v1.z * v1.z) / sqrt(v2.x * v2.x + v2.y * v2.y + v2.z * v2.z)) * 180 / PI; return angle > 90.0 ? (180.0 - angle) : angle; };
	//Features:22
	double fea_dist_c1;//Feature:euc_dist between cur and it's nearer child;
	double fea_dist_c2;//Feature:euc_dist between cur and it's further child;
	double fea_dist_soma;//Feature:euc_dist between cur and soma;
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
	double fea_bif_angle_remote;//Feature:bifurcation angle among current and two nft children nodes; 
	double fea_bif_angle_local;//Feature:bifurication angle among current and two direct children nodes;

	/*2019-06-09, Update:remove
	double fea_tilt_angle_remote;
	double fea_tilt_angle_local;
	double fea_torque_remote = 0.0;//Feature:angle between two bifu planes(remote);
	double fea_torque_local = 0.0;//Feature:angle between two bifu planes(local);
	*/
	//2019-06-09, Update:Add
	//Feature:direction vector of current branch;
	double fea_direction_x;
	double fea_direction_y;
	double fea_direction_z;

	/*2019-04-13 add features;feature_num: 24*/
	//2019-06-09, Update: remove
	//double fea_child_ratio;//Feature:ratio between two children diameter, bigger/smaller;//Remove
	double fea_tips_num;//Feature:total tips(leaf node) from this node;calculated beside this func in recurrsion way;
	double fea_tips_asymmetry;//Feature:tips asymmetry for the two children;calculated beside this func in recurrsion way;
 
	//features between node:dist and angle remote;
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
	auto recursion = [&path_len, &stems_num, &total_diameter, &total_area, &surface, &volume, neuron, current, dist, xyz, PI](int &lo_ch)
	{
		//After lambda func, lo_ch will be got, features on the path from nft child to current will be collected;
		path_len = total_diameter = total_area = surface = volume = 0.0; stems_num = 0;
		int curr = lo_ch;
		for (auto p_id = neuron.hashNeuron.value(neuron.listNeuron[curr].pn); curr != current.label; curr = p_id, p_id = neuron.hashNeuron.value(neuron.listNeuron[p_id].pn))
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
	fea_contraction_1 = fea_dist_c1 / fea_path_len_1;//Feature:ratio between euc dist of a branch and its path len;
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
	//fea_child_ratio = max(fea_diameter_avg_1, fea_diameter_avg_2) / min(fea_diameter_avg_1, fea_area_avg_2);//Feature:daimeter ratio between two children;

	auto direction = (coord_c1 - coord).cross_product(coord_c2 - coord);//A vector being vertical with this branch(remote);

	fea_direction_x = direction.x / direction.length();
	fea_direction_y = direction.y / direction.length();
	fea_direction_z = direction.z / direction.length();

	/*2019-06-09, Update:remove
	//Tilt
	auto u = coord - xyz(neuron.listNeuron[id_parent]);//A vector from current's nft parent to current;
	auto v = (coord_c1 - coord).cross_product(coord_c2 - coord);//A vector being vertical with this branch(remote);
	auto p = u - (v * (u*v) / v.length() / v.length());//Vector u's projection on the branch plane(remote);
	fea_tilt_angle_remote = 0.5 * fea_bif_angle_remote - min(acute_angle(p, (coord_c1 - coord)), acute_angle(p, (coord_c2 - coord)));
	fea_tilt_angle_remote = abs(fea_tilt_angle_remote);

	u = coord - xyz(neuron.listNeuron[neuron.hashNeuron.value(neuron.listNeuron[current.label].pn)]);//A vector from current's parent to current;
	v = (coord_c1_lo - coord).cross_product(coord_c2_lo - coord);//A vector being vertical with this branch(local);
	p = v * (u*v) / v.length() / v.length() - u;//Vector u's projection on the branch plane(local);
	fea_tilt_angle_local = 0.5 * fea_bif_angle_local - min(acute_angle(p, (coord_c1_lo - coord)), acute_angle(p, (coord_c2_lo - coord)));
	fea_tilt_angle_local = abs(fea_tilt_angle_local);

	//Torque only calculate when nft parent is a branch
	if (id_parent != 0) {//if nft parent is not root, means parent node is branch node;
		auto d1 = (coord_c1 - coord).cross_product(coord_c2 - coord);//A vector being vertical with this branch(remote);
		auto d2 = (coord - xyz(neuron.listNeuron[id_parent])).cross_product(xyz(neuron.listNeuron[id_sibling]) - xyz(neuron.listNeuron[id_parent]));
		fea_torque_remote = acute_angle(d1, d2);//Feature:angle between two bifu planes(remote);
		d1 = (coord_c1_lo - coord).cross_product(coord_c2_lo - coord);//A vector being vertical with this branch(local);
		auto sib = id_sibling;
		for (; neuron.hashNeuron.value(neuron.listNeuron[sib].pn) != id_parent; sib = neuron.hashNeuron.value(neuron.listNeuron[sib].pn));//Get parent's local child;
		auto pa_ch_lo = current.label;
		for (; neuron.hashNeuron.value(neuron.listNeuron[pa_ch_lo].pn) != id_parent; pa_ch_lo = neuron.hashNeuron.value(neuron.listNeuron[pa_ch_lo].pn));//Get parent's local child;
		d2 = (xyz(neuron.listNeuron[pa_ch_lo]) - xyz(neuron.listNeuron[id_parent])).cross_product(xyz(neuron.listNeuron[sib]) - xyz(neuron.listNeuron[id_parent]));
		fea_torque_local = acute_angle(d1, d2);//Feature:angle between two bifu planes(local);
	}
	*/

	auto tips_ch = (nfs.at(id_child).feature.count("fea_tips_num") ? nfs.at(id_child).feature.at("fea_tips_num") : 1);//for leaf node, con't have feature "fea_tips_num";
	auto tips_ch_s = (nfs.at(id_ch_sibling).feature.count("fea_tips_num") ? nfs.at(id_ch_sibling).feature.at("fea_tips_num") : 1);
	fea_tips_num = tips_ch + tips_ch_s;

	fea_tips_asymmetry = ((tips_ch + tips_ch_s - 2) == 0 ? 0 : (abs(tips_ch - tips_ch_s) / (tips_ch + tips_ch_s - 2)));

	//collect features;
	vector<double>feature_vector;
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
	/*2019-06-09, Update:
	feature_vector.push_back(fea_tilt_angle_remote);
	feature_vector.push_back(fea_tilt_angle_local);
	feature_vector.push_back(fea_torque_remote);
	feature_vector.push_back(fea_torque_local);
	*/

	//2019-06-09, Update:Add
	feature_vector.push_back(fea_direction_x);
	feature_vector.push_back(fea_direction_y);
	feature_vector.push_back(fea_direction_z);

	//feature_vector.push_back(fea_child_ratio);
	feature_vector.push_back(fea_tips_num);
	feature_vector.push_back(fea_tips_asymmetry);

	unordered_map<string, double> feature;
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

	/*2019-06-09, Update:remove
	feature["fea_tilt_angle_remote"] = fea_tilt_angle_remote;
	feature["fea_tilt_angle_local"] = fea_tilt_angle_local;
	feature["fea_torque_remote"] = fea_torque_remote;
	feature["fea_torque_local"] = fea_torque_local;
	*/

	//2019-06-09, Update:Add
	feature["fea_direction_x"] = fea_direction_x;
	feature["fea_direction_y"] = fea_direction_y;
	feature["fea_direction_z"] = fea_direction_z;

	//feature["fea_child_ratio"] = fea_child_ratio;
	feature["fea_tips_num"] = fea_tips_num;
	feature["fea_tips_asymmetry"] = fea_tips_asymmetry;

	//2019-06-09, Update: delete fea: tilt remote, local, torque remote, local; Use branch direction vector instead;
	current.feature = feature;
	current.feature_vector = feature_vector;

}