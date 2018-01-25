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
* 2017-04-01.  Giulio.     @ADDED code for completing multi-layer management
* 2017-02-10.  Giulio.     @CREATED 
*/


#include "MultiLayers.h"
#include "VirtualVolume.h"
#include "TiledVolume.h"
#include "UnstitchedVolume.h"

#include "DisplacementMIPNCC.h"

#include "volumemanager.config.h"

#include <fstream>
#include <sstream>
#include <list>
#include <string.h>

#ifdef _WIN32
#include "dirent_win.h"
#else
#include <dirent.h>
#endif

using namespace std;
using namespace iim;

MultiLayersVolume::MultiLayersVolume ( string _layers_dir, float _cut_depth, float _norm_factor_D ) {

	int offs;

	layers_dir = new char[_layers_dir.size() + 1];
	strcpy(layers_dir,_layers_dir.c_str());

	init();

    VXL_V = LAYERS[0]->getVXL_V();
	VXL_H = LAYERS[0]->getVXL_H();
	VXL_D = LAYERS[0]->getVXL_D();		 
    ORG_V = LAYERS[0]->getORG_V();
	ORG_H = LAYERS[0]->getORG_H();
	ORG_D = LAYERS[0]->getORG_D();	

	DIM_V = LAYERS[0]->getDIM_V();
	for ( int i=1; i<N_LAYERS; i++ ) {
		DIM_V = ((LAYERS[i]->getDIM_V() > (int)DIM_V) ? LAYERS[i]->getDIM_V() : DIM_V);
	}
	DIM_H = LAYERS[0]->getDIM_H();
	for ( int i=1; i<N_LAYERS; i++ ) {
		DIM_H = (iim::uint32) ((LAYERS[i]->getDIM_H() > (int)DIM_H) ? LAYERS[i]->getDIM_H() : DIM_H);
	}

	normal_factor_D = _norm_factor_D;
	cut_depth = _cut_depth;

	// compute the nominal dimension and nominal displacements along D
	// compute also nominal coords of layers
	layers_coords = new VHD_coords[N_LAYERS];
	nominal_D_overlap = new int[N_LAYERS-1];

	memset(layers_coords,0,N_LAYERS*sizeof(VHD_coords));
	DIM_D = 0;
	for ( int i=0; i<(N_LAYERS - 1); i++ ) {
		offs = (int) ROUND((LAYERS[i+1]->getORG_D() - LAYERS[i]->getORG_D()) * 1000.0F / VXL_D);
		DIM_D += offs;
		layers_coords[i+1][2] = DIM_D; // other nominal coords are 0
		nominal_D_overlap[i] = LAYERS[i]->getDIM_D() - offs; // nominal overlap may become negative if offset is too large
	}
	DIM_D += LAYERS[N_LAYERS-1]->getDIM_D();

	disps = new vector< vector<Displacement *> > *[N_LAYERS-1];
	for ( int i=0; i<(N_LAYERS - 1); i++ ) {
		disps[i] = (vector< vector<Displacement *> > *) 0;
	}

    DIM_C = LAYERS[0]->getDIM_C();	
    BYTESxCHAN = LAYERS[0]->getBYTESxCHAN();  

	layers_new_xml_fnames = (std::string *) 0;
}

MultiLayersVolume::MultiLayersVolume ( const char *xml_filepath ) {
    //extracting <stacks_dir> field from XML
    TiXmlDocument xml;
    if(!xml.LoadFile(xml_filepath))
    {
        char errMsg[2000];
        sprintf(errMsg,"in MultiLayersVolume::MultiLayersVolume(_layers_dir = \"%s\") : unable to load xml", xml_filepath);
        throw IOException(errMsg);
    }
    TiXmlHandle hRoot(xml.FirstChildElement("TeraStitcher2"));
    TiXmlElement * pelem = hRoot.FirstChildElement("layers_dir").Element();
    this->layers_dir = new char[strlen(pelem->Attribute("value"))+1];
    strcpy(this->layers_dir, pelem->Attribute("value"));
	xml.Clear();

	// load xml content and generate mdata.bin
	initFromXML(xml_filepath);

	layers_new_xml_fnames = (std::string *) 0;
}

