/********************************************************************************
** Form generated from reading UI file 'renameSWC.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_RENAMESWC_H
#define UI_RENAMESWC_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QFrame>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_dialog
{
public:
    QVBoxLayout *verticalLayout_3;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout;
    QLineEdit *lineEdit;
    QPushButton *pushButton;
    QFrame *frame;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton_2;
    QDialogButtonBox *buttonBox;
    QGroupBox *groupBox_3;
    QVBoxLayout *verticalLayout_2;
    QGroupBox *groupBox_4;
    QHBoxLayout *horizontalLayout_3;
    QLineEdit *lineEdit_2;
    QPushButton *pushButton_3;
    QFrame *frame_2;
    QHBoxLayout *horizontalLayout_4;
    QSpacerItem *horizontalSpacer_2;
    QDialogButtonBox *buttonBox_2;

    void setupUi(QDialog *dialog)
    {
        if (dialog->objectName().isEmpty())
            dialog->setObjectName(QString::fromUtf8("dialog"));
        dialog->resize(660, 398);
        QFont font;
        font.setFamily(QString::fromUtf8("Calibri"));
        dialog->setFont(font);
        verticalLayout_3 = new QVBoxLayout(dialog);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        groupBox_2 = new QGroupBox(dialog);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        QFont font1;
        font1.setPointSize(11);
        font1.setBold(true);
        font1.setWeight(75);
        groupBox_2->setFont(font1);
        verticalLayout = new QVBoxLayout(groupBox_2);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        groupBox = new QGroupBox(groupBox_2);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        QFont font2;
        font2.setFamily(QString::fromUtf8("Calibri"));
        font2.setPointSize(10);
        font2.setBold(false);
        font2.setWeight(50);
        groupBox->setFont(font2);
        horizontalLayout = new QHBoxLayout(groupBox);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        lineEdit = new QLineEdit(groupBox);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
        lineEdit->setMinimumSize(QSize(450, 0));
        lineEdit->setMaximumSize(QSize(10000, 16777215));
        QFont font3;
        font3.setBold(false);
        font3.setWeight(50);
        lineEdit->setFont(font3);

        horizontalLayout->addWidget(lineEdit);

        pushButton = new QPushButton(groupBox);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        QFont font4;
        font4.setPointSize(10);
        font4.setBold(false);
        font4.setWeight(50);
        pushButton->setFont(font4);

        horizontalLayout->addWidget(pushButton);


        verticalLayout->addWidget(groupBox);

        frame = new QFrame(groupBox_2);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        horizontalLayout_2 = new QHBoxLayout(frame);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalSpacer = new QSpacerItem(259, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);

        pushButton_2 = new QPushButton(frame);
        pushButton_2->setObjectName(QString::fromUtf8("pushButton_2"));
        pushButton_2->setFont(font4);

        horizontalLayout_2->addWidget(pushButton_2);

        buttonBox = new QDialogButtonBox(frame);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setMaximumSize(QSize(200, 16777215));
        buttonBox->setFont(font4);
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        horizontalLayout_2->addWidget(buttonBox);


        verticalLayout->addWidget(frame);


        verticalLayout_3->addWidget(groupBox_2);

        groupBox_3 = new QGroupBox(dialog);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        groupBox_3->setFont(font1);
        verticalLayout_2 = new QVBoxLayout(groupBox_3);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        groupBox_4 = new QGroupBox(groupBox_3);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        groupBox_4->setFont(font2);
        horizontalLayout_3 = new QHBoxLayout(groupBox_4);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        lineEdit_2 = new QLineEdit(groupBox_4);
        lineEdit_2->setObjectName(QString::fromUtf8("lineEdit_2"));
        lineEdit_2->setMinimumSize(QSize(450, 0));
        lineEdit_2->setMaximumSize(QSize(10000, 16777215));
        lineEdit_2->setFont(font3);

        horizontalLayout_3->addWidget(lineEdit_2);

        pushButton_3 = new QPushButton(groupBox_4);
        pushButton_3->setObjectName(QString::fromUtf8("pushButton_3"));
        pushButton_3->setFont(font4);

        horizontalLayout_3->addWidget(pushButton_3);


        verticalLayout_2->addWidget(groupBox_4);

        frame_2 = new QFrame(groupBox_3);
        frame_2->setObjectName(QString::fromUtf8("frame_2"));
        frame_2->setFrameShape(QFrame::StyledPanel);
        frame_2->setFrameShadow(QFrame::Raised);
        horizontalLayout_4 = new QHBoxLayout(frame_2);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        horizontalSpacer_2 = new QSpacerItem(259, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_2);

        buttonBox_2 = new QDialogButtonBox(frame_2);
        buttonBox_2->setObjectName(QString::fromUtf8("buttonBox_2"));
        buttonBox_2->setMaximumSize(QSize(200, 16777215));
        buttonBox_2->setFont(font4);
        buttonBox_2->setOrientation(Qt::Horizontal);
        buttonBox_2->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        horizontalLayout_4->addWidget(buttonBox_2);


        verticalLayout_2->addWidget(frame_2);

        groupBox_4->raise();
        frame->raise();
        frame_2->raise();

        verticalLayout_3->addWidget(groupBox_3);


        retranslateUi(dialog);
        QObject::connect(pushButton, SIGNAL(clicked()), dialog, SLOT(browseFolderClicked()));
        QObject::connect(buttonBox, SIGNAL(accepted()), dialog, SLOT(okClicked()));
        QObject::connect(buttonBox, SIGNAL(rejected()), dialog, SLOT(reject()));
        QObject::connect(pushButton_2, SIGNAL(clicked()), dialog, SLOT(undoClicked()));
        QObject::connect(pushButton_3, SIGNAL(clicked()), dialog, SLOT(browseFolderClicked()));
        QObject::connect(buttonBox_2, SIGNAL(accepted()), dialog, SLOT(okClicked()));
        QObject::connect(buttonBox_2, SIGNAL(rejected()), dialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(dialog);
    } // setupUi

    void retranslateUi(QDialog *dialog)
    {
        dialog->setWindowTitle(QApplication::translate("dialog", "Neuron Reconstruction File Manager", 0, QApplication::UnicodeUTF8));
        groupBox_2->setTitle(QApplication::translate("dialog", "Rename SWC files:", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("dialog", "Select a folder to process:", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QApplication::translate("dialog", "Browse", 0, QApplication::UnicodeUTF8));
        pushButton_2->setText(QApplication::translate("dialog", "Undo", 0, QApplication::UnicodeUTF8));
        groupBox_3->setTitle(QApplication::translate("dialog", "Index SEU reconstruction files:", 0, QApplication::UnicodeUTF8));
        groupBox_4->setTitle(QApplication::translate("dialog", "Select a folder to process:", 0, QApplication::UnicodeUTF8));
        pushButton_3->setText(QApplication::translate("dialog", "Browse", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class dialog: public Ui_dialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_RENAMESWC_H
