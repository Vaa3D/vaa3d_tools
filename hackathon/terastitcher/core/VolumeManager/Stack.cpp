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
*       Bria, A., et al., (2012) "Stitching Terabyte-sized 3D Images Acquired in Confocal Ultramicroscopy", Proceedings of the 9th IEEE International Symposium on Biomedical Imaging.
*       Bria, A., Iannello, G., "A Tool for Fast 3D Automatic Stitching of Teravoxel-sized Datasets", submitted on July 2012 to IEEE Transactions on Information Technology in Biomedicine.
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

#include "Stack.h"
#include <iostream>
#include <string>
#include <list>
#include <sstream>
#include "StackedVolume.h"
#include "StackStitcher.h"
#include <cxcore.h>
#include <highgui.h>
#ifdef _WIN32
	#include "dirent_win.h"
#else
	#include <dirent.h>
#endif
#include <list>
#include <string>
#include <cv.h>
#include "Displacement.h"

using namespace std;

//CONSTRUCTOR WITH ARGUMENTS
Stack::Stack(StackedVolume* _CONTAINER, int _ROW_INDEX, int _COL_INDEX, char* _DIR_NAME)
{
	#if VM_VERBOSE > 3
	printf("\t\t\t\tin Stack::Stack(StackedVolume* _CONTAINER, int _ROW_INDEX=%d, int _COL_INDEX=%d, char* _DIR_NAME=%s)\n",
		_ROW_INDEX, _COL_INDEX, _DIR_NAME);
	#endif

	this->CONTAINER = _CONTAINER;
	this->DIR_NAME = new char[strlen(_DIR_NAME)+1];
	strcpy(this->DIR_NAME, _DIR_NAME);
	this->ROW_INDEX = _ROW_INDEX;
	this->COL_INDEX = _COL_INDEX;
	STACKED_IMAGE = NULL;
	FILENAMES = NULL;
	HEIGHT = WIDTH = DEPTH = -1;
	ABS_V = ABS_H = ABS_D = -1;
	stitchable = false;

	init();
}

Stack::Stack(StackedVolume* _CONTAINER, int _ROW_INDEX, int _COL_INDEX, FILE* bin_file)
{
	#if VM_VERBOSE > 3
	printf("\t\t\t\tin Stack::Stack(StackedVolume* _CONTAINER, int _ROW_INDEX=%d, int _COL_INDEX=%d, FILE* bin_file)\n",
		_ROW_INDEX, _COL_INDEX);
	#endif

	CONTAINER = _CONTAINER;
	ROW_INDEX = _ROW_INDEX;
	COL_INDEX = _COL_INDEX;

	DIR_NAME = NULL;
	STACKED_IMAGE = NULL;
	FILENAMES = NULL;
	HEIGHT = WIDTH = DEPTH = -1;
	ABS_V = ABS_H = ABS_D = -1;
	stitchable = false;

	unBinarizeFrom(bin_file);
}

