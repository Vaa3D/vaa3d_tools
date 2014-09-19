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

#ifndef CEXPLORERWINDOW_H
#define CEXPLORERWINDOW_H

#include "CPlugin.h"
#include "v3dr_mainwindow.h"
#include "renderer_gl2.h"
#include "CImport.h"
#include "v3d_imaging_para.h"

class teramanager::CExplorerWindow : public QWidget
{
    Q_OBJECT

    private:

        //OBJECT members
        V3DPluginCallback2* V3D_env;    //handle of V3D environment
        v3dhandle window;               //generic (void *) handle of the tri-view image window
        XFormWidget* triViewWidget;     //the tri-view image window
        V3dR_GLWidget* view3DWidget;    //3D renderer widget associated to the image window
        V3dR_MainWindow* window3D;      //the window enclosing <view3DWidget>
        CExplorerWindow *next, *prev;   //the next (higher resolution) and previous (lower resolution) <CExplorerWindow> objects
        int volResIndex;                //resolution index of the volume displayed in the current window (see member <volumes> of CImport)
        itm::uint8* imgData;
        int volV0, volV1;               //first and last vertical coordinates of the volume displayed in the current window
        int volH0, volH1;               //first and last horizontal coordinates of the volume displayed in the current window
        int volD0, volD1;               //first and last depth coordinates of the volume displayed in the current window
        int volT0, volT1;               //first and last time coordinates of the volume displayed in the current window
        int nchannels;                  //number of image channels
        string title;                   //title of current window
        string titleShort;              //short title of current window
        bool toBeClosed;                //true when the current window is marked as going to be closed
        bool isActive;                  //false when the current window is set as not active (e.g. when after zooming-in/out)
        bool isReady;                   //true when current window is ready for receiving user inputs (i.e. all image data have been loaded)
        bool has_double_clicked;        //true when a double click event has been just catched (will be set to false when the double click is managed)
        int zoomHistory[ZOOM_HISTORY_SIZE];//last 4 zoom values
        int V0_sbox_min, V0_sbox_val;   //to save the state of subvolume spinboxes when the current window is hidden
        int V1_sbox_max, V1_sbox_val;   //to save the state of subvolume spinboxes when the current window is hidden
        int H0_sbox_min, H0_sbox_val;   //to save the state of subvolume spinboxes when the current window is hidden
        int H1_sbox_max, H1_sbox_val;   //to save the state of subvolume spinboxes when the current window is hidden
        int D0_sbox_min, D0_sbox_val;   //to save the state of subvolume spinboxes when the current window is hidden
        int D1_sbox_max, D1_sbox_val;   //to save the state of subvolume spinboxes when the current window is hidden
        int T0_sbox_min, T0_sbox_val;   //to save the state of subvolume spinboxes when the current window is hidden
        int T1_sbox_max, T1_sbox_val;   //to save the state of subvolume spinboxes when the current window is hidden
        int ID;
        bool waitingFor5D;              //"waiting for 5D data" state flag
        bool scribbling;                //"scribbling on the 3D renderer with right-button mouse" state flag
        QVector<QPoint> scribbling_points;
        QUndoStack undoStack;           //stack containing undo command actions
        int slidingViewerBlockID;
        bool forceZoomIn;

        //CLASS members
        static CExplorerWindow *first;  //pointer to the first window of the multiresolution explorer windows chain
        static CExplorerWindow *last;   //pointer to the last window of the multiresolution explorer windows chain
        static CExplorerWindow *current;//pointer to the current window of the multiresolution explorer windows chain
        static int nInstances;          //number of instantiated objects
        static int nTotalInstances;

        //MUTEX
        //QMutex updateGraphicsInProgress;

        //TIMER
        QElapsedTimer newViewTimer;

        //inhibiting default constructor
        CExplorerWindow();

        /**********************************************************************************
        * Returns  the  global coordinate  (which starts from 0) in  the given  resolution
        * volume image space given the local coordinate (which starts from 0) in the current
        * resolution volume image space. If resIndex is not set, the returned global coord-
        * inate will be in the highest resolution image space.
        ***********************************************************************************/
        // @moved to public section and made inline
//        int getGlobalVCoord(int localVCoord, int resIndex = -1, bool fromVaa3Dcoordinates = false, bool cutOutOfRange = false, const char* src = 0);
//        int getGlobalHCoord(int localHCoord, int resIndex = -1, bool fromVaa3Dcoordinates = false, bool cutOutOfRange = false, const char* src = 0);
//        int getGlobalDCoord(int localDCoord, int resIndex = -1, bool fromVaa3Dcoordinates = false, bool cutOutOfRange = false, const char* src = 0);
//        float getGlobalVCoord(float localVCoord, int resIndex = -1, bool fromVaa3Dcoordinates = false, bool cutOutOfRange = false, const char* src = 0);
//        float getGlobalHCoord(float localHCoord, int resIndex = -1, bool fromVaa3Dcoordinates = false, bool cutOutOfRange = false, const char* src = 0);
//        float getGlobalDCoord(float localDCoord, int resIndex = -1, bool fromVaa3Dcoordinates = false, bool cutOutOfRange = false, const char* src = 0);

