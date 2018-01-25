//------------------------------------------------------------------------------------------------
// Copyright (c) 2012  Alessandro Bria and Giulio Iannello (University Campus Bio-Medico of Rome).  
// All rights reserved.
//------------------------------------------------------------------------------------------------

/*******************************************************************************************************************************************************************************************
*    LICENSE NOTICE
********************************************************************************************************************************************************************************************
*    By downloading/using/running/editing/changing any portion of codes in this package you agree to this license. If you do not agree to this license, do not download/use/run/edit/change
*    this code.
********************************************************************************************************************************************************************************************
*    1. This material is free for non-profit research, but needs a special license for any commercial purpose. Please contact Alessandro Bria at a.bria@unicas.it or Giulio Iannello at 
*       g.iannello@unicampus.it for further details.
*    2. You agree to appropriately cite this work in your related studies and publications.
*
*       Bria, A., Iannello, G., "TeraStitcher - A Tool for Fast 3D Automatic Stitching of Teravoxel-sized Microscopy Images", (2012) BMC Bioinformatics, 13 (1), art. no. 316.
*
*    3. This material is provided by  the copyright holders (Alessandro Bria  and  Giulio Iannello),  University Campus Bio-Medico and contributors "as is" and any express or implied war-
*       ranties, including, but  not limited to,  any implied warranties  of merchantability,  non-infringement, or fitness for a particular purpose are  disclaimed. In no event shall the
*       copyright owners, University Campus Bio-Medico, or contributors be liable for any direct, indirect, incidental, special, exemplary, or  consequential  damages  (including, but not 
*       limited to, procurement of substitute goods or services; loss of use, data, or profits;reasonable royalties; or business interruption) however caused  and on any theory of liabil-
*       ity, whether in contract, strict liability, or tort  (including negligence or otherwise) arising in any way out of the use of this software,  even if advised of the possibility of
*       such damage.
*    4. Neither the name of University  Campus Bio-Medico of Rome, nor Alessandro Bria and Giulio Iannello, may be used to endorse or  promote products  derived from this software without
*       specific prior written permission.
********************************************************************************************************************************************************************************************/

/******************
*    CHANGELOG    *
*******************
* 2017-02-14.  Giulio.     @CREATED 
*/

#include <limits>
#include "TPAlgo2MST.h"
#include "S_config.h"
//#include "volumemanager.config.h"
//#include "vmStackedVolume.h"
//#include "vmVirtualStack.h"
#include "StackedVolume.h"
#include "UnstitchedVolume.h"
#include "Displacement.h"

//using namespace volumemanager;
//using namespace iomanager;

//triplet data type definition
typedef struct 							
{
	float V;
	float H;
	float D;
}triplet;

TPAlgo2MST::TPAlgo2MST(MultiLayersVolume * _volume) : TPAlgo2(S_FATPM_SP_TREE, _volume)
{
	#if S_VERBOSE>4
	printf("........in TPAlgo2MST::TPAlgo2MST(MultiLayersVolume * _volume)");
	#endif
}

