#ifndef WINPROCID_H
#define WINPROCID_H

#include <QObject>
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>

// To ensure correct resolution of symbols, add Psapi.lib to TARGETLIBS
// and compile with -DPSAPI_VERSION=1


class WinProcID : public QObject
{
    Q_OBJECT
public:
    explicit WinProcID(QObject *parent = 0);

signals:

public slots:

void PrintProcessNameAndID( DWORD processID );
int getProcesses( void );

};

#endif // WINPROCID_H
