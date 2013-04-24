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
#include "v3dr_glwidget.h"
#include "v3dr_mainwindow.h"
#include "renderer_gl2.h"

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
        int volV0, volV1;               //first and last vertical coordinates of the volume displayed in the current window
        int volH0, volH1;               //first and last horizontal coordinates of the volume displayed in the current window
        int volD0, volD1;               //first and last depth coordinates of the volume displayed in the current window
        int nchannels;                  //number of image channels
        string title;                   //title of current window
        bool toBeClosed;                //true when the current window is marked as going to be closed
        bool isActive;                  //false when the current window is set as not active (e.g. when after zooming-in/out)
        int zoomHistory[ZOOM_HISTORY_SIZE];//last 4 zoom values
        std::list<LocationSimple> loaded_markers; //list of markers loaded from <CAnnotations> when the current view is created
        std::list<NeuronSWC> loaded_curves;       //list of curve points loaded from <CAnnotations> when the current view is created
        int V0_sbox_min, V0_sbox_val;   //to save the state of subvolume spinboxes when the current window is hidden
        int V1_sbox_max, V1_sbox_val;   //to save the state of subvolume spinboxes when the current window is hidden
        int H0_sbox_min, H0_sbox_val;   //to save the state of subvolume spinboxes when the current window is hidden
        int H1_sbox_max, H1_sbox_val;   //to save the state of subvolume spinboxes when the current window is hidden
        int D0_sbox_min, D0_sbox_val;   //to save the state of subvolume spinboxes when the current window is hidden
        int D1_sbox_max, D1_sbox_val;   //to save the state of subvolume spinboxes when the current window is hidden

        //CLASS members
        static CExplorerWindow *first;  //pointer to the first window of the multiresolution explorer windows chain
        static CExplorerWindow *last;   //pointer to the last window of the multiresolution explorer windows chain
        static CExplorerWindow *current;//pointer to the current window of the multiresolution explorer windows chain
        static int nInstances;          //number of instantiated objects

        //inhibiting default constructor
        CExplorerWindow();

        /**********************************************************************************
        * Returns  the  global coordinate  (which starts from 0) in  the given  resolution
        * volume image space given the local coordinate (which starts from 0) in the current
        * resolution volume image space. If resIndex is not set, the returned global coord-
        * inate will be in the highest resolution image space.
        ***********************************************************************************/
        int getGlobalVCoord(int localVCoord, int resIndex = -1, bool fromVaa3Dcoordinates = false);
        int getGlobalHCoord(int localHCoord, int resIndex = -1, bool fromVaa3Dcoordinates = false);
        int getGlobalDCoord(int localDCoord, int resIndex = -1, bool fromVaa3Dcoordinates = false);
        float getGlobalVCoord(float localVCoord, int resIndex = -1, bool fromVaa3Dcoordinates = false);
        float getGlobalHCoord(float localHCoord, int resIndex = -1, bool fromVaa3Dcoordinates = false);
        float getGlobalDCoord(float localDCoord, int resIndex = -1, bool fromVaa3Dcoordinates = false);

        /**********************************************************************************
        * Returns the local coordinate (which starts from 0) in the current resolution vol-
        * ume image space given the global coordinate  (which starts from 0) in the highest
        * resolution volume image space.
        ***********************************************************************************/
        int getLocalVCoord(int highestResGlobalVCoord);
        int getLocalHCoord(int highestResGlobalHCoord);
        int getLocalDCoord(int highestResGlobalDCoord);
        float getLocalVCoord(float highestResGlobalVCoord);
        float getLocalHCoord(float highestResGlobalHCoord);
        float getLocalDCoord(float highestResGlobalDCoord);


        /**********************************************************************************
        * Estimates actual image voxel size (in pixels) of the 3D renderer.
        * This depends on the zoom and on the dimensions of the 3D  renderer. This calcula-
        * tion is based on MATLAB curve  fitting results on  a set of measures manually ex-
        * tracted on the renderer.
        ***********************************************************************************/
        float estimateRendererVoxelSize();

        /**********************************************************************************
        * Returns the most  likely 3D  point in the  image that the user is pointing on the
        * renderer at the given location.
        * This is based on the Vaa3D 3D point selection with one mouse click.
        ***********************************************************************************/
        XYZ getRenderer3DPoint(int x, int y) throw (MyException);

        /**********************************************************************************
        * Vaa3D Renderer_gl1 subclass needed to access some protected methods to be used to
        * select a 3D point in the image space given the coordinates of the mouse cursor.
        ***********************************************************************************/
        //---------- feature disabled because the necessary Vaa3D sources can't be included
//        class myRenderer : Renderer_gl1
//        {
//            public:

