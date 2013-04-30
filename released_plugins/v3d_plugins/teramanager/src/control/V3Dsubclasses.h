//#ifndef V3DSUBCLASSES_H
//#define V3DSUBCLASSES_H

//#include "CPlugin.h"
//#include "renderer_gl2.h"
//#include "v3dr_glwidget.h"
//#include "v3dr_mainwindow.h"

///**********************************************************************************
//* Vaa3D subclasses needed to access/override protected members/methods
//***********************************************************************************/

//class teramanager::myRenderer_gl1
//{
//    public:

//        //gives public access to members
//        friend class CExplorerWindow;
//        friend class myV3dR_GLWidget;

//        //converts mouse 2D position into image 3D point
//        static XYZ get3DPoint(Renderer_gl1* handle, int x, int y);

//        //casting
//        //--- note ---: dynamic_cast would be better, but needs too many Vaa3D sources to be included
////        static myRenderer_gl1* cast(Renderer_gl1* instance){return static_cast<myRenderer_gl1*>(instance);}
////        static myRenderer_gl1* cast(Renderer* instance){return static_cast<myRenderer_gl1*>(instance);}
//};

//class teramanager::myV3dR_GLWidget
//{
////    Q_OBJECT

//    public:

//        //gives public access to members
//        friend class CExplorerWindow;

//        //casting
//        //--- note ---: dynamic_cast would be better, but needs too many Vaa3D sources to be included
////        static myV3dR_GLWidget* cast(V3dR_GLWidget* instance){return static_cast<myV3dR_GLWidget*>(instance);}

//        //@Override
//        static void wheelEventO(V3dR_GLWidget* handle, QWheelEvent *event);

//        //zoomIn method(s)
//        static void zoomIn(V3dR_GLWidget* handle, const char* method);

//    public:

//        //@Override
//        static void setZoomO(V3dR_GLWidget* handle, int zr);
//        static void setZoomO(V3dR_GLWidget* handle, float zr);
//};

////class teramanager::myV3dR_MainWindow
////{
////    public:

////        //gives public access to members
////        friend class CExplorerWindow;

////        //casting
////        //--- note ---: dynamic_cast would be better, but needs too many Vaa3D sources to be included
////        static myV3dR_MainWindow* cast(V3dR_MainWindow* instance){return static_cast<myV3dR_MainWindow*>(instance);}
////};

////class teramanager::myImage4DSimple : Image4DSimple
////{
////public:

////    //gives public access to members
////    friend class CExplorerWindow;
////    friend class myV3dR_GLWidget;

////    //casting
////    //--- note ---: dynamic_cast would be better, but needs too many Vaa3D sources to be included
////    static myImage4DSimple* cast(Image4DSimple* instance){return static_cast<myImage4DSimple*>(instance);}
////};

//#endif // V3DSUBCLASSES_H
