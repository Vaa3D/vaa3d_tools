// find connected components and label them  
// F. Long
// 20080508
// revised from matlab bwlabelnmex.cpp
// 20090107: add some memory freeing code for detecting the memory leak


#ifndef __BW_LABEL_2D3D__
#define __BW_LABEL_2D3D__

#include <stdio.h>

#include "FL_neighborhood.h"
#include "FL_unionFind.h"
#include "FL_neighborhoodWalker.h"

#include "FL_defType.h"

unsigned short int connectedComponents(bool *BW, unsigned short int *L, V3DLONG num_elements, NeighborhoodWalker *walker);

//unsigned short int connectedComponents(bool *BW, unsigned short int *L, V3DLONG num_elements, NeighborhoodWalker *walker)
//{
//    V3DLONG p;
//    V3DLONG q;
//    V3DLONG next_label = -1;
//    V3DLONG num_sets;
//    UnionFind *uf=0;
//    bool found;
//	V3DLONG *idx=0;
//	
//    uf = new UnionFind(1000);
//    
//	//initialize L as 0, 20090103
//    for (p = 0; p < num_elements; p++)
//    {
//		L[p] = 0;
//	}
//	
//    for (p = 0; p < num_elements; p++)
//    {
//        if (BW[p] != 0)
//        {
//            /*
//             * Find the first trailing nonzero neighbor.
//             */
//            found = false;
//            walker->setWalkerLocation(p);
//			
//            while (walker->getNextInboundsNeighbor(&q, NULL))
////            while (walker->getNextInboundsNeighbor(&q, idx))
//
//            {
//                if (BW[q] != 0)
//                {
//                    /*
//                     * Assign L[p] the same label as L[q]
//                     */
//                    found = true;
//                    L[p] = L[q];
//                    break;
//                }
//            }
//
//            if (! found)
//            {
//                /*
//                 * There were no trailing nonzero neighbors.  Increment
//                 * next_label and make space for it in the unionfind structure.
//                 */
//                next_label++;
//                uf->unionFindNewNode();
//                L[p] = next_label;
//            }
//            else
//            {
//                /*
//                 * Look for additional trailing nonzero neighbors.  If any
//                 * exist that have a different label than what has just
//                 * been assigned to L[p], record the equivalence by calling
//                 * unionFindNewPair().
//                 */
//                while (walker->getNextInboundsNeighbor( &q, NULL))
//                {
//                    if ((BW[q] != 0) && (L[q] != L[p]))
//                    {
//                        uf->unionFindMergePair((V3DLONG) L[p], (V3DLONG) L[q]);
//                    }
//                }
//            }
//        }
//    }
//    
//    num_sets = uf->unionFindEnumerate(1);
//    
//    for (p = 0; p < num_elements; p++)
//    {
//        if (BW[p] != 0)
//        {
//            L[p] = (unsigned short int)uf->unionFindGetSet((V3DLONG) L[p]);
//
//        }
//    }
//    
//	//090107 add the memory freeing part
//	if (uf) {delete uf; uf=0;}
//	
//	//
//	
//    return num_sets;
//}


// use default neighborhood (when nh_code = 2 or 3) or use standard neighborhood (when nh_code = 4, 8 for 2d and 6,18,26 for 3d)
template <class T> unsigned short int findConnectedComponent(T *data, const V3DLONG *data_sz, const V3DLONG data_num_dims, const V3DLONG nh_code, unsigned short int *L)
{
	Neighborhood *nh=0;
	NeighborhoodWalker *nh_walker=0;
	V3DLONG num_elements;
	V3DLONG num_sets; 
	V3DLONG i;
	
	if ((!data)||(!L)||(!data_sz))
	{
		fprintf(stderr, "allocate memory for *data, or *L, or *data_sz before calling findConnectedComponent\n"); 
		return 0;
	}
	
	if ((data_num_dims!=2)&&(data_num_dims!=3))
	{
		fprintf(stderr, "Only support 2D and 3D data.\n"); 
		return 0;
	}
	
	nh = new Neighborhood(nh_code);
	nh_walker = new NeighborhoodWalker(nh, data_sz, data_num_dims, NH_SKIP_CENTER | NH_SKIP_LEADING);

	if (data_num_dims==2)
		num_elements = (V3DLONG)(data_sz[0]*data_sz[1]);
	else
		num_elements = (V3DLONG)(data_sz[0]*data_sz[1]*data_sz[2]);

	bool *data1 = new bool [num_elements];
	
	for (i=0;i<num_elements; i++)
		data1[i] = (bool) data[i];
		
    num_sets = connectedComponents(data1, L, num_elements, nh_walker);
	
	//090107 add the memory freeing code
	if (data1) {delete []data1; data1=0;}
	if (nh) {delete nh; nh=0;}
	if (nh_walker) {delete nh_walker; nh_walker=0;}
		
	return num_sets;
}

// use user defined neighborhood
template <class T, class T1> unsigned short int findConnectedComponent(T *data, const V3DLONG *data_sz, const V3DLONG data_num_dims, const V3DLONG nh_code, 
                                                                      T1 *D, const V3DLONG *D_sz, const V3DLONG D_num_dims, unsigned short int *L)
{
	Neighborhood *nh=0;
	NeighborhoodWalker *nh_walker=0;
	V3DLONG num_sets;
	V3DLONG num_elements;
	V3DLONG i;

	if ((!data)||(!L)||(!data_sz)||(!D))
	{
		fprintf(stderr, "allocate memory for *data, or *L, or *data_sz, or *D before calling findConnectedComponent\n"); 
		return 0;
	}
	
	if ((data_num_dims!=2)&&(data_num_dims!=3))
	{
		fprintf(stderr, "Only support 2D and 3D data.\n"); 
		return 0;
	}

	nh = new Neighborhood(D, D_sz, D_num_dims, NH_CENTER_MIDDLE_ROUNDDOWN);
	nh_walker = new NeighborhoodWalker(nh, data_sz, data_num_dims, NH_SKIP_CENTER | NH_SKIP_LEADING);
		
	if (data_num_dims==2)
		num_elements = (V3DLONG)(data_sz[0]*data_sz[1]);
	else
		num_elements = (V3DLONG)(data_sz[0]*data_sz[1]*data_sz[2]);

	bool *data1 = new bool [num_elements];
	
	for (i=0;i<num_elements; i++)
		data1[i] = (bool) data[i];
		
    num_sets = connectedComponents(data1, L, num_elements, nh_walker);

	//090107 add the memory freeing code
	if (data1) {delete []data1; data1=0;}
	if (nh) {delete nh; nh=0;}
	if (nh_walker) {delete nh_walker; nh_walker=0;}

	return num_sets;
	
}


#endif
