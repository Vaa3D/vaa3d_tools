#ifndef __PLUGIN_CREATOR_GUI_H__
#define __PLUGIN_CREATOR_GUI_H__

#include <QtGui>
#include <v3d_interface.h>

class GuidingDialog : public QDialog
{
	Q_OBJECT

public:
	GuidingDialog(V3DPluginCallback2 &callback, QWidget * parent) : QDialog(parent)
	{
		this->callback = &callback;
		curwin = callback.currentImageWindow();

		v3dhandleList win_list = callback.getImageWindowList();
		QStringList items;
		for(int i = 0; i < win_list.size(); i++) items << callback.getImageName(win_list[i]);

		label1 = new QLabel(tr("Plugin Name :"));
		editor1 = new QLineEdit(tr("test"));

		label2 = new QLabel(tr("Plugin Class Name :"));
		editor2 = new QLineEdit(tr("TestPlugin"));

		label3 = new QLabel(tr("Window Title :"));
		editor3 = new QLineEdit(tr("This is Test Plugin"));

		label4 = new QLabel(tr("Plugin Description :"));
		editor4 = new QLineEdit(tr("This is a test plugin, you can use it as a demo."));

		label5 = new QLabel(tr("Pugin Date :"));
		editor5 = new QLineEdit(tr("2012-01-01"));

		label6 = new QLabel(tr("Pugin Author :"));
		editor6 = new QLineEdit(tr("YourName"));

		label7 = new QLabel(tr("v3d_main Path :"));
		editor7 = new QLineEdit(tr("/V3D/Main/Path"));

		label8 = new QLabel(tr("Menu List : "));
		editor8 = new QLineEdit(tr("menu1 menu2"));

		label9 = new QLabel(tr("Func List : "));
		editor9 = new QLineEdit(tr("func1 func2"));

		label10 = new QLabel(tr("Save Folder : "));
		editor10 = new QLineEdit(tr("/plugin/folder"));

		ok = new QPushButton(tr("ok"));
		cancel = new QPushButton(tr("cancel"));

		gridLayout = new QGridLayout();
		gridLayout->addWidget(label1, 0, 0);
		gridLayout->addWidget(editor1, 0, 1, 1, 3);
		gridLayout->addWidget(label2, 1, 0);
		gridLayout->addWidget(editor2, 1, 1, 1, 3);
		gridLayout->addWidget(label3, 2, 0);
		gridLayout->addWidget(editor3, 2, 1, 1, 3);
		gridLayout->addWidget(label4, 3, 0);
		gridLayout->addWidget(editor4, 3, 1, 1, 3);
		gridLayout->addWidget(label5, 4, 0);
		gridLayout->addWidget(editor5, 4, 1, 1, 3);
		gridLayout->addWidget(label6, 5, 0);
		gridLayout->addWidget(editor6, 5, 1, 1, 3);
		gridLayout->addWidget(label7, 6, 0);
		gridLayout->addWidget(editor7, 6, 1, 1, 3);
		gridLayout->addWidget(label8, 7, 0);
		gridLayout->addWidget(editor8, 7, 1, 1, 3);
		gridLayout->addWidget(label9, 8, 0);
		gridLayout->addWidget(editor9, 8, 1, 1, 3);
		gridLayout->addWidget(label10, 9, 0);
		gridLayout->addWidget(editor10, 9, 1, 1, 3);
		gridLayout->addWidget(cancel, 10, 2, Qt::AlignRight);
		gridLayout->addWidget(ok, 10, 3, Qt::AlignRight);

		connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

		connect(ok, SIGNAL(clicked()), this, SLOT(update()));

		this->setLayout(gridLayout);
		this->setWindowTitle("Plugin Creator Dialog");

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

	void update()
	{
		plugin_name =  editor1->text().toStdString();
		plugin_class =  editor2->text().toStdString();
		win_title =  editor3->text().toStdString();
		plugin_desp =  editor4->text().toStdString();
		plugin_date =  editor5->text().toStdString();
		plugin_author =  editor6->text().toStdString();
		v3dmain_path =  editor7->text().toStdString();
		menulst =  editor8->text().toStdString();
		funclst =  editor9->text().toStdString();
		save_folder =  editor10->text().toStdString();

	}

public:
	string plugin_name;
	string plugin_class;
	string win_title;
	string plugin_desp;
	string plugin_date;
	string plugin_author;
	string v3dmain_path;
	string menulst;
	string funclst;
	string save_folder;
	QLabel * label1;
	QLineEdit * editor1;

	QLabel * label2;
	QLineEdit * editor2;

	QLabel * label3;
	QLineEdit * editor3;

	QLabel * label4;
	QLineEdit * editor4;

	QLabel * label5;
	QLineEdit * editor5;

	QLabel * label6;
	QLineEdit * editor6;

	QLabel * label7;
	QLineEdit * editor7;

	QLabel * label8;
	QLineEdit * editor8;

	QLabel * label9;
	QLineEdit * editor9;

	QLabel * label10;
	QLineEdit * editor10;

	QPushButton * ok;
	QPushButton * cancel;

	QGridLayout * gridLayout;

	V3DPluginCallback2 * callback;
	v3dhandle curwin;
};

#endif