MultiLayersVolume::~MultiLayersVolume ( ) {
	if ( LAYERS ) {
		for ( int i=0; i<N_LAYERS; i++ ) 
			if ( LAYERS[i] )
				delete LAYERS[i];
		delete []LAYERS;
	}

	if ( layers_coords )
		delete []layers_coords;

	if ( nominal_D_overlap )
		delete []nominal_D_overlap;

	if ( disps ) {
		for ( int i=0; i<(N_LAYERS - 1); i++ ) 
			if ( disps[i] )
				delete disps[i];
		delete disps;
	}

	if ( layers_new_xml_fnames )
		delete []layers_new_xml_fnames;
}

void MultiLayersVolume::init ( ) {

	/**/iim::debug(iim::LEV3, 0, __iim__current__function__);

	//LOCAL VARIABLES
	string tmp_path;				//string that contains temp paths during computation
    string tmp;						//string that contains temp data during computation
	DIR *cur_dir_lev1;				//pointer to DIR, the data structure that represents a DIRECTORY (level 1 of hierarchical structure)
	DIR *cur_dir_lev2;				//pointer to DIR, the data structure that represents a DIRECTORY (level 2 of hierarchical structure)
	dirent *entry_lev1;				//pointer to DIRENT, the data structure that represents a DIRECTORY ENTRY inside a directory (level 1)
	dirent *entry_lev2;				//pointer to DIRENT, the data structure that represents a DIRECTORY ENTRY inside a directory (level 2)
	int i=0, j=0;					//for counting of N_ROWS, N_COLS
 //   list<Block*> blocks_list;                       //each stack found in the hierarchy is pushed into this list
    list<string> entries_lev1;                      //list of entries of first level of hierarchy
    list<string>::iterator entry_i;                 //iterator for list 'entries_lev1'
    list<string> entries_lev2;                      //list of entries of second level of hierarchy
    list<string>::iterator entry_j;                 //iterator for list 'entries_lev2'
    char block_i_j_path[STATIC_STRINGS_SIZE];

	//obtaining DIR pointer to root_dir (=NULL if directory doesn't exist)
	if (!(cur_dir_lev1=opendir(layers_dir)))
	{
        char msg[STATIC_STRINGS_SIZE];
        sprintf(msg,"in MultiLayersVolume::init(...): Unable to open directory \"%s\"", layers_dir);
        throw IOException(msg);
	}

	//scanning first level of hierarchy which entries need to be ordered alphabetically. This is done using STL.
	while ((entry_lev1=readdir(cur_dir_lev1)))
	{
        tmp=entry_lev1->d_name;
        if((tmp.find(".") == string::npos && tmp.find(" ") == string::npos) || (tmp.find(".xml") != string::npos))
                entries_lev1.push_front(entry_lev1->d_name);
	}
	closedir(cur_dir_lev1);
	entries_lev1.sort();

	N_LAYERS = (int) entries_lev1.size();
	LAYERS = new VirtualVolume *[N_LAYERS];

	// check if volumes are stitiched on unstitched
	if ( strstr(entries_lev1.front().c_str(),".xml") == 0 ) { // volumes are stitched

		//for each entry creates a VirtualVolume
		for(entry_i = entries_lev1.begin(), i=0; entry_i!= entries_lev1.end(); entry_i++, i++)
		{
			//building absolute path of first level entry
			tmp_path=layers_dir;
			tmp_path.append("/");
			tmp_path.append(*entry_i);
			cur_dir_lev2 = opendir(tmp_path.c_str());
			if (!cur_dir_lev2)
				throw IOException("in MultiLayersVolume::init(...): A problem occurred during scanning of subdirectories");

			//scanning second level of hierarchy, actuallt just one entry ("RES(...)" directory)
			while ((entry_lev2=readdir(cur_dir_lev2)))
			{
				tmp=entry_lev2->d_name;
				if(tmp.find(".") == string::npos && tmp.find(" ") == string::npos)
					entries_lev2.push_back(entry_lev2->d_name);
			}
			closedir(cur_dir_lev2);
			entries_lev2.sort();

			//for each entry of the second level, allocating a new Block
			for(entry_j = entries_lev2.begin(), j=0; entry_j!= entries_lev2.end(); entry_j++, j++)
			{
				//allocating new layer
				sprintf(block_i_j_path,"%s/%s/%s",layers_dir,(*entry_i).c_str(), (*entry_j).c_str());
				LAYERS[i] = new TiledVolume(block_i_j_path); //,iim::ref_sys(iim::axis(1),iim::axis(2),iim::axis(3)),(float)1.0,(float)1.0,(float)1.0);
			}
			entries_lev2.clear();
		
		}
		reference_system = ((TiledVolume *) LAYERS[0])->getREF_SYS();
	}
	else { // volumes are unstitched
		//for each entry creates a VirtualVolume
		for(entry_i = entries_lev1.begin(), i=0; entry_i!= entries_lev1.end(); entry_i++, i++) {
			//building absolute path of first level entry
			tmp_path=layers_dir;
			tmp_path.append("/");
			tmp_path.append(*entry_i);
			LAYERS[i] = new UnstitchedVolume(tmp_path.c_str()); 
		}
		reference_system = ((UnstitchedVolume *) LAYERS[0])->getREF_SYS();
	}

	entries_lev1.clear();
}


