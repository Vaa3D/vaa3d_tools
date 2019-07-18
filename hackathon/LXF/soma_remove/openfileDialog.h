#ifndef OPENFILEDIALOG_H
#define OPENFILEDIALOG_H

#include <QtGui>
#include "v3d_interface.h"
class OpenfileDialog: public QDialog
{
    Q_OBJECT

public:
    V3DPluginCallback2 * callback;
    QListWidget * listWidget;
    QList<NeuronTree> _3DViewerTreeList;
    NeuronTree  nt;
    QString file_name;

    OpenfileDialog(QWidget * parent, V3DPluginCallback2 * callback);
    void getAllNeuronTreeNames(V3DPluginCallback2 * callback);

public slots:
    bool run();
    bool setTree(const QString & file);
};


#endif // OPENFILEDIALOG_H
