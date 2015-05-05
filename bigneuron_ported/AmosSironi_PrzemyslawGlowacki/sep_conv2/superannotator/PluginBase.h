#ifndef PLUGINBASE_H
#define PLUGINBASE_H

/**
 * This is a base class for plugins
 */

#include <Matrix3D.h>
#include <QObject>

#include <QMenu>
#include <QList>
#include "CommonTypes.h"

#include "PluginServices.h"

class AnnotatorWnd;

class PluginBase : public QObject
{
    Q_OBJECT

public:
    explicit PluginBase( QObject *parent = 0 ) : QObject(parent) {}
    virtual bool    initializePlugin( const PluginServices & ) = 0;

    // must return the plugin's name
    virtual QString pluginName() = 0;

    // called on mouse release
    virtual void  mouseReleaseEvent( QMouseEvent *evt, unsigned int imgX, unsigned int imgY, unsigned int imgZ )
    {
        // don't do anything default
        (void) evt;
        (void) imgX;
        (void) imgY;
        (void) imgZ;
    }

    // called on mouse move
    virtual void  mouseMoveEvent( QMouseEvent *evt, unsigned int imgX, unsigned int imgY, unsigned int imgZ )
    {
        // don't do anything default
        (void) evt;
        (void) imgX;
        (void) imgY;
        (void) imgZ;
    }
};


#endif // PLUGINBASE_H