void Stack::init()
{
	#if VM_VERBOSE > 3
	printf("\t\t\t\tin Stack[%d,%d]::init()\n",ROW_INDEX, COL_INDEX);
	#endif

	//LOCAL variables
	string tmp;
	DIR *cur_dir_lev3;
	dirent *entry_lev3;
	list<string> entries_lev3;
	list<string>::iterator entry_k;
	string entry;

	//opening stack directory
	char abs_path[S_STATIC_STRINGS_SIZE];
	sprintf(abs_path,"%s/%s", CONTAINER->getSTACKS_DIR(), DIR_NAME);
	cur_dir_lev3 = opendir(abs_path);
	if (!cur_dir_lev3)
	{
		char errMsg[S_STATIC_STRINGS_SIZE];
		sprintf(errMsg, "in Stack::init(): can't open directory \"%s\"", abs_path);
		throw MyException(errMsg);
	}

	//scanning third level of hierarchy which entries need to be ordered alphabetically. This is done using STL.
	while ((entry_lev3=readdir(cur_dir_lev3)))
	{
		tmp = entry_lev3->d_name;
		if(tmp.compare(".") != 0 && tmp.compare("..") != 0 && tmp.find(".") != string::npos)
			entries_lev3.push_back(tmp);
	}
	entries_lev3.sort();
	DEPTH = (int)entries_lev3.size();

	//checking if current stack is not empty
	if(DEPTH == 0)
	{
		char msg[1000];
		sprintf(msg,"in Stack[%d,%d]::init(): stack is empty", ROW_INDEX, COL_INDEX);
		throw MyException(msg);
	}

	//converting filenames_list (STL list of <string> objects) into FILENAMES (1-D array of C-strings)
	FILENAMES = new char*[DEPTH];
	for(int z=0; z<DEPTH; z++)
	{
		entry = entries_lev3.front();
		FILENAMES[z] = new char[entry.size()+1];
		strcpy(FILENAMES[z], entry.c_str());
		entries_lev3.pop_front();
	}
	entries_lev3.clear();

	//extracting HEIGHT and WIDTH attributes from first slice
	char slice_fullpath[S_STATIC_STRINGS_SIZE];
	sprintf(slice_fullpath, "%s/%s/%s", CONTAINER->getSTACKS_DIR(), DIR_NAME, FILENAMES[0]);
	IplImage *img_tmp = cvLoadImage(slice_fullpath);
	if(!img_tmp)
	{
		char msg[S_STATIC_STRINGS_SIZE];
		sprintf(msg,"in Stack[%d,%d]::init(): unable to open image \"%s\". Possible unsupported format or it isn't an image.\nSupported formats are BMP, DIB, JPEG, JPG, JPE, PNG, PBM, PGM, PPM, SR, RAS, TIFF, TIF", 
			ROW_INDEX, COL_INDEX, slice_fullpath);
		throw MyException(msg);
	}
	HEIGHT = img_tmp->height;
	WIDTH  = img_tmp->width;
	cvReleaseImage(&img_tmp);
}

Stack::~Stack()
{
	#if VM_VERBOSE > 3
	printf("\t\t\t\tin Stack[%d,%d]::~Stack()\n",ROW_INDEX, COL_INDEX);
	#endif

	NORTH.clear();
	EAST.clear();
	SOUTH.clear();
	WEST.clear();

	for(int z=0; z<DEPTH; z++)
		if(FILENAMES[z])
			delete[] FILENAMES[z];
	if(STACKED_IMAGE)
		delete[] STACKED_IMAGE;
	if(FILENAMES)
		delete[] FILENAMES;
	if(DIR_NAME)
		delete[] DIR_NAME;
}

void Stack::print()
{
	printf("\t |\t[%d,%d]\n", ROW_INDEX, COL_INDEX);
	printf("\t |\tDirectory:\t\t%s\n", DIR_NAME);
	printf("\t |\tDimensions:\t\t%d(V) x %d(H) x %d(D)\n", HEIGHT, WIDTH, DEPTH);
	printf("\t |\tAbsolute position:\t%d(V) , %d(H) , %d(D)\n", ABS_V, ABS_H, ABS_D);
	printf("\t |\n");
}

//binarizing-unbinarizing methods
void Stack::binarizeInto(FILE* file)
{
	#if VM_VERBOSE > 3
	printf("\t\t\t\tin Stack[%d,%d]::binarizeInto(...)\n",ROW_INDEX, COL_INDEX);
	#endif

	//LOCAL VARIABLES
	uint16 str_size;
	int i;

	fwrite(&HEIGHT, sizeof(int), 1, file);
	fwrite(&WIDTH, sizeof(int), 1, file);
	fwrite(&DEPTH, sizeof(int), 1, file);
	fwrite(&ABS_V, sizeof(int), 1, file);
	fwrite(&ABS_H, sizeof(int), 1, file);
	fwrite(&ABS_D, sizeof(int), 1, file);
	str_size = (uint16)strlen(DIR_NAME) + 1;
	fwrite(&str_size, sizeof(uint16), 1, file);
	fwrite(DIR_NAME, str_size, 1, file);
	for(i = 0; i < DEPTH; i++)
	{
		str_size = (uint16)strlen(FILENAMES[i]) + 1;
		fwrite(&str_size, sizeof(uint16), 1, file);
		fwrite(FILENAMES[i], str_size, 1, file);
	}
}

