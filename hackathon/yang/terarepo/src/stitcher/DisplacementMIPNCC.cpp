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

#include "DisplacementMIPNCC.h"
#include "S_config.h"
#include <limits>

DisplacementMIPNCC::DisplacementMIPNCC(void) : Displacement()
{
	#if S_VERBOSE>5
	printf("..........in DisplacementMIPNCC::DisplacementMIPNCC(void)\n");
	#endif

	VHD_coords[0] = VHD_coords[1] = VHD_coords [2] = std::numeric_limits<int>::max();
	NCC_maxs[0] = NCC_maxs[1] = NCC_maxs[2] = -1.0F;
	NCC_widths[0] = NCC_widths[1] = NCC_widths[2] = -1;
	rel_factors[0] = rel_factors[1] = rel_factors[2] = -1.0F;
	delays[0] = delays[1] = delays[2]= -1;
    wRangeThrs[0] = wRangeThrs[1] = wRangeThrs[2] = -1;
    invWidths[0] = invWidths[1] = invWidths[2] = -1;
	TYPE = S_MIP_NCC_DISPL_TYPE;
}

DisplacementMIPNCC::DisplacementMIPNCC(NCC_descr_t &mip_ncc_descr) : Displacement()
{
	#if S_VERBOSE>5
	printf("..........in DisplacementMIPNCC::DisplacementMIPNCC(NCC_descr_t &mip_ncc_descr)\n");
	#endif

	VHD_coords[0] = mip_ncc_descr.coord[0];
	VHD_coords[1] = mip_ncc_descr.coord[1];
	VHD_coords[2] = mip_ncc_descr.coord[2];
	NCC_maxs[0] = mip_ncc_descr.NCC_maxs[0];
	NCC_maxs[1] = mip_ncc_descr.NCC_maxs[1];
	NCC_maxs[2] = mip_ncc_descr.NCC_maxs[2];
	NCC_widths[0] = mip_ncc_descr.NCC_widths[0];
	NCC_widths[1] = mip_ncc_descr.NCC_widths[1];
    NCC_widths[2] = mip_ncc_descr.NCC_widths[2];
	rel_factors[0] = rel_factors[1] = rel_factors[2] = -1.0F;
    delays[0] = delays[1] = delays[2]= -1;
    wRangeThrs[0] = wRangeThrs[1] = wRangeThrs[2] = -1;
    invWidths[0] = invWidths[1] = invWidths[2] = -1;
	TYPE = S_MIP_NCC_DISPL_TYPE;
}


DisplacementMIPNCC::DisplacementMIPNCC(int Vnominal, int Hnominal, int Dnominal) : Displacement()
{
    #if S_VERBOSE>5
    printf("..........in DisplacementMIPNCC::DisplacementMIPNCC(int Vnominal, int Hnominal, int Dnominal)\n");
    #endif

    NCC_maxs[0] = NCC_maxs[1] = NCC_maxs[2] = 0.0F;
    NCC_widths[0] = NCC_widths[1] = NCC_widths[2] = S_NCC_WIDTH_MAX;
	delays[0] = delays[1] = delays[2]= -1;
    wRangeThrs[0] = wRangeThrs[1] = wRangeThrs[2] = S_NCC_WIDTH_MAX - 1;
    invWidths[0] = invWidths[1] = invWidths[2] = S_NCC_WIDTH_MAX;
    rel_factors[0] = rel_factors[1] = rel_factors[2] = 0.0F;
    VHD_def_coords[0] = VHD_coords[0] = Vnominal;
    VHD_def_coords[1] = VHD_coords[1] = Hnominal;
    VHD_def_coords[2] = VHD_coords[2] = Dnominal;
    TYPE = S_MIP_NCC_DISPL_TYPE;
}

DisplacementMIPNCC::DisplacementMIPNCC(const DisplacementMIPNCC &ex_instance)
{
    #if S_VERBOSE>5
    printf("..........in DisplacementMIPNCC::DisplacementMIPNCC(const DisplacementMIPNCC &ex_instance)\n");
    #endif

    TYPE = ex_instance.TYPE;
    for(int i=0; i<3; i++)
    {
        VHD_coords[i] = ex_instance.VHD_coords[i];
        VHD_def_coords[i] = ex_instance.VHD_def_coords[i];
        NCC_maxs[i] = ex_instance.NCC_maxs[i];
        NCC_widths[i] = ex_instance.NCC_widths[i];
        rel_factors[i] = ex_instance.rel_factors[i];
		delays[i] = ex_instance.delays[i];
        wRangeThrs[i] = ex_instance.wRangeThrs[i];
        invWidths[i] = ex_instance.invWidths[i];
    }
}

