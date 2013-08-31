#ifdef USE_EXPERIMENTAL_FEATURES

#include "V3Dsubclasses.h"
#include "../presentation/PMain.h"
#include "CExplorerWindow.h"
#include "v3d_imaging_para.h"
#include "renderer_gl1.h"

using namespace teramanager;
using namespace std;

//converts mouse 2D position into image 3D point
XYZ myRenderer_gl1::get3DPoint(int x, int y)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> myRenderer_gl1[%s]::get3DPoint(%d, %d)\n", "unknown", x, y);
    #endif

    Renderer_gl1::MarkerPos pos;
    pos.x = x;
    pos.y = y;
    for (int i=0; i<4; i++)
            pos.view[i] = this->viewport[i];
    for (int i=0; i<16; i++)
    {
        pos.P[i]  = this->projectionMatrix[i];
        pos.MV[i] = this->markerViewMatrix[i];
    }
    return this->getCenterOfMarkerPos(pos);
}

void myV3dR_GLWidget::setZoomO(int zr)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> myV3dR_GLWidget[%s]::setZoomO(%d)\n", data_title.toStdString().c_str(), zr);
    #endif

    //qDebug("V3dR_GLWidget::setZoom = %i",zr);
    zr = CLAMP(-ZOOM_RANGE, ZOOM_RANGE, zr);
    if (int(this->_zoom) != zr) {
        this->_zoom = zr;
        if (this->renderer)
        {
            if (zr>PMain::getInstance()->zoomInSens->value() && !CExplorerWindow::getCurrent()->isHighestRes())
                zoomIn(PMain::getInstance()->zoomInMethod->currentText().toStdString().c_str());
            else
                this->renderer->setZoom( +float(zr)/100.f * ZOOM_RANGE_RATE); //sign can switch zoom orientation
        }
        emit zoomChanged(zr);
        this->update();
    }
}

void myV3dR_GLWidget::setZoomO(float zr)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> myV3dR_GLWidget[%s]::setZoomO(%.0f)\n", data_title.toStdString().c_str(), zr);
    #endif

    //qDebug("V3dR_GLWidget::setZoom = %i",zr);
    zr = CLAMP(-ZOOM_RANGE, ZOOM_RANGE, zr);
    if (this->_zoom != zr) {
        this->_zoom = zr;
        if (this->renderer)
        {
            if (zr>PMain::getInstance()->zoomInSens->value() && !CExplorerWindow::getCurrent()->isHighestRes())
                zoomIn(PMain::getInstance()->zoomInMethod->currentText().toStdString().c_str());
            else
                this->renderer->setZoom( +float(zr)/100.f * ZOOM_RANGE_RATE); //sign can switch zoom orientation
        }
        emit zoomChanged(zr);
        this->update();
    }
}

void myV3dR_GLWidget::wheelEventO(QWheelEvent *event)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> myV3dR_GLWidget[%s]::wheelEventO(%d, %d)\n", data_title.toStdString().c_str(), event->pos().x(), event->pos().y());
    #endif

    this->setFocus(); // accept KeyPressEvent, by RZC 081028

    float d = (event->delta())/100;  // ~480
    #define MOUSE_ZOOM(dz)    (int(dz*4* MOUSE_SENSITIVE));
    #define MOUSE_ZROT(dz)    (int(dz*8* MOUSE_SENSITIVE));

    int zoomStep = MOUSE_ZOOM(d);
    int zRotStep = MOUSE_ZROT(d);

    if (QApplication::keyboardModifiers() == Qt::ShiftModifier) // shift+mouse control view space translation, 081104
    {
        this->viewRotation(0, 0, zRotStep);
    }
    else if (QApplication::keyboardModifiers()==Qt::AltModifier || QApplication::keyboardModifiers()==ALT2_MODIFIER) // alt+mouse control model space rotation, 081104
    {
        this->modelRotation(0, 0, zRotStep);
    }
    else // default
    {
        (this->renderer->hitWheel(event->x(), event->y())); //by PHC, 130424. record the wheel location when zoom-in or out
        setZoomO((-zoomStep) + this->_zoom); // scroll down to zoom in

        //---- Alessandro 2013-04-29: first attempt to combine zoom and translation using mouse position (like Google Earth does). Does not work properly.
//        int dx = (myRenderer_gl1::cast(renderer)->wheelPos.view[0]+myRenderer_gl1::cast(renderer)->wheelPos.view[2])/2.0 - event->x();
//        int dy = (myRenderer_gl1::cast(renderer)->wheelPos.view[1]+myRenderer_gl1::cast(renderer)->wheelPos.view[3])/2.0 - event->y();
//        printf("DX = %d, event->x() = %d, centerX = %.0f\n", dx, event->x(), (myRenderer_gl1::cast(renderer)->wheelPos.view[0]+myRenderer_gl1::cast(renderer)->wheelPos.view[2])/2.0);
//        printf("DY = %d, event->y() = %d, centerY = %.0f\n", dy, event->y(), (myRenderer_gl1::cast(renderer)->wheelPos.view[1]+myRenderer_gl1::cast(renderer)->wheelPos.view[3])/2.0);
//        int xShiftStep = (int(SHIFT_RANGE*2* float(dx)/viewW));
//        int yShiftStep = (int(SHIFT_RANGE*2* float(dy)/viewH));
//        setXShift(_xShift + xShiftStep);// move +view -model
//        setYShift(_yShift - yShiftStep);// move -view +model
    }

    event->accept();
}

//zoomIn method(s)
void myV3dR_GLWidget::zoomIn(const char* method)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> myV3dR_GLWidget[%s]::zoomInmethod=\"%s\")\n", data_title.toStdString().c_str(), method);
    #endif

    if(strcmp(method, "WYSIWYG (10 markers)") == 0)
        this->renderer->zoomview_wheel_event();
    else if(strcmp(method, "Foreground (1 marker)") == 0)
    {
        XYZ centralPoint = myRenderer_gl1::cast(static_cast<Renderer_gl1*>(this->getRenderer()))->get3DPoint(this->viewW/2, this->viewH/2);
        v3d_imaging_paras* roi = new v3d_imaging_paras;
        roi->ops_type = 2;
        roi->xs = centralPoint.x - PMain::getInstance()->Hdim_sbox->value()/2;
        roi->xe = centralPoint.x + PMain::getInstance()->Hdim_sbox->value()/2;
        roi->ys = centralPoint.y - PMain::getInstance()->Vdim_sbox->value()/2;
        roi->ye = centralPoint.y + PMain::getInstance()->Vdim_sbox->value()/2;
        roi->zs = centralPoint.z - PMain::getInstance()->Ddim_sbox->value()/2;
        roi->ze = centralPoint.z + PMain::getInstance()->Ddim_sbox->value()/2;
        if(CExplorerWindow::getCurrent())
            CExplorerWindow::getCurrent()->invokedFromVaa3D(roi);

    }
    else
        QMessageBox::critical(PMain::getInstance(),QObject::tr("Error"), QString("Unsupported zoom-in method \"").append(method).append("\""),QObject::tr("Ok"));
}

//fast version (without displaying progress bar) of updateImageData method
void myV3dR_GLWidget::updateImageDataFast()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> myV3dR_GLWidget[%s]::updateImageDataFast()\n", data_title.toStdString().c_str());
    #endif

    renderer->setupData(_idep);
    static_cast<Renderer_gl1*>(renderer)->loadVol();
    update();
}

#endif // USE_EXPERIMENTAL_FEATURES
