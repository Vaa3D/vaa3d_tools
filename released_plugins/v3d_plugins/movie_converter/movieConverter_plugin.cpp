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
controlPanel* controlPanel::m_pLookPanel = 0;
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
        if (controlPanel::m_pLookPanel)
        {
            controlPanel::m_pLookPanel->show();
            return;
        }

        controlPanel* p = new controlPanel(callback, parent);
        if (p)	p->show();
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


controlPanel::controlPanel(V3DPluginCallback2 &_v3d, QWidget *parent) :
    QDialog(parent), m_v3d(_v3d)
{
    m_pLookPanel = this;
    m_lframeind = 0;

    m_pLineEdit_fps = new QLineEdit(QObject::tr("14"));
    m_pLineEdit_filepath = new QLineEdit();
    m_pLineEdit_filename = new QLineEdit(QObject::tr("file_[num].bmp"));
    m_pLineEdit_ffmpegpath = new QLineEdit(getAppPath());
    QPushButton *pPushButton_start = new QPushButton(QObject::tr("convert"));
    QPushButton *pPushButton_close = new QPushButton(QObject::tr("close"));
    QPushButton *pPushButton_openFileDlg_output = new QPushButton(QObject::tr("..."));
    QPushButton *pPushButton_openFileDlg_ffmpeg = new QPushButton(QObject::tr("..."));
    check_compress = new QCheckBox(); check_compress->setText(QObject::tr("Compress "));check_compress->setChecked(true);

    QStringList items;
    items << "avi";
    items << "mpg";
    items << "mp4";
    items << "h264";
    combo_type = new QComboBox(); combo_type->addItems(items);
    QLabel* label_type = new QLabel(QObject::tr("output video type: "));


    QGridLayout *pGridLayout = new QGridLayout();
    pGridLayout->addWidget(new QLabel(QObject::tr("choose ffmpeg dir:")),1,1);
    pGridLayout->addWidget(m_pLineEdit_ffmpegpath,1,2);
    pGridLayout->addWidget(pPushButton_openFileDlg_ffmpeg,1,3);

    pGridLayout->addWidget(new QLabel(QObject::tr("choose input frames dir:")),2,1);
    pGridLayout->addWidget(m_pLineEdit_filepath,2,2);
    pGridLayout->addWidget(pPushButton_openFileDlg_output,2,3);
    pGridLayout->addWidget(new QLabel(QObject::tr("file name format:")),3,1);
    pGridLayout->addWidget(m_pLineEdit_filename,3,2);


    pGridLayout->addWidget(new QLabel(QObject::tr("choose output video fps:")),4,1);
    pGridLayout->addWidget(m_pLineEdit_fps,4,2);

    pGridLayout->addWidget(label_type, 5,1);
    pGridLayout->addWidget(combo_type, 5,2);
    pGridLayout->addWidget(check_compress, 5,3);

    pGridLayout->addWidget(pPushButton_start,6,2);
    pGridLayout->addWidget(pPushButton_close,6,3);


    setLayout(pGridLayout);
    setWindowTitle(QString("Movie Converter"));

    connect(pPushButton_start, SIGNAL(clicked()), this, SLOT(_slot_start()));
    connect(pPushButton_close, SIGNAL(clicked()), this, SLOT(_slot_close()));
    connect(pPushButton_openFileDlg_output, SIGNAL(clicked()), this, SLOT(_slots_openFileDlg_output()));
    connect(pPushButton_openFileDlg_ffmpeg, SIGNAL(clicked()), this, SLOT(_slots_openFileDlg_ffmpeg()));

}
controlPanel::~controlPanel()
{
    m_pLookPanel = 0;
}

void controlPanel::_slot_close()
{
    if (m_pLookPanel)
    {
        delete m_pLookPanel;
        m_pLookPanel=0;
    }
}
void controlPanel::_slot_start()
{
          QString selectffmpeg = m_pLineEdit_ffmpegpath->text().append("/mac_ffmpeg");
          if (!QFile(selectffmpeg).exists())
          {
             v3d_msg("Can not find ffmpeg, please select again or download from http://www.ffmpeg.org");
             return;
          }

          QString selectedFile = 0;
          selectedFile = m_pLineEdit_filepath->text();
          if(selectedFile == 0)
          {
             v3d_msg("Please select an input video frame folder");
             return;
          }

          QString pfs = m_pLineEdit_fps->text();
          QString filename = m_pLineEdit_filename->text();
          int indexL = filename.indexOf("[");
          QString filenameL =  filename.left(indexL);
          int indexR = filename.indexOf("]");
          QString filenameR =  filename.right(filename.size()-indexR-1);
          QString compress;

          QString videoType = combo_type->itemText(combo_type->currentIndex());
          if(videoType == "mp4")
          {
                compress = "-vcodec mpeg4 -acodec aac";
          }
          else if (videoType == "h264")
          {
                    v3d_msg("Please choose another video type at this time.");
                    return;
          }
          else if (videoType == "mpg")
          {
                compress = "-vcodec mpeg2video";

          }
          else if (check_compress->isChecked() && videoType =="avi")
          {
                compress = "-vcodec mjpeg -qscale 0";
          }
          else
              compress = "-vcodec rawvideo";
          QString cmd_ffmpeg = QString("%1 -r %2 -i \'%3/%4%d%5\' -y %6 \'%7/movie.%8\'").arg(selectffmpeg.toStdString().c_str()).arg(pfs.toStdString().c_str()).arg(selectedFile.toStdString().c_str()).arg(filenameL.toStdString().c_str()).arg(filenameR.toStdString().c_str()).arg(compress.toStdString().c_str()).arg(selectedFile.toStdString().c_str()).arg(videoType.toStdString().c_str());
          system(qPrintable(cmd_ffmpeg));
          //-vcodec mjpeg -qscale 0
          QString movieDir = selectedFile.append(videoType);
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

void controlPanel::_slots_openFileDlg_output()
{
    QFileDialog d(this);
    d.setWindowTitle(tr("Choose movie frames dir:"));
    d.setFileMode(QFileDialog::Directory);
    if(d.exec())
    {
        QString selectedFile=(d.selectedFiles())[0];
        m_pLineEdit_filepath->setText(selectedFile);
    }
}

void controlPanel::_slots_openFileDlg_ffmpeg()
{
    QFileDialog d(this);
    d.setWindowTitle(tr("Choose ffmpeg dir:"));
    d.setFileMode(QFileDialog::Directory);
    if(d.exec())
    {
        QString selectedFile=(d.selectedFiles())[0];
        m_pLineEdit_ffmpegpath->setText(selectedFile);
    }
}
