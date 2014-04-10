#include "dialogs.h"

setRootDialog::setRootDialog(QWidget *parent)
    : QDialog(parent)
{
   QBoxLayout *layout = new QVBoxLayout;
   QBoxLayout *layout1 = new QHBoxLayout;
   QBoxLayout *layout2 = new QHBoxLayout;
   QBoxLayout *layout3 = new QHBoxLayout;
   QBoxLayout *layout4 = new QHBoxLayout;
   label_X = new QLabel("X:");
   label_Y = new QLabel("Y:");
   label_Z = new QLabel("Z:");
   Edit_X = new QLineEdit;
   Edit_Y = new QLineEdit;
   Edit_Z = new QLineEdit;
   Edit_X->setValidator(new QDoubleValidator(0.0,
             10000.0, 3, Edit_X));
   Edit_Y->setValidator(new QDoubleValidator(0.0,
             10000.0, 3, Edit_Y));
   Edit_Z->setValidator(new QDoubleValidator(0.0,
             10000.0, 3, Edit_Z));

   okButton = new QPushButton("OK", this);
   cancelButton = new QPushButton("Cancel", this);
   connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
   connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
   
   layout1->addWidget(label_X);
   layout1->addWidget(Edit_X);
   layout2->addWidget(label_Y);
   layout2->addWidget(Edit_Y);
   layout3->addWidget(label_Z);
   layout3->addWidget(Edit_Z);

   
   layout4->addWidget(okButton);
   layout4->addWidget(cancelButton);

   layout->addLayout(layout1);
   layout->addLayout(layout2);
   layout->addLayout(layout3);
   layout->addLayout(layout4);

   X = 0;
   Y = 0;
   Z = 0;

   setLayout(layout);
}

void setRootDialog::setX(float in)
{
    X = in;
	QString X_Num;
	X_Num.setNum(X);
	Edit_X->setText(X_Num);
}
void setRootDialog::setY(float in)
{
    Y = in;
	QString Y_Num;
	Y_Num.setNum(Y);
	Edit_Y->setText(Y_Num);
}
void setRootDialog::setZ(float in)
{
    Z = in;
	QString Z_Num;
	Z_Num.setNum(Z);
	Edit_Z->setText(Z_Num);
}
float setRootDialog::getX()
{
	X = Edit_X->text().toFloat();
    return X;
}
float setRootDialog::getY()
{
	Y = Edit_Y->text().toFloat();
    return Y;
}
float setRootDialog::getZ()
{
	Z = Edit_Z->text().toFloat();
    return Z;
}
void setRootDialog::accept()
{
//    emit x_changed(X);
//	  emit y_changed(Y);
	emit root_set();
	this->close();
}