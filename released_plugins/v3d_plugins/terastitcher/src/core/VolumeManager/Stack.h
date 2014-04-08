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

#ifndef STACK_H_
#define STACK_H_

#include "VM_config.h"
#include "IOManager.h"
#include "tinyxml.h"

using namespace volumemanager;

class StackedVolume;
class Displacement;
class Stack
{
	private:

		//*********** OBJECT ATTRIBUTES ***********
		StackedVolume*	CONTAINER;				//pointer to <StackedVolume> object that contains the current object
		char**			FILENAMES;				//1D dynamic array of <char>  pointers to images filenames
		int				HEIGHT, WIDTH, DEPTH;	//VHD (Vertical, Horizontal, Depth) dimensions of current stack
		int				ROW_INDEX,  COL_INDEX;	//row and col index relative to stack matrix
		char*			DIR_NAME;				//string containing current stack directory
		bool			stitchable;				//true if current Stack is stitchable with adjacent ones		
		int ABS_V, ABS_H, ABS_D;				//absolute VHD voxel coordinates of current stack
		real_t* STACKED_IMAGE;					//pointer to 1-D array of REAL_T that stores Stack image data
		std::vector<Displacement*> NORTH;		//vector of displacements along D direction between this and northern Stack
		std::vector<Displacement*> EAST;		//vector of displacements along D direction between this and eastern  Stack
		std::vector<Displacement*> SOUTH;		//vector of displacements along D direction between this and southern Stack
		std::vector<Displacement*> WEST;		//vector of displacements along D direction between this and western  Stack
	
		
		//******** OBJECT PRIVATE METHODS *********
        Stack(void){}

		//Initializes all object's members
        void init() throw (MyException);
				
		//binarizing-unbinarizing methods
        void binarizeInto(FILE* file) throw (MyException);
		void unBinarizeFrom(FILE* file) throw (MyException);

		//******** FRIEND CLASS DECLARATION *********
		//StackedVolume can access Stack private members and methods
		friend class StackedVolume;

	public:

		//CONSTRUCTORS
        Stack(StackedVolume* _CONTAINER, int _ROW_INDEX, int _COL_INDEX, const char* _DIR_NAME) throw (MyException);
        Stack(StackedVolume* _CONTAINER, int _ROW_INDEX, int _COL_INDEX, FILE* bin_file) throw (MyException);
		~Stack(void);

		//GET methods
		char* getDIR_NAME()			{return DIR_NAME;}
		int getROW_INDEX()			{return ROW_INDEX;}
		int getCOL_INDEX()			{return COL_INDEX;}
		int getHEIGHT()				{return HEIGHT;}
		int getWIDTH()				{return WIDTH;}
		int getDEPTH()				{return DEPTH;}
		char** getFILENAMES()		{return FILENAMES;}		
		int getABS_V()				{return ABS_V;}
		int getABS_H()				{return ABS_H;}
		int getABS_D()				{return ABS_D;}
		int getABS(int direction) throw (MyException);
		real_t* getSTACKED_IMAGE()	{return STACKED_IMAGE;}
		void *getCONTAINER();
		std::vector<Displacement*>& getNORTH(){return NORTH;}
		std::vector<Displacement*>& getEAST(){return  EAST;}
		std::vector<Displacement*>& getSOUTH(){return SOUTH;}
		std::vector<Displacement*>& getWEST(){return  WEST;}
		bool isStitchable(){return this->stitchable;}
		Displacement* getDisplacement(Stack* neighbour) throw (MyException);

		//SET methods
		void setROW_INDEX(int _ROW_INDEX){ROW_INDEX = _ROW_INDEX;}
		void setCOL_INDEX(int _COL_INDEX){COL_INDEX = _COL_INDEX;}
		void setABS_V    (int _ABS_V)    {ABS_V     = _ABS_V;    }
		void setABS_H    (int _ABS_H)    {ABS_H     = _ABS_H;    }
		void setABS_D    (int _ABS_D)    {ABS_D     = _ABS_D;    }
		void setABS		 (int _ABS, int direction)  throw (MyException);
		void setStitchable(bool _stitchable){this->stitchable = _stitchable;}

		//LOAD and RELEASE methods
        real_t* loadImageStack(int first_file=-1, int last_file=-1) throw (MyException);
		void releaseImageStack();

		//XML methods
		TiXmlElement* getXML();
		void		  loadXML(TiXmlElement *stack_node) throw (MyException);

		//PRINT and SHOW methods
		void print();
		void show(int D_index, int window_HEIGHT=0, int window_WIDTH=0);
};

#endif /* STACK_H_ */

