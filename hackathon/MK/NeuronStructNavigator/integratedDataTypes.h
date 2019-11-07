//------------------------------------------------------------------------------
// Copyright (c) 2019 Hsienchi Kuo (Allen Institute, Hanchuan Peng's team)
// All rights reserved.
//------------------------------------------------------------------------------

/*******************************************************************************
*
*  [integratedDataTypes] is part of the NeuronStructNavigator library. 
*  The namespace manages all integrated data structures used by all other NeuronStructNavigator classes.
*  All data structures in this namespace are integrated with standard Vaa3D data types with additional features, aiming to make developing neuron structure operations and algorithms more convenient.
*  Any new development on the datatypes should be put in this namespace to keep them organized and avoid the confusion of header inclusion.
* 
*  [profiledTree] is the core data type in throughout the whole NeuronStructNavigator library. It profiles the NeuronTree and carries crucial information of it.
*  Particularly profiledTree provides node-location, child-location, and detailed segment information of a NeuronTree.
*  Each segment of a NeuronTree is represented as a segUnit struct. A segUnit struct carries within-segment node-location, child-location, head, and tails information.
*  All segments are stored and sorted in profiledTree's map<int, segUnit> data member.
*
********************************************************************************/

#ifndef INTEGRATEDDATATYPES_H
#define INTEGRATEDDATATYPES_H

#include <deque>
#include <string>

#include "basic_surf_objs.h"
#include "integratedDataStructures.h"

using namespace std;
using namespace integratedDataStructures;

#ifndef PI
#define PI 3.1415926
#endif

#ifndef NODE_TILE_LENGTH
#define NODE_TILE_LENGTH 100
#endif

#ifndef SEGtileXY_LENGTH
#define SEGtileXY_LENGTH 30
#endif

#ifndef radANGLE_THRE
#define radANGLE_THRE 0.25
#endif

#ifndef zRATIO
#define zRATIO (1 / 0.2) // This is the ratio of z resolution to x and y in fMOST images.
//#define zRATIO (0.28 / 0.1144) // This is the ratio of z resolution to x and y in IVSCC images.
#endif

namespace integratedDataTypes
{
	// Specification of segment orientations for any pair-wise segment operations.
	enum connectOrientation { head_head, head_tail, tail_head, tail_tail, all_ort, head, tail };

	struct profiledNode
	{
		int index;
		bool front, back;
		long segID, frontSegID, backSegID, nodeNum, x, y, z;

		double innerProduct;
		double previousSqr, nextSqr, radAngle, distToMainRoute, turnCost;
	};

	struct polarNeuronSWC
	{
		int ID, type, parent;
		float CartesianX, CartesianY, CartesianZ;

		double polarOriginX, polarOriginY, polarOriginZ;
		double radius;
		double theta; // horizontal angle
		double phi;   // vertical angle
	};

	struct topoCharacter
	{
		topoCharacter() {};
		topoCharacter(NeuronSWC centerNode, int streamLength = 10) : topoCenter(centerNode) {};
		NeuronSWC topoCenter;
		deque<NeuronSWC> upstream;
		map<int, deque<NeuronSWC>> downstreams;
	};


	/********* Segment Unit Data Structure *********/
	struct segUnit
	{
		segUnit() : to_be_deleted(false) {};
		//segUnit(const segUnit& sourceSegUnit) {};

		int segID;
		int head;                                  // segment head node ID
		vector<int> tails;                         // segment tail(s) node ID(s) (branching segment is currently not supported; only 1 element in tails vector)
		QList<NeuronSWC> nodes;                    // segment nodes
		map<int, size_t> seg_nodeLocMap;           // nodeID -> its location in nodes QList
		map<int, vector<size_t>> seg_childLocMap;  // nodeID -> its child location(s) in nodes Qlist
		vector<topoCharacter> topoCenters;         // nodes that carry information about important topology in the whole tree

		bool to_be_deleted;
	};
	/***********************************************/

	/********* Segment-segment Orientation Profiling Data Structure *********/
	struct segPairProfile
	{
		// This struct has pointer data members. Need to provide copy control constructors later.

		segPairProfile() : seg1Ptr(nullptr), seg2Ptr(nullptr) {};
		segPairProfile(const segUnit& inputSeg1, const segUnit& inputSeg2, connectOrientation connOrt = all_ort);

		const segUnit* seg1Ptr;
		const segUnit* seg2Ptr;
		connectOrientation currConnOrt;

		double distance;			// The shortest distance between the 2 segments
		double turningAngle;		// The turnning angle from seg1's displacement vector to seg2's connecting point
		double segsAngleDiff;		// The 'sequential' angle difference from displacement vector 1 to displacement vector 2.
		
		map<connectOrientation, double> connDistMap;

		void getSegDistance(connectOrientation connOrt = all_ort);  // Get the closest distance between the 2 segments
		void turning12(connectOrientation connOrt);					// Get the angle formed by elongating segment's displacement vector and connecting vector (from elongating end to connecting end).
		void segsAngleDiff12(connectOrientation connOrt);
	};
	/************************************************************************/

	/********* Complete Profile Data Structure for NeuronTree *********/
	struct profiledTree
	{
		// With reinitialization function provided, this struct needs copy control constructors. 
		// Will be implemented later.

		profiledTree() {};
		profiledTree(const NeuronTree& inputTree, float segTileLength = SEGtileXY_LENGTH);
		float segTileSize;
		float nodeTileSize;
		void nodeTileResize(float nodeTileLength);

		NeuronTree tree;
		map<int, size_t> node2LocMap;
		map<int, vector<size_t>> node2childLocMap;

		map<string, vector<int>> nodeTileMap; // tile label -> node ID
		map<int, segUnit> segs; // key = seg ID

		map<string, vector<int>> segHeadMap;   // tile label -> seg ID
		map<string, vector<int>> segTailMap;   // tile label -> seg ID

		boost::container::flat_map<int, boost::container::flat_set<int>> segHeadClusters; // key is ordered cluster number label
		boost::container::flat_map<int, boost::container::flat_set<int>> segTailClusters; // key is ordered cluster number label
		boost::container::flat_map<int, int> headSeg2ClusterMap;
		boost::container::flat_map<int, int> tailSeg2ClusterMap;

		boost::container::flat_map<int, vector<segPairProfile>> cluster2segPairMap;

		map<int, topoCharacter> topoList;
		void addTopoUnit(int nodeID);

		set<string> brainRegions;
	};

	void profiledTreeReInit(profiledTree& inputProfiledTree); // Needs to incorporate with this->getSegHeadTailClusters later.
	/******************************************************************/

	
}

#endif