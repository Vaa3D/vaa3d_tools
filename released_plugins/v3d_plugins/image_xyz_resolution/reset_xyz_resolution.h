
//by Hanchuan Peng
//2009-06-26

#ifndef __example_reset_xyz_resolution_PLUGIN_H__
#define __example_reset_xyz_resolution_PLUGIN_H__


#include <v3d_interface.h>

class example_reset_xyz_resolutionPlugin : public QObject, public V3DSingleImageInterface2_1
{
    Q_OBJECT
    Q_INTERFACES(V3DSingleImageInterface2_1)

public:
    QStringList menulist() const;
    void processImage(const QString &arg, Image4DSimple *image, QWidget *parent);
    float getPluginVersion() const {return 1.1f;}
};

#endif
