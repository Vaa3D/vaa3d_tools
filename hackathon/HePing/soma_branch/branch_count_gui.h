#ifndef BRANCH_COUNT_GUI_H
#define BRANCH_COUNT_GUI_H
#include<string>
#include<QtGui>
#include"v3d_message.h"
using namespace std;
class CountDialog : public QDialog{
    Q_OBJECT
public:
    string swc_folder;
    string save_folder;

    QLabel * label_swc_folder;
    QLineEdit * editor_swc_folder;
    QPushButton * button_swc_folder;

    QLabel * label_save_folder;
    QLineEdit * editor_save_folder;
    QPushButton * button_save_folder;

    QPushButton * ok;
    QPushButton * cancel;

    QGridLayout * gridLayout;

public:
    CountDialog(QWidget * parent){
        label_swc_folder=new QLabel(tr("SWC Folder:"));
        editor_swc_folder=new QLineEdit(tr(""));
        button_swc_folder=new QPushButton(tr("..."));

        label_save_folder=new QLabel(tr("save Folder:"));
        editor_save_folder=new QLineEdit(tr(""));
        button_save_folder=new QPushButton(tr("..."));
        button_save_folder->setBackgroundRole(QPalette::Highlight);//设定为默认保存路径

        ok = new QPushButton(tr("ok"));
        cancel = new QPushButton(tr("cancel"));

        gridLayout = new QGridLayout();

        gridLayout->addWidget(label_swc_folder,   0, 0, 1, 1);
        gridLayout->addWidget(editor_swc_folder,  0, 1, 1, 8);
        gridLayout->addWidget(button_swc_folder,  0, 9, 1, 1);

        gridLayout->addWidget(label_save_folder,   1, 0, 1, 1);
        gridLayout->addWidget(editor_save_folder,  1, 1, 1, 8);
        gridLayout->addWidget(button_save_folder,  1, 9, 1, 1);

        gridLayout->addWidget(ok, 10, 0, 1, 5, Qt::AlignRight);
        gridLayout->addWidget(cancel, 10, 5, 1, 5, Qt::AlignRight);

        connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
        connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
        connect(button_swc_folder,SIGNAL(clicked()),this,SLOT(setFolderPath()));
        connect(button_swc_folder,SIGNAL(clicked()),this,SLOT(setFolderPath()));
        connect(ok, SIGNAL(clicked()), this, SLOT(update()));
        this->setLayout(gridLayout);
        this->setWindowTitle("branch count Dialog");

    }
    ~CountDialog(){}

public slots:
    void accept(){
        return QDialog::accept();
    }

    void reject(){

        return QDialog::reject();
    }

    void update(){
        if(editor_swc_folder->text().isEmpty()||editor_save_folder->text().isEmpty()){
            v3d_msg("Please select the correct path!");
            return QDialog::reject();
        }
        swc_folder=editor_swc_folder->text().toStdString();
        save_folder=editor_save_folder->text().toStdString();

    }

    void setFolderPath(){
        QPushButton * button = (QPushButton*) sender();
        QLineEdit * editor;
        QString title;

        if(button==button_swc_folder){
            editor=editor_swc_folder;
            title="open swc file";
            QString dir = QFileDialog::getOpenFileName(this,title,
                                                            "~/"
                                                );
            if(dir.isEmpty()){

            }
            editor->setText(dir);
        }
        if(button==button_save_folder){
            editor=editor_save_folder;
            title="save marker file";
            QString current_path = editor->text();
            QString dir=QFileDialog::getExistingDirectory(this,title,
                                                          "~/",
                                                          QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks);
          if(dir.isEmpty()){
              editor->setText(current_path);
          }
          else{
              editor->setText(dir);
          }
        }
    }
};

#endif // BRANCH_COUNT_GUI_H
