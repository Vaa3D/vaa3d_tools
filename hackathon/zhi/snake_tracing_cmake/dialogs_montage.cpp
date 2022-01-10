#include "dialogs_montage.h"

setTransformDialog::setTransformDialog(QWidget *parent)
    : QDialog(parent)
{
   QBoxLayout *layout = new QVBoxLayout;
   QBoxLayout *layout1 = new QHBoxLayout;
   QBoxLayout *layout2 = new QHBoxLayout;
   QBoxLayout *layout3 = new QHBoxLayout;
   QBoxLayout *layout4 = new QHBoxLayout;
   label_Root = new QLabel("Root Tile:");
   label_Width = new QLabel("Image Width:");
   label_Height = new QLabel("Image Height:");
   Edit_Root = new QLineEdit;
   Edit_Width = new QLineEdit;
   Edit_Height = new QLineEdit;
   Edit_Root->setValidator(new QDoubleValidator(0.0,
             10000.0, 3, Edit_Root));
   Edit_Width->setValidator(new QDoubleValidator(0.0,
             10000.0, 3, Edit_Width));
   Edit_Height->setValidator(new QDoubleValidator(0.0,
             10000.0, 3, Edit_Height));

   okButton = new QPushButton("Ok", this);
   cancelButton = new QPushButton("Cancel", this);
   connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
   connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
   
   layout1->addWidget(label_Root);
   layout1->addWidget(Edit_Root);
   layout2->addWidget(label_Width);
   layout2->addWidget(Edit_Width);
   layout3->addWidget(label_Height);
   layout3->addWidget(Edit_Height);

   
   layout4->addWidget(okButton);
   layout4->addWidget(cancelButton);

   layout->addLayout(layout1);
   layout->addLayout(layout2);
   layout->addLayout(layout3);
   layout->addLayout(layout4);

   Root = 0;
   Width = 0;
   Height = 0;

   setLayout(layout);
}

void setTransformDialog::setRoot(int in)
{
    Root = in;
	QString Root_Num;
	Root_Num.setNum(Root);
	Edit_Root->setText(Root_Num);
}
void setTransformDialog::setWidth(int in)
{
    Width = in;
	QString Width_Num;
	Width_Num.setNum(Width);
	Edit_Width->setText(Width_Num);
}
void setTransformDialog::setHeight(int in)
{
    Height = in;
	QString Height_Num;
	Height_Num.setNum(Height);
	Edit_Height->setText(Height_Num);
}
int setTransformDialog::getRoot()
{
	Root = Edit_Root->text().toInt();
    return Root;
}
int setTransformDialog::getWidth()
{
	Width = Edit_Width->text().toInt();
    return Width;
}
int setTransformDialog::getHeight()
{
	Height = Edit_Height->text().toInt();
    return Height;
}
void setTransformDialog::accept()
{
//    emit x_changed(X);
//	  emit y_changed(Y);
	emit root_set();
	this->close();
}