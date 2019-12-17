/************************************************************************/
/*	neuron_factor_tree_plugin.h
	a header file for vaa3d plugin:NeuronFactorTree.
	by Longfei li, 2019-01-17.                                           */
/************************************************************************/

#ifndef __NEURON_FACTOR_TREE_PLUGIN_H__
#define __NEURON_FACTOR_TREE_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
#include "v3d_message.h"
#include "basic_surf_objs.h"
#include "neuron_factor_tree.h"
#include <math.h>
#include <queue>
#include <tuple>

using namespace std;

#define TESTDIR "C:/Users/Administrator/Desktop/neuron_factor_tree_test/test"

//A struct handle x,y,z coordinates;
struct Coordxyz
{
	double x, y, z;

	Coordxyz() { x = y = z = -1; }
	Coordxyz(double a, double b, double c) :x(a), y(b), z(c) {}
	Coordxyz(float a, float b, float c) :x(a), y(b), z(c) {}
	Coordxyz operator+(Coordxyz coord) const { return Coordxyz(x + coord.x, y + coord.y, z + coord.z); }
	Coordxyz operator-(Coordxyz coord) const { return Coordxyz(x - coord.x, y - coord.y, z - coord.z); }
	Coordxyz& operator+=(Coordxyz coord) { x += coord.x; y += coord.y; z += coord.z; return *this; }
	Coordxyz& operator-=(Coordxyz coord) { x -= coord.x; y -= coord.y; z -= coord.z; return *this; }
	float operator*(Coordxyz coord) const { return (x*coord.x + y * coord.y + z * coord.z); }
	Coordxyz operator*(float d) const { return Coordxyz(x*d, y*d, z*d); }
	friend Coordxyz operator*(float d, const Coordxyz coord) { return Coordxyz(coord.x*d, coord.y*d, coord.z*d);  }
	Coordxyz& operator-() { return (*this) * -1; };
	friend ostream & operator<<(ostream &os, const Coordxyz coord) { os << coord.x << "," << coord.y << "," << coord.z; return os; }
	Coordxyz operator/(float d) const { return Coordxyz(x / d, y / d, z / d); }
	Coordxyz cross_product(Coordxyz c) const
	{
		return Coordxyz(y*c.z - c.y*z, c.x*z - x*c.z, x*c.y - c.x*y);
	}
	double length() const
	{
		return sqrt((x*x) + (y * y) + (z * z));
	}
	Coordxyz& normalize() const
	{
		return *this / length();
	}
};

class NeuronFactorTreePlugin : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
		Q_INTERFACES(V3DPluginInterface2_1);

public:
	float getPluginVersion() const { return 1.1f; }

	QStringList menulist() const;
	void domenu(const QString &menu_name,
		V3DPluginCallback2 &callback,
		QWidget *parent);

	QStringList funclist() const;
	bool dofunc(const QString &func_name,
		const V3DPluginArgList &input,
		V3DPluginArgList &output,
		V3DPluginCallback2 &callback,
		QWidget *parent);
};

/************************************************************************/
/* Domenu fucs                                                          */
/************************************************************************/
void generate_neuron_factor_tree();//create a neuronFactorTree from a selected neuronTree by domenu fuc.
void serializate_neuron_factor_tree();//from a .nft file get a .nfss neuronFactor sequences file.
void assemble_neuron_tree();//from a .gen file assemble a new neuron tree in swc format.
void cut_neuron_soma_branch();//from a .swc, cut it's soma branch part;
//void collect_nfss_in_folder();//collect all .nfss files under a path;
//combine files by their classes under a base dir : base_dir/classes/class_n/*.sqss -> base_dir/collected_features.txt.

void get_sequence_terminate_prob();

void build_branch_dataset();
void display_generated_branch();

void construct_neuron_factor_tree(const NeuronTree nt, NeuronFactorTree &nft);//from neuronTree create a level 1 neuronFactorTree.
void construct_neuron_factor_tree(NeuronFactorTree nft_input, NeuronFactorTree &nft_output, int level);//generate a coarser level neuronFactorTree. TODO

/************************************************************************/
/* Dofunc fucs                                                          */
/************************************************************************/
bool nfss_product_pipeline(const V3DPluginArgList & input, V3DPluginArgList & output);//pipeline to trans a *.swc to *.nfss.
//bool nfss_collect_pipeline(const V3DPluginArgList & input, V3DPluginArgList & output);//pipeline to collect all the *.nfss into a folder.
void print_help();//print help infos.

