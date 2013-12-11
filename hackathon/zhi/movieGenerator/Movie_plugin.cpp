/* Movie_plugin.cpp
 * Movie Generator
 * 2013-13-10 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "Movie_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(Movie, MovieGenerator);
 
QStringList MovieGenerator::menulist() const
{
	return QStringList() 
                        << tr("convert a movie format using ffmpeg")
                        <<tr("about");
}

QStringList MovieGenerator::funclist() const
{
	return QStringList()
		<<tr("help");
}

void MovieGenerator::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("convert a movie format using ffmpeg"))
	{
        QFileDialog d;
        d.setWindowTitle(tr("Choose movie frames dir:"));
        d.setFileMode(QFileDialog::Directory);
        if(d.exec())
        {
             QString selectedFile=(d.selectedFiles())[0];
             QString cmd_ffmpeg = QString("./bin/mac_ffmpeg -f image2 -i \'%1/a*%d.BMP\' -y \'%2/movie.mpg\'").arg(selectedFile.toStdString().c_str()).arg(selectedFile.toStdString().c_str());
             system(qPrintable(cmd_ffmpeg));
             QString movieDir = selectedFile.append("/movie.mpg");
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
		v3d_msg(tr("Movie Format Converter (from sequence of frames to a mpeg file). "
            "Developed by Zhi Zhou and Hanchuan Peng, 2013-12-10"));
	}
}

bool MovieGenerator::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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

