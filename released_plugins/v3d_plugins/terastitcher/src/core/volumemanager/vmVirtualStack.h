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
* 2015-02-28. Giulio.     @ADDED fields N_CHANS and BYTESxCHAN
* 2015-01-17. Alessandro. @ADDED support for all-in-one-folder data (import from xml only).
* 2014-09-10. Alessandro. @ADDED 'isEmpty(z0,z1)' method.
* 2014-09-05. Alessandro. @ADDED 'z_end' parameter in 'loadXML()' method to support sparse data feature.
* 2014-09-03. Alessandro. @ADDED 'isEmpty()' and 'isSparse()' get methods.
* 2014-09-01. Alessandro. @ADDED 'z_ranges' attribute to manage sparse data along Z.
* 2014-08-30. Alessandro. @REMOVED 'show()' method (obsolete).
* 2014-08-30. Alessandro. @MOVED 'VirtualStack()' default constructor to .cpp source file.
* 2014-08-29. Alessandro. @ADDED 'isComplete()' concrete virtual method.
* 2014-08-25. Alessandro. @ADDED missing 'throw (iom::iom::exception)' statement in the 'loadImageStack()' method's signature.
*/

#ifndef _VM_VIRTUAL_STACK_H
#define _VM_VIRTUAL_STACK_H

#include <vector>
#include "iomanager.config.h"
#include "tinyxml.h"
#include "volumemanager.config.h"
#include "../stitcher/Displacement.h"

//class Displacement;

class vm::VirtualStack
{
    protected:

		char*  DIR_NAME;							//string containing current stack directory
		char** FILENAMES;							//1D dynamic array of <char>  pointers to images filenames
        int	   HEIGHT, WIDTH, DEPTH;				//VHD (Vertical, Horizontal, Depth) dimensions of current stack
        int	   ROW_INDEX,  COL_INDEX;				//row and col index relative to stack matrix
		int    N_CHANS;                             //number of channels
		int    N_BYTESxCHAN;                        //number of bytes per channel
        bool   stitchable;							//true if current Stack is stitchable with adjacent ones
        int    ABS_V, ABS_H, ABS_D;					//absolute VHD voxel coordinates of current stack
        iom::real_t* STACKED_IMAGE;					//pointer to 1-D array of REAL_T that stores Stack image data
        std::vector<Displacement*> NORTH;			//vector of displacements along D direction between this and northern Stack
        std::vector<Displacement*> EAST;			//vector of displacements along D direction between this and eastern  Stack
        std::vector<Displacement*> SOUTH;			//vector of displacements along D direction between this and southern Stack
        std::vector<Displacement*> WEST;			//vector of displacements along D direction between this and western  Stack

        // 2014-09-01. Alessandro. @ADDED support for sparse data.
        std::vector< vm::interval<int> > z_ranges;	//vector of Z-ranges containing valid data (each range is of type [start, end) )
													//*** WARNING ***: if vector is empty, the stack is assumed as empty

		// 2015-01-17. Alessandro. @ADDED support for all-in-one-folder data (import from xml only).
		// Each stack can be associated with a regular expression to filter image filenames. 
		std::string img_regex;
		virtual std::string readImgRegex(TiXmlElement *stack_node) throw (iom::exception);
		virtual void        writeImgRegex(TiXmlElement *stack_node) throw (iom::exception);

    public:

        VirtualStack();
        ~VirtualStack(void){}


		//GET methods
		char* getDIR_NAME()			{return DIR_NAME;}
		char** getFILENAMES()		{return FILENAMES;}		
		int getROW_INDEX()			{return ROW_INDEX;}
		int getCOL_INDEX()			{return COL_INDEX;}
		int getHEIGHT()				{return HEIGHT;}//Onofri: type changed
		int getWIDTH()				{return WIDTH;}	//Onofri: type changed
		int getDEPTH()				{return DEPTH;}	//Onofri: type changed
		int getN_CHANS()			{return N_CHANS;}
		int getN_BYTESxCHAN()		{return N_BYTESxCHAN;}
		int getABS_V()				{return ABS_V;}
		int getABS_H()				{return ABS_H;}
		int getABS_D()				{return ABS_D;}
		int getABS(int direction) throw (iom::exception)
		{
			#if VM_VERBOSE > 4
			printf("........in Stack[%d,%d]::getABS(direction = %d)\n",ROW_INDEX, COL_INDEX, direction);
			#endif

			if (direction == dir_vertical)
				return getABS_V();
			else if(direction == dir_horizontal)
				return getABS_H();
			else if(direction == dir_depth)
				return getABS_D();
			else
				throw iom::exception("in Stack::setABS(int _ABS, int direction): wrong direction inserted");
		}

