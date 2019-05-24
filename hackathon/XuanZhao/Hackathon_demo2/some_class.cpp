
#include "some_class.h"



void PARA_DEMO2::getFiles()
{
    eswcfiles=QFileDialog::getOpenFileNames(parent,QString(QObject::tr("Choose the manual file:")));
}

void PARA_DEMO2::savePath()
{
    savepath=QFileDialog::getExistingDirectory(parent,QString(QObject::tr("Choose the dir of saving")));
}

void PARA_DEMO2::getDirectory()
{
    brainpath=QFileDialog::getExistingDirectory(parent,QString(QObject::tr("Choose the brain")));
}

