/**@file zstackproject.h
 * @brief Tracing project
 * @author Ting Zhao
 */

#ifndef ZTRACEPROJECT_H
#define ZTRACEPROJECT_H

#include "tz_stack_document.h"
#include <QString>
#include <QStringList>

class ZStackFrame;

class ZTraceProject
{
public:
    ZTraceProject(ZStackFrame *parent = NULL);
    ~ZTraceProject();

public:
    QString workspacePath() const { return m_workspacePath; }
    QString rootPath() const;
    QString tubeFolder() const;

public:
    void setWorkDir(const char *path);
    void setProjFilePath(const char *path);
    void addDecoration(const QString &path, const QString &tag);

public:
    int load(const QString filePath);
    void save();
    void saveAs(QString workspacePath);

private:
    ZStackFrame *m_parent;
    QString m_workspacePath;
    QString m_projFilePath;
    QStringList m_decorationPath;
    QStringList m_decorationTag;
};

#endif // ZTRACEPROJECT_H
