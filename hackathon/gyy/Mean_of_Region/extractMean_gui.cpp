#include "extractMean_gui.h"

ExtractMeanCenterDialog :: ExtractMeanCenterDialog(QWidget *parent): QDialog(parent)
{
    rangeBox = new QSpinBox();
    rangeBox -> setRange(2, 200);
    rangeBox -> setValue(20);
    rangeBox -> setSingleStep(2);
    connect(rangeBox, SIGNAL(valueChanged(int)), rangeBox, SLOT(setValue(int)));

    openButton = new QPushButton(tr("Input a folder"));
    openEdit = new QLineEdit(tr("Select a tile folder !"));
    openHbox = new QHBoxLayout();
    openHbox -> addWidget(openButton);
    openHbox -> addWidget(openEdit);
    connect(openButton, SIGNAL(clicked(bool)), this, SLOT(selectFolder()));

    saveButton = new QPushButton(tr("Save files"));
    saveEdit = new QLineEdit(tr("Select a folder to save files !"));
    saveHbox = new QHBoxLayout();
    saveHbox -> addWidget(saveButton);
    saveHbox -> addWidget(saveEdit);
    connect(saveButton, SIGNAL(clicked(bool)), this, SLOT(saveFiles()));

    ok = new QPushButton(tr("ok"));
    ok -> setDefault(true);
    cancel = new QPushButton(tr("cancel"));
    cancel -> setDefault(false);
    hbox_ok = new QHBoxLayout();
    hbox_ok -> addWidget(ok);
    hbox_ok -> addWidget(cancel);
    connect(ok, SIGNAL(clicked(bool)), this, SLOT(accept()));
    connect(cancel, SIGNAL(clicked(bool)), this, SLOT(reject()));

    gridLayout = new QGridLayout();
    gridLayout -> addWidget(new QLabel("The range number is even !"), 0, 0);
    gridLayout -> addWidget(new QLabel("Input the number of center region !"), 1, 0);
    gridLayout -> addWidget(rangeBox, 1, 1);

    gridLayout -> addLayout(openHbox, 4, 0);
    gridLayout -> addLayout(saveHbox, 5, 0);
    gridLayout -> addLayout(hbox_ok, 6, 0);

    this -> setLayout(gridLayout);
    this -> setWindowTitle("Extract the mean image");

}

void ExtractMeanCenterDialog :: selectFolder()
{
    QPushButton *button = (QPushButton *) sender();
    if(button == openButton)
    {
        openFolderFiles = QFileDialog::getExistingDirectory(NULL, tr("Select a folder"),"D:\\");
        openEdit ->setText(openFolderFiles);
        if(openFolderFiles.isEmpty())
        {
            openEdit->setText("Select a folder !");
            v3d_msg("Folder is empty ! Input again !");
        }
    }
}

void ExtractMeanCenterDialog :: saveFiles()
{
    QPushButton *button1 = (QPushButton *) sender();
    if(button1 == saveButton)
    {
        saveFolderFiles = QFileDialog :: getExistingDirectory(0, tr("Select a folder to save files"), openFolderFiles);
        saveEdit->setText(saveFolderFiles);
        if(saveFolderFiles.isEmpty())
        {
            saveEdit->setText("Select a folder to save files !");
            v3d_msg("Please input a folder to save again !");
        }
    }

}