void MultiLayersVolume::updateLayerCoords ( ) {

	int offs;

	DIM_V = LAYERS[0]->getDIM_V();
	for ( int i=1; i<N_LAYERS; i++ ) {
		DIM_V = (iim::uint32) ((LAYERS[i]->getDIM_V() > (int)DIM_V) ? LAYERS[i]->getDIM_V() : DIM_V);
	}
	DIM_H = LAYERS[0]->getDIM_H();
	for ( int i=1; i<N_LAYERS; i++ ) {
		DIM_H = (iim::uint32) ((LAYERS[i]->getDIM_H() > (int)DIM_H) ? LAYERS[i]->getDIM_H() : DIM_H);
	}

	DIM_D = 0;
	for ( int i=0; i<(N_LAYERS - 1); i++ ) {
		offs = (int) ROUND((LAYERS[i+1]->getORG_D() - LAYERS[i]->getORG_D()) * 1000.0F / VXL_D);
		DIM_D += offs;
		layers_coords[i+1][2] = DIM_D; // other nominal coords are 0
		nominal_D_overlap[i] = LAYERS[i]->getDIM_D() - offs; // nominal overlap may become negative if offset is too large
	}
	DIM_D += LAYERS[N_LAYERS-1]->getDIM_D();
}

int	MultiLayersVolume::getLAYER_DIM(int i, int j) {
	if ( j==0 )
		return LAYERS[i]->getDIM_V();
	else if ( j==1 )
		return LAYERS[i]->getDIM_H();
	else if ( j==2 )
		return LAYERS[i]->getDIM_D();
	else
        throw IOException("in MultiLayersVolume::getLAYER_DIM(...): invalid direction");
}


iim::real32*	MultiLayersVolume::getSUBVOL(int i, int V0, int V1, int H0, int H1, int D0, int D1) {
	return LAYERS[i]->loadSubvolume_to_real32(V0,V1,H0,H1,D0,D1);
}


