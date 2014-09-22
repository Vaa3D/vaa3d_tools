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
* 2014-09-20. Alessandro. @ADDED overwrite_mdata flag to the XML-based constructor.
* 2014-09-10. Alessandro. @ADDED plugin creation/registration functions to make 'StackedVolume' a volume format plugin.
* 2014-09-05. Alessandro. @ADDED 'normalize_stacks_attributes()' method to normalize stacks attributes (width, height, etc.)
*/

#ifndef _VM_BLOCK_VOLUME_H
#define _VM_BLOCK_VOLUME_H

#include <string>
#include <sstream>
#include "volumemanager.config.h"
#include "iomanager.config.h"
#include "vmVirtualVolume.h" 
#include "vmBlock.h"

//FORWARD-DECLARATIONS
class VirtualStack;
class Block;

class BlockVolume : public volumemanager::VirtualVolume
{
	private:

		// 2014-09-10. Alessandro. @ADDED plugin creation/registration functions to make 'StackedVolume' a volume format plugin.
		static const std::string creator_id1, creator_id2;							
        static VirtualVolume* createFromXML(const char* xml_path, bool ow_mdata) { return new BlockVolume(xml_path, ow_mdata); }
		static VirtualVolume* createFromData(const char* data_path, vm::ref_sys ref, float vxl1, float vxl2, float vxl3, bool ow_mdata) { 
			return new BlockVolume(data_path, ref, vxl1, vxl2, vxl3, ow_mdata); 
		}

		Block ***BLOCKS;			    //2-D array of <Block*>

		//Given the reference system, initializes all object's members using stack's directories hierarchy
        void init() throw (iom::exception);

		void applyReferenceSystem(vm::ref_sys reference_system, float VXL_1, float VXL_2, float VXL_3) throw (iom::exception);

		//binary metadata load/save methods
		void saveBinaryMetadata(char *metadata_filepath) throw (iom::exception);
		void loadBinaryMetadata(char *metadata_filepath) throw (iom::exception);

		//rotate stacks matrix around D vm::axis (accepted values are theta=0,90,180,270)
		void rotate(int theta);

		//mirror stacks matrix along mrr_axis (accepted values are mrr_axis=1,2,3)
		void mirror(vm::axis mrr_axis);

		// iannello returns the number of channels of images composing the volume
		//void initChannels ( ) throw (iom::iom::exception);

		// 2014-09-05. Alessandro. @ADDED 'normalize_stacks_attributes()' method to normalize stacks attributes (width, height, etc.)
		void normalize_stacks_attributes() throw (iom::exception);

	public:

		// 2014-09-10. Alessandro. @ADDED plugin creation/registration functions to make 'StackedVolume' a volume format plugin.
		static const std::string id;		

		//CONSTRUCTORS-DECONSTRUCTOR
		BlockVolume() : VirtualVolume(){}
        BlockVolume(const char* _stacks_dir, vm::ref_sys reference_system, float VXL_1=0, float VXL_2=0, float VXL_3=0, bool overwrite_mdata=false) throw (iom::exception);
        BlockVolume(const char *xml_filepath, bool overwrite_mdata=false) throw (iom::exception);
		~BlockVolume();

		// ******GET METHODS******
		int		 getStacksHeight()   {return BLOCKS[0][0]->getHEIGHT();}
		int		 getStacksWidth()    {return BLOCKS[0][0]->getWIDTH();}
		VirtualStack*** getSTACKS()  {return (VirtualStack***)this->BLOCKS;}

		//loads/saves metadata from/in the given xml filename
		void loadXML(const char *xml_filename);
		void initFromXML(const char *xml_filename);
        void saveXML(const char *xml_filename=0, const char *xml_filepath=0) throw (iom::exception);


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

namespace{																
	const BlockVolume* objectBlockVolume = new BlockVolume();
} 


#endif /* BLOCK_VOLUME_H */
