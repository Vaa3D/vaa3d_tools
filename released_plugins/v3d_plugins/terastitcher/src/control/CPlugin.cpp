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

#ifdef __unix__
#include <dlfcn.h>
#endif
#include <QErrorMessage>
#include "v3d_message.h"
#include "CPlugin.h"
#include "src/presentation/PMain.h"

using namespace terastitcher;

/*******************************************************************************************************************************
 *   Interfaces, types, parameters and constants   													       *
 *******************************************************************************************************************************/
namespace terastitcher
{
    /*******************
    *    PARAMETERS    *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
    std::string version = "1.2.0";
    /*-------------------------------------------------------------------------------------------------------------------------*/
}

// 1- Export the plugin class to a target, the first item in the bracket should match the TARGET parameter in the .pro file
Q_EXPORT_PLUGIN2(terastitcherplugin, terastitcher::CPlugin)
 

// 2- Set up the items in plugin domenu
QStringList CPlugin::menulist() const
{
        return QStringList()
                <<tr("TeraStitcher");
}

// 3 - Set up the function list in plugin dofunc
QStringList CPlugin::funclist() const
{
        return QStringList();
}

// 4 - Call the functions corresponding to the domenu items. 
void CPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> CPlugin::domenu launched\n", this->thread()->currentThreadId());
    #endif

    //overriding the current locale with the standard POSIX locale
    setlocale(LC_ALL, "POSIX");

    if (menu_name == tr("TeraStitcher"))
    {
        //launching PMain's GUI
        PMain::instance(&callback, 0);
    }
}

// 5 - Call the functions corresponding to dofunc
bool CPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    printf("TeraStitcher plugin needs Vaa3D GUI to be executed.\n"
           "If you want to run TeraStitcher from command line, please download and use the command-line TeraStitcher available at http://www.iconfoundation.net/?q=products\n");

    return true;
}

