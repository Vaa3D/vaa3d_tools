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
#include "v3d_interface.h"
#include "../presentation/PMain.h"
#include "../presentation/PConverter.h"
#include "VirtualVolume.h"
#include "v3d_message.h"
#include "CPlugin.h"
#include "CViewer.h"
#include "QUndoMarkerCreate.h"
#include "PAnoToolBar.h"
#include "CAnnotations.h"

using namespace terafly;

/*******************************************************************************************************************************
 *   Interfaces, types, parameters and constants                                                                               *
 *******************************************************************************************************************************/
namespace terafly
{
    /*******************
    *    PARAMETERS    *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
    std::string version = "2.5.9";          // software version
    int DEBUG = LEV_MAX;                    // debug level
    debug_output DEBUG_DEST = TO_STDOUT;    // where debug messages should be print (default: stdout)
    std::string DEBUG_FILE_PATH = "/Users/Administrator/Desktop/terafly_debug.log";   //filepath where to save debug information
    /*-------------------------------------------------------------------------------------------------------------------------*/

    /*******************
    *  SYNCRONIZATION  *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
    QMutex updateGraphicsInProgress;
    /*-------------------------------------------------------------------------------------------------------------------------*/

    static std::map < std::string, iim::VirtualVolume* > volumes_opened = std::map< std::string, iim::VirtualVolume* >();
}

// 4 - Call the functions corresponding to the domenu items. 
void TeraFly::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    /**/tf::debug(tf::LEV1, strprintf("menu_name = %s", menu_name.toStdString().c_str()).c_str(), __itm__current__function__);

    //overriding the current locale with the standard POSIX locale
    setlocale(LC_ALL, "POSIX");

    //register custom types
    qRegisterMetaType<tf::integer_array>("tf::integer_array");

    if (menu_name == tr("TeraFly"))
    {
        // launch plugin's GUI
        PMain::instance(&callback, 0);

        // reset widgets to default state
        PMain::getInstance()->reset();
    }    
    else if(menu_name == tr("TeraConverter"))
    {
        // launch PConverter's GUI
        PConverter::instance(&callback, 0);
        PConverter::instance()->show();
        PConverter::instance()->move(QApplication::desktop()->screen()->rect().center() - PConverter::instance()->rect().center());
        PConverter::instance()->raise();
        PConverter::instance()->activateWindow();
    }
    else if(menu_name == tr("Fetch Highrez Image Data from File"))
    {
        if(CViewer::getCurrent())
            CViewer::getCurrent()->invokedFromVaa3D();
        else
            QMessageBox::information(0, "Information", "This option is available only when visualizing Big-Image-Data with TeraFly.\n\n"
                                     "You can find TeraFly under Advanced > Big-Image-Data > TeraFly.");
    }
    else
    {
        return;
    }
    /**/tf::debug(tf::LEV1, "EOF", __itm__current__function__);
}

void TeraFly::doaction(const QString &action_name)
{
    if(action_name == tr("Fetch Highrez Image Data from File"))
    {
        if(CViewer::getCurrent())
            CViewer::getCurrent()->invokedFromVaa3D();
        else
            QMessageBox::information(0, "Information", "This option is available only when visualizing Big-Image-Data with TeraFly.\n\n"
                                     "You can find TeraFly under Advanced > Big-Image-Data > TeraFly.");
    }
    else if(action_name == tr("marker multiselect"))
    {
        if(CViewer::getCurrent())
            CViewer::getCurrent()->deleteSelectedMarkers();
    }
    else if(action_name == tr("neuron edit"))
    {
        if(CViewer::getCurrent())
        {
            CViewer::getCurrent()->undoStack.beginMacro("vaa3d action");
            CViewer::getCurrent()->undoStack.push(new QUndoVaa3DNeuron(CViewer::getCurrent()));
            CViewer::getCurrent()->undoStack.endMacro();
            PAnoToolBar::instance()->buttonUndo->setEnabled(true);
        }
    }
    else
        QMessageBox::information(0, "Information", tf::strprintf("Unrecognized action \"%s\" called on TeraFly", qPrintable(action_name)).c_str());
}

