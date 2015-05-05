/* N3Dfix_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2015-05-04 : by Eduardo Conde-Sousa
 */
 
#include "v3d_message.h"
#include <vector>
#include <stdio.h>
#include <iostream>
#include "myHeader.h"
#include "Dend_Section.h"
#include "N3Dfix_plugin.h"

using namespace std;




Q_EXPORT_PLUGIN2(N3Dfix, N3DfixPlugin)
 
QStringList N3DfixPlugin::menulist() const
{
	return QStringList() 
		<<tr("Run_menu")
		<<tr("about");
}

QStringList N3DfixPlugin::funclist() const
{
	return QStringList()
		<<tr("Run_cmd")
		<<tr("help");
}

void N3DfixPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("Run_menu"))
	{
        /////////////////////////////////////////////////////////////////////////////////////////
        //	Set up global parameters and functions
        /////////////////////////////////////////////////////////////////////////////////////////
        double max_dist = 20;	// distance in um

        double thresh = 0.1;		// between 0 and 1

        double step_min = 0.15; 	// adding to the THRESHOLD, this parameter sets the necessity of the artifact to differ by STEP_MIN %
                                // from the original diameter (to prevent the existence of two very close spots with different diameters)


        // preprocessing_flag indicates to the system if proc remove_points_with_same_coordinates() has been performed
        bool preprocessing_flag = 0;


        //define vectors to contain data
        std::vector<float> x;
        std::vector<float> y;
        std::vector<float> z;
        std::vector<long> tree_id;
        std::vector<float> r;
        std::vector<long> ppid;
        std::vector<long> nodes;
        std::vector<long> endpoints;
        std::vector<long> end_sec;
        struct RawPoints Point;
        std::vector<struct RawPoints > n3d;
        std::vector< std::vector<struct RawPoints > > dend;
        std::vector<double> ARC;
        std::vector<double> DIAM;
        std::vector<double> dydx;
        std::vector<long> ppslope;
        std::vector<long> pnslope;
        std::vector<long> pzslope;
        _main(max_dist,thresh , step_min, preprocessing_flag,x,y,z,tree_id,r,ppid,nodes,endpoints,end_sec,Point,n3d,dend,ARC,DIAM,dydx,ppslope,pnslope,pzslope);

        //v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Eduardo Conde-Sousa, 2015-05-04"));
	}
}

bool N3DfixPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("Run_cmd"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

