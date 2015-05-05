#ifndef QLABELIMAGE_H
#define QLABELIMAGE_H

#include <QLabel>
#include <QMouseEvent>
#include <QPoint>
#include <QScrollArea>
#include <QScrollBar>
#include <QRectF>
#include <QPointF>
#include <QSizeF>
#include <QPixmap>
#include <QTime>

class QLabelImage : public QLabel
{
    Q_OBJECT
private:
    double mScaleFactor;
    QScrollArea *mScrollArea;    //pointer to parent?
    double  mZoomMax, mZoomMin; // max and min zoom factor

    QPixmap mOriginalPixmap;  // we control scaling ourselves to be able to choose the interpolation method.
                              // so this is the original pixmap, no scaling

private:
     inline void adjustScrollBar(QScrollBar *scrollBar, double factor)
     {
         scrollBar->setValue(int(factor * scrollBar->value()
                                 + ((factor - 1) * scrollBar->pageStep()/2)));
     }

public slots:
     // overriden
     void setPixmap( const QPixmap& p ) {
         mOriginalPixmap = p;   // will hold an internal copy
         rescalePixmap();
     }

private:
     void rescalePixmap()   // re-scale pixmap according to current size
     {
         //QTime t; t.restart();
         QPixmap p = mOriginalPixmap.scaled( size(), Qt::IgnoreAspectRatio, Qt::FastTransformation );

         //qDebug("%d", t.elapsed());

         if ( !mOriginalPixmap.isNull() )
         {
            QSize oldSize(0,0);

            if (QLabel::pixmap() != 0)
                oldSize = QLabel::pixmap()->size();


            //qDebug("val: %ld", (unsigned long int) QLabel::pixmap());
            if (oldSize != p.size())
                QLabel::setPixmap( p );
            else {
                *(QPixmap *)(QLabel::pixmap()) = p;
                QLabel::update();
            }
         }

         //qDebug("%d", t.elapsed());
     }

public:
    explicit QLabelImage(QWidget *parent = 0);

     double scaleFactor() const { return mScaleFactor; }

    // override
    const QPixmap * pixmap() const {
        return &mOriginalPixmap;
    }

    // overriden
    void resize( const QSize &newSize )
    {
        QLabel::resize( newSize );
        rescalePixmap();
    }

    void setZoomLimits( double min, double max) { mZoomMax = max; mZoomMin = min; }
    void setScrollArea( QScrollArea *sa ) { mScrollArea = sa; }

    inline void setImage( const QImage &img )
    {
        const QPixmap &p = QPixmap::fromImage(img);
        this->setPixmap( p );
    }

    // returns 'viewable rect' in image (pixmap) coordinates
    inline QRect getViewableRect() const
    {
        QSizeF viewportSize = QSizeF(mScrollArea->maximumViewportSize());

        //qDebug("Viewport size: %f %f", viewportSize.width(), viewportSize.height());

        QRectF myCoords = QRectF(this->rect());
        myCoords.setX( this->x() );
        myCoords.setY( this->y() );

        QPointF corner(0,0);
        QSizeF  size(0,0);

        //qDebug("Scale: %f", (float)mScaleFactor);
        //qDebug("x: %f",myCoords.x());
        //qDebug("y: %f",myCoords.y());

        if (myCoords.x() < 0) {
            corner.setX( -myCoords.x() / mScaleFactor );
            size.setWidth( viewportSize.width() / mScaleFactor );
        }

        size.setWidth( viewportSize.width() / mScaleFactor );

        if (myCoords.y() < 0) {
            corner.setY( -myCoords.y() / mScaleFactor );
        }

        size.setHeight( viewportSize.height() / mScaleFactor );

        if (size.width() > mOriginalPixmap.width())
            size.setWidth( mOriginalPixmap.width() );

        if (size.height() > mOriginalPixmap.height())
            size.setHeight( mOriginalPixmap.height() );

        return QRect( corner.toPoint(), size.toSize() );
    }

    void scaleImage(double factor, bool factorIsRelative = true)
    {
     qDebug("Scale: %f", (float)factor);

     double tempScale = factor;
     if (factorIsRelative)
         tempScale *= mScaleFactor;

     if ( tempScale > mZoomMax )
         tempScale = mZoomMax;
     if (tempScale < mZoomMin)
         tempScale = mZoomMin;

     mScaleFactor = tempScale;

     resize(mScaleFactor * mOriginalPixmap.size());

     adjustScrollBar(mScrollArea->horizontalScrollBar(), factor);
     adjustScrollBar(mScrollArea->verticalScrollBar(), factor);

     //zoomInAct->setEnabled(scaleFactor < 3.0);
     //zoomOutAct->setEnabled(scaleFactor > 0.333);
    }

    inline QPoint screenToImage( const QPoint &ev ) {
        int x = (int)(ev.x() / mScaleFactor - 1 + 0.5);
        int y = (int)(ev.y() / mScaleFactor - 1 + 0.5);

        return QPoint(x,y);
    }

protected:
    void wheelEvent ( QWheelEvent * event ) {
        emit wheelEventSignal(event);
    }

    void mouseMoveEvent ( QMouseEvent * ev ) {
        emit mouseMoveEventSignal(ev);
    }

    void mouseReleaseEvent ( QMouseEvent * ev ) {
        emit mouseReleaseEventSignal(ev);
    }

    void mousePressEvent ( QMouseEvent * ev ) {
        emit mousePressEventSignal(ev);
    }

signals:
    void wheelEventSignal( QWheelEvent *event );
    void mouseMoveEventSignal( QMouseEvent *event );
    void mouseReleaseEventSignal( QMouseEvent *event );
    void mousePressEventSignal( QMouseEvent *event );

public slots:
    // fits the image within the scrollarea size
    void zoomFit()
    {
        double ratioSA = mScrollArea->maximumViewportSize().width() * 1.0 / mScrollArea->maximumViewportSize().height();
        double ratioImg = mOriginalPixmap.width() * 1.0 / mOriginalPixmap.height();

        mScaleFactor = 1.0; //reset
        if (ratioSA < ratioImg)
            scaleImage( mScrollArea->maximumViewportSize().width() * 1.0 / mOriginalPixmap.width() );
        else
            scaleImage( mScrollArea->maximumViewportSize().height() * 1.0 / mOriginalPixmap.height() );
    }
};

#endif // QLABELIMAGE_H


