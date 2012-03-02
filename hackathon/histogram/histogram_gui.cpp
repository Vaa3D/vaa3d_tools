
#include "histogram_gui.h"


HistogramDialog::HistogramDialog(QWidget * parent) : QDialog(parent)
{
     histLabel = new QLabel(tr("Histogram :"));
     histFrame = new QFrame(parent);

     disp_width = 256;
     disp_height =50;
     histscale =256;

     histFrame->setFixedWidth(disp_width);
     histFrame->setFixedHeight(disp_height);

     channelLabel = new QLabel(tr("Channels :"));
     channelFrame = new QFrame(parent);
     channelFrame->setFixedWidth(50);
     channelFrame->setFixedHeight(60);


     ok = new QPushButton(tr("OK"));
     cancel = new QPushButton(tr("Cancel"));

     gridLayout = new QGridLayout();
     gridLayout->addWidget(histLabel, 0, 0, 1, 1);
     gridLayout->addWidget(channelLabel, 0, 11, 1, 1);
     gridLayout->addWidget(histFrame, 1, 1, 10, 9);
     gridLayout->addWidget(channelFrame, 1, 11, 10, 1);

     gridLayout->addWidget(cancel, 12, 0, 1, 5, Qt::AlignRight);
     gridLayout->addWidget(ok, 12, 5, 1, 5, Qt::AlignRight);

     connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
     connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

     connect(ok, SIGNAL(clicked()), this, SLOT(update()));

     this->setLayout(gridLayout);
     this->setWindowTitle("Image Histogram");

}

// void HistogramDialog::paintEvent(QPaintEvent *event)
// {
// 	QPainter painter(this);
// 	painter.save();
// 	painter.setRenderHint(QPainter::Antialiasing);
// 	painter.setRenderHint(QPainter::SmoothPixmapTransform);

// }


void HistogramDialog::paint(QPainter *painter)
{
     painter->save();
     painter->setRenderHint(QPainter::Antialiasing);
     painter->setRenderHint(QPainter::SmoothPixmapTransform);
     // draw

     // setPen to draw solid lines
     painter->setPen(QPen(QColor(255, 255, 255), 3, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin));
     painter->setBrush(Qt::NoBrush);

     // find max of histscale for scaling height
     V3DLONG maxval=0;
     for(V3DLONG i=0; i<hist.size(); i++)
     {
          if(maxval < hist.at(i)) maxval = hist.at(i);
     }

     // draw lines
     for(V3DLONG i=0; i<histscale; i++)
     {
          painter->drawLine(i, disp_height, i, hist[i]/maxval);
     }

     painter->restore();
}
