#ifndef COMMAND_H
#define COMMAND_H
#include <v3d_interface.h>
#include <vector>

using std::vector;

class Command{
public:
    Command(V3DPluginCallback2 & callback){
        mcallback=&callback;
        qDebug()<<"start";
        iput=0;
        otput=0;
        dataline=0;
    }
    ~Command(){
        qDebug()<<"end";
    }

    void readCmdTxt(QString CmdTxt);
    void convertcmd(QStringList txtline);
    void acceptcmd(const V3DPluginArgList & input, V3DPluginArgList & output);

    vector<QString> otresult;
    vector<DataFlow *> dtf;
    vector<vector<vector<char *>>> DataFlowCmd;
private:
    V3DPluginCallback2 *mcallback;
    QString CmdTxt;

    char * iput;
    char * otput;
    char * dataline;
    vector<char *> paralist;
    QString otpath;


};

#endif // COMMAND_H