void MultiLayersVolume::initFromXML(const char *xml_filename) throw (IOException) {
	#if VM_VERBOSE > 3
    printf("\t\t\t\tin MultiLayersVolume::initFromXML(char *xml_filename = %s)\n", xml_filepath);
	#endif

	TiXmlDocument xml;
	if(!xml.LoadFile(xml_filename))
	{
		char errMsg[2000];
		sprintf(errMsg,"in MultiLayersVolume::initFromXML(xml_filepath = \"%s\") : unable to load xml", xml_filename);
		throw IOException(errMsg);
	}

	//setting ROOT element (that is the first child, i.e. <TeraStitcher> node)
	TiXmlHandle hRoot(xml.FirstChildElement("TeraStitcher2"));

	//reading fields
	TiXmlElement * pelem = hRoot.FirstChildElement("layers_dir").Element();
	pelem = hRoot.FirstChildElement("voxel_dims").Element();
	pelem->QueryFloatAttribute("V", &VXL_V);
	pelem->QueryFloatAttribute("H", &VXL_H);
	pelem->QueryFloatAttribute("D", &VXL_D);
	pelem->QueryFloatAttribute("norm_factor_D", &normal_factor_D);
	pelem->QueryFloatAttribute("cut_depth", &cut_depth);
	pelem = hRoot.FirstChildElement("origin").Element();
	pelem->QueryFloatAttribute("V", &ORG_V);
	pelem->QueryFloatAttribute("H", &ORG_H);
	pelem->QueryFloatAttribute("D", &ORG_D);
	pelem = hRoot.FirstChildElement("dimensions").Element();
	int dim_v, dim_h, dim_d;
	pelem->QueryIntAttribute("DIM_V", &dim_v);
	pelem->QueryIntAttribute("DIM_H", &dim_h);
	pelem->QueryIntAttribute("DIM_D", &dim_d);
	DIM_V = dim_v;
	DIM_H = dim_h;
	DIM_D = dim_d;
	pelem->QueryIntAttribute("DIM_C", &DIM_C);
	pelem->QueryIntAttribute("BYTESxCHAN", &BYTESxCHAN);
	pelem = hRoot.FirstChildElement("ref_sys").Element();
	pelem->QueryIntAttribute("ref1", (int *) &reference_system.first);
	pelem->QueryIntAttribute("ref2", (int *) &reference_system.second);
	pelem->QueryIntAttribute("ref3", (int *) &reference_system.third);

	pelem = hRoot.FirstChildElement("LAYERS").Element();
	pelem->QueryIntAttribute("value", &N_LAYERS);
	LAYERS = new VirtualVolume *[N_LAYERS];
	layers_coords = new VHD_coords[N_LAYERS];
	
	pelem = pelem->FirstChildElement();
	int index;
	char *volume_dir;
	for(int i = 0; i < N_LAYERS; i++)
	{
		pelem->QueryIntAttribute("INDEX", &index);
		volume_dir = new char[strlen(pelem->Attribute("value"))+1];
		strcpy(volume_dir, pelem->Attribute("value"));

		// check if volumes are stitiched on unstitched
		if ( strstr(volume_dir,".xml") == 0 ) { // volumes are stitched
			LAYERS[index] = new TiledVolume(volume_dir);
			reference_system = ((TiledVolume *) LAYERS[0])->getREF_SYS();
		}
		else { // volumes are unstitched
			LAYERS[index] = new UnstitchedVolume(volume_dir);
			reference_system = ((UnstitchedVolume *) LAYERS[0])->getREF_SYS();
		}
		pelem->QueryIntAttribute("coord_V", &layers_coords[i][0]);
		pelem->QueryIntAttribute("coord_H", &layers_coords[i][1]);
		pelem->QueryIntAttribute("coord_D", &layers_coords[i][2]);
		pelem = pelem->NextSiblingElement();
	}

	pelem = hRoot.FirstChildElement("INTER_LAYERS").Element();
	int dummy_val;
	pelem->QueryIntAttribute("value", &dummy_val); // just to consume the value
	nominal_D_overlap = new int[N_LAYERS - 1];
	disps = new vector< vector<Displacement *> > *[N_LAYERS-1];
	for ( int i=0; i<(N_LAYERS - 1); i++ ) {
		disps[i] = (vector< vector<Displacement *> > *) 0;
	}

	pelem = pelem->FirstChildElement();
	for(int i = 0; i < (N_LAYERS - 1); i++)
	{
		pelem->QueryIntAttribute("INDEX", &index);
		pelem->QueryIntAttribute("nominal_overlap", &nominal_D_overlap[index]);
		if ( strcmp(pelem->Attribute("disps"),"yes") == 0 ) {
			int dim_i, dim_j;
			pelem->QueryIntAttribute("dim_i", &dim_i);
			pelem->QueryIntAttribute("dim_j", &dim_j);
			disps[i] = new vector< vector<Displacement *> >(dim_i,vector<Displacement *>(dim_j,(Displacement *) 0));
			TiXmlElement *pelem2 = pelem->FirstChildElement("tile"); // skip indices
			for (int ii=0; ii<dim_i; ii++) {
				for (int jj=0; jj<dim_j; jj++) {
					pelem2->QueryIntAttribute("i", &dummy_val);
					pelem2->QueryIntAttribute("j", &dummy_val);
					TiXmlElement *pelem3 = pelem2->FirstChildElement("Displacement");
					disps[i]->at(ii).at(jj) = Displacement::getDisplacementFromXML(pelem3);
					pelem2 = pelem2->NextSiblingElement("tile"); // next indices 
				}
			}
		}

		pelem = pelem->NextSiblingElement();
	}
}