void Stack::unBinarizeFrom(FILE* file) throw (MyException)
{
	#if VM_VERBOSE > 3
	printf("\t\t\t\tin Stack[%d,%d]::unBinarizeFrom(...)\n",ROW_INDEX, COL_INDEX);
	#endif

	//LOCAL VARIABLES
	uint16 str_size;
	int i;
	size_t fread_return_val;

	fread_return_val = fread(&HEIGHT, sizeof(int), 1, file);
	if(fread_return_val != 1)
		throw MyException("in Stack::unBinarizeFrom(...): error while reading binary metadata file");
	fread_return_val = fread(&WIDTH, sizeof(int), 1, file);
	if(fread_return_val != 1)
		throw MyException("in Stack::unBinarizeFrom(...): error while reading binary metadata file");
	fread_return_val = fread(&DEPTH, sizeof(int), 1, file);
	if(fread_return_val != 1)
		throw MyException("in Stack::unBinarizeFrom(...): error while reading binary metadata file");
	fread_return_val = fread(&ABS_V, sizeof(int), 1, file);
	if(fread_return_val != 1)
		throw MyException("in Stack::unBinarizeFrom(...): error while reading binary metadata file");
	fread_return_val = fread(&ABS_H, sizeof(int), 1, file);
	if(fread_return_val != 1)
		throw MyException("in Stack::unBinarizeFrom(...): error while reading binary metadata file");
	fread_return_val = fread(&ABS_D, sizeof(int), 1, file);
	if(fread_return_val != 1)
		throw MyException("in Stack::unBinarizeFrom(...): error while reading binary metadata file");
	fread_return_val = fread(&str_size, sizeof(uint16), 1, file);
	if(fread_return_val != 1)
		throw MyException("in Stack::unBinarizeFrom(...): error while reading binary metadata file");
	DIR_NAME = new char[str_size];
	fread_return_val = fread(DIR_NAME, str_size, 1, file);
	FILENAMES = new char*[DEPTH];
	for(i = 0; i < DEPTH; i++)
	{
		fread_return_val = fread(&str_size, sizeof(uint16), 1, file);
		if(fread_return_val != 1)
			throw MyException("in Stack::unBinarizeFrom(...): error while reading binary metadata file");
		FILENAMES[i] = new char[str_size];
		fread_return_val = fread(FILENAMES[i], str_size, 1, file);
		if(fread_return_val != 1)
			throw MyException("in Stack::unBinarizeFrom(...): error while reading binary metadata file");
	}
}

//GET methods
Displacement* Stack::getDisplacement(Stack* neighbour) throw (MyException)
{
	#if VM_VERBOSE > 4
	printf("........in Stack[%d,%d]::getDisplacement(Stack* neighbour[%d,%d])\n",ROW_INDEX, COL_INDEX, neighbour->ROW_INDEX, neighbour->COL_INDEX);
	#endif

	if(neighbour == NULL)
		throw MyException("...in Stack::getDisplacement(Stack* neighbour = NULL): invalid neighbour stack");
	else if(neighbour->ROW_INDEX == (ROW_INDEX -1) && neighbour->COL_INDEX == COL_INDEX)
		if(NORTH.size() == 1) return NORTH[0];
		else throw MyException("...in Stack::getDisplacement(Stack* neighbour): stack MUST contain one displacement only at NORTH");
	else if(neighbour->getROW_INDEX() == ROW_INDEX    && neighbour->getCOL_INDEX() == COL_INDEX -1)
		if(WEST.size() == 1) return WEST[0];
		else throw MyException("...in Stack::getDisplacement(Stack* neighbour): stack MUST contain one displacement only at WEST");
	else if(neighbour->getROW_INDEX() == ROW_INDEX +1 && neighbour->getCOL_INDEX() == COL_INDEX)
		if(SOUTH.size() == 1) return SOUTH[0];
		else throw MyException("...in Stack::getDisplacement(Stack* neighbour): stack MUST contain one displacement only at SOUTH");
	else if(neighbour->getROW_INDEX() == ROW_INDEX    && neighbour->getCOL_INDEX() == COL_INDEX +1)
		if(EAST.size() == 1) return EAST[0];
		else throw MyException("...in Stack::getDisplacement(Stack* neighbour): stack MUST contain one displacement only at EAST");
	else
		throw("...in Stack::getDisplacement(Stack* neighbour): neighbour is not a neighbour!!!");
}
int Stack::getABS(int direction) throw (MyException)
{
	#if VM_VERBOSE > 4
	printf("........in Stack[%d,%d]::getABS(direction = %d)\n",ROW_INDEX, COL_INDEX, direction);
	#endif

	if	   (direction == dir_vertical)
		return getABS_V();
	else if(direction == dir_horizontal)
		return getABS_H();
	else if(direction == dir_depth)
		return getABS_D();
	else
		throw MyException("in Stack::setABS(int _ABS, int direction): wrong direction inserted");
}