/************************************************************************/
/* neuronFactor tool fucs                                               */
/************************************************************************/

//NeuronFactor Tree construct tools
void calculate_branches(const NeuronTree &neuron, vector<V3DLONG> &branches);
void get_leaf_branch_nodes(const NeuronTree &neuron, vector<V3DLONG> branches, set<V3DLONG> &leaf_node_set, set<V3DLONG> &branch_node_set);
void get_nft_node_relation_map(const NeuronTree &neuron, vector<V3DLONG> branches, set<V3DLONG> nft_node_set, map<V3DLONG,V3DLONG> &nft_parent_map, map<V3DLONG,V3DLONG> &nft_child_map, map<V3DLONG, V3DLONG> &nft_sibling_map);
NeuronFactorTree create_neuron_factor_tree(const NeuronTree &neuron, bool is_calculate_feature = true);

//Calculate neuron features for neuronFactor
void calculate_features_at_node(const NeuronTree neuron, NeuronFactor &current, const V3DLONG id_child, const V3DLONG id_ch_sibling, const V3DLONG id_sibling, const V3DLONG id_parent, const NeuronFactors nfs);
//NeuronFactor Tree w/r tools
void save_neuron_factor_tree(const NeuronFactorTree nft, const QString path);
NeuronFactorTree read_neuron_factor_tree(const QString path);

//Serialization tools
NeuronFactorSequences serialization(const NeuronFactorTree& nft);
void save_neuron_factor_sequences(const NeuronFactorSequences &nfseqs, const string neuron_file_path, const QString path);

//NeuronTree assemble tools
void assemble_neuron_tree_from(const QString file_path);

struct SomaBranch
{
	vector<int> stem_type;
	vector<V3DLONG> stem_end;
	vector<Coordxyz> stem_edge_coord;
	vector<Coordxyz> child_branch_direction;
	int stem_num;
	int stem_id;
	vector<NeuronSWC> soma_branch_nodes;

	SomaBranch() { stem_end.clear(); stem_edge_coord.clear(); child_branch_direction.clear(); stem_num = 0; stem_id = 0; soma_branch_nodes.clear(); }

};
struct BranchInfo
{
	int branch_id;
	int branch_parent;//the branch's parent branch index in a Brach vector;
	V3DLONG branch_end[2];//branch's two end index in new swc; 
	Coordxyz branch_edge_coord[2];//branch's edge coordinate; next child branch's start coord;
	Coordxyz child_branch_direction[2];//the bracnch's child bracnch's direction vector(middle of root2edge vectors);
	//int stem_leaf_count[2];
	float stem_weight_count[2];
	int left_or_right;//0--1;

	vector<NeuronSWC> branch_nodes;
	BranchInfo() { branch_id = branch_parent = branch_end[0] = branch_end[1] = -1; branch_nodes.clear(); stem_weight_count[0] = stem_weight_count[1] = 0.f; left_or_right = -1; }
};
SomaBranch collect_neuron_soma_branch(const QString file_path);
vector<vector<BranchInfo>> collect_neuron_factor_infos(const QString file_path);// deprecated.
std::vector<vector<BranchInfo>> collect_neuron_branches(const QString file_path);

vector<NeuronSWC> adjust_branches(SomaBranch soma_branch, vector<vector<BranchInfo>> branches);//adjust the id of collected neuronSWC;
void save_soma_branch(const QString file_path, const SomaBranch soma_branch);//save a soma branch;
void save_swc_file(const QString swc_file, const vector<NeuronSWC> neuron);//save a created neuron(as a vector of NeuronSWC);

SomaBranch get_soma_branch(const NeuronTree &neuron);//get a neuronTree 's soma branch part;

/* USELESS */
void collect_nfss(const QString file_path, FILE * fp, int &sequence_total_count, const QString class_name);//collect a .nfss file under file_path marked by classname;
//don't need read sequences;

////////////////////////////////////////////////////////////////////////
// Branch collect. 

typedef Coordxyz Directionxyz;

typedef std::vector<Coordxyz> Stem;

struct Branch
{
	Branch() : type(1), stm_scale_1(1.f), stm_scale_2(1.f), angle(0.f){ stems[0].clear(); stems[1].clear(); };
	Branch(Stem& _stem, int t) : type(t) {
		for (size_t i = 0; i < _stem.size(); i++)
		{
			stems[0].push_back(_stem[i]);
		}
	};
	Branch(Stem& _stem) : type(1) { 
		for (size_t i = 0; i < _stem.size(); i++)
		{
			stems[0].push_back(_stem[i]);
		}
	};

