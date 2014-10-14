/********************************************************************************
** Modified from ui_surfaceobj_geometry_dialog.h
** Hanbo Chen 2011.10.8
********************************************************************************/

#ifndef UI_NEURON_STITCH_DIAL_H
#define UI_NEURON_STITCH_DIAL_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDial>
#include <QtGui/QDialog>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include <QtGui/QComboBox>

QT_BEGIN_NAMESPACE

class Ui_NeuronGeometryDialog
{
public:
    QLabel *label_11;
    QWidget *widget;
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout;
    QLabel *label_7;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *label;
    QDoubleSpinBox *doubleSpinBox_shift_x;
    QDoubleSpinBox *doubleSpinBox_shift_y;
    QDoubleSpinBox *doubleSpinBox_shift_z;
    QLabel *label_5;
    QDoubleSpinBox *doubleSpinBox_scale_x;
    QDoubleSpinBox *doubleSpinBox_scale_y;
    QDoubleSpinBox *doubleSpinBox_scale_z;
    QLabel *label_8;
    QCheckBox *checkBox_flip_x;
    QCheckBox *checkBox_flip_y;
    QCheckBox *checkBox_flip_z;
    QLabel *label_6;
    QDial *dial_x;
    QDial *dial_y;
    QDial *dial_z;
    QLabel *label_10;
    QDoubleSpinBox *doubleSpinBox_gscale_x;
    QDoubleSpinBox *doubleSpinBox_gscale_y;
    QDoubleSpinBox *doubleSpinBox_gscale_z;
    QHBoxLayout *hboxLayout;
    QLabel *label_9;
    QDoubleSpinBox *doubleSpinBox_scale_r;
    QFrame *line_2;
    QHBoxLayout *hboxLayout1;
    QPushButton *pushButton_reset;
    QSpacerItem *spacerItem;
    QPushButton *pushButton_cancel;
    QPushButton *pushButton_ok;
    QPushButton *pushButton_save;

    QHBoxLayout *hboxLayout_t1;
    QCheckBox *checkBox_hide;
    QDoubleSpinBox *doubleSpinBox_hide;
    QHBoxLayout *hboxLayout_t1a;
    QCheckBox *checkBox_highlight_matchpoint;
    QDoubleSpinBox *doubleSpinBox_highlight_matchpoint;
    QHBoxLayout *hboxLayout_t1b;
    QCheckBox *checkBox_highlight_boundpoint;
    QDoubleSpinBox *doubleSpinBox_highlight_boundpoint;
    QFrame *line_3;
    QHBoxLayout *hboxLayout_t2;
    QHBoxLayout *hboxLayout_t3;
    QLabel *label_sd;
    QComboBox *comboBox_sd;
    QLabel *label_ant;
    QComboBox *comboBox_ant;
    QPushButton *pushButton_quickmove;
    QPushButton *pushButton_updatebox;
    QPushButton *pushButton_neurontype;

