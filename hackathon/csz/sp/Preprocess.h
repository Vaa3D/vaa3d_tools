#ifndef PREPROCESS_H
#define PREPROCESS_H
//#include "data_io.h"
#include "v3d_interface.h"
//#include "v3d_plugin_loader.h"
//#include "mainwindow.h"
//#include "dataflow.h"
#include <QString>
#include <vector>
using std::vector;

class Preprocess{
public:
    Preprocess(V3DPluginCallback2 *callback){
        mcallback=callback;
//        data_paths=getFileNames(root);
//        Proot=transformproot(root);
//        QString count;
//        count=QString::fromStdString(std::to_string(data_paths.size()));
//        v3d_msg(count);
        //Predatamem=new DataFlow();
    }
//    QString transformproot(QString root);
    void gaussfilter(DataFlow *Predatamem,vector<char *> inputarg,int i,char * funcname);
    void imPreprocess(DataFlow *Predatamem,vector<char *> inputarg,int i,char * funcname);
    void gsdt(DataFlow *Predatamem,vector<char *> inputarg,int i,char * funcname);
    void cropTerafly(DataFlow *Predatamem,char * inputfile,vector<char *> inputarg);
    void histogramEqualization(DataFlow *Predatamem,vector<char *> inputarg,int i,char * funcname);
    void standardize(DataFlow *Predatamem,vector<char *> inputarg,int i,char * funcname);
    void datatypeconvert(DataFlow *Predatamem,vector<char *> inputarg,int i,char * funcname);
//    QStringList data_paths;
//    QString Proot;

    //DataFlow *Predatamem;
private:
    V3DPluginCallback2 *mcallback;

};

#endif // PREPROCESS_H
