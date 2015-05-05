#ifndef MYGRAPHICSVIEW_H
#define MYGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QPixmap>
#include <QMouseEvent>
#include <QGraphicsScene>
#include <QImage>

class MyGraphicsView : public QGraphicsView
{
    Q_OBJECT
protected:
    QGraphicsScene* mScene;
    QImage  mOrigImage;

public:
    explicit MyGraphicsView(QWidget *parent = 0);

    // override
    const QPixmap * pixmap() const {
        return &mPixmap;
    }

    void setZoomLimits( double min, double max) { mZoomMax = max; mZoomMin = min; }
    double scaleFactor() const { return mScaleFactor; }

    void setImage( const QImage &img, const QRect &updateRect = QRect() );

    // returns 'viewable rect' in image (pixmap) coordinates
    QRect getViewableRect() const;

    // pan / scale
    void scale(double factor);
    void pan( double x, double y );

    inline QPoint screenToImage( const QPoint &ev )
    {
        QPointF pf = mapToScene( ev );

        return QPoint( pf.x(), pf.y() );
    }

signals:
    
protected:
    //Holds the current centerpoint for the view, used for panning and zooming
    QPointF CurrentCenterPoint;

    QPixmap mPixmap;
    double mScaleFactor;
    double  mZoomMax, mZoomMin; // max and min zoom factor

    //From panning the view
    QPoint mLastMouseMovePt;

    //Set the current centerpoint in the
    void SetCenter(const QPointF& centerPoint);

    QPointF GetCenter() { return CurrentCenterPoint; }

    virtual void wheelEvent ( QWheelEvent * event ) {
        emit wheelEventSignal(event);
    }

    virtual void mouseMoveEvent ( QMouseEvent * ev ) {
        mLastMouseMovePt = ev->pos();
        emit mouseMoveEventSignal(ev);
    }

    virtual void mouseReleaseEvent ( QMouseEvent * ev ) {
        emit mouseReleaseEventSignal(ev);
    }

    virtual void mousePressEvent ( QMouseEvent * ev ) {
        emit mousePressEventSignal(ev);
    }

    virtual void resizeEvent(QResizeEvent* event);
    
signals:
    void wheelEventSignal( QWheelEvent *event );
    void mouseMoveEventSignal( QMouseEvent *event );
    void mouseReleaseEventSignal( QMouseEvent *event );
    void mousePressEventSignal( QMouseEvent *event );

public slots:
    // fits the image within the scrollarea size
    void zoomFit()
    {
        qDebug("zoomfit: Implement me!");
        mScaleFactor = 1.0;
    }


};

#endif // MYGRAPHICSVIEW_H