    void setupUi(QDialog *NeuronGeometryDialog)
    {
        if (NeuronGeometryDialog->objectName().isEmpty())
            NeuronGeometryDialog->setObjectName(QString::fromUtf8("NeuronGeometryDialog"));
        NeuronGeometryDialog->resize(514,620);
        NeuronGeometryDialog->setMinimumSize(QSize(514,480));
        NeuronGeometryDialog->setMaximumSize(QSize(514,720));
        label_11 = new QLabel(NeuronGeometryDialog);
        label_11->setObjectName(QString::fromUtf8("label_11"));
        //label_11->setGeometry(QRect(24, 448, 469, 53));
        label_11->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        widget = new QWidget(NeuronGeometryDialog);
        widget->setObjectName(QString::fromUtf8("widget"));
        //widget->setGeometry(QRect(19, 14, 478, 425));
        widget->setMinimumWidth(514);
        widget->setMaximumWidth(514);
        verticalLayout = new QVBoxLayout(widget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        verticalLayout->setMargin(19);

        hboxLayout_t1 = new QHBoxLayout();
        hboxLayout_t1->setObjectName(QString::fromUtf8("hboxLayout_t1"));
        hboxLayout_t1->setContentsMargins(0, 0, 0, 0);
        hboxLayout_t1->setMargin(0);

        checkBox_hide = new QCheckBox(widget);
        checkBox_hide->setObjectName(QString::fromUtf8("checkBox_hide"));
        hboxLayout_t1->addWidget(checkBox_hide);

        doubleSpinBox_hide = new QDoubleSpinBox(widget);
        doubleSpinBox_hide->setObjectName(QString::fromUtf8("doubleSpinBox_hide"));
        doubleSpinBox_hide->setDecimals(4);
        hboxLayout_t1->addWidget(doubleSpinBox_hide);

        verticalLayout->addLayout(hboxLayout_t1);

        hboxLayout_t1a = new QHBoxLayout();
        hboxLayout_t1a->setObjectName(QString::fromUtf8("hboxLayout_t1a"));
        hboxLayout_t1a->setContentsMargins(0, 0, 0, 0);
        hboxLayout_t1a->setMargin(0);

        checkBox_highlight_matchpoint = new QCheckBox(widget);
        checkBox_highlight_matchpoint->setObjectName(QString::fromUtf8("checkBox_highlight_matchpoint"));
        hboxLayout_t1a->addWidget(checkBox_highlight_matchpoint);

        doubleSpinBox_highlight_matchpoint = new QDoubleSpinBox(widget);
        doubleSpinBox_highlight_matchpoint->setObjectName(QString::fromUtf8("doubleSpinBox_hide_matchpoint"));
        doubleSpinBox_highlight_matchpoint->setDecimals(4);
        hboxLayout_t1a->addWidget(doubleSpinBox_highlight_matchpoint);

        verticalLayout->addLayout(hboxLayout_t1a);

        hboxLayout_t1b = new QHBoxLayout();
        hboxLayout_t1b->setObjectName(QString::fromUtf8("hboxLayout_t1b"));
        hboxLayout_t1b->setContentsMargins(0, 0, 0, 0);
        hboxLayout_t1b->setMargin(0);

        checkBox_highlight_boundpoint = new QCheckBox(widget);
        checkBox_highlight_boundpoint->setObjectName(QString::fromUtf8("checkBox_highlight_boundpoint"));
        hboxLayout_t1b->addWidget(checkBox_highlight_boundpoint);

        doubleSpinBox_highlight_boundpoint = new QDoubleSpinBox(widget);
        doubleSpinBox_highlight_boundpoint->setObjectName(QString::fromUtf8("doubleSpinBox_hide_boundpoint"));
        doubleSpinBox_highlight_boundpoint->setDecimals(4);
        hboxLayout_t1b->addWidget(doubleSpinBox_highlight_boundpoint);

        verticalLayout->addLayout(hboxLayout_t1b);

        line_3 = new QFrame(widget);
        line_3->setObjectName(QString::fromUtf8("line_3"));
        line_3->setFrameShape(QFrame::HLine);
        line_3->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line_3);

        hboxLayout_t2 = new QHBoxLayout();
        hboxLayout_t2->setObjectName(QString::fromUtf8("hboxLayout_t2"));
        hboxLayout_t2->setContentsMargins(0, 0, 0, 0);
        hboxLayout_t2->setMargin(0);

        label_ant = new QLabel(widget);
        label_ant->setObjectName(QString::fromUtf8("label_ant"));
        label_ant->setAlignment(Qt::AlignRight);
        hboxLayout_t2->addWidget(label_ant);

        comboBox_ant = new QComboBox(widget);
        comboBox_ant->setObjectName(QString::fromUtf8("comboBox_ant"));
        hboxLayout_t2->addWidget(comboBox_ant);

        pushButton_neurontype = new QPushButton(widget);
        pushButton_neurontype->setObjectName(QString::fromUtf8("pushButton_neurontype"));
        pushButton_neurontype->setAutoDefault(false);
        hboxLayout_t2->addWidget(pushButton_neurontype);

        verticalLayout->addLayout(hboxLayout_t2);

        hboxLayout_t3 = new QHBoxLayout();
        hboxLayout_t3->setObjectName(QString::fromUtf8("hboxLayout_t3"));
        hboxLayout_t3->setContentsMargins(0, 0, 0, 0);
        hboxLayout_t3->setMargin(0);

        label_sd = new QLabel(widget);
        label_sd->setObjectName(QString::fromUtf8("label_sd"));
        label_sd->setAlignment(Qt::AlignRight);
        hboxLayout_t3->addWidget(label_sd);

        comboBox_sd = new QComboBox(widget);
        comboBox_sd->setObjectName(QString::fromUtf8("comboBox_sd"));
        hboxLayout_t3->addWidget(comboBox_sd);

        pushButton_quickmove = new QPushButton(widget);
        pushButton_quickmove->setObjectName(QString::fromUtf8("pushButton_quickmove"));
        pushButton_quickmove->setAutoDefault(false);
        hboxLayout_t3->addWidget(pushButton_quickmove);

        pushButton_updatebox = new QPushButton(widget);
        pushButton_updatebox->setObjectName(QString::fromUtf8("pushButton_updatebox"));
        pushButton_updatebox->setAutoDefault(false);
        hboxLayout_t3->addWidget(pushButton_updatebox);

        verticalLayout->addLayout(hboxLayout_t3);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label_7 = new QLabel(widget);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_7, 0, 0, 1, 1);

