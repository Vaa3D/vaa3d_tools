#include <set>
#include <cmath>
#include "PLog.h"

using namespace teramanager;

PLog* PLog::uniqueInstance = 0;

PLog::PLog(QWidget *parent) : QDialog(parent)
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    setWindowTitle("TeraFly's log");

    timeComponents = new QTextEdit();
    timeComponents->setReadOnly(true);
    timeComponents->setMinimumHeight(10);
    QFont font = QFont("Courier New", 9);
    timeComponents->setFont(font);
    timeComponents->setWordWrapMode(QTextOption::NoWrap);

    timeOperations = new QTextEdit();
    timeOperations->setReadOnly(true);
    timeOperations->setMinimumHeight(10);
    timeOperations->setFont(font);
    timeOperations->setWordWrapMode(QTextOption::NoWrap);
    enableIoCoreOperationsCheckBox = new QCheckBox("I/O core logs");
    enableIoCoreOperationsCheckBox->setToolTip("Enable I/O core logs with very low-level performance measures.");
    enableIoCoreOperationsCheckBox->setChecked(false);
    enableIoCoreOperations = false;

    log = new QTextEdit();
    log->setReadOnly(true);
    log->setMinimumHeight(100);
    log->setFont(font);
    log->setWordWrapMode(QTextOption::NoWrap);


    autoUpdateCheckBox = new QCheckBox("Auto update");
    autoUpdateCheckBox->setChecked(false);
    autoUpdateCheckBox->setToolTip("Enable automatic update of log and performance measures.");
    updatePushButton = new QPushButton("Update");
    closeButton = new QPushButton("Close");
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
    resetButton = new QPushButton("Reset");
    connect(resetButton, SIGNAL(clicked()), this, SLOT(reset()));

    timeComponentsPanel = new QGroupBox("System performance (components time)");
    QHBoxLayout* timeComponentsPanelLayout = new QHBoxLayout();
    timeComponentsPanelLayout->addWidget(timeComponents);
    timeComponentsPanel->setMaximumHeight(160);
    timeComponentsPanel->setLayout(timeComponentsPanelLayout);
    #ifdef Q_OS_LINUX
    timeComponentsPanel->setStyle(new QWindowsStyle());
    #endif

    timeOperationsPanel = new QGroupBox("TeraFly's performance (average operation time)");
    QVBoxLayout* timeOperationsPanelLayout = new QVBoxLayout();
    timeOperationsPanelLayout->addWidget(timeOperations);
    timeOperationsPanelLayout->addWidget(enableIoCoreOperationsCheckBox);
    timeOperationsPanel->setLayout(timeOperationsPanelLayout);
    #ifdef Q_OS_LINUX
    timeOperationsPanel->setStyle(new QWindowsStyle());
    #endif

    logPanel = new QGroupBox("Log");
    QHBoxLayout* logPanelLayout = new QHBoxLayout();
    logPanelLayout->addWidget(log);
    logPanel->setLayout(logPanelLayout);
    #ifdef Q_OS_LINUX
    logPanel->setStyle(new QWindowsStyle());
    #endif
    logPanel->setEnabled(false);

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(resetButton, 1);
    buttonsLayout->addWidget(closeButton, 1);
    buttonsLayout->setContentsMargins(0,0,0,0);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(timeComponentsPanel);
    layout->addWidget(timeOperationsPanel);
    layout->addWidget(logPanel);
    layout->addWidget(autoUpdateCheckBox);
    layout->addWidget(updatePushButton);
    layout->addLayout(buttonsLayout);
    setLayout(layout);

    this->setMinimumWidth(500);

    connect(this, SIGNAL(sendAppend(void*)), this, SLOT(appendOperationVoid(void*)), Qt::QueuedConnection);
    connect(enableIoCoreOperationsCheckBox, SIGNAL(stateChanged(int)), this, SLOT(enableIoCoreOperationsCheckBoxChanged(int)));
    connect(autoUpdateCheckBox, SIGNAL(stateChanged(int)), this, SLOT(autoUpdateCheckBoxChanged(int)));
    connect(updatePushButton, SIGNAL(clicked()), this, SLOT(updatePushButtonClicked()));


    reset();
}

//extracts from the given time field in the corresponding time measure
float PLog::toFloat(QString timeField)
{
    string str = timeField.toStdString();
    str = str.substr(0, str.find('s'));
    QString qstr = str.c_str();
    return qstr.toFloat();
}

