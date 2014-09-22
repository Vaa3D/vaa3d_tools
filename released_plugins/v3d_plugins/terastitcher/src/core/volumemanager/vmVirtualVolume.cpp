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
* 2014-09-10. Alessandro. @ADDED 'getVolumeFormat' method to be applied on xml file.
* 2014-09-01. Alessandro. @FIXED 'extractCoordinates()' method to deal with sparse tiles w/o causing crashes.
* 2014-09-01. Alessandro. @ADDED 'name2coordZ()' utility method to extract the z-coordinate substring from a given filename.
* 2014-08-30. Alessandro. @REMOVED 'extractPathFromFilePath()' method (obsolete).
* 2014-08-30. Alessandro. @REMOVED 'print()' method from abstract class and all concrete classes (obsolete).
*/

#include <string>
#include "vmVirtualVolume.h"
#include "vmVirtualStack.h"
#ifdef _WIN32
#include "dirent_win.h"
#else
#include <dirent.h>
#endif
#include <math.h> //fabs


using namespace std;
using namespace vm;

class VirtualStack;

float	VirtualVolume::getORG_V()					{return ORG_V;}
float	VirtualVolume::getORG_H()					{return ORG_H;}
float	VirtualVolume::getORG_D()					{return ORG_D;}
float	VirtualVolume::getABS_V(int ABS_PIXEL_V)	{return ORG_V * 1000 + ABS_PIXEL_V*this->getVXL_V();}	//Alessandro - 23/03/2013: removed conversion from int to float
float	VirtualVolume::getABS_H(int ABS_PIXEL_H)	{return ORG_H * 1000 + ABS_PIXEL_H*this->getVXL_H();}	//Alessandro - 23/03/2013: removed conversion from int to float
float	VirtualVolume::getABS_D(int ABS_PIXEL_D)	{return ORG_D * 1000 + ABS_PIXEL_D*this->getVXL_D();}	//Alessandro - 23/03/2013: removed conversion from int to float
float	VirtualVolume::getVXL_V()					{return VXL_V;}
float	VirtualVolume::getVXL_H()					{return VXL_H;}
float	VirtualVolume::getVXL_D()					{return VXL_D;}
float	VirtualVolume::getMEC_V()					{return MEC_V;}
float	VirtualVolume::getMEC_H()					{return MEC_H;}
int		VirtualVolume::getN_ROWS()					{return this->N_ROWS;}
int		VirtualVolume::getN_COLS()					{return this->N_COLS;}
int		VirtualVolume::getN_SLICES()				{return this->N_SLICES;}
//char*   VirtualVolume::getSTACKS_DIR()				{return this->stacks_dir;}
int		VirtualVolume::getOVERLAP_V()				{return (int)(getStacksHeight() - MEC_V/VXL_V);}
int		VirtualVolume::getOVERLAP_H()				{return (int)(getStacksWidth() -  MEC_H/VXL_H);}
int		VirtualVolume::getDEFAULT_DISPLACEMENT_V()	{return (int)(fabs(MEC_V/VXL_V));}
int		VirtualVolume::getDEFAULT_DISPLACEMENT_H()	{return (int)(fabs(MEC_H/VXL_H));}
int		VirtualVolume::getDEFAULT_DISPLACEMENT_D()	{return 0;}

void VirtualVolume::init() throw (iom::exception)
{
	stacks_dir = 0;

	reference_system.first = reference_system.second = reference_system.third = axis_invalid;

	VXL_V = 0;
	VXL_H = 0;
	VXL_D = 0;

	ORG_V = ORG_H = ORG_D = (float) 0.0;
	MEC_V = MEC_H = 0;

	N_ROWS = N_COLS = 0;
	N_SLICES = 0;
}

//returns true if file exists at the given filepath
bool VirtualVolume::fileExists(const char *filepath)  throw (iom::exception)
{
	//LOCAL VARIABLES
	string file_path_string =filepath;
	string file_name;
	string dir_path;
	bool file_exists = false;
	DIR* directory;
	dirent* dir_entry;

	//extracting dir_path and file_name from file_path
	char * tmp;
	tmp = strtok (&file_path_string[0],"/\\");
	while (tmp != NULL)
	{
		file_name = tmp;
		tmp = strtok (NULL, "/\\");
	}
	file_path_string =filepath;
	dir_path=file_path_string.substr(0,file_path_string.find(file_name));

	//obtaining DIR pointer to directory (=NULL if directory doesn't exist)
	if (!(directory=opendir(&(dir_path[0]))))
	{
		char msg[1000];
		sprintf(msg,"in fileExists(filepath=%s): Unable to open directory \"%s\"", filepath, &dir_path[0]);
		throw iom::exception(msg);
	}

	//scanning for given file
	while (!file_exists && (dir_entry=readdir(directory)))
	{
		//storing in tmp i-th entry and checking that it not contains '.', so that I can exclude '..', '.' and files entries
		if(!strcmp(&(file_name[0]), dir_entry->d_name))
			file_exists = true;
	}
	closedir(directory);

	return file_exists;
}

// returns the Z-coordinate string extracted from the given filename. Supported filenames formats are
// [0-9]+_[0-9]+_[0-9]+.*   and
// [0-9]+.*
std::string VirtualVolume::name2coordZ(const std::string & filename) throw (iom::exception)
{
	// split string using "_" token (also works if "_" is not present)
	std::vector<std::string> tokens;
	vm::split(filename, "_", tokens);

	// check
	if(tokens.empty())
		throw iom::exception(vm::strprintf("in VirtualVolume::name2coordZ(): error when parsing filename %s", filename.c_str()).c_str());

	// remove file extension and return
	return tokens[tokens.size()-1].substr(0, tokens[tokens.size()-1].find_last_of("."));
}