/*************************************************************************************************************
* Finds the optimal tile placement on the <volume> object member via Minimum Spanning Tree.
* Stacks matrix is considered as a graph  where  displacements are edges and stacks are nodes. The inverse of
* displacements reliability factors are  edge weights,  so that a totally unreliable displacement has a weight
* 1/0.0 = +inf and a totally reliable displacement has a weight 1/1.0 = 1. Thus, weights will be in [1, +inf].
* After computing the MST, the absolute tile positions are obtained from a stitchable source stack by means of
* navigating the MST and using the selected displacements.
* PROs: very general method; it ignores bad displacements since they have +inf weight.
* CONs: the best path between two adjacent stacks can pass through many stacks even if the rejected displacem-
*       ent is quite reliable, with a very little reliability gain.  This implies possible bad absolute  posi-
*       tions estimations when the path is too long.
*
* After recomputing all displacements check if MultiLayerVolume size should be updated
**************************************************************************************************************/
int **TPAlgo2MST::execute() throw (iom::exception)
{
	#if S_VERBOSE > 2
	printf("....in TPAlgo2MST::execute()");
	#endif

	float ****D;									//distances matrix for VHD directions
	//std::pair<int*,int*> ***predecessors;	        //predecessor matrix for VHD directions
	struct triplet {
		int *first;                                 // row index
		int *second;                                // col index
		int *third;                                 // layer index
	} ***predecessors;                              //predecessor matrix for VHD directions
	int src_row=0, src_col=0, src_layer=0;		    //source vertex

	int N_ROWS = ((UnstitchedVolume *) (volume->getLAYER(0)))->volume->getN_ROWS();
	int N_COLS = ((UnstitchedVolume *) (volume->getLAYER(0)))->volume->getN_COLS();
	vm::VirtualStack*** vstack;
	
	//0) fixing the source as the stitchable VirtualStack nearest to the top-left corner
	float min_distance = std::numeric_limits<float>::infinity();
	for(int layer=0; layer<volume->getN_LAYERS(); layer++) {
		vstack = ((UnstitchedVolume *) (volume->getLAYER(layer)))->volume->getSTACKS();
		for(int row=0; row<N_ROWS; row++) {
			for(int col=0; col<N_COLS; col++)
 				if ( vstack[row][col]->isStitchable() && sqrt((float)(row*row+col*col+layer*layer)) < min_distance )
				{
					src_row      = row;
					src_col      = col;
					src_layer    = layer;
					min_distance = sqrt((float)(row*row+col*col));
				}
		}
	}
	#if S_VERBOSE > 4
	printf("....in TPAlgoMST::execute(): SOURCE is [%d,%d]\n",src_row,src_col);
	#endif

	//1) initializing distance and predecessor matrix
	D = new float ***[volume->getN_LAYERS()];
	//predecessors = new std::pair<int*,int*> **[volume->getN_LAYERS()];
	predecessors = new triplet **[volume->getN_LAYERS()];
	for(int layer=0; layer<volume->getN_LAYERS(); layer++) {
		D[layer] = new float**[N_ROWS];
		//predecessors[layer] = new std::pair<int*,int*> *[volume->getN_COLS()];
		predecessors[layer] = new triplet *[N_ROWS];
		for(int row=0; row<N_ROWS; row++)
		{
			D[layer][row] = new float*[N_COLS];
			//predecessors[layer][row] = new std::pair<int*,int*>[volume->getN_COLS()];
			predecessors[layer][row] = new triplet[N_COLS];
			for(int col=0; col<N_COLS; col++)
			{
				D[layer][row][col] = new float[3];
				predecessors[layer][row][col].first = new int[3];
				predecessors[layer][row][col].second = new int[3];
				predecessors[layer][row][col].third = new int[3];
				for(int k=0; k<3; k++)
				{
					D[layer][row][col][k] = std::numeric_limits<float>::infinity();
					predecessors[layer][row][col].first[k] = predecessors[layer][row][col].second[k] = predecessors[layer][row][col].third[k] = -1;
				}
			}
		}
	}
	D[src_layer][src_row][src_col][0] = D[src_layer][src_row][src_col][1] = D[src_layer][src_row][src_col][2] = 0.0F;

	//2) processing edges in order to obtain distance matrix
	for(int vertices=1; vertices <= volume->getN_LAYERS()*N_ROWS*N_COLS; vertices++) 
	{
		for(int E_layer=0; E_layer<volume->getN_LAYERS(); E_layer++) 
		{
			vstack = ((UnstitchedVolume *) (volume->getLAYER(E_layer)))->volume->getSTACKS();
			for(int E_row = 0; E_row<N_ROWS; E_row++) 
			{
				for(int E_col = 0; E_col<N_COLS; E_col++) 
				{
					for(int k=0; k<3; k++) 
					{
						if(E_row != N_ROWS-1 )	//NORTH-SOUTH displacements
						{
							float weight = SAFE_DIVIDE(1, vstack[E_row][E_col]->getSOUTH()[0]->getReliability(direction(k)), S_UNRELIABLE_WEIGHT);
							if(D[E_layer][E_row][E_col][k] + weight < D[E_layer][E_row+1][E_col][k])
							{
								D[E_layer][E_row+1][E_col][k] = D[E_layer][E_row][E_col][k] + weight;
								predecessors[E_layer][E_row+1][E_col].first[k] = E_row;
								predecessors[E_layer][E_row+1][E_col].second[k] = E_col;
								predecessors[E_layer][E_row+1][E_col].third[k] = E_layer;
							}
							if(D[E_layer][E_row+1][E_col][k] + weight < D[E_layer][E_row][E_col][k])
							{
								D[E_layer][E_row][E_col][k] = D[E_layer][E_row+1][E_col][k] + weight;
								predecessors[E_layer][E_row][E_col].first[k] = E_row+1;
								predecessors[E_layer][E_row][E_col].second[k]= E_col;
								predecessors[E_layer][E_row][E_col].third[k]= E_layer;
							}
						}
						if(E_col != N_COLS-1 )	//EAST-WEST displacements
						{
							float weight = SAFE_DIVIDE(1, vstack[E_row][E_col]->getEAST()[0]->getReliability(direction(k)), S_UNRELIABLE_WEIGHT);
							if(D[E_layer][E_row][E_col][k] + weight < D[E_layer][E_row][E_col+1][k])
							{
								D[E_layer][E_row][E_col+1][k] = D[E_layer][E_row][E_col][k] + weight;
								predecessors[E_layer][E_row][E_col+1].first[k] = E_row;
								predecessors[E_layer][E_row][E_col+1].second[k]= E_col;
								predecessors[E_layer][E_row][E_col+1].third[k]= E_layer;
							}
							if(D[E_layer][E_row][E_col+1][k] + weight < D[E_layer][E_row][E_col][k])
							{
								D[E_layer][E_row][E_col][k] = D[E_layer][E_row][E_col+1][k] + weight;
								predecessors[E_layer][E_row][E_col].first[k] = E_row;
								predecessors[E_layer][E_row][E_col].second[k] = E_col+1;
								predecessors[E_layer][E_row][E_col].third[k] = E_layer;
							}
						}
						if(E_layer != volume->getN_LAYERS()-1 )	//UP-DOWN displacements
						{
							float weight = SAFE_DIVIDE(1, volume->disps[E_layer]->at(E_row).at(E_col)->getReliability(direction(k)), S_UNRELIABLE_WEIGHT);
							if(D[E_layer][E_row][E_col][k] + weight < D[E_layer+1][E_row][E_col][k])
							{
								D[E_layer+1][E_row][E_col][k] = D[E_layer][E_row][E_col][k] + weight;
								predecessors[E_layer+1][E_row][E_col].first[k] = E_row;
								predecessors[E_layer+1][E_row][E_col].second[k] = E_col;
								predecessors[E_layer+1][E_row][E_col].third[k] = E_layer;
							}
							if(D[E_layer+1][E_row][E_col][k] + weight < D[E_layer][E_row][E_col][k])
							{
								D[E_layer][E_row][E_col][k] = D[E_layer+1][E_row][E_col][k] + weight;
								predecessors[E_layer][E_row][E_col].first[k] = E_row;
								predecessors[E_layer][E_row][E_col].second[k]= E_col;
								predecessors[E_layer][E_row][E_col].third[k]= E_layer+1;
							}
						}
					}
				}
			}
		}
	}

	#if S_VERBOSE > 4
	for(int k=0; k<3; k++)
	{
		printf("\n\n....in TPAlgoMST2::execute(): %d DIRECTION:\n",  k);
		for(int layer = 0; layer < volume->getN_LAYERS(); layer++)
		{
			printf("\nLayer: %d\n\n",layer);
			printf("\n\t");
			for(int col=0; col < N_COLS; col++)
				printf("[%d]\t\t", col);
			printf("\n\n\n");
			for(int row = 0; row < N_ROWS; row++)
			{
				printf("[%d]\t",row);
				for(int col= 0; col < N_COLS; col++)
					printf("(%d,%d,%d)\t", predecessors[layer][row][col].first[k], predecessors[layer][row][col].second[k], predecessors[layer][row][col].third[k]);
				printf("\n\n\n");
			}
		
			printf("\n");
			printf("\n\t");
			for(int col=0; col < N_COLS; col++)
				printf("[%d]\t\t", col);
			printf("\n\n\n");
			for(int row = 0; row < N_ROWS; row++)
			{
				printf("[%d]\t",row);
				for(int col= 0; col < N_COLS; col++)
					printf("%8.3f\t", D[layer][row][col][k]);
				printf("\n\n\n");
			}
			printf("\n\n");
		}
	}

	char stopstr[100];
	printf("---------------> give me one character followed by a <cr> \n");
	scanf("%s",stopstr);
	#endif

	//3) resetting to 0 all stacks absolute coordinates
	for(int layer = 0; layer < volume->getN_LAYERS(); layer++)
	{
		vstack = ((UnstitchedVolume *) (volume->getLAYER(layer)))->volume->getSTACKS();
		// set to 0 coords of the layer ?
		for(int row = 0; row<N_ROWS; row++)
		{
			for(int col = 0; col<N_COLS; col++)
			{
				vstack[row][col]->setABS_V(0);
				vstack[row][col]->setABS_H(0);
				vstack[row][col]->setABS_D(0);
			}
		}
	}

	//4) assigning absolute coordinates using predecessors matrix
	for(int layer = 0; layer < volume->getN_LAYERS(); layer++)
	{
		for(int row = 0; row<N_ROWS; row++)
		{
			for(int col = 0; col<N_COLS; col++)
			{
				vm::VirtualStack *source = ((UnstitchedVolume *) (volume->getLAYER(src_layer)))->volume->getSTACKS()[src_row][src_col];
				vm::VirtualStack  *dest  = ((UnstitchedVolume *) (volume->getLAYER(layer)))->volume->getSTACKS()[row][col];;
				if(!(row == src_row && col == src_col))
				{
					for(int k=0; k<3; k++)
					{
						vm::VirtualStack *v, *u;
						int v_layer;
						v       = dest;
						v_layer = layer;

						#if S_VERBOSE > 4
						printf("S[%d,%d] [%d]_path:\n", k, row, col);
						#endif
						while (v != source)
						{
							int u_row, u_col, u_layer;
							u_row = predecessors[v_layer][v->getROW_INDEX()][v->getCOL_INDEX()].first[k];
							if(u_row>= N_ROWS || u_row < 0)
								throw iom::exception("...in TPAlgoMST::execute(): error in the predecessor matrix");
							u_col = predecessors[v_layer][v->getROW_INDEX()][v->getCOL_INDEX()].second[k];
							if(u_col>= N_COLS || u_col < 0)
								throw iom::exception("...in TPAlgoMST::execute(): error in the predecessor matrix");
							u_layer = predecessors[v_layer][v->getROW_INDEX()][v->getCOL_INDEX()].third[k];;
							if(u_layer>= volume->getN_LAYERS() || u_layer < 0)
								throw iom::exception("...in TPAlgoMST::execute(): error in the predecessor matrix");
							u = ((UnstitchedVolume *) (volume->getLAYER(u_layer)))->volume->getSTACKS()[u_row][u_col ];

							#if S_VERBOSE > 4
							printf("\t[%d,%d,%d] (ABS_[%d] = %d %+d)\n",u->getROW_INDEX(), u->getCOL_INDEX(), u_layer, k, dest->getABS(k), u->getDisplacement(v)->getDisplacement(direction(k)));
							#endif

							int d_abs = dest->getABS(k);
							int u_disp;
							
							if ( u_layer == v_layer ) { // predecessor is in the same layer
								u_disp = u->getDisplacement(v)->getDisplacement(direction(k));
								dest->setABS(dest->getABS(k) + u->getDisplacement(v)->getDisplacement(direction(k)), k);
							}
							else { // predecessor is in a different layer
								if ( u_layer < v_layer ) {
									u_disp = volume->disps[u_layer]->at(u_row).at(u_col)->getDisplacement(direction(k));
									dest->setABS(dest->getABS(k) + volume->disps[u_layer]->at(u_row).at(u_col)->getDisplacement(direction(k)), k);
								}
								else { // u_layer > v_layer  
									// disps[v_layer] must be used since the interlayer information is the same in both cases
									u_disp = volume->disps[v_layer]->at(u_row).at(u_col)->getDisplacement(direction(k));
									dest->setABS(dest->getABS(k) - volume->disps[v_layer]->at(u_row).at(u_col)->getDisplacement(direction(k)), k);
								}
							}
							d_abs = dest->getABS(k);
							v = u;
							v_layer = u_layer;

							if(dest->isStitchable() && !(v->isStitchable()))
								printf("\nWARNING! in TPAlgoMST::execute(): direction %d: VirtualStack [%d,%d] is passing through VirtualStack [%d,%d], that is NOT STITCHABLE\n", k, row, col, v->getROW_INDEX(), v->getCOL_INDEX());
						}
						#if S_VERBOSE > 4
						printf("\n");
						#endif
					}

					#if S_VERBOSE > 4
					system("PAUSE");
					#endif
				}

				// the layer D coordinate must be added because initially it was relative to the layer and not the whole volume
				dest->setABS(dest->getABS(2) + volume->getLAYER_COORDS(layer,2),2);
			}
		}
	}

	//5) translating stacks absolute coordinates by choosing VirtualStack[0][0] as the new source
	int trasl_X = ((UnstitchedVolume *) (volume->getLAYER(0)))->volume->getSTACKS()[0][0]->getABS_V();
	int trasl_Y = ((UnstitchedVolume *) (volume->getLAYER(0)))->volume->getSTACKS()[0][0]->getABS_H();
	int trasl_Z = ((UnstitchedVolume *) (volume->getLAYER(0)))->volume->getSTACKS()[0][0]->getABS_D();
	for(int layer = 0; layer < volume->getN_LAYERS(); layer++)
	{
		vm::VirtualStack ***vstacks = ((UnstitchedVolume *) (volume->getLAYER(layer)))->volume->getSTACKS();
		for(int row = 0; row<N_ROWS; row++)
		{
			for(int V_col = 0; V_col<N_COLS; V_col++)
			{
				vstacks[row][V_col]->setABS_V(vstacks[row][V_col]->getABS_V()-trasl_X);
				vstacks[row][V_col]->setABS_H(vstacks[row][V_col]->getABS_H()-trasl_Y);
				vstacks[row][V_col]->setABS_D(vstacks[row][V_col]->getABS_D()-trasl_Z);
			}
		}
	}

	//6) saving new stack absolute coordinates of tile (0,0) of each layer and translating all stack absolute coordinates of layers with respect to tile (0,0)
	int **newABS_VHD = new int *[volume->getN_LAYERS()];
	newABS_VHD[0] = (int *) 0; 
	// first layer (0) has coordinates (0,0,0) by definition and it is skipped
	for(int layer = 1; layer < volume->getN_LAYERS(); layer++)
	{
		newABS_VHD[layer] = new int[3];
		// save absolute poistions of of tile (0,0)
		newABS_VHD[layer][0] = trasl_X = ((UnstitchedVolume *) (volume->getLAYER(layer)))->volume->getSTACKS()[0][0]->getABS_V();
		newABS_VHD[layer][1] = trasl_Y = ((UnstitchedVolume *) (volume->getLAYER(layer)))->volume->getSTACKS()[0][0]->getABS_H();
		newABS_VHD[layer][2] = trasl_Z = ((UnstitchedVolume *) (volume->getLAYER(layer)))->volume->getSTACKS()[0][0]->getABS_D();
		vm::VirtualStack ***vstacks = ((UnstitchedVolume *) (volume->getLAYER(layer)))->volume->getSTACKS();
		for(int row = 0; row<N_ROWS; row++)
		{
			for(int V_col = 0; V_col<N_COLS; V_col++)
			{
				vstacks[row][V_col]->setABS_V(vstacks[row][V_col]->getABS_V()-trasl_X);
				vstacks[row][V_col]->setABS_H(vstacks[row][V_col]->getABS_H()-trasl_Y);
				vstacks[row][V_col]->setABS_D(vstacks[row][V_col]->getABS_D()-trasl_Z);
			}
		}
	}

	//deallocating distance matrix and predecessor matrix
	for(int layer = 0; layer < volume->getN_LAYERS(); layer++)
	{
		for(int row=0; row<volume->getN_ROWS(); row++)
		{
			for(int col=0; col<volume->getN_COLS(); col++)
			{
				delete[] D[layer][row][col];
				delete[] predecessors[layer][row][col].first;
				delete[] predecessors[layer][row][col].second;
			}
			delete[] D[layer][row];
			delete[] predecessors[layer][row];
		}
			delete[] D[layer];
			delete[] predecessors[layer];
	}
	delete[] D;
	delete[] predecessors;

	return newABS_VHD;
}
