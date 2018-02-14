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

/******************
*    CHANGELOG    *
*******************
* 2017-05-15. Giulio.    @CHANGED introduced a symbol to enable/disable the management of the resume state
* 2017-04-07. Giulio.    @CHANGED some error message (more informative)
* 2017-04-01. Giulio.    @CHANGED the interface of the routines to be used for 2D output formats
* 2017-04-01. Giulio.    @FIXED some bugs
* 2014-10-29. Giulio.    @ADDED fflush after writes
*/

#include "vcresumer.h"

#ifdef _WIN32
#include "dirent_win.h"
#else
#include <dirent.h>
#endif

#include <string.h>
#include <string>

//#define SAVE_RESUMER_STATUS

#define RESUMER_STATUS_FILE_NAME   "resume_status.bin"
#define RECORD_LENGTH	(	\
	sizeof(sint64) +		\
	sizeof(sint64)			\
)
#define RECORD_LENGTH_L(resolutions_size)  (	\
	sizeof(int) +								\
	sizeof(int)*resolutions_size +				\
	sizeof(int)*resolutions_size +				\
	sizeof(int)*resolutions_size +				\
	sizeof(sint64) +							\
	sizeof(sint64)								\
)


using namespace std;
using namespace iim;



bool initVCResumer ( const char *out_fmt, const char *output_path, int resolutions_size, bool* resolutions, 
				   int slice_height, int slice_width, int method, const char* saved_img_format, int saved_img_depth, FILE *&fhandle ) throw (iim::IOException) {

#ifdef SAVE_RESUMER_STATUS

	size_t  str_len;
	char resumer_filepath[STATIC_STRINGS_SIZE];
	char err_msg[STATIC_STRINGS_SIZE];

	sprintf(resumer_filepath, "%s/%s", output_path, RESUMER_STATUS_FILE_NAME);
    if ( iim::isFile(resumer_filepath) ) {
		if ( (fhandle = fopen(resumer_filepath,"rb")) == 0 ) {
			sprintf(err_msg, "in initResumer: file %s cannot be opened for reading",output_path);
            throw IOException(err_msg);
		}
		else {
			char _out_fmt[STATIC_STRINGS_SIZE];
			char _output_path[STATIC_STRINGS_SIZE];
			int  _resolutions_size;
            bool _resolutions[TMITREE_MAX_HEIGHT];
			int  _slice_height;
			int  _slice_width;
			int  _method;
			char _saved_img_format[STATIC_STRINGS_SIZE];
			int  _saved_img_depth;
			size_t dummy;

			rewind(fhandle);
			dummy = fread(&str_len,sizeof(size_t),1,fhandle);
			dummy = fread(_out_fmt,sizeof(char),str_len,fhandle);
			if ( strcmp(out_fmt,_out_fmt) ) {
				fclose(fhandle);
				sprintf(err_msg, "in initResumer: saved format of output image (%s) differ from requested format (%s)", 
					_out_fmt,out_fmt);
                throw IOException(err_msg);
			}

			dummy = fread(&str_len,sizeof(size_t),1,fhandle);
			dummy = fread(_output_path,sizeof(char),str_len,fhandle);
			dummy = fread(&_resolutions_size,sizeof(int),1,fhandle);
			dummy = fread(&_resolutions,sizeof(bool),_resolutions_size,fhandle);
			dummy = fread(&_slice_height,sizeof(int),1,fhandle);
			dummy = fread(&_slice_width,sizeof(int),1,fhandle);
			dummy = fread(&_method,sizeof(int),1,fhandle);
			dummy = fread(&str_len,sizeof(size_t),1,fhandle);
			dummy = fread(_saved_img_format,sizeof(char),str_len,fhandle);
			dummy = fread(&_saved_img_depth,sizeof(int),1,fhandle);
			
			if ( strcmp(output_path,_output_path) ||
				 slice_height!=_slice_height || slice_width!=_slice_width ||
				 strcmp(saved_img_format,_saved_img_format) || saved_img_depth!=_saved_img_depth ) {
				fclose(fhandle);
				sprintf(err_msg, "in initResumer: saved parameters differ from current parameters "
					"(output path: %s vs. %s, height: %d vs. %d, width: %d vs. %d, saved format: %s vs. %s, image depth: %d vs. %d)", 
					output_path,_output_path,slice_height,_slice_height,slice_width,_slice_width,saved_img_format,_saved_img_format,saved_img_depth,_saved_img_depth);
                throw IOException(err_msg);
			}

			bool res_err = true;
			int i;
			if ( resolutions_size==_resolutions_size ) {
				for ( i=0, res_err=false; i<resolutions_size && !res_err; i++ )
					res_err = (resolutions[i] != _resolutions[i]);
			}

			if ( res_err ) {
				fclose(fhandle);
				sprintf(err_msg, "in initResumer: saved resolutions differ from requested resolutions (%s vs. %s)", 
					_out_fmt,out_fmt);
                throw IOException(err_msg);
			}

		}

		int headerSize = ftell(fhandle);
		fseek(fhandle, 0, SEEK_END);
		int fileSize = ftell(fhandle);
		fseek(fhandle, fileSize-RECORD_LENGTH, SEEK_SET);

		return true;
	}
	else {
		if ( (fhandle = fopen(resumer_filepath,"ab")) == 0 ) {
			sprintf(err_msg, "in initResumer: file %s cannot be opened for append",output_path);
            throw IOException(err_msg);
		}
		else {
			str_len = strlen(out_fmt) + 1;
			fwrite(&str_len,sizeof(size_t),1,fhandle);
			fwrite(out_fmt,sizeof(char),str_len,fhandle);
			str_len = strlen(output_path) + 1;
			fwrite(&str_len,sizeof(size_t),1,fhandle);
			fwrite(output_path,sizeof(char),str_len,fhandle);
			fwrite(&resolutions_size,sizeof(int),1,fhandle);
			fwrite(resolutions,sizeof(bool),resolutions_size,fhandle);
			fwrite(&slice_height,sizeof(int),1,fhandle);
			fwrite(&slice_width,sizeof(int),1,fhandle);
			fwrite(&method,sizeof(int),1,fhandle);
			str_len = strlen(saved_img_format) + 1;
			fwrite(&str_len,sizeof(size_t),1,fhandle);
			fwrite(saved_img_format,sizeof(char),str_len,fhandle);
			fwrite(&saved_img_depth,sizeof(int),1,fhandle);
			fflush(fhandle);
		}
		return false;
	}

#else

	return false;
	
#endif
}


