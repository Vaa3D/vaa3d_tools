//------------------------------------------------------------------------------------------------
// Copyright (c) 2014  Alessandro Bria and Giulio Iannello (University Campus Bio-Medico of Rome).  
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
* 2017-05-10. Giulio.     @ADDED 'input_plugin' attribute to <TeraStitcher> XML node
* 2014-11-23. Giulio.     @CREATED 
*/

#include "displacementManager.h"
#include "iomanager.config.h"

#include <string.h>
#include <list>
#include <fstream>
#include <string>

#ifdef _WIN32
#include "dirent_win.h"
#else
#include <dirent.h>
#endif

using namespace std;

XMLDisplacementBag::XMLDisplacementBag ( const char *main_dir_ ) {
	main_dir = new char[strlen(main_dir_+1)];
	strcpy(main_dir,main_dir_);

	init();

}


XMLDisplacementBag::~XMLDisplacementBag ( ) {
	if  (XMLFileNames ) {
		for ( int i=0; i<nFiles; i++ )
			if ( XMLFileNames[i] )
				delete []XMLFileNames[i];
		delete []XMLFileNames;
	}
}


void XMLDisplacementBag::init ( ) {

    /**/iom::debug(iom::LEV3, iom::strprintf("main_dir=%s", main_dir).c_str(), __iom__current__function__);

	//LOCAL variables
	string tmp;
	DIR *cur_dir;
	dirent *entry;
	list<string> entries;
	string file;

	//building filenames_list
	cur_dir = opendir(main_dir);
	if (!cur_dir)
		throw iom::exception(iom::strprintf("unable to open directory %s", main_dir), __iom__current__function__);

	//scanning third level of hierarchy which entries need to be ordered alphabetically. This is done using STL.
	while ((entry=readdir(cur_dir)))
	{
		tmp = entry->d_name;
		if(tmp.compare(".") != 0 && tmp.compare("..") != 0 && tmp.find(".") != string::npos)
			entries.push_back(tmp);
	}
	entries.sort();

	nFiles = (int)entries.size();

	XMLFileNames = new char *[nFiles];

	for ( int i=0; i<nFiles; i++ ) {
		file = entries.front();
		XMLFileNames[i] = new char[file.size()+1];
		strcpy(XMLFileNames[i],file.c_str());
		entries.pop_front();
	}
}


