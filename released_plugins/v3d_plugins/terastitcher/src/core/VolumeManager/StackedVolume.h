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

#ifndef STACKED_VOLUME_H
#define STACKED_VOLUME_H

#include <string>
#include "VM_config.h"
#include <sstream>
#include "MyException.h"
#include "IOManager.h"
#include "VirtualVolume.h" 
#include <cstdarg>
#include <vector>
#include <sstream>
#include <limits>
#include <cstring>

//FORWARD-DECLARATIONS
//struct VHD_triple;
//struct interval_t;
//enum axis {vertical=1, inv_vertical=-1, horizontal=2, inv_horizontal=-2, depth=3, inv_depth=-3, axis_invalid=0};
//struct ref_sys;
class VirtualStack;
class Stack;
//class Displacement;
//const char* axis_to_str(axis ax);

class StackedVolume : public volumemanager::VirtualVolume
{

	private:

		//******OBJECT ATTRIBUTES******
		//char* stacks_dir;					//C-string that contains the directory path of stacks matrix
		//float  VXL_V, VXL_H, VXL_D;			//[microns]: voxel dimensions (in microns) along V(Vertical), H(horizontal) and D(Depth) axes
		//float  ORG_V, ORG_H, ORG_D;			//[millimeters]: origin spatial coordinates (in millimeters) along VHD axes
		//float  MEC_V, MEC_H;				//[microns]: mechanical displacements of the microscope between two adjacent stacks
		//uint16 N_ROWS, N_COLS, N_SLICES;	//dimensions (in stacks) of stacks matrix along VH axes
		Stack ***STACKS;					//2-D array of <Stack*>	

		//***OBJECT PRIVATE METHODS****
        StackedVolume(void){}

		//initialization methods
		void init() throw (MyException);
		void applyReferenceSystem(ref_sys reference_system, float VXL_1, float VXL_2, float VXL_3) throw (MyException);

		//binary metadata load/save methods
		void saveBinaryMetadata(char *metadata_filepath) throw (MyException);
		void loadBinaryMetadata(char *metadata_filepath) throw (MyException);

		//rotates stacks matrix around D axis (accepted values are theta=0,90,180,270)
		void rotate(int theta);

		//mirrors stacks matrix along mrr_axis (accepted values are mrr_axis=1,2,3)
		void mirror(axis mrr_axis) throw (MyException);

	public:

		//CONSTRUCTORS-DECONSTRUCTOR
        StackedVolume(const char* _stacks_dir, ref_sys reference_system, float VXL_1=0, float VXL_2=0, float VXL_3=0, bool overwrite_mdata=false, bool make_n_slices_equal = false) throw (MyException);
		StackedVolume(const char *xml_filepath, bool make_n_slices_equal = false) throw (MyException);
		~StackedVolume();

		// ******GET METHODS******
		int		 getStacksHeight();
		int		 getStacksWidth();
		VirtualStack*** getSTACKS();

		
		//print all informations contained in this data structure
		void print();

		//loads/saves metadata from/in the given xml filename
		void loadXML(const char *xml_filename) throw (MyException);
		void initFromXML(const char *xml_filename) throw (MyException);
        void saveXML(const char *xml_filename=0, const char *xml_filepath=0) throw (MyException);


        /**********************************************************************************
        * UTILITY methods
        ***********************************************************************************/

        //counts the total number of displacements and the number of displacements per pair of adjacent stacks
        void countDisplacements(int& total, float& per_stack_pair);

        //counts the number of single-direction displacements having a reliability measure above the given threshold
        void countReliableSingleDirectionDisplacements(float threshold, int& total, int& reliable);

        //counts the number of stitchable stacks given the reliability threshold
        int countStitchableStacks(float threshold);

		// print mdata.bin content to stdout
		static void dumpMData(const char* volumePath) throw (MyException);

		// utility functions
		// string-based sprintf function
		inline static std::string strprintf(const std::string fmt, ...){
			int size = 100;
			std::string str;
			va_list ap;
			while (1) {
				str.resize(size);
				va_start(ap, fmt);
				int n = vsnprintf((char *)str.c_str(), size, fmt.c_str(), ap);
				va_end(ap);
				if (n > -1 && n < size) {
					str.resize(n);
					return str;
				}
				if (n > -1)
					size = n + 1;
				else
					size *= 2;
			}
			return str;
		}
};

//******* ABSTRACT TYPES DEFINITIONS *******
//struct VHD_triple{int V, H, D;};
//struct interval_t
//{
//	int start, end;
//        interval_t(void) :				   start(-1),	  end(-1)  {}
//        interval_t(int _start, int _end) : start(_start), end(_end){}
//};
//
//struct ref_sys 
//{
//	axis first, second, third; 
//	ref_sys(axis _first, axis _second, axis _third) : first(_first), second(_second), third(_third){}
//	ref_sys(): first(axis_invalid), second(axis_invalid), third(axis_invalid){}
//};



#endif /* STACKED_VOLUME_H */

