
#ifndef ROTATEIMG90PLUGIN_H
#define ROTATEIMG90PLUGIN_H


#include <v3d_interface.h>

class RotateImg90Plugin : public QObject, public V3DSingleImageInterface2_1
{
    Q_OBJECT
    Q_INTERFACES(V3DSingleImageInterface2_1)

public:
    QStringList menulist() const;
    void processImage(const QString &arg, Image4DSimple *image, QWidget *parent);
    float getPluginVersion() const {return 1.1f;}
};

#endif
