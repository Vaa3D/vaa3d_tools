/********************************************************************************
** Form generated from reading UI file 'dialog_pointcloudatlas_detect_coexpression_input.ui'
**
** Created: Thu Dec 29 15:26:39 2011
**      by: Qt User Interface Compiler version 4.7.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOG_POINTCLOUDATLAS_DETECT_COEXPRESSION_INPUT_H
#define UI_DIALOG_POINTCLOUDATLAS_DETECT_COEXPRESSION_INPUT_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QFrame>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_Dialog_pointcloudatlas_detect_coexpression_input
{
public:
    QFrame *line_3;
    QFrame *line;
    QListWidget *listWidget_info_existing_atlases;
    QLabel *label_info_existing_atlases;
    QLabel *label_atlas_file;
    QPushButton *pushButton_atlas_file;
    QLineEdit *lineEdit_atlas_file;
    QPushButton *pushButton_cancel;
    QPushButton *pushButton_remove;
    QPushButton *pushButton_finish;

    void setupUi(QDialog *Dialog_pointcloudatlas_detect_coexpression_input)
    {
        if (Dialog_pointcloudatlas_detect_coexpression_input->objectName().isEmpty())
            Dialog_pointcloudatlas_detect_coexpression_input->setObjectName(QString::fromUtf8("Dialog_pointcloudatlas_detect_coexpression_input"));
        Dialog_pointcloudatlas_detect_coexpression_input->resize(772, 400);
        Dialog_pointcloudatlas_detect_coexpression_input->setMinimumSize(QSize(772, 190));
        Dialog_pointcloudatlas_detect_coexpression_input->setMaximumSize(QSize(1200, 400));
        line_3 = new QFrame(Dialog_pointcloudatlas_detect_coexpression_input);
        line_3->setObjectName(QString::fromUtf8("line_3"));
        line_3->setGeometry(QRect(11, 56, 876, 16));
        line_3->setFrameShape(QFrame::HLine);
        line_3->setFrameShadow(QFrame::Sunken);
        line = new QFrame(Dialog_pointcloudatlas_detect_coexpression_input);
        line->setObjectName(QString::fromUtf8("line"));
        line->setGeometry(QRect(-49, 373, 876, 16));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        listWidget_info_existing_atlases = new QListWidget(Dialog_pointcloudatlas_detect_coexpression_input);
        listWidget_info_existing_atlases->setObjectName(QString::fromUtf8("listWidget_info_existing_atlases"));
        listWidget_info_existing_atlases->setGeometry(QRect(11, 94, 751, 192));
        label_info_existing_atlases = new QLabel(Dialog_pointcloudatlas_detect_coexpression_input);
        label_info_existing_atlases->setObjectName(QString::fromUtf8("label_info_existing_atlases"));
        label_info_existing_atlases->setGeometry(QRect(11, 70, 130, 16));
        label_atlas_file = new QLabel(Dialog_pointcloudatlas_detect_coexpression_input);
        label_atlas_file->setObjectName(QString::fromUtf8("label_atlas_file"));
        label_atlas_file->setGeometry(QRect(12, 15, 158, 16));
        pushButton_atlas_file = new QPushButton(Dialog_pointcloudatlas_detect_coexpression_input);
        pushButton_atlas_file->setObjectName(QString::fromUtf8("pushButton_atlas_file"));
        pushButton_atlas_file->setGeometry(QRect(670, 10, 77, 32));
        lineEdit_atlas_file = new QLineEdit(Dialog_pointcloudatlas_detect_coexpression_input);
        lineEdit_atlas_file->setObjectName(QString::fromUtf8("lineEdit_atlas_file"));
        lineEdit_atlas_file->setGeometry(QRect(180, 15, 481, 22));
        pushButton_cancel = new QPushButton(Dialog_pointcloudatlas_detect_coexpression_input);
        pushButton_cancel->setObjectName(QString::fromUtf8("pushButton_cancel"));
        pushButton_cancel->setGeometry(QRect(280, 310, 181, 32));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(pushButton_cancel->sizePolicy().hasHeightForWidth());
        pushButton_cancel->setSizePolicy(sizePolicy);
        pushButton_remove = new QPushButton(Dialog_pointcloudatlas_detect_coexpression_input);
        pushButton_remove->setObjectName(QString::fromUtf8("pushButton_remove"));
        pushButton_remove->setGeometry(QRect(102, 310, 181, 32));
        sizePolicy.setHeightForWidth(pushButton_remove->sizePolicy().hasHeightForWidth());
        pushButton_remove->setSizePolicy(sizePolicy);
        pushButton_finish = new QPushButton(Dialog_pointcloudatlas_detect_coexpression_input);
        pushButton_finish->setObjectName(QString::fromUtf8("pushButton_finish"));
        pushButton_finish->setGeometry(QRect(460, 310, 181, 32));
        sizePolicy.setHeightForWidth(pushButton_finish->sizePolicy().hasHeightForWidth());
        pushButton_finish->setSizePolicy(sizePolicy);
        pushButton_finish->setDefault(true);

        retranslateUi(Dialog_pointcloudatlas_detect_coexpression_input);

        QMetaObject::connectSlotsByName(Dialog_pointcloudatlas_detect_coexpression_input);
    } // setupUi

    void retranslateUi(QDialog *Dialog_pointcloudatlas_detect_coexpression_input)
    {
        Dialog_pointcloudatlas_detect_coexpression_input->setWindowTitle(QApplication::translate("Dialog_pointcloudatlas_detect_coexpression_input", "Pointcloud atlas detect coexpression editor", 0, QApplication::UnicodeUTF8));
        label_info_existing_atlases->setText(QApplication::translate("Dialog_pointcloudatlas_detect_coexpression_input", "Existing atlases are: ", 0, QApplication::UnicodeUTF8));
        label_atlas_file->setText(QApplication::translate("Dialog_pointcloudatlas_detect_coexpression_input", "Select an atlas file (.apo):", 0, QApplication::UnicodeUTF8));
        pushButton_atlas_file->setText(QApplication::translate("Dialog_pointcloudatlas_detect_coexpression_input", "File...", 0, QApplication::UnicodeUTF8));
        pushButton_cancel->setText(QApplication::translate("Dialog_pointcloudatlas_detect_coexpression_input", "Cancel", 0, QApplication::UnicodeUTF8));
        pushButton_remove->setText(QApplication::translate("Dialog_pointcloudatlas_detect_coexpression_input", "Remove", 0, QApplication::UnicodeUTF8));
        pushButton_finish->setText(QApplication::translate("Dialog_pointcloudatlas_detect_coexpression_input", "Finish", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Dialog_pointcloudatlas_detect_coexpression_input: public Ui_Dialog_pointcloudatlas_detect_coexpression_input {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOG_POINTCLOUDATLAS_DETECT_COEXPRESSION_INPUT_H
