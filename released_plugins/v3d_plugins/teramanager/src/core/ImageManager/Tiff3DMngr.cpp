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

#include "Tiff3DMngr.h"
#include <stdlib.h> // needed by clang: defines size_t
#include <string.h>
#include "tiffio.h"


//char *initTiff3DFile ( char *filename, unsigned int sz0, unsigned int sz1, unsigned int sz2, unsigned int sz3, int datatype ) {
//
//    uint32 image_width, image_height;
//    float xres, yres;
//    uint16 spp, bpp, photo, res_unit;
//	uint32 rps;
//    uint16 comp, planar_config;
//    TIFF *out;
//    int check, StripsPerImage,LastStripSize;
//	char img_filepath[5000];
//
//	sprintf(img_filepath, "%s.%s", filename, "tif");
//
//	out=TIFFOpen(img_filepath,"w");
//	if (!out)
//    {
//            return "Can't create tiff multipage file.";
//    }
//
//    image_width = sz0;
//    image_height = sz1;
//    spp = sz3; // Samples per pixel 
//    bpp = 8 * datatype; // Bits per sample 
//	// WARNING: the number of pages (sz[2]) should be also stored
//
//    photo = PHOTOMETRIC_MINISBLACK;
//
//	TIFFSetField(out, TIFFTAG_IMAGEWIDTH, image_width / spp);
//    TIFFSetField(out, TIFFTAG_IMAGELENGTH, image_height);
//    TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, bpp);
//    TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, spp);
//    TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
//    TIFFSetField(out, TIFFTAG_PHOTOMETRIC, photo);
//    TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_BOTLEFT);
//
//    // It is good to set resolutions too (but it is not nesessary) 
//    //xres = yres = 10;
//    //res_unit = RESUNIT_INCH;
//    //TIFFSetField(out, TIFFTAG_XRESOLUTION, xres);
//    //TIFFSetField(out, TIFFTAG_YRESOLUTION, yres);
//    //TIFFSetField(out, TIFFTAG_RESOLUTIONUNIT, res_unit);
//
//    // We are writing single page of the multipage file 
//    TIFFSetField(out, TIFFTAG_SUBFILETYPE, FILETYPE_PAGE);
//
//    // Set the page number 
//    TIFFSetField(out, TIFFTAG_PAGENUMBER, 0, sz2); 
//
//    TIFFWriteDirectory(out);
//
//	TIFFClose(out);
//
//	return 0;
//}