DisplacementMIPNCC::DisplacementMIPNCC(TiXmlElement *displ_node) : Displacement()
{
    #if S_VERBOSE>5
    printf("..........in DisplacementMIPNCC::DisplacementMIPNCC(TiXmlElement *displ_node)\n");
    #endif

    TYPE = S_MIP_NCC_DISPL_TYPE;
    this->loadXML(displ_node);
}

//evaluates displacement reliability possibly along the given direction. The result(s) should be stored
//in one or more object members, so that they have to be computed once and then accessed by GET methods
float DisplacementMIPNCC::evalReliability(direction _direction) throw (iom::exception)
{
	#if S_VERBOSE>4
	printf("........in DisplacementMIPNCC::evalReliability(direction _direction = %d)\n", _direction);
	#endif

	if(_direction == 0 || _direction == 1 || _direction == 2)
	{
        float NCC_width_normalized = (100.0F - (NCC_widths[_direction] * 100.0F / invWidths[_direction]))/100.0F;
		rel_factors[_direction] = (float) sqrt( S_NCC_WIDTH_WEIGHT*NCC_width_normalized*NCC_width_normalized + S_NCC_PEAK_WEIGHT*NCC_maxs[_direction]*NCC_maxs[_direction]);
	}
	else if(_direction == -1)
		throw iom::exception("in DisplacementMIPNCC::evalReliability(void): feature not yet supported");
	else
		throw iom::exception("in DisplacementMIPNCC::evalReliability(...): wrong direction value");

	return rel_factors[_direction];
}

//returns the reliability possibly along the given direction. An exception is thrown if the reliability
//index(es) are not computed yet.
float DisplacementMIPNCC::getReliability(direction _direction)  throw (iom::exception)
{
	#if S_VERBOSE>4
	printf("........in DisplacementMIPNCC::getReliability(direction _direction = %d)\n", _direction);
	#endif

	if( (_direction == 0 || _direction == 1 || _direction == 2) && rel_factors[_direction] == -1.0F)
		throw iom::exception("in DisplacementMIPNCC::evalReliability(direction _direction): reliability factor not yet computed");
	else if(_direction == -1)
		throw iom::exception("in DisplacementMIPNCC::evalReliability(void): feature not yet supported");
	else if( !(_direction == 0 || _direction == 1 || _direction == 2) )
	{
		char errMsg[1000];
		sprintf(errMsg, "in DisplacementMIPNCC::evalReliability(...): wrong direction value ( = %d )", _direction);
		throw iom::exception(errMsg);
	}

	return rel_factors[_direction];
}

//returns the displacement along the given direction
int	DisplacementMIPNCC::getDisplacement(direction _direction) throw (iom::exception)
{
	#if S_VERBOSE>4
	printf("\t\t\t\t\tin DisplacementMIPNCC::getDisplacement(direction _direction = %d)\n", _direction);
	#endif

	if( _direction != 0 && _direction != 1 && _direction != 2)
		throw iom::exception("in DisplacementMIPNCC::getDisplacement(...): wrong direction value");
	else if(VHD_coords[_direction] == std::numeric_limits<int>::max())
		throw iom::exception("in DisplacementMIPNCC::getDisplacement(...): displacement not computed yet");

	return VHD_coords[_direction];
}

//sets to default values the displacements with a reliability factor above the given threshold
void DisplacementMIPNCC::threshold(float rel_threshold) throw (iom::exception)
{
	#if S_VERBOSE>4
	printf("........in DisplacementMIPNCC::threshold(rel_threshold = %.4f)\n", rel_threshold);
	#endif

	evalReliability(dir_vertical);
	evalReliability(dir_horizontal);
	evalReliability(dir_depth);

	for(int i=0; i<3; i++)
		if(rel_factors[i] < rel_threshold)
		{
			VHD_coords[i] = VHD_def_coords[i];
			NCC_maxs[i]	  = 0;
            NCC_widths[i] = invWidths[i];
			evalReliability(direction(i));
		}
}