void XMLDisplacementBag::merge ( const char *xml_out_file ) {

	int i, j, k;

	// LOCAL VARIABLES FOR INPUT
	char xml_filepath[2000];
	TiXmlDocument *xml;

	//LOCAL VARIABLES FOR OUTPUT
	TiXmlDocument out_xml;
	TiXmlElement * out_root;
	TiXmlElement * out_pelem;

	xml = new TiXmlDocument[nFiles];

	// open input files
	for ( i=0; i<nFiles; i++ ) {
		sprintf(xml_filepath,"%s/%s",main_dir,XMLFileNames[i]);
		try {
			if(!xml[i].LoadFile(xml_filepath)) {
				char errMsg[2000];
				sprintf(errMsg,"in XMLDisplacementBag::merge() : unable to load xml %s", xml_filepath);
				throw iom::exception(errMsg);
			}
		}
		catch( iom::exception& exception){
			char errMsg[2000];
			sprintf(errMsg,"in XMLDisplacementBag::merge() : unable to load xml %s (cause: %s)", xml_filepath, exception.what());
			throw iom::exception(errMsg);
		}
		catch(bad_exception& be){
			char errMsg[2000];
			sprintf(errMsg,"in XMLDisplacementBag::merge() : unable to load xml %s (cause: %s)", xml_filepath, be.what());
			throw iom::exception(errMsg);
		}
		catch(char* error){
			char errMsg[2000];
			sprintf(errMsg,"in XMLDisplacementBag::merge() : unable to load xml %s (cause: %s)", xml_filepath, error);
			throw iom::exception(errMsg);
		}
	}

	// initializing output XML file with DTD declaration
	fstream XML_FILE(xml_out_file, ios::out);
	XML_FILE<<"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"<<endl;
	XML_FILE<<"<!DOCTYPE TeraStitcher SYSTEM \"TeraStitcher.DTD\">"<<endl;
	XML_FILE.close();

	//loading previously initialized output XML file 
    if(!out_xml.LoadFile(xml_out_file))
	{
		char errMsg[5000];
		sprintf(errMsg, "in XMLDisplacementBag::merge(...) : unable to load xml file at \"%s\"", xml_out_file);
		throw iom::exception(errMsg);
	}

	// IN: setting ROOT element (that is the first child, i.e. <TeraStitcher> node)
	TiXmlHandle hRoot(xml[0].FirstChildElement("TeraStitcher"));

	// OUT: inserting root node <TeraStitcher> and children nodes
	out_root = new TiXmlElement("TeraStitcher");  
	out_xml.LinkEndChild( out_root );  
	out_root->SetAttribute("volume_format", hRoot.ToElement()->Attribute("volume_format"));

	// 2017-05-10. Giulio. ADDED 'input_plugin' attribute to <TeraStitcher> XML node
	if(hRoot.ToElement()->Attribute("input_plugin"))
		out_root->SetAttribute("input_plugin", hRoot.ToElement()->Attribute("input_plugin"));

	// IN: reading fields and checking coherence with metadata previously read from VM_BIN_METADATA_FILE_NAME
	TiXmlElement *pelem = hRoot.FirstChildElement("stacks_dir").Element();

	// OUT: 
	out_pelem = new TiXmlElement("stacks_dir");
	out_pelem->SetAttribute("value", pelem->Attribute("value"));
	out_root->LinkEndChild(out_pelem);

	// IN:
	if ( (pelem = hRoot.FirstChildElement("ref_sys").Element()) != 0 ) { // skip if not present (for compatibility with previous versions)
		out_pelem = new TiXmlElement("ref_sys");
		out_pelem->SetAttribute("ref1", pelem->Attribute("ref1"));
		out_pelem->SetAttribute("ref2", pelem->Attribute("ref2"));
		out_pelem->SetAttribute("ref3", pelem->Attribute("ref3"));
		out_root->LinkEndChild(out_pelem);
	}
	pelem = hRoot.FirstChildElement("voxel_dims").Element();
	out_pelem = new TiXmlElement("voxel_dims");
	out_pelem->SetAttribute("V", pelem->Attribute("V"));
	out_pelem->SetAttribute("H", pelem->Attribute("H"));
	out_pelem->SetAttribute("D", pelem->Attribute("D"));
	out_root->LinkEndChild(out_pelem);
	pelem = hRoot.FirstChildElement("origin").Element();
	out_pelem = new TiXmlElement("origin");
	out_pelem->SetAttribute("V", pelem->Attribute("V"));
	out_pelem->SetAttribute("H", pelem->Attribute("H"));
	out_pelem->SetAttribute("D", pelem->Attribute("D"));
	out_root->LinkEndChild(out_pelem);
	pelem = hRoot.FirstChildElement("mechanical_displacements").Element();
	out_pelem = new TiXmlElement("mechanical_displacements");
	out_pelem->SetAttribute("V", pelem->Attribute("V"));
	out_pelem->SetAttribute("H", pelem->Attribute("H"));
	out_root->LinkEndChild(out_pelem);

	int N_ROWS;
	int N_COLS;

	pelem = hRoot.FirstChildElement("dimensions").Element();
	pelem->QueryIntAttribute("stack_rows", &N_ROWS);
	pelem->QueryIntAttribute("stack_columns", &N_COLS);
	out_pelem = new TiXmlElement("dimensions");
	out_pelem->SetAttribute("stack_rows", pelem->Attribute("stack_rows"));
	out_pelem->SetAttribute("stack_columns", pelem->Attribute("stack_columns"));
	out_pelem->SetAttribute("stack_slices", pelem->Attribute("stack_slices"));
	out_root->LinkEndChild(out_pelem);

	TiXmlHandle  **hRootList = new TiXmlHandle *[nFiles];
	TiXmlElement **pelemList = new TiXmlElement *[nFiles];
	TiXmlElement  *out_pelem2;

	for ( i=0; i<nFiles; i++ ) {
		hRootList[i] = new TiXmlHandle(xml[i].FirstChildElement("TeraStitcher"));
		pelemList[i] = hRootList[i]->FirstChildElement("STACKS").Element()->FirstChildElement();
	}

	out_pelem = new TiXmlElement("STACKS");

	pelem = hRoot.FirstChildElement("STACKS").Element()->FirstChildElement();

	for(i=0; i<N_ROWS; i++) {
		for(j=0; j<N_COLS; j++, pelem=pelem->NextSiblingElement()) {

			const char *attrib;
			out_pelem2 = new TiXmlElement("Stack");
			//out_pelem2->SetAttribute("N_BLOCKS", pelem->Attribute("N_BLOCKS"));
			attrib = pelem->Attribute("N_BLOCKS");
			if ( attrib ) {
			// is a tiled format: the attribute has to be added
				out_pelem2->SetAttribute("N_BLOCKS", attrib);
			}			
			//out_pelem2->SetAttribute("BLOCK_SIZES", pelem->Attribute("BLOCK_SIZES"));
			attrib = pelem->Attribute("BLOCK_SIZES");
			if ( attrib ) {
			// is a tiled format: the attribute has to be added
				out_pelem2->SetAttribute("BLOCK_SIZES", attrib);
			}			
			//out_pelem2->SetAttribute("BLOCKS_ABS_D", pelem->Attribute("BLOCKS_ABS_D"));
			attrib = pelem->Attribute("BLOCKS_ABS_D");
			if ( attrib ) {
			// is a tiled format: the attribute has to be added
				out_pelem2->SetAttribute("BLOCKS_ABS_D", attrib);
			}			
			out_pelem2->SetAttribute("N_CHANS", pelem->Attribute("N_CHANS"));
			out_pelem2->SetAttribute("N_BYTESxCHAN", pelem->Attribute("N_BYTESxCHAN"));
			out_pelem2->SetAttribute("ROW", pelem->Attribute("ROW"));
			out_pelem2->SetAttribute("COL", pelem->Attribute("COL"));
			out_pelem2->SetAttribute("ABS_V", pelem->Attribute("ABS_V"));
			out_pelem2->SetAttribute("ABS_H", pelem->Attribute("ABS_H"));
			out_pelem2->SetAttribute("ABS_D", pelem->Attribute("ABS_D"));
			out_pelem2->SetAttribute("STITCHABLE", pelem->Attribute("STITCHABLE"));
			out_pelem2->SetAttribute("DIR_NAME", pelem->Attribute("DIR_NAME"));
			//out_pelem2->SetAttribute("Z_RANGES", pelem->Attribute("Z_RANGES"));
			attrib = pelem->Attribute("Z_RANGES");
			if ( attrib ) {
			// is a stacked format: the attribute has to be added
				out_pelem2->SetAttribute("Z_RANGES", attrib);
			}			
			//out_pelem2->SetAttribute("IMG_REGEX", pelem->Attribute("IMG_REGEX"));
			attrib = pelem->Attribute("IMG_REGEX");
			if ( attrib ) {
			// is a stacked format: the attribute has to be added
				out_pelem2->SetAttribute("IMG_REGEX", attrib);
			}			

			mergeStack(pelemList,out_pelem2);

			out_pelem->LinkEndChild(out_pelem2);

			for ( k=0; k<nFiles; k++) 
				pelemList[k] = pelemList[k]->NextSiblingElement();
		}
	}

	out_root->LinkEndChild(out_pelem);

	//saving the file
	out_xml.SaveFile();
}