//update
void PLog::update()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    // time components
    float timeIOperc = (timeIO / timeSum) * 100.0f;
    float timeGPUperc = (timeGPU / timeSum) * 100.0f;
    float timeCPUperc = (timeCPU / timeSum) * 100.0f;
    timeSum = timeIO + timeGPU + timeCPU;
    float perfGainf = ( (timeSum-timeActual)/timeSum ) * 100.0f;
    if(perfGainf < 0.0f)
        perfGainf = 0.0f;
    QString compText =  "Actual waiting time (with parallelism): " + QString::number(timeActual, 'f', 3).append("s\n") +
                        "Total  waiting time (w/o parallelism) : " + QString::number(timeSum, 'f', 3).append("s\n") +
                        "Performance gain : " + QString::number(perfGainf, 'f', 1).append("\%\n") +
                        "I/O waiting time: " + QString::number(timeIO, 'f', 3).append("s (").append(QString::number(timeIOperc, 'f', 1)).append("\%)\n") +
                        "GPU waiting time: " + QString::number(timeGPU, 'f', 3).append("s (").append(QString::number(timeGPUperc, 'f', 1)).append("\%)\n") +
                        "CPU waiting time: " + QString::number(timeCPU, 'f', 3).append("s (").append(QString::number(timeCPUperc, 'f', 1)).append("\%)\n");
    timeComponents->setText(compText);

    // time operations
    std::string opText;
    // group by operation
    for(std::map< std::string, std::vector<itm::Operation*> >::iterator it = loggedOperations.begin(); it != loggedOperations.end(); it++)
    {
        // group by component
        for(int c=0; c<4; c++)
        {
            // find macro-groups ids
            std::set< int > gids;
            std::string compName;
            for(int k=0; k< it->second.size(); k++)
                if(it->second[k]->comp == c)
                {
                    gids.insert(it->second[k]->groupID);
                    if(compName.empty())
                        compName = it->second[k]->compName();
                }

            // if something has been found
            if(!gids.empty())
            {
                // first accumulate individual contributions to an aggregate measure
                std::vector < int > measures;
                for(std::set< int >::iterator git = gids.begin(); git != gids.end(); git++)
                {
                    measures.push_back(0);
                    for(int k=0; k< it->second.size(); k++)
                        if(it->second[k]->comp == c && it->second[k]->groupID == *git)
                            measures.back() += it->second[k]->milliseconds;
                }

                // then calculate average and stdev
                double sum=0, sumsq=0;
                for(int k=0; k< measures.size(); k++)
                {
                    sum   += measures[k];
                    sumsq += std::pow(measures[k], 2.0);
                }

                // and display the result
                opText += it->first + "(" + compName + "): " + QString::number((sum/gids.size())/1000.0, 'f', 3).toStdString() + "s " + QChar(0x00B1).toAscii() + " " + QString::number( std::sqrt( sumsq/gids.size() - (sum/gids.size())*(sum/gids.size()) )/1000.0, 'f', 3).toStdString() + " (x" + QString::number(gids.size()).toStdString() + ", sum = " + QString::number(sum/1000.0, 'f', 1).toStdString() + " s)"+ "\n";
            }
        }
        opText += "\n";
    }
    timeOperations->setText(opText.c_str());
}

void PLog::append(std::string text)
{
    log->append(QString("@").append(QTime::currentTime().toString()).append(": ").append(text.c_str()));
}

void PLog::appendOperation(itm::Operation *op, bool update_time_comps /* = true */)
{
    // add operation to its group vector
    loggedOperations[op->name()].push_back(op);

    // update GUI
    if(autoUpdateCheckBox->isChecked())
    {
        // update time components
        if(update_time_comps)
        {
            // add operation to log
            this->append( std::string("[") + QString::number(op->groupID).toStdString() + "]" + op->name() + "(" + op->compName() + ")( " + QString::number(op->milliseconds/1000.0f, 'f', 3).toStdString() + "s ): " + op->message);

            if(op->comp == itm::IO)
                timeIO += op->milliseconds / 1000.0f;
            else if(op->comp == itm::GPU)
                timeGPU += op->milliseconds / 1000.0f;
            else if(op->comp == itm::CPU)
                timeCPU += op->milliseconds / 1000.0f;
            else if(op->comp == itm::ALL_COMPS)
                timeActual += op->milliseconds / 1000.0f;
        }

        this->update();
    }
}


void PLog::reset()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    timeIO = timeGPU = timeCPU = timeActual = 0.0f;

    timeOperations->setText("");
    timeComponents->setText("");
    log->setText("");
    append(QString("Log started on ").append(QDateTime::currentDateTime().date().toString()).toStdString());

    for(std::map< std::string, std::vector<itm::Operation*> >::iterator it = loggedOperations.begin(); it != loggedOperations.end(); it++)
        for(int k=0; k< it->second.size(); k++)
            delete it->second[k];
    loggedOperations.clear();
}

/**********************************************************************************
* Called by algorithms running from different threads.
* Emits <sendAppend> signal
***********************************************************************************/
void PLog::emitSendAppend(void* op)
{
    emit sendAppend(op);
}

/**********************************************************************************
* <sendAppend> event handler
***********************************************************************************/
void PLog::appendOperationVoid(void* op)
{
    appendOperation((itm::Operation*)(op), false);
}

/**********************************************************************************
* <enableIoCoreOperationsCheckBox> event handler
***********************************************************************************/
void PLog::enableIoCoreOperationsCheckBoxChanged(int s)
{
    if(autoUpdateCheckBox->isChecked() && s == Qt::Checked)
    {
        if(QMessageBox::information(this, "Warning", "Auto-update is enabled. Enabling I/O core logs will heavily deteriorate the performance.\n\n Proceed anyway?", QMessageBox::Cancel | QMessageBox::Yes, QMessageBox::Yes) == QMessageBox::Cancel)
            enableIoCoreOperationsCheckBox->setChecked(false);
    }

    enableIoCoreOperations = enableIoCoreOperationsCheckBox->isChecked();
}

/**********************************************************************************
* <autoUpdateCheckBox> event handler
***********************************************************************************/
void PLog::autoUpdateCheckBoxChanged(int s)
{
    if(enableIoCoreOperationsCheckBox->isChecked() && s == Qt::Checked)
    {
        if(QMessageBox::information(this, "Warning", "I/O core logs are enabled. Enabling auto-update will heavily deteriorate the performance.\n\n Proceed anyway?", QMessageBox::Cancel | QMessageBox::Yes, QMessageBox::Yes) == QMessageBox::Cancel)
            autoUpdateCheckBox->setChecked(false);
    }

    updatePushButton->setEnabled(!autoUpdateCheckBox->isChecked());
    logPanel->setEnabled(autoUpdateCheckBox->isChecked());
}

/**********************************************************************************
* <updatePushButton> event handler
***********************************************************************************/
void PLog::updatePushButtonClicked()
{
    update();
}
