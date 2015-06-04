#include "Image3D.h"
#include <math.h>
#include <cstring>
#include <stdio.h>
#include <vector>
#include <list>

//Image3D-1

void BranchPoint2D_Iter2D( unsigned char* const inBuf, const int width, const int height, const int zSize, std::list<VectorCordinate >& Bps2D )
{
	const int Area = width * height;

	for(int z=0; z<zSize; ++z)
	{
		for(int y=0; y<height; ++y)
		{
			for(int x=0; x<width; ++x)
			{
				int pos = z * Area + y * width + x;
				
				if( inBuf[ pos ] > 0)
				{
					int Cnt = 0;
					int validCnt = 0;
					bool location[9];
					memset(location, 0, sizeof(bool) * 9);
					for(int my=-1; my<=1; ++my)
					{
						for(int mx = -1; mx<=1; ++mx)
						{
							if( x + mx >= 0 && x + mx < width &&
								y + my >= 0 && y + my < height )
							{
								//++validCnt;
								int neighbor = pos + my * width + mx;
								if( inBuf[ neighbor ] )
								{
									++Cnt;
									if( -1 == mx && -1 == my )
										location[2] = true;
									else if( -1 == mx && 0 == my )
										location[1] = true;
									else if( -1 == mx && 1 == my )
										location[0] = true;
									else if( 0 == mx && 1 == my )
										location[7] = true;
									else if( 0 == mx && -1 == my )
										location[3] = true;
									else if( 1 == mx && -1 == my )
										location[4] = true;
									else if( 1 == mx && 0 == my )
										location[5] = true;
									else if( 1 == mx && 1 == my )
										location[6] = true;

								}
							}
						}
					}

					if( Cnt == 4 || Cnt == 5 )
					{
						
						int pivot;
						for(int i=0; i<9; ++i)
						{
							if( location[i] )
							{
								pivot = i;
								break;
							}
						}
						
						if( location[ pivot + 2 ] && location[ pivot + 5 ] ) 
						{
							//std::cout<<pivot<<' '<<pivot + 2 <<' '<<pivot + 5<<' '<<x<<' '<<y<<' '<<z<<'\n';
							Bps2D.push_back( VectorCordinate(x, y ,z ) );
						}
						
						if( (location[ pivot + 3 ] && location[ pivot + 5 ] ) )
						{
							//std::cout<<pivot<<' '<<pivot + 3 <<' '<<pivot + 5<<' '<<x<<' '<<y<<' '<<z<<'\n';
							Bps2D.push_back( VectorCordinate(x, y ,z ) );
						}
						if( pivot == 0 || pivot == 1 )
						{
							if( location[ pivot + 3 ] && location[ pivot + 6 ] )
							{
								//std::cout<<pivot<<' '<<pivot + 3 <<' '<<pivot + 6<<' '<<x<<' '<<y<<' '<<z<<'\n';
								Bps2D.push_back( VectorCordinate(x, y, z) );
							}
						}

						if( pivot == 0 || pivot == 2 )
						{
							if( location[ pivot + 2 ] && location[ pivot + 4 ] )
							{
								//std::cout<<pivot<<' '<<pivot + 2 <<' '<<pivot + 4<<' '<<x<<' '<<y<<' '<<z<<'\n';
								Bps2D.push_back( VectorCordinate(x, y, z) );
							} 
						}
					}
				}
			}
		}
	}
}

void Endpoint3D_Iter2D( unsigned char* const inBuf, const int width, const int height, const int zSize, std::vector<VectorCordinate >& endP )
{
	int area = width * height;
	
	for(int z=0; z<zSize; z++)
	{
		unsigned char* slice = new unsigned char [ area ];
		memcpy( slice, inBuf + z * area, area );

		for(int y=0; y<height; y++)
		{
			for(int x=0; x<width; x++)
			{
				int neighborCnt = 0;

				if( slice[ y * height + x ] )
				{
					for(int my = -1; my <= 1; my++)
					{
						for(int mx = -1; mx <= 1; mx++)
						{
							if( my + y >=0 && my + y < height &&
								mx + x >=0 && mx + x < width )
							{
								if( slice[ (my+y) * width + (mx+x) ] )
									neighborCnt++;
							}
						}
					}
				}

				if( neighborCnt == 2 )
					endP.push_back( VectorCordinate(x, y, z) );
			}
		}

		delete [] slice;
	}
}


void Binarize(unsigned char* inBuf, const int width, const int height, const int zSize)
{
	if( inBuf == NULL ) return;
	float totalCnt = .0;
	int slot[256];
	for(int i=0; i<256; i++) slot[i] = 0;

	int area = width * height;
	int volume = area * zSize;

	for(int i=0; i<volume; i++)
	{
		if(inBuf[i] > 0)
		{
			slot[ (int)inBuf[i] ]++;
			totalCnt++;
		}
	}
	std::cout<<"TotalCnt: "<<totalCnt<<'\n';

	float accumCnt = .0;
	int threshold;
	
	for(int i=255; i>=1; i--)
	{
		accumCnt += slot[ i ];
		if( accumCnt / totalCnt >= .88 )
		{
			threshold = i;
			break;
		}
	}

	std::cout<<"Threshold: "<<threshold<<'\n';

	for(int i=0; i<volume; i++)
	{
		if( inBuf[ i ] >= threshold )
			inBuf[i] = 255;
		else
			inBuf[i] = 0;
	}

}