void XMLDisplacementBag::mergeStack ( TiXmlElement **stack_nodes, TiXmlElement *out_pelem ) {

	TiXmlElement *out_pelem2;
	//TiXmlElement *out_pelem3;
	//TiXmlElement *out_pelem4;

	out_pelem2 = new TiXmlElement("NORTH_displacements");
	for ( int k=0; k<nFiles; k++) {
		TiXmlElement *NORTH_displacements = stack_nodes[k]->FirstChildElement("NORTH_displacements");
		for(TiXmlElement *displ_node = NORTH_displacements->FirstChildElement("Displacement"); displ_node; displ_node = displ_node->NextSiblingElement()) {
			mergeDisp(displ_node,out_pelem2);
		}
	}
	out_pelem->LinkEndChild(out_pelem2);

	out_pelem2 = new TiXmlElement("EAST_displacements");
	for ( int k=0; k<nFiles; k++) {
		TiXmlElement *EAST_displacements = stack_nodes[k]->FirstChildElement("EAST_displacements");
		for(TiXmlElement *displ_node = EAST_displacements->FirstChildElement("Displacement"); displ_node; displ_node = displ_node->NextSiblingElement()) {
			mergeDisp(displ_node,out_pelem2);
		}
	}
	out_pelem->LinkEndChild(out_pelem2);

	out_pelem2 = new TiXmlElement("SOUTH_displacements");
	for ( int k=0; k<nFiles; k++) {
		TiXmlElement *SOUTH_displacements = stack_nodes[k]->FirstChildElement("SOUTH_displacements");
		for(TiXmlElement *displ_node = SOUTH_displacements->FirstChildElement("Displacement"); displ_node; displ_node = displ_node->NextSiblingElement()) {
			mergeDisp(displ_node,out_pelem2);
		}
	}
	out_pelem->LinkEndChild(out_pelem2);

	out_pelem2 = new TiXmlElement("WEST_displacements");
	for ( int k=0; k<nFiles; k++) {
		TiXmlElement *WEST_displacements = stack_nodes[k]->FirstChildElement("WEST_displacements");
		for(TiXmlElement *displ_node = WEST_displacements->FirstChildElement("Displacement"); displ_node; displ_node = displ_node->NextSiblingElement()) {
			mergeDisp(displ_node,out_pelem2);
		}
	}
	out_pelem->LinkEndChild(out_pelem2);
}


