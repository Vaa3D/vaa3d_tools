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
    friend class NeuriteInstructor;

public:
    neuriteInstructorUI(QWidget* parent, V3DPluginCallback2* callback);
    ~neuriteInstructorUI();

    LandmarkList markerList;
    QString imageName;

    QString deployDisplay;
    QString modelDisplay;
    QString meanDisplay;

    string deployName;
    string modelName;
    string meanName;

    unsigned char* ImgPtr;
    V3DLONG imgX;
    V3DLONG imgY;
    V3DLONG imgZ;
    V3DLONG channel;
    Image4DSimple* p4DImage;

public slots:
    void okClicked();
    void filePath();

    void uiCall();

private:
    Ui::neuriteInstructorUI* ui;
    V3DPluginCallback2* mainCallBack;
    QWidget* pluginParent;


};




#endif // NEURITEINSTRUCTORUI_H
