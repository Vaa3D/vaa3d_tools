/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).
 * All rights reserved.
 */


/************
                                            ********* LICENSE NOTICE ************

This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it.

You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.

1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.

2. You agree to appropriately cite this work in your related studies and publications.

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/




/*
 * basic_4dimage.cpp
 * last update: 100819: Hanchuan Peng. use MYLIB only for Llinux and Mac, but not WIN32. FIXME: add VC support later.
 * 20120410: add curFileSuffix check for the potential strcmp crashing. by Hanchuan Peng
 */

#include "v3d_message.h"

#include "stackutil.h"
#include "basic_4dimage.h"

//extern "C" {
//#include "../common_lib/src_packages/mylib_tiff/image.h"
//};

#ifdef _ALLOW_WORKMODE_MENU_
#include "../neuron_annotator/utility/ImageLoaderBasic.h"
#endif

typedef unsigned short int USHORTINT16;


void Image4DSimple::loadImage(const char* filename)
{
	return this->loadImage(filename, false); //default don't use MYLib
}

void Image4DSimple::loadImage(const char* filename, bool b_useMyLib)
{
	cleanExistData(); // note that this variable must be initialized as NULL.

	strcpy(imgSrcFile, filename);

	V3DLONG * tmp_sz = 0; // note that this variable must be initialized as NULL.
	int tmp_datatype = 0;
	int pixelnbits=1; //100817

    const char * curFileSuffix = getSuffix(imgSrcFile);
    //printf("The current input file has the suffix [%s]\n", curFileSuffix);

    if (curFileSuffix && (strcasecmp(curFileSuffix, "tif")==0 || strcasecmp(curFileSuffix, "tiff")==0 ||
        strcasecmp(curFileSuffix, "lsm")==0) ) //read tiff/lsm stacks
	{
            //printf("Image4DSimple::loadImage loading filename=[%s]\n", filename);

#if defined _WIN32
		{
            //v3d_msg("(Win32) Now try to use LIBTIFF (slightly revised by PHC) to read the TIFF/LSM...\n",0);
            if (strcasecmp(curFileSuffix, "tif")==0 || strcasecmp(curFileSuffix, "tiff")==0)
			{
				if (loadTif2Stack(imgSrcFile, data1d, tmp_sz, tmp_datatype))
				{
					v3d_msg("Error happens in TIF file reading (using libtiff). \n", false);
					b_error=1;
				}
			}
            else //if ( strcasecmp(curFileSuffix, "lsm")==0 ) //read lsm stacks
			{
				if (loadLsm2Stack(imgSrcFile, data1d, tmp_sz, tmp_datatype))
				{
					v3d_msg("Error happens in LSM file reading (using libtiff, slightly revised by PHC). \n", false);
					b_error=1;
				}
			}
		}

#else
		if (b_useMyLib)
		{
			v3d_msg("Now try to use MYLIB to read the TIFF/LSM again...\n",0);
			if (loadTif2StackMylib(imgSrcFile, data1d, tmp_sz, tmp_datatype, pixelnbits))
			{
				v3d_msg("Error happens in TIF/LSM file reading (using MYLIB). Stop. \n", false);
				b_error=1;
				return;
			}
			else
				b_error=0; //when succeed then reset b_error
		}
		else
		{
            //v3d_msg("Now try to use LIBTIFF (slightly revised by PHC) to read the TIFF/LSM...\n",0);
            if (strcasecmp(curFileSuffix, "tif")==0 || strcasecmp(curFileSuffix, "tiff")==0)
			{
				if (loadTif2Stack(imgSrcFile, data1d, tmp_sz, tmp_datatype))
				{
					v3d_msg("Error happens in TIF file reading (using libtiff). \n", false);
					b_error=1;
				}
			}
            else //if ( strcasecmp(curFileSuffix, "lsm")==0 ) //read lsm stacks
			{
				if (loadLsm2Stack(imgSrcFile, data1d, tmp_sz, tmp_datatype))
				{
					v3d_msg("Error happens in LSM file reading (using libtiff, slightly revised by PHC). \n", false);
					b_error=1;
				}
			}
		}
               // printf("Image4DSimple::loadImage finished\n");

#endif

	}    

    else if (curFileSuffix && (strcasecmp(curFileSuffix, "nrrd")==0 || strcasecmp(curFileSuffix, "nhdr")==0)) //read nrrd stacks
    {
       // printf("Image4DSimple::loadImage loading filename=[%s]\n", filename);
        float pxinfo[4];
        float spaceorigin[3];
        if (!read_nrrd_with_pxinfo(imgSrcFile, data1d, tmp_sz, tmp_datatype, pxinfo, spaceorigin))
        {
            v3d_msg("Error happens in NRRD file reading. Stop. \n", false);
            b_error=1;
            return;
        }
        else
        {   //copy the pixel size and origin info when the nrrd read is successful
            //note that pixinfo[3] is reserved for future extension to include the temporal sampling rate. It is not used right now.
            rez_x = pxinfo[0];
            rez_y = pxinfo[1];
            rez_z = pxinfo[2];
            
            origin_x = spaceorigin[0];
            origin_y = spaceorigin[1];
            origin_z = spaceorigin[2];
        }
    }

    else if ( curFileSuffix && strcasecmp(curFileSuffix, "mrc")==0 ) //read mrc stacks
	{
		if (loadMRC2Stack(imgSrcFile, data1d, tmp_sz, tmp_datatype))
		{
			v3d_msg("Error happens in MRC file reading. Stop. \n", false);
			b_error=1;
			return;
		}
	}
#ifdef _ALLOW_WORKMODE_MENU_
    //else if ( curFileSuffix && ImageLoader::hasPbdExtension(QString(filename)) ) // read v3dpbd - pack-bit-difference encoding for sparse stacks
    else if ( curFileSuffix && strcasecmp(curFileSuffix, "v3dpbd")==0 ) // read v3dpbd - pack-bit-difference encoding for sparse stacks
    {
    	  v3d_msg("start to try v3dpbd", 0);
        ImageLoaderBasic imageLoader;
        if (! imageLoader.loadRaw2StackPBD(imgSrcFile, this, false) == 0) {
            v3d_msg("Error happens in v3dpbd file reading. Stop. \n", false);
            b_error=1;
            return;
        }
        // The following few lines are to avoid disturbing the existing code below
        tmp_datatype=this->getDatatype();
        tmp_sz=new V3DLONG[4];
        tmp_sz[0]=this->getXDim();
        tmp_sz[1]=this->getYDim();
        tmp_sz[2]=this->getZDim();
        tmp_sz[3]=this->getCDim();

        this->setFileName(filename); // PHC added 20121213 to fix a bug in the PDB reading.
    }
#endif
	else //then assume it is Hanchuan's Vaa3D RAW format
	{
		v3d_msg("The data does not have supported image file suffix, -- now this program assumes it is Vaa3D's RAW format and tries to load it... \n", false);
		if (loadRaw2Stack(imgSrcFile, data1d, tmp_sz, tmp_datatype))
		{
			printf("The data doesn't look like a correct 4-byte-size Vaa3D's RAW file. Try 2-byte-raw. \n");
			if (loadRaw2Stack_2byte(imgSrcFile, data1d, tmp_sz, tmp_datatype))
			{
				v3d_msg("Error happens in reading 4-byte-size and 2-byte-size Vaa3D's RAW file. Stop. \n", false);
				b_error=1;
				return;
			}
		}
	}

	//080302: now convert any 16 bit or float data to the range of 0-255 (i.e. 8bit)
	switch (tmp_datatype)
	{
		case 1:
			datatype = V3D_UINT8;
			break;

		case 2: //080824
			//convert_data_to_8bit((void *&)data1d, tmp_sz, tmp_datatype);
			//datatype = UINT8; //UINT16;
			datatype = V3D_UINT16;
			break;

		case 4:
			//convert_data_to_8bit((void *&)data1d, tmp_sz, tmp_datatype);
			datatype = V3D_FLOAT32; //FLOAT32;
			break;

		default:
			v3d_msg("The data type is not UINT8, UINT16 or FLOAT32. Something wrong with the program, -- should NOT display this message at all. Check your program. \n");
			if (tmp_sz) {delete []tmp_sz; tmp_sz=0;}
			return;
	}

	sz0 = tmp_sz[0];
	sz1 = tmp_sz[1];
	sz2 = tmp_sz[2];
	sz3 = tmp_sz[3]; //no longer merge the 3rd and 4th dimensions

	/* clean all workspace variables */

	if (tmp_sz) {delete []tmp_sz; tmp_sz=0;}

	return;
}

