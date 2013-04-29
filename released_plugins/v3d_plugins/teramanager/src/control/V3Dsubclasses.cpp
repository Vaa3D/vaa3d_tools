#include "V3Dsubclasses.h"
#include "../presentation/PMain.h"
#include "CExplorerWindow.h"

using namespace teramanager;
using namespace std;

//converts mouse 2D position into image 3D point
XYZ myRenderer_gl1::get3DPoint(int x, int y)
{
    MarkerPos pos;
    pos.x = x;
    pos.y = y;
    for (int i=0; i<4; i++)
            pos.view[i] = viewport[i];
    for (int i=0; i<16; i++)
    {
        pos.P[i]  = projectionMatrix[i];
        pos.MV[i] = markerViewMatrix[i];
    }
    return this->getCenterOfMarkerPos(pos);
}

void myV3dR_GLWidget::setZoomO(int zr)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> myV3dR_GLWidget::setZoomO(%d)\n", zr);
    #endif

    //qDebug("V3dR_GLWidget::setZoom = %i",zr);
    zr = CLAMP(-ZOOM_RANGE, ZOOM_RANGE, zr);
    if (int(_zoom) != zr) {
        _zoom = zr;
        if (renderer)
        {
            if (zr>PMain::getInstance()->zoomInSens->value() && !CExplorerWindow::getCurrent()->isHighestRes())
            {
                renderer->zoomview_wheel_event();
            }
            else
                renderer->setZoom( +float(zr)/100.f * ZOOM_RANGE_RATE); //sign can switch zoom orientation
        }
        emit zoomChanged(zr);
        update();
    }
}

void myV3dR_GLWidget::setZoomO(float zr)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> myV3dR_GLWidget::setZoomO(%.0f)\n", zr);
    #endif

    //qDebug("V3dR_GLWidget::setZoom = %i",zr);
    zr = CLAMP(-ZOOM_RANGE, ZOOM_RANGE, zr);
    if (_zoom != zr) {
        _zoom = zr;
        if (renderer)
        {
            if (zr>PMain::getInstance()->zoomInSens->value() && !CExplorerWindow::getCurrent()->isHighestRes())
            {
                renderer->zoomview_wheel_event();
            }
            else
                renderer->setZoom( +float(zr)/100.f * ZOOM_RANGE_RATE); //sign can switch zoom orientation
        }
        emit zoomChanged(int(zr));
        update();
    }
}

void myV3dR_GLWidget::wheelEventO(QWheelEvent *event)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> myV3dR_GLWidget::wheelEventO(%d, %d)\n", event->pos().x(), event->pos().y());
    #endif

    setFocus(); // accept KeyPressEvent, by RZC 081028

    float d = (event->delta())/100;  // ~480
    #define MOUSE_ZOOM(dz)    (int(dz*4* MOUSE_SENSITIVE));
    #define MOUSE_ZROT(dz)    (int(dz*8* MOUSE_SENSITIVE));

    int zoomStep = MOUSE_ZOOM(d);
    int zRotStep = MOUSE_ZROT(d);

    if (QApplication::keyboardModifiers() == Qt::ShiftModifier) // shift+mouse control view space translation, 081104
    {
        viewRotation(0, 0, zRotStep);
    }
    else if (QApplication::keyboardModifiers()==Qt::AltModifier || QApplication::keyboardModifiers()==ALT2_MODIFIER) // alt+mouse control model space rotation, 081104
    {
        modelRotation(0, 0, zRotStep);
    }
    else // default
    {
        (renderer->hitWheel(event->x(), event->y())); //by PHC, 130424. record the wheel location when zoom-in or out
        this->setZoomO((-zoomStep) + _zoom); // scroll down to zoom in
    }

    event->accept();
}
