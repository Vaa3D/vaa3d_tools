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

#ifndef _STACK_H
#define _STACK_H

#include <stdio.h>
#include "IM_config.h"

//FORWARD-DECLARATIONS
struct CvMat;
class  VirtualVolume;
/* modified (iannello)
 * abstract class VirtualVolume has been substituted to derived class StackedVolume
 * since class Stack can be used to manage also volumes stored simply as a sequence of images 
 * in one directory
 */

//TYPE DEFINITIONS
typedef struct {int V0, V1, H0, H1;} Rect_t;

class Stack
{
	private:

		//*********** OBJECT ATTRIBUTES ***********
		VirtualVolume*	CONTAINER;					//pointer to <VirtualVolume> object that contains the current object
		CvMat**			STACKED_IMAGE;				//1-D dinamic array of <CvMat> pointers. Every <CvMat> stores a single 2-D image
		char**			FILENAMES;					//1-D dinamic array of <char>  pointers to images filanames
        iim::uint32		HEIGHT, WIDTH, DEPTH;		//VHD (Vertical, Horizontal, Depth) dimensions of current stack
		int				ROW_INDEX,  COL_INDEX;		//row and col index relative to stack matrix
		int				ABS_V,		ABS_H;			//absolute VH voxel coordinates of current stack
		char*			DIR_NAME;					//string containing current stack directory

		//******** OBJECT PRIVATE METHODS *********
		Stack(void);

		//Initializes all object's members given DIR_NAME
        void init() throw (iim::IOException);

	public:

        Stack(VirtualVolume* _CONTAINER, int _ROW_INDEX, int _COL_INDEX, FILE* bin_file) throw (iim::IOException);
        Stack(VirtualVolume* _CONTAINER, int _ROW_INDEX, int _COL_INDEX, char* _DIR_NAME) throw (iim::IOException);
		~Stack(void);

		//GET methods
		char* getDIR_NAME()			{return DIR_NAME;}
		int getROW_INDEX()			{return ROW_INDEX;}
		int getCOL_INDEX()			{return COL_INDEX;}
        iim::uint32 getHEIGHT()		{return HEIGHT;}
        iim::uint32 getWIDTH()		{return WIDTH;}
        iim::uint32 getDEPTH()		{return DEPTH;}
		int getABS_V()				{return ABS_V;}
		int getABS_H()				{return ABS_H;}
		char** getFILENAMES()		{return FILENAMES;}
		CvMat** getSTACKED_IMAGE()	{return STACKED_IMAGE;}

		//SET methods
		void setROW_INDEX(int _ROW_INDEX){ROW_INDEX = _ROW_INDEX;}
		void setCOL_INDEX(int _COL_INDEX){COL_INDEX = _COL_INDEX;}
		void setABS_V    (int _ABS_V)    {ABS_V     = _ABS_V;    }
		void setABS_H    (int _ABS_H)    {ABS_H     = _ABS_H;    }

		//PRINT method
		void print();

		//binarizing-unbinarizing methods
		void binarizeInto(FILE* file);
        void unBinarizeFrom(FILE* file) throw (iim::IOException);

		//loads/releases images of current stack (from 'first_file' to 'last_file' extremes included, if not specified loads entire stack)
		void loadStack   (int first_file=-1, int last_file=-1);
		void releaseStack(int first_file=-1, int last_file=-1);

		//returns a pointer to the intersection rectangle if the given area intersects current stack, otherwise returns NULL
		Rect_t* Intersects(const Rect_t& area);
};

#endif //_STACK_H
