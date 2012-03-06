#ifndef __COMMON_DIALOG_H__
#define __COMMON_DIALOG_H__
#include <QtGui>

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include "advanced_parser.h"

using namespace std;

static int split_string(const char * paras, char ** &argv)
{
    int argc = 0;
    int len = strlen(paras);
    int posb[200];
    char * myparas = new char[len];
    strcpy(myparas, paras);
    for(int i = 0; i < len; i++)
    {
        if(i==0 && myparas[i] != ' ' && myparas[i] != '\t')
        {
            posb[argc++]=i;
        }
        else if((myparas[i-1] == ' ' || myparas[i-1] == '\t') &&
                (myparas[i] != ' ' && myparas[i] != '\t'))
        {
            posb[argc++] = i;
        }
    }

    argv = new char*[argc];
    for(int i = 0; i < argc; i++)
    {
        argv[i] = myparas + posb[i];
    }

    for(int i = 0; i < len; i++)
    {
        if(myparas[i]==' ' || myparas[i]=='\t')myparas[i]='\0';
    }

    return argc;
}

class CommonDialog : public QDialog
{
	Q_OBJECT

public:
	CommonDialog(vector<string> _items, QWidget * parent = 0): QDialog(parent)
	{
		items = _items;

		int nitem = items.size();
		all_labels.resize(nitem);
		all_editors.resize(nitem);

		QGridLayout * gridLayout = new QGridLayout(parent);

		for(int i = 0; i < nitem; i++)
		{
			all_labels[i] = new QLabel(QObject::tr((char*)(items[i].c_str())), parent);
			all_editors[i] = new QLineEdit(parent);
			gridLayout->addWidget(all_labels[i], i, 0);
			gridLayout->addWidget(all_editors[i], i, 1);
		}
		QPushButton * ok = new QPushButton("  ok  ");
		QPushButton * cancel = new QPushButton("cancel");

		gridLayout->addWidget(cancel, nitem, 0, Qt::AlignRight);
		gridLayout->addWidget(ok, nitem, 1, Qt::AlignRight);

		connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

		this->setLayout(gridLayout);
		this->setWindowTitle("v3d convert");
	}
	CommonDialog(vector<pair<string, string> > _items, QWidget * parent = 0): QDialog(parent)
	{
		cur_demo_id = -1;
		cur_history_id = -1;
		title = "swc2mask";
		int nitem = _items.size();
		all_labels.resize(nitem);
		all_editors.resize(nitem);

		QGridLayout * gridLayout = new QGridLayout(parent);

		QGridLayout * check_layout = new QGridLayout(parent);
		int check_count = 0;

		int r = 0;
		for(int i = 0; i < nitem; i++)
		{
			string item = _items[i].first;
			items.push_back(item);

			if(item.find("input ") == 0 || item.find("output ") == 0)
			{
				all_labels[i] = new QLabel(QObject::tr((char*)(_items[i].first.c_str())), parent);
				all_editors[i] =new QLineEdit(QObject::tr((char*)(_items[i].second.c_str())), parent); 
				QPushButton * button = new QPushButton("...");
				button_map[button] = i;
				connect(button, SIGNAL(clicked()), this, SLOT(setFilePath()));
				gridLayout->addWidget(all_labels[i],  r, 0, 1, 1);
				gridLayout->addWidget(all_editors[i], r, 1, 1, 8);
				gridLayout->addWidget(button,         r, 9, 1, 1);
				r++;
			}
			else if(item.find("is ") == 0)
			{
				QCheckBox * checker = new QCheckBox(QObject::tr(item.c_str()), parent);
				all_editors[i] = checker;
				check_layout->addWidget(checker, check_count/3, check_count%3);
				check_count++;
			}
			else
			{
				all_labels[i] = new QLabel(QObject::tr((char*)(_items[i].first.c_str())), parent);
				all_editors[i] = new QLineEdit(QObject::tr((char*)(_items[i].second.c_str())), parent);
				gridLayout->addWidget(all_labels[i] , r, 0, 1, 1);
				gridLayout->addWidget(all_editors[i], r, 1, 1, 9);
				r++;
			}
		}

		if(check_count > 0) gridLayout->addLayout(check_layout, r++, 0, 1, 10);

		QPushButton * ok = new QPushButton("ok");
		QPushButton * cancel = new QPushButton("cancel");
		QPushButton * help = new QPushButton("help");
		demo = new QPushButton("demo"); demo->setVisible(false);
		history = new QPushButton("history"); history->setVisible(false);

		ok->setDefault(true);

		connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
		connect(help, SIGNAL(clicked()), this, SLOT(showHelp()));
		connect(demo, SIGNAL(clicked()), this, SLOT(showDemo()));
		connect(history, SIGNAL(clicked()), this, SLOT(showHistory()));

		QHBoxLayout * hbox = new QHBoxLayout;
		hbox->addWidget(help);
		hbox->addWidget(demo);
		hbox->addWidget(history);
		hbox->addWidget(cancel);
		hbox->addWidget(ok);

		gridLayout->addLayout(hbox, r++,0, 1, 10);

		this->setLayout(gridLayout);
		this->setWindowTitle("v3d convert");
	}

