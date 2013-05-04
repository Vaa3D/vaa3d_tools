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
*       Bria, A., et al., (2012) "Stitching Terabyte-sized 3D Images Acquired in Confocal Ultramicroscopy", Proceedings of the 9th IEEE International Symposium on Biomedical Imaging.
*       Bria, A., Iannello, G., "TeraStitcher - A Tool for Fast 3D Automatic Stitching of Teravoxel-sized Microscopy Images", submitted for publication, 2012.
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

#include "Displacement.h"
#include "DisplacementMIPNCC.h"
#include "S_config.h"
#include <limits>

Displacement::Displacement(void)
{
	#if S_VERBOSE>5
	printf("..........in Displacement::Displacement(void)\n");
	#endif

	TYPE = -1;
	VHD_def_coords[0] = VHD_def_coords[1] = VHD_def_coords [2] = std::numeric_limits<int>::max();
}

Displacement* Displacement::getDisplacementFromXML(TiXmlElement *displ_node) throw (MyException)
{
	#if S_VERBOSE>5
	printf("..........in Displacement::getDisplacementFromXML(int displ_type=%d, TiXmlElement *displ_node)\n",displ_type);
	#endif

	const char *displ_type = displ_node->Attribute("TYPE");
	if     (strcmp(displ_type, "MIP_NCC") == 0)
		return (Displacement*)(new DisplacementMIPNCC(displ_node));
	else
	{
		char err_msg[200];
		sprintf(err_msg, "in Displacement::getDisplacementFromXML(....): unsupported displacement type (\"%s\")", displ_type);
		throw MyException(err_msg);
	}
}

//instances a displacement which is a deep copy of the given displacement
Displacement* Displacement::instance(Displacement* displacement)  throw (MyException)
{
	#if S_VERBOSE>5
	printf("..........in Displacement::instance(displacement)\n");
	#endif

	if (displacement->TYPE == S_MIP_NCC_DISPL_TYPE)
	{
		DisplacementMIPNCC* displ_MIPNCC = (DisplacementMIPNCC*)displacement;
		return (Displacement*)(new DisplacementMIPNCC(*displ_MIPNCC));
	}
	else
	{
		char err_msg[200];
		sprintf(err_msg, "in Displacement::instance(displacement): unsupported displacement type (\"%d\")", displacement->TYPE);
		throw MyException(err_msg);
	}
}

/*************************************************************************************************************
* The given vector of redundant displacements along D is projected into the displacement which embeds the most
* reliable parameters. After this operation, the given vector will contain only the projected displacement.
**************************************************************************************************************/
void Displacement::projectDisplacements(std::vector<Displacement*> &displacements) throw (MyException)
{
	#if S_VERBOSE>2
	printf("....in Displacement::projectDisplacements(displacements[size=%d])\n",displacements.size());
	#endif

	if(displacements.size() <= 0)
		throw MyException("in Displacement::projectDisplacements(...): the given vector of displacements is EMPTY. Nothing to project.");

	//performing projection pairwise. At the end of this process, the last
	//displacement will contain the displacement resulting from projection.
	for(int i=0; i<displacements.size()-1; i++)
	{
		//propagating projection pairwise
		displacements[i]->combine(*displacements[i+1]);

		//the current displacement is now useless
		delete displacements[i];
	}
	Displacement* projected_displ = displacements[displacements.size()-1];
	displacements.clear();
	displacements.push_back(projected_displ);
}