void XMLDisplacementBag::mergeDisp ( TiXmlElement *disp_node, TiXmlElement *out_pelem ) {

	TiXmlElement *out_pelem2;
	TiXmlElement *out_pelem3;

	out_pelem2 = new TiXmlElement("Displacement");
	out_pelem2->SetAttribute("TYPE", disp_node->Attribute("TYPE"));

	TiXmlElement *disp_node2 = disp_node->FirstChildElement();	

	out_pelem3 = new TiXmlElement("V");
	out_pelem3->SetAttribute("displ", disp_node2->Attribute("displ"));
	out_pelem3->SetAttribute("default_displ", disp_node2->Attribute("default_displ"));
	out_pelem3->SetAttribute("reliability", disp_node2->Attribute("reliability"));
	out_pelem3->SetAttribute("nccPeak", disp_node2->Attribute("nccPeak"));
	out_pelem3->SetAttribute("nccWidth", disp_node2->Attribute("nccWidth"));
	out_pelem3->SetAttribute("nccWRangeThr", disp_node2->Attribute("nccWRangeThr"));
	out_pelem3->SetAttribute("nccInvWidth", disp_node2->Attribute("nccInvWidth"));
	out_pelem3->SetAttribute("delay", disp_node2->Attribute("delay"));
	out_pelem2->LinkEndChild(out_pelem3);

	disp_node2 = disp_node2->NextSiblingElement();

	out_pelem3 = new TiXmlElement("H");
	out_pelem3->SetAttribute("displ", disp_node2->Attribute("displ"));
	out_pelem3->SetAttribute("default_displ", disp_node2->Attribute("default_displ"));
	out_pelem3->SetAttribute("reliability", disp_node2->Attribute("reliability"));
	out_pelem3->SetAttribute("nccPeak", disp_node2->Attribute("nccPeak"));
	out_pelem3->SetAttribute("nccWidth", disp_node2->Attribute("nccWidth"));
	out_pelem3->SetAttribute("nccWRangeThr", disp_node2->Attribute("nccWRangeThr"));
	out_pelem3->SetAttribute("nccInvWidth", disp_node2->Attribute("nccInvWidth"));
	out_pelem3->SetAttribute("delay", disp_node2->Attribute("delay"));
	out_pelem2->LinkEndChild(out_pelem3);

	disp_node2 = disp_node2->NextSiblingElement();

	out_pelem3 = new TiXmlElement("D");
	out_pelem3->SetAttribute("displ", disp_node2->Attribute("displ"));
	out_pelem3->SetAttribute("default_displ", disp_node2->Attribute("default_displ"));
	out_pelem3->SetAttribute("reliability", disp_node2->Attribute("reliability"));
	out_pelem3->SetAttribute("nccPeak", disp_node2->Attribute("nccPeak"));
	out_pelem3->SetAttribute("nccWidth", disp_node2->Attribute("nccWidth"));
	out_pelem3->SetAttribute("nccWRangeThr", disp_node2->Attribute("nccWRangeThr"));
	out_pelem3->SetAttribute("nccInvWidth", disp_node2->Attribute("nccInvWidth"));
	out_pelem3->SetAttribute("delay", disp_node2->Attribute("delay"));
	out_pelem2->LinkEndChild(out_pelem3);

	out_pelem->LinkEndChild(out_pelem2);
}


