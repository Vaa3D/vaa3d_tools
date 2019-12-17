/*
 *neuron_factor_tree.h
 *Header file for struct neuronFactor tree
 *By LongfeiLi.
 */
#ifndef __NEURON_FACTOR_TREE_H__
#define __NEURON_FACTOR_TREE_H__

#include <vector>
#include <iostream>
#include <unordered_map>
#include <set>

#define FACTORFEATURENUM 22

using namespace std;

struct NeuronFactor
{
	int level;
	int type;
	V3DLONG label;
	float x, y, z;
	unordered_map<string, double> feature;
	vector<double> feature_vector;
	V3DLONG parent;
	V3DLONG child;
	V3DLONG child_sibling; 
	V3DLONG sibling;//use sibling to handle more than one child;

	NeuronFactor()
	{
		//radius may be needed;
		level = -1;
		type = -1;
		label = -1;
		x = y = z = 0.0;
		//feature_vector = vector<double>(FACTORFEATURENUM, 0.0);
		parent = -1;
		child = -1;//if child is -1, leaf node;
		child_sibling = -1;
		sibling = -1;
	}

	NeuronFactor(int le, int t, V3DLONG la, float a, float b, float c, 
		V3DLONG pa, V3DLONG ch, V3DLONG ch_sb, V3DLONG sb) : //a featureless vision;
		level(le), type(t), label(la), x(a), y(b), z(c), 
		parent(pa), child(ch), child_sibling(ch_sb), sibling(sb) {}

	NeuronFactor(int le, int t, V3DLONG la, float a, float b, float c, unordered_map<string, double> fea, vector<double> fea_v, 
		V3DLONG pa, V3DLONG ch, V3DLONG ch_sb, V3DLONG sb) :
		level(le), type(t), label(la), x(a), y(b), z(c), feature(fea), feature_vector(fea_v), 
		parent(pa), child(ch), child_sibling(ch_sb), sibling(sb) {}
};

typedef unordered_map<V3DLONG, NeuronFactor> NeuronFactors;

struct NeuronFactorTree//A nft also contains a neuronTree's root and leaf;
{
	int level;
	string neuron_path;
	NeuronFactor neuron_factor_tree_root;
	NeuronFactors neuron_factors;

	NeuronFactorTree()
	{
		level = 0;
		neuron_path.clear();
		neuron_factor_tree_root = NeuronFactor();
		neuron_factors.clear();
	}

	NeuronFactorTree(int le, string path, NeuronFactor nft_root, NeuronFactors nfs) :
		level(le), neuron_path(path), neuron_factor_tree_root(nft_root), neuron_factors(nfs) {}
};

typedef list<NeuronFactor> NeuronFactorSequence;//in NFSequence, neuron factors still record children in neuron factor tree;

typedef vector<NeuronFactorSequence> NeuronFactorSequences;



#endif // !__NEURON_FACTOR_TREE_H__


