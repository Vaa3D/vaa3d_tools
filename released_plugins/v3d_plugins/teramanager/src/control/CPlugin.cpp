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

#include <QErrorMessage>
#include "../presentation/PMain.h"
#include "../presentation/PConverter.h"
#include "v3d_message.h"
#include "CPlugin.h"
#include "CExplorerWindow.h"

using namespace teramanager;

/*******************************************************************************************************************************
 *   Interfaces, types, parameters and constants   													       *
 *******************************************************************************************************************************/
namespace teramanager
{
    /*******************
    *    PARAMETERS    *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
    std::string version = "0.9.11";
    /*-------------------------------------------------------------------------------------------------------------------------*/
}

// 1- Export the plugin class to a target, the first item in the bracket should match the TARGET parameter in the .pro file
Q_EXPORT_PLUGIN2(teramanagerplugin, teramanager::CPlugin)
 

// 2- Set up the items in plugin domenu
QStringList CPlugin::menulist() const
{
        return QStringList()
                <<tr("TeraFly")
                <<tr("TeraConverter");
}

// 3 - Set up the function list in plugin dofunc
QStringList CPlugin::funclist() const
{
        return QStringList()
                <<tr("Help");
}

// 4 - Call the functions corresponding to the domenu items. 
void CPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> CPlugin::domenu(\"%s\")\n",  menu_name.toStdString().c_str());
    #endif

    //overriding the current locale with the standard POSIX locale
    setlocale(LC_ALL, "POSIX");

    if (menu_name == tr("TeraFly"))
    {
        //launching plugin's GUI
        PMain::instance(&callback, 0);
    }    
    else if(menu_name == tr("TeraConverter"))
    {
        //launching PConverter's GUI
        PConverter::instance(&callback, parent);
        PConverter::instance()->show();
        PConverter::instance()->move(QApplication::desktop()->screen()->rect().center() - PConverter::instance()->rect().center());
        PConverter::instance()->raise();
        PConverter::instance()->activateWindow();
    }
    else if(menu_name == tr("Fetch Highrez Image Data from File"))
    {
        if(CExplorerWindow::getCurrent())
            CExplorerWindow::getCurrent()->invokedFromVaa3D();
    }
    else
    {
        return;
    }
    printf("--------------------- teramanager plugin [thread *] >> CPlugin::domenu() finished\n");
}

// 5 - Call the functions corresponding to dofunc
bool CPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> CPlugin::dofunc(\"%s\")\n",  func_name.toStdString().c_str());
    #endif

    printf("TeraManager plugin needs Vaa3D GUI to be executed.\n");
    return true;
}

