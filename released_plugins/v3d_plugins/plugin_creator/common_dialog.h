#ifndef __COMMON_DIALOG_H__
#define __COMMON_DIALOG_H__
#include <QtGui>

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

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

		int r = 0;
		for(int i = 0; i < nitem; i++)
		{
			string item = items[i];

			if(item.find("path") != string::npos || item.find("Path") != string::npos ||
				item.find("dir") != string::npos || item.find("Dir") != string::npos ||
				item.find("folder") != string::npos || item.find("Folder") != string::npos ||
				item.find("input") != string::npos || item.find("Input") != string::npos ||
				item.find("output") != string::npos || item.find("Output") != string::npos)
			{
				all_labels[i] = new QLabel(QObject::tr((char*)(item.c_str())));
				all_editors[i] =new QLineEdit(); 
				QPushButton * button = new QPushButton("...");
				button_map[button] = i;
				connect(button, SIGNAL(clicked()), this, SLOT(setFilePath()));
				gridLayout->addWidget(all_labels[i],  r, 0, 1, 1);
				gridLayout->addWidget(all_editors[i], r, 1, 1, 8);
				gridLayout->addWidget(button,         r, 9, 1, 1);
				r++;
			}
			else
			{
				all_labels[i] = new QLabel(QObject::tr((char*)(item.c_str())));
				all_editors[i] = new QLineEdit();
				gridLayout->addWidget(all_labels[i] , r, 0, 1, 1);
				gridLayout->addWidget(all_editors[i], r, 1, 1, 9);
				r++;

			}
		}

		help = new QPushButton("help"); help->setVisible(false);
		QPushButton * cancel = new QPushButton("cancel");
		QPushButton * ok = new QPushButton("  ok  ");

		QHBoxLayout * hbox = new QHBoxLayout;
		hbox->addWidget(help);
		hbox->addWidget(cancel);
		hbox->addWidget(ok);

		gridLayout->addLayout(hbox, r++, 0, 1, 10);

		connect(help, SIGNAL(clicked()), this, SLOT(showHelp()));
		connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

		this->setLayout(gridLayout);
		this->setWindowTitle("Set Paramenter");
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

			QLineEdit * line_editor = (QLineEdit *)(all_editors[i]);
			item_vals[item] = line_editor->text().toStdString();

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
		QString editor_text;
		if(item.find("input") != string::npos || item.find("Input") != string::npos ||
			item.find("output") != string::npos || item.find("Output") != string::npos)
		{
			editor_text = QFileDialog::getOpenFileName(this,
					tr("Open File"), "~/", tr("File (*.*)"));
		}
		if(item.find("path") != string::npos || item.find("Path") != string::npos ||
				item.find("dir") != string::npos || item.find("Dir") != string::npos ||
				item.find("folder") != string::npos || item.find("Folder") != string::npos)
		{
			editor_text = QFileDialog::getExistingDirectory(0, QObject::tr("Open Directory"),
					"~/",
					QFileDialog::ShowDirsOnly
					| QFileDialog::DontResolveSymlinks);	
		}
		if(editor_text != "") editor->setText(editor_text);
	}

	void showHelp()
	{
		QMessageBox msg;
		msg.setText(help_str);
		msg.exec();
	}
	
public:

	void setHelp(QString str)
	{
		help_str = str;
		help->setVisible(true);
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

private:
	vector<string> items;
	map<string, string> item_vals;
	vector<QLabel*> all_labels;
	vector<QWidget*> all_editors;
	map<QPushButton*, int> button_map;

	QPushButton * help;
	QString help_str;
};

#endif
