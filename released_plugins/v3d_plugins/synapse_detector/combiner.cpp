#include "combiner.h"
#include "map"

combiner::combiner(V3DPluginCallback2 *cb)
{
    //qDebug()<<"in combiner construction";
    create();
    check_button();
}

void combiner::create()
{
    //qDebug()<<"in create";
    QGridLayout *mygridLayout = new QGridLayout();
    QLabel* label_swc = new QLabel(QObject::tr("Load swc:"));
    mygridLayout->addWidget(label_swc,0,0,1,1);
    edit_swc = new QLineEdit("");
    //edit_swc->setText("");
    edit_swc->setReadOnly(true);
    mygridLayout->addWidget(edit_swc,0,1,1,6);
    btn_swc = new QPushButton("...");
    mygridLayout->addWidget(btn_swc,0,7,1,1);
    QLabel* label_spine = new QLabel(QObject::tr("Load csv from spineDetector:"));
    mygridLayout->addWidget(label_spine,1,0,1,1);
    edit_spine = new QLineEdit("");
    edit_spine->setReadOnly(true);
    mygridLayout->addWidget(edit_spine,1,1,1,6);
    btn_spine = new QPushButton("...");
    mygridLayout->addWidget(btn_spine,1,7,1,1);

    QLabel* label_is = new QLabel(QObject::tr("Load csv from ISquantifier:"));
    mygridLayout->addWidget(label_is,3,0,1,1);
    edit_is = new QLineEdit("");
    edit_is->setReadOnly(true);
    mygridLayout->addWidget(edit_is,3,1,1,6);
    btn_is = new QPushButton("...");
    mygridLayout->addWidget(btn_is,3,7,1,1);

    btn_run    = new QPushButton("Run");
    QPushButton *cancel = new QPushButton("Cancel");
    mygridLayout->addWidget(btn_run,6,1,1,2);
    mygridLayout->addWidget(cancel,6,4,1,2);

    QFrame *line_3 = new QFrame();
    line_3->setFrameShape(QFrame::HLine);
    line_3->setFrameShadow(QFrame::Sunken);
    mygridLayout->addWidget(line_3,7,0,1,8);
    QString info=">> Combiner maps all previously generated spine and IS csv tables to the original "
            " swc file and outputs an eswc file, prepring for stitching in the next phase.<br>";
    info+=">> It can perform on either spine tables/IS tables or both types.<br>";
    info+=">> Multiple csv files can be selected and added.<br>";
    info+=">> For further questions, please contact Yujie Li at yujie.jade@gmail.com)";

    QTextEdit *text_info = new QTextEdit;
    text_info->setText(info);
    text_info->setReadOnly(true);
    mygridLayout->addWidget(text_info,8,0,1,8);

    this->setLayout(mygridLayout);
    this->setWindowTitle("Combiner plug-in");
    this->show();

    //this is put here to make sure every time the load btn is hit, it goes back
    //to previous dialog window
    load_spine_dialog=new QDialog(this);
    load_spine_dialog->setWindowTitle("Load spine info table csv");
    load_spine_combo=new QListWidget();
    spine_layout = new QGridLayout();
    load_is_dialog=new QDialog();
    load_is_dialog->setWindowTitle("Load IS info table csv");
    load_is_combo=new QListWidget();
    is_layout = new QGridLayout();

    connect(btn_run,SIGNAL(clicked()), this, SLOT(run()));
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(btn_swc, SIGNAL(clicked()), this, SLOT(get_swc_name()));
    connect(btn_spine,SIGNAL(clicked()),this,SLOT(load_spine_panel()));
    connect(btn_is,SIGNAL(clicked()),this,SLOT(load_is_panel()));

    qDebug()<<"create done";
}

