#include <set>
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

    log = new QTextEdit();
    log->setReadOnly(true);
    log->setMinimumHeight(100);
    log->setFont(font);
    log->setWordWrapMode(QTextOption::NoWrap);


    closeButton = new QPushButton("Close");
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
    resetButton = new QPushButton("Reset");
    connect(resetButton, SIGNAL(clicked()), this, SLOT(reset()));

    timeComponentsPanel = new QGroupBox("System performance (components time)");
    QHBoxLayout* timeComponentsPanelLayout = new QHBoxLayout();
    timeComponentsPanelLayout->addWidget(timeComponents);
    timeComponentsPanel->setMaximumHeight(160);
    timeComponentsPanel->setLayout(timeComponentsPanelLayout);
    #ifndef _USE_NATIVE_FONTS
    timeComponentsPanel->setStyle(new QWindowsStyle());
    #endif

    timeOperationsPanel = new QGroupBox("TeraFly's performance (average operation time)");
    QHBoxLayout* timeOperationsPanelLayout = new QHBoxLayout();
    timeOperationsPanelLayout->addWidget(timeOperations);
    timeOperationsPanel->setLayout(timeOperationsPanelLayout);
    #ifndef _USE_NATIVE_FONTS
    timeOperationsPanel->setStyle(new QWindowsStyle());
    #endif

    logPanel = new QGroupBox("Log");
    QHBoxLayout* logPanelLayout = new QHBoxLayout();
    logPanelLayout->addWidget(log);
    logPanel->setLayout(logPanelLayout);
    #ifndef _USE_NATIVE_FONTS
    logPanel->setStyle(new QWindowsStyle());
    #endif

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(resetButton);
    buttonsLayout->addWidget(closeButton);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(timeComponentsPanel);
    layout->addWidget(timeOperationsPanel);
    layout->addWidget(logPanel);
    layout->addLayout(buttonsLayout);
    setLayout(layout);

    this->setMinimumWidth(500);

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
                opText += it->first + "(" + compName + "): " + QString::number((sum/gids.size())/1000.0, 'f', 3).toStdString() + "s " + QChar(0x00B1).toAscii() + " " + QString::number( std::sqrt( sumsq/gids.size() - (sum/gids.size())*(sum/gids.size()) )/1000.0, 'f', 3).toStdString() + " (" + QString::number(gids.size()).toStdString() + " measures)"+ "\n";
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

void PLog::appendOperation(itm::Operation *op)
{
    // add operation to its group vector
    loggedOperations[op->name()].push_back(op);

    // add operation to log
    this->append( std::string("[") + QString::number(op->groupID).toStdString() + "]" + op->name() + "(" + op->compName() + ")( " + QString::number(op->milliseconds/1000.0f, 'f', 3).toStdString() + "s ): " + op->message);

    // update time
    if(op->comp == itm::IO)
        timeIO += op->milliseconds / 1000.0f;
    else if(op->comp == itm::GPU)
        timeGPU += op->milliseconds / 1000.0f;
    else if(op->comp == itm::CPU)
        timeCPU += op->milliseconds / 1000.0f;
    else if(op->comp == itm::ALL_COMPS)
        timeActual += op->milliseconds / 1000.0f;

    // update GUI
    this->update();
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
