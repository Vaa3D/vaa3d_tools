/********************************************************************************
** Form generated from reading UI file 'FL_watershedSegPara.ui'
**
** Created: Fri Jul 5 11:51:56 2013
**      by: Qt User Interface Compiler version 4.7.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FL_WATERSHEDSEGPARA_H
#define UI_FL_WATERSHEDSEGPARA_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Dialog_WatershedPara
{
public:
    QDialogButtonBox *buttonBox;
    QWidget *layoutWidget;
    QGridLayout *gridLayout;
    QLabel *label;
    QSpinBox *spinBox_Channel;
    QLabel *label_5;
    QGridLayout *gridLayout1;
    QLabel *label_2;
    QSpinBox *spinBox_MedianSz;
    QLabel *label_4;
    QSpinBox *spinBox_GaussianSz;
    QLabel *label_7;
    QDoubleSpinBox *doubleSpinBox_GaussianSigma;
    QLabel *label_3;
    QComboBox *comboBox_Threshold;
    QLabel *label_6;
    QComboBox *comboBox_Method;

    void setupUi(QDialog *Dialog_WatershedPara)
    {
        if (Dialog_WatershedPara->objectName().isEmpty())
            Dialog_WatershedPara->setObjectName(QString::fromUtf8("Dialog_WatershedPara"));
        Dialog_WatershedPara->resize(400, 300);
        buttonBox = new QDialogButtonBox(Dialog_WatershedPara);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setGeometry(QRect(30, 240, 341, 32));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::NoButton|QDialogButtonBox::Ok);
        layoutWidget = new QWidget(Dialog_WatershedPara);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(61, 20, 308, 205));
        gridLayout = new QGridLayout(layoutWidget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(layoutWidget);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 0, 0, 1, 2);

        spinBox_Channel = new QSpinBox(layoutWidget);
        spinBox_Channel->setObjectName(QString::fromUtf8("spinBox_Channel"));

        gridLayout->addWidget(spinBox_Channel, 0, 2, 1, 1);

        label_5 = new QLabel(layoutWidget);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        gridLayout->addWidget(label_5, 1, 0, 1, 2);

        gridLayout1 = new QGridLayout();
        gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
        label_2 = new QLabel(layoutWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout1->addWidget(label_2, 0, 0, 1, 1);

        spinBox_MedianSz = new QSpinBox(layoutWidget);
        spinBox_MedianSz->setObjectName(QString::fromUtf8("spinBox_MedianSz"));

        gridLayout1->addWidget(spinBox_MedianSz, 0, 1, 1, 1);

        label_4 = new QLabel(layoutWidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout1->addWidget(label_4, 1, 0, 1, 1);

        spinBox_GaussianSz = new QSpinBox(layoutWidget);
        spinBox_GaussianSz->setObjectName(QString::fromUtf8("spinBox_GaussianSz"));

        gridLayout1->addWidget(spinBox_GaussianSz, 1, 1, 1, 1);

        label_7 = new QLabel(layoutWidget);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        gridLayout1->addWidget(label_7, 2, 0, 1, 1);

        doubleSpinBox_GaussianSigma = new QDoubleSpinBox(layoutWidget);
        doubleSpinBox_GaussianSigma->setObjectName(QString::fromUtf8("doubleSpinBox_GaussianSigma"));

        gridLayout1->addWidget(doubleSpinBox_GaussianSigma, 2, 1, 1, 1);


        gridLayout->addLayout(gridLayout1, 1, 2, 1, 1);

        label_3 = new QLabel(layoutWidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 2, 0, 1, 2);

        comboBox_Threshold = new QComboBox(layoutWidget);
        comboBox_Threshold->setObjectName(QString::fromUtf8("comboBox_Threshold"));

        gridLayout->addWidget(comboBox_Threshold, 2, 2, 1, 1);

        label_6 = new QLabel(layoutWidget);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        gridLayout->addWidget(label_6, 3, 0, 1, 1);

        comboBox_Method = new QComboBox(layoutWidget);
        comboBox_Method->setObjectName(QString::fromUtf8("comboBox_Method"));

        gridLayout->addWidget(comboBox_Method, 3, 1, 1, 2);


        retranslateUi(Dialog_WatershedPara);
        QObject::connect(buttonBox, SIGNAL(accepted()), Dialog_WatershedPara, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), Dialog_WatershedPara, SLOT(reject()));

        QMetaObject::connectSlotsByName(Dialog_WatershedPara);
    } // setupUi

    void retranslateUi(QDialog *Dialog_WatershedPara)
    {
        Dialog_WatershedPara->setWindowTitle(QApplication::translate("Dialog_WatershedPara", "Define watershed parameters", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("Dialog_WatershedPara", "Channel", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("Dialog_WatershedPara", "Smoothing", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("Dialog_WatershedPara", "Median filter size", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("Dialog_WatershedPara", "Gaussian filter size", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("Dialog_WatershedPara", "Gaussian filter sigma", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("Dialog_WatershedPara", "Thresholding", 0, QApplication::UnicodeUTF8));
        comboBox_Threshold->clear();
        comboBox_Threshold->insertItems(0, QStringList()
         << QApplication::translate("Dialog_WatershedPara", "Adaptive", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("Dialog_WatershedPara", "Global (3D)", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("Dialog_WatershedPara", "Global (2D)", 0, QApplication::UnicodeUTF8)
        );
        label_6->setText(QApplication::translate("Dialog_WatershedPara", "Method", 0, QApplication::UnicodeUTF8));
        comboBox_Method->clear();
        comboBox_Method->insertItems(0, QStringList()
         << QApplication::translate("Dialog_WatershedPara", "Intensity based", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("Dialog_WatershedPara", "Shape based", 0, QApplication::UnicodeUTF8)
        );
    } // retranslateUi

};

namespace Ui {
    class Dialog_WatershedPara: public Ui_Dialog_WatershedPara {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FL_WATERSHEDSEGPARA_H
