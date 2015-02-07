#include <set>
#include <cmath>
#include <fstream>
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

    appendCheckBox = new QCheckBox("Append and clear");
    appendOpComboBox = new QComboBox();
    appendOpComboBox->setEnabled(false);
    appendOpComboBox->addItem("--operation--");
    appendOpComboBox->addItem(itm::ImportOperation().name().c_str());
    appendOpComboBox->addItem(itm::NewViewerOperation().name().c_str());
    appendOpComboBox->addItem(itm::RestoreViewerOperation().name().c_str());
    appendOpComboBox->addItem(itm::AnnotationOperation().name().c_str());
    appendOpComboBox->addItem(itm::ZoominRoiOperation().name().c_str());
    appendOpComboBox->addItem(itm::ConverterLoadBlockOperation().name().c_str());
    appendOpComboBox->addItem(itm::ConverterWriteBlockOperation().name().c_str());
    appendOpComboBox->addItem(itm::TiffLoadMetadata().name().c_str());
    appendOpComboBox->addItem(itm::TiffLoadData().name().c_str());
    appendOpComboBox->addItem(itm::TiffInitData().name().c_str());
    appendOpComboBox->addItem(itm::TiffAppendData().name().c_str());
    appendCompComboBox = new QComboBox();
    appendCompComboBox->setEnabled(false);
    appendCompComboBox->addItem("--component--");
    appendCompComboBox->addItem(itm::comp2str(ALL_COMPS).c_str());
    appendCompComboBox->addItem(itm::comp2str(GPU).c_str());
    appendCompComboBox->addItem(itm::comp2str(CPU).c_str());
    appendCompComboBox->addItem(itm::comp2str(IO).c_str());
    appendToFileLineEdit = new QLineEdit();
    appendToFileLineEdit->setText("[to filepath]");
    appendToFileLineEdit->setEnabled(false);
    appendEverySecondsSpinBox = new QSpinBox();
    appendEverySecondsSpinBox->setEnabled(false);
    appendEverySecondsSpinBox->setPrefix("every ");
    appendEverySecondsSpinBox->setSuffix(" s");
    appendEverySecondsSpinBox->setMinimum(1);
    appendEverySecondsSpinBox->setMaximum(100);
    appendEverySecondsSpinBox->setValue(1);


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
    QVBoxLayout* logPanelLayout = new QVBoxLayout();
    logPanelLayout->addWidget(log);
    QHBoxLayout* appendOpLayout = new QHBoxLayout();
    appendOpLayout->addWidget(appendCheckBox);
    appendOpLayout->addWidget(appendOpComboBox,1);
    appendOpLayout->addWidget(appendCompComboBox,1);
    QHBoxLayout* appendOpLayout2 = new QHBoxLayout();
    appendOpLayout2->addWidget(appendToFileLineEdit, 1);
    appendOpLayout2->addWidget(appendEverySecondsSpinBox, 1);
    logPanelLayout->addLayout(appendOpLayout);
    logPanelLayout->addLayout(appendOpLayout2);
    logPanel->setLayout(logPanelLayout);
    #ifdef Q_OS_LINUX
    logPanel->setStyle(new QWindowsStyle());
    #endif
    log->setEnabled(false);

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
    connect(appendCheckBox, SIGNAL(stateChanged(int)), this, SLOT(appendCheckBoxChanged(int)));


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

    // also append operation to file, if required
    if(appendCheckBox->isChecked())
        appendOperationToFile(op);

}

void PLog::appendOperationToFile(itm::Operation* op)
{
    // static variables
    static int writings = 0;
    static bool first_time = true;
    static QElapsedTimer timer;
    static std::vector < int > op_times;

    // start a global timer the first time this function is entered
    if(first_time)
    {
        timer.start();
        first_time = false;
    }

    // append to file every 'x' seconds, where x is inputted by the user
    if(timer.elapsed()/1000.0 >= appendEverySecondsSpinBox->value() && op_times.size())
    {
        // append to file
        std::ofstream f(appendToFileLineEdit->text().toStdString().c_str(), std::fstream::app);
        if(f.is_open())
        {
            for(int k=0; k < op_times.size(); k++)
            {
                f.precision(5);
                f << op_times[k]/1000.0f << "\n";
            }

            // notify the successfull writing to the user
            appendEverySecondsSpinBox->setSuffix(QString(" seconds (x") + QString::number(++writings) + ")");
        }
        else
            itm::warning(itm::strprintf("Cannot open file at \"%s\" in append mode", appendToFileLineEdit->text().toStdString().c_str()).c_str(), __itm__current__function__);

        // clear
        op_times.clear();
        for(std::map< std::string, std::vector<itm::Operation*> >::iterator it = loggedOperations.begin(); it != loggedOperations.end(); it++)
        {
            for(int k=0; k< it->second.size(); k++)
                delete it->second[k];
            it->second.clear();
        }

        // restart the timer
        timer.restart();
    }
    else if(op->name().compare(appendOpComboBox->currentText().toStdString()) == 0 &&
            op->comp == itm::str2comp(appendCompComboBox->currentText().toStdString()))
        op_times.push_back(op->milliseconds);
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
    log->setEnabled(autoUpdateCheckBox->isChecked());
}

/**********************************************************************************
* <appendCheckBox> event handler
***********************************************************************************/
void PLog::appendCheckBoxChanged(int s)
{
    appendToFileLineEdit->setEnabled(s == Qt::Checked);
    appendOpComboBox->setEnabled(s == Qt::Checked);
    appendCompComboBox->setEnabled(s == Qt::Checked);
    appendEverySecondsSpinBox->setEnabled(s == Qt::Checked);
}

/**********************************************************************************
* <updatePushButton> event handler
***********************************************************************************/
void PLog::updatePushButtonClicked()
{
    update();
}
