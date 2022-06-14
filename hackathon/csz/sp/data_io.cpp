#include "data_io.h"

QStringList getImgNames(const QString &path)
{
    QDir dir(path);
    QStringList nameFilters;
    nameFilters << "*.v3draw"<<"*.tiff"<<"*.v3dpbd";
    QStringList files = dir.entryList(nameFilters, QDir::Files|QDir::Readable, QDir::Name);
    return files;
}

QStringList getSwcNames(const QString &path)
{
    QDir dir(path);
    QStringList nameFilters;
    nameFilters << "*.swc"<<"*.eswc";
    QStringList files = dir.entryList(nameFilters, QDir::Files|QDir::Readable, QDir::Name);
    return files;
}

QStringList getNames(const QString &path)
{
    QDir dir(path);
    QStringList nameFilters;
    nameFilters << "*.v3draw"<<"*.tiff"<<"*.v3dpbd"<< "*.swc"<<"*.eswc";
    QStringList files = dir.entryList(nameFilters, QDir::Files|QDir::Readable, QDir::Name);
    return files;
}
