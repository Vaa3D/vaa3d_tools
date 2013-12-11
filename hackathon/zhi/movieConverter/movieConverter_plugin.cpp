/* Movie_plugin.cpp
 * Movie Converter
 * 2013-13-10 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "movieConverter_plugin.h"
#include <boost/lexical_cast.hpp>
using namespace std;
Q_EXPORT_PLUGIN2(Movie, MovieConverter);
 
QStringList MovieConverter::menulist() const
{
	return QStringList() 
                        << tr("convert a movie format using ffmpeg")
                        <<tr("about");
}

QStringList MovieConverter::funclist() const
{
	return QStringList()
		<<tr("help");
}

void MovieConverter::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("convert a movie format using ffmpeg"))
	{
        QFileDialog d;
        d.setWindowTitle(tr("Choose movie frames dir:"));
        d.setFileMode(QFileDialog::Directory);
        if(d.exec())
        {
             QString selectedFile=(d.selectedFiles())[0];
             //input
             bool ok1;
             int pfs = 30;

             pfs = QInputDialog::getInteger(parent, "Frame rate",
                                           "Enter frame rate:",
                                           30, 1, 200, 1, &ok1);
             if(!ok1)
                 return;
             string pfs_s = boost::lexical_cast<string>(pfs);
             QString lociDir = getAppPath().append("/mac_ffmpeg");
             QString cmd_ffmpeg = QString("%1 -r %2 -i \'%3/a%d.BMP\' -y -vcodec mjpeg -qscale 0 \'%4/movie.avi\'").arg(lociDir.toStdString().c_str()).arg(pfs_s.c_str()).arg(selectedFile.toStdString().c_str()).arg(selectedFile.toStdString().c_str());
             system(qPrintable(cmd_ffmpeg));
             QString movieDir = selectedFile.append("/movie.avi");
             if (!QFile(movieDir).exists())
             {
                v3d_msg("The format is not supported, or something is wrong in your file\n");
                return;
             }
             else
             {
                v3d_msg(QString("The movie is saved in %1").arg(movieDir));
                return;
             }
        }
    }
	else
	{
        v3d_msg(tr("Movie Format Converter (from sequence of frames to an avi file). "
            "Developed by Zhi Zhou and Hanchuan Peng, 2013-12-10"));
	}
}

bool MovieConverter::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

QString getAppPath()
{
    QString v3dAppPath("~/Work/v3d_external/v3d");
    QDir testPluginsDir = QDir(qApp->applicationDirPath());

#if defined(Q_OS_WIN)
    if (testPluginsDir.dirName().toLower() == "debug" || testPluginsDir.dirName().toLower() == "release")
        testPluginsDir.cdUp();
#elif defined(Q_OS_MAC)
    // In a Mac app bundle, plugins directory could be either
    //  a - below the actual executable i.e. v3d.app/Contents/MacOS/plugins/
    //  b - parallel to v3d.app i.e. foo/v3d.app and foo/plugins/
    if (testPluginsDir.dirName() == "MacOS") {
        QDir testUpperPluginsDir = testPluginsDir;
        testUpperPluginsDir.cdUp();
        testUpperPluginsDir.cdUp();
        testUpperPluginsDir.cdUp(); // like foo/plugins next to foo/v3d.app
        if (testUpperPluginsDir.cd("plugins")) testPluginsDir = testUpperPluginsDir;
        testPluginsDir.cdUp();
    }
#endif

    v3dAppPath = testPluginsDir.absolutePath();
    return v3dAppPath;
}