		iom::real_t* getSTACKED_IMAGE()	{return STACKED_IMAGE;}
		virtual void *getCONTAINER() =0;

		std::vector<Displacement*>& getNORTH(){return NORTH;}
		std::vector<Displacement*>& getEAST(){return  EAST;}
		std::vector<Displacement*>& getSOUTH(){return SOUTH;}
		std::vector<Displacement*>& getWEST(){return  WEST;}

        bool isStitchable(){return this->stitchable;}

		// 2014-09-03. Alessandro. @ADDED 'isEmpty()' and 'isSparse()' get methods
		bool isEmpty(){return z_ranges.empty();}
		bool isSparse(){return !isEmpty() && !isComplete(0, DEPTH-1);}

		Displacement* getDisplacement(VirtualStack* neighbour) throw (iom::exception)
		{
			#if VM_VERBOSE > 4
			printf("........in Stack[%d,%d]::getDisplacement(Stack* neighbour[%d,%d])\n",ROW_INDEX, COL_INDEX, neighbour->ROW_INDEX, neighbour->COL_INDEX);
			#endif

			if(neighbour == NULL)
				throw iom::exception("...in Stack::getDisplacement(Stack* neighbour = NULL): invalid neighbour stack");
			else if(neighbour->getROW_INDEX() == (ROW_INDEX -1) && neighbour->getCOL_INDEX() == COL_INDEX)
				if(NORTH.size() == 1) return NORTH[0];
				else throw iom::exception("...in Stack::getDisplacement(Stack* neighbour): stack MUST contain one displacement only at NORTH");
			else if(neighbour->getROW_INDEX() == ROW_INDEX    && neighbour->getCOL_INDEX() == COL_INDEX -1)
				if(WEST.size() == 1) return WEST[0];
				else throw iom::exception("...in Stack::getDisplacement(Stack* neighbour): stack MUST contain one displacement only at WEST");
			else if(neighbour->getROW_INDEX() == ROW_INDEX +1 && neighbour->getCOL_INDEX() == COL_INDEX)
				if(SOUTH.size() == 1) return SOUTH[0];
				else throw iom::exception("...in Stack::getDisplacement(Stack* neighbour): stack MUST contain one displacement only at SOUTH");
			else if(neighbour->getROW_INDEX() == ROW_INDEX    && neighbour->getCOL_INDEX() == COL_INDEX +1)
				if(EAST.size() == 1) return EAST[0];
				else throw iom::exception("...in Stack::getDisplacement(Stack* neighbour): stack MUST contain one displacement only at EAST");
			else
				throw iom::exception("...in Stack::getDisplacement(Stack* neighbour): neighbour is not a neighbour!!!");
		}

		//SET methods
		void setROW_INDEX(int _ROW_INDEX){ROW_INDEX = _ROW_INDEX;}
		void setCOL_INDEX(int _COL_INDEX){COL_INDEX = _COL_INDEX;}
		void setABS_V    (int _ABS_V)    {ABS_V     = _ABS_V;    }
		void setABS_H    (int _ABS_H)    {ABS_H     = _ABS_H;    }
		void setABS_D    (int _ABS_D)    {ABS_D     = _ABS_D;    }
		void setABS		 (int _ABS, int direction)  throw (iom::exception)
		{
			#if VM_VERBOSE > 4
			printf("........in Stack[%d,%d]::setABS(_ABS = %d, direction = %d)\n",ROW_INDEX, COL_INDEX, _ABS, direction);
			#endif

			if	   (direction == dir_vertical)
				setABS_V(_ABS);
			else if(direction == dir_horizontal)
				setABS_H(_ABS);
			else if(direction == dir_depth)
				setABS_D(_ABS);
			else
				throw iom::exception("in Stack::setABS(int _ABS, int direction): wrong direction inserted");
		}
		void setStitchable(bool _stitchable){this->stitchable = _stitchable;}

		//LOAD and RELEASE methods
        virtual iom::real_t* loadImageStack(int first_file=-1, int last_file=-1) throw (iom::exception) = 0;
		virtual void releaseImageStack()= 0;

        // return true if the given range [z0,z1] does not contain missing slices/blocks
        virtual bool isComplete(int z0, int z1);

		// return true if the given range [z0,z1] does not contain any slice/block
		virtual bool isEmpty(int z0, int z1);

		//XML methods
		virtual TiXmlElement* getXML()= 0;
		virtual void loadXML(
			TiXmlElement *stack_node,
			int z_end)					// 2014-09-05. Alessandro. @ADDED 'z_end' parameter to support sparse data feature
										//			   Here 'z_end' identifies the range [0, z_end) that slices can span
		throw (iom::exception)= 0;
};

#endif
