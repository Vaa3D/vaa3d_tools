#include "paint_dialog.h"
#include "scribblearea.h"

Paint_Dialog::Paint_Dialog(QWidget *parent) :
    QDialog(parent)

{   paintarea = new ScribbleArea;
    create();
}

void Paint_Dialog::create()
{
    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->addWidget(paintarea,1,0,1,1);

    QToolBar *tool = new QToolBar;
    tool->setGeometry(0,0,200,20);
    QVBoxLayout *layout = new QVBoxLayout;
    QToolButton *button_load = new QToolButton;
    button_load->setGeometry(0,0,10,20);
    button_load->setText("Load");
    QToolButton *button_save = new QToolButton;
    button_save->setText("Save");
    QToolButton *button_color = new QToolButton;
    button_color->setText("Color");
    QToolButton *button_pen = new QToolButton;
    button_pen->setText("Pen Width");
    QToolButton *button_print = new QToolButton;
    button_print->setText("Print");
    QToolButton *button_clear = new QToolButton;
    button_clear->setText("Clear screen");


    tool->addWidget(button_load);
    tool->addSeparator();
    tool->addWidget(button_save);
    tool->addSeparator();
    tool->addWidget(button_color);
    tool->addSeparator();
    tool->addWidget(button_pen);
    tool->addSeparator();
    tool->addWidget(button_clear);
    tool->addSeparator();
    tool->addWidget(button_print);
    tool->addSeparator();

    layout->addWidget(tool);
    gridLayout->addLayout(layout,0,0,1,1);
    this->setLayout(gridLayout);

    this->setMinimumHeight(800);
    this->setMinimumWidth(800);
    connect(button_load, SIGNAL(clicked()), this, SLOT(load()));
    connect(button_save, SIGNAL(clicked()), this, SLOT(save()));
    connect(button_color, SIGNAL(clicked()), this, SLOT(penColor()));
    connect(button_pen, SIGNAL(clicked()), this, SLOT(penWidth()));
    connect(button_clear, SIGNAL(clicked()), paintarea, SLOT(clearImage()));
    connect(button_print, SIGNAL(clicked()), paintarea, SLOT(print()));
}


bool Paint_Dialog::maybeSave()
{
    if (paintarea->isModified()) {
       QMessageBox::StandardButton ret;
       ret = QMessageBox::warning(this, tr("Paint"),
                          tr("The image has been modified.\n"
                             "Do you want to save your changes?"),
                          QMessageBox::Save | QMessageBox::Discard
                          | QMessageBox::Cancel);
        if (ret == QMessageBox::Save) {
            return saveFile("jpg");
        } else if (ret == QMessageBox::Cancel) {
            return false;
        }
        qDebug()<<"MaybeSave";
    }
    return true;
}


void Paint_Dialog::load()
{
// fileName = QFileDialog::getOpenFileName(0, QObject::tr("Choose the input image "),
//                                          QDir::currentPath(),
//       QObject::tr("Images (*.raw *.tif *.lsm *.v3dpbd *.v3draw);;All(*)"));

    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this,
                                    tr("Open File"), QDir::currentPath());
         if (!fileName.isEmpty())
             paintarea->openImage(fileName);
     }

}

bool Paint_Dialog::saveFile(const QByteArray &fileFormat)
{
    QString initialPath = QDir::currentPath() + "/untitled." + fileFormat;

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),
                               initialPath,
                               tr("%1 Files (*.%2);;All Files (*)")
                               .arg(QString(fileFormat.toUpper()))
                               .arg(QString(fileFormat)));
    if (fileName.isEmpty()) {
        return false;
    } else {
        return paintarea->saveImage(fileName, fileFormat);
    }
    qDebug()<<"SaveFile";
}


void Paint_Dialog::save()
{

    saveFile("jpg");


}

void Paint_Dialog::penColor()
{
    QColor newColor = QColorDialog::getColor(paintarea->penColor());
    if (newColor.isValid())
        paintarea->setPenColor(newColor);
}

void Paint_Dialog::penWidth()
{
    bool ok;
    int newWidth = QInputDialog::getInteger(this, tr("Paint"),
                                            tr("Select pen width:"),
                                            paintarea->penWidth(),
                                            1, 50, 1, &ok);
    if (ok)
        paintarea->setPenWidth(newWidth);
}