        label_2 = new QLabel(widget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_2, 0, 1, 1, 1);

        label_3 = new QLabel(widget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_3, 0, 2, 1, 1);

        label_4 = new QLabel(widget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_4, 0, 3, 1, 1);

        label = new QLabel(widget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label, 1, 0, 1, 1);

        doubleSpinBox_shift_x = new QDoubleSpinBox(widget);
        doubleSpinBox_shift_x->setObjectName(QString::fromUtf8("doubleSpinBox_shift_x"));
        doubleSpinBox_shift_x->setDecimals(4);

        gridLayout->addWidget(doubleSpinBox_shift_x, 1, 1, 1, 1);

        doubleSpinBox_shift_y = new QDoubleSpinBox(widget);
        doubleSpinBox_shift_y->setObjectName(QString::fromUtf8("doubleSpinBox_shift_y"));
        doubleSpinBox_shift_y->setDecimals(4);

        gridLayout->addWidget(doubleSpinBox_shift_y, 1, 2, 1, 1);

        doubleSpinBox_shift_z = new QDoubleSpinBox(widget);
        doubleSpinBox_shift_z->setObjectName(QString::fromUtf8("doubleSpinBox_shift_z"));
        doubleSpinBox_shift_z->setDecimals(4);

        gridLayout->addWidget(doubleSpinBox_shift_z, 1, 3, 1, 1);