//Image3D
void Endpoint3D( unsigned char* const inBuf, const int width, const int height, const int zSize, std::vector< VectorCordinate >& endP )
{
	int area = width * height;

	for(int z=0; z<zSize; z++)
	{
		for(int y=0; y<height; y++)
		{
			for(int x=0; x<width; x++)
			{
				if( inBuf[z * area + y * width + x] )
				{
					//檢查在9個平面上是不是有一個面上該點是 4-connected endpoint in 2D
					
					int neighborCnt = 0;
					//轉 xy-plane
					if( z + 1 < zSize && y + 1 < height )
						if( inBuf[(z+1) * area + (y+1) * width + x] )
							neighborCnt++;

					if( x - 1 >= 0 )
						if( inBuf[ z * area + y * width + (x-1) ] )
							neighborCnt++;

					if( x + 1 < width )
						if( inBuf[ z * area + y * width + (x+1) ] )
							neighborCnt++;

					if( z - 1 >= 0 && y - 1 >= 0 )
						if( inBuf[(z-1) * area + (y-1) * width + x ] )
							neighborCnt++;

					if( neighborCnt == 1 )
					{
						VectorCordinate tmpP(x, y, z);
						endP.push_back( tmpP );

						break;
					}
					else
						neighborCnt = 0;


					if( x + 1 < width )
						if( inBuf[ z * area + y * width + ( x + 1 ) ] )
							neighborCnt++;

					if( x - 1 >= 0 )
						if( inBuf[ z * area + y * width + ( x - 1 ) ] )
							neighborCnt++;

					if( y + 1 < width )
						if( inBuf[ z * area + (y+1) * width + x ] )
							neighborCnt++;

					if( y - 1 >= 0 )
						if( inBuf[ z * area + (y-1) * width + x ] )
							neighborCnt++;

					if( neighborCnt == 1 )
					{
						VectorCordinate tmpP(x, y, z);
						endP.push_back( tmpP );

						break;
					}
					else
						neighborCnt = 0;

					if( z+1 < zSize && y-1>=0 )
						if( inBuf[ (z+1) * area + (y-1) * width + x ] )
							neighborCnt++;

					if( x - 1 >= 0 )
						if( inBuf[ z * area + y * width + (x-1) ] )
							neighborCnt++;

					if( x + 1 >= 0 )
						if( inBuf[ z * area + y * width + (x+1) ] )
							neighborCnt++;

					if( z - 1 >= 0 && y+1 < height)
						if( inBuf[ (z-1) * area + (y+1) * width + x ] )
							neighborCnt++;

					if( neighborCnt == 1 )
					{
						VectorCordinate tmpP(x, y, z);
						endP.push_back( tmpP );

						break;
					}
					else
						neighborCnt = 0;

					//轉 xz-plane
					if( z+1 < zSize )
						if( inBuf[ (z+1) * area + y * width + x ] )
							neighborCnt++;

					if( z-1 >= 0 )
						if( inBuf[ (z-1) * area + y * width + x ] )
							neighborCnt++;

					if( x+1 < width )
						if( inBuf[ z * area + y * width + (x+1) ] )
							neighborCnt++;
					
					if( x-1 >= 0 )
						if( inBuf[ z * area + y * width + (x-1) ] )
							neighborCnt++;

					if( neighborCnt == 1 )
					{
						VectorCordinate tmpP(x, y, z);
						endP.push_back( tmpP );

						break;
					}
					else
						neighborCnt = 0;

					if( x - 1 >=0 && y-1 >= 0)
						if( inBuf[ z * area + (y-1) * width + (x-1) ] )
							neighborCnt++;

					if( x+1 < width && y+1 < height )
						if( inBuf[ z * area + (y+1) * width + (x+1) ] )
							neighborCnt++;

					if( z - 1 >=0 )
						if( inBuf[ (z-1) * area + y * width + x ] )
							neighborCnt++;

					if( z + 1 < zSize )
						if( inBuf[ (z+1) * area + y * width + x ] )
							neighborCnt++;

					if( neighborCnt == 1 )
					{
						VectorCordinate tmpP(x, y, z);
						endP.push_back( tmpP );

						break;
					}
					else
						neighborCnt = 0;

					if( x + 1 < width && y - 1 >=0 )
						if( inBuf[ z * area + (y -1) * width + (x+1) ] )
							neighborCnt++;
					if( x - 1 >= 0 && y + 1 >= 0 )
						if( inBuf[ z * area + (y+1) * width + (x-1) ] )
							neighborCnt++;
					if( z - 1 >= 0)
						if( inBuf[ (z-1) * area + y * width + x ] )
							neighborCnt++;
					if( z + 1 < zSize )
						if( inBuf[ (z+1) * area + y * width + x ] )
							neighborCnt++;

					if( neighborCnt == 1 )
					{
						VectorCordinate tmpP(x, y, z);
						endP.push_back( tmpP );

						break;
					}
					else
						neighborCnt = 0;

					//轉yz-plane

					if( z + 1 < zSize )
						if( inBuf[ (z+1) * area + y * width + x ] )
							neighborCnt++;

					if( z - 1 >= 0)
						if( inBuf[ (z-1) * area + y * width + x ] )
							neighborCnt++;

					if( y + 1 < height )
						if( inBuf[ z* area + (y+1) * width + x ] )
							neighborCnt++;

					if( y - 1 >= 0 )
						if( inBuf[ z * area + (y-1) * width + x ] )
							neighborCnt++;

					if( neighborCnt == 1 )
					{
						VectorCordinate tmpP(x, y, z);
						endP.push_back( tmpP );

						break;
					}
					else
						neighborCnt = 0;
					
					if( z + 1 < zSize && x + 1 < width )
						if( inBuf[ (z+1) * area + y * width + (x+1) ] )
							neighborCnt++;
					if( z - 1 >=0 && x - 1 >= 0)
						if( inBuf[ (z-1) * area + y * width + (x-1) ] )
							neighborCnt++;
					if( y + 1 < height )
						if( inBuf[ z * area + (y+1) * width + x ] )
							neighborCnt++;
					if( y - 1 >= 0 )
						if( inBuf[ z * area + (y-1) * width + x ] )
							neighborCnt++;

					if( neighborCnt == 1 )
					{
						VectorCordinate tmpP(x, y, z);
						endP.push_back( tmpP );

						break;
					}
					else
						neighborCnt = 0;

					if( z + 1 < zSize && x - 1 >= 0 )
						if( inBuf[ (z+1) * area + y * width + (x-1) ] )
							neighborCnt++;

					if( z-1 >= 0 && x + 1 < width )
						if( inBuf[ (z-1) * area + y * width + (x+1) ] )
							neighborCnt++;

					if( y + 1 < height )
						if( inBuf[ z * area + (y+1) * width + x ] )
							neighborCnt++;

					if( y - 1 >= 0 )
						if( inBuf[ z * area + (y-1) * width + x ] )
							neighborCnt++;

					if( neighborCnt == 1 )
					{
						VectorCordinate tmpP(x, y, z);
						endP.push_back( tmpP );

						break;
					}
					else
						neighborCnt = 0;

				}
			}
		}
	}
}