void combiner::load_spine_panel()
{
    //qDebug()<<"in load spine panel";

    QPushButton *add_btn = new QPushButton("Add file");
    QPushButton *rm_btn = new QPushButton("Remove file");

    load_spine_combo->setSelectionMode(QAbstractItemView::ExtendedSelection);
    load_spine_combo->setFocusPolicy(Qt::NoFocus);

    QPushButton *ok = new QPushButton("Ok");
    QPushButton *cancel = new QPushButton("Cancel");

    spine_layout->addWidget(add_btn,0,0,1,1);
    spine_layout->addWidget(rm_btn,0,1,1,1);
    spine_layout->addWidget(load_spine_combo,1,0,8,10);
    spine_layout->addWidget(ok,10,0,1,1);
    spine_layout->addWidget(cancel,10,1,1,1);
    load_spine_dialog->setLayout(spine_layout);
    load_spine_dialog->setMinimumSize(QSize(600,600));

    connect(add_btn,SIGNAL(clicked()),this,SLOT(get_spine_csv_name()));
    connect(rm_btn,SIGNAL(clicked()),this,SLOT(spine_rm_file()));
    connect(cancel, SIGNAL(clicked()), load_spine_dialog, SLOT(reject()));
    connect(ok, SIGNAL(clicked()),this, SLOT(spine_dialog_ok_clicked()));
    load_spine_dialog->exec();
    qDebug()<<"finish panel building";
}

void combiner::load_is_panel()
{
    //qDebug()<<"in load is panel";

    QPushButton *add_btn = new QPushButton("Add file");
    QPushButton *rm_btn = new QPushButton("Remove file");

    load_is_combo->setSelectionMode(QAbstractItemView::ExtendedSelection);
    load_is_combo->setFocusPolicy(Qt::NoFocus);

    QPushButton *ok = new QPushButton("Ok");
    QPushButton *cancel = new QPushButton("Cancel");

    is_layout->addWidget(add_btn,0,0,1,1);
    is_layout->addWidget(rm_btn,0,1,1,1);
    is_layout->addWidget(load_is_combo,1,0,8,10);
    is_layout->addWidget(ok,10,0,1,1);
    is_layout->addWidget(cancel,10,1,1,1);
    load_is_dialog->setLayout(is_layout);
    load_is_dialog->setMinimumSize(QSize(600,600));
    qDebug()<<"2";
    connect(add_btn,SIGNAL(clicked()),this,SLOT(get_is_csv_name()));
    connect(rm_btn,SIGNAL(clicked()),this,SLOT(is_rm_file()));
    connect(cancel, SIGNAL(clicked()), load_is_dialog, SLOT(reject()));
    connect(ok, SIGNAL(clicked()),this, SLOT(is_dialog_ok_clicked()));
    load_is_dialog->exec();
}

void combiner::spine_rm_file()
{
    QList<QListWidgetItem*> sel_list=load_spine_combo->selectedItems();
    for (int i=0;i<sel_list.size();i++)
    {
        load_spine_combo->takeItem(i);
    }
    //qDebug()<<"done removing"<<"size now:"<<load_spine_combo->size();
}

bool combiner::get_spine_csv_name()
{
    qDebug()<<"in get spine csv_name";
    //QString fileOpenName=this->edit_spine->text();
    QSettings MySettings("V3D plugin","combiner");
    spine_csv_name_list = QFileDialog::getOpenFileNames(0,0,MySettings.value("combiner_spine_csv").toString(),"Supported file (*.csv)" ";;",0,0);
    if(spine_csv_name_list.isEmpty())
        return false;
    for (int i=0;i<spine_csv_name_list.size();i++)
    {
        QString one_csv=spine_csv_name_list[i];
        if (one_csv.isEmpty())
            continue;
        else
        {
            //check for duplicates.
            QList<QListWidgetItem *> tmp_list;
            tmp_list = load_spine_combo->findItems(one_csv,Qt::MatchExactly);
            if (!tmp_list.empty())
            {
                v3d_msg("Repeated! This file is in the list");

                continue;
            }
            else
                load_spine_combo->addItem(one_csv);
        }
    }
    QDir filedir = QFileInfo(spine_csv_name_list[0]).absoluteDir();
    MySettings.setValue("combiner_spine_csv", filedir.absolutePath());
    //qDebug()<<"Current path:"<<filedir.absolutePath();
    //check_button();
    return true;
}

void combiner::spine_dialog_ok_clicked()
{
    //check number of files.
    //close window
    if (load_spine_combo->count()>0)
    {
        load_spine_dialog->close();
        edit_spine->setText(QString::number(load_spine_combo->count())+" csv file(s) selected");
        check_button();
    }
    else
    {
        v3d_msg("No files selected. Please select");
    }
}