        /**********************************************************************************
        * Returns the local coordinate (which starts from 0) in the current resolution vol-
        * ume image space given the global coordinate  (which starts from 0) in the highest
        * resolution volume image space.
        ***********************************************************************************/
        // @moved to public section and made inline
//        int getLocalVCoord(int highestResGlobalVCoord, bool toVaa3Dcoordinates = false);
//        int getLocalHCoord(int highestResGlobalHCoord, bool toVaa3Dcoordinates = false);
//        int getLocalDCoord(int highestResGlobalDCoord, bool toVaa3Dcoordinates = false);
//        float getLocalVCoord(float highestResGlobalVCoord, bool toVaa3Dcoordinates = false);
//        float getLocalHCoord(float highestResGlobalHCoord, bool toVaa3Dcoordinates = false);
//        float getLocalDCoord(float highestResGlobalDCoord, bool toVaa3Dcoordinates = false);


        /**********************************************************************************
        * Returns the most  likely 3D  point in the  image that the user is pointing on the
        * renderer at the given location.
        * This is based on the Vaa3D 3D point selection with one mouse click.
        ***********************************************************************************/
        XYZ getRenderer3DPoint(int x, int y) throw (itm::RuntimeException);

        /**********************************************************************************
        * Syncronizes widgets from <src> to <dst>
        ***********************************************************************************/
        void syncWindows(V3dR_MainWindow* src, V3dR_MainWindow* dst);


    public:

        //CONSTRUCTOR, DECONSTRUCTOR
        CExplorerWindow(V3DPluginCallback2* _V3D_env, int _resIndex, itm::uint8* _imgData, int _volV0, int _volV1,
                        int _volH0, int _volH1, int _volD0, int _volD1, int _volT0, int _volT1, int _nchannels, CExplorerWindow* _prev, int _slidingViewerBlockID = -1);
        ~CExplorerWindow();
        static void uninstance()
        {
            /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

            while(first){
                CExplorerWindow* p = first->next;
                first->toBeClosed = true;
                delete first;
                first = p;
            }
            first=last=0;
        }

        //performs all the operations needed to show 3D data (such as creating Vaa3D widgets)
        void show();

        //GET methods
        static CExplorerWindow* getCurrent(){return current;}
        int getResIndex(){return volResIndex;}
        V3dR_MainWindow* getWindow3D(){return window3D;}
        V3dR_GLWidget* getGLWidget(){return view3DWidget;}
        bool isHighestRes(){return volResIndex == CImport::instance()->getResolutions() -1;}

        /**********************************************************************************
        * Filters events generated by the 3D rendering window <view3DWidget>
        * We're interested to intercept these events to provide many useful ways to explore
        * the 3D volume at different resolutions without changing Vaa3D code.
        ***********************************************************************************/
        bool eventFilter(QObject *object, QEvent *event);

        /**********************************************************************************
        * Restores the current view from the given (neighboring) source view.
        * Called by the next(prev) <CExplorerWindow>  when the user  zooms out(in) and  the
        * lower(higher) resoolution has to be reestabilished.
        ***********************************************************************************/
        void restoreViewFrom(CExplorerWindow* source) throw (itm::RuntimeException);

        /**********************************************************************************
        * Generates a new view using the given coordinates.
        * Called by the current <CExplorerWindow> when the user zooms in and the higher res-
        * lution has to be loaded.
        ***********************************************************************************/
        void
        newView(
            int x, int y, int z,                //can be either the VOI's center (default)
                                                //or the VOI's ending point (see x0,y0,z0)
            int resolution,                     //resolution index of the view requested
            int t0, int t1,                     //time frames selection
            bool fromVaa3Dcoordinates = false,  //if coordinates were obtained from Vaa3D
            int dx=-1, int dy=-1, int dz=-1,    //VOI [x-dx,x+dx), [y-dy,y+dy), [z-dz,z+dz)
            int x0=-1, int y0=-1, int z0=-1,    //VOI [x0, x), [y0, y), [z0, z)
            bool auto_crop = true,              //whether to crop the VOI to the max dims
            bool scale_coords = true,           //whether to scale VOI coords to the target res
            int sliding_viewer_block_ID = -1    //block ID in "Sliding viewer" mode
        );