//                friend class CExplorerWindow;
//                XYZ get3DPoint(int x, int y)
//                {
//                    MarkerPos pos;
//                    pos.x = x;
//                    pos.y = y;
//                    for (int i=0; i<4; i++)
//                            pos.view[i] = viewport[i];
//                    for (int i=0; i<16; i++)
//                    {
//                        pos.P[i]  = projectionMatrix[i];
//                        pos.MV[i] = markerViewMatrix[i];
//                    }
//                    return this->getCenterOfMarkerPos(pos);
//                }
//        };

        /**********************************************************************************
        * Vaa3D V3dR_MainWindow subclass needed to access some protected members
        ***********************************************************************************/
        class myV3dR_MainWindow : V3dR_MainWindow
        {
            public:

            friend class CExplorerWindow;
        };



    public:

        //CONSTRUCTOR, DECONSTRUCTOR
        CExplorerWindow(V3DPluginCallback2* _V3D_env, int _resIndex, uint8* imgData, int _volV0, int _volV1,
                        int _volH0, int _volH1, int _volD0, int _volD1, int _nchannels, CExplorerWindow* _prev);
        ~CExplorerWindow();
        static void uninstance()
        {
            #ifdef TMP_DEBUG
            printf("--------------------- teramanager plugin [thread unknown] >> CExplorerWindow uninstance() launched\n");
            #endif
            while(first)
            {
                CExplorerWindow* p = first->next;
                first->toBeClosed = true;
                delete first;
                first = p;
            }
            first=last=0;
        }

        //GET methods
        //static CExplorerWindow* getLast(){return last;}
        static CExplorerWindow* getCurrent(){return current;}
        int getResIndex(){return volResIndex;}
        V3dR_MainWindow* getWindow3D(){return window3D;}

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
        void restoreViewFrom(CExplorerWindow* source) throw (MyException);

        /**********************************************************************************
        * Generates new view centered at the given 3D point on the given resolution and ha-
        * ving the given dimensions (optional).  VOI's dimensions from the GUI will be used
        * if dx,dy,dz are not provided.
        * Called by the current <CExplorerWindow> when the user zooms in and the higher res-
        * lution has to be loaded.
        ***********************************************************************************/
        void newView(int x, int y, int z, int resolution, bool fromVaa3Dcoordinates = false,
                     int dx=-1, int dy=-1, int dz=-1);

        /**********************************************************************************
        * Makes the current view the last one by  deleting (and deallocting) its subsequent
        * views.
        ***********************************************************************************/
        void makeLastView() throw (MyException);

        /**********************************************************************************
        * Annotations are stored/loaded) to/from the <CAnnotations> object
        ***********************************************************************************/
        void storeAnnotations() throw (MyException);
        void loadAnnotations() throw (MyException);

        /**********************************************************************************
        * Saves/restores the state of PMain spinboxes for subvolume selection
        ***********************************************************************************/
        void saveSubvolSpinboxState();
        void restoreSubvolSpinboxState();

        /**********************************************************************************
        * Linked to rightStrokeROI and rightClickROI right-menu entries of the 3D renderer.
        * This implements the selection of a ROI in the 3D renderer.
        ***********************************************************************************/
        void Vaa3D_selectedROI();

        /**********************************************************************************
        * Alignes the given widget to the left(right) of the current window
        ***********************************************************************************/
        void alignToLeft(QWidget* widget);
        void alignToRight(QWidget* widget);

        /**********************************************************************************
        * Activates / deactives the current window (in terms of responding to events)
        ***********************************************************************************/
        void setActive(bool active)
        {
            #ifdef TMP_DEBUG
            printf("--------------------- teramanager plugin [thread %d] >> CExplorerWindow[\"%s\"] setActive(%s)\n",
                   this->thread()->currentThreadId(), title.c_str() , active ? "true" : "false");
            #endif

            isActive = active;
        }

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
        inline bool zoomHistoryPushBack(int zoom){
            if(zoomHistory[ZOOM_HISTORY_SIZE-1] != zoom)
                for(int i=0; i<ZOOM_HISTORY_SIZE-1; i++)
                    zoomHistory[i] = zoomHistory[i+1];
            zoomHistory[ZOOM_HISTORY_SIZE-1] = zoom;
        }

        //PMain instance is allowed to access class private members
        friend class PMain;


    public slots:

        /**********************************************************************************
        * Called by the signal emitted by <CVolume> when the associated  operation has been
        * performed. If an exception has occurred in the <CVolume> thread, it is propagated
        * and managed in the current thread (ex != 0).
        ***********************************************************************************/
        void loadingDone(MyException *ex, void* sourceObject);

        /**********************************************************************************
        * Linked to volume cut scrollbars of Vaa3D widget containing the 3D renderer.
        * This implements the syncronization Vaa3D-->TeraManager of subvolume selection.
        ***********************************************************************************/
        void Vaa3D_changeXCut0(int s);
        void Vaa3D_changeXCut1(int s);
        void Vaa3D_changeYCut0(int s);
        void Vaa3D_changeYCut1(int s);
        void Vaa3D_changeZCut0(int s);
        void Vaa3D_changeZCut1(int s);

        /**********************************************************************************
        * Linked to PMain GUI VOI's widgets.
        * This implements the syncronization TeraManager-->Vaa3D of subvolume selection.
        ***********************************************************************************/
        void PMain_changeV0sbox(int s);
        void PMain_changeV1sbox(int s);
        void PMain_changeH0sbox(int s);
        void PMain_changeH1sbox(int s);
        void PMain_changeD0sbox(int s);
        void PMain_changeD1sbox(int s);
};

#endif // CEXPLORERWINDOW_H
