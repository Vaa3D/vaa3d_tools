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

#ifndef _VM_VIRTUAL_VOLUME_H
#define _VM_VIRTUAL_VOLUME_H

//# define HALVE_BY_MEAN 1
//# define HALVE_BY_MAX  2

#include <string>

#include "VM_config.h"
#include "MyException.h"
#include "IOManager.h"


//FORWARD-DECLARATIONS
//struct VHD_triple;
//struct interval_t;
//struct ref_sys;
class VirtualStack;
class Displacement;

//******* ABSTRACT TYPES DEFINITIONS *******

enum axis {vertical=1, inv_vertical=-1, horizontal=2, inv_horizontal=-2, depth=3, inv_depth=-3, axis_invalid=0};

struct VHD_triple{int V, H, D;};

struct interval_t
{
	int start, end;
        interval_t(void) :				   start(-1),	  end(-1)  {}
        interval_t(int _start, int _end) : start(_start), end(_end){}
};

struct ref_sys 
{
	axis first, second, third; 
	ref_sys(axis _first, axis _second, axis _third) : first(_first), second(_second), third(_third){}
	ref_sys(): first(axis_invalid), second(axis_invalid), third(axis_invalid){}
};


const char* axis_to_str(axis ax);
//{
//    if(ax==axis_invalid)         return "axis_invalid";
//    else if(ax==vertical)        return "vertical";
//    else if(ax==inv_vertical)    return "inv_vertical";
//    else if(ax==horizontal)      return "horizontal";
//    else if(ax==inv_horizontal)  return "inv_horizontal";
//    else if(ax==depth)           return "depth";
//    else if(ax==inv_depth)       return "inv_depth";
//    else                         return "unknown";
//}

class volumemanager::VirtualVolume {

protected:
		//******OBJECT ATTRIBUTES******
		char* stacks_dir;					//C-string that contains the directory path of stacks matrix
		float  VXL_V, VXL_H, VXL_D;			//[microns]: voxel dimensions (in microns) along V(Vertical), H(horizontal) and D(Depth) axes
		float  ORG_V, ORG_H, ORG_D;			//[millimeters]: origin spatial coordinates (in millimeters) along VHD axes
		float  MEC_V, MEC_H;				//[microns]: mechanical displacements of the microscope between two adjacent stacks
		uint16 N_ROWS, N_COLS, N_SLICES;	//dimensions (in stacks) of stacks matrix along VH axes
		ref_sys reference_system;


		//initialization methods
		virtual void init() throw (MyException)=0;
		virtual void applyReferenceSystem(ref_sys reference_system, float VXL_1, float VXL_2, float VXL_3) throw (MyException)=0;

		//binary metadata load/save methods
		virtual void saveBinaryMetadata(char *metadata_filepath) throw (MyException)=0;
		virtual void loadBinaryMetadata(char *metadata_filepath) throw (MyException)=0;

		//rotates stacks matrix around D axis (accepted values are theta=0,90,180,270)
		virtual void rotate(int theta)=0;

		//mirrors stacks matrix along mrr_axis (accepted values are mrr_axis=1,2,3)
		virtual void mirror(axis mrr_axis)=0;

		//extract spatial coordinates (in millimeters) of given Stack object reading directory and filenames as spatial coordinates
		void extractCoordinates(VirtualStack* stk, int z, int* crd_1, int* crd_2, int* crd_3);

public:
		//CONSTRUCTORS-DECONSTRUCTOR
		VirtualVolume(float VXL_1=0, float VXL_2=0, float VXL_3=0) throw (MyException)
		{
			//this->root_dir = new char[strlen(_root_dir)+1];
			//strcpy(this->root_dir, _root_dir);

			stacks_dir = (char *) 0;

			reference_system.first = reference_system.second = reference_system.third = axis_invalid;

			VXL_V = VXL_1;
			VXL_H = VXL_2;
			VXL_D = VXL_3;

			ORG_V = ORG_H = ORG_D = (float) 0.0;
			MEC_V = MEC_H = 0;

			N_ROWS = N_COLS = N_SLICES = 0;
		}

		virtual ~VirtualVolume() { 
			//if(root_dir)
			//	delete[] root_dir;
		}


		// ******GET METHODS******
		float	 getORG_V();
		float	 getORG_H();
		float	 getORG_D();
		float	 getABS_V(int ABS_PIXEL_V);
		float	 getABS_H(int ABS_PIXEL_H);
		float	 getABS_D(int ABS_PIXEL_D);
		float	 getVXL_V();
		float	 getVXL_H();
		float	 getVXL_D();
		float	 getMEC_V();
		float	 getMEC_H();
		virtual int		 getStacksHeight() = 0;
		virtual int		 getStacksWidth() = 0;
		int		 getN_ROWS();
		int		 getN_COLS();
		int		 getN_SLICES();
		virtual VirtualStack*** getSTACKS() = 0;
		char*    getSTACKS_DIR(){return this->stacks_dir;}
		int		 getOVERLAP_V();
		int		 getOVERLAP_H();
		int		 getDEFAULT_DISPLACEMENT_V();
		int		 getDEFAULT_DISPLACEMENT_H();
		virtual int		 getDEFAULT_DISPLACEMENT_D();
		ref_sys getREF_SYS(){return reference_system;}
		
		//print all informations contained in this data structure
		virtual void print() = 0;

		//loads/saves metadata from/in the given xml filename
		virtual void loadXML(const char *xml_filename) = 0;
		virtual void initFromXML(const char *xml_filename) = 0;
        virtual void saveXML(const char *xml_filename=0, const char *xml_filepath=0) throw (MyException) = 0;

		//inserts the given displacement in the given stacks
		void insertDisplacement(VirtualStack *stk_A, VirtualStack *stk_B, Displacement *displacement) throw (MyException);

        /**********************************************************************************
        * UTILITY methods
        ***********************************************************************************/

        //counts the total number of displacements and the number of displacements per pair of adjacent stacks
        virtual void countDisplacements(int& total, float& per_stack_pair) = 0;

        //counts the number of single-direction displacements having a reliability measure above the given threshold
        virtual void countReliableSingleDirectionDisplacements(float threshold, int& total, int& reliable) = 0;

        //counts the number of stitchable stacks given the reliability threshold
        virtual int countStitchableStacks(float threshold) = 0;

		//returns true if file exists at the given filepath
		static bool fileExists(const char *filepath)  throw (MyException);

		//extract absolute path from file path(i.e. "C:/Users/Alex/Desktop/" from "C:/Users/Alex/Desktop/text.xml")
		static std::string extractPathFromFilePath(const char* file_path);

};

#endif