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
#include "CExplorerWindow.h"
#include "presentation/PMain.h"
#include "presentation/PConverter.h"

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
    std::string version = "0.7.17";
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
        //checking shared libraries
        if(!CPlugin::isSharedLibraryLoadable("opencv_core"))
        {
            QMessageBox::critical(parent,QObject::tr("Error"),
                                         QObject::tr("Unable to load shared library opencv_core.\n\nPlease check if OpenCV library is installed. You can download it at http://opencv.willowgarage.com/"),
                                         QObject::tr("Ok"));
            return;
        }
        if(!CPlugin::isSharedLibraryLoadable("opencv_highgui"))
        {
            QMessageBox::critical(parent,QObject::tr("Error"),
                                         QObject::tr("Unable to load shared library opencv_highgui.\n\nPlease check if OpenCV library is installed. You can download it at http://opencv.willowgarage.com/"),
                                         QObject::tr("Ok"));
            return;
        }

        //launching plugin's GUI
        PMain::instance(&callback, 0);
    }    
    else if(menu_name == tr("TeraConverter"))
    {
        //checking shared libraries
        if(!CPlugin::isSharedLibraryLoadable("opencv_core"))
        {
            QMessageBox::critical(parent,QObject::tr("Error"),
                                         QObject::tr("Unable to load shared library opencv_core.\n\nPlease check if OpenCV library is installed. You can download it at http://opencv.willowgarage.com/"),
                                         QObject::tr("Ok"));
            return;
        }
        if(!CPlugin::isSharedLibraryLoadable("opencv_highgui"))
        {
            QMessageBox::critical(parent,QObject::tr("Error"),
                                         QObject::tr("Unable to load shared library opencv_highgui.\n\nPlease check if OpenCV library is installed. You can download it at http://opencv.willowgarage.com/"),
                                         QObject::tr("Ok"));
            return;
        }

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
            CExplorerWindow::getCurrent()->Vaa3D_selectedROI();
    }
    else
    {
        return;
    }
}

// 5 - Call the functions corresponding to dofunc
bool CPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> CPlugin::dofunc(\"%s\")\n",  func_name.toStdString().c_str());
    #endif

    printf("TeraManager plugin needs Vaa3D GUI to be executed.\n");
}

//returns true if the given shared library can be loaded
bool CPlugin::isSharedLibraryLoadable(const char* name)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread ?] >> CPlugin::isSharedLibraryLoadable(\"%s\")\n", name);
    #endif

//    // ------ TO DO ------
//    #ifdef __unix__
//    //UNIX
//    QString tmp("");
//    tmp.append("lib");
//    tmp.append(name);
//    tmp.append(".so");
//    void* my_lib_handle = dlopen(tmp.toStdString().c_str(),RTLD_NOW);
//    if(!my_lib_handle)
//        return false;
//    else
//    {
//        dlclose(my_lib_handle);
//        return true;
//    }
//    return true;    //to be fixed: _unix_ is defined also on MAC OS
//    #endif
//    #ifdef _WIN32
//    return true;    //not yet supported: anyway Windows O.S. should display an error message about the missing DLL.
//    #endif
	
    return true;
}

