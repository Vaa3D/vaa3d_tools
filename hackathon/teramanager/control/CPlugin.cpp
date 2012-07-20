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
#include "presentation/PMain.h"

using namespace teramanager;

// 1- Export the plugin class to a target, the first item in the bracket should match the TARGET parameter in the .pro file
Q_EXPORT_PLUGIN2(teramanagerplugin, teramanager::CPlugin)
 

// 2- Set up the items in plugin domenu
QStringList CPlugin::menulist() const
{
        return QStringList()
                <<tr("Teravoxel-sized Image Manager")
                <<tr("About and help");
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
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> CPlugin::domenu launched\n", this->thread()->currentThreadId());
    #endif

    if (menu_name == tr("Teravoxel-sized Image Manager"))
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
        PMain::instance(&callback, parent);
        PMain::instance()->show();
        PMain::instance()->raise();
        PMain::instance()->activateWindow();
    }
    else if(menu_name == tr("Acquisition: ROI from High Resolution Image"))
    {
        v3d_msg("****** \"Acquisition: ROI from High Resolution Image\" DETECTED\n");
        //printf("****** \"Acquisition: ROI from High Resolution Image\" DETECTED\n");
        if(PMain::isInstantiated())
            PMain::instance()->Vaa3D_selectedROI();
    }
    else
    {
        printf("MENU_NAME = %s\n", menu_name.toStdString().c_str());
        QMessageBox msgBox;
        QSpacerItem* horizontalSpacer = new QSpacerItem(800, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
        msgBox.setText( "<html><h1>TeraManager plugin v. 0.1</h1>"
                        "<big>An experimental tool designed for Teravoxel-sized datasets visualization into Vaa3D.</big><br><br>"
                        "<u>Developed by:</u><ul>"
                        "<li><b>Alessandro Bria</b> (email: a.bria@unicas.it)<br>"
                               "Ph.D. Student at University of Cassino</li>"
                        "<li><b>Giulio Iannello</b> (email: g.iannello@unicampus.it)<br>"
                               "Full Professor at University Campus Bio-Medico of Rome</li></ul><br>"
                        "<u>Features:</u><ul>"
                        "<li>user can select a subvolume to be shown into Vaa3D</li>"
                        "<li>low memory requirements (< 1 GB) for multi-stacked datasets</li></ul><br>"
                        "<u>Supported input formats:</u><ul>"
                        "<li>two-level directory structure with each tile containing a series of image slices (see documentation for further information)</li>"
                        "<li>supported formats for image slices are BMP, DIB, JPEG, JPG, JPE, PNG, PBM, PGM, PPM, SR, RAS, TIFF, TIF</li>"
                        "<li>no restriction on the bit depth</li>"
                        "<li>no restriction on the number of channels</li></ul></html>" );

        QGridLayout* layout = (QGridLayout*)msgBox.layout();
        layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());
        msgBox.exec();

        return;
    }
}

// 5 - Call the functions corresponding to dofunc
bool CPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    printf("TeraManager plugin needs Vaa3D GUI to be executed.\n");
}

//returns true if the given shared library can be loaded
bool CPlugin::isSharedLibraryLoadable(const char* name)
{
    #ifdef __unix__
    //UNIX
    QString tmp("");
    tmp.append("lib");
    tmp.append(name);
    tmp.append(".so");
    void* my_lib_handle = dlopen(tmp.toStdString().c_str(),RTLD_NOW);
    if(!my_lib_handle)
        return false;
    else
    {
        dlclose(my_lib_handle);
        return true;
    }
    #endif
    #ifdef _WIN32
    return true;    //not yet supported: anyway Windows O.S. should display an error message about the missing DLL.
    #endif
}