void Image4DSimple::loadImage_slice(char filename[], bool b_useMyLib, V3DLONG zsliceno)
{
    cleanExistData(); // note that this variable must be initialized as NULL.

    strcpy(imgSrcFile, filename);

    V3DLONG * tmp_sz = 0; // note that this variable must be initialized as NULL.
    int tmp_datatype = 0;
    int pixelnbits=1; //100817

    const char * curFileSuffix = getSuffix(imgSrcFile);
    printf("The current input file has the suffix [%s]\n", curFileSuffix);

    if (curFileSuffix && (strcasecmp(curFileSuffix, "tif")==0 || strcasecmp(curFileSuffix, "tiff")==0 ||
        strcasecmp(curFileSuffix, "lsm")==0) ) //read tiff/lsm stacks
    {
           // printf("Image4DSimple::loadImage loading filename=[%s] slice =[%ld]\n", filename, zsliceno);

#if defined _WIN32
        {
            //v3d_msg("(Win32) Now try to use LIBTIFF (slightly revised by PHC) to read the TIFF/LSM...\n",0);
            if (strcasecmp(curFileSuffix, "tif")==0 || strcasecmp(curFileSuffix, "tiff")==0)
            {
                if (loadTifSlice(imgSrcFile, data1d, tmp_sz, tmp_datatype, zsliceno, false))
                {
                    v3d_msg("Error happens in TIF file reading (using libtiff). \n", false);
                    b_error=1;
                }
            }
            else //if ( strcasecmp(curFileSuffix, "lsm")==0 ) //read lsm stacks
            {
                if (loadLsmSlice(imgSrcFile, data1d, tmp_sz, tmp_datatype, zsliceno, false))
                {
                    v3d_msg("Error happens in LSM file reading (using libtiff, slightly revised by PHC). \n", false);
                    b_error=1;
                }
            }
        }

#else
        if (b_useMyLib)
        {
            if (loadTif2StackMylib_slice(imgSrcFile, data1d, tmp_sz, tmp_datatype, pixelnbits, zsliceno))
            {
                v3d_msg("Error happens in TIF/LSM file reading (using MYLIB). Stop. \n", false);
                b_error=1;
                return;
            }
            else
                b_error=0; //when succeed then reset b_error
        }
        else
        {
            //v3d_msg("Now try to use LIBTIFF (slightly revised by PHC) to read the TIFF/LSM...\n",0);
            if (strcasecmp(curFileSuffix, "tif")==0 || strcasecmp(curFileSuffix, "tiff")==0)
            {
                if (loadTifSlice(imgSrcFile, data1d, tmp_sz, tmp_datatype, zsliceno, false))
                {
                    v3d_msg("Error happens in TIF file reading (using libtiff). \n", false);
                    b_error=1;
                }
            }
            else //if ( strcasecmp(curFileSuffix, "lsm")==0 ) //read lsm stacks
            {
                if (loadLsmSlice(imgSrcFile, data1d, tmp_sz, tmp_datatype, zsliceno, false))
                {
                    v3d_msg("Error happens in LSM file reading (using libtiff, slightly revised by PHC). \n", false);
                    b_error=1;
                }
            }
        }
               // printf("Image4DSimple::loadImage finished\n");

#endif

    }

   /*
    else if ( curFileSuffix && strcasecmp(curFileSuffix, "mrc")==0 ) //read mrc stacks
    {
        if (loadMRC2Stack_slice(imgSrcFile, data1d, tmp_sz, tmp_datatype, layer))
        {
            v3d_msg("Error happens in MRC file reading. Stop. \n", false);
            b_error=1;
            return;
        }
    }
#ifdef _ALLOW_WORKMODE_MENU_
    //else if ( curFileSuffix && ImageLoader::hasPbdExtension(QString(filename)) ) // read v3dpbd - pack-bit-difference encoding for sparse stacks
    else if ( curFileSuffix && strcasecmp(curFileSuffix, "v3dpbd")==0 ) // read v3dpbd - pack-bit-difference encoding for sparse stacks
    {
          v3d_msg("start to try v3dpbd", 0);
        ImageLoaderBasic imageLoader;
        if (! imageLoader.loadRaw2StackPBD(imgSrcFile, this, false) == 0) {
            v3d_msg("Error happens in v3dpbd file reading. Stop. \n", false);
            b_error=1;
            return;
        }
        // The following few lines are to avoid disturbing the existing code below
        tmp_datatype=this->getDatatype();
        tmp_sz=new V3DLONG[4];
        tmp_sz[0]=this->getXDim();
        tmp_sz[1]=this->getYDim();
        tmp_sz[2]=this->getZDim();
        tmp_sz[3]=this->getCDim();

        this->setFileName(filename); // PHC added 20121213 to fix a bug in the PDB reading.
    }
#endif
    else //then assume it is Hanchuan's Vaa3D RAW format
    {
        v3d_msg("The data does not have supported image file suffix, -- now this program assumes it is Vaa3D's RAW format and tries to load it... \n", false);
        if (loadRaw2Stack_slice(imgSrcFile, data1d, tmp_sz, tmp_datatype, layer))
        {
            printf("The data doesn't look like a correct 4-byte-size Vaa3D's RAW file. Try 2-byte-raw. \n");
            if (loadRaw2Stack_2byte_slice(imgSrcFile, data1d, tmp_sz, tmp_datatype, layer))
            {
                v3d_msg("Error happens in reading 4-byte-size and 2-byte-size Vaa3D's RAW file. Stop. \n", false);
                b_error=1;
                return;
            }
        }
    }

    */

    //Temporarily do nothing to read other single slice from other formats
    else
    {
        v3d_msg("The single slice reading function is NOT available for other format at this moment.");
        b_error=1;
        return;
    }

    //080302: now convert any 16 bit or float data to the range of 0-255 (i.e. 8bit)
    switch (tmp_datatype)
    {
        case 1:
            datatype = V3D_UINT8;
            break;

        case 2: //080824
            //convert_data_to_8bit((void *&)data1d, tmp_sz, tmp_datatype);
            //datatype = UINT8; //UINT16;
            datatype = V3D_UINT16;
            break;

        case 4:
            //convert_data_to_8bit((void *&)data1d, tmp_sz, tmp_datatype);
            datatype = V3D_FLOAT32; //FLOAT32;
            break;

        default:
            v3d_msg("The data type is not UINT8, UINT16 or FLOAT32. Something wrong with the program, -- should NOT display this message at all. Check your program. \n");
            if (tmp_sz) {delete []tmp_sz; tmp_sz=0;}
            return;
    }

    sz0 = tmp_sz[0];
    sz1 = tmp_sz[1];
    sz2 = tmp_sz[2];
    sz3 = tmp_sz[3]; //no longer merge the 3rd and 4th dimensions

    /* clean all workspace variables */

    if (tmp_sz) {delete []tmp_sz; tmp_sz=0;}

    return;
}