	~CommonDialog(){}

public slots:
	void accept()
	{
		update();
		return QDialog::accept();
	}

	void reject()
	{
		return QDialog::reject();
	}

	void update()
	{
		int nitem = items.size();
		cout<<"======================================"<<endl;
		for(int i = 0; i < nitem; i++)
		{
			string item = items[i];
			if(item.find("is ") == 0)
			{
				QCheckBox * checker = (QCheckBox*) (all_editors[i]);
				item_vals[item] = (checker->checkState() == Qt::Checked) ? "yes" : "no";
			}
			else
			{
				QLineEdit * line_editor = (QLineEdit *)(all_editors[i]);
				item_vals[item] = line_editor->text().toStdString();
			}
			cout<<"\""<<items[i]<<"\"\t= "<<item_vals[items[i]]<<endl;
		}
		cout<<"======================================"<<endl;
	}

	void setFilePath()
	{
		QPushButton * button = (QPushButton*) sender();
		int id = button_map[button];
		QLineEdit * editor = (QLineEdit*)(all_editors[id]);
		string item = items[id];
		QString fileName;
		if(item.find("image") != string::npos)
		{
			fileName = QFileDialog::getOpenFileName(this,
				     tr("Open Image"), "~/", tr("Image Files (*.tif *.tiff *.lsm *.raw *.jpg *.png *.bmp *.ppm)"));
		}
		else if(item.find("marker") != string::npos)
		{
			fileName = QFileDialog::getOpenFileName(this,
				     tr("Load Marker File"), "~/", tr("Marker File (*.marker)"));
		}
		else if(item.find("swc") != string::npos)
		{
			fileName = QFileDialog::getOpenFileName(this,
				     tr("Load SWC File"), "~/", tr("SWC File (*.swc)"));
		}
		else
		{
			fileName = QFileDialog::getOpenFileName(this,
				     tr("Open File"), "~/", tr("File (*.*)"));
		}
		if(fileName != "") editor->setText(fileName);
	}

	void showHelp()
	{
		QMessageBox msgBox;
		msgBox.setText(help_str);
		msgBox.exec();
	}

	void showDemo()
	{
		if(demo_list.empty()) return;
		cur_demo_id = (cur_demo_id + 1) % demo_list.size();
		//demo->setText(QObject::tr("demo %1/%2").arg(cur_demo_id).arg(demo_list.size()));
		this->setWindowTitle(QObject::tr("%1 - demo %2/%3").arg(title).arg(cur_demo_id+1).arg(demo_list.size()));

		int argc = 0, arg_begin = 1;
		char ** argv = 0;
		argc = split_string(demo_list[cur_demo_id].c_str(), argv);
		extern AdvancedParser parser;
		parser.parse(argc, argv, arg_begin);

		for(int i = 0; i < items.size(); i++)
		{
			string item = items[i];
			if(item.find("is ") == 0)
			{
				QCheckBox * checker = (QCheckBox*) (all_editors[i]);
				checker->setCheckState(Qt::Unchecked);
			}
			else
			{
				QLineEdit * line_editor = (QLineEdit *)(all_editors[i]);
				line_editor->setText("");
			}
		}

		for(int i = 0; i < items.size(); i++)
		{
			string item = items[i];
			if(item == "input image")
			{
				if(parser.filelist.empty()) continue;
				QLineEdit * line_editor = (QLineEdit *)(all_editors[i]);
				string all_images;
				int maxJ = (parser.filelist.size() > 1) ? parser.filelist.size() - 1 : 1;
				for(int j = 0; j < maxJ; j++) 
				{
					if(all_images == "") all_images = parser.filelist[j];
					else all_images += " " + parser.filelist[j];
				}
				line_editor->setText(QObject::tr(all_images.c_str()));
				continue;
			}
			else if(item == "output image")
			{
				if(parser.filelist.size() <= 1) continue;
				QLineEdit * line_editor = (QLineEdit *)(all_editors[i]);
				string last = parser.filelist[parser.filelist.size() - 1];
				if(parser.filelist.size() > 1) line_editor->setText(QObject::tr(last.c_str()));
				continue;
			}

			string para_name = item;
			if(para_name.find("input ") == 0) para_name = "-in" + para_name.substr(6, para_name.size() - 6);
			else if(para_name.find("output ") == 0) para_name = "-out" + para_name.substr(7, para_name.size() - 7);
			else if(para_name.find("is ") == 0) para_name == "-" + para_name.substr(3, para_name.size() - 3);
			else para_name = "-" + para_name;

			for(int j = 0; j < para_name.size(); j++) if(para_name[j] == ' ') para_name[j] = '-';

			if(item.find("is ") == 0)
			{
				QCheckBox * checker = (QCheckBox*) (all_editors[i]);
				if(parser.is_exist(para_name)) checker->setCheckState(Qt::Checked);
				else checker->setCheckState(Qt::Unchecked);
			}
			else
			{
				QLineEdit * line_editor = (QLineEdit *)(all_editors[i]);
				string para_str = parser.get_para(para_name);
				if(para_str == "" && para_name == "-outmarker") line_editor->setText("/tmp/output.marker");
				else if(para_str == "" && para_name == "-outswc") line_editor->setText("/tmp/output.swc");
				else line_editor->setText(QObject::tr(parser.get_para(para_name).c_str()));
			}
		}
	}