        /**********************************************************************************
        * Resizes  the  given image subvolume in a  newly allocated array using the fastest
        * achievable interpolation method. The image currently shown is used as data source.
        * Missing pieces of data are filled with black and returned to the caller.
        ***********************************************************************************/
        itm::uint8*
            getVOI(int x0, int x1,              // VOI [x0, x1) in the local reference sys
                   int y0, int y1,              // VOI [y0, y1) in the local reference sys
                   int z0, int z1,              // VOI [z0, z1) in the local reference sys
                   int t0, int t1,              // VOI [t0, t1] in the local reference sys
                   int xDimInterp,              // interpolated VOI dimension along X
                   int yDimInterp,              // interpolated VOI dimension along Y
                   int zDimInterp,              // interpolated VOI dimension along Z
                   int& x0m, int& x1m,          // black-filled VOI [x0m, x1m) in the local rfsys
                   int& y0m, int& y1m,          // black-filled VOI [y0m, y1m) in the local rfsys
                   int& z0m, int& z1m,          // black-filled VOI [z0m, z1m) in the local rfsys
                   int& t0m, int& t1m)          // black-filled VOI [t0m, t1m] in the local rfsys
        throw (itm::RuntimeException);

        /**********************************************************************************
        * Returns  the  maximum intensity projection  of the given VOI in a newly allocated
        * array. Data is taken from the currently displayed image.
        ***********************************************************************************/
        itm::uint8*
            getMIP(int x0, int x1,              // VOI [x0, x1) in the local reference sys
                   int y0, int y1,              // VOI [y0, y1) in the local reference sys
                   int z0, int z1,              // VOI [z0, z1) in the local reference sys
                   int t0 = -1, int t1 = -1,    // VOI [t0, t1] in the local reference sys
                   itm::direction dir = itm::z,
                   bool to_BGRA = false,        //true if mip data must be stored into BGRA format
                   itm::uint8 alpha = 255)      //alpha transparency used if to_BGRA is true
        throw (itm::RuntimeException);


        /**********************************************************************************
        * Makes the current view the last one by  deleting (and deallocting) its subsequent
        * views.
        ***********************************************************************************/
        void makeLastView() throw (itm::RuntimeException);

        /**********************************************************************************
        * Annotations are stored/loaded to/from the <CAnnotations> object
        ***********************************************************************************/
        void storeAnnotations() throw (itm::RuntimeException);
        void loadAnnotations() throw (itm::RuntimeException);
        void clearAnnotations() throw (itm::RuntimeException);
        void deleteMarkerROI(QVector<QPoint> ROI_contour) throw (itm::RuntimeException);
        void deleteMarkerAt(int x, int y, QList<LocationSimple>* deletedMarkers = 0) throw (itm::RuntimeException);
        void createMarkerAt(int x, int y) throw (itm::RuntimeException);

        /**********************************************************************************
        * Saves/restores the state of PMain spinboxes for subvolume selection
        ***********************************************************************************/
        void saveSubvolSpinboxState();
        void restoreSubvolSpinboxState();

        /**********************************************************************************
        * method (indirectly) invoked by Vaa3D to propagate VOI's coordinates
        ***********************************************************************************/
        void invokedFromVaa3D(v3d_imaging_paras* params = 0);

        /**********************************************************************************
        * Alignes the given widget to the left(right) of the current window
        ***********************************************************************************/
        void alignToLeft(QWidget* widget);
        void alignToRight(QWidget* widget);

        /**********************************************************************************
        * Overriding position, size and resize QWidget methods
        ***********************************************************************************/
        inline QPoint pos() const{ return window3D->pos(); }
        inline QSize size() const{ return window3D->size();}
        inline void resize(QSize new_size){
            if(window3D->size() != new_size)
                window3D->resize(new_size);
        }
        inline void move(QPoint p){
            if(pos().x() != p.x() || pos().y() != p.y())
                window3D->move(p);
        }
        inline void resize(int w, int h)  { resize(QSize(w, h)); }
        inline void move(int ax, int ay)  { move(QPoint(ax, ay)); }

        /**********************************************************************************
        * Activates / deactives the current window (in terms of responding to events)
        ***********************************************************************************/
        void setActive(bool active)
        {
            /**/itm::debug(itm::LEV1, strprintf("title = %s, active = %s", titleShort.c_str() , active ? "true" : "false").c_str(), __itm__current__function__);

            isActive = active;
            if(!isActive)
                isReady = false;
        }

        /**********************************************************************************
        * Change to "waiting for 5D" state (i.e., when 5D data are to be loaded or are loading)
        ***********************************************************************************/
        void setWaitingFor5D(bool wait, bool pre_wait=false);