        label_5 = new QLabel(widget);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_5, 2, 0, 1, 1);

        doubleSpinBox_scale_x = new QDoubleSpinBox(widget);
        doubleSpinBox_scale_x->setObjectName(QString::fromUtf8("doubleSpinBox_scale_x"));
        doubleSpinBox_scale_x->setDecimals(4);

        gridLayout->addWidget(doubleSpinBox_scale_x, 2, 1, 1, 1);

        doubleSpinBox_scale_y = new QDoubleSpinBox(widget);
        doubleSpinBox_scale_y->setObjectName(QString::fromUtf8("doubleSpinBox_scale_y"));
        doubleSpinBox_scale_y->setDecimals(4);

        gridLayout->addWidget(doubleSpinBox_scale_y, 2, 2, 1, 1);

        doubleSpinBox_scale_z = new QDoubleSpinBox(widget);
        doubleSpinBox_scale_z->setObjectName(QString::fromUtf8("doubleSpinBox_scale_z"));
        doubleSpinBox_scale_z->setDecimals(4);

        gridLayout->addWidget(doubleSpinBox_scale_z, 2, 3, 1, 1);

        label_8 = new QLabel(widget);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_8, 3, 0, 1, 1);

        checkBox_flip_x = new QCheckBox(widget);
        checkBox_flip_x->setObjectName(QString::fromUtf8("checkBox_flip_x"));

        gridLayout->addWidget(checkBox_flip_x, 3, 1, 1, 1);

        checkBox_flip_y = new QCheckBox(widget);
        checkBox_flip_y->setObjectName(QString::fromUtf8("checkBox_flip_y"));

        gridLayout->addWidget(checkBox_flip_y, 3, 2, 1, 1);

        checkBox_flip_z = new QCheckBox(widget);
        checkBox_flip_z->setObjectName(QString::fromUtf8("checkBox_flip_z"));

        gridLayout->addWidget(checkBox_flip_z, 3, 3, 1, 1);

        label_6 = new QLabel(widget);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_6, 4, 0, 1, 1);

        dial_x = new QDial(widget);
        dial_x->setObjectName(QString::fromUtf8("dial_x"));
        dial_x->setCursor(QCursor(Qt::PointingHandCursor));
        dial_x->setAutoFillBackground(false);

        gridLayout->addWidget(dial_x, 4, 1, 1, 1);

        dial_y = new QDial(widget);
        dial_y->setObjectName(QString::fromUtf8("dial_y"));
        dial_y->setCursor(QCursor(Qt::PointingHandCursor));

        gridLayout->addWidget(dial_y, 4, 2, 1, 1);

        dial_z = new QDial(widget);
        dial_z->setObjectName(QString::fromUtf8("dial_z"));
        dial_z->setCursor(QCursor(Qt::PointingHandCursor));

        gridLayout->addWidget(dial_z, 4, 3, 1, 1);

        label_10 = new QLabel(widget);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        label_10->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_10, 5, 0, 1, 1);

        doubleSpinBox_gscale_x = new QDoubleSpinBox(widget);
        doubleSpinBox_gscale_x->setObjectName(QString::fromUtf8("doubleSpinBox_gscale_x"));
        doubleSpinBox_gscale_x->setDecimals(4);

        gridLayout->addWidget(doubleSpinBox_gscale_x, 5, 1, 1, 1);

        doubleSpinBox_gscale_y = new QDoubleSpinBox(widget);
        doubleSpinBox_gscale_y->setObjectName(QString::fromUtf8("doubleSpinBox_gscale_y"));
        doubleSpinBox_gscale_y->setDecimals(4);

        gridLayout->addWidget(doubleSpinBox_gscale_y, 5, 2, 1, 1);

        doubleSpinBox_gscale_z = new QDoubleSpinBox(widget);
        doubleSpinBox_gscale_z->setObjectName(QString::fromUtf8("doubleSpinBox_gscale_z"));
        doubleSpinBox_gscale_z->setDecimals(4);

        gridLayout->addWidget(doubleSpinBox_gscale_z, 5, 3, 1, 1);


        verticalLayout->addLayout(gridLayout);

        hboxLayout = new QHBoxLayout();
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        label_9 = new QLabel(widget);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setAlignment(Qt::AlignCenter);

        hboxLayout->addWidget(label_9);

        doubleSpinBox_scale_r = new QDoubleSpinBox(widget);
        doubleSpinBox_scale_r->setObjectName(QString::fromUtf8("doubleSpinBox_scale_r"));
        doubleSpinBox_scale_r->setDecimals(4);

        hboxLayout->addWidget(doubleSpinBox_scale_r);


        verticalLayout->addLayout(hboxLayout);

        line_2 = new QFrame(widget);
        line_2->setObjectName(QString::fromUtf8("line_2"));
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line_2);

        hboxLayout1 = new QHBoxLayout();
        hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));

        pushButton_reset = new QPushButton(widget);
        pushButton_reset->setObjectName(QString::fromUtf8("pushButton_reset"));
        pushButton_reset->setAutoDefault(false);

        hboxLayout1->addWidget(pushButton_reset);

        pushButton_save = new QPushButton(widget);
        pushButton_save->setObjectName(QString::fromUtf8("pushButton_save"));
        pushButton_save->setAutoDefault(false);

        hboxLayout1->addWidget(pushButton_save);

        spacerItem = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout1->addItem(spacerItem);

        pushButton_cancel = new QPushButton(widget);
        pushButton_cancel->setObjectName(QString::fromUtf8("pushButton_cancel"));
        pushButton_cancel->setAutoDefault(false);

        hboxLayout1->addWidget(pushButton_cancel);

        pushButton_ok = new QPushButton(widget);
        pushButton_ok->setObjectName(QString::fromUtf8("pushButton_ok"));
        pushButton_ok->setAutoDefault(false);

        hboxLayout1->addWidget(pushButton_ok);

        verticalLayout->addLayout(hboxLayout1);

        verticalLayout->addWidget(label_11);


        retranslateUi(NeuronGeometryDialog);

        QMetaObject::connectSlotsByName(NeuronGeometryDialog);
    } // setupUi

    void retranslateUi(QDialog *NeuronGeometryDialog)
    {
        NeuronGeometryDialog->setWindowTitle(QApplication::translate("NeuronGeometryDialog", "Geometry Adjustment For Neuron Stitching", 0, QApplication::UnicodeUTF8));
        label_11->setText(QApplication::translate("NeuronGeometryDialog", "Tip: \"scaling\" means scaling an object w/o changing the 3D location \n"
"of the center; \"G-scaling\" means just multiplying a scaling factor to the \n"
"current absolute coordinates.", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("NeuronGeometryDialog", "axis", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("NeuronGeometryDialog", "X", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("NeuronGeometryDialog", "Y", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("NeuronGeometryDialog", "Z", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("NeuronGeometryDialog", "shift", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("NeuronGeometryDialog", "scaling * 1000", 0, QApplication::UnicodeUTF8));
        label_8->setText(QApplication::translate("NeuronGeometryDialog", "mirror", 0, QApplication::UnicodeUTF8));
        checkBox_flip_x->setText(QApplication::translate("NeuronGeometryDialog", "flip X", 0, QApplication::UnicodeUTF8));
        checkBox_flip_y->setText(QApplication::translate("NeuronGeometryDialog", "flip Y", 0, QApplication::UnicodeUTF8));
        checkBox_flip_z->setText(QApplication::translate("NeuronGeometryDialog", "flip Z", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("NeuronGeometryDialog", "rotation \n"
"(around the axis)", 0, QApplication::UnicodeUTF8));
        label_10->setText(QApplication::translate("NeuronGeometryDialog", "G-scaling * 1000", 0, QApplication::UnicodeUTF8));
        label_9->setText(QApplication::translate("NeuronGeometryDialog", "radius scaling * 1000", 0, QApplication::UnicodeUTF8));
        pushButton_reset->setText(QApplication::translate("NeuronGeometryDialog", "Reset", 0, QApplication::UnicodeUTF8));
        pushButton_cancel->setText(QApplication::translate("NeuronGeometryDialog", "Cancel", 0, QApplication::UnicodeUTF8));
        pushButton_ok->setText(QApplication::translate("NeuronGeometryDialog", "OK", 0, QApplication::UnicodeUTF8));
        pushButton_save->setText(QApplication::translate("NeuronGeometryDialog", "Save Affine Mat", 0, QApplication::UnicodeUTF8));

        checkBox_hide->setText(QApplication::translate("NeuronGeometryDialog", "Hide branches, span:", 0, QApplication::UnicodeUTF8));
        checkBox_highlight_matchpoint->setText(QApplication::translate("NeuronGeometryDialog", "Highlight Matching Points, span:", 0, QApplication::UnicodeUTF8));
        checkBox_highlight_boundpoint->setText(QApplication::translate("NeuronGeometryDialog", "Highlight Boundary Points, span:", 0, QApplication::UnicodeUTF8));
        label_ant->setText(QApplication::translate("NeuronGeometryDialog", "Nueron to Adjust", 0, QApplication::UnicodeUTF8));
        label_sd->setText(QApplication::translate("NeuronGeometryDialog", "Direction of Stacks", 0, QApplication::UnicodeUTF8));
        pushButton_quickmove->setText(QApplication::translate("NeuronGeometryDialog", "Stick Neuron in Stack Direction", 0, QApplication::UnicodeUTF8));
        pushButton_updatebox->setText(QApplication::translate("NeuronGeometryDialog", "Update Bounding Box", 0, QApplication::UnicodeUTF8));
        pushButton_neurontype->setText(QApplication::translate("NeuronGeometryDialog", "Change Color", 0, QApplication::UnicodeUTF8));

    } // retranslateUi

};

namespace Ui {
    class NeuronGeometryDialog: public Ui_NeuronGeometryDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_NEURON_STITCH_DIAL_H