// returns true if version >= min_required_version, where version format is version.major.minor
bool TeraFly::checkVersion(std::string version, std::string min_required_version)
{
    vector<string> tokens_A, tokens_B;
    tf::split(version, ".", tokens_A);
    tf::split(min_required_version, ".", tokens_B);

    int verA = tokens_A.size() > 0 ? atoi(tokens_A[0].c_str()) : 0;
    int verB = tokens_B.size() > 0 ? atoi(tokens_B[0].c_str()) : 0;

    int majA = tokens_A.size() > 1 ? atoi(tokens_A[1].c_str()) : 0;
    int majB = tokens_B.size() > 1 ? atoi(tokens_B[1].c_str()) : 0;

    int minA = tokens_A.size() > 2 ? atoi(tokens_A[2].c_str()) : 0;
    int minB = tokens_B.size() > 2 ? atoi(tokens_B[2].c_str()) : 0;

    if(verA < verB)
        return false;
    else if(verA > verB)
        return true;
    else    // same version
    {
        if(majA < majB)
            return false;
        else if(majA > majB)
            return true;
        else    // same major
            return minA >= minB;
    }
}


// access the 3D curve set for the whole image at the given resolution (default: highest resolution)
NeuronTree tf::PluginInterface::getSWC(int resolution)
{
    NeuronTree nt;

    try
    {
        // set default parameter
        if(resolution == infp<int>())
            resolution = CImport::instance()->getResolutions() - 1;

        // check preconditions
        if(resolution != CImport::instance()->getResolutions() - 1)
            throw tf::RuntimeException(tf::strprintf("Accessing curve/marker structures at lower resolutions (res index = %d) not yet implemented", resolution));
        if(CViewer::getCurrent() == 0)
            throw tf::RuntimeException(tf::strprintf("Cannot access current image viewer"));

        // store last changes made on the viewer to the octree
        CViewer::getCurrent()->storeAnnotations();

        // get entire octree content
        interval_t x_range(0, std::numeric_limits<int>::max());
        interval_t y_range(0, std::numeric_limits<int>::max());
        interval_t z_range(0, std::numeric_limits<int>::max());
        CAnnotations::getInstance()->findCurves(x_range, y_range, z_range, nt.listNeuron);

        return nt;
    }
    catch (tf::RuntimeException & e)
    {
        v3d_msg(QString("Exception catched in TeraFly plugin API: ") + e.what(), true);
    }

    return nt;
}

bool tf::PluginInterface::setSWC(NeuronTree & nt, int resolution)
{
    try
    {
        // set default parameter
        if(resolution == infp<int>())
            resolution = CImport::instance()->getResolutions() - 1;

        // check preconditions
        if(resolution != CImport::instance()->getResolutions() - 1)
            throw tf::RuntimeException(tf::strprintf("Accessing curve/marker structures at lower resolutions (res index = %d) not yet implemented", resolution));
        if(CViewer::getCurrent() == 0)
            throw tf::RuntimeException(tf::strprintf("Cannot access current image viewer"));

        // set entire octree content
        interval_t x_range(0, std::numeric_limits<int>::max());
        interval_t y_range(0, std::numeric_limits<int>::max());
        interval_t z_range(0, std::numeric_limits<int>::max());
        CAnnotations::getInstance()->addCurves(x_range, y_range, z_range, nt);

        // push content to viewer
        CViewer::getCurrent()->loadAnnotations();
    }
    catch (tf::RuntimeException & e)
    {
        v3d_msg(QString("Exception catched in TeraFly plugin API: ") + e.what(), true);
    }
}

