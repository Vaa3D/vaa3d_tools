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
* 2014-09-05. Alessandro. @ADDED 'z_end' parameter in 'loadXML()' method to support sparse data feature.
* 2014-09-01. Alessandro. @ADDED 'compute_z_ranges()' method to compute z-ranges on sparse data given the complete set of z-coordinates.
*/

#ifndef _VM_STACK_H_
#define _VM_STACK_H_

#include "volumemanager.config.h"
#include "iomanager.config.h"
#include "tinyxml.h"
#include "vmVirtualStack.h" 

using namespace volumemanager;

class StackedVolume;
class Displacement;
class Stack : public VirtualStack
{
	private:

		//*********** OBJECT ATTRIBUTES ***********
		StackedVolume*	CONTAINER;				//pointer to <StackedVolume> object that contains the current object
		
		//******** OBJECT PRIVATE METHODS *********
		Stack(void){}

		//Initializes all object's members
        void init() throw (iom::exception);
				
		//binarizing-unbinarizing methods
		void binarizeInto(FILE* file) throw (iom::exception);
		void unBinarizeFrom(FILE* file) throw (iom::exception);

		// compute 'z_ranges'
		void 
			compute_z_ranges(
			std::set<std::string> const * z_coords = 0)		// set of z-coordinates where at least one slice (of a certain stack) is available
		throw (iom::exception);								// if null, 'z_ranges' will be compute based on 'FILENAMES' vector

		//******** FRIEND CLASS DECLARATION *********
		//StackedVolume can access Stack private members and methods
		friend class StackedVolume;

	public:

		//CONSTRUCTORS
		Stack(StackedVolume* _CONTAINER, int _ROW_INDEX, int _COL_INDEX, const char* _DIR_NAME) throw (iom::exception);
        Stack(StackedVolume* _CONTAINER, int _ROW_INDEX, int _COL_INDEX, FILE* bin_file) throw (iom::exception);
		~Stack(void);

		//GET methods
        void *getCONTAINER() {return CONTAINER;}

		//LOAD and RELEASE methods
		iom::real_t* loadImageStack(int first_file=-1, int last_file=-1) throw (iom::exception);
		void releaseImageStack();

		//XML methods
		TiXmlElement* getXML();
		void loadXML(
			TiXmlElement *stack_node, 
			int z_end)					// 2014-09-05. Alessandro. @ADDED 'z_end' parameter to support sparse data feature
										//						   Here 'z_end' identifies the range [0, z_end) that slices can span
		throw (iom::exception);
};

#endif /* STACK_H_ */