//Seems this one is not required
bool Endpoint3D( unsigned char* const inBuf, const int x, const int y, const int z, const int width, const int height, const int zSize )
{
	int area = width * height;
	if( inBuf[z * area + y * width + x] )
	{
		//檢查在9個平面上是不是有一個面上該點是 4-connected endpoint in 2D
		
		int neighborCnt = 0;
		//轉 xy-plane
		if( z + 1 < zSize && y + 1 < height )
			if( inBuf[(z+1) * area + (y+1) * width + x] )
				neighborCnt++;

		if( x - 1 >= 0 )
			if( inBuf[ z * area + y * width + (x-1) ] )
				neighborCnt++;

		if( x + 1 < width )
			if( inBuf[ z * area + y * width + (x+1) ] )
				neighborCnt++;

		if( z - 1 >= 0 && y - 1 >= 0 )
			if( inBuf[(z-1) * area + (y-1) * width + x ] )
				neighborCnt++;

		if( neighborCnt == 1 )
			return true;
		else
			neighborCnt = 0;


		if( x + 1 < width )
			if( inBuf[ z * area + y * width + ( x + 1 ) ] )
				neighborCnt++;

		if( x - 1 >= 0 )
			if( inBuf[ z * area + y * width + ( x - 1 ) ] )
				neighborCnt++;

		if( y + 1 < width )
			if( inBuf[ z * area + (y+1) * width + x ] )
				neighborCnt++;

		if( y - 1 >= 0 )
			if( inBuf[ z * area + (y-1) * width + x ] )
				neighborCnt++;

		if( neighborCnt == 1 )
			return true;
		else
			neighborCnt = 0;

		if( z+1 < zSize && y-1>=0 )
			if( inBuf[ (z+1) * area + (y-1) * width + x ] )
				neighborCnt++;

		if( x - 1 >= 0 )
			if( inBuf[ z * area + y * width + (x-1) ] )
				neighborCnt++;

		if( x + 1 >= 0 )
			if( inBuf[ z * area + y * width + (x+1) ] )
				neighborCnt++;

		if( z - 1 >= 0 && y+1 < height)
			if( inBuf[ (z-1) * area + (y+1) * width + x ] )
				neighborCnt++;

		if( neighborCnt == 1 )
			return true;
		else
			neighborCnt = 0;

		//轉 xz-plane
		if( z+1 < zSize )
			if( inBuf[ (z+1) * area + y * width + x ] )
				neighborCnt++;

		if( z-1 >= 0 )
			if( inBuf[ (z-1) * area + y * width + x ] )
				neighborCnt++;

		if( x+1 < width )
			if( inBuf[ z * area + y * width + (x+1) ] )
				neighborCnt++;
		
		if( x-1 >= 0 )
			if( inBuf[ z * area + y * width + (x-1) ] )
				neighborCnt++;

		if( neighborCnt == 1 )
			return true;
		else
			neighborCnt = 0;

		if( x - 1 >=0 && y-1 >= 0)
			if( inBuf[ z * area + (y-1) * width + (x-1) ] )
				neighborCnt++;

		if( x+1 < width && y+1 < height )
			if( inBuf[ z * area + (y+1) * width + (x+1) ] )
				neighborCnt++;

		if( z - 1 >=0 )
			if( inBuf[ (z-1) * area + y * width + x ] )
				neighborCnt++;

		if( z + 1 < zSize )
			if( inBuf[ (z+1) * area + y * width + x ] )
				neighborCnt++;

		if( neighborCnt == 1 )
			return true;
		else
			neighborCnt = 0;

		if( x + 1 < width && y - 1 >=0 )
			if( inBuf[ z * area + (y -1) * width + (x+1) ] )
				neighborCnt++;
		if( x - 1 >= 0 && y + 1 >= 0 )
			if( inBuf[ z * area + (y+1) * width + (x-1) ] )
				neighborCnt++;
		if( z - 1 >= 0)
			if( inBuf[ (z-1) * area + y * width + x ] )
				neighborCnt++;
		if( z + 1 < zSize )
			if( inBuf[ (z+1) * area + y * width + x ] )
				neighborCnt++;

		if( neighborCnt == 1 )
			return true;
		else
			neighborCnt = 0;

		//轉yz-plane

		if( z + 1 < zSize )
			if( inBuf[ (z+1) * area + y * width + x ] )
				neighborCnt++;

		if( z - 1 >= 0)
			if( inBuf[ (z-1) * area + y * width + x ] )
				neighborCnt++;

		if( y + 1 < height )
			if( inBuf[ z* area + (y+1) * width + x ] )
				neighborCnt++;

		if( y - 1 >= 0 )
			if( inBuf[ z * area + (y-1) * width + x ] )
				neighborCnt++;

		if( neighborCnt == 1 )
			return true;
		else
			neighborCnt = 0;
		
		if( z + 1 < zSize && x + 1 < width )
			if( inBuf[ (z+1) * area + y * width + (x+1) ] )
				neighborCnt++;
		if( z - 1 >=0 && x - 1 >= 0)
			if( inBuf[ (z-1) * area + y * width + (x-1) ] )
				neighborCnt++;
		if( y + 1 < height )
			if( inBuf[ z * area + (y+1) * width + x ] )
				neighborCnt++;
		if( y - 1 >= 0 )
			if( inBuf[ z * area + (y-1) * width + x ] )
				neighborCnt++;

		if( neighborCnt == 1 )
			return true;
		else
			neighborCnt = 0;

		if( z + 1 < zSize && x - 1 >= 0 )
			if( inBuf[ (z+1) * area + y * width + (x-1) ] )
				neighborCnt++;

		if( z-1 >= 0 && x + 1 < width )
			if( inBuf[ (z-1) * area + y * width + (x+1) ] )
				neighborCnt++;

		if( y + 1 < height )
			if( inBuf[ z * area + (y+1) * width + x ] )
				neighborCnt++;

		if( y - 1 >= 0 )
			if( inBuf[ z * area + (y-1) * width + x ] )
				neighborCnt++;

		if( neighborCnt == 1 )
			return true;
		else
			neighborCnt = 0;

	}
	return false;
}

void MaxCC_3D26( unsigned char* const inBuf, const int width, const int height, const int zSize, std::vector<unsigned int>& MaxCC26 )
{
	bool* visited = new bool [ width * height * zSize ];
	for(int i=0; i<width * height * zSize; i++)
	{
		if( inBuf[i] > 0 ) 
			visited[i] = false;
		else
			visited[i] = true;
	}
	int MaxSize = 0;
	int MaxIdx = 0;
	int index = 0;

	unsigned char* GroupingMap = new unsigned char [ width * height * zSize ];
	for(int i=0; i<width * height * zSize; i++)
		GroupingMap[i] = index;

	std::vector< int > stack;
	bool ALL_VISIT_FLAG = false;

	int start = 0;
	for(int i=0; i<width * height * zSize; i++)
	{
		if( inBuf[i] > 0 ) start = i;  
		break;
	}

	while( !ALL_VISIT_FLAG )
	{
		int size = 0;
		++index;

		if(inBuf[start] > 0 && !visited[start] )
		{
			stack.push_back( start );
			while( !stack.empty() )
			{
				int pos = stack.back();
				stack.pop_back();

				for(int z=-1; z<=1; z++)
				{
					for(int y=-1; y<=1; y++)
					{
						for(int x=-1; x<=1; x++)
						{
							int neighborPos = pos + z * width * height + y * width + x;
							if( neighborPos >= 0 && neighborPos < width * height * zSize  && !visited[neighborPos] && inBuf[neighborPos] > 0 )
							{
								GroupingMap[neighborPos] = index;
								size++ ;
								visited[neighborPos] = true;
								stack.push_back( neighborPos );
							}
						}
					}
				}
			}

		}
		
		if( size > MaxSize )
		{
			MaxSize = size;
			MaxIdx = index;
		}
		
		ALL_VISIT_FLAG = true;
		for(int i=start; i<width * height * zSize; i++)
		{
			if( !visited[i] && inBuf[i] > 0)
			{
				start = i;
				ALL_VISIT_FLAG = false;
				break;
			}
		}

	}

	for(int i=0; i<width * height * zSize; i++)
		if( GroupingMap[i] == MaxIdx )
			MaxCC26.push_back( i );
}

void FindCCs(unsigned char* ImgBuf, const IPTHeader header, const int CC_THRESHOLD, std::vector< std::vector<int> >& CCs )
{
	//Find CCs
	int volume = header.width * header.height * header.zSize;
	int area = header.width * header.height;

	bool* ccVisit = new bool [volume];
	for(int i=0; i<volume; i++)
		ccVisit[i] = false;

//	const int CC_THRESHOLD = 0;

//	std::vector< std::vector<int> > All_ccStack;
	for(int i=0; i<volume; i++)
	{
		if( ImgBuf[i] > 0 && !ccVisit[i] )
		{
			std::vector<int> ccStack;
			std::vector<int> tmpStack;

			ccStack.push_back(i);
			tmpStack.push_back(i);

			ccVisit[ i ] = true;

			while( !tmpStack.empty() )
			{
				int curPos = tmpStack.back();
				tmpStack.pop_back();

				int x = curPos % header.width, y = (curPos % area) / header.width;
				for( int mx = -1; mx <= 1; mx++)
				{
					for(int my = -1; my <= 1 ; my++)
					{
						for( int mz = -1; mz <= 1; mz++)
						{
							int newPos = curPos + mz * area + my * header.width + mx;
							if( x + mx >=0 && x + mx < header.width && y + my >= 0 && y + my < header.height &&
								newPos >=0 && newPos < volume && !ccVisit[newPos] )
							{
								ccVisit[ newPos ] = true;

								if( ImgBuf[newPos] > 0 )
								{
									ccStack.push_back( newPos );
									tmpStack.push_back( newPos );
								}
							}
						}
					}
				}
			}

			if( ccStack.size() > CC_THRESHOLD )
				CCs.push_back( ccStack );
		}
	}

//	return All_ccStack;

}