	float stm_scale_1;
	float stm_scale_2;
	float angle;
	int type;
	// Coords : terminal_1 -> root, treminal_2 -> root.
	Stem stems[2];
	//Directionxyz BranchDirection;

	void append(Stem& _stem)
	{
		for (size_t i = 0; i < _stem.size(); i++)
		{
			stems[1].push_back(_stem[i]);
		}
	}
};

const int BRANCH_SIZE = 127;

// Neuron id + branch root node id hash.

typedef std::tuple<int, V3DLONG> key_t;

struct key_hash : public std::unary_function<key_t, std::size_t>
{
	std::size_t operator()(const key_t& k) const
	{
		return std::get<0>(k) ^ std::get<1>(k);
	}
};

struct key_equal : public std::binary_function<key_t, key_t, bool>
{
	bool operator()(const key_t& v0, const key_t& v1) const
	{
		return (
			std::get<0>(v0) == std::get<0>(v1) &&
			std::get<1>(v0) == std::get<1>(v1)
			);
	}
};

//typedef unordered_map<tuple<int, V3DLONG>, Branch, key_hash, key_equal> branch_map_t;
//typedef unordered_map<tuple<int, V3DLONG>, int, key_hash, key_equal> branch_id_map_t;
//typedef std::vector<std::vector<tuple<int, V3DLONG>>> branch_sequence_vector;
typedef unordered_map<std::string, Branch> branch_map_t;// From string : "neuron_id : branch_root_id" to a Branch.
typedef unordered_map<std::string, int> branch_id_map_t;// From string : "neuron_id : branch_root_id" to global id.
typedef std::vector<std::vector<std::string>> branch_sequence_vector;

void collect_branches(const NeuronTree& neuronTree, const int neuron_id, branch_map_t& branch_map, branch_sequence_vector& sequences);

void branch_data_normalization(branch_map_t& branch_map, branch_sequence_vector& sequences, int& shift_neuron_id);// Global normalization func.
void branch_data_normalization_(branch_map_t& branch_map, branch_sequence_vector& sequences, int& shift_neuron_id);// Global normalization func.

void normalize_branch_direction(Branch& branch, const Coordxyz& branch_target_direction);

// The function to save total branch coordinates info and branch id sequences.
bool save_global_branch_dataset(const QString path, branch_map_t& branch_map, branch_sequence_vector& sequences);

// The function to save all branch codes.
bool save_global_branch_code(const QString path, branch_map_t& branch_map, branch_sequence_vector& sequences);

// Function to generate a fixed length branch code.
bool create_single_branch_code(const Branch & branch, Branch& created_branch, const int globalID);

bool read_branch_from_br(const QString& br_file_path, Branch& branch);

// The function to save a single branch info includes a bmp branch pic.
bool save_single_branch_picture(const Branch & branch, const QString& file_path);// Save a branch as picture.
inline bool save_single_branch_picture(const Branch & branch, const QString& folder_path, const int globalID)// folder save version, file name is globalID.
{
	return save_single_branch_picture(branch, folder_path + QString("/%1.bmp").arg(globalID));
}

bool save_single_branch_swc(const Branch & branch, const QString& file_path);// Save a branch as swc.

// Branch bmp file for branch visulization, colored unit's index in bmp defines a branch coordinate, the color value is constructed by coordinate's z-value.
bool create_branch_bmp(const Branch & branch, float x_scale, float y_scale, const QString & bmp_path);

const int STEM_SAMPLE_POINT_NUM = 20;// Fixed sample point num for one stem.


// bmp.
#pragma pack(2)
typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;
typedef long    LONG;
typedef struct {
	WORD    bfType;
	DWORD   bfSize;
	WORD    bfReserved1;
	WORD    bfReserved2;
	DWORD   bfOffBits;
} BITMAPFILEHEADER;

typedef struct {
	DWORD      biSize;
	LONG       biWidth;
	LONG       biHeight;
	WORD       biPlanes;
	WORD       biBitCount;
	DWORD      biCompression;
	DWORD      biSizeImage;
	LONG       biXPelsPerMeter;
	LONG       biYPelsPerMeter;
	DWORD      biClrUsed;
	DWORD      biClrImportant;
} BITMAPINFOHEADER;

bool read_branch_coords(const QString gen_file_path, std::unordered_map<int, Branch>& branches);


#endif
