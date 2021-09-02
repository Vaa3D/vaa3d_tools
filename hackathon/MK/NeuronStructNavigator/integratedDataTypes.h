//------------------------------------------------------------------------------
// Copyright (c) 2019 Hsien-Chi Kuo (Allen Institute)
// All rights reserved.
//------------------------------------------------------------------------------

/*******************************************************************************
*
*  [integratedDataTypes] is part of the NeuronStructNavigator library. 
*  This namespace manages all integrated data structures used by all other NeuronStructNavigator classes.
*  All data structures in this namespace are extension of standard Vaa3D data types with additional features, aiming to make neuron structure operations and algorithms easier.
*  Any creation new datatypes should be put in this namespace to keep them organized and avoid the confusion of header inclusion.
*
********************************************************************************/

#ifndef INTEGRATEDDATATYPES_H
#define INTEGRATEDDATATYPES_H

#include "integratedDataStructures.h"

#include "v_neuronswc.h"

#include <deque>
#include <string>

using namespace integratedDataStructures;

const double PI_MK            = 3.1415926535897932; // This name is given simply to avoid the conflict with [const double PI] in IM_config.h of terafly submodule.
const float  NODE_TILE_LENGTH = 100;
const float  SEGtileXY_LENGTH = 30;
const float  zRATIO           = 5; // This is the ratio of z resolution to x and y in fMOST images.
//const float  zRATIO            = (0.28 / 0.1144);

namespace integratedDataTypes
{
	// Specification of segment orientations for any pair-wise segment operations.
	enum connectOrientation { head_head, head_tail, tail_head, tail_tail, all_ort, head, tail, body };

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

	struct overlappedCoord
	{
		// This data structure profiles coordinates that have multiple nodes sitting on it.

		overlappedCoord() = default;
		overlappedCoord(float x, float y, float z);
		overlappedCoord(string coordKey);
		
		int nodeID;
		float x, y, z;
		string coordKey;

		// [integratedDatatypes::head or tail or body] -> set of pair<segment ID, ID of the node that sitting on this coordinate>
		// This map reveals how segments are spatially involved in this coordinatethe, i.e., which segments have head nodes, tail nodes, or body nodes sitting on this coordiniate.
		map<integratedDataTypes::connectOrientation, set<pair<int, int>>> involvedSegsOriMap; 
	};

	struct topoCharacter
	{
		topoCharacter() = default;
		topoCharacter(NeuronSWC centerNode, int streamLength = 10) : topoCenter(centerNode) {};
		NeuronSWC topoCenter;
		deque<NeuronSWC> upstream;
		map<int, deque<NeuronSWC>> downstreams;
	};

	struct coordUnit
	{
		coordUnit() = default;
		coordUnit(const NeuronSWC& inputNode) : x(inputNode.x), y(inputNode.y), z(inputNode.z), nodeID(inputNode.n), parentCoordPtr(nullptr) { this->childCoordPtrs.clear(); }
		coordUnit(const coordUnit&) = delete;
		coordUnit& operator=(const coordUnit&) = delete;
		~coordUnit();

		const bool operator==(const coordUnit& comparedCoordUnit) const;
		const bool operator==(const NeuronSWC& comparedNode) const;
		const bool operator!=(const coordUnit& comparedCoordUnit) const { return this->operator==(comparedCoordUnit) ? false : true; }
		const bool operator!=(const NeuronSWC& comparedNode) const { return this->operator==(comparedNode) ? false : true; }

		float x, y, z;
		int nodeID;

		coordUnit* parentCoordPtr;
		boost::container::flat_set<coordUnit*> childCoordPtrs;
	};

	struct segLevel
	{
		segLevel() : childLevel(nullptr), parentLevel(nullptr) {}
		segLevel(const segLevel&) = delete;
		segLevel& operator=(const segLevel&) = delete;
		~segLevel();

		int level;

		boost::container::flat_set<int> segIDs, endSegIDs;
		boost::container::flat_map<int, int> paSegMap;
		boost::container::flat_map<int, boost::container::flat_set<int>> childSegMap;