void FindCCs(unsigned char* ImgBuf, const int width, const int height, const int zSize, const int CC_THRESHOLD, std::vector< std::vector<int> >& CCs)
{
	//Find CCs
	int volume = width * height * zSize;
	int area = width * height;

	bool* ccVisit = new bool [volume];
	for(int i=0; i<volume; i++)
		ccVisit[i] = false;

	for(int i=0; i<volume; i++)
	{
		if( ImgBuf[i] > 0 && !ccVisit[i] )
		{
			std::vector<int> ccStack;
			std::vector<int> tmpStack;

			ccStack.push_back(i);
			tmpStack.push_back(i);

			ccVisit[ i ] = true;

			while( !tmpStack.empty() )
			{
				int curPos = tmpStack.back();
				tmpStack.pop_back();
				
				int x = curPos % width, y = (curPos % area) / width;
				for( int mx = -1; mx <= 1; mx++)
				{
					for(int my = -1; my <= 1 ; my++)
					{
						for( int mz = -1; mz <= 1; mz++)
						{
							int newPos = curPos + mz * area + my * width + mx;
							if( x + mx >=0 && x + mx < width && y + my >= 0 && y + my < height && 
								newPos >=0 && newPos < volume && !ccVisit[newPos] )
							{
								ccVisit[ newPos ] = true;

								if( ImgBuf[newPos] > 0 )
								{
									ccStack.push_back( newPos );
									tmpStack.push_back( newPos );
								}
							}
						}
					}
				}
			}

			if( ccStack.size() > CC_THRESHOLD )
				CCs.push_back( ccStack );
		}
	}
}

void FindCCs(unsigned char* ImgBuf, const int width, const int height, const int zSize, const int CC_THRESHOLD, const int GREY_RADIUS, std::vector< std::vector<int> >& CCs)
{
	//Find CCs
	int volume = width * height * zSize;
	int area = width * height;

	bool* ccVisit = new bool [volume];
	for(int i=0; i<volume; i++)
	{
		if( ImgBuf[i] > 0 )
			ccVisit[i] = false;
		else
			ccVisit[i] = true;
	}

	int* Order = new int [ volume ];
	for(int i=0; i<volume; i++)
		Order[i]=i;

	std::vector< std::list<int> > busket;
	for(int i=0; i<256; i++)
		busket.push_back( std::list<int>() );
	
	for(int i=0; i<volume; i++)
		busket[ ImgBuf[i] ].push_back( i );

	int Cnt = 0;
	for(int i = busket.size()-1; i>=0; i--)
	{
		std::list<int>::iterator l_Itr = busket[i].begin();
		for(; l_Itr != busket[i].end(); l_Itr++)
		{
			Order[ Cnt ] = *l_Itr;
			Cnt++;
		}
	}
	
	for(int i=0; i<busket.size(); i++)
		busket[i].clear();
	std::vector< std::list<int> >().swap( busket );

	/*
	bool SWAP_FLAG;
	do
	{
		SWAP_FLAG = false;
		for(int i=0; i<volume-1; i++)
		{
			if( ImgBuf[ Order[i] ]< ImgBuf[ Order[i+1] ] )
			{
				int tmp = Order[i];
				Order[i] = Order[i+1];
				Order[i+1] = tmp;
				SWAP_FLAG = true;
			}
		}
	}while( SWAP_FLAG );
	*/

	std::cout<<"Sorting\n";

	bool ALL_VISIT = false;
	while( !ALL_VISIT )
	{
		//std::cout<<"ALL_VISIT\n";

		for(int i=0; i<volume; i++)
		{
			int idx = Order[ i ];
			if( !ccVisit[ idx ] )
			{
				std::vector<int> ccStack;
				std::vector<int> tmpStack;

				ccStack.push_back(idx);
				tmpStack.push_back(idx);

				ccVisit[ idx ] = true;
				int originPos = idx;
				while( !tmpStack.empty() )
				{
					int curPos = tmpStack.back();
					tmpStack.pop_back();
					
					int x = curPos % width, y = (curPos % area) / width;
					for( int mx = -1; mx <= 1; mx++)
					{
						for(int my = -1; my <= 1 ; my++)
						{
							for( int mz = -1; mz <= 1; mz++)
							{
								int newPos = curPos + mz * area + my * width + mx;
								if( x + mx >=0 && x + mx < width && y + my >= 0 && y + my < height && 
									newPos >=0 && newPos < volume && !ccVisit[newPos] )
								{
									if( ImgBuf[newPos] > 0 && abs( ImgBuf[newPos] - ImgBuf[ originPos ] ) <= GREY_RADIUS  )
									{
										ccVisit[ newPos ] = true;
										ccStack.push_back( newPos );
										tmpStack.push_back( newPos );
									}
								}
							}
						}
					}
				}

				if( ccStack.size() > CC_THRESHOLD )
					CCs.push_back( ccStack );
			}
		}
	
		ALL_VISIT = true;
		Cnt = 0;
		for(int i=0; i<volume; i++)
			if( !ccVisit[i] )
				Cnt++; 
		std::cout<<Cnt<<'\n';
		if( Cnt > 0 )
			ALL_VISIT = false;
				
	}

	delete [] Order;
}

void FindCC2Ds(unsigned char* ImgBuf, const IPTHeader header, const int CC_THRESHOLD, std::vector< std::vector<int> >& CCs )
{
	//Find CCs
	int area = header.width * header.height;

	bool* ccVisit = new bool [area];
	for(int i=0; i<area; i++)
		ccVisit[i] = false;

//	const int CC_THRESHOLD = 0;

//	std::vector< std::vector<int> > All_ccStack;
	for(int i=0; i<area; i++)
	{
		if( ImgBuf[i] > 0 && !ccVisit[i] )
		{
			std::vector<int> ccStack;
			std::vector<int> tmpStack;

			ccStack.push_back(i);
			tmpStack.push_back(i);

			ccVisit[ i ] = true;

			while( !tmpStack.empty() )
			{
				int curPos = tmpStack.back();
				int curX = curPos % header.width;
				tmpStack.pop_back();

				for( int mx = -1; mx <= 1; mx++)
				{
					for(int my = -1; my <= 1 ; my++)
					{
						//for( int mz = -1; mz <= 1; mz++)
						//{
							int newPos = curPos + my * header.width + mx;
							if( curX + mx >=0 && curX + mx < header.width && newPos >=0 && newPos < area && !ccVisit[newPos] && ImgBuf[newPos] > 0 )
							{
								ccVisit[ newPos ] = true;
								ccStack.push_back( newPos );
								tmpStack.push_back( newPos );
							}
						//}
					}
				}
			}

			if( ccStack.size() > CC_THRESHOLD )
				CCs.push_back( ccStack );
		}
	}

//	return All_ccStack;

}

