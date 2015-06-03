/* ocp_2_vaa3d_plugin.cpp
 * Plugin to import OCP data to Vaa3D
 * 2015-6-1 : by JHU-APL
 */
 
#include "v3d_message.h"
#include <vector>
#include "ocp_2_vaa3d_plugin.h"
#include <boost/filesystem.hpp>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
using namespace std;
Q_EXPORT_PLUGIN2(ocp_2_vaa3d, OCP2Vaa3D);
 
QStringList OCP2Vaa3D::menulist() const
{
	return QStringList() 
		<<tr("OCP_to_Vaa3D")
		<<tr("about");
}

QStringList OCP2Vaa3D::funclist() const
{
	return QStringList()
		<<tr("Import_data")
		<<tr("help");
}

//---------------------------------------------------------------------------------------------------------
void select_project(const vector<string> &ocp_projects, string &selected_project)
{
    // -------------------
    //  Select project
    // -------------------
    QDialog dialog;
    QFormLayout form(&dialog);
    QComboBox projects(&dialog);

    for (int i=0; i<ocp_projects.size(); ++i) {
        projects.addItem(QString(ocp_projects[i].c_str()));
    }
    form.addRow(&projects);

    QLabel *label = new QLabel;
    label->setText(ocp_projects[0].c_str());

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));
    QObject::connect(&projects, SIGNAL(currentIndexChanged(QString)), label, SLOT(setText(QString)));

    if (dialog.exec() == QDialog::Accepted) {
        selected_project = label->text().toStdString();
        qDebug() << QString(selected_project.c_str());
    } else {
        selected_project = "";
    }
}
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
void retrieve_ocp_url(const string &project, const vector<string> &channels,
                      const vector<string> &resolutions, string &data_url)
{
    int num_resolutions = resolutions.size();
    int num_channels = channels.size();

    // -------------------
    //  Select dataset parameters
    // -------------------
    QDialog dialog;
    QGridLayout *mainLayout = new QGridLayout;

    // Image Resolution Selection
    QGroupBox *resolutionBox = new QGroupBox(QString("Resolution (x_end,y_end,z_start,z_end):"));
    QVBoxLayout *resolutionBoxLayout = new QVBoxLayout();
    QRadioButton *buttons[num_resolutions];
    for (int i=0; i<num_resolutions; ++i) {
        buttons[i] = new QRadioButton(resolutions[i].c_str());
        resolutionBoxLayout->addWidget(buttons[i]);
        if (i == 0) {
            buttons[i]->setChecked(true);
        }
    }
    resolutionBox->setLayout(resolutionBoxLayout);
    mainLayout->addWidget(resolutionBox,0,0,2,1);

    // Channel Selection
    QLabel *channelLabel = new QLabel;
    if (channels.empty()) {
        channelLabel->setText(QString(""));
    } else {
        channelLabel->setText(QString(channels[0].c_str()));
    }

    QGroupBox *channelBox = new QGroupBox(QString("Channel:"));
    QVBoxLayout *channelBoxLayout = new QVBoxLayout;
    QComboBox *channelComboBox = new QComboBox;

    for (int i=0; i<num_channels; ++i) {
        channelComboBox->addItem(QString(channels[i].c_str()));
    }
    channelBoxLayout->addWidget(channelComboBox);
    channelBox->setLayout(channelBoxLayout);
    mainLayout->addWidget(channelBox,0,1,1,1);

    // User volume info
    QGroupBox *formBox = new QGroupBox("OCP Data Parameters:");
    QFormLayout *formLayout = new QFormLayout;

    // Add some text above the fields
    QList<QLineEdit *> fields;
    QLineEdit *xLine = new QLineEdit;
    QString xLabel = QString("X [start,stop]:");
    formLayout->addRow(xLabel, xLine);
    fields << xLine;

    QLineEdit *yLine = new QLineEdit;
    QString yLabel = QString("Y [start,stop]:");
    formLayout->addRow(yLabel, yLine);
    fields << yLine;

    QLineEdit *zLine = new QLineEdit;
    QString zLabel = QString("Z [start,stop]:");
    formLayout->addRow(zLabel, zLine);
    fields << zLine;

    formBox->setLayout(formLayout);
    mainLayout->addWidget(formBox,1,1,1,1);

    // Ok/cancel button
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                         | QDialogButtonBox::Cancel);

    mainLayout->addWidget(buttonBox,2,1,1,1);
    QObject::connect(buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));
    QObject::connect(channelComboBox, SIGNAL(currentIndexChanged(QString)), channelLabel, SLOT(setText(QString)));

    // Prepare dialog for viewing
    dialog.setLayout(mainLayout);
    string x_extents,y_extents,z_extents;
    string channel = "";
    int resolution_idx = -1;
    if (dialog.exec() == QDialog::Accepted) {
        // Retrieve resolution value
        for (int i=0; i<num_resolutions; ++i) {
            if (buttons[i]->isChecked()) {
                resolution_idx = i;
                break;
            }
        }

        // Retrieve channel value
        if (num_channels > 0) {
            channel = channelLabel->text().toStdString();
        }

        // Retrieve data extents
        x_extents = xLine->text().toStdString();
        y_extents = yLine->text().toStdString();
        z_extents = zLine->text().toStdString();

        // Debugging
        qDebug() << QString(project.c_str());
        qDebug() << resolution_idx;
        qDebug() << QString(channel.c_str());
        qDebug() << QString(x_extents.c_str());
        qDebug() << QString(y_extents.c_str());
        qDebug() << QString(z_extents.c_str());
    } else {
        qDebug() << "Exiting";
        data_url = "";
        return;
    }

    // -------------------
    //  Format URL string
    // -------------------
    string base_url = "/ocp/ca";
    char buffer[200];
    if (num_channels == 0) {
        // Omit channel from the URL
        sprintf(buffer,"%s/%s/%d/%s/%s/%s",base_url.c_str(),project.c_str(),resolution_idx,x_extents.c_str(),y_extents.c_str(),z_extents.c_str());
    } else {
        sprintf(buffer,"%s/%s/%s/%d/%s/%s/%s",base_url.c_str(),project.c_str(),channel.c_str(),resolution_idx,x_extents.c_str(),y_extents.c_str(),z_extents.c_str());
    }
    data_url = buffer;

    qDebug() << QString(data_url.c_str());
}
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
void OCP2Vaa3D::import_from_ocp(V3DPluginCallback2 &callback, QWidget *parent) {

    // -------------------
    // Retrieve list of projects from OCP url
    // -------------------
    /*
     * TODO: Code this up
     */
    vector<string> ocp_projects;
    // Dummy list
    ocp_projects.push_back("proj1");
    ocp_projects.push_back("proj2");
    ocp_projects.push_back("proj3");

    // -------------------
    //  Get project data
    // -------------------
    string project;
    select_project(ocp_projects,project);
//    get_proj_info_file(project,save_path);

    // -------------------
    //  Display options and retrieve URL
    // -------------------
    string ocp_url;
    vector<string> channels,resolutions;
    resolutions.push_back("0: 100,100,100");
    resolutions.push_back("1: 100,100,100");
    resolutions.push_back("2: 100,100,100");
    resolutions.push_back("3: 100,100,100");
    resolutions.push_back("4: 100,100,100");
    resolutions.push_back("5: 100,100,100");

//    channels.push_back("A");
//    channels.push_back("B");
//    channels.push_back("C");
    retrieve_ocp_url(project,channels,resolutions,ocp_url);

    // -------------------
    //  Retrieve data
    // -------------------
//    get_hdf_data(ocp_url);

    // -------------------
    //  Convert volume to Vaa3D format
    // -------------------
//    V3DLONG sz_relative[4];
//    Image4DSimple p4DImage;
//    p4DImage.setData((unsigned char*)relative1d, sz_relative[0], sz_relative[1], sz_relative[2], sz_relative[3], V3D_UINT8);
//    v3dhandle newwin = callback.newImageWindow();
//    callback.setImage(newwin, &p4DImage);
//    callback.updateImageWindow(newwin);

}
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
void OCP2Vaa3D::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("OCP_to_Vaa3D"))
	{
        import_from_ocp(callback,parent);
	}
	else
	{
		v3d_msg(tr("Plugin to import OCP data to Vaa3D. "
			"Developed by JHU-APL, 2015-6-1"));
	}
}

bool OCP2Vaa3D::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("Import_data"))
	{
        v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}