void readVCResumerState ( FILE *&fhandle, const char *output_path, iim::sint64 &z, iim::sint64 &z_parts ) throw (iim::IOException) {

#ifdef SAVE_RESUMER_STATUS

	size_t dummy = fread(&z,sizeof(sint64),1,fhandle);
	dummy = fread(&z_parts,sizeof(sint64),1,fhandle);

	fclose(fhandle);

	char resumer_filepath[STATIC_STRINGS_SIZE];
	sprintf(resumer_filepath, "%s/%s", output_path, RESUMER_STATUS_FILE_NAME);
	if ( (fhandle = fopen(resumer_filepath,"ab")) == 0 ) {
		char err_msg[STATIC_STRINGS_SIZE];
		sprintf(err_msg, "in initResumer: the resume state file cannot be re-opened in append mode");
        throw IOException(err_msg);
	}

#endif
}


void saveVCResumerState ( FILE *fhandle, iim::sint64 z, iim::sint64 z_parts ) throw (iim::IOException) {

#ifdef SAVE_RESUMER_STATUS

	fwrite(&z,sizeof(sint64),1,fhandle);
	fwrite(&z_parts,sizeof(sint64),1,fhandle);
	fflush(fhandle);

#endif
}



bool initVCResumer ( const char *out_fmt, const char *output_path, int resolutions_size, bool* resolutions, 
				   int block_height, int block_width, int block_depth, int method, 
                   const char* saved_img_format, int saved_img_depth, FILE *&fhandle ) throw (IOException)
{

#ifdef SAVE_RESUMER_STATUS

	size_t  str_len;
	char resumer_filepath[STATIC_STRINGS_SIZE];
	char err_msg[STATIC_STRINGS_SIZE];

	sprintf(resumer_filepath, "%s/%s", output_path, RESUMER_STATUS_FILE_NAME);
    if ( iim::isFile(resumer_filepath) ) {
		if ( (fhandle = fopen(resumer_filepath,"rb")) == 0 ) {
			sprintf(err_msg, "in initResumer: file %s cannot be opened for reading",output_path);
            throw IOException(err_msg);
		}
		else {
			char _out_fmt[STATIC_STRINGS_SIZE];
			char _output_path[STATIC_STRINGS_SIZE];
			int  _resolutions_size;
            bool _resolutions[TMITREE_MAX_HEIGHT];
			int  _block_height;
			int  _block_width;
			int  _block_depth;
			int  _method;
			char _saved_img_format[STATIC_STRINGS_SIZE];
			int  _saved_img_depth;
			size_t dummy;

			rewind(fhandle);
			dummy = fread(&str_len,sizeof(size_t),1,fhandle);
			dummy = fread(_out_fmt,sizeof(char),str_len,fhandle);
			if ( strcmp(out_fmt,_out_fmt) ) {
				fclose(fhandle);
				sprintf(err_msg, "in initResumer: saved format of output image (%s) differ from requested format (%s)", 
					_out_fmt,out_fmt);
                throw IOException(err_msg);
			}

			dummy = fread(&str_len,sizeof(size_t),1,fhandle);
			dummy = fread(_output_path,sizeof(char),str_len,fhandle);
			dummy = fread(&_resolutions_size,sizeof(int),1,fhandle);
			dummy = fread(&_resolutions,sizeof(bool),_resolutions_size,fhandle);
			dummy = fread(&_block_height,sizeof(int),1,fhandle);
			dummy = fread(&_block_width,sizeof(int),1,fhandle);
			dummy = fread(&_block_depth,sizeof(int),1,fhandle);
			dummy = fread(&_method,sizeof(int),1,fhandle);
			dummy = fread(&str_len,sizeof(size_t),1,fhandle);
			dummy = fread(_saved_img_format,sizeof(char),str_len,fhandle);
			dummy = fread(&_saved_img_depth,sizeof(int),1,fhandle);
			
			if ( strcmp(output_path,_output_path) ||
				 block_height!=_block_height || block_width!=_block_width || block_depth!=_block_depth ||
				 strcmp(saved_img_format,_saved_img_format) || saved_img_depth!=_saved_img_depth ) {
				fclose(fhandle);
				sprintf(err_msg, "in initResumer: saved parameters differ from current parameters "
					"(output path: %s vs. %s, height: %d vs. %d, width: %d vs. %d, depth: %d vs. %d, saved format: %s vs. %s, image depth: %d vs. %d)", 
					output_path,_output_path,block_height,_block_height,block_width,_block_width,block_depth,_block_depth,saved_img_format,_saved_img_format,saved_img_depth,_saved_img_depth);
                throw IOException(err_msg);
			}

			bool res_err = true;
			int i;
			if ( resolutions_size==_resolutions_size ) {
				for ( i=0, res_err=false; i<resolutions_size && !res_err; i++ )
					res_err = (resolutions[i] != _resolutions[i]);
			}

			if ( res_err ) {
				fclose(fhandle);
				sprintf(err_msg, "in initResumer: saved resolutions differ from requested resolutions (%s vs. %s)", 
					_out_fmt,out_fmt);
                throw IOException(err_msg);
			}

		}

		int headerSize = ftell(fhandle);
		fseek(fhandle, 0, SEEK_END);
		int fileSize = ftell(fhandle);
		fseek(fhandle, fileSize-RECORD_LENGTH_L(resolutions_size), SEEK_SET);

		return true;
	}
	else {
		if ( (fhandle = fopen(resumer_filepath,"ab")) == 0 ) {
			sprintf(err_msg, "in initResumer: file %s cannot be opened for append",output_path);
            throw IOException(err_msg);
		}
		else {
			str_len = strlen(out_fmt) + 1;
			fwrite(&str_len,sizeof(size_t),1,fhandle);
			fwrite(out_fmt,sizeof(char),str_len,fhandle);
			str_len = strlen(output_path) + 1;
			fwrite(&str_len,sizeof(size_t),1,fhandle);
			fwrite(output_path,sizeof(char),str_len,fhandle);
			fwrite(&resolutions_size,sizeof(int),1,fhandle);
			fwrite(resolutions,sizeof(bool),resolutions_size,fhandle);
			fwrite(&block_height,sizeof(int),1,fhandle);
			fwrite(&block_width,sizeof(int),1,fhandle);
			fwrite(&block_depth,sizeof(int),1,fhandle);
			fwrite(&method,sizeof(int),1,fhandle);
			str_len = strlen(saved_img_format) + 1;
			fwrite(&str_len,sizeof(size_t),1,fhandle);
			fwrite(saved_img_format,sizeof(char),str_len,fhandle);
			fwrite(&saved_img_depth,sizeof(int),1,fhandle);
			fflush(fhandle);
		}
		return false;
	}

#else

	return false;
	
#endif
}

