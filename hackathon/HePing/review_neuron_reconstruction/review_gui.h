#ifndef REVIEW_GUI_H
#define REVIEW_GUI_H

#include<string>
#include<QtGui>
#include "v3d_message.h"
#include "review.h"

using namespace std;

class EmployDialog : public QDialog{
    Q_OBJECT

public:
    string swc_file;
    string brain_file;
    string save_folder;

    QLabel * label_swc_file;
    QLineEdit * editor_swc_file;
    QPushButton * button_swc_file;

    QLabel * label_brain_file;
    QLineEdit * editor_brain_file;
    QPushButton * button_brain_file;

    QLabel * label_save_folder;
    QLineEdit * editor_save_folder;
    QPushButton * button_save_folder;

    QPushButton * ok;
    QPushButton * cancel;

    QGridLayout * gridLayout;


public:
    EmployDialog(QWidget * parent) : QDialog(parent)
    {
        label_swc_file = new QLabel(tr("SWC File :"));
        editor_swc_file = new QLineEdit(tr(""));
        button_swc_file = new QPushButton(tr("..."));

        label_brain_file = new QLabel(tr("Brain location:"));
        editor_brain_file = new QLineEdit(tr(""));
        button_brain_file = new QPushButton(tr("..."));

        label_save_folder = new QLabel(tr("Save Folder:"));
        editor_save_folder = new QLineEdit(tr(""));
        button_save_folder = new QPushButton(tr("..."));
        button_save_folder->setBackgroundRole(QPalette::Highlight);//设定为默认保存路径

        ok = new QPushButton(tr("ok"));
        cancel = new QPushButton(tr("cancel"));

        gridLayout = new QGridLayout();
        gridLayout->addWidget(label_brain_file,   0, 0, 1, 1);
        gridLayout->addWidget(editor_brain_file,  0, 1, 1, 8);
        gridLayout->addWidget(button_brain_file,  0, 9, 1, 1);
        gridLayout->addWidget(label_swc_file,     1, 0, 1, 1);
        gridLayout->addWidget(editor_swc_file,    1, 1, 1, 8);
        gridLayout->addWidget(button_swc_file,    1, 9, 1, 1);
        gridLayout->addWidget(label_save_folder,  2, 0, 1, 1);
        gridLayout->addWidget(editor_save_folder, 2, 1, 1, 8);
        gridLayout->addWidget(button_save_folder, 2, 9, 1, 8);
        gridLayout->addWidget(ok, 10, 0, 1, 5, Qt::AlignRight);
        gridLayout->addWidget(cancel, 10, 5, 1, 5, Qt::AlignRight);

        connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
        connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
        connect(button_save_folder, SIGNAL(clicked()), this, SLOT(setFolderPath()) );
        connect(button_brain_file, SIGNAL(clicked()), this, SLOT(setFolderPath()));
        connect(button_swc_file, SIGNAL(clicked()), this, SLOT(setFolderPath()));
        connect(ok, SIGNAL(clicked()), this, SLOT(update()));

        this->setLayout(gridLayout);
        this->setWindowTitle("Review Neuron Dialog");

    }

    ~EmployDialog(){}



public slots:
        void accept(){
            return QDialog::accept();
        }

        void reject(){

            return QDialog::reject();
        }

        void update(){
            if(editor_brain_file->text().isEmpty()||editor_save_folder->text().isEmpty()||editor_swc_file->text().isEmpty()){
                v3d_msg("Please select the correct path!");
                return QDialog::reject();
            }
            //获取所选文件的路径
            swc_file = editor_swc_file->text().toStdString();
            brain_file = editor_brain_file->text().toStdString();
            save_folder = editor_save_folder->text().toStdString();
        }

        void setFolderPath(){
            QPushButton * button = (QPushButton*) sender();
            QLineEdit * editor;
            QString title;
            QString current_swc_file;
            bool flag = false;
            if(button == button_brain_file){
                editor = editor_brain_file;
                title = "open brain file";

            }
            else if(button == button_swc_file){
                editor = editor_swc_file;
                title = "open swc file";
                current_swc_file = editor->text();
                QString dir = QFileDialog::getOpenFileName(this,title,
                                                                "~/"
                                                    );
                if(dir.isEmpty()){

                }
                editor->setText(dir);
                flag = true;
            }
            else if(button == button_save_folder){
                editor = editor_save_folder;
                title = "save location";
            }
            if(flag == true){
               flag = false;
            }
            else{
                QString current_path = editor->text();
                QString dir = QFileDialog::getExistingDirectory(this,title,
                                                                "~/",
                                                                QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks);
                if(dir.isEmpty()){
                    editor->setText(current_path);
                }
                else
                    editor->setText(dir);

            }

        }
};



#endif // REVIEW_GUI_H