void XMLDisplacementBag::mergeTileGroups ( const char *xml_out_file ) {

	int i, j, k;

	// LOCAL VARIABLES FOR INPUT
	char xml_filepath[2000];
	TiXmlDocument *xml;

	//LOCAL VARIABLES FOR OUTPUT
	TiXmlDocument out_xml;
	TiXmlElement * out_root;
	TiXmlElement * out_pelem;

	xml = new TiXmlDocument[nFiles];

	// open input files
	for ( i=0; i<nFiles; i++ ) {
		sprintf(xml_filepath,"%s/%s",main_dir,XMLFileNames[i]);
		try {
			if(!xml[i].LoadFile(xml_filepath)) {
				char errMsg[2000];
				sprintf(errMsg,"in XMLDisplacementBag::mergeTileGroups() : unable to load xml %s", xml_filepath);
				throw iom::exception(errMsg);
			}
		}
		catch( iom::exception& exception){
			char errMsg[2000];
			sprintf(errMsg,"in XMLDisplacementBag::mergeTileGroups() : unable to load xml %s (cause: %s)", xml_filepath, exception.what());
			throw iom::exception(errMsg);
		}
		catch(bad_exception& be){
			char errMsg[2000];
			sprintf(errMsg,"in XMLDisplacementBag::mergeTileGroups() : unable to load xml %s (cause: %s)", xml_filepath, be.what());
			throw iom::exception(errMsg);
		}
		catch(char* error){
			char errMsg[2000];
			sprintf(errMsg,"in XMLDisplacementBag::mergeTileGroups() : unable to load xml %s (cause: %s)", xml_filepath, error);
			throw iom::exception(errMsg);
		}
	}

	// initializing output XML file with DTD declaration
	fstream XML_FILE(xml_out_file, ios::out);
	XML_FILE<<"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"<<endl;
	XML_FILE<<"<!DOCTYPE TeraStitcher SYSTEM \"TeraStitcher.DTD\">"<<endl;
	XML_FILE.close();

	//loading previously initialized output XML file 
    if(!out_xml.LoadFile(xml_out_file))
	{
		char errMsg[5000];
		sprintf(errMsg, "in XMLDisplacementBag::mergeTileGroups(...) : unable to load xml file at \"%s\"", xml_out_file);
		throw iom::exception(errMsg);
	}

	// IN: setting ROOT element (that is the first child, i.e. <TeraStitcher> node)
	TiXmlHandle hRoot(xml[0].FirstChildElement("TeraStitcher"));

	// OUT: inserting root node <TeraStitcher> and children nodes
	out_root = new TiXmlElement("TeraStitcher");  
	out_xml.LinkEndChild( out_root );  
	out_root->SetAttribute("volume_format", hRoot.ToElement()->Attribute("volume_format"));

	// 2017-05-10. Giulio. ADDED 'input_plugin' attribute to <TeraStitcher> XML node
	if(hRoot.ToElement()->Attribute("input_plugin"))
		out_root->SetAttribute("input_plugin", hRoot.ToElement()->Attribute("input_plugin"));

	// IN: reading fields and checking coherence with metadata previously read from VM_BIN_METADATA_FILE_NAME
	TiXmlElement *pelem = hRoot.FirstChildElement("stacks_dir").Element();

	// OUT: 
	out_pelem = new TiXmlElement("stacks_dir");
	out_pelem->SetAttribute("value", pelem->Attribute("value"));
	out_root->LinkEndChild(out_pelem);

	// IN:
	if ( (pelem = hRoot.FirstChildElement("ref_sys").Element()) != 0 ) { // skip if not present (for compatibility with previous versions)
		out_pelem = new TiXmlElement("ref_sys");
		out_pelem->SetAttribute("ref1", pelem->Attribute("ref1"));
		out_pelem->SetAttribute("ref2", pelem->Attribute("ref2"));
		out_pelem->SetAttribute("ref3", pelem->Attribute("ref3"));
		out_root->LinkEndChild(out_pelem);
	}
	pelem = hRoot.FirstChildElement("voxel_dims").Element();
	out_pelem = new TiXmlElement("voxel_dims");
	out_pelem->SetAttribute("V", pelem->Attribute("V"));
	out_pelem->SetAttribute("H", pelem->Attribute("H"));
	out_pelem->SetAttribute("D", pelem->Attribute("D"));
	out_root->LinkEndChild(out_pelem);
	pelem = hRoot.FirstChildElement("origin").Element();
	out_pelem = new TiXmlElement("origin");
	out_pelem->SetAttribute("V", pelem->Attribute("V"));
	out_pelem->SetAttribute("H", pelem->Attribute("H"));
	out_pelem->SetAttribute("D", pelem->Attribute("D"));
	out_root->LinkEndChild(out_pelem);
	pelem = hRoot.FirstChildElement("mechanical_displacements").Element();
	out_pelem = new TiXmlElement("mechanical_displacements");
	out_pelem->SetAttribute("V", pelem->Attribute("V"));
	out_pelem->SetAttribute("H", pelem->Attribute("H"));
	out_root->LinkEndChild(out_pelem);

	int N_ROWS;
	int N_COLS;

	pelem = hRoot.FirstChildElement("dimensions").Element();
	pelem->QueryIntAttribute("stack_rows", &N_ROWS);
	pelem->QueryIntAttribute("stack_columns", &N_COLS);
	out_pelem = new TiXmlElement("dimensions");
	out_pelem->SetAttribute("stack_rows", pelem->Attribute("stack_rows"));
	out_pelem->SetAttribute("stack_columns", pelem->Attribute("stack_columns"));
	out_pelem->SetAttribute("stack_slices", pelem->Attribute("stack_slices"));
	out_root->LinkEndChild(out_pelem);

	TiXmlHandle  **hRootList = new TiXmlHandle *[nFiles];
	TiXmlElement **pelemList = new TiXmlElement *[nFiles];
	TiXmlElement  *out_pelem2;

	for ( i=0; i<nFiles; i++ ) {
		hRootList[i] = new TiXmlHandle(xml[i].FirstChildElement("TeraStitcher"));
		pelemList[i] = hRootList[i]->FirstChildElement("STACKS").Element()->FirstChildElement();
	}

	out_pelem = new TiXmlElement("STACKS");

	pelem = hRoot.FirstChildElement("STACKS").Element()->FirstChildElement();

	for(i=0; i<N_ROWS; i++) {
		for(j=0; j<N_COLS; j++, pelem=pelem->NextSiblingElement()) {

			const char *attrib;
			out_pelem2 = new TiXmlElement("Stack");
			//out_pelem2->SetAttribute("N_BLOCKS", pelem->Attribute("N_BLOCKS"));
			attrib = pelem->Attribute("N_BLOCKS");
			if ( attrib ) {
			// is a tiled format: the attribute has to be added
				out_pelem2->SetAttribute("N_BLOCKS", attrib);
			}			
			//out_pelem2->SetAttribute("BLOCK_SIZES", pelem->Attribute("BLOCK_SIZES"));
			attrib = pelem->Attribute("BLOCK_SIZES");
			if ( attrib ) {
			// is a tiled format: the attribute has to be added
				out_pelem2->SetAttribute("BLOCK_SIZES", attrib);
			}			
			//out_pelem2->SetAttribute("BLOCKS_ABS_D", pelem->Attribute("BLOCKS_ABS_D"));
			attrib = pelem->Attribute("BLOCKS_ABS_D");
			if ( attrib ) {
			// is a tiled format: the attribute has to be added
				out_pelem2->SetAttribute("BLOCKS_ABS_D", attrib);
			}			
			out_pelem2->SetAttribute("N_CHANS", pelem->Attribute("N_CHANS"));
			out_pelem2->SetAttribute("N_BYTESxCHAN", pelem->Attribute("N_BYTESxCHAN"));
			out_pelem2->SetAttribute("ROW", pelem->Attribute("ROW"));
			out_pelem2->SetAttribute("COL", pelem->Attribute("COL"));
			out_pelem2->SetAttribute("ABS_V", pelem->Attribute("ABS_V"));
			out_pelem2->SetAttribute("ABS_H", pelem->Attribute("ABS_H"));
			out_pelem2->SetAttribute("ABS_D", pelem->Attribute("ABS_D"));
			out_pelem2->SetAttribute("STITCHABLE", pelem->Attribute("STITCHABLE"));
			out_pelem2->SetAttribute("DIR_NAME", pelem->Attribute("DIR_NAME"));
			//out_pelem2->SetAttribute("Z_RANGES", pelem->Attribute("Z_RANGES"));
			attrib = pelem->Attribute("Z_RANGES");
			if ( attrib ) {
			// is a stacked format: the attribute has to be added
				out_pelem2->SetAttribute("Z_RANGES", attrib);
			}			
			//out_pelem2->SetAttribute("IMG_REGEX", pelem->Attribute("IMG_REGEX"));
			attrib = pelem->Attribute("IMG_REGEX");
			if ( attrib ) {
			// is a stacked format: the attribute has to be added
				out_pelem2->SetAttribute("IMG_REGEX", attrib);
			}			

			mergeStack2(pelemList,out_pelem2);

			out_pelem->LinkEndChild(out_pelem2);

			for ( k=0; k<nFiles; k++) 
				pelemList[k] = pelemList[k]->NextSiblingElement();
		}
	}

	out_root->LinkEndChild(out_pelem);

	//saving the file
	out_xml.SaveFile();
}


