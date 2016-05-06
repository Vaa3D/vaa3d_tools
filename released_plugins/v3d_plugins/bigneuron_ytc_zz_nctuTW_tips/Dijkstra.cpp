#ifndef _DIJKSTRA
#define _DIJKSTRA
#include <iostream>
#include <vector>
//#include <set> 
#include <deque>
#include <cmath>
#include <algorithm>
#include "Dijkstra.h"

//== test ==//
#include <fstream>
//==========//

const double MAX_COST = 100000.0;

void Single_Dijkstra_GridGraph_3D(const int x, const int y, const int z, GridGraph_3& graph, int* All_Path)//std::vector< GridNode_3D >& All_Path)	
{
	std::cout<<"In dijkstra\n";
	
	int SrcIdx = graph.GetNodeIndex(x , y, z);
	std::vector< std::list<GridNode_3D> > AdjList = graph.GetAdjacencyList();

	GridNode_3D SrcNode = AdjList[SrcIdx].front();
	std::deque< GridNode_3D > deQ;

//	double* localCost = new double [ graph.GetSize() ];

	for(int i=0; i<graph.GetSize(); i++)
		All_Path[i] = -1;

	All_Path[ SrcNode.index ] = SrcNode.index;

//	SrcNode.weight = .0;
	deQ.push_back( SrcNode );

	for(int i=0; i<graph.GetSize(); i++)
	{
		if( AdjList[i].front().index != SrcNode.index )
		{
			GridNode_3D tmpNode = AdjList[i].front();
			tmpNode.weight = MAX_COST;
			deQ.push_back( tmpNode );
		}
	}

	while( !deQ.empty() )
	{
		GridNode_3D minNode = deQ.front();

/*		for(int i=0; i<10 && i<deQ.size(); i++ )
			std::cout<<deQ[i].weight<<'\n';
		std::cout<<'\n';
*/
		deQ.pop_front();

		std::list< GridNode_3D >::iterator l_Itr = AdjList[ minNode.index ].begin();
		GridNode_3D minOriginNode = AdjList[ minNode.index ].front();

		for( l_Itr; l_Itr != AdjList[ minNode.index ].end(); l_Itr++ )	//relax neighbor node;
		{
			for(int i=0; i<deQ.size(); i++)
			{
				if( deQ[i].index == l_Itr->index )
				{
					double cost = GvfCostFunction3D( minNode, deQ[i] );
//					double cost = .0;
					if( deQ[i].weight > minOriginNode.weight - minNode.weight + cost )
					{
						deQ[i].weight = minOriginNode.weight - minNode.weight + cost;
						All_Path[ l_Itr->index] = minNode.index;
					}

					break;
				}
			}
		}

		std::sort( deQ.begin(), deQ.end() );
	}
/*	
	std::list<GridNode_4>::iterator l_Itr = AdjList[SrcIdx].begin();
	for( ; l_Itr != AdjList[SrcIdx].end(); l_Itr++)
		AllPath[ l_Itr->index ] = AdjList[ SrcIdx ].front();
*/
}

