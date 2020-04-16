#include "downsample_gui.h"

DownsampleDialog :: DownsampleDialog(QWidget *parent): QDialog(parent)
{

    res_output_x = new QSpinBox();
    res_output_x -> setRange(1, 100);
    res_output_x -> setValue(32);
    res_output_y = new QSpinBox();
    res_output_y -> setRange(1, 100);
    res_output_y -> setValue(32);
    res_output_z = new QSpinBox();
    res_output_z -> setRange(1, 100);
    res_output_z -> setValue(32);

    connect(res_output_x, SIGNAL(valueChanged(int)), res_output_x, SLOT(setValue(int)));
    connect(res_output_y, SIGNAL(valueChanged(int)), res_output_y, SLOT(setValue(int)));
    connect(res_output_z, SIGNAL(valueChanged(int)), res_output_z, SLOT(setValue(int)));

    input_file_Btn = new QPushButton(tr("Input a file"));
    file_edit = new QLineEdit(tr("Select a file !"));
    hbox1 = new QHBoxLayout();
    hbox1 -> addWidget(input_file_Btn);
    hbox1 -> addWidget(file_edit);
    connect(input_file_Btn, SIGNAL(clicked(bool)), this, SLOT(selectFiles()));

    input_folder_Btn = new QPushButton(tr("Input a folder"));
    folder_edit = new QLineEdit(tr("Select a folder !"));
    hbox2 = new QHBoxLayout();
    hbox2 -> addWidget(input_folder_Btn);
    hbox2 -> addWidget(folder_edit);
    connect(input_folder_Btn, SIGNAL(clicked(bool)), this, SLOT(selectFiles()));

    saveBtn = new QPushButton(tr("Save files"));
    files_save = new QLineEdit(tr("Select a file or folder to save files !"));
    hbox_save = new QHBoxLayout();
    hbox_save -> addWidget(saveBtn);
    hbox_save -> addWidget(files_save);
    connect(saveBtn, SIGNAL(clicked(bool)), this, SLOT(saveFiles()));

    ok = new QPushButton(tr("ok"));
    ok -> setDefault(true);
    cancel = new QPushButton(tr("cancel"));
    cancel -> setDefault(false);
    hbox3 = new QHBoxLayout();
    hbox3 -> addWidget(ok);
    hbox3 -> addWidget(cancel);
    connect(ok, SIGNAL(clicked(bool)), this, SLOT(accept()));
    connect(cancel, SIGNAL(clicked(bool)), this, SLOT(reject()));

    gridLayout = new QGridLayout();
    gridLayout -> addWidget(new QLabel("The adjacent resolutions is number 2 !"), 0, 0);
    gridLayout -> addWidget(new QLabel("The x resolution of the exported files"), 1, 0);
    gridLayout -> addWidget(res_output_x, 1, 1);
    gridLayout -> addWidget(new QLabel("The y resolution of the exported files"), 2, 0);
    gridLayout -> addWidget(res_output_y, 2, 1);
    gridLayout -> addWidget(new QLabel("The z resolution of the exported files"), 3, 0);
    gridLayout -> addWidget(res_output_z, 3, 1);

    gridLayout -> addLayout(hbox1, 4, 0);
    gridLayout -> addLayout(hbox2, 5, 0);
    gridLayout -> addLayout(hbox_save, 6, 0);
    gridLayout -> addLayout(hbox3, 7, 0);

    this -> setLayout(gridLayout);
    this -> setWindowTitle("DownSample files");
}

void DownsampleDialog:: selectFiles()
{
    QPushButton *button = (QPushButton *) sender();
    if(button == input_file_Btn)
    {
        file = QFileDialog :: getOpenFileName(NULL,tr("Select a file"),"D:\\","files(*.eswc *.swc *.apo)");
        file_edit->setText(file);
        qDebug()<<__LINE__<<" : "<<file.toStdString().c_str();
        if(file.isEmpty())
        {
            file_edit->setText("Select a file !");
            v3d_msg("File is empty ! Input again !");
        }
        files="";
        folder_edit->setText("Select a folder !");

    }
    else if(button == input_folder_Btn)
    {
        files = QFileDialog::getExistingDirectory(NULL, tr("Select a folder"),"D:\\");
        folder_edit->setText(files);
        if(files.isEmpty())
        {
            folder_edit->setText("Select a folder !");
            v3d_msg("Folder is empty ! Input again !");
        }
        file="";
        file_edit->setText("Select a file !");
    }
}

void DownsampleDialog:: saveFiles()
{
    QPushButton *button1 = (QPushButton *) sender();
    if(button1 == saveBtn)
    {
        if(file != "" && files == "")
        {
            QString file_tem = nameSaveFile(file);
            fileSave = QFileDialog :: getSaveFileName(0, tr("Select a file to save"), file_tem, tr("(.*).eswc.swc.apo"));
            files_save->setText(fileSave);
            if(fileSave.isEmpty())
            {
                files_save->setText("Select a file or folder to save files !");
                v3d_msg("Please input a file to save again !");
            }
        }
        else if(file == "" && files != "")
        {
            folderSave = QFileDialog :: getExistingDirectory(0, tr("Select a folder to save files"), files);
            files_save->setText(folderSave);
            if(folderSave.isEmpty())
            {
                files_save->setText("Select a file or folder to save files !");
                v3d_msg("Please input a folder to save again !");
            }
        }
        else if(file == "" && files == "")
        {
            v3d_msg("Please input files first !");
            //files_save->setText("Select a file or folder to save files !");
        }
    }
}

void DownsampleDialog:: downsampleEswc(const QString file, QString saveName, double down_x, double down_y, double down_z)
{
    qDebug()<<__LINE__<<" : "<<file.toStdString().c_str();
    NeuronTree nt = readSWC_file(file);
    qDebug()<<"nt size : "<<nt.listNeuron.size();
    for(V3DLONG i = 0; i < nt.listNeuron.size(); i ++)
    {
        nt.listNeuron[i].x /= down_x;
        nt.listNeuron[i].y /= down_y;
        nt.listNeuron[i].z /= down_z;
    }
    if(file.endsWith(".eswc"))
        writeESWC_file(saveName, nt);
    else if(file.endsWith(".swc"))
        writeSWC_file(saveName, nt);
}

void DownsampleDialog:: downsampleApo(const QString file, QString saveName, double down_x, double down_y, double down_z)
{
    QList<CellAPO> marker_apo = readAPO_file(file);
    qDebug()<<"apo size : "<<marker_apo.size();
    for(V3DLONG i = 0; i < marker_apo.size(); i ++)
    {
        marker_apo[i].x /= down_x;
        marker_apo[i].y /= down_y;
        marker_apo[i].z /= down_z;
    }
    writeAPO_file(saveName, marker_apo);
}

QString DownsampleDialog:: nameSaveFile(const QString file)
{
    QString file_tem_save;
    if(file.endsWith(".eswc"))
        file_tem_save = file + "_downsample.eswc";
    else if(file.endsWith(".swc"))
        file_tem_save = file + "_downsample.swc";
    else if(file.endsWith(".apo"))
        file_tem_save = file + "_downsample.apo";
    return file_tem_save;
}