bool Image4DSimple::saveImage(const char filename[])
{
    if (!data1d || !filename)
	{
		v3d_msg("This image data is empty or the file name is invalid. Nothing done.\n");
		return false;
	}
    
    V3DLONG mysz[4];
	mysz[0] = sz0;
	mysz[1] = sz1;
	mysz[2] = sz2;
	mysz[3] = sz3;

	int dt;
	switch (datatype)
	{
		case V3D_UINT8:  dt=1; break;
		case V3D_UINT16:  dt=2; break;
		case V3D_FLOAT32:  dt=4; break;
		default:
			v3d_msg("The data type is unsupported. Nothing done.\n");
			return false;
			break;
	}

    if (strlen(filename) > 5 ) {
        const char * suffix = getSuffix((char *)filename);
        if (suffix && (strcasecmp(suffix, "nrrd")==0 ||
                       strcasecmp(suffix, "nhdr")==0) )
        {
            float pxinfo[4];
            float spaceorigin[3];
            // use nrrd_write
            pxinfo[0] = rez_x;
            pxinfo[1] = rez_y;
            pxinfo[2] = rez_z;
            
            spaceorigin[0] = origin_x;
            spaceorigin[1] = origin_y;
            spaceorigin[2] = origin_z;
            return write_nrrd_with_pxinfo(filename, data1d, mysz, dt, pxinfo, spaceorigin);
        }
    }
    return ::saveImage(filename, data1d, mysz, dt);
}


