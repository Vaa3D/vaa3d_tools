#ifndef FIJIHELPER_H
#define FIJIHELPER_H

#include <QString>
#include <QProcess>
#include <QTemporaryFile>
#include <QDir>
#include <QFile>

#include "Matrix3D.h"

struct FijiConfig
{
    QString fijiExe;
};

class FijiShow3D
{
public:
    FijiShow3D() {}

    void setConfig( const FijiConfig& c ) {
        mConfig = c;
    }

    // gray-scale image
    template<typename T>
    bool run( const Matrix3D<T> &data )
    {
        QString fVolume = QDir::tempPath() + "/annVol.tif";
        QString fScript = QDir::tempPath() + "/annVol.ijm";

        bool ret = data.save( fVolume.toLatin1().constData() );
        if (!ret)
            return false;

        QFile script( fScript );
        if (!script.open(QIODevice::WriteOnly | QIODevice::Text))
            return false;

        // open volume
        script.write( QString("open(\"%1\");").arg(fVolume).toLatin1() );

        // run 3D viewer
        script.write( QString("run(\"3D Viewer\");").toLatin1() );

        // opts
        script.write( QString(
                          "call(\"ij3d.ImageJ3DViewer.setCoordinateSystem\", \"false\");"
                          "call(\"ij3d.ImageJ3DViewer.add\", \"annVol.tif\", \"None\", \"annVol.tif\", \"0\", \"true\", \"true\", \"true\", \"2\", \"0\");"
                          ).toLatin1() );

        // close img window
        script.write( QString("selectWindow(\"annVol.tif\");").toLatin1() );
        script.write( QString("close();").toLatin1() );

        script.close();

        QString toRun = mConfig.fijiExe + " -macro " + fScript + " &";
        if ( !QProcess::startDetached( toRun.toLatin1().constData() ) )
            return false;

        return true;
    }

    // rgb-image, take 3 gray-scale images and merge them in Fiji
    template<typename T>
    bool run( const Matrix3D<T> &dataR, const Matrix3D<T> &dataG, const Matrix3D<T> &dataB )
    {
        QString fVolumeR = QDir::tempPath() + "/annVol-R.tif";
        QString fVolumeG = QDir::tempPath() + "/annVol-G.tif";
        QString fVolumeB = QDir::tempPath() + "/annVol-B.tif";

        QString fScript = QDir::tempPath() + "/annVol.ijm";

        bool ret = dataR.save( fVolumeR.toLatin1().constData() );
        if (!ret)
            return false;

        ret = dataG.save( fVolumeG.toLatin1().constData() );
        if (!ret)
            return false;

        ret = dataB.save( fVolumeB.toLatin1().constData() );
        if (!ret)
            return false;

        QFile script( fScript );
        if (!script.open(QIODevice::WriteOnly | QIODevice::Text))
            return false;

        // open volumes
        script.write( QString("open(\"%1\");").arg(fVolumeR).toLatin1() );
        script.write( QString("open(\"%1\");").arg(fVolumeG).toLatin1() );
        script.write( QString("open(\"%1\");").arg(fVolumeB).toLatin1() );

        // merge
        script.write( QString("run(\"Merge Channels...\", \"red=annVol-R.tif green=annVol-G.tif blue=annVol-B.tif gray=*None* create\");").toLatin1() );

        // run 3D viewer
        script.write( QString("run(\"3D Viewer\");").toLatin1() );

        // opts
        script.write( QString(
                          "call(\"ij3d.ImageJ3DViewer.setCoordinateSystem\", \"false\");"
                          "call(\"ij3d.ImageJ3DViewer.add\", \"Composite\", \"None\", \"Composite\", \"0\", \"true\", \"true\", \"true\", \"2\", \"0\");"
                          ).toLatin1() );

        // close img window
        script.write( QString("selectWindow(\"Composite\");").toLatin1() );
        script.write( QString("close();").toLatin1() );

        script.close();

        QString toRun = mConfig.fijiExe + " -macro " + fScript + " &";
        if ( QProcess::startDetached( toRun.toLatin1().constData() ) == false)
            return false;

        return true;
    }

private:
    FijiConfig mConfig;
};

#endif // FIJIHELPER_H