//SET methods
void Stack::setABS(int _ABS, int direction)  throw (MyException)
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
		throw MyException("in Stack::setABS(int _ABS, int direction): wrong direction inserted");
}

//XML methods
TiXmlElement* Stack::getXML()
{
	#if VM_VERBOSE > 3
	printf("......in Stack[%d,%d]::getXML()\n",ROW_INDEX, COL_INDEX);
	#endif

	TiXmlElement *xml_representation = new TiXmlElement("Stack");
	xml_representation->SetAttribute("ROW",ROW_INDEX);
	xml_representation->SetAttribute("COL",COL_INDEX);
	xml_representation->SetAttribute("ABS_V",ABS_V);
	xml_representation->SetAttribute("ABS_H",ABS_H);
	xml_representation->SetAttribute("ABS_D",ABS_D);
	xml_representation->SetAttribute("STITCHABLE",stitchable ? "yes" : "no");
	xml_representation->SetAttribute("DIR_NAME",DIR_NAME);
	vector<Displacement*>::iterator i;
	TiXmlElement *NORTH_displacements = new TiXmlElement("NORTH_displacements");
	for(i = NORTH.begin(); i != NORTH.end(); i++)
		NORTH_displacements->LinkEndChild((*i)->getXML());
	TiXmlElement *EAST_displacements = new TiXmlElement("EAST_displacements");
	for(i = EAST.begin(); i != EAST.end(); i++)
		EAST_displacements->LinkEndChild((*i)->getXML());
	TiXmlElement *SOUTH_displacements = new TiXmlElement("SOUTH_displacements");
	for(i = SOUTH.begin(); i != SOUTH.end(); i++)
		SOUTH_displacements->LinkEndChild((*i)->getXML());
	TiXmlElement *WEST_displacements = new TiXmlElement("WEST_displacements");
	for(i = WEST.begin(); i != WEST.end(); i++)
		WEST_displacements->LinkEndChild((*i)->getXML());
	xml_representation->LinkEndChild(NORTH_displacements);
	xml_representation->LinkEndChild(EAST_displacements);
	xml_representation->LinkEndChild(SOUTH_displacements);
	xml_representation->LinkEndChild(WEST_displacements);

	return xml_representation;
}

void Stack::loadXML(TiXmlElement *stack_node) throw (MyException)
{
	#if VM_VERBOSE > 3
	printf("\t\t\t\tin Stack[%d,%d]::loadXML(TiXmlElement *stack_node)\n",ROW_INDEX, COL_INDEX);
	#endif

	stack_node->QueryIntAttribute("ABS_V", &ABS_V);
	stack_node->QueryIntAttribute("ABS_H", &ABS_H);
	stack_node->QueryIntAttribute("ABS_D", &ABS_D);
	stitchable = strcmp(stack_node->Attribute("STITCHABLE"),"yes")==0 ? true : false;
	if(strcmp(stack_node->Attribute("DIR_NAME"), DIR_NAME) != 0)
	{
		char errMsg[2000];
		sprintf(errMsg, "in Stack[%d,%d]::loadXML(...): Mismatch between xml file and %s in <DIR_NAME> field.", ROW_INDEX, COL_INDEX, VM_BIN_METADATA_FILE_NAME);
		throw MyException(errMsg);
	}
	TiXmlElement *NORTH_displacements = stack_node->FirstChildElement("NORTH_displacements");
	for(TiXmlElement *displ_node = NORTH_displacements->FirstChildElement("Displacement"); displ_node; displ_node = displ_node->NextSiblingElement())
		NORTH.push_back(Displacement::getDisplacementFromXML(displ_node));
	TiXmlElement *EAST_displacements = stack_node->FirstChildElement("EAST_displacements");
	for(TiXmlElement *displ_node = EAST_displacements->FirstChildElement("Displacement"); displ_node; displ_node = displ_node->NextSiblingElement())
		EAST.push_back(Displacement::getDisplacementFromXML(displ_node));
	TiXmlElement *SOUTH_displacements = stack_node->FirstChildElement("SOUTH_displacements");
	for(TiXmlElement *displ_node = SOUTH_displacements->FirstChildElement("Displacement"); displ_node; displ_node = displ_node->NextSiblingElement())
		SOUTH.push_back(Displacement::getDisplacementFromXML(displ_node));
	TiXmlElement *WEST_displacements = stack_node->FirstChildElement("WEST_displacements");
	for(TiXmlElement *displ_node = WEST_displacements->FirstChildElement("Displacement"); displ_node; displ_node = displ_node->NextSiblingElement())
		WEST.push_back(Displacement::getDisplacementFromXML(displ_node));

}


