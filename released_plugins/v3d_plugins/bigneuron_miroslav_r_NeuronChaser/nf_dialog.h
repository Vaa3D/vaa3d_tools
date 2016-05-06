#ifndef NF_DIALOG_H
#define NF_DIALOG_H
#include <QtGui>

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <set>
#include <sstream>
#include <fstream>

using namespace std;

class CommonDialog : public QDialog
{
    Q_OBJECT

public:
    CommonDialog(vector<string> _items, QWidget * parent = 0): QDialog(parent)
    {
        history_identity = "";
        is_history = false;
        cur_history_id = 0;

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
                printf("\n111\n");
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
                all_editors[i] = new QLineEdit(); // QString::number(22).toStdString().c_str()
                gridLayout->addWidget(all_labels[i] , r, 0, 1, 1);
                gridLayout->addWidget(all_editors[i], r, 1, 1, 9);
                r++;

            }
        }

        help = new QPushButton("help"); help->setVisible(false);
        history = new QPushButton("history"); history->setVisible(false);
        QPushButton * cancel = new QPushButton("cancel");
        QPushButton * ok = new QPushButton("  ok  ");

        ok->setDefault(true);

        QHBoxLayout * hbox = new QHBoxLayout;
        hbox->addWidget(help);
        hbox->addWidget(history);
        hbox->addWidget(cancel);
        hbox->addWidget(ok);

        gridLayout->addLayout(hbox, r++, 0, 1, 10);

        connect(help, SIGNAL(clicked()), this, SLOT(showHelp()));
        connect(history, SIGNAL(clicked()), this, SLOT(showHistory()));
        connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
        connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

        this->setLayout(gridLayout);
        this->setWindowTitle("Set Paramenter");
    }

    CommonDialog(vector<string> _items, vector<string> _inits, QWidget * parent = 0): QDialog(parent)
    {
        history_identity = "";
        is_history = false;
        cur_history_id = 0;

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
                printf("\n111\n");
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
                all_editors[i] = new QLineEdit(QString::fromStdString(_inits[i]).toStdString().c_str());
                gridLayout->addWidget(all_labels[i] , r, 0, 1, 1);
                gridLayout->addWidget(all_editors[i], r, 1, 1, 9);
                r++;

            }
        }

        help = new QPushButton("help"); help->setVisible(false);
        history = new QPushButton("history"); history->setVisible(false);
        QPushButton * cancel = new QPushButton("cancel");
        QPushButton * ok = new QPushButton("  ok  ");

        ok->setDefault(true);

        QHBoxLayout * hbox = new QHBoxLayout;
        hbox->addWidget(help);
        hbox->addWidget(history);
        hbox->addWidget(cancel);
        hbox->addWidget(ok);

        gridLayout->addLayout(hbox, r++, 0, 1, 10);

        connect(help, SIGNAL(clicked()), this, SLOT(showHelp()));
        connect(history, SIGNAL(clicked()), this, SLOT(showHistory()));
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
            saveHistory();
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

    void showHistory()
    {
        if(!is_history) return;
        istringstream iss(history_list[cur_history_id]);
        string item_val;
        getline(iss, item_val, ',');
        int item_id = 0;
        while(iss.good() && item_id < items.size())
        {
            getline(iss, item_val, ',');
            all_editors[item_id++]->setText((char*) item_val.c_str());
        }
        history->setText(QObject::tr("history %1/%2").arg(cur_history_id+1).arg(history_list.size()));
        cur_history_id = (cur_history_id + 1) % history_list.size();
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
    void get_num(string item, int &val)
    {
        if(item_vals.find(item) == item_vals.end()) val = 0;
        string str = item_vals[item];
        std::istringstream iss(str);
//        cout << item << endl;
//        cout << "str ----->" << str << endl;
        iss >> val;
//        cout << "val ----->" << val << endl;
    }
    void get_num(string item, long &val)
    {
        if(item_vals.find(item) == item_vals.end()) val = 0;
        string str = item_vals[item];
        std::istringstream iss(str);
        iss >> val;
    }
    void get_num(string item, float &val)
    {
        if(item_vals.find(item) == item_vals.end()) val = 0;
        string str = item_vals[item];
        std::istringstream iss(str);
        iss >> val;
    }
    void get_num(string item, double &val)
    {
        if(item_vals.find(item) == item_vals.end()) val = 0;
        string str = item_vals[item];
        std::istringstream iss(str);
        iss >> val;
    }
    void setHistory(bool _is_history = true)
    {
        is_history = _is_history;
        if(!is_history) return;

        history_identity = this->windowTitle();
        loadHistory();
        if(!history_list.empty())  history->setVisible(true);
    }
private:

    void saveHistory()
    {
        if(!is_history) return;
        cout<<"============== saveHistory() ==================="<<endl;
        string history_file = getenv("HOME") + string("/.common_dialog_history");

        cout<<"save history to "<<history_file<<endl;
        ofstream ofs(history_file.c_str(), ios_base::app);
        ofs<<history_identity.toStdString();
        for(int i = 0; i < items.size(); i++)
        {
            string item = items[i];
            string item_val = item_vals[item];
            ofs<<","<<item_val;
        }
        ofs<<endl;
        ofs.close();
    }
    void loadHistory()
    {
        if(!is_history) return;
        string history_file = getenv("HOME") + string("/.common_dialog_history");

        ifstream ifs(history_file.c_str());
        vector<string> history_matched;
        set<string> history_set;
        while(ifs.good())
        {
            string line; getline(ifs, line);
            if(line != "" && line.find(history_identity.toStdString()) == 0) history_matched.push_back(line);
        }
        ifs.close();

        for(vector<string>::reverse_iterator it = history_matched.rbegin(); it != history_matched.rend(); it++)
        {
            if(history_set.find(*it) == history_set.end())
            {
                history_list.push_back(*it);
                history_set.insert(*it);
            }
        }
        history_matched.clear();
        history_set.clear();
    }
private:
    vector<string> items;
    map<string, string> item_vals;
    vector<QLabel*> all_labels;
    vector<QLineEdit*> all_editors;
    map<QPushButton*, int> button_map;

    QPushButton * help;
    QString help_str;

    QPushButton * history;
    vector<string> history_list;
    int cur_history_id;
    QString history_identity;
    bool is_history;
};

#endif // NF_DIALOG_H
