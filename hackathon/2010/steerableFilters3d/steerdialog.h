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

#ifndef STEERDIALOG_H
#define STEERDIALOG_H

#include <QDialog>
#include <vector>

namespace Ui {
    class SteerDialog;
}

#define CONVOLVE_DIRAC 2

class SteerDialog : public QDialog {
    Q_OBJECT
public:
    SteerDialog(QWidget *parent = 0);
    ~SteerDialog();

    void getCoeffs(std::vector<double>& dcoeffs);

    int getOrder();

    double getSigmaX();
    double getSigmaY();
    double getSigmaZ();

    double getPhi();
    double getAlpha();
    double getTheta();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::SteerDialog *ui;

    bool convolveDiracCliked;

public slots:
    void setOrder_changed(int v);
    void convolveDirac_clicked();

 public Q_SLOTS:
    int exec();
};

#endif // STEERDIALOG_H
