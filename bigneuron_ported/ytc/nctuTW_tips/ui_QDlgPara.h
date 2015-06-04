/********************************************************************************
** Form generated from reading UI file 'QDlgPara.ui'
**
** Created: Thu Jun 4 15:15:15 2015
**      by: Qt User Interface Compiler version 4.7.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QDLGPARA_H
#define UI_QDLGPARA_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>

QT_BEGIN_NAMESPACE

class Ui_QDlgPara
{
public:
    QGridLayout *gridLayout;
    QFrame *frame;
    QDoubleSpinBox *dudThreshold;
    QLabel *label;
    QLabel *label_2;
    QSpinBox *nudSomaX;
    QSpinBox *nudSomaY;
    QSpinBox *nudSomaZ;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *QDlgPara)
    {
        if (QDlgPara->objectName().isEmpty())
            QDlgPara->setObjectName(QString::fromUtf8("QDlgPara"));
        QDlgPara->resize(380, 149);
        gridLayout = new QGridLayout(QDlgPara);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        frame = new QFrame(QDlgPara);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setFrameShape(QFrame::NoFrame);
        frame->setFrameShadow(QFrame::Raised);
        dudThreshold = new QDoubleSpinBox(frame);
        dudThreshold->setObjectName(QString::fromUtf8("dudThreshold"));
        dudThreshold->setGeometry(QRect(110, 10, 91, 24));
        dudThreshold->setDecimals(2);
        dudThreshold->setMaximum(1);
        dudThreshold->setSingleStep(0.1);
        label = new QLabel(frame);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(10, 10, 101, 21));
        label_2 = new QLabel(frame);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(10, 40, 101, 21));
        nudSomaX = new QSpinBox(frame);
        nudSomaX->setObjectName(QString::fromUtf8("nudSomaX"));
        nudSomaX->setGeometry(QRect(110, 40, 71, 24));
        nudSomaX->setMinimum(-1);
        nudSomaX->setMaximum(99999);
        nudSomaY = new QSpinBox(frame);
        nudSomaY->setObjectName(QString::fromUtf8("nudSomaY"));
        nudSomaY->setGeometry(QRect(190, 40, 71, 24));
        nudSomaY->setMinimum(-1);
        nudSomaY->setMaximum(99999);
        nudSomaZ = new QSpinBox(frame);
        nudSomaZ->setObjectName(QString::fromUtf8("nudSomaZ"));
        nudSomaZ->setGeometry(QRect(270, 40, 71, 24));
        nudSomaZ->setMinimum(-1);
        nudSomaZ->setMaximum(99999);

        gridLayout->addWidget(frame, 0, 0, 1, 1);

        buttonBox = new QDialogButtonBox(QDlgPara);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout->addWidget(buttonBox, 1, 0, 1, 1);


        retranslateUi(QDlgPara);
        QObject::connect(buttonBox, SIGNAL(accepted()), QDlgPara, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), QDlgPara, SLOT(reject()));

        QMetaObject::connectSlotsByName(QDlgPara);
    } // setupUi

    void retranslateUi(QDialog *QDlgPara)
    {
        QDlgPara->setWindowTitle(QApplication::translate("QDlgPara", "Dialog", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("QDlgPara", "Threshold:", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("QDlgPara", "Soma(x, y, z):", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class QDlgPara: public Ui_QDlgPara {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QDLGPARA_H