void XMLDisplacementBag::mergeStack2 ( TiXmlElement **stack_nodes, TiXmlElement *out_pelem ) {

	TiXmlElement *out_pelem2;
	//TiXmlElement *out_pelem3;
	//TiXmlElement *out_pelem4;
	bool found;

	out_pelem2 = new TiXmlElement("NORTH_displacements");
	found = false;
	for ( int k=0; k<nFiles && !found; k++) {
		TiXmlElement *NORTH_displacements = stack_nodes[k]->FirstChildElement("NORTH_displacements");
		if ( !NORTH_displacements->NoChildren() ) {
			for(TiXmlElement *displ_node = NORTH_displacements->FirstChildElement("Displacement"); displ_node; displ_node = displ_node->NextSiblingElement()) {
				mergeDisp(displ_node,out_pelem2);
			}
			found = true; // just one list of alignments
		}
	}
	if ( !found ) 
	{
		char errMsg[5000];
		sprintf(errMsg, "in XMLDisplacementBag::mergeStack2(...) : there are no NORTH alignments");
		//throw iom::exception(errMsg);
	}
	out_pelem->LinkEndChild(out_pelem2);

	out_pelem2 = new TiXmlElement("EAST_displacements");
	found = false;
	for ( int k=0; k<nFiles && !found; k++) {
		TiXmlElement *EAST_displacements = stack_nodes[k]->FirstChildElement("EAST_displacements");
		if ( !EAST_displacements->NoChildren() ) {
			for(TiXmlElement *displ_node = EAST_displacements->FirstChildElement("Displacement"); displ_node; displ_node = displ_node->NextSiblingElement()) {
				mergeDisp(displ_node,out_pelem2);
			}
			found = true; // just one list of alignments
		}
	}
	if ( !found ) 
	{
		char errMsg[5000];
		sprintf(errMsg, "in XMLDisplacementBag::mergeStack2(...) : there are no EAST alignments");
		//throw iom::exception(errMsg);
	}
	out_pelem->LinkEndChild(out_pelem2);

	out_pelem2 = new TiXmlElement("SOUTH_displacements");
	found = false;
	for ( int k=0; k<nFiles && !found; k++) {
		TiXmlElement *SOUTH_displacements = stack_nodes[k]->FirstChildElement("SOUTH_displacements");
		if ( !SOUTH_displacements->NoChildren() ) {
			for(TiXmlElement *displ_node = SOUTH_displacements->FirstChildElement("Displacement"); displ_node; displ_node = displ_node->NextSiblingElement()) {
				mergeDisp(displ_node,out_pelem2);
			}
			found = true; // just one list of alignments
		}
	}
	if ( !found ) 
	{
		char errMsg[5000];
		sprintf(errMsg, "in XMLDisplacementBag::mergeStack2(...) : there are no SOUTH alignments");
		//throw iom::exception(errMsg);
	}
	out_pelem->LinkEndChild(out_pelem2);

	out_pelem2 = new TiXmlElement("WEST_displacements");
	found = false;
	for ( int k=0; k<nFiles && !found; k++) {
		TiXmlElement *WEST_displacements = stack_nodes[k]->FirstChildElement("WEST_displacements");
		if ( !WEST_displacements->NoChildren() ) {
			for(TiXmlElement *displ_node = WEST_displacements->FirstChildElement("Displacement"); displ_node; displ_node = displ_node->NextSiblingElement()) {
				mergeDisp(displ_node,out_pelem2);
			}
			found = true; // just one list of alignments
		}
	}
	if ( !found ) 
	{
		char errMsg[5000];
		sprintf(errMsg, "in XMLDisplacementBag::mergeStack2(...) : there are no WEST alignments");
		//throw iom::exception(errMsg);
	}
	out_pelem->LinkEndChild(out_pelem2);
}