void Single_Dijkstra_GridGraph_3D(const int x, const int y, const int z, GridGraph_3& graph, int* All_Path, bool*** skeleMap, unsigned char* inImg, 
								  const int width, const int height, const int zSize)
{
	
	std::cout<<"In dijkstra\n";
	
	int SrcIdx = graph.GetNodeIndex(x , y, z);
	std::vector< std::list<GridNode_3D> > AdjList = graph.GetAdjacencyList();

	std::cout<<SrcIdx<<'\n';
	GridNode_3D SrcNode = AdjList[SrcIdx].front();
	std::deque< GridNode_3D > deQ;


	for(int i=0; i<graph.GetSize(); i++)
		All_Path[i] = -1;

	All_Path[ SrcNode.index ] = SrcNode.index;


	int imgVolume = zSize * height * width;
	int imgArea = height * width;
	//static bool* visited = new bool [ imgVolume ];
	static bool visited[ 1024 * 1024 * 256 ];

	int preAllocateSize = 1024 * 1024 * 256;
	/*
	for(int i=0; i<imgVolume; i++)
		visited[i] = false;
	*/
	for(int i=0; i<preAllocateSize; i++)
		visited[ i ] = false;

	deQ.push_back( SrcNode );
	visited[ z * imgArea + y * width + x ] = true;

	for(int mz = -1; mz <=1; mz++)
	{
		for(int my =-1; my <= 1; my++)
		{
			for(int mx = -1; mx<=1; mx++)
			{
				if( x + mx >= 0 && x + mx < width && y + my >= 0 && y + my < height )
				{
					int pos = (z + mz) * imgArea + (y + my) * width + (x + mx);
					if( pos >= 0 && pos < imgVolume && !visited[ pos ] )
					{
						if( inImg[ pos ] > 0 )
						{
							int Idx = graph.GetNodeIndex(x+mx , y+my, z+mz);
							if( Idx >= 0 )
							{
								GridNode_3D tmpNode = AdjList[ Idx ].front();
								tmpNode.weight = MAX_COST;
								deQ.push_back( tmpNode );
							}
						}

						visited[ pos ] = true;
					}
				}
			}
		}
	}

	
	while( !deQ.empty() )
	{
		GridNode_3D minNode = deQ.front();

		deQ.pop_front();

		std::list< GridNode_3D >::iterator l_Itr = AdjList[ minNode.index ].begin();
		GridNode_3D minOriginNode = AdjList[ minNode.index ].front();

		for( l_Itr; l_Itr != AdjList[ minNode.index ].end(); l_Itr++ )	//relax neighbor node;
		{
			for(int i=0; i<deQ.size(); i++)
			{
				if( deQ[i].index == l_Itr->index )
				{
					double cost = GvfWithMedialCost3D( minNode, deQ[i], skeleMap );
					if( deQ[i].weight > minOriginNode.weight - minNode.weight + cost )
					{
						deQ[i].weight = minOriginNode.weight - minNode.weight + cost;
						All_Path[ l_Itr->index] = minNode.index;
					}
					
					//explore the neighbor of relaxed node and add to the container
					for(int mz=-1; mz<=1; mz++)
					{
						for(int my=-1; my<=1; my++)
						{
							for(int mx =-1; mx<=1; mx++)
							{
								if( l_Itr->x + mx >= 0 && l_Itr->x + mx < width && l_Itr->y + my >=0 && l_Itr->y + my < height )
								{
									int pos = (l_Itr->z + mz ) * imgArea + (l_Itr->y + my) * width + (l_Itr->x + mx);
									
									if( pos >= 0 && pos < imgVolume && !visited[ pos ] )
									{
										if( inImg[ pos ] > 0 )
										{
											int Idx = graph.GetNodeIndex( l_Itr->x+mx, l_Itr->y+my, l_Itr->z+mz );
											if( Idx >= 0 )
											{
												GridNode_3D tmpNode = AdjList[ Idx ].front();
												tmpNode.weight = MAX_COST;
												deQ.push_back( tmpNode );
											}
										}

										visited[ pos ] = true;
									}
								}
							}
						}
					}
					break;
				}
			}
		}

		if( deQ.size() > 1 )
		{
			int nextMinIdx = 0;
			for(int i=0; i<deQ.size(); i++)
			{
				if( deQ[ nextMinIdx ].weight > deQ[i].weight )
					nextMinIdx = i;
			}

			GridNode_3D swapNode = deQ[ nextMinIdx ];
			deQ[ nextMinIdx ] = deQ[ 0 ];
			deQ[0] = swapNode;
		}
	}

	//delete [] visited;
	deQ.clear();
}