char *initTiff3DFile ( char *filename, unsigned int sz0, unsigned int sz1, unsigned int sz2, unsigned int sz3, int datatype ) {
//int initTiff3DFile ( char *filename, uint32 XSIZE, uint32 YSIZE, uint16 spp, uint16 Npages, int datatype){

	uint32 XSIZE  = sz0;
	uint32 YSIZE  = sz1;
	uint16 Npages = sz2;
	uint16 spp    = sz3; 

	char *completeFilename = new char[strlen(filename)+4+1];
	strcpy(completeFilename,filename);
	strcat(completeFilename,".");
	strcat(completeFilename,TIFF3D_SUFFIX);

	TIFF *output;
	output=TIFFOpen(completeFilename,"w");
	uint16 bpp=8 * datatype;
	unsigned char *fakeData=new unsigned char[XSIZE * YSIZE];
	

	TIFFSetField(output, TIFFTAG_IMAGEWIDTH, XSIZE);
	TIFFSetField(output, TIFFTAG_IMAGELENGTH, YSIZE);
	TIFFSetField(output, TIFFTAG_BITSPERSAMPLE, bpp); 
	TIFFSetField(output, TIFFTAG_SAMPLESPERPIXEL, spp);
	TIFFSetField(output, TIFFTAG_ROWSPERSTRIP, YSIZE); // one page per strip
	TIFFSetField(output, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
	TIFFSetField(output, TIFFTAG_PLANARCONFIG,PLANARCONFIG_CONTIG);
	TIFFSetField(output, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);	
	/* We are writing single page of the multipage file */
	TIFFSetField(output, TIFFTAG_SUBFILETYPE, FILETYPE_PAGE);
	TIFFSetField(output, TIFFTAG_PAGENUMBER, 0, Npages); 

	TIFFWriteEncodedStrip(output, 0, fakeData, XSIZE * YSIZE);
	delete[] fakeData;

	TIFFWriteDirectory(output);

	TIFFClose(output);

	return (char *) 0;
}

char *appendSlice2Tiff3DFile ( char *filename, int slice, unsigned char *img, unsigned int img_height, unsigned int img_width ) {
	TIFF *output;
	uint16 spp, bpp, NPages, pg0;

	output=TIFFOpen(filename,"r");
	TIFFGetField(output, TIFFTAG_BITSPERSAMPLE, &bpp); 
	TIFFGetField(output, TIFFTAG_SAMPLESPERPIXEL, &spp);
	TIFFGetField(output, TIFFTAG_PAGENUMBER, &pg0, &NPages);
	TIFFClose(output);
	// since we are 
	output = (slice==0)? TIFFOpen(filename,"w") : TIFFOpen(filename,"a");

	TIFFSetDirectory(output,slice); // WARNING: slice must be the first page after the last, otherwise the file can be corrupted

	TIFFSetField(output, TIFFTAG_IMAGEWIDTH, img_width);
	TIFFSetField(output, TIFFTAG_IMAGELENGTH, img_height);
	TIFFSetField(output, TIFFTAG_BITSPERSAMPLE, bpp); 
	TIFFSetField(output, TIFFTAG_SAMPLESPERPIXEL, spp);
	TIFFSetField(output, TIFFTAG_ROWSPERSTRIP, img_height);
	TIFFSetField(output, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
	TIFFSetField(output, TIFFTAG_PLANARCONFIG,PLANARCONFIG_CONTIG);
	TIFFSetField(output, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);	
	// We are writing single page of the multipage file 
	TIFFSetField(output, TIFFTAG_SUBFILETYPE, FILETYPE_PAGE);
	TIFFSetField(output, TIFFTAG_PAGENUMBER, (uint16)slice, NPages); 

	TIFFWriteEncodedStrip(output, 0, img, img_width * img_height);
	//img +=  img_width * img_height;

	TIFFWriteDirectory(output);

	TIFFClose(output);

	return (char *) 0;
}

char *readTiff3DFile2Buffer ( char *filename, unsigned char *img, unsigned int img_width, unsigned int img_height, unsigned int first, unsigned int last ) {
	uint32 rps;
    uint16 spp, bpp, photo, comp, planar_config;
    TIFF *input;
    int check, StripsPerImage,LastStripSize;

	input=TIFFOpen(filename,"r");
	if (!input)
    {
		//throw MyException(strprintf("in IOManager::readTiffMultipage(...): Cannot open the file %s",finName).c_str());
		return ((char *) "Cannot open the file.");
    }

	//if (!check)
	//{
	//	NPAGES=0;
	//	do {
	//		NPAGES++;
	//	} while (TIFFReadDirectory(input));
	//	/*return to first directory*/
	//	check=TIFFSetDirectory(input, 0);
	//}

	//check=TIFFGetField(input, TIFFTAG_SUBFILETYPE, &subType);
	//if (!check)
	//{
	//	fprintf (stderr, "Cannot determine subfile data descriptor of %s \n", finName);
	//	//return 1;
	//}
	
	//check=TIFFGetField(input, TIFFTAG_XRESOLUTION, &xres);
	//if (!check)
	//{
	//	fprintf (stderr, "Cannot determine x resolution of %s \n", finName);
	//}
	//
	//check=TIFFGetField(input, TIFFTAG_YRESOLUTION, &yres);
	//if (!check)
	//{
	//	fprintf (stderr, "Cannot determine y resolution of %s \n", finName);
	//}
	//
	//check=TIFFGetField(input, TIFFTAG_RESOLUTIONUNIT, &res_unit);
	//if (!check)
	//{
	//	fprintf (stderr, "Cannot determine units of resolutions of %s \n", finName);
	//}

	//!!!!!!!!!!!!!!!!!!!!!usare come  controllo!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//check=TIFFGetField(input, TIFFTAG_IMAGEWIDTH, &XSIZE);
	//if (!check)
	//{
	//	fprintf (stderr, "Image width of %s undefined\n", finName);
	//}

	//check=TIFFGetField(input, TIFFTAG_IMAGELENGTH, &YSIZE);
	//if (!check)
	//{
	//	fprintf (stderr, "Image length of %s undefined\n", finName);
	//}

	check=TIFFGetField(input, TIFFTAG_ROWSPERSTRIP, &rps);
	if (!check)
	{
		TIFFClose(input);
		//throw MyException(strprintf("in IOManager::readTiffMultipage(...): Image length of %s undefined\n", finName).c_str());
		return ((char *) "Image length of undefined.");
	}		
    
	check=TIFFGetField(input, TIFFTAG_BITSPERSAMPLE, &bpp); 
	if (!check)
	{
		TIFFClose(input);
		//throw MyException(strprintf("in IOManager::readTiffMultipage(...): Undefined bits per sample in %s \n", finName).c_str());
		return ((char *) "Undefined bits per sample.");
	}

	check=TIFFGetField(input, TIFFTAG_SAMPLESPERPIXEL, &spp);
	if (!check)
	{
		TIFFClose(input);
		//throw MyException(strprintf("in IOManager::readTiffMultipage(...): Undefined bits per sample in %s \n", finName).c_str());
		return ((char *) "Undefined samples per pixel.");
	}

	check=TIFFGetField(input, TIFFTAG_PHOTOMETRIC, &photo);
	if (!check)
	{
		TIFFClose(input);
		//throw MyException(strprintf("in IOManager::readTiffMultipage(...): Cannot determine photometric interpretation of %s \n", finName).c_str());
		return ((char *) "Cannot determine photometric interpretation.");
	}

	check=TIFFGetField(input, TIFFTAG_COMPRESSION, &comp);
	if (!check)
	{
		TIFFClose(input);
		//throw MyException(strprintf("in IOManager::readTiffMultipage(...): Cannot determine compression technique of %s \n", finName).c_str());
		return ((char *) "Cannot determine compression technique.");
	}

	check=TIFFGetField(input, TIFFTAG_PLANARCONFIG, &planar_config);
	if (!check)
	{
		TIFFClose(input);
		//throw MyException(strprintf("in IOManager::readTiffMultipage(...): Cannot determine planar configuration of %s \n", finName).c_str());
		return ((char *) "Cannot determine planar configuration.");
	}


	StripsPerImage =  (img_height + rps - 1) / rps;
	LastStripSize = img_height % rps;
	if (LastStripSize==0)
		LastStripSize=rps;

	check=TIFFSetDirectory(input, first);
	if (!check)
	{
		//fprintf (stderr, "Cannot open the requested first strip from %s", finName);
		TIFFClose(input);
		return ((char *) "Cannot open the requested first strip.");
	}

	unsigned char *buf = img;
	int page=0;
	do{

		for (int i=0; i < StripsPerImage-1; i++){
			if (comp==1) {
				TIFFReadRawStrip(input, i, buf, spp * rps * img_width * (bpp/8));
				buf = buf + spp * rps * img_width * (bpp/8);
			}
			else{
				TIFFReadEncodedStrip(input, i, buf, spp * rps * img_width * (bpp/8));
				buf = buf + spp * rps * img_width * (bpp/8);
			}
		}

		if (comp==1) {
			TIFFReadRawStrip(input, StripsPerImage-1, buf, spp * LastStripSize * img_width * (bpp/8));
		}
		else{
			TIFFReadEncodedStrip(input, StripsPerImage-1, buf, spp * LastStripSize * img_width * (bpp/8));
		}
		buf = buf + spp * LastStripSize * img_width * (bpp/8);

		page++;
	
	}while ( page < (last-first+1) && TIFFReadDirectory(input));//while (TIFFReadDirectory(input));

	TIFFClose(input);

	if ( page < (last-first+1) ){
		return ((char *) "Cannot read all the pages.");
	}

	return (char *) 0;
}
