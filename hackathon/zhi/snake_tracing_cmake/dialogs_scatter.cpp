#include <QtGui>

#include "dialogs_scatter.h"

ScatterPlotDialog::ScatterPlotDialog(QWidget *parent)
    : QDialog(parent)
{
   QBoxLayout *layout = new QVBoxLayout;
   QBoxLayout *layout1 = new QHBoxLayout;
   QBoxLayout *layout2 = new QHBoxLayout;

   label_X = new QLabel("X Axis:");
   label_Y = new QLabel("Y Axis:");
   
   ComboBox_X = new QComboBox;
   ComboBox_Y = new QComboBox;

   ComboBox_X->addItem(tr("Length"));
   ComboBox_X->addItem(tr("Average Radius"));
   ComboBox_X->addItem(tr("Volume"));
   ComboBox_X->addItem(tr("Surface"));
   ComboBox_X->addItem(tr("Average Section Area"));
   ComboBox_X->addItem(tr("#Snaxels"));
   ComboBox_X->addItem(tr("Tortuosity"));
   ComboBox_X->addItem(tr("Average Intensity"));
   ComboBox_X->addItem(tr("Average Vesselness"));

   ComboBox_Y->addItem(tr("Length"));
   ComboBox_Y->addItem(tr("Average Radius"));
   ComboBox_Y->addItem(tr("Volume"));
   ComboBox_Y->addItem(tr("Surface"));
   ComboBox_Y->addItem(tr("Average Section Area"));
   ComboBox_Y->addItem(tr("#Snaxels"));
   ComboBox_Y->addItem(tr("Tortuosity"));
   ComboBox_Y->addItem(tr("Average Intensity"));
   ComboBox_Y->addItem(tr("Average Vesselness"));

   connect(ComboBox_X, SIGNAL(activated(int)), this, SLOT(setX(int)));
   connect(ComboBox_X, SIGNAL(currentIndexChanged(int)), this, SLOT(setX(int)));
   connect(ComboBox_X, SIGNAL(activated(int)), this, SIGNAL(x_changed(int)));
   connect(ComboBox_X, SIGNAL(currentIndexChanged(int)), this, SIGNAL(x_changed(int)));

   connect(ComboBox_Y, SIGNAL(activated(int)), this, SLOT(setY(int)));
   connect(ComboBox_Y, SIGNAL(currentIndexChanged(int)), this, SLOT(setY(int)));
   connect(ComboBox_Y, SIGNAL(activated(int)), this, SIGNAL(y_changed(int)));
   connect(ComboBox_Y, SIGNAL(currentIndexChanged(int)), this, SIGNAL(y_changed(int)));

   ComboBox_X->setCurrentIndex(0);
   ComboBox_Y->setCurrentIndex(1);
   
   okButton = new QPushButton("ok", this);
   cancelButton = new QPushButton("cancel", this);
   connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
   connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
   
   layout1->addWidget(label_X);
   layout1->addWidget(ComboBox_X);
   layout1->addWidget(label_Y);
   layout1->addWidget(ComboBox_Y);

   layout2->addWidget(okButton);
   layout2->addWidget(cancelButton);

   layout->addLayout(layout1);
   layout->addLayout(layout2);
   setLayout(layout);
}

void ScatterPlotDialog::setX(int in)
{
    X = in;
}
void ScatterPlotDialog::setY(int in)
{
    Y = in;
}
int ScatterPlotDialog::getX()
{
    return X;
}
int ScatterPlotDialog::getY()
{
    return Y;
}
void ScatterPlotDialog::accept()
{
//    emit x_changed(X);
//	  emit y_changed(Y);
	this->close();
}