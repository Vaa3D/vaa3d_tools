/////////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or       //
// modify it under the terms of the GNU General Public License         //
// version 2 as published by the Free Software Foundation.             //
//                                                                     //
// This program is distributed in the hope that it will be useful, but //
// WITHOUT ANY WARRANTY; without even the implied warranty of          //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU   //
// General Public License for more details.                            //
//                                                                     //
// Written and (C) by German Gonzale and Aurelien Lucchi               //
// Contact <german.gonzale@epfl.ch> or <aurelien.lucchi@gmail.com>     //
// for comments & bug reports                                          //
/////////////////////////////////////////////////////////////////////////

#include "steerdialog.h"
#include "ui_steerdialog.h"
#include "SteerableFilter3D.h"
#include <stdio.h>

using namespace std;

SteerDialog::SteerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SteerDialog)
{
    ui->setupUi(this);

    connect(ui->sbOrder,SIGNAL(valueChanged(int)),this,SLOT(setOrder_changed(int)));
    connect(ui->pbConvolveDirac, SIGNAL(clicked()), this,SLOT(convolveDirac_clicked()));

    convolveDiracCliked = false;

    setOrder_changed(1);
}

SteerDialog::~SteerDialog()
{
    delete ui;
}

void SteerDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

double SteerDialog::getSigmaX()
{
   return ui->sbSigmaX->value();
}

double SteerDialog::getSigmaY()
{
   return ui->sbSigmaX->value();
}

double SteerDialog::getSigmaZ()
{
   return ui->sbSigmaX->value();
}

double SteerDialog::getAlpha()
{
   return ui->sbAlpha->value();
}

double SteerDialog::getPhi()
{
   return ui->sbPhi->value();
}

double SteerDialog::getTheta()
{
   return ui->sbTheta->value();
}

void SteerDialog::getCoeffs(vector<double>& dcoeffs)
{
   string deriv_name;
   int M = getOrder();
   int m,n,p,idx,ord;
   for(ord = 1; ord <=M; ord++){
      for(idx = 0; idx < SteerableFilter3D::numDerivsOrder(ord); idx++){
         SteerableFilter3D::idx2deriv(idx, ord, m, n, p);
         deriv_name = getDerivativeName(m,n,p);

         stringstream sSbName;
         sSbName << "sb_" << deriv_name;
         QString sbName;
         sbName = sSbName.str().c_str();

         QDoubleSpinBox* sb = qFindChild<QDoubleSpinBox*>(this, sbName);
         if(sb == 0)
         {
            printf("warning : Spinbox %s not found\n", sSbName.str().c_str());
            dcoeffs.push_back(0);
         }
         else
         {
            dcoeffs.push_back(sb->value());
         }
      }
   }
}

void SteerDialog::setOrder_changed(int M)
{
   //Now loads the derivatives
   string deriv_name;
   QLayoutItem *child;
   while ((child = ui->gbCoeffs->layout()->takeAt(0)) != 0) {
      ui->gbCoeffs->layout()->removeItem(child);
      delete child->widget();
      delete child;
   }

   int row = 0;
   int m,n,p,idx,ord;
   for(ord = 1; ord <=M; ord++){
     for(idx = 0; idx < SteerableFilter3D::numDerivsOrder(ord); idx++){
        SteerableFilter3D::idx2deriv(idx, ord, m, n, p);
        deriv_name = getDerivativeName(m,n,p);

        stringstream sSbName;
        sSbName << "sb_"  << deriv_name;
        stringstream sLabelName;
        sLabelName << deriv_name << "label";

        QLabel* label = new QLabel(ui->gbCoeffs);
        label->setObjectName(QString::fromUtf8(sLabelName.str().c_str()));
        label->setText(deriv_name.c_str());
        ((QGridLayout*)ui->gbCoeffs->layout())->addWidget(label, row, 0, 1, 1);

        QDoubleSpinBox* sb = new QDoubleSpinBox(ui->gbCoeffs);
        sb->setObjectName(QString::fromUtf8(sSbName.str().c_str()));
        ((QGridLayout*)ui->gbCoeffs->layout())->addWidget(sb, row, 1, 1, 1);

        row++;
        label->show();
        sb->show();
    }
  }

   ui->gbCoeffs->show();
   ui->gbCoeffs->update();
   this->show();
   this->update();
}

int SteerDialog::getOrder()
{
   return ui->sbOrder->value();
}

void SteerDialog::convolveDirac_clicked()
{
   convolveDiracCliked = true;
   close();
}

int SteerDialog::exec()
{
   int retValue = QDialog::exec();
   if(convolveDiracCliked)
   {
      convolveDiracCliked = false;
      retValue = CONVOLVE_DIRAC;
   }
   return retValue;
}
