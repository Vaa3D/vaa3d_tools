#include "winProcID.h"
#include <QDebug>

WinProcID::WinProcID(QObject *parent) :
    QObject(parent)
{
}

void WinProcID::PrintProcessNameAndID(DWORD processID){
    TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

    // Get a handle to the process.

    HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                                   PROCESS_VM_READ,
                                   FALSE, processID );

    // Get the process name.

    if (NULL != hProcess )
    {
        HMODULE hMod;
        DWORD cbNeeded;

        if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod),
             &cbNeeded) )
        {
            GetModuleBaseName( hProcess, hMod, szProcessName,
                               sizeof(szProcessName)/sizeof(TCHAR) );
        }
    }

    // Print the process name and identifier.

    qDebug()<< szProcessName << processID ;

    // Release the handle to the process.

    CloseHandle( hProcess );
}

int WinProcID::getProcesses(){
        // Get the list of process identifiers.

        DWORD aProcesses[1024], cbNeeded, cProcesses;
        unsigned int i;

        if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
        {
            return 1;
        }


        // Calculate how many process identifiers were returned.

        cProcesses = cbNeeded / sizeof(DWORD);

        // Print the name and process identifier for each process.

        for ( i = 0; i < cProcesses; i++ )
        {
            if( aProcesses[i] != 0 )
            {
                PrintProcessNameAndID( aProcesses[i] );
            }
        }

        return 0;
    }