void FindCC2Ds(unsigned char* ImgBuf, const IPTHeader header, std::vector< std::vector<int> >& CCs, const int CC_THRESHOLD = 0, const unsigned char GRAY_THRESHOLD = 0 )
{
	//Find CCs
	int area = header.width * header.height;

	bool* ccVisit = new bool [area];
	for(int i=0; i<area; i++)
		ccVisit[i] = false;

//	const int CC_THRESHOLD = 0;

//	std::vector< std::vector<int> > All_ccStack;
	for(int i=0; i<area; i++)
	{
		if( ImgBuf[i] > GRAY_THRESHOLD && !ccVisit[i] )
		{
			std::vector<int> ccStack;
			std::vector<int> tmpStack;

			ccStack.push_back(i);
			tmpStack.push_back(i);

			ccVisit[ i ] = true;

			while( !tmpStack.empty() )
			{
				int curPos = tmpStack.back();
				int curX = curPos % header.width;
				tmpStack.pop_back();

				for( int mx = -1; mx <= 1; mx++)
				{
					for(int my = -1; my <= 1 ; my++)
					{
						//for( int mz = -1; mz <= 1; mz++)
						//{
							int newPos = curPos + my * header.width + mx;
							if( curX + mx >=0 && curX + mx < header.width && newPos >=0 && newPos < area 
								&& !ccVisit[newPos] && ImgBuf[newPos] > GRAY_THRESHOLD )
							{
								ccVisit[ newPos ] = true;
								ccStack.push_back( newPos );
								tmpStack.push_back( newPos );
							}
						//}
					}
				}
			}

			if( ccStack.size() > CC_THRESHOLD )
				CCs.push_back( ccStack );
		}
	}

//	return All_ccStack;

}

void FindCC2D4s(unsigned char* ImgBuf, const int width, const int height, const int CC_THRESHOLD, std::vector< std::vector<int> >& CCs )
{
	std::cout<<"in\n";
	//Find CCs
	int area = width * height;

	bool* ccVisit = new bool [area];
	for(int i=0; i<area; i++)
		ccVisit[i] = false;

//	const int CC_THRESHOLD = 0;

//	std::vector< std::vector<int> > All_ccStack;
	for(int i=0; i<area; i++)
	{
		if( ImgBuf[i] > 0 && !ccVisit[i] )
		{
			std::vector<int> ccStack;
			std::vector<int> tmpStack;

			ccStack.push_back(i);
			tmpStack.push_back(i);

			ccVisit[ i ] = true;

			while( !tmpStack.empty() )
			{
				int curPos = tmpStack.back();
				int curX = curPos % width;
				int curY = curPos / width;
				tmpStack.pop_back();

				if( curX - 1 >= 0 && !ccVisit[ curPos - 1 ] && ImgBuf[curPos - 1] > 0 )
				{
					ccVisit[ curPos - 1 ] = true;
					ccStack.push_back( curPos - 1 );
					tmpStack.push_back( curPos - 1 );
				}
				if( curX + 1 < width && !ccVisit[ curPos + 1 ] && ImgBuf[curPos + 1] > 0 )
				{
					ccVisit[ curPos + 1 ] = true;
					ccStack.push_back( curPos + 1 );
					tmpStack.push_back( curPos + 1 );
				}
				if( curY - 1 >= 0 && !ccVisit[ curPos - width ] && ImgBuf[curPos - width] > 0 )
				{
					ccVisit[ curPos - width ] = true;
					ccStack.push_back( curPos - width );
					tmpStack.push_back( curPos - width );
				}
				if( curY + 1 < height && !ccVisit[ curPos + width ] && ImgBuf[curPos + width] > 0 )
				{
					ccVisit[ curPos + width ] = true;
					ccStack.push_back( curPos + width );
					tmpStack.push_back( curPos + width );
				}
			}

			if( ccStack.size() > CC_THRESHOLD )
				CCs.push_back( ccStack );
		}
	}

//	return All_ccStack;

}
void FindCC2Ds(unsigned char* ImgBuf, const int width, const int height, const int CC_THRESHOLD, std::vector< std::vector<int> >& CCs )
{
	//Find CCs
	int area = width * height;

	bool* ccVisit = new bool [area];
	for(int i=0; i<area; i++)
		ccVisit[i] = false;

//	const int CC_THRESHOLD = 0;

//	std::vector< std::vector<int> > All_ccStack;
	for(int i=0; i<area; i++)
	{
		if( ImgBuf[i] > 0 && !ccVisit[i] )
		{
			std::vector<int> ccStack;
			std::vector<int> tmpStack;

			ccStack.push_back(i);
			tmpStack.push_back(i);

			ccVisit[ i ] = true;

			while( !tmpStack.empty() )
			{
				int curPos = tmpStack.back();
				int curX = curPos % width;
				tmpStack.pop_back();

				for( int mx = -1; mx <= 1; mx++)
				{
					for(int my = -1; my <= 1 ; my++)
					{
						//for( int mz = -1; mz <= 1; mz++)
						//{
							int newPos = curPos + my * width + mx;
							if( curX + mx >=0 && curX + mx < width && newPos >=0 && newPos < area && !ccVisit[newPos] && ImgBuf[newPos] > 0 )
							{
								ccVisit[ newPos ] = true;
								ccStack.push_back( newPos );
								tmpStack.push_back( newPos );
							}
						//}
					}
				}
			}

			if( ccStack.size() > CC_THRESHOLD )
				CCs.push_back( ccStack );
		}
	}

//	return All_ccStack;

}

void FindCC2Ds(unsigned char* ImgBuf, const int width, const int height, const int CC_THRESHOLD, std::vector<int> seeds, std::vector< std::vector<int> >& CCs, const VectorCordinate stopV, const int stopR )
{
	int area = width * height;

	bool* ccVisit = new bool [area];
	for(int i=0; i<area; i++)
		ccVisit[i] = false;

	const int stopIdx = stopV.y * width + stopV.x;
	for(int s=0; s<seeds.size(); ++s)
	{
		int idx = seeds[s];
		if( ImgBuf[idx] > 0 && !ccVisit[idx] )
		{
			//std::vector<int> ccStack;
			//std::vector<int> tmpStack;
			std::queue< int > ccQueue;
			std::queue< int > tmpQueue;

			//ccStack.push_back(idx);
			//tmpStack.push_back(idx);

			ccQueue.push( idx );
			tmpQueue.push( idx);

			ccVisit[ idx ] = true;

			
			if( stopR < 0 )
			{
				//while( !tmpStack.empty() )
				while( !tmpQueue.empty() )
				{
					//int curPos = tmpStack.back();
					int curPos = tmpQueue.front();
					int curX = curPos % width;
					//tmpStack.pop_back();
					tmpQueue.pop();

					if( curPos  == stopIdx )
					{
						std::cout<<stopIdx<<' '<<curPos<<'\n';
						break;
					}

					for( int mx = -1; mx <= 1; mx++)
					{
						for(int my = -1; my <= 1 ; my++)
						{
							int newPos = curPos + my * width + mx;
							if( curX + mx >=0 && curX + mx < width && newPos >=0 && newPos < area && !ccVisit[newPos] && ImgBuf[newPos] > 0 )
							{
								ccVisit[ newPos ] = true;
								//ccStack.push_back( newPos );
								//tmpStack.push_back( newPos );
								ccQueue.push( newPos );
								tmpQueue.push( newPos );
							}
						}
					}
				}
			}
			else
			{
				int R = stopR;
				std::vector<int> currentFrontier;
				std::vector<int> nxtFrontier;

				currentFrontier.push_back( idx );

				std::cout<<"size: "<<currentFrontier.size()<<'\n';
				while( currentFrontier.size() > 0 && R >= 0 )
				{
					for(int i=0; i<currentFrontier.size(); ++i)
					{
						ccQueue.push( currentFrontier[i] );

						int pos = currentFrontier[i];
						int x = pos % width;
						int y = pos / width;

						for(int mx =-1; mx <= 1; ++mx)
						{
							if( x+mx < 0 || x + mx >= width)
								continue;

							for(int my=-1; my<=1; ++my)
							{
								if( y+my < 0 || y+my >= height )
									continue;

								int newPos = pos + my * width + mx;
								if( !ccVisit[ newPos ] && ImgBuf[ newPos ] > 0 )
								{
									ccVisit[ newPos ] = true;
									nxtFrontier.push_back( newPos );
								}
							}
						}
					}

					std::cout<<currentFrontier.size()<<'\t';
					nxtFrontier.swap( currentFrontier );
					std::cout<<currentFrontier.size()<<'\n';
					nxtFrontier.clear();

					--R;
				}

			}

			//if( ccStack.size() > CC_THRESHOLD )
			if( ccQueue.size() > CC_THRESHOLD )
			{
				std::vector< int > tmpS;
				for(int i=0; i<ccQueue.size(); ++i)
				{
					tmpS.push_back( ccQueue.front() );
					ccQueue.pop();
				}
				CCs.push_back( tmpS );
				//CCs.push_back( ccStack );
			}
		}
	}

//	return All_ccStack;
}