void Single_Dijkstra_GridGraph_3D(const int x, const int y, const int z, GridGraph_3& graph, int* All_Path, bool*** skeleMap, unsigned char* CostDT, 
								  unsigned char* inImg, const int width, const int height, const int zSize )
{
	
	std::cout<<"In dijkstra\n";
	
	int SrcIdx = graph.GetNodeIndex(x , y, z);
	std::vector< std::list<GridNode_3D> > AdjList = graph.GetAdjacencyList();
	std::cout<<SrcIdx<<'\n';
	GridNode_3D SrcNode = AdjList[SrcIdx].front();
	std::deque< GridNode_3D > deQ;

//	double* localCost = new double [ graph.GetSize() ];


	for(int i=0; i<graph.GetSize(); i++)
		All_Path[i] = -1;

	All_Path[ SrcNode.index ] = SrcNode.index;

//	SrcNode.weight = .0;

	int imgVolume = zSize * height * width;
	int imgArea = height * width;
	bool* visited = new bool [ imgVolume ];
	
	for(int i=0; i<imgVolume; i++)
		visited[i] = false;

	deQ.push_back( SrcNode );
	visited[ z * imgArea + y * width + x ] = true;

	for(int mz = -1; mz <=1; mz++)
	{
		for(int my =-1; my <= 1; my++)
		{
			for(int mx = -1; mx<=1; mx++)
			{
				int pos = (z + mz) * imgArea + (y + my) * width + (x + mx);
				if( pos >= 0 && pos < imgVolume && !visited[ pos ] )
				{
					if( inImg[ pos ] > 0 )
					{
						int Idx = graph.GetNodeIndex(x+mx , y+my, z+mz);
						if( Idx >= 0 )
						{
							GridNode_3D tmpNode = AdjList[ Idx ].front();
							tmpNode.weight = MAX_COST;
							deQ.push_back( tmpNode );
						}
					}

					visited[ pos ] = true;
				}
			}
		}
	}

	
	while( !deQ.empty() )
	{
		GridNode_3D minNode = deQ.front();

		deQ.pop_front();

		std::list< GridNode_3D >::iterator l_Itr = AdjList[ minNode.index ].begin();
		GridNode_3D minOriginNode = AdjList[ minNode.index ].front();

		for( l_Itr; l_Itr != AdjList[ minNode.index ].end(); l_Itr++ )	//relax neighbor node;
		{
			for(int i=0; i<deQ.size(); i++)
			{
				if( deQ[i].index == l_Itr->index )
				{
					double cost = DTWithMedialCost3D( minNode, deQ[i], skeleMap, CostDT, width, height, zSize );

					if( deQ[i].weight > minOriginNode.weight - minNode.weight + cost )
					{
						deQ[i].weight = minOriginNode.weight - minNode.weight + cost;
						All_Path[ l_Itr->index] = minNode.index;
					}
					
					//explore the neighbor of relaxed node and add to the container
					for(int mz=-1; mz<=1; mz++)
					{
						for(int my=-1; my<=1; my++)
						{
							for(int mx =-1; mx<=1; mx++)
							{
								int pos = (l_Itr->z + mz ) * imgArea + (l_Itr->y + my) * width + (l_Itr->x + mx);
								
								if( pos >= 0 && pos < imgVolume && !visited[ pos ] )
								{
									if( inImg[ pos ] > 0 )
									{
										int Idx = graph.GetNodeIndex( l_Itr->x+mx, l_Itr->y+my, l_Itr->z+mz );
										if( Idx >= 0 )
										{
											GridNode_3D tmpNode = AdjList[ Idx ].front();
											tmpNode.weight = MAX_COST;
											deQ.push_back( tmpNode );
										}
									}

									visited[ pos ] = true;
								}
							}
						}
					}
					break;
				}
			}
		}

		//std::sort( deQ.begin(), deQ.end() );
		if( deQ.size() > 0 )
		{
			int nextMinIdx = 0;
			for(int i=0; i<deQ.size(); i++)
			{
				if( deQ[ nextMinIdx ].weight > deQ[i].weight )
					nextMinIdx = i;
			}

			GridNode_3D swapNode = deQ[ nextMinIdx ];
			deQ[ nextMinIdx ] = deQ[ 0 ];
			deQ[0] = swapNode;
		}
//		std::cout<<"size: "<<deQ.size()<<'\n';
	}

	delete [] visited;
	deQ.clear();
}

