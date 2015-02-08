#include <QtGui>
#include "scribblearea.h"

 ScribbleArea::ScribbleArea(QWidget *parent)
     : QWidget(parent)
 {
     setAttribute(Qt::WA_StaticContents);
     modified = false;
     scribbling = false;
     myPenWidth = 1;
     myPenColor = Qt::blue;

 }

 bool ScribbleArea::openImage(QImage &loadimage)
 {


     //
     image=loadimage;
     update();

     return true;


 }

bool ScribbleArea::saveImage(const QString &fileName, const char *fileFormat)
     {
         QImage visibleImage = image;
     resizeImage(&visibleImage, size());

     if (visibleImage.save(fileName, fileFormat)) {
         modified = false;
         return true;
     } else {
         return false;
     }
 }

 void ScribbleArea::setPenColor(const QColor &newColor)
 {
     myPenColor = newColor;
 }

 void ScribbleArea::setPenWidth(int newWidth)
 {
     myPenWidth = newWidth;
 }

 void ScribbleArea::clearImage()
 {
     image.fill(qRgb(255, 255, 255));
     modified = true;
     update();
 }

 void ScribbleArea::mousePressEvent(QMouseEvent *event)
 {
     if (event->button() == Qt::LeftButton) {
         lastPoint = event->pos();
         scribbling = true;
     }
 }

 void ScribbleArea::mouseMoveEvent(QMouseEvent *event)
 {
     if ((event->buttons() & Qt::LeftButton) && scribbling)
         drawLineTo(event->pos());
 }

 void ScribbleArea::mouseReleaseEvent(QMouseEvent *event)
 {
     if (event->button() == Qt::LeftButton && scribbling) {
         drawLineTo(event->pos());
         scribbling = false;
     }
 }

 void ScribbleArea::paintEvent(QPaintEvent *event)
 {
     QPainter painter(this);
     QRect dirtyRect = event->rect();
     painter.drawImage(dirtyRect, image, dirtyRect);
 }

// void ScribbleArea::resizeEvent(QResizeEvent *event)
// {
//     if (width() > image.width() || height() > image.height()) {
//         int newWidth = qMax(width() + 128, image.width());
//         int newHeight = qMax(height() + 128, image.height());
//         resizeImage(&image, QSize(newWidth, newHeight));
//         update();
//     }
//     QWidget::resizeEvent(event);
// }

 void ScribbleArea::drawLineTo(const QPoint &endPoint)
 {
     QPainter painter(&image);
     painter.setPen(QPen(myPenColor, myPenWidth, Qt::SolidLine, Qt::RoundCap,
                         Qt::RoundJoin));
     painter.drawLine(lastPoint, endPoint);
     modified = true;

     int rad = (myPenWidth / 2) + 2;
     update(QRect(lastPoint, endPoint).normalized()
                                      .adjusted(-rad, -rad, +rad, +rad));
     lastPoint = endPoint;
 }

 void ScribbleArea::resizeImage(QImage *image, const QSize &newSize)
 {
     if (image->size() == newSize)
         return;

     QImage newImage(newSize, QImage::Format_RGB32);
     newImage.fill(qRgb(255, 255, 255));
     QPainter painter(&newImage);
     painter.drawImage(QPoint(0, 0), *image);
     *image = newImage;
 }

 void ScribbleArea::print()
 {
 #ifndef QT_NO_PRINTER
     QPrinter printer(QPrinter::HighResolution);

     QPrintDialog *printDialog = new QPrintDialog(&printer, this);
     if (printDialog->exec() == QDialog::Accepted) {
         QPainter painter(&printer);
         QRect rect = painter.viewport();
         QSize size = image.size();
         size.scale(rect.size(), Qt::KeepAspectRatio);
         painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
         painter.setWindow(image.rect());
         painter.drawImage(0, 0, image);
     }
#endif // QT_NO_PRINTER
}

