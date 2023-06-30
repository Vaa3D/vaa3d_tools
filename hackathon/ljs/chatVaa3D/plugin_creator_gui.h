#ifndef __PLUGIN_CREATOR_GUI_H__
#define __PLUGIN_CREATOR_GUI_H__

#include <string>
#include <QtGui>
#include "v3d_message.h"
#include "plugin_creator_func.h"
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGridLayout>
#include <QTextEdit>
using namespace std;

class GuidingDialog : public QDialog
{
	Q_OBJECT

public:
	GuidingDialog(QWidget * parent) : QDialog(parent)
	{
		label_plugin_name = new QLabel(tr("Plugin Name :"));
		editor_plugin_name = new QLineEdit(tr("test"));

		label_plugin_class = new QLabel(tr("Plugin Class Name :"));
		editor_plugin_class = new QLineEdit(tr("TestPlugin"));
        text1 = new QTextEdit();
        text2 = new QTextEdit();


        run = new QPushButton(tr("Get Result"));
//        get = new QPushButton(tr("Get Plugin"));
//        createpipe = new QPushButton(tr("Get Result"));

		gridLayout = new QGridLayout();
       // gridLayout->addWidget(label_plugin_name,         0, 0, 1, 1);
        gridLayout->addWidget(text1,        0, 2, 5, 13);
        gridLayout->addWidget(text2,        6, 2, 5, 13);
//        gridLayout->addWidget(label_plugin_class,        5, 0, 5, 1);
//        gridLayout->addWidget(editor_plugin_class,       5, 1, 10, 9);

//        gridLayout->addWidget(get, 15, 2, 1, 5, Qt::AlignLeft);
//        gridLayout->addWidget(createpipe, 15, 7, 1, 5, Qt::AlignLeft);

        gridLayout->addWidget(run, 15, 12, 1,5, Qt::AlignLeft);



//        connect(get, SIGNAL(clicked()), this, SLOT(chatVaa3D()));

//        connect(createpipe, SIGNAL(clicked()), this, SLOT(create_pipe()));

        connect(run, SIGNAL(clicked()), this, SLOT(chatVaa3D()));

		this->setLayout(gridLayout);
        this->setWindowTitle("ChatVaa3D-x");

	}

	~GuidingDialog(){}

public slots:
	void accept()
	{
		return QDialog::accept();
	}

	void reject()
	{
		return QDialog::reject();
	}

    void chatVaa3D()
	{
        text1->clear();
        QStringList args;
        QString input = text2->document()->toPlainText();
        args << input;

        QProcess process;
        QString s(input[0]);
//        if(){
//            text1->append("当前加载BERT模型");
//            process.start("/Users/jazz/anaconda3/envs/tensorflow115/bin/python3.7", QStringList() << "/Users/jazz/Desktop/classifier_multi_label/predict.py" << args);
//        }
//        else{
            text1->append("当前加载LlamaIndex问答模型");
            process.start("/Users/jazz/try-llama-index-master/bin/python3.9", QStringList() << "/Users/jazz/Desktop/try-llama-index-master/query_index.py" << args);
        //}
        process.waitForFinished();

        QByteArray errorData = process.readAllStandardError(); // 获取标准错误输出数据
        QString errorString = QString::fromLocal8Bit(errorData); // 将字节数组转换为QString

        if (!errorString.isEmpty()) {
            qDebug() << "Python错误信息：" << errorString;
        } else {
            qDebug() << "Python脚本执行成功！";
        }
        QString output = process.readAllStandardOutput();
        qDebug()<<output;
        result = output;
        text1->append(output);
	}
    void create_pipe() {
        text1->clear();
        QString temp = "Robot: /Users/jazz/Desktop/vaa3dapp/Vaa3D-x.app/Contents/MacOS/Vaa3D-x -x super_plugin_pipe -f sp -i $input_path -o $output_path -p gf yes 7 7 7 1 3 app2 yes NULL 0 AUTO";
        text1->append(temp);
    }
    void Run()
    {
        QString str = "/Users/jazz/Desktop/vaa3dapp/Vaa3D-x.app/Contents/MacOS/Vaa3D-x -x /Users/jazz/Desktop/vaa3dapp/plugins/SuperPlugin/libSuperPlugin_Jazz.dylib  -f func1";
        char* ch;
        QByteArray ba = str.toLatin1(); // must
        ch = ba.data();
        //qDebug()<<str;
        system(ch);
    }

	void update()
	{
        if(editor_save_folder->text().isEmpty() || editor_vaa3d_path->text().isEmpty())
        {
            v3d_msg("Please select the correct path!");
            return QDialog::reject();
        }

        plugin_name =  editor_plugin_name->text().toStdString();
		plugin_class =  editor_plugin_class->text().toStdString();


	}

public:
	string plugin_name;
	string plugin_class;
	string win_title;
	string plugin_desp;
	string plugin_date;
	string plugin_author;
	string vaa3d_path;
	string menulst;
	string funclst;
	string save_folder;
    QString result;
	QLabel * label_plugin_name;
	QLineEdit * editor_plugin_name;
    QTextEdit *text1;
    QTextEdit *text2;
	QLabel * label_plugin_class;
	QLineEdit * editor_plugin_class;

	QLabel * label_win_title;
	QLineEdit * editor_win_title;

	QLabel * label_plugin_description;
	QLineEdit * editor_plugin_description;

	QLabel * label_plugin_date;
	QLineEdit * editor_plugin_date;

	QLabel * label_plugin_author;
	QLineEdit * editor_plugin_author;

	QLabel * label_vaa3d_path;
	QLineEdit * editor_vaa3d_path;
	QPushButton * button_vaa3d_path;

	QLabel * label_menu_list;
	QLineEdit * editor_menu_list;

	QLabel * label_func_list;
	QLineEdit * editor_func_list;

	QLabel * label_save_folder;
	QLineEdit * editor_save_folder;
	QPushButton * button_save_folder;

    QPushButton * run;
    QPushButton * get;
    QPushButton * createpipe;
	QGridLayout * gridLayout;

};

#endif
