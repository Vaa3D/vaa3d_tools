#ifndef V3DSUBCLASSES_H
#define V3DSUBCLASSES_H

#include "CPlugin.h"
#include "renderer_gl2.h"
#include "v3dr_glwidget.h"
#include "v3dr_mainwindow.h"

/**********************************************************************************
* Vaa3D subclasses needed to access/override protected members/methods
***********************************************************************************/

class teramanager::myRenderer_gl1 : Renderer_gl1
{
    public:

        //gives public access to members
        friend class CExplorerWindow;

        //converts mouse 2D position into image 3D point
        XYZ get3DPoint(int x, int y);

        //casting
        static myRenderer_gl1* cast(Renderer_gl1* instance){return static_cast<myRenderer_gl1*>(instance);}
        static myRenderer_gl1* cast(Renderer* instance){return static_cast<myRenderer_gl1*>(instance);}
};

class teramanager::myV3dR_GLWidget : V3dR_GLWidget
{
    Q_OBJECT

    public:

        //gives public access to members
        friend class CExplorerWindow;

        //casting
        static myV3dR_GLWidget* cast(V3dR_GLWidget* instance){return static_cast<myV3dR_GLWidget*>(instance);}

        //@Override
        void wheelEventO(QWheelEvent *event);

    public slots:

        //@Override
        void setZoomO(int zr);
        void setZoomO(float zr);
};

class teramanager::myV3dR_MainWindow : V3dR_MainWindow
{
    public:

        //gives public access to members
        friend class CExplorerWindow;

        //casting
        static myV3dR_MainWindow* cast(V3dR_MainWindow* instance){return static_cast<myV3dR_MainWindow*>(instance);}
};

#endif // V3DSUBCLASSES_H
