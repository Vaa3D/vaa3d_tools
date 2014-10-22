/* cellSegmentation.cpp
 * It aims to automatically segment cells;
 * 2014-10-12 : by Xiang Li (lindbergh.li@gmail.com);
 */
 
#ifndef __CELLSEGMENTATION_PLUGIN_H__
#define __CELLSEGMENTATION_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
#include <sstream>
#include <math.h>

class cellSegmentation : public QObject, public V3DPluginInterface2_1
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

class dialogMain: public QDialog
{
    Q_OBJECT

	public:
    dialogMain(V3DPluginCallback2 &V3DPluginCallback2_currentCallback, QWidget *QWidget_parent, int int_channelDim)
    {
        QPushButton *QPushButton_start = new QPushButton(QObject::tr("start segmentation"));
        QPushButton *QPushButton_close = new QPushButton(QObject::tr("close"));

        QStringList QStringList_channelItems;
        if (int_channelDim==1)
		{
			QStringList_channelItems << "1";
		}
        else if (int_channelDim==3)
        {
            QStringList_channelItems << "1 - red";
            QStringList_channelItems << "2 - green";
            QStringList_channelItems << "3 - blue";
        }
        else //for non-RGB int_channel setups;
        {
            for (int i=1; i<=int_channelDim; i++)
            {
                QStringList_channelItems << QString().setNum(i);
            }
        }
        QComboBox_channel = new QComboBox();
		QComboBox_channel->addItems(QStringList_channelItems);
        QLabel* QLabel_channel = new QLabel(QObject::tr("Channel: "));

        QStringList QStringList_thresholdItems;
        QStringList_thresholdItems << "Calculate from Markers" << "User Input";
        QComboBox_thresholdType = new QComboBox();
		QComboBox_thresholdType->addItems(QStringList_thresholdItems);
        QLabel* QLabel_thresholdType = new QLabel(QObject::tr("Threshold: "));

        QGroupBox *QGroupBox_channel = new QGroupBox("Channel");
        QGroupBox_channel->setStyle(new QWindowsStyle());
        QGridLayout *QGridLayout_channel = new QGridLayout();
        QGroupBox_channel->setStyle(new QWindowsStyle());
        QGridLayout_channel->addWidget(QLabel_channel, 2,1,1,1);
        QGridLayout_channel->addWidget(QComboBox_channel, 2,2,1,2);
        QGroupBox_channel->setLayout(QGridLayout_channel);

        QGroupBox *QGroupBox_threshold = new QGroupBox("Threshold");
        QGroupBox_threshold->setStyle(new QWindowsStyle());
        QGridLayout *QGridLayout_threshold = new QGridLayout();
        QGroupBox_threshold->setStyle(new QWindowsStyle());
        QGridLayout_threshold->addWidget(QLabel_thresholdType, 2,1);
        QGridLayout_threshold->addWidget(QComboBox_thresholdType, 2,2);
        QDoubleSpinBox_threshold = new QDoubleSpinBox();
        QGridLayout_threshold->addWidget(QDoubleSpinBox_threshold, 2,3);
        QDoubleSpinBox_threshold->setEnabled(false);
        QDoubleSpinBox_threshold->setMaximum(65535);
		QDoubleSpinBox_threshold->setMinimum(-65535);
        QGroupBox_threshold->setLayout(QGridLayout_threshold);

        QWidget* QWidget_bottomBar = new QWidget();
        QGridLayout* QGridLayout_bottomBar = new QGridLayout();
        QGridLayout_bottomBar->addWidget(QPushButton_start,1,1);
        QGridLayout_bottomBar->addWidget(QPushButton_close,1,2);
        QWidget_bottomBar->setLayout(QGridLayout_bottomBar);

        QGridLayout *QGridLayout_main = new QGridLayout();
        QGridLayout_main->addWidget(QGroupBox_threshold);
        QGridLayout_main->addWidget(QGroupBox_channel);
        QGridLayout_main->addWidget(QWidget_bottomBar);

        setLayout(QGridLayout_main);
        setWindowTitle(QString("Cell Segmentation"));

        connect(QPushButton_start, SIGNAL(clicked()), this, SLOT(_slot_start()));
        connect(QPushButton_close, SIGNAL(clicked()), this, SLOT(reject()));
        connect(QComboBox_thresholdType, SIGNAL(currentIndexChanged(int)), this, SLOT(update()));

        update();
    }

    ~dialogMain(){}

	public:
    QComboBox* QComboBox_channel;
    QComboBox* QComboBox_thresholdType;
    QDoubleSpinBox* QDoubleSpinBox_threshold;
    V3DLONG int_channel;
    int int_thresholdType;
    double threshold;

	public slots:
    void update()
    {
        int_thresholdType = QComboBox_thresholdType->currentIndex();

        if(int_thresholdType == 1)
        {
            QDoubleSpinBox_threshold->setEnabled(true);
        }
        else
        {
            QDoubleSpinBox_threshold->setEnabled(false);
        }
    }
    void _slot_start()
    {
        int_channel = QComboBox_channel->currentIndex() + 1;
        threshold = -1;
        if (QDoubleSpinBox_threshold->isEnabled())
        {
            threshold = QDoubleSpinBox_threshold->text().toDouble();
        }
        else
		{
			threshold = -1; //calculate automatically;
		}

        accept();
    }
};


#endif

