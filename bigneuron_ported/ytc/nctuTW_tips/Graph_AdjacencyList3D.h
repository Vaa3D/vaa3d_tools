#ifndef GRAPH_ADJ_3D_H
#define GRAPH_ADJ_3D_H
#include <list>
#include <vector>
#include <cstdlib>
#include <iostream>

//==================================
//   0 -----> x+
//   |
//   |
//   |
//   y+
//
//	P.C. Lee cis dept. NCTU 
//==================================


namespace MyGridGraph_3D
{

typedef struct _GridNode_3D
{

	_GridNode_3D():weight(.0){}
	_GridNode_3D( float w):weight(w){}
	_GridNode_3D( int i, int j, int k ):x(i), y(j), z(k), weight(.0){}
	_GridNode_3D( int i, int j, int k, int p, int s): x(i), y(j), z(k), index(p), weight(.0){} 
	

	bool friend operator< (const struct _GridNode_3D& lhs, const struct _GridNode_3D& rhs) { return lhs.weight < rhs.weight; }
	bool friend operator> (const struct _GridNode_3D& lhs, const struct _GridNode_3D& rhs) { return lhs.weight > rhs.weight; }


	int x, y, z;	//location
	int index;  //node Index
	float weight;

	friend std::ostream& operator<< (std::ostream& os, const struct _GridNode_3D& rhs )
	{
		os<<rhs.index<<'\t'<<rhs.x<<'\t'<<rhs.y<<'\t'<<rhs.z;
		return os;
	}

}GridNode_3D;


class GridGraph_3
{
public:

	GridGraph_3( unsigned char* const inImage, double* const CostBuf, const int width, const int height, const int zSize )
	{ MakeGraph( inImage, CostBuf, width, height, zSize); } 

	GridGraph_3( unsigned char* const inImage, const int width, const int height, const int zSize )
	{ MakeGraph( inImage, width, height, zSize); } 

	GridGraph_3( std::ifstream& inG ){ MakeGraph( inG ); }

	~GridGraph_3(){ Delete(); }

	void MakeGraph(unsigned char* const inImage, double* const CostBuf, const int width, const int height, const int zSize );
	void MakeGraph(unsigned char* const inImage, const int width, const int height, const int zSize );
	void MakeGraph( std::ifstream& inG );
	void Delete();

	inline std::vector< std::list<GridNode_3D> > GetAdjacencyList() { return AdjList; }
	//inline std::vector< std::list<GridNode_3D> >& GetAdjacencyList() { return AdjList; }
	inline unsigned int GetSize() const { return size;}
	inline void SetSize( int i ) { size = i; }
	int GetNodeIndex(const int x, const int y, const int z) const ;
	GridNode_3D GetNode( int index ) const;

	void OutputGraph( std::ofstream& outFile);

private:
	unsigned int size;
	std::vector< std::list< GridNode_3D > > AdjList;
};

}
#endif