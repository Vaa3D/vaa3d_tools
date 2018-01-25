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
*       Bria, A., Iannello, G., "TeraStitcher - A Tool for Fast 3D Automatic Stitching of Teravoxel-sized Microscopy Images", submitted for publication, 2012.
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

#include "TemplateCLI.h"
#include "GUI_config.h"
#include <CmdLine.h>
#include <sstream>

#include "IM_config.h"
#include "config.h"

using namespace iim;

TemplateCLI::TemplateCLI(void)
{
	// we use TCLAP default values to initialize most parameters (see readParams)

	// initialize other parameters that are not provided in the command line
	;
}

//reads options and parameters from command line
void TemplateCLI::readParams(int argc, char** argv) throw (iom::exception)
{
	//command line object definition
	TCLAP::CmdLine cmd(getHelpText(), '=', terastitcher::mdatagenerator_version);
		/**
		 * Command line constructor. Defines how the arguments will be
		 * parsed.
		 * \param message - The message to be used in the usage
		 * output.
		 * \param delimiter - The character that is used to separate
		 * the argument flag/name from the value.  Defaults to ' ' (space).
		 * \param version - The version number to be used in the
		 * --version switch.
		 * \param helpAndVersion - Whether or not to create the Help and
		 * Version switches. Defaults to true.
		 */


	//argument objects definitions
	TCLAP::SwitchArg p_overwrite_mdata("","ovrw","Overwrite data.",false); 
 	TCLAP::SwitchArg p_update_mdata("","update","Overwrite data.",false); 
       /**
		 * SwitchArg constructor.
		 * \param flag - The one character flag that identifies this
		 * argument on the command line.
		 * \param name - A one word name for the argument.  Can be
		 * used as a long flag on the command line.
		 * \param desc - A description of what the argument is for or
		 * does.
		 * \param def - The default value for this Switch. 
		 * \param v - An optional visitor.  You probably should not
		 * use this unless you have a very good reason.
		 */

	TCLAP::ValueArg<std::string> p_root_dir("r","rdir","Root directory path.",true,"","string");
	TCLAP::ValueArg<int> p_axis_V("","vax","Vertical axis (default=1).",false,1,"[-3,3]");
	TCLAP::ValueArg<int> p_axis_H("","hax","Horizontal axis (default=2).",false,2,"[-3,3]");
	TCLAP::ValueArg<int> p_axis_D("","dax","Depth axis (default=3).",false,3,"[-3,3]");
	TCLAP::ValueArg<float> p_vxl_size("s","size","Voxel size (the same in all directions, default=1.0).",false,(float)1.0,"real");
	TCLAP::ValueArg<float> p_vxl_size_V("","szV","Voxel vertical size (default=1.0).",false,(float)1.0,"real");
	TCLAP::ValueArg<float> p_vxl_size_H("","szH","Voxel horizontal size (default=1.0).",false,(float)1.0,"real");
	TCLAP::ValueArg<float> p_vxl_size_D("","szD","Voxel depth size (default=1.0).",false,(float)1.0,"real");
	string temp = "Source format (\"" + 
		iim::TILED_MC_FORMAT + "\"/\"" + 
		iim::TILED_FORMAT + "\"/\"" + 
		iim::STACKED_FORMAT + "\"/\"" + 
		iim::SIMPLE_FORMAT + "\"/\"" + 
		iim::SIMPLE_RAW_FORMAT + "\"/\"" + 
		iim::RAW_FORMAT + "\"/\"" + 
		iim::TIF3D_FORMAT + "\"/\"" + 
		iim::TILED_TIF3D_FORMAT  + "\"/\"" +
		iim::TILED_MC_TIF3D_FORMAT  + "\"/\"" +
		iim::UNST_TIF3D_FORMAT  + "\")";
	TCLAP::ValueArg<string> p_src_format("","sfmt",temp.c_str(),true,"","string");

        /**
         * Labeled ValueArg constructor.
         * You could conceivably call this constructor with a blank flag, 
         * but that would make you a bad person.  It would also cause
         * an exception to be thrown.   If you want an unlabeled argument, 
         * use the other constructor.
         * \param flag - The one character flag that identifies this
         * argument on the command line.
         * \param name - A one word name for the argument.  Can be
         * used as a long flag on the command line.
         * \param desc - A description of what the argument is for or
         * does.
         * \param req - Whether the argument is required on the command
         * line.
         * \param value - The default value assigned to this argument if it
         * is not present on the command line.
         * \param typeDesc - A short, human readable description of the
         * type that this object expects.  This is used in the generation
         * of the USAGE statement.  The goal is to be helpful to the end user
         * of the program.
         * \param v - An optional visitor.  You probably should not
         * use this unless you have a very good reason.
         */


	//argument objects must be inserted using LIFO policy (last inserted, first shown)
	cmd.add(p_src_format);
	cmd.add(p_axis_D);
	cmd.add(p_axis_H);
	cmd.add(p_axis_V);
	cmd.add(p_vxl_size_D);
	cmd.add(p_vxl_size_H);
	cmd.add(p_vxl_size_V);
	cmd.add(p_vxl_size);
	cmd.add(p_update_mdata);
	cmd.add(p_overwrite_mdata);
	cmd.add(p_root_dir);

	// Parse the argv array and catch <TCLAP> exceptions, which are translated into <iim::IOException> exceptions
	char errMsg[S_STATIC_STRINGS_SIZE];
	try{ cmd.parse( argc, argv ); } 
	catch (TCLAP::ArgException &e)
	{ 
		sprintf(errMsg, "%s for arg %s\n", e.error().c_str(), e.argId().c_str());
		throw iom::exception(errMsg);
	}

	/* Checking parameter consistency */

	if( p_axis_V.isSet() || p_axis_H.isSet() || p_axis_D.isSet() ) { // there is at least one parameter
		if ( !p_axis_V.isSet() || !p_axis_H.isSet() || !p_axis_D.isSet() ) { // some parameter missing
			sprintf(errMsg, "Complete reference system must be specified!\nUSAGE is:\n\t--%s%c<%s>%c--%s%c<%s>%c--%s%c<%s>", 
				p_axis_V.getName().c_str(), cmd.getDelimiter(), "[-3,3]", cmd.getDelimiter(), 
				p_axis_H.getName().c_str(), cmd.getDelimiter(), "[-3,3]", cmd.getDelimiter(), 
				p_axis_D.getName().c_str(), cmd.getDelimiter(), "[-3,3]" );
			throw iom::exception(errMsg);
		}
	}

	if ( p_vxl_size.isSet() ) { // the same size for all dimensions is specified 
		if ( p_vxl_size_V.isSet() || p_vxl_size_H.isSet() || p_vxl_size_D.isSet() ) { // also single dimensions are specified 
			sprintf(errMsg, "Conflicting voxel size!\nUSAGE is either:\n\t--%s%c<%s>\nor:\n\t--%s%c<%s>%c--%s%c<%s>%c--%s%c<%s>", 
				p_vxl_size.getName().c_str(), cmd.getDelimiter(), "real", 
				p_vxl_size_V.getName().c_str(), cmd.getDelimiter(), "real", cmd.getDelimiter(), 
				p_vxl_size_H.getName().c_str(), cmd.getDelimiter(), "real", cmd.getDelimiter(), 
				p_vxl_size_D.getName().c_str(), cmd.getDelimiter(), "real" );
			throw iom::exception(errMsg);
		}
		else { // only the same size for all dimensions is specified
			this->vxlsz_V = this->vxlsz_H = this->vxlsz_D = p_vxl_size.getValue();
		}
	}
	else if ( p_vxl_size_V.isSet() || p_vxl_size_H.isSet() || p_vxl_size_D.isSet() ) { // different size specified for each dimension
		if ( !p_vxl_size_V.isSet() || !p_vxl_size_H.isSet() || !p_vxl_size_D.isSet() ) { // some parameter is missing
			sprintf(errMsg, "Voxel size missing in some dimension!\nUSAGE is:\n\t--%s%c<%s>%c--%s%c<%s>%c--%s%c<%s>", 
				p_vxl_size_V.getName().c_str(), cmd.getDelimiter(), "real", cmd.getDelimiter(), 
				p_vxl_size_H.getName().c_str(), cmd.getDelimiter(), "real", cmd.getDelimiter(), 
				p_vxl_size_D.getName().c_str(), cmd.getDelimiter(), "real" );
			throw iom::exception(errMsg);
		}
		else {
			this->vxlsz_V = p_vxl_size_V.getValue();
			this->vxlsz_H = p_vxl_size_H.getValue();
			this->vxlsz_D = p_vxl_size_D.getValue();
		}
	}

	/* Checking parameter consistency */
	if ( p_src_format.getValue() != iim::STACKED_FORMAT && 
		 p_src_format.getValue() != iim::SIMPLE_FORMAT  && 
		 p_src_format.getValue() != iim::SIMPLE_RAW_FORMAT  && 
		 p_src_format.getValue() != iim::RAW_FORMAT  && 
		 p_src_format.getValue() != iim::TILED_FORMAT  && 
		 p_src_format.getValue() != iim::TILED_MC_FORMAT &&
		 p_src_format.getValue() != iim::TIF3D_FORMAT  && 
		 p_src_format.getValue() != iim::TILED_TIF3D_FORMAT  && 
		 p_src_format.getValue() != iim::TILED_MC_TIF3D_FORMAT  && 
		 p_src_format.getValue() != iim::UNST_TIF3D_FORMAT ) {
		temp = "Unknown source format!\nAllowed formats are:\n\t\"" + 
			iim::TILED_MC_FORMAT + "\"/\"" + 
			iim::TILED_FORMAT + "\"/\"" + 
			iim::STACKED_FORMAT + "\"/\"" + 
			iim::SIMPLE_FORMAT + "\"/\"" + 
			iim::SIMPLE_RAW_FORMAT + "\"/\"" + 
			iim::RAW_FORMAT + "\"/\"" + 
			iim::TIF3D_FORMAT + "\"/\"" + 
			iim::TILED_TIF3D_FORMAT  + "\"/\"" +
			iim::TILED_MC_TIF3D_FORMAT  + "\"/\"" +
			iim::UNST_TIF3D_FORMAT  + "\"";
		//sprintf(errMsg, "Unknown source format!\nAllowed formats are:\n\tStacked / Simple / SimpeRaw / Raw / Tiled / TiledMC");
		sprintf(errMsg, "%s", temp.c_str());
		throw iom::exception(errMsg);
	}

	//importing parameters not set yet

	this->root_dir = p_root_dir.getValue();
	this->overwrite_mdata = p_overwrite_mdata.getValue();
	if ( p_src_format.getValue() == iim::TILED_MC_FORMAT ||
		 p_src_format.getValue() == iim::TILED_MC_TIF3D_FORMAT )  
		this->update_mdata = p_update_mdata.getValue();
	else
		this->update_mdata = false;
	this->axis_V = axis(p_axis_V.getValue());
	this->axis_H = axis(p_axis_H.getValue());
	this->axis_D = axis(p_axis_D.getValue());
	this->src_format  = p_src_format.getValue();

}