	void showHistory()
	{
		if(history_list.empty()) return;
		cur_history_id = (cur_history_id + 1) % history_list.size();
		this->setWindowTitle(QObject::tr("%1 - history %2/%3").arg(title).arg(cur_history_id+1).arg(history_list.size()));

		int argc = 0, arg_begin = 1;
		char ** argv = 0;
		argc = split_string(history_list[cur_history_id].c_str(), argv);
		extern AdvancedParser parser;
		parser.parse(argc, argv, arg_begin);

		for(int i = 0; i < items.size(); i++)
		{
			string item = items[i];
			if(item.find("is ") == 0)
			{
				QCheckBox * checker = (QCheckBox*) (all_editors[i]);
				checker->setCheckState(Qt::Unchecked);
			}
			else
			{
				QLineEdit * line_editor = (QLineEdit *)(all_editors[i]);
				line_editor->setText("");
			}
		}

		for(int i = 0; i < items.size(); i++)
		{
			string item = items[i];
			if(item == "input image")
			{
				if(parser.filelist.empty()) continue;
				QLineEdit * line_editor = (QLineEdit *)(all_editors[i]);
				string all_images;
				int maxJ = (parser.filelist.size() > 1) ? parser.filelist.size() - 1 : 1;
				for(int j = 0; j < maxJ; j++) 
				{
					if(all_images == "") all_images = parser.filelist[j];
					else all_images += " " + parser.filelist[j];
				}
				line_editor->setText(QObject::tr(all_images.c_str()));
				continue;
			}
			else if(item == "output image")
			{
				if(parser.filelist.size() <= 1) continue;
				QLineEdit * line_editor = (QLineEdit *)(all_editors[i]);
				string last = parser.filelist[parser.filelist.size() - 1];
				if(parser.filelist.size() > 1) line_editor->setText(QObject::tr(last.c_str()));
				continue;
			}

			string para_name = item;
			if(para_name.find("input ") == 0) para_name = "-in" + para_name.substr(6, para_name.size() - 6);
			else if(para_name.find("output ") == 0) para_name = "-out" + para_name.substr(7, para_name.size() - 7);
			else if(para_name.find("is ") == 0) para_name == "-" + para_name.substr(3, para_name.size() - 3);
			else para_name = "-" + para_name;

			for(int j = 0; j < para_name.size(); j++) if(para_name[j] == ' ') para_name[j] = '-';

			if(item.find("is ") == 0)
			{
				QCheckBox * checker = (QCheckBox*) (all_editors[i]);
				if(parser.is_exist(para_name)) checker->setCheckState(Qt::Checked);
				else checker->setCheckState(Qt::Unchecked);
			}
			else
			{
				QLineEdit * line_editor = (QLineEdit *)(all_editors[i]);
				string para_str = parser.get_para(para_name);
				if(para_str == "" && para_name == "-outmarker") line_editor->setText("/tmp/output.marker");
				if(para_str == "" && para_name == "-outswc") line_editor->setText("/tmp/output.swc");
				else line_editor->setText(QObject::tr(parser.get_para(para_name).c_str()));

			}
		}
	}

public:
	void setDemo(vector<string> & _demo_list)
	{
		demo_list = _demo_list;
		if(!demo_list.empty()) 
		{
			demo->setVisible(true);
		}
	}
	void setHistory(vector<string> & _history_list)
	{
		history_list = _history_list;
		if(!history_list.empty()) 
		{
			history->setVisible(true);
		}
	}
	void setHelp(QString _help_str)
	{
		help_str = _help_str;
	}
	string get_para(string item)
	{
		if(item_vals.find(item) == item_vals.end()) return "";
		return item_vals[item];
	}
	template<class T> void get_num(string item, T val)
	{
		if(item_vals.find(item) == item_vals.end()) val = 0;
		string str = item_vals[item];
		std::istringstream iss(str);
		iss >> val;
	}

	void setTitle(QString _title)
	{
		title = _title;
		this->setWindowTitle(title);
	}

private:
	vector<string> items;
	map<string, string> item_vals;
	vector<QLabel*> all_labels;
	vector<QWidget*> all_editors;
	map<QPushButton*, int> button_map;
	QString help_str;
	
	vector<string> demo_list;
	QPushButton * demo;
	int cur_demo_id;

	QPushButton * history;
	vector<string> history_list;
	int cur_history_id;

	QString title;
};

#endif