void MultiLayersVolume::saveXML(const char *xml_filename, const char *xml_filepath) throw (IOException) {
	#if VM_VERBOSE > 3
	printf("\t\t\t\tin MultiLayersVolume::saveXML(char *xml_filename = %s)\n", xml_filename);
	#endif

	//LOCAL VARIABLES
    char xml_abs_path[STATIC_STRINGS_SIZE];
	TiXmlDocument xml;
	TiXmlElement * root;
	TiXmlElement * pelem;
	int i;
	int ii,jj;

	//obtaining XML absolute path
	if(xml_filename)
		sprintf(xml_abs_path, "%s/%s.xml", layers_dir, xml_filename);
	else if(xml_filepath)
		strcpy(xml_abs_path, xml_filepath);
	else
		throw IOException("in MultiLayersVolume::saveXML(...): no xml path provided");

	//initializing XML file with DTD declaration
	fstream XML_FILE(xml_abs_path, ios::out);
	XML_FILE<<"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"<<endl;
	XML_FILE<<"<!DOCTYPE TeraStitcher2 SYSTEM \"TeraStitcher2.DTD\">"<<endl;
	XML_FILE.close();

	//loading previously initialized XML file 
	if(!xml.LoadFile(xml_abs_path))
	{
		char errMsg[5000];
		sprintf(errMsg, "in MultiLayersVolume::saveToXML(...) : unable to load xml file at \"%s\"", xml_abs_path);
		throw IOException(errMsg);
	}

	//inserting root node <TeraStitcher2> and children nodes
	root = new TiXmlElement("TeraStitcher2");  
	xml.LinkEndChild( root );  
	pelem = new TiXmlElement("layers_dir");
	pelem->SetAttribute("value", layers_dir);
	root->LinkEndChild(pelem);
	pelem = new TiXmlElement("voxel_dims");
	pelem->SetDoubleAttribute("V", VXL_V);
	pelem->SetDoubleAttribute("H", VXL_H);
	pelem->SetDoubleAttribute("D", VXL_D);
	pelem->SetDoubleAttribute("norm_factor_D", normal_factor_D);
	pelem->SetDoubleAttribute("cut_depth", cut_depth);
	root->LinkEndChild(pelem);
	pelem = new TiXmlElement("origin");
	pelem->SetDoubleAttribute("V", ORG_V);
	pelem->SetDoubleAttribute("H", ORG_H);
	pelem->SetDoubleAttribute("D", ORG_D);
	root->LinkEndChild(pelem);
	pelem = new TiXmlElement("dimensions");
	pelem->SetAttribute("DIM_V", DIM_V);
	pelem->SetAttribute("DIM_H", DIM_H);
	pelem->SetAttribute("DIM_D", DIM_D);
	pelem->SetAttribute("DIM_C", DIM_C);
	pelem->SetAttribute("BYTESxCHAN", BYTESxCHAN);
	root->LinkEndChild(pelem);
	pelem = new TiXmlElement("ref_sys");
	pelem->SetAttribute("ref1", reference_system.first);
	pelem->SetAttribute("ref2", reference_system.second);
	pelem->SetAttribute("ref3", reference_system.third);
	root->LinkEndChild(pelem);

	//inserting layers nodes
	pelem = new TiXmlElement("LAYERS");
	pelem->SetAttribute("value", N_LAYERS);
	for(i=0; i<N_LAYERS; i++) {
		TiXmlElement * pelem2 = new TiXmlElement("LAYER");
		pelem2->SetAttribute("INDEX", i);
		if ( layers_new_xml_fnames ) 
			// use new xml files associated to layers have been generated
			pelem2->SetAttribute("value", layers_new_xml_fnames[i].c_str());
		else 
			pelem2->SetAttribute("value", LAYERS[i]->getROOT_DIR());
		pelem2->SetAttribute("coord_V", layers_coords[i][0]);
		pelem2->SetAttribute("coord_H", layers_coords[i][1]);
		pelem2->SetAttribute("coord_D", layers_coords[i][2]);
		pelem->LinkEndChild(pelem2);
	}
	root->LinkEndChild(pelem);

	//inserting inter-layer information (overlap, displacements)
	pelem = new TiXmlElement("INTER_LAYERS");
	pelem->SetAttribute("value", N_LAYERS-1);
	for(i=0; i<(N_LAYERS - 1); i++) {
		TiXmlElement * pelem2 = new TiXmlElement("INTER_LAYER_INFO");
		pelem2->SetAttribute("INDEX", i);
		pelem2->SetAttribute("nominal_overlap", nominal_D_overlap[i]);

		if ( disps[i] ) {
			for (ii=0; ii<disps[i]->size(); ii++ ) {
				for (jj=0; jj<disps[i]->at(ii).size(); jj++ ) {
					TiXmlElement * pelem3 = new TiXmlElement("tile");
					pelem3->SetAttribute("i", ii);
					pelem3->SetAttribute("j", jj);
					pelem3->LinkEndChild(disps[i]->at(ii).at(jj)->getXML());
					//pelem3->SetAttribute("disp_V", disps[i]->at(ii).at(jj)->getDisplacement(dir_vertical));
					//pelem3->SetAttribute("disp_H", disps[i]->at(ii).at(jj)->getDisplacement(dir_horizontal));
					//pelem3->SetAttribute("disp_D", disps[i]->at(ii).at(jj)->getDisplacement(dir_depth));
					pelem2->LinkEndChild(pelem3);
				}
			}
			pelem2->SetAttribute("disps", "yes");
			pelem2->SetAttribute("dim_i", (int)disps[i]->size());
			pelem2->SetAttribute("dim_j", (int)disps[i]->at(0).size()); // all rows of tiles have the same number of tiles
		}
		else
			pelem2->SetAttribute("disps", "no");

		pelem->LinkEndChild(pelem2);
	}
	root->LinkEndChild(pelem);

	//saving the file
	xml.SaveFile();
}


