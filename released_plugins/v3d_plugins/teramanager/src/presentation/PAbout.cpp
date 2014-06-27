#include "PAbout.h"

using namespace teramanager;

PAbout* PAbout::uniqueInstance = 0;

PAbout::PAbout(QWidget *parent) : QDialog(parent)
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    setWindowTitle("About TeraFly");

    desc = new QLabel();
    desc->setText( QString("<html><h3>Vaa3D-TeraFly v. ").append(teramanager::version.c_str()).append("</h3>"
                         "<p>A tool designed for Teravoxel-sized datasets 3D navigation and Vaa3D-aided analysis.</p>"
                         "<small><u>Developed by:</u><ul style=\"padding-left:5px\">"
                         "<li><b>Alessandro Bria</b> (email: a.bria@unicas.it)<br>"
                                "University of Cassino</li>"
                         "<li><b>Giulio Iannello</b> (email: g.iannello@unicampus.it)<br>"
                                "University Campus Bio-Medico of Rome</li>"
                         "<li><b>Hanchuan Peng</b> (email: hanchuan.peng@gmail.com)<br>"
                                 "Allen Institute for Brain Science and Janelia @ HHMI</li></ul></small></html>"));
    desc->setWordWrap(true);

    logo = new QLabel();
    QPixmap *logo_img = new QPixmap(":/icons/terafly.png");
    logo->setPixmap(logo_img->scaled(150, 200, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

    changelog = new QTextEdit();
    changelog->setReadOnly(true);
    QFile file(":/changelog.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
      return;
    QTextStream in(&file);
    changelog->setText(in.readAll());
    QFont tinyFont = QFont("Courier New", 9);
    changelog->setFont(tinyFont);
    changelog->setWordWrapMode(QTextOption::NoWrap);
    changelog->setStyleSheet("QTextEdit {padding:5px; background-color: white}");

    closeButton = new QPushButton("Close");
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

    QGridLayout* layout = new QGridLayout(this);
    layout->setMargin(20);
    layout->setSpacing(20);
    layout->addWidget(logo, 0, 0, 1, 1);
    layout->addWidget(desc, 0, 1, 1, 1);
    layout->addWidget(changelog, 2, 0, 1, 2);
    layout->addWidget(closeButton, 3, 0, 1, 2, Qt::AlignRight);

    /*msgBox->setText( QString("<html><h1>TeraFly plugin v. ").append(CPlugin::getMajorVersion().c_str()).append("</h1>"
                    "<big>An experimental tool designed for Teravoxel-sized datasets 3D navigation into Vaa3D.</big><br><br>"
                    "<u>Developed by:</u><ul>"
                    "<li style=\"margin-left: 0\"><b>Alessandro Bria</b> (email: a.bria@unicas.it)<br>"
                           "University of Cassino</li>"
                    "<li><b>Giulio Iannello</b> (email: g.iannello@unicampus.it)<br>"
                           "University Campus Bio-Medico of Rome</li>"
                    "<li><b>Hanchuan Peng</b> (email: hanchuan.peng@gmail.com)<br>"
                            "Allen Institute for Brain Science and Janelia @ HHMI</li></ul><br>"
                    "<u>Features:</u><ul>"
                    "<li>Google Earth-like 3D navigation through multiresolution teravoxel-sized datasets</li>"
                    "<li>computer-aided annotation of markers and curves</li>"
                    "<li>low memory requirements (4 GB)</li></ul><br>"
                    "<u>Supported input formats:</u><ul>"
                    "<li>two-level directory structure with each tile containing a series of image slices (see documentation for further information)</li>"
                    "<li>supported formats for image slices are BMP, DIB, JPEG, JPG, JPE, PNG, PBM, PGM, PPM, SR, RAS, TIFF, TIF</li>"
                    "<li>no restriction on the bit depth</li>"
                    "<li>no restriction on the number of channels</li></ul></html>" ));*/
    setLayout(layout);
    setFixedWidth(550);
}
