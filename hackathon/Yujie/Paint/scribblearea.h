#ifndef SCRIBBLEAREA_H
#define SCRIBBLEAREA_H

#include <QColor>
#include <QImage>
#include <QPoint>
#include <QWidget>
#include <v3d_interface.h>

 class ScribbleArea : public QWidget
 {
     Q_OBJECT

 public:
     ScribbleArea( QWidget *parent = 0);

     bool openImage(QImage &loadimage);
     bool saveImage(const QString &fileName, const char *fileFormat);
     void setPenColor(const QColor &newColor);
     void setPenWidth(int newWidth);

     bool isModified() const { return modified; }
     QColor penColor() const { return myPenColor; }
     int penWidth() const { return myPenWidth; }
     QImage image;


 public slots:
     void print();

 protected:
     void mousePressEvent(QMouseEvent *event);
     void mouseMoveEvent(QMouseEvent *event);
     void mouseReleaseEvent(QMouseEvent *event);
     void paintEvent(QPaintEvent *event);
     //void resizeEvent(QResizeEvent *event);

 private:
     void drawLineTo(const QPoint &endPoint);
     void resizeImage(QImage *image, const QSize &newSize);

     bool modified;
     bool scribbling;
     int myPenWidth;
     QColor myPenColor;

     QPoint lastPoint;
     QPlainTextEdit *edit;
 };
#endif // SCRIBBLEAREA_H