//checks parameters correctness
void TemplateCLI::checkParams() throw (iom::exception)
{
	//parameters check should be done here.
	//We trust in current tool functions checks.
	//print();

	;
}

//returns help text
string TemplateCLI::getHelpText()
{
	stringstream helptext;

	helptext << "Generator of volume descriptor " << terastitcher::mdatagenerator_version << "\n";
	helptext << "  developed at University Campus Bio-Medico of Rome by:\n";
	helptext << "   -\tAlessandro Bria (email: a.bria@unicas.it)                            ";
	helptext << "    \tPhD student at Departement of Electrical and Information Engineering";
	helptext << "    \tFaculty of Engineering of University of Cassino\n";
	helptext << "   -\tGiulio Iannello, Ph.D. (email: g.iannello@unicampus.it)              ";
	helptext << "    \tFull Professor of Computer Science and Computer Engineering          ";
	helptext << "    \tFaculty of Engineering of University Campus Bio-Medico of Rome\n";
	helptext << "  Official website/repo: http://abria.github.io/TeraStitcher";

	return helptext.str();
}

//print all arguments
void TemplateCLI::print()
{
	printf("\n\n");
	printf("overwrite_mdata = \t\t%s\n", overwrite_mdata ? "ENABLED" : "disabled");
	printf("root_dir = \t%s\n", root_dir.c_str());
}