		segLevel *childLevel, *parentLevel;
	};


	/************* Segment Unit Data Structure *************/
	class segUnit
	{
	public:
		segUnit() : to_be_deleted(false) {}
		segUnit(const QList<NeuronSWC>& inputSeg);
		segUnit(const V_NeuronSWC& inputV_NeuronSWC);

		const bool operator==(const segUnit& comparedSeg) const;												         // operator== for comparing 2 segUnits
		const bool operator!=(const segUnit& comparedSeg) const { return this->operator==(comparedSeg) ? false : true; } // operator!= for comparing 2 segUnits
		
		// Current operator> and operator< are based on the number of nodes on the segment.
		const bool operator>(const segUnit& comparedSeg) const;
		const bool operator<(const segUnit& comparedSeg) const; 

		bool profiled;
		int segID, type;
		int head;                                  // segment head node ID
		vector<int> tails;                         // segment tail(s) node ID(s) (branching segment is currently not supported; only 1 element in tails vector)
		QList<NeuronSWC> nodes;                    // segment nodes
		map<int, size_t> seg_nodeLocMap;           // nodeID -> its location in nodes QList
		map<int, vector<size_t>> seg_childLocMap;  // nodeID -> its child location(s) in nodes QList. Note, tip nodes still have entries in the map with empty second value.
		vector<topoCharacter> topoCenters;         // nodes that carry information about important topology in the whole tree

		// [segSmoothnessMap] profiles smooth measures node by node through the whole segment.
		// Each node carries a map in which key is measuring range (with node centered) and value is a map of different measures.
		// node ID -> <measuring range -> <measure name -> value>>
		map<int, boost::container::flat_map<int, map<string, double>>> segSmoothnessMap;

		bool to_be_deleted;

		bool reverse(int nodeID = -1); 
		V_NeuronSWC convert2V_NeuronSWC() const;

		void reInit(segUnit& inputSegUnit);

	private:
		void rc_nodeRegister2V_NeuronSWC(V_NeuronSWC& sbjV_NeuronSWC, int parentID, int branchRootID) const;
		QList<NeuronSWC> changeTreeHead(const int newHeadID) const;
	};
	/*******************************************************/


	/********* Segment-segment Orientation Profiling Data Structure *********/
	struct segPairProfile
	{
		// This struct does not need copy/assignnebt control because [segUnit] is managed somewhere else.
		// [segPairProfile] does not attempt to make any changes onto the memory to which the pointers in this struct point.

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


	/******************* Complete Profile Data Structure for NeuronTree *******************/
	class profiledTree
	{
	public:
		profiledTree() = default;
		profiledTree(const NeuronTree& inputTree, float nodeTileLength = NODE_TILE_LENGTH, float segTileLength = SEGtileXY_LENGTH);
		//profiledTree(const vector<segUnit>& inputSegs, float nodeTileLength = NODE_TILE_LENGTH, float segTileLength = SEGtileXY_LENGTH);
		profiledTree(const vector<V_NeuronSWC>& inputV_NeuronSWC, float nodeTileLength = NODE_TILE_LENGTH, float segTileLength = SEGtileXY_LENGTH);
		~profiledTree() { this->cleanUpSegLevels(); }
		
		float segTileSize;
		float nodeTileSize;
		void nodeTileResize(float nodeTileLength);

		NeuronTree tree;
		map<int, size_t> node2LocMap;
		map<int, vector<size_t>> node2childLocMap;
		map<string, vector<int>> nodeTileMap; // tile label -> node IDs
		
		/* ====================== Segment-related Data Members and Functions ====================== */
		map<int, segUnit> segs;											  // key = seg ID
		map<string, vector<int>> segHeadMap;							  // tile label -> seg ID
		map<string, vector<int>> segTailMap;							  // tile label -> seg ID