void Single_Dijkstra_GridGraph_3D_Ver2(const int x, const int y, const int z, GridGraph_3& graph, int* All_Path, bool*** skeleMap, unsigned char* CostDT, 
								  unsigned char* inImg, const int width, const int height, const int zSize )
{
	
	//std::cout<<"In dijkstra\n";
	
	int SrcIdx = graph.GetNodeIndex(x , y, z);
	std::vector< std::list<GridNode_3D> > AdjList = graph.GetAdjacencyList();
	//std::cout<<SrcIdx<<'\n';
	GridNode_3D SrcNode = AdjList[SrcIdx].front();
	std::deque< GridNode_3D > deQ;

//	double* localCost = new double [ graph.GetSize() ];


	for(int i=0; i<graph.GetSize(); i++)
		All_Path[i] = -1;

	All_Path[ SrcNode.index ] = SrcNode.index;

//	SrcNode.weight = .0;

	int imgVolume = zSize * height * width;
	int imgArea = height * width;
	bool* visited = new bool [ imgVolume ];
	
	for(int i=0; i<imgVolume; i++)
		visited[i] = false;

	deQ.push_back( SrcNode );
	visited[ z * imgArea + y * width + x ] = true;

	for(int mz = -1; mz <=1; mz++)
	{
		for(int my =-1; my <= 1; my++)
		{
			for(int mx = -1; mx<=1; mx++)
			{
				int pos = (z + mz) * imgArea + (y + my) * width + (x + mx);
				if( pos >= 0 && pos < imgVolume && !visited[ pos ] )
				{
					if( inImg[ pos ] > 0 )
					{
						int Idx = graph.GetNodeIndex(x+mx , y+my, z+mz);
						if( Idx >= 0 )
						{
							GridNode_3D tmpNode = AdjList[ Idx ].front();
							tmpNode.weight = MAX_COST;
							deQ.push_back( tmpNode );
						}
					}

					visited[ pos ] = true;
				}
			}
		}
	}

	
	while( !deQ.empty() )
	{
		GridNode_3D minNode = deQ.front();

		deQ.pop_front();

		std::list< GridNode_3D >::iterator l_Itr = AdjList[ minNode.index ].begin();
		GridNode_3D minOriginNode = AdjList[ minNode.index ].front();

		for( l_Itr; l_Itr != AdjList[ minNode.index ].end(); l_Itr++ )	//relax neighbor node;
		{
			for(int i=0; i<deQ.size(); i++)
			{
				if( deQ[i].index == l_Itr->index )
				{
					double cost = DTWithMedialCost3D_Ver2( minNode, deQ[i], skeleMap, CostDT, width, height, zSize );

					if( deQ[i].weight > minOriginNode.weight - minNode.weight + cost )
					{
						deQ[i].weight = minOriginNode.weight - minNode.weight + cost;
						All_Path[ l_Itr->index] = minNode.index;
					}
					
					//explore the neighbor of relaxed node and add to the container
					for(int mz=-1; mz<=1; mz++)
					{
						for(int my=-1; my<=1; my++)
						{
							for(int mx =-1; mx<=1; mx++)
							{
								int pos = (l_Itr->z + mz ) * imgArea + (l_Itr->y + my) * width + (l_Itr->x + mx);
								
								if( pos >= 0 && pos < imgVolume && !visited[ pos ] )
								{
									if( inImg[ pos ] > 0 )
									{
										int Idx = graph.GetNodeIndex( l_Itr->x+mx, l_Itr->y+my, l_Itr->z+mz );
										if( Idx >= 0 )
										{
											GridNode_3D tmpNode = AdjList[ Idx ].front();
											tmpNode.weight = MAX_COST;
											deQ.push_back( tmpNode );
										}
									}

									visited[ pos ] = true;
								}
							}
						}
					}
					break;
				}
			}
		}

		//std::sort( deQ.begin(), deQ.end() );
		if( deQ.size() > 0 )
		{
			int nextMinIdx = 0;
			for(int i=0; i<deQ.size(); i++)
			{
				if( deQ[ nextMinIdx ].weight > deQ[i].weight )
					nextMinIdx = i;
			}

			GridNode_3D swapNode = deQ[ nextMinIdx ];
			deQ[ nextMinIdx ] = deQ[ 0 ];
			deQ[0] = swapNode;
		}
//		std::cout<<"size: "<<deQ.size()<<'\n';
	}

	delete [] visited;
	deQ.clear();
}