void combiner::is_dialog_ok_clicked()
{
    //check number of files.
    //close window
    if (load_is_combo->count()>0)
    {
        load_is_dialog->close();
        edit_is->setText(QString::number(load_is_combo->count())+" csv file(s) selected");
        check_button();
    }
    else
    {
        v3d_msg("No files selected. Please select");
    }
}

bool combiner::spine_files_check()
{
    //starts to load files.
    for (int i=0;i<load_spine_combo->count();i++)
    {
        if (!load_spine_csv(load_spine_combo->item(i)->text())){
            v3d_msg("Loading SPINE csv error.\n" + load_spine_combo->item(i)->text()+"\nPlease check.");
            return false;
        }
    }
    return true;
}

bool combiner::is_files_check()
{
    //starts to load files.
    //qDebug()<<"in IS files check";
    for (int i=0;i<load_is_combo->count();i++)
    {
        if (!load_is_csv(load_is_combo->item(i)->text())){
            v3d_msg("Loading IS csv error.\n" + load_is_combo->item(i)->text() + "\nPlease check.");
            return false;
        }
    }
    return true;
}

void combiner::is_rm_file()
{
    QList<QListWidgetItem*> sel_list=load_is_combo->selectedItems();
    for (int i=0;i<sel_list.size();i++)
    {
        load_is_combo->takeItem(i);
    }
    //qDebug()<<"done removing"<<"size now:"<<load_is_combo->size();
}

bool combiner::get_is_csv_name()
{ 
    QSettings MySettings("V3D plugin","combiner");
    is_csv_name_list = QFileDialog::getOpenFileNames(0,0,MySettings.value("combiner_is_csv").toString(),"Supported file (*.csv)" ";;",0,0);
    if(is_csv_name_list.isEmpty())
        return false;
    for (int i=0;i<is_csv_name_list.size();i++)
    {
        QString one_csv=is_csv_name_list[i];
        if (one_csv.isEmpty())
            continue;
        else
        {
            //check for duplicates.
            QList<QListWidgetItem *> tmp_list;
            tmp_list = load_is_combo->findItems(one_csv,Qt::MatchExactly);
            if (!tmp_list.empty())
            {
                v3d_msg("Repeated! This file is in the list");
                continue;
            }
            else
                load_is_combo->addItem(one_csv);
        }
    }
    QDir filedir = QFileInfo(is_csv_name_list[0]).absoluteDir();
    MySettings.setValue("combiner_is_csv", filedir.absolutePath());
    //check_button();
    return true;
}

bool combiner::get_swc_name()
{
    QString fileOpenName=this->edit_swc->text();
    input_swc_name = QFileDialog::getOpenFileName(0, 0,fileOpenName,"Supported file (*.swc *.eswc)" ";;Neuron structure(*.swc *eswc)",0,0);

    if(input_swc_name.isEmpty())
    {
        //v3d_msg("No swc file loaded.");
        return false;
    }
    else{
        edit_swc->setText(input_swc_name);
        QSettings settings("V3D plugin","spine_detector");
        settings.setValue("combiner_swc",edit_swc->text());
        check_button();
        return true;
    }
}

bool combiner::check_button()
{
    if (edit_swc->text().isEmpty() ||
       (edit_spine->text().isEmpty() && edit_is->text().isEmpty()))
    {
        btn_run->setEnabled(false);
        return false;
    }else{
        btn_run->setEnabled(true);
        return true;
    }
}


bool combiner::load_swc()
{
    input_swc_name=edit_swc->text();
    //load swc
    NeuronSWC *p_cur=0;
    neuron = readSWC_file(input_swc_name);
    qDebug()<<"reading finished";
    //checking
    for (V3DLONG ii=0; ii<neuron.listNeuron.size(); ii++)
    {
        p_cur = (NeuronSWC *)(&(neuron.listNeuron.at(ii)));
        if (p_cur->r<0)
        {
            v3d_msg("You have illeagal radius values. Check your data.");
            return false;
        }
        p_cur->fea_val.clear();
        for (int i=0;i<7;i++) //first 5 for spine, last 2 for IS
        {
            p_cur->fea_val.push_back(-1.0);
        }
     }

    return true;
}

