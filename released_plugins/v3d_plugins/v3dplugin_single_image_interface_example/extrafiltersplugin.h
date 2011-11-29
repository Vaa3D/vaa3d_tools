
#ifndef EXTRAFILTERSPLUGIN_H
#define EXTRAFILTERSPLUGIN_H


#include <v3d_interface.h>

class ExtraFilters : public QObject, public V3DSingleImageInterface2_1
{
    Q_OBJECT
    Q_INTERFACES(V3DSingleImageInterface2_1);

public:
    float getPluginVersion() const {return 1.1f;}
    QStringList menulist() const;
    void processImage(const QString &arg, Image4DSimple *image, QWidget *parent);
};

#endif
