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

#ifndef BLOCK_VOLUME_H
#define BLOCK_VOLUME_H

#include <string>
#include "VM_config.h"
#include <sstream>
#include "MyException.h"
#include "IOManager.h"
#include "VirtualVolume.h" 
#include "Block.h"
//#include "StackedVolume.h" 

//FORWARD-DECLARATIONS
//struct VHD_triple;
//struct interval_t;
//enum axis {vertical=1, inv_vertical=-1, horizontal=2, inv_horizontal=-2, depth=3, inv_depth=-3, axis_invalid=0};
//struct ref_sys;
class VirtualStack;
class Block;
//class Displacement;
//const char* axis_to_str(axis ax);

class BlockVolume : public volumemanager::VirtualVolume
{
	private:
		Block ***BLOCKS;			    //2-D array of <Block*>

		//Given the reference system, initializes all object's members using stack's directories hierarchy
        void init() throw (MyException);

		void applyReferenceSystem(ref_sys reference_system, float VXL_1, float VXL_2, float VXL_3) throw (MyException);

		//binary metadata load/save methods
		void saveBinaryMetadata(char *metadata_filepath) throw (MyException);
		void loadBinaryMetadata(char *metadata_filepath) throw (MyException);

		//rotate stacks matrix around D axis (accepted values are theta=0,90,180,270)
		void rotate(int theta);

		//mirror stacks matrix along mrr_axis (accepted values are mrr_axis=1,2,3)
		void mirror(axis mrr_axis);

		// iannello returns the number of channels of images composing the volume
		//void initChannels ( ) throw (MyException);

	public:
		//CONSTRUCTORS-DECONSTRUCTOR
        BlockVolume(const char* _stacks_dir, ref_sys reference_system, float VXL_1=0, float VXL_2=0, float VXL_3=0, bool overwrite_mdata=false) throw (MyException);
		BlockVolume(const char *xml_filepath) throw (MyException);
		~BlockVolume();

		// ******GET METHODS******
		int		 getStacksHeight()   {return BLOCKS[0][0]->getHEIGHT();}
		int		 getStacksWidth()    {return BLOCKS[0][0]->getWIDTH();}
		VirtualStack*** getSTACKS()  {return (VirtualStack***)this->BLOCKS;}

		//print all informations contained in this data structure
		void print();

		//loads/saves metadata from/in the given xml filename
		void loadXML(const char *xml_filename);
		void initFromXML(const char *xml_filename);
        void saveXML(const char *xml_filename=0, const char *xml_filepath=0) throw (MyException);


        /**********************************************************************************
        * UTILITY methods
        ***********************************************************************************/

        //counts the total number of displacements and the number of displacements per pair of adjacent stacks
        void countDisplacements(int& total, float& per_stack_pair);

        //counts the number of single-direction displacements having a reliability measure above the given threshold
        void countReliableSingleDirectionDisplacements(float threshold, int& total, int& reliable);

        //counts the number of stitchable stacks given the reliability threshold
        int countStitchableStacks(float threshold);


};


#endif /* BLOCK_VOLUME_H */