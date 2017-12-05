#ifndef NEURITEINSTRUCTORUI_H
#define NEURITEINSTRUCTORUI_H

#include <v3d_interface.h>
#include "Neurite_Instructor_plugin.h"
#include "ui_Neurite_Instructor.h"

namespace Ui
{
    class neuriteInstructorUI;
}

class neuriteInstructorUI : public QDialog
{
    Q_OBJECT

public:
    neuriteInstructorUI(QWidget* parent, V3DPluginCallback2* callback);
    ~neuriteInstructorUI();

    LandmarkList markerList;
    QString imageName;

public slots:
    void okClicked();

private:
    Ui::neuriteInstructorUI* ui;
    V3DPluginCallback2* mainCallBack;
    QWidget* pluginParent;


};




#endif // NEURITEINSTRUCTORUI_H