void readVCResumerState ( FILE *&fhandle, const char *output_path, int &resolutions_size, int *stack_block, int *slice_start, int *slice_end, 
                 sint64 &z, sint64 &z_parts ) throw (IOException)
{

#ifdef SAVE_RESUMER_STATUS

	size_t dummy = fread(&resolutions_size,sizeof(int),1,fhandle);
	dummy = fread(stack_block,sizeof(int),resolutions_size,fhandle);
	dummy = fread(slice_start,sizeof(int),resolutions_size,fhandle);
	dummy = fread(slice_end,sizeof(int),resolutions_size,fhandle);
	dummy = fread(&z,sizeof(sint64),1,fhandle);
	dummy = fread(&z_parts,sizeof(sint64),1,fhandle);

	fclose(fhandle);

	char resumer_filepath[STATIC_STRINGS_SIZE];
	sprintf(resumer_filepath, "%s/%s", output_path, RESUMER_STATUS_FILE_NAME);
	if ( (fhandle = fopen(resumer_filepath,"ab")) == 0 ) {
		char err_msg[STATIC_STRINGS_SIZE];
		sprintf(err_msg, "in initResumer: the resume state file cannot be re-opened in append mode");
        throw IOException(err_msg);
	}
	
#endif
}

void saveVCResumerState ( FILE *fhandle, int resolutions_size, int *stack_block, int *slice_start, int *slice_end, 
                 sint64 z, sint64 z_parts ) throw (IOException)
{

#ifdef SAVE_RESUMER_STATUS

	fwrite(&resolutions_size,sizeof(int),1,fhandle);
	fwrite(stack_block,sizeof(int),resolutions_size,fhandle);
	fwrite(slice_start,sizeof(int),resolutions_size,fhandle);
	fwrite(slice_end,sizeof(int),resolutions_size,fhandle);
	fwrite(&z,sizeof(sint64),1,fhandle);
	fwrite(&z_parts,sizeof(sint64),1,fhandle);
	fflush(fhandle);

#endif
}

void closeVCResumer ( FILE *fhandle, const char *output_path ) throw (IOException)
{

#ifdef SAVE_RESUMER_STATUS

	//char err_msg[STATIC_STRINGS_SIZE];

	fclose(fhandle);

	if ( output_path ) {
		char resumer_filepath[STATIC_STRINGS_SIZE];
		sprintf(resumer_filepath, "%s/%s", output_path, RESUMER_STATUS_FILE_NAME);
		remove(resumer_filepath);
	}

#endif
}