// access the 3D landmark list defined for the whole image at the given resolution (default: highest resolution)
LandmarkList tf::PluginInterface::getLandmark(int resolution)
{
    LandmarkList markers;

    try
    {
        // set default parameter
        if(resolution == infp<int>())
            resolution = CImport::instance()->getResolutions() - 1;

        // check preconditions
        if(resolution != CImport::instance()->getResolutions() - 1)
            throw tf::RuntimeException(tf::strprintf("Accessing curve/marker structures at lower resolutions (res index = %d) not yet implemented", resolution));
        if(CViewer::getCurrent() == 0)
            throw tf::RuntimeException(tf::strprintf("Cannot access current image viewer"));

        // store last changes made on the viewer to the octree
        CViewer::getCurrent()->storeAnnotations();

        // get entire octree content
        interval_t x_range(0, std::numeric_limits<int>::max());
        interval_t y_range(0, std::numeric_limits<int>::max());
        interval_t z_range(0, std::numeric_limits<int>::max());
        CAnnotations::getInstance()->findLandmarks(x_range, y_range, z_range, markers);
    }
    catch (tf::RuntimeException & e)
    {
        v3d_msg(QString("Exception catched in TeraFly plugin API: ") + e.what(), true);
    }

    return markers;
}

bool tf::PluginInterface::setLandmark(LandmarkList & landmark_list, int resolution)
{
    try
    {
        // set default parameter
        if(resolution == infp<int>())
            resolution = CImport::instance()->getResolutions() - 1;

        // check preconditions
        if(resolution != CImport::instance()->getResolutions() - 1)
            throw tf::RuntimeException(tf::strprintf("Accessing curve/marker structures at lower resolutions (res index = %d) not yet implemented", resolution));
        if(CViewer::getCurrent() == 0)
            throw tf::RuntimeException(tf::strprintf("Cannot access current image viewer"));

        // set entire octree content
        interval_t x_range(0, std::numeric_limits<int>::max());
        interval_t y_range(0, std::numeric_limits<int>::max());
        interval_t z_range(0, std::numeric_limits<int>::max());
        CAnnotations::getInstance()->addLandmarks(x_range, y_range, z_range, landmark_list);

        // push content to viewer
        CViewer::getCurrent()->loadAnnotations();
    }
    catch (tf::RuntimeException & e)
    {
        v3d_msg(QString("Exception catched in TeraFly plugin API: ") + e.what(), true);
    }
}

// get path of the image volume at the given resolution (default: highest resolution)
std::string tf::PluginInterface::getPath(int resolution)
{
    try
    {
        // set default parameter
        if(resolution == infp<int>())
            resolution = CImport::instance()->getResolutions() - 1;

        // check preconditions
        if(CImport::instance()->getVolume(resolution) == 0)
            throw tf::RuntimeException(tf::strprintf("Cannot access image volume at resolution \"%d\"", resolution));

        return CImport::instance()->getVolume(resolution)->getROOT_DIR();
    }
    catch (tf::RuntimeException & e)
    {
        v3d_msg(QString("Exception catched in TeraFly plugin API: ") + e.what(), true);
    }
    return NULL;
}

// get currently displayed image (readonly)
const Image4DSimple* tf::PluginInterface::getImage()
{
    try
    {
        // check preconditions
        if(CViewer::getCurrent() == 0)
            throw tf::RuntimeException(tf::strprintf("Cannot access current image viewer"));

        // get and return image
        return CViewer::getCurrent()->getImage();
    }
    catch (tf::RuntimeException & e)
    {
        v3d_msg(QString("Exception catched in TeraFly plugin API: ") + e.what(), true);
    }
    return NULL;

}

// get image metadata from the given image file/folder path
size_t tf::PluginInterface::getXDim(const std::string & path)
{
    try
    {
        if(volumes_opened.find(path) == volumes_opened.end())
            volumes_opened[path] = iim::VirtualVolume::instance(path.c_str());

        return size_t(volumes_opened[path]->getDIM_H());
    }
    catch (iom::exception & e)
    {
        v3d_msg(QString("Exception catched in TeraFly plugin API: ") + e.what(), true);
    }
    catch (iim::IOException & e)
    {
        v3d_msg(QString("Exception catched in TeraFly plugin API: ") + e.what(), true);
    }
    return NULL;

}

