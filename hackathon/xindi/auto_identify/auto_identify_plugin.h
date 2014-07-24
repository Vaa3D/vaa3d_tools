/* auto_identify_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2012-01-01 : by YourName
 */
 
#ifndef __AUTO_IDENTIFY_PLUGIN_H__
#define __AUTO_IDENTIFY_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class AutoIdentifyPlugin : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
};

QString getAppPath();

class Dialog_SWC: public QDialog
{
    Q_OBJECT

public:
    Dialog_SWC(V3DPluginCallback2 &cb, QWidget *parent)
    {
        QString exepath = getAppPath();

        m_pLineEdit_testfilepath = new QLineEdit();
        m_pLineEdit_outputfilepath = new QLineEdit(exepath);
        m_pLineEdit_channelno = new QLineEdit(QObject::tr("1"));
        QPushButton *pPushButton_start = new QPushButton(QObject::tr("start labeling"));
        QPushButton *pPushButton_close = new QPushButton(QObject::tr("close"));
        QPushButton *pPushButton_openFileDlg_input = new QPushButton(QObject::tr("Browse"));
        QPushButton *pPushButton_openFileDlg_output = new QPushButton(QObject::tr("Browse"));

        QGroupBox *input_panel = new QGroupBox("Input:");
        input_panel->setStyle(new QWindowsStyle());
        QGridLayout *inputLayout = new QGridLayout();
        input_panel->setStyle(new QWindowsStyle());
        inputLayout->addWidget(new QLabel(QObject::tr("Labeled Example SWC:")),1,1);
        inputLayout->addWidget(m_pLineEdit_testfilepath,2,1,1,2);
        inputLayout->addWidget(pPushButton_openFileDlg_input,2,3,1,1);
        input_panel->setLayout(inputLayout);

        QGroupBox *output_panel = new QGroupBox("Output:");
        output_panel->setStyle(new QWindowsStyle());
        QGridLayout *outputLayout = new QGridLayout();
        outputLayout->addWidget(new QLabel(QObject::tr("Choose directory to save labeled SWC:")),1,1);
        outputLayout->addWidget(m_pLineEdit_outputfilepath,2,1,1,2);
        outputLayout->addWidget(pPushButton_openFileDlg_output,2,3,1,1);
        output_panel->setLayout(outputLayout);


        QGroupBox *channel_panel = new QGroupBox("Channel Number:");
        channel_panel->setStyle(new QWindowsStyle());
        QGridLayout *channelLayout = new QGridLayout();
        channelLayout->addWidget(new QLabel(QObject::tr("Channel:")),4,1);
        channelLayout->addWidget(m_pLineEdit_channelno,4,2);
        channel_panel->setLayout(channelLayout);

        QWidget* container = new QWidget();
        QGridLayout* bottomBar = new QGridLayout();
        bottomBar->addWidget(pPushButton_start,1,1);
        bottomBar->addWidget(pPushButton_close,1,2);
        container->setLayout(bottomBar);

        QGridLayout *pGridLayout = new QGridLayout();
        pGridLayout->addWidget(input_panel);
        pGridLayout->addWidget(output_panel);
        pGridLayout->addWidget(channel_panel);
        pGridLayout->addWidget(container);

        setLayout(pGridLayout);
        setWindowTitle(QString("Label currently displayed SWC using example file"));

        connect(pPushButton_start, SIGNAL(clicked()), this, SLOT(_slot_start()));
        connect(pPushButton_close, SIGNAL(clicked()), this, SLOT(_slot_close()));
        connect(pPushButton_openFileDlg_input, SIGNAL(clicked()), this, SLOT(_slots_openFileDlg_input()));
        connect(pPushButton_openFileDlg_output, SIGNAL(clicked()), this, SLOT(_slots_openFileDlg_output()));
    }

    ~Dialog_SWC(){}

public:
    QLineEdit *m_pLineEdit_testfilepath;
    QLineEdit *m_pLineEdit_outputfilepath;
    QLineEdit *m_pLineEdit_channelno;
   // static controlPanel_SWC *m_pLookPanel_SWC;
    QString infileName;
    QString outfileName;
    int channel;

public slots:
    void _slot_start()
    {}
    void _slot_close()
    {}
    void _slots_openFileDlg_input()
    {}
    void _slots_openFileDlg_output()
    {}
};

#endif

