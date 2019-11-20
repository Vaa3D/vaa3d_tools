#ifndef SHOW_SEGMENT_GUI_H
#define SHOW_SEGMENT_GUI_H

#include<string>
#include<QtGui>
#include "v3d_message.h"
#include<v3d_interface.h>
#include "review.h"
using namespace std;

class ShowDialog : public QDialog{
    Q_OBJECT

public:
    V3DLONG current_index;
    QString swc_file;
    QString save_folders;
    //string brain_file;
    //V3DPluginCallback2 callback;
    QLabel * label_swc_file;
    QLineEdit * editor_swc_file;
    QPushButton * button_swc_file;

    QLabel * label_brain_file;
    QLineEdit * editor_brain_file;
    QPushButton * button_brain_file;

    QLabel * label_save_folder;
    QLineEdit * editor_save_folder;
    QPushButton * button_save_folder;
    QLabel * label_index;
    QLineEdit * editor_index;

    QLabel * label_forward;
    QPushButton * button_forward;

    QLabel * label_backward;
    QPushButton *button_backward;

    QPushButton *ok;
    QPushButton *close;

    QGridLayout * gridLayout;

public:
    ShowDialog(QWidget * parent): QDialog(parent){
        //this->callback=callback;
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

        label_index = new QLabel(tr("current segment index:"));
        editor_index = new QLineEdit(tr("1"));

        label_forward = new QLabel(tr("forward segment"));
        button_forward = new QPushButton(tr(" -> "));

        label_backward = new QLabel(tr("backward segment"));
        button_backward = new QPushButton(tr(" <- "));

        ok = new QPushButton(tr("ok"));
        close = new QPushButton(tr("close"));

        gridLayout = new QGridLayout();
        gridLayout->addWidget(label_swc_file,     0, 0, 1, 1);
        gridLayout->addWidget(editor_swc_file,    0, 1, 1, 8);
        gridLayout->addWidget(button_swc_file,    0, 9, 1, 1);
        gridLayout->addWidget(label_save_folder,   1, 0, 1, 1);
        gridLayout->addWidget(editor_save_folder,  1, 1, 1, 8);
        gridLayout->addWidget(button_save_folder,  1, 9, 1, 1);
        gridLayout->addWidget(label_index,    2, 0, 1, 1);
        gridLayout->addWidget(editor_index,   2, 1, 1, 8);
        gridLayout->addWidget(label_forward,  3, 0, 1, 1);
        gridLayout->addWidget(button_forward, 3, 9, 1, 2);
        gridLayout->addWidget(label_backward, 4, 0, 1, 1);
        gridLayout->addWidget(button_backward,4, 9, 1, 2);
        gridLayout->addWidget(ok, 10, 0, 1, 5, Qt::AlignRight);
        gridLayout->addWidget(close, 10, 5, 1, 5, Qt::AlignRight);

        connect(button_save_folder, SIGNAL(clicked()), this, SLOT(setFolderPath()));
        connect(button_swc_file, SIGNAL(clicked()), this, SLOT(setFolderPath()));
        connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
        connect(close, SIGNAL(clicked()), this, SLOT(reject()));
        connect(button_forward, SIGNAL(clicked()), this, SLOT(move_segment()));
        connect(button_backward, SIGNAL(clicked()), this, SLOT(move_segment()));
        connect(ok, SIGNAL(clicked()), this, SLOT(update()));

        this->setLayout(gridLayout);
        this->setWindowTitle("show segment dialog");
    }

    ~ShowDialog()
    {
        //delete this;
    }

public slots:

    void accept(){
        return QDialog::accept();
    }

    void reject(){

        return QDialog::reject();
    }

    void setFolderPath(){
        QPushButton * button = (QPushButton*) sender();
        QLineEdit * editor;
        QString title;

        if(button == button_save_folder){
            editor = editor_save_folder;
            title = "open brain file";
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
        else if(button == button_swc_file){
            editor = editor_swc_file;
            title = "open swc file";
            QString current_file = editor->text();
            QString file = QFileDialog::getOpenFileName(this,title,
                                                            "~/"
                                                );
            if(file.isEmpty()){
                editor->setText(current_file);
            }
            else
                editor->setText(file);

        }





    }
    void move_segment(){
        QPushButton * button = (QPushButton*) sender();

        //先获取之前的index
        if(editor_index->text().isEmpty()){
            current_index=1;
        }
        else{
            current_index=stoll(editor_index->text().toStdString().c_str());//将字符串转化为v3dlong

        }
        if(button == button_forward){
           current_index+=1;
        }
        else if(button == button_backward){
           current_index-=1;
           if(current_index<=0){
               v3d_msg("It is the first segment.");
           }
        }
        editor_index->setText(QString::number(current_index));//更新下标
        //update();
        //show_segment(callback,current_index,swc_file);
    }


    void update(){
        if(editor_index->text().isEmpty()||editor_swc_file->text().isEmpty()){
            v3d_msg("please input the right segment index and select the right swc file!");
            return QDialog::reject();
        }
        current_index = stoll(editor_index->text().toStdString().c_str());//获取框中输入的index
        swc_file = editor_swc_file->text().toStdString().c_str();
        save_folders=editor_save_folder->text().toStdString().c_str();
    }



};


#endif // SHOW_SEGMENT_GUI_H

