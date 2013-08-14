/********************************************************************************
** Form generated from reading UI file 'dialog_pointcloudatlas_detect_coexpression_output.ui'
**
** Created: Mon Dec 19 15:04:03 2011
**      by: Qt User Interface Compiler version 4.7.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOG_POINTCLOUDATLAS_DETECT_COEXPRESSION_OUTPUT_H
#define UI_DIALOG_POINTCLOUDATLAS_DETECT_COEXPRESSION_OUTPUT_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_Dialog_pointcloudatlas_detect_coexpression_output
{
public:
    QLabel *label_output_file;
    QLineEdit *lineEdit_output_file;
    QPushButton *pushButton_output_file;
    QPushButton *pushButton_ok;
    QPushButton *pushButton_cancel;

    void setupUi(QDialog *Dialog_pointcloudatlas_detect_coexpression_output)
    {
        if (Dialog_pointcloudatlas_detect_coexpression_output->objectName().isEmpty())
            Dialog_pointcloudatlas_detect_coexpression_output->setObjectName(QString::fromUtf8("Dialog_pointcloudatlas_detect_coexpression_output"));
        Dialog_pointcloudatlas_detect_coexpression_output->resize(772, 190);
        Dialog_pointcloudatlas_detect_coexpression_output->setMinimumSize(QSize(772, 190));
        Dialog_pointcloudatlas_detect_coexpression_output->setMaximumSize(QSize(1200, 400));
        label_output_file = new QLabel(Dialog_pointcloudatlas_detect_coexpression_output);
        label_output_file->setObjectName(QString::fromUtf8("label_output_file"));
        label_output_file->setGeometry(QRect(100, 44, 221, 16));
        lineEdit_output_file = new QLineEdit(Dialog_pointcloudatlas_detect_coexpression_output);
        lineEdit_output_file->setObjectName(QString::fromUtf8("lineEdit_output_file"));
        lineEdit_output_file->setGeometry(QRect(310, 44, 291, 21));
        pushButton_output_file = new QPushButton(Dialog_pointcloudatlas_detect_coexpression_output);
        pushButton_output_file->setObjectName(QString::fromUtf8("pushButton_output_file"));
        pushButton_output_file->setGeometry(QRect(600, 40, 77, 32));
        pushButton_ok = new QPushButton(Dialog_pointcloudatlas_detect_coexpression_output);
        pushButton_ok->setObjectName(QString::fromUtf8("pushButton_ok"));
        pushButton_ok->setGeometry(QRect(400, 100, 202, 32));
        pushButton_ok->setDefault(true);
        pushButton_cancel = new QPushButton(Dialog_pointcloudatlas_detect_coexpression_output);
        pushButton_cancel->setObjectName(QString::fromUtf8("pushButton_cancel"));
        pushButton_cancel->setGeometry(QRect(190, 100, 201, 32));

        retranslateUi(Dialog_pointcloudatlas_detect_coexpression_output);

        QMetaObject::connectSlotsByName(Dialog_pointcloudatlas_detect_coexpression_output);
    } // setupUi

    void retranslateUi(QDialog *Dialog_pointcloudatlas_detect_coexpression_output)
    {
        Dialog_pointcloudatlas_detect_coexpression_output->setWindowTitle(QApplication::translate("Dialog_pointcloudatlas_detect_coexpression_output", "Pointcloud atlas detect coexpression output", 0, QApplication::UnicodeUTF8));
        label_output_file->setText(QApplication::translate("Dialog_pointcloudatlas_detect_coexpression_output", "Save predicted coexpression to:", 0, QApplication::UnicodeUTF8));
        pushButton_output_file->setText(QApplication::translate("Dialog_pointcloudatlas_detect_coexpression_output", "File...", 0, QApplication::UnicodeUTF8));
        pushButton_ok->setText(QApplication::translate("Dialog_pointcloudatlas_detect_coexpression_output", "OK", 0, QApplication::UnicodeUTF8));
        pushButton_cancel->setText(QApplication::translate("Dialog_pointcloudatlas_detect_coexpression_output", "Cancel", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Dialog_pointcloudatlas_detect_coexpression_output: public Ui_Dialog_pointcloudatlas_detect_coexpression_output {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOG_POINTCLOUDATLAS_DETECT_COEXPRESSION_OUTPUT_H