		// --------- NOT INITIALIZED WHEN PROFILEDTREE IS CONSTRUCTED --------- //
		boost::container::flat_map<int, boost::container::flat_set<int>> segHeadClusters; // key is ordered cluster number label; cluster number -> all seg IDs with heads in the cluster
		boost::container::flat_map<int, boost::container::flat_set<int>> segTailClusters; // key is ordered cluster number label; cluster number -> all seg IDs with tails in the cluster
		boost::container::flat_map<int, int> headSeg2ClusterMap;						  // segment ID -> the cluster in which the segment head is located
		boost::container::flat_map<int, int> tailSeg2ClusterMap;						  // segment ID -> the cluster in which the segment tail is located
		
		boost::container::flat_map<int, int> node2segMap;											 // node ID -> seg ID
		boost::container::flat_multimap<string, int> nodeCoordKey2segMap;							 // node coord key -> seg ID
		boost::container::flat_multimap<string, int> segEndCoordKey2segMap;							 // seg end node coord key -> seg ID
		boost::container::flat_multimap<string, int> nodeCoordKey2nodeIDMap;						 // node coord key -> node ID
		boost::container::flat_map<string, integratedDataTypes::overlappedCoord> overlappedCoordMap; // node coord key -> profiled overlapped coordinate

		boost::container::flat_map<int, boost::container::flat_set<vector<float>>> segEndClusterNodeMap; // segEnd cluster ID -> all seg end nodes' coordinates in the cluster
		boost::container::flat_map<int, vector<float>> segEndClusterCentroidMap;                         // segEnd cluster ID -> the coordiate of the centroid of all nodes in the cluster	
		boost::container::flat_map<int, vector<segPairProfile>> cluster2segPairMap;						 // segEnd cluster -> all possible seg pair combinations in the cluster	
		// -------------------------------------------------------------------- //
		
		// ----------------- Advanced Node - Segment Profiling Methods ----------------- //
		void getSegEndClusterNodeMap();   // -> this->segEndClusterNodeMap
		void getSegEndClusterCentoirds(); // -> this->segEndClusterCentroidMap

		void nodeSegMapGen();             // -> this->node2segMap
		void nodeCoordKeySegMapGen();     // -> this->nodeCoordKey2segMap
		void segEndCoordKeySegMapGen();   // -> this->segEndCoordKey2segMap
		void nodeCoordKeyNodeIDmapGen();  // -> this->nodeCoordKey2nodeIDMap
		void overlappedCoordMapGen();     // -> this->overlappedCoordMap
		// ----------------------------------------------------------------------------- //
		
		// ----------------- Segment Assembling Methods ----------------- // 
		// Pick up the nearst node to the given soma coordinate. The node has to be a singular node that doesn't overlap any other nodes.
		// If no node is found within the soma node tile and its 26 surrounding tiles, then a random singular will be picked.
		int findNearestSegEndNodeID(const CellAPO inputAPO, int threshold = 35); 

		// Clean up and organize the 'mess' of manually traced segments, so that the segments will be "combed through" with a given starting node.
		// This method is required before running [this->assembleSegs2singleTree] to transform segments into a single tree. 
		void combSegs(int rootNodeID);

		// This method takes "combed" segments and assembles them to a single tree. (Only 1 segment)
		void assembleSegs2singleTree(int rootNodeID);
		// -------------------------------------------------------------- //
		
	private:
		void nodeSegMapGen(const map<int, segUnit>& segMap, boost::container::flat_map<int, int>& node2segMap);		
		void nodeCoordKeySegMapGen(const map<int, segUnit>& segMap, boost::container::flat_multimap<string, int>& nodeCoordKey2segMap);
		void segEndCoordKeySegMapGen(const map<int, segUnit>& segMap, boost::container::flat_multimap<string, int>& segEndCoordKey2segMap);
		void nodeCoordKeyNodeIDmapGen(const QList<NeuronSWC>& nodeList, boost::container::flat_multimap<string, int>& nodeCoordKey2nodeIDmap);
		
		// This method is called by [this->combSegs] to recursively examine every segment and correct the heading direction.
		void rc_reverseSegs(const int leadingSegID, const int startingNodeID, set<int>& checkedSegIDs);
		