bool SimplePoint(unsigned char* inBuf, const int x, const int y, const int z, const int width, const int height, const int zSize)
{
	int area = width * height;
	int volume = area * zSize;
	
	bool neighbors[27];
	for(int i=0; i<27; i++)
		neighbors[i] = false;

	//condition 1 : all neighbor forms only one 26-connected component
	
	for(int mx =-1; mx <= 1; mx++)
	{
		for(int my = -1; my <= 1; my++)
		{
			for(int mz = -1; mz <= 1; mz++)
			{
				int pos = (z + mz) * area + (y + my) * width + (x + mx);
				if( pos >=0 && pos < volume && inBuf[pos] > 0 && (mx != 0 || my != 0 || mz != 0) )
					neighbors[ (1+mz) * 9 + (1+my) * 3 + (1+mz) ] = true;
			}
		}
	}

	bool visited[27];
	for(int i=0; i<27; i++)
		visited[i] = false;

	std::vector< std::vector< int > > CCs;
	for(int i=0; i<27; i++)
	{
		
		if( neighbors[i] && !visited[i])
		{
			std::vector< int > tmpCCs, singleCCs;
			tmpCCs.push_back( i );
			singleCCs.push_back( i );
			visited[i] = true;

			while( !tmpCCs.empty() )
			{
				int curPos = tmpCCs.back();
				tmpCCs.pop_back();
				for(int mz = -1; mz <= 1; mz++)
				{
					for(int my = -1; my <= 1; my++)
					{
						for(int mx = -1; mx <= 1; mx++)
						{
							int pos = curPos + mz * 9 + my * 3 + mx; 
							if( neighbors[pos] && !visited[pos])
							{
								visited[pos] = true;
								tmpCCs.push_back( pos );
								singleCCs.push_back( pos );
							}
						}
					}
				}
			}

			CCs.push_back( singleCCs );
		}

		visited[i] = true;
	}

	if( CCs.size() != 1)
		return false;

	//condition 2 : p is 6-adjacent to only one CC of 26-neighbor - F

	for(int mx =-1; mx <= 1; mx++)
	{
		for(int my = -1; my <= 1; my++)
		{
			for(int mz = -1; mz <= 1; mz++)
			{
				int Idx = (1+mz) * 9 + (1+my) * 3 + (1+mz);
				if( !neighbors[ Idx ]  )
				{
					if( Idx != 13 ) neighbors[ Idx ] = true;
				}
				else
					neighbors[ Idx ] = false;

			}
		}
	}


	for(int i=0; i<CCs.size(); i++)
		CCs[i].clear();
	CCs.clear();


	for(int i=0; i<27; i++)
		visited[i] = false;

	for(int i=0; i<27; i++)
	{
		
		if( neighbors[i] && !visited[i])
		{
			std::vector< int > tmpCCs, singleCCs;
			tmpCCs.push_back( i );
			singleCCs.push_back( i );
			visited[i] = true;

			while( !tmpCCs.empty() )
			{
				int curPos = tmpCCs.back();
				tmpCCs.pop_back();
				for(int mz = -1; mz <= 1; mz++)
				{
					for(int my = -1; my <= 1; my++)
					{
						for(int mx = -1; mx <= 1; mx++)
						{
							int pos = curPos + mz * 9 + my * 3 + mx; 
							if( neighbors[pos] && !visited[pos])
							{
								visited[pos] = true;
								tmpCCs.push_back( pos );
								singleCCs.push_back( pos );
							}
						}
					}
				}
			}

			CCs.push_back( singleCCs );
		}

		visited[i] = true;
	}

	int adjCnt = 0;
	for(int i=0; i<CCs.size(); i++)
	{
		for(int j=0; j<CCs[i].size(); j++)
		{
			if( CCs[i][j] == 4 || CCs[i][j] == 10 || CCs[i][j] == 12 || CCs[i][j] == 14 || CCs[i][j] == 16 || CCs[i][j] == 22)
			{
				adjCnt++;
				break;
			}
		}
	}


	if( adjCnt != 1 )
		return false;
	else
		return true;
}

void BinaryDilation2D(unsigned char *imgBuf, const int width, const int height, const int R )
{
	const int Area = width * height;
	unsigned char *Buf = new unsigned char [ Area ];
	memcpy( Buf, imgBuf, Area );

	for(int i=0; i<Area; ++i)
	{
		int x = i % width;
		int y = i / width;

		if( Buf[i] <= 0 )
			continue;

		for(int my = -R; my <= R; ++my )
		{
			if( y+my >= height || y+my < 0 )
				continue;
			for(int mx =-R; mx <= R; ++mx )
			{
				if( x+mx >= width || x+mx < 0 )
					continue;
				
				int pos = (y+my) * width + (x+mx);

				imgBuf[ pos ] = 255;
			}
		}
	}
	delete [] Buf;
}

void GradientField2D(VectorCordinate*** Field, unsigned char* inBuf, const int width, const int height, const int zSize)
{
	for(int x=0; x<width; x++)
	{
		for(int y=0; y<height; y++)
		{
			for(int z=0; z<zSize; z++)
				Field[x][y][z] = VectorCordinate(.0, .0, .0);
		}
	}

	int Area = width * height;
	for(int x=1; x<width-1; x++)
	{
		for(int y=1; y<height-1; y++)
		{
			for(int z=1; z<zSize-1; z++)
			{
				int pos = z * Area + y * width + x;
				/*
				double diff_x = ( inBuf[pos+1] + inBuf[pos-1] - 2 * inBuf[pos] ) / 2.0;
				double diff_y = ( inBuf[pos+width] + inBuf[pos-width] - 2 * inBuf[pos] ) / 2.0;
				*/
				double diff_x = ( inBuf[pos+1] - inBuf[pos-1] ) / 2.0;
				double diff_y = ( inBuf[pos+width] - inBuf[pos-width]  ) / 2.0;

				Field[x][y][z].x = diff_x;
				Field[x][y][z].y = diff_y;
			}
		}
	}	
}