//loads image stack from <first_file> to <last_file> extremes included, if not specified loads entire Stack
real_t* Stack::loadImageStack(int first_file, int last_file)
{
	#if VM_VERBOSE > 3
	printf("\t\t\t\tin Stack[%d,%d]::loadImageStack(first_file = %d, last_file = %d)\n",ROW_INDEX, COL_INDEX, first_file, last_file);
	#endif

	char base_path[2000];
	sprintf(base_path, "%s/%s", CONTAINER->getSTACKS_DIR(), DIR_NAME);
	STACKED_IMAGE=IOManager::loadImageStack(FILENAMES, depth, base_path,first_file, last_file);
	return STACKED_IMAGE;
}


//deallocates memory used by STACKED_IMAGE
void Stack::releaseImageStack()
{
	if(STACKED_IMAGE)
		delete[] STACKED_IMAGE;
	STACKED_IMAGE=NULL;
}

//show the selected slice with a simple GUI
void Stack::show(int D_index, int window_HEIGHT, int window_WIDTH)
{
	#if VM_VERBOSE > 2
	printf("\t\t\tin Stack::show(D_index=%d, win_height=%d, win_width=%d)\n", D_index, window_HEIGHT, window_WIDTH);
	#endif

	//converting selected slice of vol (1-D array of REAL_T) into a CvMat
	CvMat *slice = cvCreateMat(HEIGHT, WIDTH, CV_32FC1);
	for(int i=0; i<slice->rows; i++)
	{
		float *row_ptr = (float*)(slice->data.ptr+slice->step*i);
		for(int j=0; j<slice->cols; j++)
			row_ptr[j] = STACKED_IMAGE[D_index*HEIGHT*WIDTH + i*WIDTH +j];
	}

	//showing slice
	CvSize window_dims;
	window_dims.height = window_HEIGHT ? window_HEIGHT : HEIGHT;
	window_dims.width  = window_WIDTH  ? window_WIDTH  : WIDTH;
	char buffer[200];
	CvMat* mat_rescaled = cvCreateMat(HEIGHT, WIDTH, CV_16UC1);
	IplImage* image_resized = cvCreateImage(window_dims, IPL_DEPTH_16U, 1);
	IplImage* img_buffer = cvCreateImageHeader(window_dims, IPL_DEPTH_16U, 1);

	cvConvertScale(slice,mat_rescaled, 65535);
	cvResize(cvGetImage(mat_rescaled,img_buffer), image_resized, CV_INTER_CUBIC);

	sprintf(buffer,"SLICE %d of volume %d x %d",D_index, HEIGHT, WIDTH);
	cvNamedWindow(buffer,1);
	cvShowImage(buffer,image_resized);
	cvMoveWindow(buffer, 10,10);
	while(1)
	{
		if(cvWaitKey(100)==27) break;
	}

	cvDestroyWindow(buffer);

	cvReleaseMat(&slice);
	cvReleaseMat(&mat_rescaled);
	cvReleaseImage(&image_resized);
	cvReleaseImageHeader(&img_buffer);
}
