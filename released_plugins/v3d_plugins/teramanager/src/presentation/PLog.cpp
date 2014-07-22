#include "PLog.h"

using namespace teramanager;

PLog* PLog::uniqueInstance = 0;

PLog::PLog(QWidget *parent) : QDialog(parent)
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    setWindowTitle("TeraFly's log");

    timeActual = new QLineEdit();
    timeActual->setReadOnly(true);
    timeActual->setAlignment(Qt::AlignCenter);
    perfGain = new QLineEdit();
    perfGain->setReadOnly(true);
    perfGain->setAlignment(Qt::AlignCenter);
    timeSum = new QLineEdit();
    timeSum->setReadOnly(true);
    timeSum->setAlignment(Qt::AlignCenter);
    timeIO = new QLineEdit();
    timeIO->setReadOnly(true);
    timeIO->setAlignment(Qt::AlignCenter);
    timeGPU = new QLineEdit();
    timeGPU->setReadOnly(true);
    timeGPU->setAlignment(Qt::AlignCenter);
    timeCPU = new QLineEdit();
    timeCPU->setReadOnly(true);
    timeCPU->setAlignment(Qt::AlignCenter);

    log = new QTextEdit();
    log->setReadOnly(true);
    log->setMinimumHeight(100);
    QFont font = QFont("Courier New", 10);
    log->setFont(font);
    log->setWordWrapMode(QTextOption::NoWrap);


    closeButton = new QPushButton("Close");
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
    resetButton = new QPushButton("Reset");
    connect(resetButton, SIGNAL(clicked()), this, SLOT(reset()));

    QGridLayout* timeLayout = new QGridLayout();
    timeLayout->setSpacing(5);
    timeLayout->addWidget(new QLabel("Actual time"), 0, 0, 1, 1, Qt::AlignCenter);
    timeLayout->addWidget(new QLabel("Performance gain"), 0, 1, 1, 1, Qt::AlignCenter);
    timeLayout->addWidget(new QLabel("Sum"), 0, 2, 1, 1, Qt::AlignCenter);
    timeLayout->addWidget(new QLabel("IO"), 0, 3, 1, 1, Qt::AlignCenter);
    timeLayout->addWidget(new QLabel("GPU"), 0, 4, 1, 1, Qt::AlignCenter);
    timeLayout->addWidget(new QLabel("CPU"), 0, 5, 1, 1, Qt::AlignCenter);
    timeLayout->addWidget(timeActual, 1, 0, 1, 1);
    timeLayout->addWidget(perfGain, 1, 1, 1, 1);
    timeLayout->addWidget(timeSum, 1, 2, 1, 1);
    timeLayout->addWidget(timeIO, 1, 3, 1, 1);
    timeLayout->addWidget(timeGPU, 1, 4, 1, 1);
    timeLayout->addWidget(timeCPU, 1, 5, 1, 1);

    QGridLayout* layout = new QGridLayout();
    layout->setMargin(20);
    layout->setSpacing(20);
    layout->addLayout(timeLayout, 0, 0, 1, 10);
    layout->addWidget(log, 1, 0, 1, 10);
    layout->addWidget(resetButton, 2, 8, 1, 1, Qt::AlignRight);
    layout->addWidget(closeButton, 2, 9, 1, 1, Qt::AlignRight);
    setLayout(layout);

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

//updates time percentages
void PLog::updatePercentages()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    float timeActualf = toFloat(timeActual->text());
    float timeIOf = toFloat(timeIO->text());
    float timeGPUf = toFloat(timeGPU->text());
    float timeCPUf = toFloat(timeCPU->text());
    float timeSumf = timeIOf + timeGPUf + timeCPUf;

    float timeIOperc = (timeIOf / timeSumf) * 100.0f;
    float timeGPUperc = (timeGPUf / timeSumf) * 100.0f;
    float timeCPUperc = (timeCPUf / timeSumf) * 100.0f;

    float perfGainf = ( (timeSumf-timeActualf)/timeSumf ) * 100.0f;
    if(perfGainf < 0.0f)
        perfGainf = 0.0f;

    timeSum->setText(QString::number(timeSumf, 'f', 3).append("s"));
    timeIO->setText(QString::number(timeIOf, 'f', 3).append("s (").append(QString::number(timeIOperc, 'f', 1)).append("\%)"));
    timeGPU->setText(QString::number(timeGPUf, 'f', 3).append("s (").append(QString::number(timeGPUperc, 'f', 1)).append("\%)"));
    timeCPU->setText(QString::number(timeCPUf, 'f', 3).append("s (").append(QString::number(timeCPUperc, 'f', 1)).append("\%)"));
    perfGain->setText(QString::number(perfGainf, 'f', 1).append("\%"));
}

void PLog::append(string text)
{
    log->append(QString("@").append(QTime::currentTime().toString()).append(": ").append(text.c_str()));
}

void PLog::appendIO(int milliseconds, string message, bool sum)
{
    append(QString("IO  operation ( ").append(QString::number(milliseconds/1000.0f, 'f', 3)).append("s ): ").append(message.c_str()).toStdString());

    if(sum)
    {
        timeIO->setText(QString::number(toFloat(timeIO->text()) + milliseconds / 1000.0f, 'f', 3).append("s"));
        updatePercentages();
    }
}

void PLog::appendGPU(int milliseconds, string message, bool sum)
{
    append(QString("GPU operation ( ").append(QString::number(milliseconds/1000.0f, 'f', 3)).append("s ): ").append(message.c_str()).toStdString());

    if(sum)
    {
        timeGPU->setText(QString::number(toFloat(timeGPU->text()) + milliseconds / 1000.0f, 'f', 3).append("s"));
        updatePercentages();
    }
}
void PLog::appendCPU(int milliseconds, string message, bool sum)
{
    append(QString("CPU operation ( ").append(QString::number(milliseconds/1000.0f, 'f', 3)).append("s ): ").append(message.c_str()).toStdString());

    if(sum)
    {
        timeCPU->setText(QString::number(toFloat(timeCPU->text()) + milliseconds / 1000.0f, 'f', 3).append("s"));
        updatePercentages();
    }
}
void PLog::appendActual(int milliseconds, string message, bool sum)
{
    append(QString("ACT operation ( ").append(QString::number(milliseconds/1000.0f, 'f', 3)).append("s ): ").append(message.c_str()).toStdString());

    if(sum)
    {
        timeActual->setText(QString::number(toFloat(timeActual->text()) + milliseconds / 1000.0f, 'f', 3).append("s"));
        updatePercentages();
    }
}


void PLog::reset()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    log->setText("");
    append(QString("Log started on ").append(QDateTime::currentDateTime().date().toString()).toStdString());
    timeActual->setText("0.000s");
    perfGain->setText("0.0\%");
    timeSum->setText("0.000s");
    timeIO->setText("0.000s (0.0\%)");
    timeGPU->setText("0.000s (0.0\%)");
    timeCPU->setText("0.000s (0.0\%)");
}