//returns the displacement mirrored along the given direction.
Displacement* DisplacementMIPNCC::getMirrored(direction _direction) throw (iom::exception)
{
	#if S_VERBOSE>4
	printf("........in DisplacementMIPNCC::getMirrored(direction _direction = %d)\n", _direction);
	#endif

	DisplacementMIPNCC *mirrored = new DisplacementMIPNCC();
	if(_direction == dir_vertical)
	{
		mirrored->VHD_coords[0]		= -VHD_coords[0];
		mirrored->VHD_def_coords[0] = -VHD_def_coords[0];
		mirrored->VHD_coords[1]		= VHD_coords[1];
		mirrored->VHD_def_coords[1] = VHD_def_coords[1];
		mirrored->VHD_coords[2]		= VHD_coords[2];
		mirrored->VHD_def_coords[2] = VHD_def_coords[2];

	}
	else if(_direction == dir_horizontal)
	{
		mirrored->VHD_coords[0]		= VHD_coords[0];
		mirrored->VHD_def_coords[0] = VHD_def_coords[0];
		mirrored->VHD_coords[1]		= -VHD_coords[1];
		mirrored->VHD_def_coords[1] = -VHD_def_coords[1];
		mirrored->VHD_coords[2]		= VHD_coords[2];
		mirrored->VHD_def_coords[2] = VHD_def_coords[2];
	}
	else if(_direction == dir_depth)
	{
		mirrored->VHD_coords[0]		= VHD_coords[0];
		mirrored->VHD_def_coords[0] = VHD_def_coords[0];
		mirrored->VHD_coords[1]		= VHD_coords[1];
		mirrored->VHD_def_coords[1] = VHD_def_coords[1];
		mirrored->VHD_coords[2]		= -VHD_coords[2];
		mirrored->VHD_def_coords[2] = -VHD_def_coords[2];
	}
	else
		throw iom::exception("in DisplacementMIPNCC::getMirrored(...): unsupported or wrong given mirroring direction");

	mirrored->TYPE = TYPE;
	mirrored->NCC_maxs[0] = NCC_maxs[0];
	mirrored->NCC_maxs[1] = NCC_maxs[1];
	mirrored->NCC_maxs[2] = NCC_maxs[2];
	mirrored->NCC_widths[0] = NCC_widths[0];
	mirrored->NCC_widths[1] = NCC_widths[1];
	mirrored->NCC_widths[2] = NCC_widths[2];
	mirrored->rel_factors[0] = rel_factors[0];
	mirrored->rel_factors[1] = rel_factors[1];
	mirrored->rel_factors[2] = rel_factors[2];
    mirrored->wRangeThrs[0] = wRangeThrs[0];
    mirrored->wRangeThrs[1] = wRangeThrs[1];
    mirrored->wRangeThrs[2] = wRangeThrs[2];
    mirrored->invWidths[0] = invWidths[0];
    mirrored->invWidths[1] = invWidths[1];
    mirrored->invWidths[2] = invWidths[2];
	mirrored->delays[0] = delays[0];
	mirrored->delays[1] = delays[1];
	mirrored->delays[2] = delays[2];
	return (Displacement*) mirrored;
}