void MultiLayersVolume::saveLayersXML(const char *xml_filename, const char *xml_filepath) throw (IOException) {
	#if VM_VERBOSE > 3
	printf("\t\t\t\tin MultiLayersVolume::saveLayersXML(char *xml_filename = %s)\n", xml_filename);
	#endif

	//LOCAL VARIABLES
    std::string xml_base_abs_path = "";

	//obtaining XML absolute path
	if(xml_filename)
		xml_base_abs_path = xml_base_abs_path + layers_dir + xml_filename;
	else if(xml_filepath)
		xml_base_abs_path = xml_filepath;
	else
		throw IOException("in MultiLayersVolume::saveLayersXML(...): no xml path provided");

	// eliminate suffix if any
	if ( xml_base_abs_path.find(".xml") == (xml_base_abs_path.size() - 4) )
		//for ( int i=0; i<4; i++ )
		//	xml_base_abs_path.pop_back();
		xml_base_abs_path = xml_base_abs_path.substr(0,xml_base_abs_path.size() - 4);

	// computing channel directory names
	int n_digits = 1;
	int _N_LAYERS = N_LAYERS / 10;	
	while ( _N_LAYERS ) {
		n_digits++;
		_N_LAYERS /= 10;
	}

	// create list of new xml file names associated to layers
	layers_new_xml_fnames = new std::string [N_LAYERS];

	for ( int i=0; i<N_LAYERS; i++ ) {
		std::stringstream xmlfile_num;
		xmlfile_num.width(n_digits);
		xmlfile_num.fill('0');
		xmlfile_num << i;
		layers_new_xml_fnames[i] = xml_base_abs_path + "L" + xmlfile_num.str() + ".xml";
		((UnstitchedVolume *) LAYERS[i])->volume->saveXML(0,layers_new_xml_fnames[i].c_str());
	}
}


void MultiLayersVolume::initDISPS(int i, int _DIM_V, int _DIM_H) {
	if ( disps[i] != ((vector< vector<Displacement *> > *) 0) )
		delete disps[i];

	disps[i] = new vector< vector<Displacement *> >(_DIM_V,vector<Displacement *>(_DIM_H,(Displacement *) 0));
}


void MultiLayersVolume::insertDisplacement(int i, int j, int k, Displacement *displacement) throw (IOException) {

	displacement->evalReliability(dir_vertical);
	displacement->evalReliability(dir_horizontal);
	displacement->evalReliability(dir_depth);

	// we assume that by default corresponding tiles in adjacent layers are aligned with respect to motorized stages coordinates 
	displacement->setDefaultV(0);	
	displacement->setDefaultH(0);
	displacement->setDefaultD(0);

	if ( disps[k]->at(i).at(j) ) 
		delete disps[k]->at(i).at(j);

	disps[k]->at(i).at(j) = displacement;
}

