#ifndef __PLUGIN_CREATOR_GUI_H__
#define __PLUGIN_CREATOR_GUI_H__

#include <string>
#include <QtGui>
#include <v3d_interface.h>

using namespace std;

class GuidingDialog : public QDialog
{
	Q_OBJECT

public:
	GuidingDialog(QWidget * parent) : QDialog(parent)
	{
        label_plugin_name = new QLabel(tr("Plugin Name(Tracing Method) :"));
        editor_plugin_name = new QLineEdit(tr("mostVesselTracer"));

        label_function_name = new QLabel(tr("Function Name(Tracing Method) :"));
        editor_function_name = new QLineEdit(tr("MOST_trace"));

        label_swc_name = new QLabel(tr("Output SWC Format(Set NULL if Defined by The User):"));
        editor_swc_name = new QLineEdit(tr("_MOST.swc"));

        parameter_panel = new QGroupBox("");
        parameter_panel->setStyle(new QWindowsStyle());
        parameterLayout = new QGridLayout();
        parameterLayout->addWidget(new QLabel(QObject::tr("Number of Input Parameters                                          ")),0,0,1,1);
        para_spinbox = new QSpinBox();
        para_spinbox->setRange(0,10);
        para_spinbox->setValue(0);
        parameterLayout->addWidget(para_spinbox,0,1,1,9);
        parameter_panel->setLayout(parameterLayout);


		label_plugin_description = new QLabel(tr("Plugin Description :"));
		editor_plugin_description = new QLineEdit(tr("This is a test plugin, you can use it as a demo."));

		label_plugin_date = new QLabel(tr("Plugin Date :"));
        editor_plugin_date = new QLineEdit(tr("2015-01-01"));

		label_plugin_author = new QLabel(tr("Plugin Author :"));
		editor_plugin_author = new QLineEdit(tr("YourName"));

		label_vaa3d_path = new QLabel(tr("Vaa3D whole-project path :"));
		editor_vaa3d_path = new QLineEdit(tr(""));
		button_vaa3d_path = new QPushButton(tr("..."));

		label_save_folder = new QLabel(tr("Save Folder : "));
		editor_save_folder = new QLineEdit(tr(""));
		button_save_folder = new QPushButton(tr("..."));

		ok = new QPushButton(tr("ok"));
		cancel = new QPushButton(tr("cancel"));

		gridLayout = new QGridLayout();
		gridLayout->addWidget(label_plugin_name,         0, 0, 1, 1);
		gridLayout->addWidget(editor_plugin_name,        0, 1, 1, 9);
        gridLayout->addWidget(label_function_name,        1, 0, 1, 1);
        gridLayout->addWidget(editor_function_name,       1, 1, 1, 9);
        gridLayout->addWidget(label_swc_name,        2, 0, 1, 1);
        gridLayout->addWidget(editor_swc_name,       2, 1, 1, 9);

        gridLayout->addWidget(parameter_panel,3,0,1,10);

//		gridLayout->addWidget(label_plugin_description,  3, 0, 1, 1);
//		gridLayout->addWidget(editor_plugin_description, 3, 1, 1, 9);
		gridLayout->addWidget(label_plugin_date,         4, 0, 1, 1);
		gridLayout->addWidget(editor_plugin_date,        4, 1, 1, 9);
		gridLayout->addWidget(label_plugin_author,       5, 0, 1, 1);
		gridLayout->addWidget(editor_plugin_author,      5, 1, 1, 9);
		gridLayout->addWidget(label_vaa3d_path,       6, 0, 1, 1);
		gridLayout->addWidget(editor_vaa3d_path,      6, 1, 1, 8);
		gridLayout->addWidget(button_vaa3d_path,      6, 9, 1, 1);
		gridLayout->addWidget(label_save_folder,         9, 0, 1, 1);
		gridLayout->addWidget(editor_save_folder,        9, 1, 1, 8);
		gridLayout->addWidget(button_save_folder,        9, 9, 1, 1);
		gridLayout->addWidget(cancel, 10, 0, 1, 5, Qt::AlignRight);
		gridLayout->addWidget(ok, 10, 5, 1, 5, Qt::AlignRight);

		connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
		connect(button_vaa3d_path, SIGNAL(clicked()), this, SLOT(setFolderPath()));
		connect(button_save_folder, SIGNAL(clicked()), this, SLOT(setFolderPath()));

        connect(para_spinbox, SIGNAL(valueChanged(int)), this, SLOT(setParameters()));


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

	void setFolderPath()
	{
        QPushButton * button = (QPushButton*) sender();
        QLineEdit * editor;
        QString title;
        if(button == button_vaa3d_path)
        {
            editor = editor_vaa3d_path;
            title = "Open V3D_MAIN Directory";
        }
        else if(button == button_save_folder)
        {
            editor = editor_save_folder;
            title = "Save to Directory";
        }

        QString dir = QFileDialog::getExistingDirectory(this, title,
                                                 "~/",
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
        editor->setText(dir);

    }

    void setParameters()
    {
        para_number =  para_spinbox->value();
        for(int i = 0; i < para_number; i++)
             parameterLayout->addWidget(new QLabel(QObject::tr("...")),1+i,1,1,9);

        parameter_panel->setLayout(parameterLayout);
        gridLayout->addWidget(parameter_panel,3,0,1,10);

    }

	void update()
	{
		plugin_name =  editor_plugin_name->text().toStdString();
        function_name =  editor_function_name->text().toStdString();

		plugin_desp =  editor_plugin_description->text().toStdString();
		plugin_date =  editor_plugin_date->text().toStdString();
		plugin_author =  editor_plugin_author->text().toStdString();
		vaa3d_path =  editor_vaa3d_path->text().toStdString();
		save_folder =  editor_save_folder->text().toStdString();

	}

public:
	string plugin_name;
    string function_name;
    string outputswc_name;

	string plugin_desp;
	string plugin_date;
	string plugin_author;
	string vaa3d_path;
	string save_folder;

	QLabel * label_plugin_name;
	QLineEdit * editor_plugin_name;

    QLabel * label_function_name;
    QLineEdit * editor_function_name;

    QLabel * label_swc_name;
    QLineEdit * editor_swc_name;

    QLabel * label_plugin_description;
	QLineEdit * editor_plugin_description;

	QLabel * label_plugin_date;
	QLineEdit * editor_plugin_date;

	QLabel * label_plugin_author;
	QLineEdit * editor_plugin_author;

	QLabel * label_vaa3d_path;
	QLineEdit * editor_vaa3d_path;
	QPushButton * button_vaa3d_path;

	QLabel * label_save_folder;
	QLineEdit * editor_save_folder;
	QPushButton * button_save_folder;

	QPushButton * ok;
	QPushButton * cancel;

	QGridLayout * gridLayout;

    QSpinBox * para_spinbox;
    QGroupBox *parameter_panel;
    QGridLayout *parameterLayout;

    int para_number;

};

#endif