void GradientField3D(VectorCordinate*** Field, unsigned char* inBuf, const int width, const int height, const int zSize)
{
	for(int x=0; x<width; x++)
	{
		for(int y=0; y<height; y++)
		{
			for(int z=0; z<zSize; z++)
				Field[x][y][z] = VectorCordinate(.0, .0, .0);
		}
	}

	int Area = width * height;
	for(int x=0; x<width-1; x++)
	{
		for(int y=0; y<height-1; y++)
		{
			for(int z=0; z<zSize-1; z++)
			{
				int pos = z * Area + y * width + x;
				/*
				double diff_x = ( inBuf[pos+1] + inBuf[pos-1] - 2 * inBuf[pos] ) / 2.0;
				double diff_y = ( inBuf[pos+width] + inBuf[pos-width] - 2 * inBuf[pos] ) / 2.0;
				double diff_z = ( inBuf[pos+Area] + inBuf[pos-Area] - 2 * inBuf[pos] ) / 2.0;
				*/
				/*
				double diff_x = ( inBuf[pos+1] - inBuf[pos-1] ) / 2.0;
				double diff_y = ( inBuf[pos+width] - inBuf[pos-width]  ) / 2.0;
				double diff_z = ( inBuf[pos+Area] - inBuf[pos-Area] ) / 2.0;
				*/
				double diff_x = ( inBuf[pos+1] - inBuf[pos] ) / 2.0;
				double diff_y = ( inBuf[pos+width] - inBuf[pos]  ) / 2.0;
				double diff_z = ( inBuf[pos+Area] - inBuf[pos] ) / 2.0;
				Field[x][y][z] = VectorCordinate( diff_x, diff_y, diff_z );
			}
		}
	}
}

void GradientField2D_V2(VectorCordinate*** Field, unsigned char* inBuf, const int width, const int height, const int zSize)
{
	for(int x=0; x<width; x++)
	{
		for(int y=0; y<height; y++)
		{
			for(int z=0; z<zSize; z++)
				Field[x][y][z] = VectorCordinate(.0, .0, .0);
		}
	}

	int Area = width * height;


	for(int x=0; x<width; x++)
	{
		for(int y=0; y<height; y++)
		{
			for(int z=0; z<zSize; z++)
			{
				/*
				0 1 2
                3 4 5
				6 7 8
				*/
				float diff_Y[9];
				float diff_X[9];
				
				for(int i=0; i<9; i++)
					diff_X[i] = diff_Y[i] = .0;
				
				float mask[9];

				for(int i=0, my = -1; my<=1; my++)
				{
					for(int mx = -1; mx<=1; mx++ )
					{
						int pos = z * Area + (y+my) * width + (x+mx);
						if( x+mx < 0 || x+mx >= width || y+my < 0 || y+my >= height )
							mask[i] = 0;
						else
							mask[i] = inBuf[pos];
						i++;
					}
				}

				for(int i=0; i<9; i++)
				{
					const int mCenter = 4;
					if( mask[i] < mask[mCenter] )
					{
						if( i%3 == 0 )
							diff_X[i] = mask[i] - mask[mCenter];
						else if( i%3 == 2 )
							diff_X[i] = mask[mCenter] - mask[i];

						if( i/3 == 0 )
							diff_Y[i] = mask[mCenter] - mask[i];
						else if( i/3 == 2 )
							diff_Y[i] = mask[i] - mask[mCenter];
					}

				}

				VectorCordinate gradV;
				for(int i=0; i<9; i++)
				{
					if( i%2 == 0)
					{
						gradV.x += diff_X[i] / sqrt(2.0);
						gradV.y += diff_Y[i] / sqrt(2.0);
					}
					else
					{
						gradV.x += diff_X[i];
						gradV.y += diff_Y[i];
					}
				}
				gradV /= 6.0;
				Field[x][y][z] = gradV;
			}
		}
	}
}

void Skeleton2D( const unsigned char *input, unsigned char  *output, const int Xsize, const int Ysize )
{
   if( input==NULL || output==NULL ) return ;

   int N = 3, NP, SP;
   int MID = N/2;
   unsigned int p;
   bool cd_c, cd_d, change, step1=true;
   bool *flag = new bool[Xsize*Ysize];

   for( int i=0; i<Xsize*Ysize; i++)
      output[i] = input[i];

   do{
      for( int j=MID; j<Ysize-MID; j++)
         for( int k=MID; k<Xsize-MID; k++ )
            flag[ j*Xsize + k ] = false;

      for( int j=MID; j<Ysize-MID; j++){
         for( int k=MID; k<Xsize-MID; k++ ){
            p = j*Xsize+k;
            if( output[p] !=0 ){
               NP = 0;
               if( output[p + Xsize*(-1) + -1] != 0 )
                  NP++;
               if( output[p + Xsize*(-1) +  0] != 0 )
                  NP++;
               if( output[p + Xsize*(-1) +  1] != 0 )
                  NP++;
               if( output[p + Xsize*( 0) + -1] != 0 )
                  NP++;
               if( output[p + Xsize*( 0) +  1] != 0 )
                  NP++;
               if( output[p + Xsize*( 1) + -1] != 0 )
                  NP++;
               if( output[p + Xsize*( 1) +  0] != 0 )
                  NP++;
               if( output[p + Xsize*( 1) +  1] != 0 )
                  NP++;

               SP = 0;
               if( output[p+Xsize*(-1)+ -1]==0 && output[p+Xsize*(-1)+  0]!=0 )
                  SP++;
               if( output[p+Xsize*(-1)+  0]==0 && output[p+Xsize*(-1)+  1]!=0 )
                  SP++;
               if( output[p+Xsize*(-1)+  1]==0 && output[p+Xsize*( 0)+  1]!=0 )
                  SP++;
               if( output[p+Xsize*( 0)+  1]==0 && output[p+Xsize*( 1)+  1]!=0 )
                  SP++;
               if( output[p+Xsize*( 1)+  1]==0 && output[p+Xsize*( 1)+  0]!=0 )
                  SP++;
               if( output[p+Xsize*( 1)+  0]==0 && output[p+Xsize*( 1)+ -1]!=0 )
                  SP++;
               if( output[p+Xsize*( 1)+ -1]==0 && output[p+Xsize*( 0)+ -1]!=0 )
                  SP++;
               if( output[p+Xsize*( 0)+ -1]==0 && output[p+Xsize*(-1)+ -1]!=0 )
                  SP++;

               if( step1 == true ){
                  cd_c = false;
                  if( output[p+Xsize*(-1)+  0]==0 || output[p+Xsize*( 0)+  1]==0 || output[p+Xsize*( 1)+  0]==0 )
                     cd_c = true;
                  cd_d = false;
                  if( output[p+Xsize*( 0)+  1]==0 || output[p+Xsize*( 1)+  0]==0 || output[p+Xsize*( 0)+ -1]==0 )
                     cd_d = true;
               }
               else{  // step2
                  cd_c = false;
                  if( output[p+Xsize*(-1)+  0]==0 || output[p+Xsize*( 0)+  1]==0 || output[p+Xsize*( 0)+  -1]==0 )
                     cd_c = true;
                  cd_d = false;
                  if( output[p+Xsize*(-1)+  0]==0 || output[p+Xsize*( 1)+  0]==0 || output[p+Xsize*( 0)+ -1]==0 )
                     cd_d = true;
               }
               if ( 2<=NP && NP<=6 && SP==1 && cd_c==true && cd_d==true )
                  flag[p] = true;
            }
         } // end of for_k
      } // end of for_j
      change = false;
      for( int j=MID; j<Ysize-MID; j++)
         for( int k=MID; k<Xsize-MID; k++ ){
            if( flag[ j*Xsize + k ] == true ){
               output[ j*Xsize + k ] = 0x00;
               change = true;
            }
         }
      step1 = !step1;
   }
   while( change==true );

   delete []flag;
 //  return output;
}
void Skeletonization3D_Slicewise( unsigned char* inBuf, unsigned char* skeleBuf, const int width, const int height, const int zSize )
{
	int area = width * height;

	for(int i=0; i<zSize; i++)
	{
		unsigned char* slice = new unsigned char [area];
		unsigned char* sliceSkele = new unsigned char [ area ];

		memcpy( slice, inBuf + i * area, area);

		Skeleton2D( slice, sliceSkele, width, height);

		memcpy( skeleBuf + i * area, sliceSkele, area );

		delete [] slice;
		delete [] sliceSkele;
	}

}

