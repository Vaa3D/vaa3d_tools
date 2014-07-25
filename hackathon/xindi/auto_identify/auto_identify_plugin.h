/* auto_identify_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2012-01-01 : by YourName
 */
 
#ifndef __AUTO_IDENTIFY_PLUGIN_H__
#define __AUTO_IDENTIFY_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
#include <sstream>

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
    Dialog_SWC(V3DPluginCallback2 &cb, QWidget *parent, int sc)
    {
        QString exepath = getAppPath();

        m_pLineEdit_testfilepath = new QLineEdit();
        m_pLineEdit_outputfilepath = new QLineEdit(exepath);
        QPushButton *pPushButton_start = new QPushButton(QObject::tr("start labeling"));
        QPushButton *pPushButton_close = new QPushButton(QObject::tr("close"));
        QPushButton *pPushButton_openFileDlg_input = new QPushButton(QObject::tr("Browse"));
        QPushButton *pPushButton_openFileDlg_output = new QPushButton(QObject::tr("Browse"));

        QStringList ch_items;
        if (sc==1) {ch_items << "1";}
        else if (sc==3)
        {
            ch_items << "1 - red";
            ch_items << "2 - green";
            ch_items << "3 - blue";
        }
        else
        {
            for (int i=1; i<=sc; i++)
            {
                std::stringstream chStr;
                chStr << i;
                QString qstr = QString::fromStdString(chStr.str());
                ch_items << qstr;
            }
        }
        combo_channelno = new QComboBox(); combo_channelno->addItems(ch_items);
        QLabel* ch_label_type = new QLabel(QObject::tr("Channel: "));

        QGroupBox *input_panel = new QGroupBox("Input");
        input_panel->setStyle(new QWindowsStyle());
        QGridLayout *inputLayout = new QGridLayout();
        input_panel->setStyle(new QWindowsStyle());
        inputLayout->addWidget(new QLabel(QObject::tr("Labeled Example SWC:")),1,1);
        inputLayout->addWidget(m_pLineEdit_testfilepath,2,1,1,2);
        inputLayout->addWidget(pPushButton_openFileDlg_input,2,3,1,1);
        input_panel->setLayout(inputLayout);

        QGroupBox *output_panel = new QGroupBox("Output");
        output_panel->setStyle(new QWindowsStyle());
        QGridLayout *outputLayout = new QGridLayout();
        outputLayout->addWidget(new QLabel(QObject::tr("Choose directory to save labeled output SWC:")),1,1);
        outputLayout->addWidget(m_pLineEdit_outputfilepath,2,1,1,2);
        outputLayout->addWidget(pPushButton_openFileDlg_output,2,3,1,1);
        output_panel->setLayout(outputLayout);


        QGroupBox *channel_panel = new QGroupBox("Channel");
        channel_panel->setStyle(new QWindowsStyle());
        QGridLayout *channelLayout = new QGridLayout();
        channel_panel->setStyle(new QWindowsStyle());
        channelLayout->addWidget(ch_label_type, 2,1,1,1);
        channelLayout->addWidget(combo_channelno, 2,2,1,2);
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
        connect(pPushButton_close, SIGNAL(clicked()), this, SLOT(reject()));
        connect(pPushButton_openFileDlg_input, SIGNAL(clicked()), this, SLOT(_slots_openFileDlg_input()));
        connect(pPushButton_openFileDlg_output, SIGNAL(clicked()), this, SLOT(_slots_openFileDlg_output()));
    }

    ~Dialog_SWC(){}

public:
    QLineEdit *m_pLineEdit_testfilepath;
    QLineEdit *m_pLineEdit_outputfilepath;
    QComboBox* combo_channelno;
   // static controlPanel_SWC *m_pLookPanel_SWC;
    QString infileName;
    QString outfileName;
    int channel;

public slots:
    void _slot_start()
    {
        infileName = m_pLineEdit_testfilepath->text();
        if (!infileName.toUpper().endsWith(".SWC"))
        {
            v3d_msg("You did not choose a valid file type, or the example file you chose is empty. Will attempt to find exampler set in window.");
        }

        outfileName= m_pLineEdit_outputfilepath->text();
        if (!QFile(outfileName).exists())
        {
            v3d_msg("Output file path does not exist, ");
        }

        int chInt = combo_channelno->currentIndex();
        channel = chInt+1;

        accept();
    }
    void _slots_openFileDlg_input()
    {
        QFileDialog d(this);
        QString fileOpenName;
        fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open Example SWC File"),
                "",
                QObject::tr("Supported file (*.swc)"));;
        if(!fileOpenName.isEmpty())
        {
            m_pLineEdit_testfilepath->setText(fileOpenName);
        }
    }
    void _slots_openFileDlg_output()
    {
        QFileDialog d(this);
        d.setWindowTitle(tr("Choose save dir:"));
        d.setFileMode(QFileDialog::Directory);
        if(d.exec())
        {
            QString selectedFile=(d.selectedFiles())[0];
            m_pLineEdit_outputfilepath->setText(selectedFile);
        }
    }
};