//extract spatial coordinates (in millimeters) of given Stack object
void VirtualVolume::extractCoordinates(VirtualStack* stk, int z, int* crd_1, int* crd_2, int* crd_3)
{
    #if VM_VERBOSE > 3
    printf("\t\t\t\tin VirtualVolume::extractCoordinates(stk=\"%s\", z = %d)\n", stk->getDIR_NAME(), z);
    #endif

	bool found_ABS_X=false;
	bool found_ABS_Y=false;

	//loading estimations for absolute X and Y stack positions
	char * pch;
	char buffer[100];
	strcpy(buffer,&(stk->getDIR_NAME()[0]));
	pch = strtok (buffer,"/_");
	pch = strtok (NULL, "/_");

	while (pch != NULL)
	{
		if(!found_ABS_X)
		{
			if(sscanf(pch, "%d", crd_1) == 1)
				found_ABS_X=true;
		}
		else if(!found_ABS_Y)
		{
			if(sscanf(pch, "%d", crd_2) == 1)
				found_ABS_Y=true;
		}
		else
			break;

		pch = strtok (NULL, "/_");
	}

	if(!found_ABS_X || !found_ABS_Y)
		throw iom::exception(vm::strprintf("in VirtualVolume::extractCoordinates(directory_name=\"%s\"): format 000000_000000 or X_000000_X_000000 not found", stk->getDIR_NAME()).c_str());

	//loading estimation for absolute Z stack position
	if(crd_3!= NULL)
	{
		// check for existing slice at z
		// 2014-09-01. Alessandro. @FIXED 'extractCoordinates()' method to deal with sparse tiles w/o causing crashes.
		if(!stk->isComplete(z,z))
			throw iom::exception(vm::strprintf("in VirtualVolume::extractCoordinates(directory_name=\"%s\"): no slice found at z=%d", stk->getDIR_NAME(), z).c_str());

		char* first_file_name = stk->getFILENAMES()[z];

		char * pch;
		char lastTokenized[100];
		char buffer[500];
		strcpy(buffer,&(first_file_name[0]));

		pch = strtok (buffer,"_");
		while (pch != NULL)
		{
			strcpy(lastTokenized,pch);
			pch = strtok (NULL, "_");
		}

		pch = strtok (lastTokenized,".");
		strcpy(lastTokenized,pch);

		if(sscanf(lastTokenized, "%d", crd_3) != 1)
			throw iom::exception(vm::strprintf("in VirtualVolume::extractCoordinates(...): unable to extract Z position from filename %s", first_file_name).c_str());
	}
}

//inserts the given displacements in the given stacks
void VirtualVolume::insertDisplacement(VirtualStack *stk_A, VirtualStack *stk_B, Displacement *displacement)  throw (iom::exception)
{
	int stk_A_row = stk_A->getROW_INDEX();
	int stk_A_col = stk_A->getCOL_INDEX();
	int stk_B_row = stk_B->getROW_INDEX();
	int stk_B_col = stk_B->getCOL_INDEX();
	displacement->evalReliability(dir_vertical);
	displacement->evalReliability(dir_horizontal);
	displacement->evalReliability(dir_depth);
	if(stk_B_row == stk_A_row && stk_B_col == stk_A_col+1)
	{
		displacement->setDefaultV(0);	//we assume that adjacent tiles are aligned with respect to motorized stages coordinates
		displacement->setDefaultH(getDEFAULT_DISPLACEMENT_H());
		displacement->setDefaultD(getDEFAULT_DISPLACEMENT_D());
		stk_A->getEAST().push_back(displacement);
		stk_B->getWEST().push_back(displacement->getMirrored(dir_horizontal));
	}
	else if(stk_B_row == stk_A_row +1 && stk_B_col == stk_A_col)
	{
		displacement->setDefaultV(getDEFAULT_DISPLACEMENT_V());
		displacement->setDefaultH(0);	//we assume that adjacent tiles are aligned with respect to motorized stages coordinates					
		displacement->setDefaultD(getDEFAULT_DISPLACEMENT_D());
		stk_A->getSOUTH().push_back(displacement);
		stk_B->getNORTH().push_back(displacement->getMirrored(dir_vertical));
	}
	else
	{
		char errMsg[1000];
		sprintf(errMsg, "in VirtualVolume::insertDisplacement(stk_A[%d,%d], stk_B[%d,%d], displacement): stacks are not adjacent", 
			    stk_A->getROW_INDEX(), stk_A->getCOL_INDEX(), stk_B->getROW_INDEX(), stk_B->getCOL_INDEX());
		throw iom::exception(errMsg);
	}
}

// return 'volume_format' attribute of <TeraStitcher> node from the given xml. 
std::string VirtualVolume::getVolumeFormat(const std::string& xml_path) throw (iom::exception)
{
	// open xml
	TiXmlDocument xml;
	if(!xml.LoadFile(xml_path.c_str()))
		throw iom::exception(vm::strprintf("in VirtualVolume::getVolumeFormat(): cannot open xml file at \"%s\"", xml_path.c_str()));

	// get root node
	TiXmlHandle hRoot(xml.FirstChildElement("TeraStitcher"));

	// get 'volume_format' attribute
	const char *volformat = hRoot.ToElement()->Attribute("volume_format");
	if(!volformat)
		throw iom::exception(vm::strprintf("in VirtualVolume::getVolumeFormat(): cannot find 'volume_format' <TeraStitcher> attribute in xml file at \"%s\". Too old xml, please regenerate it.", xml_path.c_str()).c_str());

	return volformat;
}
