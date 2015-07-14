#ifndef __OPENSWCDIALOG_H_
#define __OPENSWCDIALOG_H_
#include <QtGui>
#include "v3d_interface.h"
class OpenSWCDialog: public QDialog
{
	Q_OBJECT

public:
	V3DPluginCallback2 * callback;
    QListWidget * listWidget, * listWidget1;
    QStringList _3DViewerTreeList_name;
    QList<int> _3DViewerTreeList_id;
    QList<V3dR_MainWindow *> _3DViewerTreeList_window;
    QList<V3dR_MainWindow *> _3DViewerList;
    QList<NeuronTree>  ntList;
	QString file_name;
    Image4DSimple * p_img4d;

	OpenSWCDialog(QWidget * parent, V3DPluginCallback2 * callback);
private:
    void getAllWindowsAndNeurons(V3DPluginCallback2 * callback);
    void getImage();

public slots:
    bool chooseNeuron();
    bool chooseWindow();
	bool setTree(const QString & file);
};


#endif
