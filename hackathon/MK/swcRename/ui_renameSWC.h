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

    void setupUi(QDialog *dialog)
    {
        if (dialog->objectName().isEmpty())
            dialog->setObjectName(QString::fromUtf8("dialog"));
        dialog->resize(615, 158);
        QFont font;
        font.setFamily(QString::fromUtf8("Calibri"));
        dialog->setFont(font);
        verticalLayout = new QVBoxLayout(dialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        groupBox = new QGroupBox(dialog);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        QFont font1;
        font1.setFamily(QString::fromUtf8("Calibri"));
        font1.setPointSize(10);
        font1.setBold(true);
        font1.setWeight(75);
        groupBox->setFont(font1);
        horizontalLayout = new QHBoxLayout(groupBox);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        lineEdit = new QLineEdit(groupBox);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
        lineEdit->setMinimumSize(QSize(450, 0));
        lineEdit->setMaximumSize(QSize(10000, 16777215));
        QFont font2;
        font2.setBold(false);
        font2.setWeight(50);
        lineEdit->setFont(font2);

        horizontalLayout->addWidget(lineEdit);

        pushButton = new QPushButton(groupBox);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        QFont font3;
        font3.setPointSize(10);
        font3.setBold(false);
        font3.setWeight(50);
        pushButton->setFont(font3);

        horizontalLayout->addWidget(pushButton);


        verticalLayout->addWidget(groupBox);

        frame = new QFrame(dialog);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        horizontalLayout_2 = new QHBoxLayout(frame);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalSpacer = new QSpacerItem(259, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);

        pushButton_2 = new QPushButton(frame);
        pushButton_2->setObjectName(QString::fromUtf8("pushButton_2"));
        QFont font4;
        font4.setPointSize(10);
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


        retranslateUi(dialog);
        QObject::connect(pushButton, SIGNAL(clicked()), dialog, SLOT(browseFolderClicked()));
        QObject::connect(buttonBox, SIGNAL(accepted()), dialog, SLOT(okClicked()));
        QObject::connect(buttonBox, SIGNAL(rejected()), dialog, SLOT(reject()));
        QObject::connect(pushButton_2, SIGNAL(clicked()), dialog, SLOT(undoClicked()));

        QMetaObject::connectSlotsByName(dialog);
    } // setupUi

    void retranslateUi(QDialog *dialog)
    {
        dialog->setWindowTitle(QApplication::translate("dialog", "Rename neuron reconstruction files", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("dialog", "Select a folder to process:", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QApplication::translate("dialog", "Browse", 0, QApplication::UnicodeUTF8));
        pushButton_2->setText(QApplication::translate("dialog", "Undo", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class dialog: public Ui_dialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_RENAMESWC_H
