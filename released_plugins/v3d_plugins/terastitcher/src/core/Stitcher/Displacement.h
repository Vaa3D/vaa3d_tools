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

#ifndef DISPLACEMENT_H
#define DISPLACEMENT_H

#include "tinyxml.h"
#include "MyException.h"
#include <vector>

enum direction {dir_vertical = 0, dir_horizontal = 1, dir_depth = 2, invalid = -1};

class Displacement
{
	protected:

		int TYPE;					//type of displacement
		int VHD_def_coords[3];      //default VHD displacements as offsets of the second stack with respect to the first one

	public:

		Displacement(void);
		virtual ~Displacement(void){};


		/*** ABSTRACT METHODS that derived classes must implement ***/

		//evaluates displacement reliability possibly along the given direction. The result(s) should be stored
		//in one or more object members, so that they have to be computed once and then accessed by GET methods
		virtual float			 evalReliability(direction _direction=invalid)	throw (MyException) = 0;

		//returns the reliability possibly along the given direction. An exception is thrown if the reliability
		//index(es) are not computed yet. Values are in [0,1] where 0 = totally unreliable, 1 = reliable
		virtual	float			 getReliability(direction _direction=invalid)	throw (MyException) = 0;

		//returns the displacement along the given direction
		virtual int				 getDisplacement(direction _direction)			throw (MyException) = 0;

		//sets to default values the displacements with a reliability factor above the given threshold
		virtual void			 threshold(float rel_threshold)					throw (MyException) = 0;

		//returns the displacement mirrored along the given direction.
		virtual Displacement*	 getMirrored(direction _direction)				throw (MyException) = 0;

		//combines the parameters of the current and the given displacement so that after this operation
		//the two displacements are more reliable (and are EQUAL).
		virtual void			 combine(Displacement& displ)					throw (MyException) = 0;

		//XML methods: convert/load displacement object into/from XML schema
		virtual TiXmlElement*	 getXML()										throw (MyException) = 0;
		virtual void			 loadXML(TiXmlElement *displ_node)				throw (MyException) = 0;

		/*** PUBLIC METHODS that derived classes inherit and can override ***/
		
		/*************************************************************************************************************
		* Set methods
		**************************************************************************************************************/
		void setDefaultV(int V){VHD_def_coords[0] = V;}
		void setDefaultH(int H){VHD_def_coords[1] = H;}
		void setDefaultD(int D){VHD_def_coords[2] = D;}
		


		//*** CLASS methods ***
		
		//instances the specified displacement object from an XML element
		static  Displacement*	 getDisplacementFromXML(TiXmlElement *displ_node) throw (MyException);

		//instances a displacement which is a deep copy of the given displacement
		static  Displacement*	 instance(Displacement* displacement)  throw (MyException);

		/*************************************************************************************************************
		* The given vector of redundant displacements along D is projected into the displacement which embeds the most
		* reliable parameters. After this operation, the given vector will contain only the projected displacement.
		**************************************************************************************************************/
		static void projectDisplacements(std::vector<Displacement*> &displacements)				   throw (MyException);

};

#endif /*DISPLACEMENT_H*/