        /**********************************************************************************
        * Zoom history methods (inline because the are called frequently)
        ***********************************************************************************/
        inline void resetZoomHistory(){
            for(int i=0; i<ZOOM_HISTORY_SIZE; i++)
                zoomHistory[i] = int_inf;
        }
        inline bool isZoomDerivativePos(){
            for(int i=1; i<ZOOM_HISTORY_SIZE; i++)
                if(zoomHistory[i-1] == int_inf || zoomHistory[i] <= zoomHistory[i-1])
                    return false;
            return true;
        }
        inline bool isZoomDerivativeNeg(){
            for(int i=1; i<ZOOM_HISTORY_SIZE; i++)
                if(zoomHistory[i-1] == int_inf || zoomHistory[i] >= zoomHistory[i-1])
                    return false;
            return true;
        }
        inline void zoomHistoryPushBack(int zoom){
            if(zoomHistory[ZOOM_HISTORY_SIZE-1] != zoom)
                for(int i=0; i<ZOOM_HISTORY_SIZE-1; i++)
                    zoomHistory[i] = zoomHistory[i+1];
            zoomHistory[ZOOM_HISTORY_SIZE-1] = zoom;
        }

        template<class T>
        inline bool is_outside(T x, T y, T z){
           return
                   x < 0  || x >= volH1-volH0 ||
                   y < 0  || y >= volV1-volV0 ||
                   z < 0  || z >= volD1-volD0;
        }

        /**********************************************************************************
        * Change current Vaa3D's rendered cursor
        ***********************************************************************************/
        static void setCursor(const QCursor& cur, bool renderer_only = false);

        //PMain instance is allowed to access class private members
        friend class PMain;
        friend class PAnoToolBar;
        friend class CVolume;
        friend class QUndoMarkerCreate;
        friend class QUndoMarkerDelete;
        friend class QUndoMarkerDeleteROI;


    public slots:

        /*********************************************************************************
        * Receive data (and metadata) from <CVolume> throughout the loading process
        **********************************************************************************/
        void receiveData(
                itm::uint8* data,                   // data (any dimension)
                itm::integer_array data_s,          // data start coordinates along X, Y, Z, C, t
                itm::integer_array data_c,          // data count along X, Y, Z, C, t
                QWidget* dest,                         // address of the listener
                bool finished,                      // whether the loading operation is terminated
                itm::RuntimeException* ex = 0,      // exception (optional)
                qint64 elapsed_time = 0,            // elapsed time (optional)
                QString op_dsc="",                  // operation descriptor (optional)
                int step=0);                        // step number (optional)


        /**********************************************************************************
        * Linked to volume cut scrollbars of Vaa3D widget containing the 3D renderer.
        * This implements the syncronization Vaa3D-->TeraFly of subvolume selection.
        ***********************************************************************************/
        void Vaa3D_changeXCut0(int s);
        void Vaa3D_changeXCut1(int s);
        void Vaa3D_changeYCut0(int s);
        void Vaa3D_changeYCut1(int s);
        void Vaa3D_changeZCut0(int s);
        void Vaa3D_changeZCut1(int s);
        void Vaa3D_changeTSlider(int s, bool editingFinished = false);

        /**********************************************************************************
        * Linked to PMain GUI VOI's widgets.
        * This implements the syncronization TeraFly-->Vaa3D of subvolume selection.
        ***********************************************************************************/
        void PMain_changeV0sbox(int s);
        void PMain_changeV1sbox(int s);
        void PMain_changeH0sbox(int s);
        void PMain_changeH1sbox(int s);
        void PMain_changeD0sbox(int s);
        void PMain_changeD1sbox(int s);

        /**********************************************************************************
        * Linked to PMain GUI<->QGLRefSys widget.
        * This implements the syncronization Vaa3D<-->TeraFly of rotations.
        ***********************************************************************************/
        void Vaa3D_rotationchanged(int s);
        void PMain_rotationchanged();

        #ifdef USE_EXPERIMENTAL_FEATURES
        /**********************************************************************************
        * Linked to Vaa3D renderer slider
        ***********************************************************************************/
        void setZoom(int z);
        #endif

    public:

        /**********************************************************************************
        * Returns  the  global coordinate  (which starts from 0) in  the given  resolution
        * volume image space given the local coordinate (which starts from 0) in the current
        * resolution volume image space. If resIndex is not set, the returned global coord-
        * inate will be in the highest resolution image space.
        ***********************************************************************************/
        inline int getGlobalVCoord(int localVCoord, int resIndex = -1, bool fromVaa3Dcoordinates = false, bool cutOutOfRange = false, const char *src =0)
        {
            #ifdef terafly_enable_debug_max_level
            /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, coord = %d, res = %d, fromVaa3D = %s, cutOutOfRange = %s, src = %s",
                                                titleShort.c_str(), localVCoord, resIndex,  fromVaa3Dcoordinates ? "true" : "false", cutOutOfRange ? "true" : "false", src ? src : "unknown").c_str(), __itm__current__function__);
            #endif

            //setting resIndex if it has not been set
            if(resIndex == -1)
                resIndex = CImport::instance()->getResolutions()-1;

            //cutting out-of-range coordinate, if <cutOutOfRange> is set
            if(cutOutOfRange)
            {
                localVCoord = localVCoord <  0                                                       ? 0                                                       : localVCoord;
                localVCoord = localVCoord >= CImport::instance()->getVolume(volResIndex)->getDIM_V() ? CImport::instance()->getVolume(volResIndex)->getDIM_V() : localVCoord;
            }

            //if the Vaa3D image size limit has been reached along this direction, mapping Vaa3D coordinates to the non-downsampled image space coordinate system
            if(fromVaa3Dcoordinates && (volV1-volV0 > LIMIT_VOLY))
                localVCoord = static_cast<int>(localVCoord* ( static_cast<float>(volV1-volV0-1)/(LIMIT_VOLY-1) ) +0.5f);

            //--- Alessandro 29/09/2013: fixing "division by zero" bug in case of volumes with 1 pixel size along the considered direction
            if(CImport::instance()->getVolume(volResIndex)->getDIM_V() == 1)
                return 0;

            float ratio = (CImport::instance()->getVolume(resIndex)->getDIM_V()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_V()-1.0f);

            #ifdef terafly_enable_debug_max_level
            /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, returning %d", titleShort.c_str(), static_cast<int>((volV0+localVCoord)*ratio + 0.5f)).c_str(), __itm__current__function__);
            #endif

            return (volV0+localVCoord)*ratio + 0.5f;
        }
        inline int getGlobalHCoord(int localHCoord, int resIndex = -1, bool fromVaa3Dcoordinates = false, bool cutOutOfRange = false, const char *src =0)
        {
            #ifdef terafly_enable_debug_max_level
            /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, coord = %d, res = %d, fromVaa3D = %s, cutOutOfRange = %s, src = %s",
                                                titleShort.c_str(), localHCoord, resIndex,  fromVaa3Dcoordinates ? "true" : "false", cutOutOfRange ? "true" : "false", src ? src : "unknown").c_str(), __itm__current__function__);
            #endif

            //setting resIndex if it has not been set
            if(resIndex == -1)
                resIndex = CImport::instance()->getResolutions()-1;

            //cutting out-of-range coordinate, if <cutOutOfRange> is set
            if(cutOutOfRange)
            {
                localHCoord = localHCoord <  0                                                       ? 0                                                       : localHCoord;
                localHCoord = localHCoord >= CImport::instance()->getVolume(volResIndex)->getDIM_H() ? CImport::instance()->getVolume(volResIndex)->getDIM_H() : localHCoord;
            }

            //if the Vaa3D image size limit has been reached along this direction, mapping Vaa3D coordinates to the non-downsampled image space coordinate system
            if(fromVaa3Dcoordinates && (volH1-volH0 > LIMIT_VOLX))
                localHCoord = static_cast<int>(localHCoord* ( static_cast<float>(volH1-volH0-1)/(LIMIT_VOLX-1) ) +0.5f);

            //--- Alessandro 29/09/2013: fixing "division by zero" bug in case of volumes with 1 pixel size along the considered direction
            if(CImport::instance()->getVolume(volResIndex)->getDIM_H() == 1)
                return 0;

            float ratio = (CImport::instance()->getVolume(resIndex)->getDIM_H()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_H()-1.0f);

            #ifdef terafly_enable_debug_max_level
            /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, returning %d", titleShort.c_str(), static_cast<int>((volH0+localHCoord)*ratio + 0.5f)).c_str(), __itm__current__function__);
            #endif

            return (volH0+localHCoord)*ratio + 0.5f;
        }
        inline int getGlobalDCoord(int localDCoord, int resIndex = -1, bool fromVaa3Dcoordinates = false, bool cutOutOfRange = false, const char *src =0)
        {
            #ifdef terafly_enable_debug_max_level
            /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, coord = %d, res = %d, fromVaa3D = %s, cutOutOfRange = %s, src = %s",
                                                titleShort.c_str(), localDCoord, resIndex,  fromVaa3Dcoordinates ? "true" : "false", cutOutOfRange ? "true" : "false", src ? src : "unknown").c_str(), __itm__current__function__);
            #endif

            //setting resIndex if it has not been set
            if(resIndex == -1)
                resIndex = CImport::instance()->getResolutions()-1;

            //cutting out-of-range coordinate, if <cutOutOfRange> is set
            if(cutOutOfRange)
            {
                localDCoord = localDCoord <  0                                                       ? 0                                                       : localDCoord;
                localDCoord = localDCoord >= CImport::instance()->getVolume(volResIndex)->getDIM_D() ? CImport::instance()->getVolume(volResIndex)->getDIM_D() : localDCoord;
            }

            //if the Vaa3D image size limit has been reached along this direction, mapping Vaa3D coordinates to the non-downsampled image space coordinate system
            if(fromVaa3Dcoordinates && (volD1-volD0 > LIMIT_VOLZ))
                localDCoord = static_cast<int>(localDCoord* ( static_cast<float>(volD1-volD0-1)/(LIMIT_VOLZ-1) ) +0.5f);

            //--- Alessandro 29/09/2013: fixing "division by zero" bug in case of volumes with 1 pixel size along the considered direction
            if(CImport::instance()->getVolume(volResIndex)->getDIM_D() == 1)
                return 0;

            float ratio = (CImport::instance()->getVolume(resIndex)->getDIM_D()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_D()-1.0f);

            #ifdef terafly_enable_debug_max_level
            /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, returning %d", titleShort.c_str(), static_cast<int>((volD0+localDCoord)*ratio + 0.5f)).c_str(), __itm__current__function__);
            #endif

            return (volD0+localDCoord)*ratio + 0.5f;
        }
        inline float getGlobalVCoord(float &localVCoord, int resIndex = -1, bool fromVaa3Dcoordinates = false, bool cutOutOfRange = false, const char *src =0)
        {
            #ifdef terafly_enable_debug_max_level
            /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, coord = %.2f, res = %d, fromVaa3D = %s, cutOutOfRange = %s, src = %s",
                                                titleShort.c_str(), localVCoord, resIndex,  fromVaa3Dcoordinates ? "true" : "false", cutOutOfRange ? "true" : "false", src ? src : "unknown").c_str(), __itm__current__function__);
            #endif

            //setting resIndex if it has not been set
            if(resIndex == -1)
                resIndex = CImport::instance()->getResolutions()-1;

            //cutting out-of-range coordinate, if <cutOutOfRange> is set
            if(cutOutOfRange)
            {
                localVCoord = localVCoord <  0                                                       ? 0                                                       : localVCoord;
                localVCoord = localVCoord >= CImport::instance()->getVolume(volResIndex)->getDIM_V() ? CImport::instance()->getVolume(volResIndex)->getDIM_V() : localVCoord;
            }

            //if the Vaa3D image size limit has been reached along this direction, mapping Vaa3D coordinates to the non-downsampled image space coordinate system
            if(fromVaa3Dcoordinates && (volV1-volV0 > LIMIT_VOLY))
                localVCoord *= static_cast<float>(volV1-volV0-1)/(LIMIT_VOLY-1);

            //--- Alessandro 29/09/2013: fixing "division by zero" bug in case of volumes with 1 pixel size along the considered direction
            if(CImport::instance()->getVolume(volResIndex)->getDIM_V() == 1)
                return 0.0f;

            float ratio = (CImport::instance()->getVolume(resIndex)->getDIM_V()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_V()-1.0f);

            #ifdef terafly_enable_debug_max_level
            /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, returning %.2f", titleShort.c_str(), (volV0+localVCoord)*ratio).c_str(), __itm__current__function__);
            #endif

            return (volV0+localVCoord)*ratio;
        }
        inline float getGlobalHCoord(float &localHCoord, int resIndex = -1, bool fromVaa3Dcoordinates = false, bool cutOutOfRange = false, const char *src =0)
        {
            #ifdef terafly_enable_debug_max_level
            /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, coord = %.2f, res = %d, fromVaa3D = %s, cutOutOfRange = %s, src = %s",
                                                titleShort.c_str(), localHCoord, resIndex,  fromVaa3Dcoordinates ? "true" : "false", cutOutOfRange ? "true" : "false", src ? src : "unknown").c_str(), __itm__current__function__);
            #endif

            //setting resIndex if it has not been set
            if(resIndex == -1)
                resIndex = CImport::instance()->getResolutions()-1;

            //cutting out-of-range coordinate, if <cutOutOfRange> is set
            if(cutOutOfRange)
            {
                localHCoord = localHCoord <  0                                                       ? 0                                                       : localHCoord;
                localHCoord = localHCoord >= CImport::instance()->getVolume(volResIndex)->getDIM_H() ? CImport::instance()->getVolume(volResIndex)->getDIM_H() : localHCoord;
            }

            //if the Vaa3D image size limit has been reached along this direction, mapping Vaa3D coordinates to the non-downsampled image space coordinate system
            if(fromVaa3Dcoordinates && (volH1-volH0 > LIMIT_VOLX))
                localHCoord *= static_cast<float>(volH1-volH0-1)/(LIMIT_VOLX-1);

            //--- Alessandro 29/09/2013: fixing "division by zero" bug in case of volumes with 1 pixel size along the considered direction
            if(CImport::instance()->getVolume(volResIndex)->getDIM_H() == 1)
                return 0.0f;

            float ratio = (CImport::instance()->getVolume(resIndex)->getDIM_H()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_H()-1.0f);

            #ifdef terafly_enable_debug_max_level
            /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, returning %.2f", titleShort.c_str(), (volH0+localHCoord)*ratio).c_str(), __itm__current__function__);
            #endif

            return (volH0+localHCoord)*ratio;
        }
        inline float getGlobalDCoord(float &localDCoord, int resIndex = -1, bool fromVaa3Dcoordinates = false, bool cutOutOfRange = false, const char *src =0)
        {
            #ifdef terafly_enable_debug_max_level
            /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, coord = %.2f, res = %d, fromVaa3D = %s, cutOutOfRange = %s, src = %s",
                                                titleShort.c_str(), localDCoord, resIndex,  fromVaa3Dcoordinates ? "true" : "false", cutOutOfRange ? "true" : "false", src ? src : "unknown").c_str(), __itm__current__function__);
            #endif

            //setting resIndex if it has not been set
            if(resIndex == -1)
                resIndex = CImport::instance()->getResolutions()-1;

            //cutting out-of-range coordinate, if <cutOutOfRange> is set
            if(cutOutOfRange)
            {
                localDCoord = localDCoord <  0                                                       ? 0                                                       : localDCoord;
                localDCoord = localDCoord >= CImport::instance()->getVolume(volResIndex)->getDIM_D() ? CImport::instance()->getVolume(volResIndex)->getDIM_D() : localDCoord;
            }

            //if the Vaa3D image size limit has been reached along this direction, mapping Vaa3D coordinates to the non-downsampled image space coordinate system
            if(fromVaa3Dcoordinates && (volD1-volD0 > LIMIT_VOLZ))
                localDCoord *= static_cast<float>(volD1-volD0-1)/(LIMIT_VOLZ-1);

            //--- Alessandro 29/09/2013: fixing "division by zero" bug in case of volumes with 1 pixel size along the considered direction
            if(CImport::instance()->getVolume(volResIndex)->getDIM_D() == 1)
                return 0.0f;

            float ratio = (CImport::instance()->getVolume(resIndex)->getDIM_D()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_D()-1.0f);

            #ifdef terafly_enable_debug_max_level
            /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, returning %.2f", titleShort.c_str(), (volD0+localDCoord)*ratio).c_str(), __itm__current__function__);
            #endif

            return (volD0+localDCoord)*ratio;
        }

        /**********************************************************************************
        * Returns the local coordinate (which starts from 0) in the current resolution vol-
        * ume image space given the global coordinate  (which starts from 0) in the highest
        * resolution volume image space.
        ***********************************************************************************/
        inline int getLocalVCoord(int highestResGlobalVCoord, bool toVaa3Dcoordinates  = false)
        {
            #ifdef terafly_enable_debug_max_level
            /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, coord = %d, toVaa3Dcoordinates = %s",
                                                titleShort.c_str(), highestResGlobalVCoord, toVaa3Dcoordinates ? "true" : "false").c_str(), __itm__current__function__);
            #endif

            float ratio = (CImport::instance()->getHighestResVolume()->getDIM_V()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_V()-1.0f);
            int localCoord =  static_cast<int>(highestResGlobalVCoord/ratio - volV0 + 0.5f);

            //if the Vaa3D image size limit has been reached along this direction, mapping coordinate to the downsampled image space coordinate system
            if(toVaa3Dcoordinates && (volV1-volV0 > LIMIT_VOLY))
                localCoord = static_cast<int>(localCoord* ( static_cast<float>(LIMIT_VOLY-1)/(volV1-volV0-1) ) +0.5f);

            #ifdef terafly_enable_debug_max_level
            /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, returning %d", titleShort.c_str(), localCoord).c_str(), __itm__current__function__);
            #endif

            return localCoord;
        }
        inline int getLocalHCoord(int highestResGlobalHCoord, bool toVaa3Dcoordinates  = false)
        {
            #ifdef terafly_enable_debug_max_level
            /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, coord = %d, toVaa3Dcoordinates = %s",
                                                titleShort.c_str(), highestResGlobalHCoord, toVaa3Dcoordinates ? "true" : "false").c_str(), __itm__current__function__);
            #endif

            float ratio = (CImport::instance()->getHighestResVolume()->getDIM_H()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_H()-1.0f);
            int localCoord =  static_cast<int>(highestResGlobalHCoord/ratio - volH0 + 0.5f);

            //if the Vaa3D image size limit has been reached along this direction, mapping coordinate to the downsampled image space coordinate system
            if(toVaa3Dcoordinates && (volH1-volH0 > LIMIT_VOLX))
                localCoord = static_cast<int>(localCoord* ( static_cast<float>(LIMIT_VOLX-1)/(volH1-volH0-1) ) +0.5f);

            #ifdef terafly_enable_debug_max_level
            /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, returning %d", titleShort.c_str(), localCoord).c_str(), __itm__current__function__);
            #endif

            return localCoord;
        }
        inline int getLocalDCoord(int highestResGlobalDCoord, bool toVaa3Dcoordinates  = false)
        {
            #ifdef terafly_enable_debug_max_level
            /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, coord = %d, toVaa3Dcoordinates = %s",
                                                titleShort.c_str(), highestResGlobalDCoord, toVaa3Dcoordinates ? "true" : "false").c_str(), __itm__current__function__);
            #endif

            float ratio = (CImport::instance()->getHighestResVolume()->getDIM_D()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_D()-1.0f);
            int localCoord =  static_cast<int>(highestResGlobalDCoord/ratio - volD0 + 0.5f);

            //if the Vaa3D image size limit has been reached along this direction, mapping coordinate to the downsampled image space coordinate system
            if(toVaa3Dcoordinates && (volD1-volD0 > LIMIT_VOLZ))
                localCoord = static_cast<int>(localCoord* ( static_cast<float>(LIMIT_VOLZ-1)/(volD1-volD0-1) ) +0.5f);

            #ifdef terafly_enable_debug_max_level
            /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, returning %d", titleShort.c_str(), localCoord).c_str(), __itm__current__function__);
            #endif

            return localCoord;
        }
        inline float getLocalVCoord(float &highestResGlobalVCoord, bool toVaa3Dcoordinates  = false)
        {
            #ifdef terafly_enable_debug_max_level
            /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, coord = %.2f, toVaa3Dcoordinates = %s",
                                                titleShort.c_str(), highestResGlobalVCoord, toVaa3Dcoordinates ? "true" : "false").c_str(), __itm__current__function__);
            #endif

            float ratio = (CImport::instance()->getHighestResVolume()->getDIM_V()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_V()-1.0f);
            float localCoord = highestResGlobalVCoord/ratio - volV0;

            //if the Vaa3D image size limit has been reached along this direction, mapping coordinate to the downsampled image space coordinate system
            if(toVaa3Dcoordinates && (volV1-volV0 > LIMIT_VOLY))
                localCoord = localCoord* ( static_cast<float>(LIMIT_VOLY-1)/(volV1-volV0-1) );

            #ifdef terafly_enable_debug_max_level
            /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, returning %.2f", titleShort.c_str(), localCoord).c_str(), __itm__current__function__);
            #endif

            return localCoord;
        }
        inline float getLocalHCoord(float &highestResGlobalHCoord, bool toVaa3Dcoordinates  = false)
        {
            #ifdef terafly_enable_debug_max_level
            /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, coord = %.2f, toVaa3Dcoordinates = %s",
                                                titleShort.c_str(), highestResGlobalHCoord, toVaa3Dcoordinates ? "true" : "false").c_str(), __itm__current__function__);
            #endif

            float ratio = (CImport::instance()->getHighestResVolume()->getDIM_H()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_H()-1.0f);
            float localCoord = highestResGlobalHCoord/ratio - volH0;

            //if the Vaa3D image size limit has been reached along this direction, mapping coordinate to the downsampled image space coordinate system
            if(toVaa3Dcoordinates && (volH1-volH0 > LIMIT_VOLX))
                localCoord = localCoord* ( static_cast<float>(LIMIT_VOLX-1)/(volH1-volH0-1) );

            #ifdef terafly_enable_debug_max_level
            /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, returning %.2f", titleShort.c_str(), localCoord).c_str(), __itm__current__function__);
            #endif

            return localCoord;
        }
        inline float getLocalDCoord(float &highestResGlobalDCoord, bool toVaa3Dcoordinates  = false)
        {
            #ifdef terafly_enable_debug_max_level
            /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, coord = %.2f, toVaa3Dcoordinates = %s",
                                                titleShort.c_str(), highestResGlobalDCoord, toVaa3Dcoordinates ? "true" : "false").c_str(), __itm__current__function__);
            #endif

            float ratio = (CImport::instance()->getHighestResVolume()->getDIM_D()-1.0f)/(CImport::instance()->getVolume(volResIndex)->getDIM_D()-1.0f);
            float localCoord = highestResGlobalDCoord/ratio - volD0;

            //if the Vaa3D image size limit has been reached along this direction, mapping coordinate to the downsampled image space coordinate system
            if(toVaa3Dcoordinates && (volD1-volD0 > LIMIT_VOLZ))
                localCoord = localCoord* ( static_cast<float>(LIMIT_VOLZ-1)/(volD1-volD0-1) );

            #ifdef terafly_enable_debug_max_level
            /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, returning %.2f", titleShort.c_str(), localCoord).c_str(), __itm__current__function__);
            #endif

            return localCoord;
        }
};

#endif // CEXPLORERWINDOW_H
