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
	TCLAP::CmdLine cmd(getHelpText(), '=', "1.0.0");
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
	//TCLAP::SwitchArg p_overwrite_mdata("","ovrw","Overwrite data.",false); 
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

	TCLAP::ValueArg<std::string> p_root_dir("r","rdir","Source root directory path.",true,"","string");
	TCLAP::ValueArg<std::string> p_dst_dir("d","ddir","Destination path.",true,"","string");
	TCLAP::ValueArg<int> p_V0("","V0","First coordinate along V.",true,-1,"unsigned");
	TCLAP::ValueArg<int> p_V1("","V1","Last coordinate along V.",true,-1,"unsigned");
	TCLAP::ValueArg<int> p_H0("","H0","First coordinate along H.",true,-1,"unsigned");
	TCLAP::ValueArg<int> p_H1("","H1","Last coordinate along H.",true,-1,"unsigned");
	TCLAP::ValueArg<int> p_D0("","D0","First coordinate along D.",true,-1,"unsigned");
	TCLAP::ValueArg<int> p_D1("","D1","Last coordinate along D.",true,-1,"unsigned");
	TCLAP::ValueArg<double> p_mem("m","mem","Max memory occupancy (MB).",false,1000.0,"unsigned");
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
	cmd.add(p_mem);
	cmd.add(p_D1);
	cmd.add(p_D0);
	cmd.add(p_H1);
	cmd.add(p_H0);
	cmd.add(p_V1);
	cmd.add(p_V0);
	cmd.add(p_dst_dir);
	cmd.add(p_root_dir);

	// Parse the argv array and catch <TCLAP> exceptions, which are translated into <MyException> exceptions
	char errMsg[S_STATIC_STRINGS_SIZE];
	try{ cmd.parse( argc, argv ); } 
	catch (TCLAP::ArgException &e)
	{ 
		sprintf(errMsg, "%s for arg %s\n", e.error().c_str(), e.argId().c_str());
		throw iom::exception(errMsg);
	}

	/* Checking parameter consistency */

	//importing parameters not set yet

	this->root_dir = p_root_dir.getValue();
	this->dst_dir = p_dst_dir.getValue();
	this->V0 = p_V0.getValue();
	this->V1 = p_V1.getValue();
	this->H0 = p_H0.getValue();
	this->H1 = p_H1.getValue();
	this->D0 = p_D0.getValue();
	this->D1 = p_D1.getValue();
	this->mem = p_mem.getValue();
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
	helptext << "Subvolume extractor v1.0.0\n";
	return helptext.str();
}

//print all arguments
void TemplateCLI::print()
{
	printf("\n\n");
	printf("overwrite_mdata = \t\t%s\n", overwrite_mdata ? "ENABLED" : "disabled");
	printf("root_dir = \t%s\n", root_dir.c_str());
}