class Dialog_cells: public QDialog
{
    Q_OBJECT

public:
    Dialog_cells(V3DPluginCallback2 &cb, QWidget *parent, int sc, int p)
    {
        QPushButton *pPushButton_start = new QPushButton(QObject::tr("start counting"));
        QPushButton *pPushButton_close = new QPushButton(QObject::tr("close"));

        QStringList sort_items;
        sort_items << "Default (binary) sort";
        sort_items << "Type/Category sort";
        combo_sort_type = new QComboBox(); combo_sort_type->addItems(sort_items);
        QLabel* sort_label_type = new QLabel(QObject::tr("Sorting Options: "));

        QStringList ch_items;
        if (sc==1) {ch_items << "1";}
        else if (sc==3)
        {
            ch_items << "1 - red";
            ch_items << "2 - green";
            ch_items << "3 - blue";
        }
        else
        {
            for (int i=1; i<=sc; i++)
            {
                std::stringstream chStr;
                chStr << i;
                QString qstr = QString::fromStdString(chStr.str());
                ch_items << qstr;
            }
        }
        combo_channelno = new QComboBox(); combo_channelno->addItems(ch_items);
        QLabel* ch_label_type = new QLabel(QObject::tr("Channel: "));

        QStringList input_items;
        input_items << "Markers only";
        input_items << "SWCs only";
        input_items << "Markers and SWCs";
        combo_input_type = new QComboBox(); combo_input_type->addItems(input_items);
        QLabel* input_label_type = new QLabel(QObject::tr("Example Data Input Type: "));

        QGroupBox *sort_panel = new QGroupBox("Sorting");
        sort_panel->setStyle(new QWindowsStyle());
        QGridLayout *sortLayout = new QGridLayout();
        sort_panel->setStyle(new QWindowsStyle());
        sortLayout->addWidget(sort_label_type, 2,1,1,1);
        sortLayout->addWidget(combo_sort_type, 2,2,1,2);
        sort_panel->setLayout(sortLayout);

        QGroupBox *channel_panel = new QGroupBox("Channel");
        channel_panel->setStyle(new QWindowsStyle());
        QGridLayout *channelLayout = new QGridLayout();
        channel_panel->setStyle(new QWindowsStyle());
        channelLayout->addWidget(ch_label_type, 2,1,1,1);
        channelLayout->addWidget(combo_channelno, 2,2,1,2);
        channel_panel->setLayout(channelLayout);

        QGroupBox *input_panel = new QGroupBox("Input Data Type");
        input_panel->setStyle(new QWindowsStyle());
        QGridLayout *inputLayout = new QGridLayout();
        input_panel->setStyle(new QWindowsStyle());
        inputLayout->addWidget(input_label_type, 2,1,1,1);
        inputLayout->addWidget(combo_input_type, 2,2,1,2);
        input_panel->setLayout(inputLayout);

        QWidget* container = new QWidget();
        QGridLayout* bottomBar = new QGridLayout();
        bottomBar->addWidget(pPushButton_start,1,1);
        bottomBar->addWidget(pPushButton_close,1,2);
        container->setLayout(bottomBar);

        QGridLayout *pGridLayout = new QGridLayout();
        if (p==0) {pGridLayout->addWidget(input_panel);}
        pGridLayout->addWidget(sort_panel);
        pGridLayout->addWidget(channel_panel);
        pGridLayout->addWidget(container);

        setLayout(pGridLayout);
        setWindowTitle(QString("Cell Counting"));

        input_type = p;

        connect(pPushButton_start, SIGNAL(clicked()), this, SLOT(_slot_start()));
        connect(pPushButton_close, SIGNAL(clicked()), this, SLOT(reject()));
    }

    ~Dialog_cells(){}

public:
    QComboBox* combo_sort_type;
    QComboBox* combo_channelno;
    QComboBox* combo_input_type;
   // static controlPanel_SWC *m_pLookPanel_SWC;
    int sort_type;
    int channel;
    int input_type;

public slots:
    void _slot_start()
    {
        sort_type = combo_sort_type->currentIndex();
        //sort_type = 0 for default
        //            1 for type/cat

        int chInt = combo_channelno->currentIndex();
        channel = chInt+1;

        if (input_type==0)
        {
            int inpInt = combo_input_type->currentIndex();
            input_type = inpInt+1;
            //1 for mark, 2 for swc, 3 for both
        }

        accept();
    }
};


#endif