size_t tf::PluginInterface::getYDim(const std::string & path)
{
    try
    {
        if(volumes_opened.find(path) == volumes_opened.end())
            volumes_opened[path] = iim::VirtualVolume::instance(path.c_str());

        return size_t(volumes_opened[path]->getDIM_V());
    }
    catch (iom::exception & e)
    {
        v3d_msg(QString("Exception catched in TeraFly plugin API: ") + e.what(), true);
    }
    catch (iim::IOException & e)
    {
        v3d_msg(QString("Exception catched in TeraFly plugin API: ") + e.what(), true);
    }
    return NULL;

}
size_t tf::PluginInterface::getZDim(const std::string & path)
{
    try
    {
        if(volumes_opened.find(path) == volumes_opened.end())
            volumes_opened[path] = iim::VirtualVolume::instance(path.c_str());

        return size_t(volumes_opened[path]->getDIM_D());
    }
    catch (iom::exception & e)
    {
        v3d_msg(QString("Exception catched in TeraFly plugin API: ") + e.what(), true);
    }
    catch (iim::IOException & e)
    {
        v3d_msg(QString("Exception catched in TeraFly plugin API: ") + e.what(), true);
    }
    return NULL;

}
size_t tf::PluginInterface::getCDim(const std::string & path)
{
    try
    {
        if(volumes_opened.find(path) == volumes_opened.end())
            volumes_opened[path] = iim::VirtualVolume::instance(path.c_str());

        return size_t(volumes_opened[path]->getDIM_C());
    }
    catch (iom::exception & e)
    {
        v3d_msg(QString("Exception catched in TeraFly plugin API: ") + e.what(), true);
    }
    catch (iim::IOException & e)
    {
        v3d_msg(QString("Exception catched in TeraFly plugin API: ") + e.what(), true);
    }
    return NULL;

}
size_t tf::PluginInterface::getTDim(const std::string & path)
{
    try
    {
        if(volumes_opened.find(path) == volumes_opened.end())
            volumes_opened[path] = iim::VirtualVolume::instance(path.c_str());

        return size_t(volumes_opened[path]->getDIM_T());
    }
    catch (iom::exception & e)
    {
        v3d_msg(QString("Exception catched in TeraFly plugin API: ") + e.what(), true);
    }
    catch (iim::IOException & e)
    {
        v3d_msg(QString("Exception catched in TeraFly plugin API: ") + e.what(), true);
    }
    return NULL;

}

// get image subvolume from the given image file/folder path
// x = horizontal axis, y = vertical axis, z = depth axis, t = time axis
// intervals are open at right, e.g. [x0, x1)
unsigned char* tf::PluginInterface::getSubVolume(const std::string & path, size_t x0, size_t x1, size_t y0, size_t y1, size_t z0, size_t z1, size_t t0, size_t t1)
{
    try
    {
        if(volumes_opened.find(path) == volumes_opened.end())
            volumes_opened[path] = iim::VirtualVolume::instance(path.c_str());

        volumes_opened[path]->setActiveFrames(int(t0), int(t1));
        return volumes_opened[path]->loadSubvolume_to_UINT8(y0, y1, x0, x1, z0, z1);
    }
    catch (iom::exception & e)
    {
        v3d_msg(QString("Exception catched in TeraFly plugin API: ") + e.what(), true);
    }
    catch (iim::IOException & e)
    {
        v3d_msg(QString("Exception catched in TeraFly plugin API: ") + e.what(), true);
    }
    return NULL;

}

// release memory allocated for opened volumes
void tf::PluginInterface::releaseOpenedVolumes()
{
    try
    {
        for(std::map <std::string, iim::VirtualVolume*>::iterator it = volumes_opened.begin(); it != volumes_opened.end(); it++)
            delete it->second;
        volumes_opened.clear();
    }
    catch (iom::exception & e)
    {
        v3d_msg(QString("Exception catched in TeraFly plugin API: ") + e.what(), true);
    }
    catch (iim::IOException & e)
    {
        v3d_msg(QString("Exception catched in TeraFly plugin API: ") + e.what(), true);
    }
}


