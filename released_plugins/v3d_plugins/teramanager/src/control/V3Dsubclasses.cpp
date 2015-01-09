#ifdef USE_EXPERIMENTAL_FEATURES

#include "V3Dsubclasses.h"
#include "../presentation/PMain.h"
#include "CViewer.h"

using namespace teramanager;
using namespace std;

//converts mouse 2D position into image 3D point
XYZ myRenderer_gl1::get3DPoint(int x, int y)
{
    /**/itm::debug(itm::LEV1, strprintf("x = %d, y = %d", x, y).c_str(), __itm__current__function__);

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
    /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, zoom = %d", data_title.toStdString().c_str(), zr).c_str(), __itm__current__function__);

    //qDebug("V3dR_GLWidget::setZoom = %i",zr);
    zr = CLAMP(-ZOOM_RANGE, ZOOM_RANGE, zr);
    if (int(this->_zoom) != zr) {
        this->_zoom = zr;
        if (this->renderer)
        {
            if (zr>PMain::getInstance()->zoomInSens->value() && !CViewer::getCurrent()->isHighestRes())
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
    /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, zoom = %.2f", data_title.toStdString().c_str(), zr).c_str(), __itm__current__function__);

    //qDebug("V3dR_GLWidget::setZoom = %i",zr);
    zr = CLAMP(-ZOOM_RANGE, ZOOM_RANGE, zr);
    if (this->_zoom != zr) {
        this->_zoom = zr;
        if (this->renderer)
        {
            if (zr>PMain::getInstance()->zoomInSens->value() && !CViewer::getCurrent()->isHighestRes())
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
    /**/itm::debug(itm::LEV_MAX, strprintf("title = %s, mouse_x = %d, mouse_y = %d",
                                           data_title.toStdString().c_str(), event->pos().x(), event->pos().y()).c_str(), __itm__current__function__);

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
    /**/itm::debug(itm::LEV1, strprintf("title = %s, method = %s", data_title.toStdString().c_str(), method).c_str(), __itm__current__function__);

    if(strcmp(method, "WYSIWYG (5 markers)") == 0)
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
        if(CViewer::getCurrent())
            CViewer::getCurrent()->invokedFromVaa3D(roi);

    }
    else if(strcmp(method, "Foreground (20 markers + mean-shift)") == 0)
    {
        // parameters
        float dperc = 0.1f;             // distance to center of the viewport, expressed in terms of fraction of the viewport diagonal
        int n = 20;                     // number of points
        int msradius = 100;             // mean-shift kernel radius (in pixels)
        float shift_tolerance = 0.1f;   // mean-shift shift detection threshold (in pixels)

        QElapsedTimer timer;
        timer.start();

        //finding the n points in the cylinder of radius r centered on the viewport
        std::vector<XYZ> points;
        int dmax   = static_cast<int>(sqrt(static_cast<float>(viewW*viewW + viewH*viewH))*dperc +0.5f);
        int centerX = viewW/2;
        int centerY = viewH/2;
        for(int i=0; i<n; i++)
        {
            double alpha = ( (2*pi) / n) * i;
            double r = rand()%dmax;
            int px = static_cast<int>(centerX + r*cos(alpha) +0.5f);
            int py = static_cast<int>(centerY + r*sin(alpha) +0.5f);
            points.push_back(myRenderer_gl1::cast(static_cast<Renderer_gl1*>(this->getRenderer()))->get3DPoint(px, py));
        }

        //mean-shift clustering with kernel K(s)= { 1 if ||s|| <= msradius, 0 otherwise }
        std::vector<XYZ> points_shifted = points;
        bool shifted = true;
        while(shifted)
        {
            shifted = false;
            for(int i=0; i<n; i++)
            {
                float xi = points[i].x;
                float yi = points[i].y;
                float zi = points[i].z;
                points_shifted[i].x = points_shifted[i].y = points_shifted[i].z = 0;
                int N = 0;

                for(int j=0; j<n; j++)
                {
                    float xj = points[j].x;
                    float yj = points[j].y;
                    float zj = points[j].z;

                    if( sqrt( (xj-xi)*(xj-xi) + (yj-yi)*(yj-yi) + (zj-zi)*(zj-zi) ) <= msradius)
                    {
                        points_shifted[i].x += xj;
                        points_shifted[i].y += yj;
                        points_shifted[i].z += zj;
                        N++;
                    }
                }

                // N should never be 0, because at least one point is within the ball centered on the ith point (i.e., the point itself)
                points_shifted[i].x /= N;
                points_shifted[i].y /= N;
                points_shifted[i].z /= N;

                // detect shift
                if(fabs(points_shifted[i].x - points[i].x) > shift_tolerance ||
                   fabs(points_shifted[i].y - points[i].y) > shift_tolerance ||
                   fabs(points_shifted[i].z - points[i].z) > shift_tolerance)
                    shifted = true;
            }

            points = points_shifted;
        }
        std:map<point, int> votes;
        for(int i=0; i<n; i++)
            votes[point(points[i])]++;

        point thepoint;
        int mostvoted = 0;
        for(std::map<point, int>::iterator it = votes.begin(); it != votes.end(); it++)
        {
            if(it->second > mostvoted)
            {
                thepoint  = it->first;
                mostvoted = it->second;
            }
        }

        //QMessageBox::information(this, "Mean-shift based zoom-in", strprintf("ROI center is (%.0f, %.0f, %.0f) with %d/%d votes (%d ms)\n", thepoint.x, thepoint.y, thepoint.z, mostvoted, n, timer.elapsed()).c_str());

        v3d_imaging_paras* roi = new v3d_imaging_paras;
        roi->ops_type = 2;
        roi->xs = thepoint.x - PMain::getInstance()->Hdim_sbox->value()/2;
        roi->xe = thepoint.x + PMain::getInstance()->Hdim_sbox->value()/2;
        roi->ys = thepoint.y - PMain::getInstance()->Vdim_sbox->value()/2;
        roi->ye = thepoint.y + PMain::getInstance()->Vdim_sbox->value()/2;
        roi->zs = thepoint.z - PMain::getInstance()->Ddim_sbox->value()/2;
        roi->ze = thepoint.z + PMain::getInstance()->Ddim_sbox->value()/2;
        if(CViewer::getCurrent())
            CViewer::getCurrent()->invokedFromVaa3D(roi);
    }
    else
        QMessageBox::critical(PMain::getInstance(),QObject::tr("Error"), QString("Unsupported zoom-in method \"").append(method).append("\""),QObject::tr("Ok"));
}

//fast version (without displaying progress bar) of updateImageData method
void myV3dR_GLWidget::updateImageDataFast()
{
    /**/itm::debug(itm::LEV_MAX, strprintf("title = %s", data_title.toStdString().c_str()).c_str(), __itm__current__function__);

    renderer->setupData(_idep);
    static_cast<Renderer_gl1*>(renderer)->loadVol();
    update();
}

#endif // USE_EXPERIMENTAL_FEATURES
