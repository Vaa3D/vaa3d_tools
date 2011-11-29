//
//=======================================================================
// Copyright 2011 Institute PICB.
// Authors: Hang Xiao
// Data : March 20, 2011
//=======================================================================
//

#ifndef COMPONENT_TREE_H_H
#define COMPONENT_TREE_H_H

#include <iostream>
#include <vector>
#include <list>
#ifdef __v3d__
#include "v3d_basicdatatype.h"
#include "stackutil.h"
#endif
using namespace std;


class ComponentTree
{
	public:
		class Pixel;
		class Node;

		typedef int Vertex;
		typedef vector<Vertex> Vertices;
		typedef vector<int> Path;
		typedef vector<Path> Paths;

		class Pixel
		{
			public:
				friend class ComponentTree;
				friend class Node;
			public:
				Pixel();
				bool save(ofstream& ofs, bool saveType = true) const;
				bool load(ifstream& ifs, vector<Pixel>& pixels, vector<Node*>& nodes, bool saveType=true);
				void merge_entry(Pixel* entry);
				Node* getNode() const;

			private:
				int  pos;
				Pixel * next;		 // the next pixel
				unsigned short level;
				Node* node;
		};
		typedef vector<Node*> Nodes;

		class Node
		{
			public:
				friend class ComponentTree;
				friend class Pixel;
			public:
				Node(){};
				bool save(ofstream& ofs, bool saveType=true) const;
				bool load(ifstream& ifs, vector<Pixel>& pixels, vector<Node*>& nodes, bool saveType=true);
				vector<Pixel*> getAlphaPixels();
				vector<Pixel*> getBetaPixels();
				vector<int> getAlphaPoints();
				vector<int> getBetaPoints();
				void getCenter(float& meanX, float& meanY, float& meanZ, int width, int height, int depth);
				vector<int> getCenterPoints(int width, int height, int depth);
				void merge_node(Node* node);  // node may be a child
				Nodes getPostOrderNodes() const; //return all the node which stores in post order, equilivalent to m_root
				Nodes getPreOrderNodes() const; //return all the node which stores in post order, equilivalent to m_root
				Nodes getBreadthFirstNodes() const; //return all the node which stores in post order, equilivalent to m_root
				vector<Node*>& getChilds();
				int getTreeHeight() const;       // the distant from root, the root node with height 0

				int getLabel() const;
				int getLowestLevel() const;
				int getHighestAlphaLevel() const;
				int getHighestLevel() const;

				int getAlphaSize() const;
				int getBetaSize() const;
				Node* getParent() const;

			private:
				int label;          // the store index in m_nodes, start from 0
				int lowest_level;          // the lowest level
				int highest_alpha_level;   // the highest alpha level

				int alpha_size;  // the pixel in the component exclude the pixels in child nodes
				int beta_size;   // the total number of pixels 
				Node* parent;    // we will make Node as dynamic memory, for the label is not easy to 
				Pixel* entry_pixel; // pixel will set static, the entry shoud set as the one of the lowest pixel in this component 
				vector<Node*> childs;
		};

	public:
		ComponentTree();
		ComponentTree(char * imgfile, int _minSize, int _maxSize, int _singleSize);
		ComponentTree(char* treefile);
		~ComponentTree();
		bool create(char * imgfile, int _minSize, int _maxSize, int _singleSize);

		bool load(const char* from_tree_file);
		bool reload(const char* from_tree_file);
		bool load(ifstream& ifs, bool saveType = true);
		bool save(const char* to_tree_file) const;
		bool save(ofstream& ofs, bool saveType = true) const;
		bool saveGraph(char* graph_file);
		void clear();

		int width() const;
		int height() const;
		int depth() const;
		int getMinThresh() const;
		int getMaxThresh() const;
		int getSingleThresh() const;
		Node* root() const;
		Node* getNode(int label) const;  //node of label 
		Node* getNode(vector<int> points) const;  //node of points
		Pixel* getPixel(int pos);

		Paths getPaths() const;

		vector<int> getReverseAlphaMapping() const; //get the matrix of labels
		//int* getMatrix(vector<int> labels , vector<int> values, int ini_value) const; 
		void setWeightMatrix(ComponentTree* tree2, vector<float> &weights);

		int nodeNum() const;
		int leafNum() const;
		int pixelNum() const;

		void printTree(Node* node = NULL) const;
		void printReverseAlphaMapping() const;
		void printPaths() const;

	private:
		int m_width ;
		int m_height;
		int m_depth;	

		int m_minSize;
		int m_maxSize;
		int m_singleSize;

		int m_numPixels;
		int m_numNodes;
		int m_numLeafs;

		vector<Pixel> m_pixels;
		Nodes m_nodes; //store the nodes in post order
		Nodes m_leafs; //store all the leafs
		Node* m_root;  //the root Node point to itself
};

class DisjointSets
{
	public:

		// Create an empty DisjointSets data structure
		DisjointSets();
		// Create a DisjointSets data structure with a specified number of pixels (with pixel id's from 0 to count-1)
		DisjointSets(int count);
		// Copy constructor
		DisjointSets(const DisjointSets & s);
		// Destructor
		~DisjointSets();

		// Find the set identifier that an pixel currently belongs to.
		// Note: some internal data is modified for optimization even though this method is consant.
		int FindSet(int pixel) const;
		// Combine two sets into one. All pixels in those two sets will share the same set id that can be gotten using FindSet.
		int Union(int setId1, int setId2);
		// Add a specified number of pixels to the DisjointSets data structure. The pixel id's of the new pixels are numbered
		// consequitively starting with the first never-before-used pixelId.
		void AddPixels(int numToAdd);
		// Returns the number of pixels currently in the DisjointSets data structure.
		int NumPixels() const;
		// Returns the number of sets currently in the DisjointSets data structure.
		int NumSets() const;

	private:

		// Internal Node data structure used for representing an pixel
		struct Node
		{
			int rank; // This roughly represent the max height of the node in its subtree
			int index; // The index of the pixel the node represents
			Node* parent; // The parent node of the node
		};

		int m_numPixels; // the number of pixels currently in the DisjointSets data structure.
		int m_numSets; // the number of sets currently in the DisjointSets data structure.
		std::vector<Node*> m_nodes; // the list of nodes representing the pixels
};

#endif