		// This method is called when it needs to be converted to a segment that self-branches from the node to which other segment's end is attached.
		pair<int, segUnit> splitSegWithMiddleHead(const segUnit& inputSeg, int newHeadID);

		map<int, segUnit> seg2MiddleBranchingMap; // original segment ID -> rearranged segment with head node in the middle (self branching,  ( X - root node, the new head that has 2 child nodes) )
		//																																	   / \
		//																																      o   o
		//										     This data member is only used in tree assembling function, and doesn't serve profiling purposes. Therefore, it's set to be inaccessible by public for now.
		/* ================= END of [Segment-related Data Members and Functions] ================= */

	public:
		/* ================= Segment-Topology Data Members and Functions ================= */
		map<int, set<int>> loops;
		boost::container::flat_map<int, segLevel*> segLevelMap;

		bool loopCheck(int leadingSegID);
		void cleanUpSegLevels();
		/* ============ END of [Segment-Topology Data Members and Functions] ============= */

		boost::container::flat_set<int> spikeRootIDs;    // IDs of the nodes where "spikes" grow upon
		boost::container::flat_set<int> smoothedNodeIDs; // IDs of the nodes that have been "dragged" to the smoothed positions 

		map<int, topoCharacter> topoList;
		void addTopoUnit(int nodeID);

		set<string> brainRegions;
	};

	void profiledTreeReInit(profiledTree& inputProfiledTree); // Needs to incorporate with this->getSegHeadTailClusters later.
	inline void segEndClusterCentroid(const set<vector<float>>& allCoords, vector<float>& segEndClusterCentroid);
	inline void segEndClusterCentroid(const boost::container::flat_set<vector<float>>& allCoords, vector<float>& segEndClusterCentroid);
	/**************************************************************************************/


	/***************** segEnd Cluster Tree *****************/
	struct segEndClusterUnit
	{
		// This data structure makes up a chain list of segment end clusters with pointers.
		// To avoid memory violations due to mismanagement, copy and assignment are not allowed for [segEndClusterUnit],
		// so that 1 [segEndClusterUnit*] is strictly mapping to only 1 memory space.

		segEndClusterUnit() { this->parentCluster = nullptr; }
		segEndClusterUnit(const segEndClusterUnit&) = delete;
		segEndClusterUnit& operator=(const segEndClusterUnit&) = delete;
		~segEndClusterUnit();

		int ID;
		boost::container::flat_set<int> headSegs;
		boost::container::flat_set<int> tailSegs;

		segEndClusterUnit* parentCluster;
		map<int, segEndClusterUnit*> childClusterMap;
	};

	void cleanUp_segEndClusterChain_downStream(segEndClusterUnit* currCluster);
	/*******************************************************/
}

inline void integratedDataTypes::segEndClusterCentroid(const set<vector<float>>& allCoords, vector<float>& segEndClusterCentroid)
{
	if (allCoords.empty())
	{
		segEndClusterCentroid.clear();
		return;
	}

	float xSum = 0, ySum = 0, zSum = 0;
	for (auto& coord : allCoords)
	{
		xSum += coord.at(0);
		ySum += coord.at(1);
		zSum += coord.at(2);
	}
	segEndClusterCentroid[0] = xSum / allCoords.size();
	segEndClusterCentroid[1] = ySum / allCoords.size();
	segEndClusterCentroid[2] = zSum / allCoords.size();
}

inline void integratedDataTypes::segEndClusterCentroid(const boost::container::flat_set<vector<float>>& allCoords, vector<float>& segEndClusterCentroid)
{
	if (allCoords.empty())
	{
		segEndClusterCentroid.clear();
		return;
	}

	segEndClusterCentroid.clear();
	float xSum = 0, ySum = 0, zSum = 0;
	for (auto& coord : allCoords)
	{
		xSum += coord.at(0);
		ySum += coord.at(1);
		zSum += coord.at(2);
	}
	segEndClusterCentroid.push_back(xSum / allCoords.size());
	segEndClusterCentroid.push_back(ySum / allCoords.size());
	segEndClusterCentroid.push_back(zSum / allCoords.size());
}

#endif