double GvfCostFunction3D( GridNode_3D SrcNode, GridNode_3D node, float scaler)
{
//	return 0;
	return scaler * sqrt( (float) (SrcNode.x - node.x ) * (SrcNode.x - node.x ) + (SrcNode.y - node.y ) * (SrcNode.y - node.y ) +
			(SrcNode.z - node.z ) * (SrcNode.z - node.z ) );	
}

double GvfWithMedialCost3D( GridNode_3D SrcNode, GridNode_3D node, bool*** skeleMap, float scaler, float skeleton_const )
{
	if( skeleMap[ node.z ][ node.y ][ node.x ] )
		return scaler * sqrt( (float) (SrcNode.x - node.x ) * (SrcNode.x - node.x ) + (SrcNode.y - node.y ) * (SrcNode.y - node.y ) +
		(SrcNode.z - node.z ) * (SrcNode.z - node.z ) ) + skeleton_const;
	else
		return scaler * sqrt( (float) (SrcNode.x - node.x ) * (SrcNode.x - node.x ) + (SrcNode.y - node.y ) * (SrcNode.y - node.y ) +
		(SrcNode.z - node.z ) * (SrcNode.z - node.z ) );

}

double DTWithMedialCost3D( GridNode_3D SrcNode, GridNode_3D node, bool*** skeleMap, unsigned char* CostDT, 
						  const int width, const int height, const int zSize, float scaler, float skeleton_const )
{
	int area = width * height;

	if( skeleMap[ node.z ][ node.y ][ node.x ] )
		//return CostDT[ node.z * area + node.y * header.width + node.x ] * .8;
		return sqrt( (float) (SrcNode.x - node.x ) * (SrcNode.x - node.x ) + (SrcNode.y - node.y ) * (SrcNode.y - node.y ) +
		(SrcNode.z - node.z ) * (SrcNode.z - node.z ) ) + skeleton_const;
	else
		return scaler * sqrt( (float) (SrcNode.x - node.x ) * (SrcNode.x - node.x ) + (SrcNode.y - node.y ) * (SrcNode.y - node.y ) +
		(SrcNode.z - node.z ) * (SrcNode.z - node.z ) ) + CostDT[ node.z * area + node.y * width + node.x ];
		//return CostDT[ node.z * area + node.y * header.width + node.x ];
}

double DTWithMedialCost3D_Ver2( GridNode_3D SrcNode, GridNode_3D node, bool*** skeleMap, unsigned char* CostDT, 
						  const int width, const int height, const int zSize )
{
	double epsilon = 0.00000001;

	const int Area = width * height;

	double geoDis = sqrt( (float) 0.32 * ( (SrcNode.x - node.x ) * (SrcNode.x - node.x ) + (SrcNode.y - node.y ) * (SrcNode.y - node.y ) )  +
								(SrcNode.z - node.z ) * (SrcNode.z - node.z ) );
	double medialAward = CostDT[ node.z * Area + node.y * width + node.x ];

	return exp( -1 * (epsilon + geoDis + medialAward) );
	//return  1.0 / ( geoDis + medialAward + epsilon );

}
#endif