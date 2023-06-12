#ifndef SPLOG_H
#define SPLOG_H
#include <QMutex>
#include <QString>
#include "GlobalConfig.h"

class SpLog{
public:
    SpLog(const SpLog&) = delete;
    SpLog& operator=(const SpLog&) = delete;

    static void debug(QString txt) {
        if (!_instance) {
            instancelock.lock();
            if (!_instance) {
                _instance = new SpLog();
            }
            instancelock.unlock();
        }
        _instance->WritetoText(txt);
    }

    static SpLog* instance() {
        if (!_instance) {
            instancelock.lock();
            if (!_instance) {
                _instance = new SpLog();
            }
            instancelock.unlock();
        }
        return _instance;
    }
    ~SpLog();


    void DebugToFile(QString txt,QString path);

private:
    static QMutex mainlock;
    static QMutex instancelock;
    static QMutex templock;
    static SpLog* _instance;

    SpLog(){};

    void WritetoText(QString text);
};

#endif // SPLOG_H