void thinning_1frT( const unsigned char *input, unsigned char *output, const int Xsize, const int Ysize )
{
   if( input==NULL || output==NULL ) return ;

   int N = 3, NP, SP;
   int MID = N/2;
   unsigned int p;
   bool cd_c, cd_d, change, step1=true;
   bool *flag = new bool[Xsize*Ysize];

   for( int i=0; i<Xsize*Ysize; i++)
      output[i] = input[i];

   do{
      for( int j=MID; j<Ysize-MID; j++)
         for( int k=MID; k<Xsize-MID; k++ )
            flag[ j*Xsize + k ] = false;

      for( int j=MID; j<Ysize-MID; j++){
         for( int k=MID; k<Xsize-MID; k++ ){
            p = j*Xsize+k;
            if( output[p] !=0 ){
               NP = 0;
               if( output[p + Xsize*(-1) + -1] != 0 )
                  NP++;
               if( output[p + Xsize*(-1) +  0] != 0 )
                  NP++;
               if( output[p + Xsize*(-1) +  1] != 0 )
                  NP++;
               if( output[p + Xsize*( 0) + -1] != 0 )
                  NP++;
               if( output[p + Xsize*( 0) +  1] != 0 )
                  NP++;
               if( output[p + Xsize*( 1) + -1] != 0 )
                  NP++;
               if( output[p + Xsize*( 1) +  0] != 0 )
                  NP++;
               if( output[p + Xsize*( 1) +  1] != 0 )
                  NP++;

               SP = 0;
               if( output[p+Xsize*(-1)+ -1]==0 && output[p+Xsize*(-1)+  0]!=0 )
                  SP++;
               if( output[p+Xsize*(-1)+  0]==0 && output[p+Xsize*(-1)+  1]!=0 )
                  SP++;
               if( output[p+Xsize*(-1)+  1]==0 && output[p+Xsize*( 0)+  1]!=0 )
                  SP++;
               if( output[p+Xsize*( 0)+  1]==0 && output[p+Xsize*( 1)+  1]!=0 )
                  SP++;
               if( output[p+Xsize*( 1)+  1]==0 && output[p+Xsize*( 1)+  0]!=0 )
                  SP++;
               if( output[p+Xsize*( 1)+  0]==0 && output[p+Xsize*( 1)+ -1]!=0 )
                  SP++;
               if( output[p+Xsize*( 1)+ -1]==0 && output[p+Xsize*( 0)+ -1]!=0 )
                  SP++;
               if( output[p+Xsize*( 0)+ -1]==0 && output[p+Xsize*(-1)+ -1]!=0 )
                  SP++;

               if( step1 == true ){
                  cd_c = false;
                  if( output[p+Xsize*(-1)+  0]==0 || output[p+Xsize*( 0)+  1]==0 || output[p+Xsize*( 1)+  0]==0 )
                     cd_c = true;
                  cd_d = false;
                  if( output[p+Xsize*( 0)+  1]==0 || output[p+Xsize*( 1)+  0]==0 || output[p+Xsize*( 0)+ -1]==0 )
                     cd_d = true;
               }
               else{  // step2
                  cd_c = false;
                  if( output[p+Xsize*(-1)+  0]==0 || output[p+Xsize*( 0)+  1]==0 || output[p+Xsize*( 0)+  -1]==0 )
                     cd_c = true;
                  cd_d = false;
                  if( output[p+Xsize*(-1)+  0]==0 || output[p+Xsize*( 1)+  0]==0 || output[p+Xsize*( 0)+ -1]==0 )
                     cd_d = true;
               }
               if ( 2<=NP && NP<=6 && SP==1 && cd_c==true && cd_d==true )
                  flag[p] = true;
            }
         } // end of for_k
      } // end of for_j
      change = false;
      for( int j=MID; j<Ysize-MID; j++)
         for( int k=MID; k<Xsize-MID; k++ ){
            if( flag[ j*Xsize + k ] == true ){
               output[ j*Xsize + k ] = 0x00;
               change = true;
            }
         }
      step1 = !step1;
   }
   while( change==true );

   delete []flag;
 //  return output;
};

void MedianFilter3D(unsigned char* ImgBuf, const int width, const int height, const int zSize, const int Radius)
{
	int imgArea = width * height;
	int imgVolume = imgArea * zSize;

	unsigned char* tmpBuf =  new unsigned char [ imgVolume ];
	int samples = (2 * Radius + 1);
	samples *= ( samples * samples );

	unsigned char* orderBuf = new unsigned char [ samples ];

	for(int i = 0; i<imgVolume; i++)
	{
		int x = i % width;
		int y = (i % imgArea) / width;
		int z = i / imgArea;

		if( x - Radius >= 0 && x + Radius < width && 
			y - Radius >= 0 && y + Radius < height &&
			z - Radius >= 0 && z + Radius < zSize )
		{
			int pos = 0;
			for(int mx = -Radius; mx<=Radius; mx++)
			{
				for(int my = -Radius; my<=Radius; my++)
				{
					for(int mz = -Radius; mz<=Radius; mz++)
					{
						int newPos = i + mz * imgArea + my * width + mx;
						orderBuf[ pos ] = ImgBuf[ newPos ];
						pos++;
					}	
				}
			}

			int median = samples / 2;
			bool SWAP_FLAG;
			do
			{
				SWAP_FLAG = false;
				for( int i=0; i<samples-1; i++)
				{
					if( orderBuf[i] > orderBuf[i+1] )
					{
						unsigned char tmp = orderBuf[i];
						orderBuf[i] = orderBuf[i+1];
						orderBuf[i+1] = tmp;
						SWAP_FLAG = true;
					}
				}
			}while( SWAP_FLAG );
			
			tmpBuf[i] = orderBuf[ median ];
		}
		else
			tmpBuf[i] = ImgBuf[i];
	}
	
	memcpy( ImgBuf, tmpBuf, imgVolume );

	delete [] tmpBuf;
	delete [] orderBuf;
}







