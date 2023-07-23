#include "veppluginmanager.h"

#include <QFile>
#include <QTextStream>

VEPPluginManager::VEPPluginManager()
{

}

void VEPPluginManager::createProFile(QString dir, QString pluginName)
{
    QString fileName = dir + pluginName + ".pro";
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);

        out << "TEMPLATE = lib\n";
        out << "CONFIG += c++11\n";
        out << "TARGET = test\n";
        out << "INCLUDEPATH += .\n";
        out << "SOURCES += test.cpp\n";

        file.close();
    }
}

void VEPPluginManager::createPluginHeader(QString dir, std::shared_ptr<PluginRequestCallback> pluginRequestCallback)
{

}
