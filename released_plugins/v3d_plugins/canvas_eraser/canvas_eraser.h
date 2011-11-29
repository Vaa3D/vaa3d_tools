
//by Hanchuan Peng
//2009-08-14

#ifndef __CanvasEraserPLUGIN_H__
#define __CanvasEraserPLUGIN_H__


#include <v3d_interface.h>

class CanvasEraserPlugin : public QObject, public V3DSingleImageInterface2_1
{
    Q_OBJECT
    Q_INTERFACES(V3DSingleImageInterface2_1)

private:
    // static const float pluginVersion = 1.1; // not in msvc
    
public:
    QStringList menulist() const;
    void processImage(const QString &arg, Image4DSimple *image, QWidget *parent);
    float getPluginVersion() const {return 1.1f;}
};

#endif
