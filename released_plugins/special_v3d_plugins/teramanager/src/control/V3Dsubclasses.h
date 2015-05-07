#ifdef USE_EXPERIMENTAL_FEATURES

#ifndef V3DSUBCLASSES_H
#define V3DSUBCLASSES_H

#include "CPlugin.h"
#include "renderer_gl1.h"
#include "renderer_gl2.h"
#include "v3dr_glwidget.h"
#include "v3dr_mainwindow.h"
#include "v3d_imaging_para.h"

/**********************************************************************************
* Vaa3D subclasses needed to access/override protected members/methods
***********************************************************************************/

class teramanager::myRenderer_gl1 : public Renderer_gl1
{
    public:

        //gives public access to members
        friend class CViewer;
        friend class myV3dR_GLWidget;

        //converts mouse 2D position into image 3D point
        XYZ get3DPoint(int x, int y);

        //casting
        //--- note ---: dynamic_cast would be better, but needs too many Vaa3D sources to be included
        static myRenderer_gl1* cast(Renderer_gl1* instance){return static_cast<myRenderer_gl1*>(instance);}
        static myRenderer_gl1* cast(Renderer* instance){return static_cast<myRenderer_gl1*>(instance);}
};

class teramanager::myV3dR_GLWidget : public V3dR_GLWidget
{
    Q_OBJECT

    public:

        //gives public access to members
        friend class CViewer;

        //casting
        //--- note ---: dynamic_cast would be better, but needs too many Vaa3D sources to be included
        static myV3dR_GLWidget* cast(V3dR_GLWidget* instance){return static_cast<myV3dR_GLWidget*>(instance);}

        //@Override
        void wheelEventO(QWheelEvent *event);

        //zoomIn method(s)
        void zoomIn(const char* method);

        //fast version (without displaying progress bar) of updateImageData method
        void updateImageDataFast();

    public:

        //@Override
        void setZoomO(int zr);
        void setZoomO(float zr);
};

class teramanager::myV3dR_MainWindow : public V3dR_MainWindow
{
    public:

        //gives public access to members
        friend class CViewer;

        //casting
        //--- note ---: dynamic_cast would be better, but needs too many Vaa3D sources to be included
        static myV3dR_MainWindow* cast(V3dR_MainWindow* instance){return static_cast<myV3dR_MainWindow*>(instance);}
};

class teramanager::myImage4DSimple : Image4DSimple
{
    public:

        //gives public access to members
        friend class CViewer;
        friend class myV3dR_GLWidget;

        //casting
        //--- note ---: dynamic_cast would be better, but needs too many Vaa3D sources to be included
        static myImage4DSimple* cast(Image4DSimple* instance){return static_cast<myImage4DSimple*>(instance);}
};


struct teramanager::point
{
    float x,y,z;
    point(void) : x(0.0f), y(0.0f), z(0.0f){}
    point(XYZ &p) : x(p.x), y(p.y), z(p.z){}

    bool operator == (const point &p) const{
        return p.x == x && p.y == y && p.z == z;
    }

    bool operator <  (const point &p) const{
        return p.x < x && p.y < y && p.z < z;
    }
};

#endif // V3DSUBCLASSES_H

#endif // USE_EXPERIMENTAL_FEATURES