//combines the parameters of the current and the given displacement so that after this operation
//the two displacements are more reliable (and are EQUAL).
void DisplacementMIPNCC::combine(Displacement& displ) throw (iom::exception)
{
	#if S_VERBOSE>3
	printf("......in DisplacementMIPNCC::combine(Displacement& displ)\n");
	#endif

	Displacement *displ_ptr = &displ;
	DisplacementMIPNCC *displ_MIPNCC = (DisplacementMIPNCC*) displ_ptr;
	for(int k=0; k<3; k++)
	{
		evalReliability(direction(k));
		displ.evalReliability(direction(k));
		if(rel_factors[k] < displ_MIPNCC->rel_factors[k])
		{
			rel_factors   [k]	= displ_MIPNCC->rel_factors   [k];
			NCC_maxs      [k]	= displ_MIPNCC->NCC_maxs      [k];
			NCC_widths    [k]	= displ_MIPNCC->NCC_widths    [k];
			VHD_coords    [k]	= displ_MIPNCC->VHD_coords    [k];
			VHD_def_coords[k]	= displ_MIPNCC->VHD_def_coords[k];
			delays[k]			= displ_MIPNCC->delays[k];
            wRangeThrs[k]		= displ_MIPNCC->wRangeThrs[k];
            invWidths[k]		= displ_MIPNCC->invWidths[k];
		}
		else
		{
			displ_MIPNCC->rel_factors   [k] = rel_factors   [k];
			displ_MIPNCC->NCC_maxs      [k] = NCC_maxs      [k];
			displ_MIPNCC->NCC_widths    [k] = NCC_widths    [k];
			displ_MIPNCC->VHD_coords    [k] = VHD_coords    [k];
			displ_MIPNCC->VHD_def_coords[k] = VHD_def_coords[k];
			displ_MIPNCC->delays[k]			= delays[k];
            displ_MIPNCC->wRangeThrs[k]		= wRangeThrs[k];
            displ_MIPNCC->invWidths[k]		= invWidths[k];
		}
	}
}

//XML methods: convert/load displacement object into/from XML schema
TiXmlElement* DisplacementMIPNCC::getXML() throw (iom::exception)
{
	#if S_VERBOSE>5
	printf("\t\t\t\t\tin DisplacementMIPNCC::getXML()\n");
	#endif

	TiXmlElement * xml_representation = new TiXmlElement("Displacement");
	xml_representation->SetAttribute("TYPE","MIP_NCC");

	for(int i=0; i<3; i++)
	{
		TiXmlElement* displacement;
		if(i==0)
			displacement = new TiXmlElement("V");
		else if(i==1)
			displacement = new TiXmlElement("H");
		else
			displacement = new TiXmlElement("D");
		displacement->SetAttribute("displ",VHD_coords[i]);
		displacement->SetAttribute("default_displ",VHD_def_coords[i]);
		displacement->SetDoubleAttribute("reliability",rel_factors[i]);
		displacement->SetDoubleAttribute("nccPeak",NCC_maxs[i]);
		displacement->SetAttribute("nccWidth",NCC_widths[i]);

		// Alessandro - 31/05/2013 - storing additional MIP-NCC parameters
        displacement->SetAttribute("nccWRangeThr", wRangeThrs[i]);
        displacement->SetAttribute("nccInvWidth", invWidths[i]);
		displacement->SetAttribute("delay", delays[i]);

		xml_representation->LinkEndChild(displacement);
	}

	return xml_representation;
}
void DisplacementMIPNCC::loadXML(TiXmlElement *displ_node) throw (iom::exception)
{
	#if S_VERBOSE>5
	printf("\t\t\t\t\tin DisplacementMIPNCC::loadXML(displ_node)\n");
	#endif

	TiXmlElement *displElem = displ_node->FirstChildElement("V");
	int displ_index=0;
	while(displElem)
	{
		displElem->QueryIntAttribute("displ", &(VHD_coords[displ_index]));
		displElem->QueryIntAttribute("default_displ",&(VHD_def_coords[displ_index]));
		displElem->QueryFloatAttribute("reliability", &(rel_factors[displ_index]));
		displElem->QueryFloatAttribute("nccPeak", &(NCC_maxs[displ_index]));
		displElem->QueryIntAttribute("nccWidth", &(NCC_widths[displ_index]));

		// Alessandro - 31/05/2013 - loading additional MIP-NCC parameters
		if(displElem->Attribute("nccWRangeThr") != 0)
            displElem->QueryIntAttribute("nccWRangeThr", &wRangeThrs[displ_index]);
		else
            wRangeThrs[displ_index] = S_NCC_WIDTH_MAX - 1;
		if(displElem->Attribute("nccInvWidth") != 0)
            displElem->QueryIntAttribute("nccInvWidth", &invWidths[displ_index]);
		else
            invWidths[displ_index] = S_NCC_WIDTH_MAX;
		if(displElem->Attribute("delay") != 0)
			displElem->QueryIntAttribute("delay", &delays[displ_index]);
        else
            delays[displ_index]=-1;

		displElem = displElem->NextSiblingElement();
		displ_index++;
	}
}

