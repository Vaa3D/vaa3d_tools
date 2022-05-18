#ifndef SUPERPLUGIN_UI_H
#define SUPERPLUGIN_UI_H
#include <QWidget>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QString>
#include <QFileDialog>
#include <v3d_interface.h>
#include <unordered_map>
#include "basic_surf_objs.h"
#include "Preprocess.h"
#include "Computation.h"
#include "Postprocess.h"
#include "data_io.h"
//#include "dataflow.h"

using std::unordered_map;


class SuperUI:public QWidget{
    Q_OBJECT
public:
    explicit SuperUI(V3DPluginCallback2 &callback, QWidget *parent){
        this->mcallback=&callback;
        this->mparent=parent;
        datapath=QFileDialog::getExistingDirectory(this,"data_path","d:");
        this->drawlayout();
        datamem=new DataFlow();
    }
    explicit SuperUI(V3DPluginCallback2 & callback,const V3DPluginArgList & input, V3DPluginArgList & output ){
        this->mcallback=&callback;
        datamem=new DataFlow();
        initmap();
        processcmd(input,output);

    }

    void drawlayout(){
        this->setFixedSize(480,360);
        this->setWindowTitle("SuperPlugin");
        preprocess=new QComboBox();
        preprocess->addItem(QWidget::tr("gaussfilter"));
        preprocess->addItem(QWidget::tr("imPreprocess"));
        hlayout=new QHBoxLayout();
        hlayout->addWidget(preprocess);
        confirm=new QPushButton(QWidget::tr("Yes"));
        vlayout=new QVBoxLayout();
        vlayout->addWidget(preprocess);
        vlayout->addWidget(confirm);
        this->setLayout(vlayout);

        connect(confirm,SIGNAL(clicked()),this,SLOT(click_yes()));
    }


    void processcmd(const V3DPluginArgList & input, V3DPluginArgList & output);
    void initmap();
    void assemblyline();
    QString finddll(char * funcname);
    void saveimgresult(DataFlow* data,int i);
    void saveswcresult(DataFlow* data,int i);
public slots:
    void click_yes();

private:
    V3DPluginCallback2 *mcallback;
    QWidget *mparent;

//    V3DPluginArgList Input;
//    V3DPluginArgList Output;
    char * inputfile;           // can be optimized
//    char * inputapo;
    char * outputfile;
    vector<vector<char *>> DataFlowArg;
    QString qinputfile;
    QStringList inputimglist;
    QStringList inputswclist;
    QString inputway;
    QString outresult;

    unordered_map<QString,QString> fnametodll;
    unordered_map<QString,QString> dlltomode;
    //unordered_map<QString,QString> dlltofunc;

    QHBoxLayout *hlayout;
    QVBoxLayout *vlayout;

    QString datapath;

    QComboBox *preprocess;
    QPushButton *confirm;

    DataFlow *datamem;

    Preprocess *preproc;
};

#endif // SUPERPLUGIN_UI_H
