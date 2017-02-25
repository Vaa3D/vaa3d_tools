#ifndef TIP_MARKER_FUNC_H
#define TIP_MARKER_FUNC_H

#include <v3d_interface.h>
#include <qdialog.h>
#include <qstringlist.h>
#include "qlineedit.h"
#include "qpushbutton.h"

bool TipProcessor(QStringList input, int menu);

namespace Ui {
class TipMarkerFinderUI;
}

class TipMarkerFinderUI : public QDialog
{
    Q_OBJECT

public:
	V3DPluginCallback2* callback;
    TipMarkerFinderUI(QWidget* parent, V3DPluginCallback2* callback, int menu);
	QStringList inputs;
	
    ~TipMarkerFinderUI();

public slots:
	bool okClicked();
	void filePath();

private:
    Ui::TipMarkerFinderUI* ui;
};

#endif // TIP_MARKER_FUNC_H