bool combiner::load_spine_csv(QString filename)
{
    //QList<spineunit> tmp_list;
    //qDebug()<<"in load spine csv";
    QFile qf(filename);
    if (! qf.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    V3DLONG k=0;
    int skip_line_num=0;
    while (! qf.atEnd())
    {
        char curline[2000];
        qf.readLine(curline, sizeof(curline));
        k++;

        if (curline[0]=='#') continue;
        QStringList qsl = QString(curline).trimmed().split(",");
        int qsl_count=qsl.size();
        if (qsl_count<=10)   {skip_line_num++;continue;}
        spineunit S;
        S.id=k;
        S.volume = qsl[2].toInt();
        S.max_dis = qsl[3].toInt();
        S.min_dis = qsl[4].toInt();
        S.center_dis = qsl[5].toInt();
        S.head_width = qsl[6].toFloat();
        S.nearest_node = qsl[10].toInt();

        if (nodegroup_map.find(S.nearest_node)==nodegroup_map.end()) //this node has not existed
        {
            QVector<spineunit> tmp_unit;
            tmp_unit.push_back(S);
            node_info one_node;
            one_node.spine_lut=tmp_unit;
            nodegroup_map.insert(S.nearest_node,one_node);
        }
        else
        {
            node_info exist_node = nodegroup_map.value(S.nearest_node);
            exist_node.spine_lut.push_back(S);
            nodegroup_map.insert(S.nearest_node,exist_node);
        }

    }
    if ((float)skip_line_num>0.6*k)
        return false;
    if (k<=1)
        return false;
    //qDebug()<<"in load_spine";
    return true;
}

bool combiner::load_is_csv(QString filename)
{
    //QList<isunit> tmp_list;
    //qDebug()<<"in load is csv";
    QFile qf(filename);
    if (! qf.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    V3DLONG k=0;
    V3DLONG skip_line_num=0;
    while (! qf.atEnd())
    {
        char curline[2000];
        qf.readLine(curline, sizeof(curline));
        k++;
        if (curline[0]=='#') continue;
        QStringList qsl = QString(curline).trimmed().split(",");
        int qsl_count=qsl.size();
        if (qsl_count!=7)   {skip_line_num++; continue;}
        isunit S;
        S.id=k;
        S.volume = qsl[1].toInt();
        S.nearest_node = qsl[5].toInt();
        S.on_dendrite=(bool)qsl[6].toInt();

        if (nodegroup_map.find(S.nearest_node)==nodegroup_map.end()) //this node has not existed
        {
            node_info new_node;
            new_node.is_lut.push_back(S);
            nodegroup_map.insert(S.nearest_node,new_node);
        }
        else
        {
            node_info exist_node=nodegroup_map.value(S.nearest_node);
            exist_node.is_lut.push_back(S);
            nodegroup_map.insert(S.nearest_node,exist_node);
        }
        //tmp_list.append(S);

    }
    if (k<=1)
        return false;
    if ((float)skip_line_num>0.8*k)
        return false;
    //qDebug()<<"in load_isv";
    return true;
}

void combiner::run()
{
    if(!check_button()){
        v3d_msg("You have not provided valid input");
        return;
    }
    this->accept();
    qDebug()<<"load and initialize";

    if (!load_swc())
    {
        v3d_msg("Loading swc error");
        return;
    }
    qDebug()<<"swc loaded";

    if (!spine_files_check()){
        //v3d_msg("Loading spine csv error");
        return;
    }

    qDebug()<<"spine csv finished";

    if (!is_files_check())
    {
        //v3d_msg("Loading IS csv error");
        return;
    }

    qDebug()<<"is csv finished";
//    for (QMap<int,node_info>::iterator iter_map=nodegroup_map.begin();
//         iter_map!=nodegroup_map.end();iter_map++)
//    {
//        qDebug()<<"key:"<<iter_map.key()<<" spine:"<<iter_map.value().spine_lut.size()<<
//                  " IS:"<<iter_map.value().is_lut.size();
//    }
    eswc_generate();
    return;
}


bool combiner::eswc_generate()
{
    //qDebug()<<"in eswc generate";
    //1-5 spine. 1:volume; 2:max_dis; 3:min_dis; 4:center_dis; 5:head-width
    //6-7 IS. 6:volume; 2:on_dendrite

    for (QMap<int,node_info>::iterator iter_map=nodegroup_map.begin();
         iter_map!=nodegroup_map.end();iter_map++)
    {
//        qDebug()<<"key:"<<iter_map.key()<<" spine:"<<iter_map.value().spine_lut.size()<<
//                  " IS:"<<iter_map.value().is_lut.size();
        int node_name = iter_map.key();
        int node_id=neuron.hashNeuron.value(node_name);
        QVector<spineunit> spine_vec = iter_map.value().spine_lut;
        QVector<isunit> is_vec = iter_map.value().is_lut;
//        if (node_id==0)
//            qDebug()<<"node id 0, key:"<<iter_map.key()<<"spine:"<<spine_vec.size()<<" is:"<<is_vec.size();
//        if (node_id==1)
//            qDebug()<<"node id 1, key:"<<iter_map.key()<<"spine:"<<spine_vec.size()<<" is:"<<is_vec.size();
        //needs to first check if the node_id goes beyond the swc
        //if true, then the swc and the table is not the right match
        if (node_id>neuron.listNeuron.size())
        {
            v3d_msg("SWC and tables do not match. Please check inputs.\n");
            return false;
        }

        //fill in the original node
        if (!spine_vec.isEmpty())
        {
            //take out the first spine
            spineunit first_spine = spine_vec.front();
            neuron.listNeuron[node_id].fea_val[0] = (float)first_spine.volume;
            neuron.listNeuron[node_id].fea_val[1] = (float)first_spine.max_dis;
            neuron.listNeuron[node_id].fea_val[2] = (float)first_spine.min_dis;
            neuron.listNeuron[node_id].fea_val[3] = (float)first_spine.center_dis;
            neuron.listNeuron[node_id].fea_val[4] = (float)first_spine.head_width;
            neuron.listNeuron[node_id].fea_val[5] = -1;
            neuron.listNeuron[node_id].fea_val[6] = -1;
            //pop out this spine
            spine_vec.pop_front();
        }
        if (!is_vec.isEmpty())
        {
            isunit first_is = is_vec.front();
            neuron.listNeuron[node_id].fea_val[5] = first_is.volume;
            neuron.listNeuron[node_id].fea_val[6] = first_is.on_dendrite;
            is_vec.pop_front();
        }
        //start creating new nodes
        while (!(is_vec.isEmpty() && spine_vec.isEmpty()))
        {
            NeuronSWC S;
            S.n = neuron.listNeuron.length()+1;
//            qDebug()<<"~~~~~~~~~~~~~New node add:"<<S.n<<"ori node: "<<neuron.listNeuron[node_id].n
//                   <<" ori node parent:"<<neuron.listNeuron[node_id].parent;
            S.type = neuron.listNeuron[node_id].type;
            S.x = neuron.listNeuron[node_id].x;
            S.y = neuron.listNeuron[node_id].y;
            S.z = neuron.listNeuron[node_id].z;
            S.r = neuron.listNeuron[node_id].r;
            S.parent = neuron.listNeuron[node_id].parent;
            S.fea_val.clear();
            for (int jj=0;jj<7;jj++)
                S.fea_val.push_back(-1.0);
            if (!spine_vec.isEmpty())
            {
                S.fea_val[0] = (float)spine_vec.front().volume;
                S.fea_val[1] = (float)spine_vec.front().max_dis;
                S.fea_val[2] = (float)spine_vec.front().min_dis;
                S.fea_val[3] = (float)spine_vec.front().center_dis;
                S.fea_val[4] = (float)spine_vec.front().head_width;
                spine_vec.pop_front();
            }
            if (!is_vec.isEmpty())
            {
                S.fea_val[5] = (float)is_vec.front().volume;
                S.fea_val[6] = (float)is_vec.front().on_dendrite;
                is_vec.pop_front();
            }
            neuron.listNeuron[node_id].parent=S.n;  //change the ori node's parent to this new node
            neuron.listNeuron.append(S);
        }
    }

    QString eswc_out_name="";
    if (!writeESWC_file(eswc_out_name,neuron))
        return false;
    //qDebug()<<"voxel_group:"<<neuron.listNeuron.size()<<"finish writing out"<<endl;
    QMessageBox::information(0,"combiner plug-in","File has been succesfully saved to the specified folder.",
                             QMessageBox::Ok);
    return true;
}




