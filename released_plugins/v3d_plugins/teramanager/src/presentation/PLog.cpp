#include "PLog.h"

using namespace teramanager;

PLog* PLog::uniqueInstance = 0;

PLog::PLog(QWidget *parent) : QDialog(parent)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> PLog::PLog()\n");
    #endif

    setWindowTitle("TeraFly's log");

    timeTotal = new QLineEdit();
    timeTotal->setReadOnly(true);
    timeTotal->setAlignment(Qt::AlignCenter);
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
    timeLayout->addWidget(new QLabel("Total"), 0, 0, 1, 1, Qt::AlignCenter);
    timeLayout->addWidget(new QLabel("IO"), 0, 1, 1, 1, Qt::AlignCenter);
    timeLayout->addWidget(new QLabel("GPU"), 0, 2, 1, 1, Qt::AlignCenter);
    timeLayout->addWidget(new QLabel("CPU"), 0, 3, 1, 1, Qt::AlignCenter);
    timeLayout->addWidget(timeTotal, 1, 0, 1, 1);
    timeLayout->addWidget(timeIO, 1, 1, 1, 1);
    timeLayout->addWidget(timeGPU, 1, 2, 1, 1);
    timeLayout->addWidget(timeCPU, 1, 3, 1, 1);

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
    float timeIOf = toFloat(timeIO->text());
    float timeGPUf = toFloat(timeGPU->text());
    float timeCPUf = toFloat(timeCPU->text());
    float timeTotalf = timeIOf + timeGPUf + timeCPUf;

    float timeIOperc = (timeIOf / timeTotalf) * 100.0f;
    float timeGPUperc = (timeGPUf / timeTotalf) * 100.0f;
    float timeCPUperc = (timeCPUf / timeTotalf) * 100.0f;

    timeTotal->setText(QString::number(timeTotalf, 'f', 3).append("s"));
    timeIO->setText(QString::number(timeIOf, 'f', 3).append("s (").append(QString::number(timeIOperc, 'f', 1)).append("\%)"));
    timeGPU->setText(QString::number(timeGPUf, 'f', 3).append("s (").append(QString::number(timeGPUperc, 'f', 1)).append("\%)"));
    timeCPU->setText(QString::number(timeCPUf, 'f', 3).append("s (").append(QString::number(timeCPUperc, 'f', 1)).append("\%)"));
}

void PLog::append(string text)
{
    log->append(QString("@").append(QTime::currentTime().toString()).append(": ").append(text.c_str()));
}

void PLog::appendIO(int milliseconds, string message)
{
    timeIO->setText(QString::number(toFloat(timeIO->text()) + milliseconds / 1000.0f, 'f', 3).append("s"));
    append(QString("IO  operation ( ").append(QString::number(milliseconds/1000.0f, 'f', 3)).append("s ): ").append(message.c_str()).toStdString());
    updatePercentages();
}

void PLog::appendGPU(int milliseconds, string message)
{
    timeGPU->setText(QString::number(toFloat(timeGPU->text()) + milliseconds / 1000.0f, 'f', 3).append("s"));
    append(QString("GPU operation ( ").append(QString::number(milliseconds/1000.0f, 'f', 3)).append("s ): ").append(message.c_str()).toStdString());
    updatePercentages();
}
void PLog::appendCPU(int milliseconds, string message)
{
    timeCPU->setText(QString::number(toFloat(timeCPU->text()) + milliseconds / 1000.0f, 'f', 3).append("s"));
    append(QString("CPU operation ( ").append(QString::number(milliseconds/1000.0f, 'f', 3)).append("s ): ").append(message.c_str()).toStdString());
    updatePercentages();
}

void PLog::reset()
{
    log->setText("");
    append(QString("Log started on ").append(QDateTime::currentDateTime().date().toString()).toStdString());
    timeTotal->setText("0.000s");
    timeIO->setText("0.000s (0.0\%)");
    timeGPU->setText("0.000s (0.0\%)");
    timeCPU->setText("0.000s (0.0\%)");
}
