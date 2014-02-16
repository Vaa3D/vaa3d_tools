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

#ifndef _TILED_MC_VOLUME_H
#define _TILED_MC_VOLUME_H

#include "VirtualVolume.h"
#include "TiledVolume.h"
#include <list>
#include <string>

//every object of this class has the default (1,2,3) reference system
class TiledMCVolume : public VirtualVolume
{
	private:	
		//******OBJECT ATTRIBUTES******
        iim::uint16 N_ROWS, N_COLS;			//dimensions (in stacks) of stacks matrix along VH axes
        //Block ***BLOCKS;                  //2-D array of <Block*>
        iim::ref_sys reference_system;      //reference system of the stored volume
        float  VXL_1, VXL_2, VXL_3;         //voxel dimensions of the stored volume

		char        **CHDIRNAMES;			//1-D dinamic array of <char>  pointers to channels directory names
		TiledVolume **vol_ch;
        iim::uint32       *active;
		int           n_active;

		//***OBJECT PRIVATE METHODS****
		TiledMCVolume(void);

		//Given the reference system, initializes all object's members using stack's directories hierarchy
        void init();

		//rotate stacks matrix around D axis (accepted values are theta=0,90,180,270)
		void rotate(int theta);

		//mirror stacks matrix along mrr_axis (accepted values are mrr_axis=1,2,3)
        void mirror(iim::axis mrr_axis);

		// iannello returns the number of channels of images composing the volume
        void initChannels ( ) throw (iim::IOException);

	public:
		//CONSTRUCTORS-DECONSTRUCTOR
        TiledMCVolume(const char* _root_dir)  throw (iim::IOException);
        TiledMCVolume(const char* _root_dir, iim::ref_sys _reference_system,
					float _VXL_1, float _VXL_2, float _VXL_3, 
                    bool overwrite_mdata = false, bool save_mdata=true)  throw (iim::IOException);

		~TiledMCVolume(void);

		//GET methods
        iim::uint16 getN_ROWS(){return N_ROWS;}
        iim::uint16 getN_COLS(){return N_COLS;}
        float  getVXL_1(){return VXL_1;}
        float  getVXL_2(){return VXL_2;}
        float  getVXL_3(){return VXL_3;}
        iim::axis   getAXS_1(){return reference_system.first;}
        iim::axis   getAXS_2(){return reference_system.second;}
        iim::axis   getAXS_3(){return reference_system.third;}


		//PRINT method
		void print();

		//saving-loading methods to/from metadata binary file
        void save(char* metadata_filepath) throw (iim::IOException);
        void load(char* metadata_filepath) throw (iim::IOException);

        //loads given subvolume in a 1-D array of iim::real32 while releasing stacks slices memory when they are no longer needed
        inline iim::real32 *loadSubvolume_to_real32(int V0=-1,int V1=-1, int H0=-1, int H1=-1, int D0=-1, int D1=-1)  throw (iim::IOException) {
			return loadSubvolume(V0,V1,H0,H1,D0,D1,0,true);
		}

        //loads given subvolume in a 1-D array and puts used Stacks into 'involved_stacks' iff not null
        iim::real32 *loadSubvolume(int V0=-1,int V1=-1, int H0=-1, int H1=-1, int D0=-1, int D1=-1,
                                                                  std::list<Block*> *involved_blocks = 0, bool release_blocks = false)  throw (iim::IOException);

        //loads given subvolume in a 1-D array of iim::uint8 while releasing stacks slices memory when they are no longer needed
        iim::uint8 *loadSubvolume_to_UINT8(int V0=-1,int V1=-1, int H0=-1, int H1=-1, int D0=-1, int D1=-1, int *channels=0, int ret_type=iim::DEF_IMG_DEPTH) throw (iim::IOException);

		//sets active channels
        void setActiveChannels ( iim::uint32 *_active, int _n_active );


		// OPERATIONS FOR STREAMED SUBVOLUME LOAD 

		/* start a streamed load operation: returns an opaque descriptor of the streamed operation
		 * buf is a dynamically allocated, initialized buffer that should not be neither manipulated 
		 * nor deallocated by the caller until the operation terminates 
		 * (see close operations for details)
		 */
        void *streamedLoadSubvolume_open ( int steps, iim::uint8 *buf, int V0=-1,int V1=-1, int H0=-1, int H1=-1, int D0=-1, int D1=-1 );

		/* perform one step of a streamed operation: returns a pointer to a read-only buffer 
		 * with updated data; the returned buffer should not be deallocated;
		 * the optional parameter buffer2 is an initialized buffer of the same dimensions of 
		 * the returned buffer with reference data to be used to check that the updated positions 
		 * of the returned buffer contain exactly the same data contained in buffer2 at those positions
		 * no check is performed if the default value of buffer2 (null pointer) is passed 
		 */
        iim::uint8 *streamedLoadSubvolume_dostep ( void *stream_descr, unsigned char *buffer2=0 );

		/* copies the last data read from files to a user provided buffer
		 * positions to which data are copied are depend on which data have been read in 
		 * the last step
		 * the user provided buffer can be freely accessed by the caller after the operation is terminated;
		 * the optional parameter buffer2 is an initialized buffer of the same dimensions of 
		 * the user provided buffer with reference data to be used to check that the updated positions 
		 * of the user provided buffer contain exactly the same data contained in buffer2 at those positions
		 * no check is performed if the default value of buffer2 (null pointer) is passed 
		 */
         void streamedLoadSubvolume_cpydata ( void *stream_descr, unsigned char *buffer, unsigned char *buffer2=0 );

		/* close a streamed load operation: by default return the initial buffer that can be 
		 * freely used and must be deallocated by the caller
		 * if return_buffer is set to false, the initial buffer is deallocated, it cannot be 
		 * reused by the caller and the operation returns a null pointer
		 */
         iim::uint8 *streamedLoadSubvolume_close ( void *stream_descr, bool return_buffer=true );

};

#endif //_TILED_MC_VOLUME_H
