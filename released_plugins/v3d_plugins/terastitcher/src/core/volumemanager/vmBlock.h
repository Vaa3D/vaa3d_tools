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
* 2015-01-17. Alessandro. @ADDED constructor for initialization from XML.
* 2015-01-17. Alessandro. @FIXED missing throw(iom::exception) declaration in many methods.
* 2014-09-05. Alessandro. @ADDED 'z_end' parameter in 'loadXML()' method to support sparse data feature.
* 2014-08-25. Alessandro. @ADDED missing 'throw (iom::iom::exception)' statement in the 'loadImageStack()' method's signature
*/

#ifndef _VM_BLOCK_H
#define _VM_BLOCK_H

#include "iomanager.config.h"
#include "tinyxml.h"
#include "vmVirtualStack.h" 

class BlockVolume;
class Displacement;

//TYPE DEFINITIONS
//structure representing a substack
//D0: first slice, D1: last slice, ind0: index of 1st block (containing D0), ind1: index of last block (containing D1) 
typedef struct {int D0, D1, ind0, ind1;} Segm_t;

class Block : public VirtualStack
{
	private:

		BlockVolume* CONTAINER;					//pointer to <VirtualVolume> object that contains the current object
		int          N_BLOCKS;                   //number of blocks along z
		int         *BLOCK_SIZE;                 //dimensions of blocks along z
		int         *BLOCK_ABS_D;                //absolute D voxel coordinates of blocks
		int          N_CHANS;                    //number of channels
		int          N_BYTESxCHAN;               //number of bytes per channel

		//******** OBJECT PRIVATE METHODS *********
        Block(void){}

		//Initializes all object's members given DIR_NAME
		void init() throw (iom::exception);

	    //binarizing-unbinarizing methods
		void binarizeInto(FILE* file) throw (iom::exception);
		void unBinarizeFrom(FILE* file) throw (iom::exception);

		//returns a pointer to the intersection segment (along D) if the given segment (D0,D1-1) intersects current stack, otherwise returns NULL
		//D0 first index of the segment
		//D1 last index of the segment + 1
		Segm_t* Intersects(int D0, int D1);

		//******** FRIEND CLASS DECLARATION *********
		//BlockVolume can access Block private members and methods
		friend class BlockVolume;
	
	public:
		//CONSTRUCTORS
		Block(BlockVolume* _CONTAINER, int _ROW_INDEX, int _COL_INDEX, const char* _DIR_NAME) throw (iom::exception);				// build from scratch
		Block(BlockVolume* _CONTAINER, int _ROW_INDEX, int _COL_INDEX, FILE* bin_file) throw (iom::exception);						// build from mdata.bin
		Block(BlockVolume* _CONTAINER, int _ROW_INDEX, int _COL_INDEX, TiXmlElement* stack_node, int z_end) throw (iom::exception);	// build from XML
		~Block(void);

		//GET methods
		int  getN_BLOCKS()		{return N_BLOCKS;}
		int  getN_CHANS()		{return N_CHANS;}
		int  getN_BYTESxCHAN()	{return N_BYTESxCHAN;}

		int  *getBLOCK_SIZE()   {return BLOCK_SIZE;}
		int  *getBLOCK_ABS_D()  {return BLOCK_ABS_D;}
		
        void *getCONTAINER()    {return CONTAINER;}

		//LOAD and RELEASE methods
        iom::real_t* loadImageStack(int first_file=-1, int last_file=-1) throw (iom::exception);
		void releaseImageStack();

		//XML methods
		TiXmlElement* getXML();
		void loadXML(
			TiXmlElement *stack_node,
			int z_end)					// 2014-09-05. Alessandro. @ADDED 'z_end' parameter to support sparse data feature
										//			   Here 'z_end' identifies the range [0, z_end) that slices can span
		throw (iom::exception);
};

#endif //_BLOCK_H
