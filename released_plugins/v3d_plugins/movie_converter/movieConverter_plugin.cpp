/* Movie_plugin.cpp
 * Movie Converter
 * 2013-13-10 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "movieConverter_plugin.h"
#include <boost/lexical_cast.hpp>
#include <iostream>

using namespace std;
Q_EXPORT_PLUGIN2(movie_format_convert, MovieConverter);
controlPanel* controlPanel::m_pLookPanel = 0;
QStringList MovieConverter::menulist() const
{
	return QStringList() 
                        << tr("convert a series of movie frames to a single movie file")
                        <<tr("about");
}

QStringList MovieConverter::funclist() const
{
	return QStringList()
            <<tr("convert_frames_to_movie")
        <<tr("help");
}

void MovieConverter::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("convert a series of movie frames to a single movie file"))
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
        v3d_msg(tr("Movie Format Converter (from sequence of frames to a single movie file). "
            "Developed by Zhi Zhou and Hanchuan Peng, 2013-12-10"));
    }


}

bool MovieConverter::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("convert_frames_to_movie"))
    {
        cout<<"Welcome to movie converter"<<endl;

        char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
        const char * ffmpeg_file = getAppPath().append("/mac_ffmpeg").toStdString().c_str();
        QString check_ffmpeg;
        char * inimg_format = "file_[NUM].bmp";
        char * output_fps = "1";
        unsigned int output_Type = 1, c = 1;
        if (input.size()>=2)
        {

            vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
            cout<<paras.size()<<endl;
            if(paras.size() >= 1) check_ffmpeg = paras[0];
            if(check_ffmpeg != "NULL") ffmpeg_file = paras[0] ;
            if(paras.size() >= 2) inimg_format = paras[1];// ((vector<char*> *)(input.at(1).p))->at(1);
            if(paras.size() >= 3) output_fps = paras[2];// ((vector<char*> *)(input.at(1).p))->at(2);
            if(paras.size() >= 4) output_Type = atoi(paras.at(3));
            if(paras.size() >= 5) c = atoi(paras.at(4));
        }

        cout<<"input_folder = "<<inimg_file<<endl;
        cout<<"ffmpeg_file = "<<ffmpeg_file<<endl;
        cout<<"input_format = "<<inimg_format<<endl;
        cout<<"output_fps = "<<output_fps<<endl;
        cout<<"output_type = "<<output_Type<<endl;
        cout<<"compress = "<<c<<endl;

        QString selectedFile(inimg_file);
        QString selectffmpeg(ffmpeg_file);
        QString filename(inimg_format);
        QString fps(output_fps);

        if (!QFile(selectffmpeg).exists())
        {
           printf("Can not find ffmpeg, please select again or download from http://www.ffmpeg.org");
           return false;
        }

        int indexL = filename.indexOf("[");
        QString filenameL =  filename.left(indexL);
        int indexR = filename.indexOf("]");
        QString filenameR =  filename.right(filename.size()-indexR-1);
        QString compress;
        QString videoType;
        if(output_Type == 3)
        {
              compress = "-vcodec mpeg4 -acodec aac";
              videoType = "mp4";

        }
        else if (output_Type == 2)
        {
              compress = "-vcodec mpeg2video";
              videoType = "mpg";

        }
        else if (c == 1 && output_Type == 1)
        {
              compress = "-vcodec mjpeg -qscale 0";
              videoType = "avi";
        }
        else if (c == 0 && output_Type == 1)
        {
              compress = "-vcodec rawvideo";
              videoType = "avi";
        }
        else
        {
            return false;
        }
        QString cmd_ffmpeg = QString("%1 -r %2 -i \'%3/%4%d%5\' -y %6 \'%7/movie.%8\'").arg(selectffmpeg.toStdString().c_str()).arg(fps.toStdString().c_str()).arg(selectedFile.toStdString().c_str()).arg(filenameL.toStdString().c_str()).arg(filenameR.toStdString().c_str()).arg(compress.toStdString().c_str()).arg(selectedFile.toStdString().c_str()).arg(videoType.toStdString().c_str());
        system(qPrintable(cmd_ffmpeg));
        //-vcodec mjpeg -qscale 0
        QString movieDir = selectedFile.append(QString("/movie.%1").arg(videoType));

        if (!QFile(movieDir).exists())
        {
           printf("The format is not supported, or can not find ffmpeg, please select again or download from http://www.ffmpeg.org\n");
           return false;
        }
        else
        {
           cout<<"The movie is saved in "<<movieDir.toStdString()<<endl;
           return true;
        }
    }
    else if (func_name == tr("help"))
	{
        printf("\n**** Usage of Movie Converter ****\n");
        printf("vaa3d -x plugin_name -f convert_frames_to_movie -i <inimg_folder> -p <converter_path> <frame_format> <output_fps> <video_type> <is_compress>\n");
        printf("inimg_folder     the video frame folder path \n");
        printf("converter_path   the movie converter(ffmpeg) path. If do not know, please set this para to NULL and the default path will be the vaa3d main path.\n");
        printf("frame_format     the format of video frames,e.g file_[NUM].bmp\n");
        printf("output_fps       the fps for the output video. Default 14\n");
        printf("video_type       the output video type (1 for avi, 2 for mpg, and 3 for mp4. Default 1.\n");
        printf("is_compress      if compress the video (1 for yes and 0 for no. Default 1.\n");

        return true;
	}
	else return false;
}

QString getAppPath()
{
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

    return testPluginsDir.absolutePath();
}


controlPanel::controlPanel(V3DPluginCallback2 &_v3d, QWidget *parent) :
    QDialog(parent), m_v3d(_v3d)
{
    m_pLookPanel = this;
    m_lframeind = 0;

    m_pLineEdit_fps = new QLineEdit(QObject::tr("14"));
    m_pLineEdit_filepath = new QLineEdit();
    m_pLineEdit_filename = new QLineEdit(QObject::tr("file_[num].bmp"));
    m_pLineEdit_ffmpegpath = new QLineEdit(getAppPath().append("/mac_ffmpeg"));
    QPushButton *pPushButton_start = new QPushButton(QObject::tr("convert"));
    QPushButton *pPushButton_close = new QPushButton(QObject::tr("close"));
    QPushButton *pPushButton_openFileDlg_output = new QPushButton(QObject::tr("..."));
    QPushButton *pPushButton_openFileDlg_ffmpeg = new QPushButton(QObject::tr("..."));
    check_compress = new QCheckBox(); check_compress->setText(QObject::tr("Compress "));check_compress->setChecked(true);

    QStringList items;
    items << "avi";
    items << "mpg";
    items << "mp4";
   // items << "h264";
    combo_type = new QComboBox(); combo_type->addItems(items);
    QLabel* label_type = new QLabel(QObject::tr("Output video type: "));


    QGridLayout *pGridLayout = new QGridLayout();
    pGridLayout->addWidget(new QLabel(QObject::tr("Movie converter (ffmpeg) path:")),1,1);
    pGridLayout->addWidget(m_pLineEdit_ffmpegpath,1,2);
    pGridLayout->addWidget(pPushButton_openFileDlg_ffmpeg,1,3);

    pGridLayout->addWidget(new QLabel(QObject::tr("Input movie-frame folder:")),2,1);
    pGridLayout->addWidget(m_pLineEdit_filepath,2,2);
    pGridLayout->addWidget(pPushButton_openFileDlg_output,2,3);
    pGridLayout->addWidget(new QLabel(QObject::tr("Input movie-frame file-naming format:")),3,1);
    pGridLayout->addWidget(m_pLineEdit_filename,3,2);


    pGridLayout->addWidget(new QLabel(QObject::tr("Output video fps:")),4,1);
    pGridLayout->addWidget(m_pLineEdit_fps,4,2);

    pGridLayout->addWidget(label_type, 5,1);
    pGridLayout->addWidget(combo_type, 5,2);
    pGridLayout->addWidget(check_compress, 5,3);

    pGridLayout->addWidget(pPushButton_start,6,2);
    pGridLayout->addWidget(pPushButton_close,6,3);


    setLayout(pGridLayout);
    setWindowTitle(QString("Convert movie frames to a single movie file"));

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
          QString selectffmpeg = m_pLineEdit_ffmpegpath->text();
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

          QString fps = m_pLineEdit_fps->text();
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
          QString cmd_ffmpeg = QString("%1 -r %2 -i \'%3/%4%d%5\' -y %6 \'%7/movie.%8\'").arg(selectffmpeg.toStdString().c_str()).arg(fps.toStdString().c_str()).arg(selectedFile.toStdString().c_str()).arg(filenameL.toStdString().c_str()).arg(filenameR.toStdString().c_str()).arg(compress.toStdString().c_str()).arg(selectedFile.toStdString().c_str()).arg(videoType.toStdString().c_str());
          system(qPrintable(cmd_ffmpeg));
          //-vcodec mjpeg -qscale 0
          QString movieDir = selectedFile.append(QString("/movie.%1").arg(videoType));
          v3d_msg(movieDir);
          if (!QFile(movieDir).exists())
          {
             v3d_msg("The format is not supported, or can not find ffmpeg, please select again or download from http://www.ffmpeg.org\n");
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
    QString fileOpenName;
    fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Choose ffmpeg dir:"));
    if(!fileOpenName.isEmpty())
    {
        m_pLineEdit_ffmpegpath->setText(fileOpenName);
    }
}
