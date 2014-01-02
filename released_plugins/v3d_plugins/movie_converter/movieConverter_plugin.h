/* Movie_plugin.h
 * Movie Generator
 * 2013-13-10 : by Zhi Zhou
 */
 
#ifndef __MOVIE_PLUGIN_H__
#define __MOVIE_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class MovieConverter : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
    float getPluginVersion() const {return 1.2f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
};

QString getAppPath();

class controlPanel: public QDialog
{
    Q_OBJECT

public:
    controlPanel(V3DPluginCallback2 &v3d, QWidget *parent);
    ~controlPanel();

public:
    long m_lframeind;
    QLineEdit *m_pLineEdit_filepath;
    QLineEdit *m_pLineEdit_fps;
    QLineEdit *m_pLineEdit_filename;
    QLineEdit *m_pLineEdit_ffmpegpath;
    V3DPluginCallback2 &m_v3d;
    static controlPanel *m_pLookPanel;
    QCheckBox* check_compress;
    QComboBox* combo_type;

private slots:
    void _slot_start();
    void _slot_close();
    void _slots_openFileDlg_output();
    void _slots_openFileDlg_ffmpeg();
};

#endif

