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
* 2014-09-10. Alessandro. @ADDED 'isEmpty(z0,z1)' method.
* 2014-08-30. Alessandro. @REMOVED 'show()' method (obsolete).
* 2014-08-30. Alessandro. @ADDED 'VirtualStack()' default constructor definition with member initialization.
* 2014-08-29. Alessandro. @ADDED 'isComplete()' implementation.
* 2014-08-25. Alessandro. @ADDED SPARSE_DATA parameter definition. Default is false.
*/

#include "vmVirtualStack.h"

VirtualStack::VirtualStack()
{
    FILENAMES = 0;
    HEIGHT = WIDTH = DEPTH = -1;
    ROW_INDEX = COL_INDEX = -1;
    DIR_NAME = 0;
    stitchable = false;
    ABS_V = ABS_H = ABS_D = -1;
    STACKED_IMAGE = 0;
}

// return true if the given range [z0,z1] does not contain missing slices/blocks
bool VirtualStack::isComplete(int z0, int z1)
{
	// precondition 1 (unchecked)
	// z_ranges should contain consecutive but not contiguous intervals [a_1,b_1), [a_2,b_2), ... such that a_n > b_(n-1)
	
	// precondition 2 and 3: nonempty tile, and valid range.
	if(isEmpty() || z0 > z1)
		return false;

	// search for one z-range that contains [z0,z1]
	bool is_contained = false;
	for(int k=0; k<z_ranges.size() && !is_contained; k++)
		is_contained = z0 >= z_ranges[k].start && z1 < z_ranges[k].end;

	return is_contained;
}

// return true if the given range [z0,z1] does not contain any slice/block
bool VirtualStack::isEmpty(int z0, int z1)
{
	// precondition 1 (unchecked)
	// z_ranges should contain consecutive but not contiguous intervals [a_1,b_1), [a_2,b_2), ... such that a_n > b_(n-1)
	
	// precondition 2 and 3: nonempty tile, and valid range.
	if(isEmpty() || z0 > z1)
		return true;

	// search for one z-range that intersects with [z0,z1]
	bool intersects = false;
	for(int k=0; k<z_ranges.size() && !intersects; k++)
		intersects = z1 >= z_ranges[k].start && z_ranges[k].end > z0;

	return !intersects;
}
