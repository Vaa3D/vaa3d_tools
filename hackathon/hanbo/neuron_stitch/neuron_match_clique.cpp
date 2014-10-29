/****************************************************************************
**
** neuron_match_clique.cpp
** by Hanbo Chen 2014.10.16
**
****************************************************************************/

#include "neuron_match_clique.h"
#include "../../../v3d_main/neuron_editing/neuron_xforms.h"
#include <iostream>

#define CANDMS_ENTRY(a,b) (candMS[(a)+(b)*MS_x])


//NeuronMatchDialog::NeuronMatchDialog(V3DPluginCallback2 * cb, V3dR_MainWindow* inwin)
//{
//    if(!inwin)
//        return;
//    v3dwin=inwin;
//    callback = cb;

//    ntList=cb->getHandleNeuronTrees_Any3DViewer(v3dwin);
//    if(ntList->size()!=2){ //this should not happen
//        v3d_msg("The number of neurons in the window is not 2");
//        return;
//    }

//    creat();
//}

NeuronMatchDialog::NeuronMatchDialog()
{
    nt0 = nt1 = NULL;
    folder_output.clear();
    fname_output.clear();
    name_nt0.clear();
    name_nt1.clear();
    fname_nt0.clear();
    fname_nt1.clear();

    creat();
}

void NeuronMatchDialog::creat()
{
    gridLayout = new QGridLayout();

    //input zone
    label_load0 = new QLabel(QObject::tr("First Neuron for stitch (smaller in stacking direction):"));
    gridLayout->addWidget(label_load0,0,0,1,6);
    edit_load0 = new QLineEdit();
    edit_load0->setText(fname_nt0); edit_load0->setReadOnly(true);
    gridLayout->addWidget(edit_load0,1,0,1,5);
    btn_load0 = new QPushButton("...");
    gridLayout->addWidget(btn_load0,1,5,1,1);

    label_load1 = new QLabel(QObject::tr("Second Neuron for stitch (larger in stacking direction):"));
    gridLayout->addWidget(label_load1,2,0,1,6);
    edit_load1 = new QLineEdit();
    edit_load1->setText(fname_nt1); edit_load1->setReadOnly(true);
    gridLayout->addWidget(edit_load1,3,0,1,5);
    btn_load1 = new QPushButton("...");
    gridLayout->addWidget(btn_load1,3,5,1,1);

    label_output = new QLabel(QObject::tr("Output Direction"));
    gridLayout->addWidget(label_output,4,0,1,6);
    edit_output = new QLineEdit();
    edit_output->setText(fname_output);
    gridLayout->addWidget(edit_output,5,0,1,5);
    btn_output = new QPushButton("...");
    gridLayout->addWidget(btn_output,5,5,1,1);

    connect(btn_load0, SIGNAL(clicked()), this, SLOT(load0()));
    connect(btn_load1, SIGNAL(clicked()), this, SLOT(load1()));
    connect(btn_output, SIGNAL(clicked()), this, SLOT(output()));
    connect(edit_output, SIGNAL(textChanged(QString)), this, SLOT(outputchange(QString)));

    //parameter zone
    QFrame *line_1 = new QFrame();
    line_1->setFrameShape(QFrame::HLine);
    line_1->setFrameShadow(QFrame::Sunken);
    gridLayout->addWidget(line_1,8,0,1,6);

    cb_dir = new QComboBox(); cb_dir->addItem("x"); cb_dir->addItem("y"); cb_dir->addItem("z");
    cb_dir->setCurrentIndex(2);
    spin_zscale = new QDoubleSpinBox();
    spin_zscale->setRange(0,100000); spin_zscale->setValue(1);
    spin_ang = new QDoubleSpinBox();
    spin_ang->setRange(0,180); spin_ang->setValue(60);
    spin_matchdis = new QDoubleSpinBox();
    spin_matchdis->setRange(0,100000); spin_matchdis->setValue(100);
    spin_searchspan = new QDoubleSpinBox();
    spin_searchspan->setRange(0,100000); spin_searchspan->setValue(20);
    spin_cmatchdis = new QDoubleSpinBox();
    spin_cmatchdis->setRange(0,100000); spin_cmatchdis->setValue(100);
    spin_segthr = new QDoubleSpinBox();
    spin_segthr->setRange(0,100000); spin_segthr->setValue(0);
    QLabel* label_0 = new QLabel("stacking direction: ");
    gridLayout->addWidget(label_0,9,0,1,2);
    gridLayout->addWidget(cb_dir,9,2,1,1);
    QLabel* label_1 = new QLabel("resacle stacking direction: ");
    gridLayout->addWidget(label_1,9,3,1,2);
    gridLayout->addWidget(spin_zscale,9,5,1,1);
    QLabel* label_2 = new QLabel("Max angular to match points (0~180): ");
    gridLayout->addWidget(label_2,10,0,1,2);
    gridLayout->addWidget(spin_ang,10,2,1,1);
    QLabel* label_3 = new QLabel("Max distance to match points: ");
    gridLayout->addWidget(label_3,10,3,1,2);
    gridLayout->addWidget(spin_matchdis,10,5,1,1);
    QLabel* label_4 = new QLabel("match candidates searching span: ");
    gridLayout->addWidget(label_4,11,0,1,2);
    gridLayout->addWidget(spin_searchspan,11,2,1,1);
    QLabel* label_5 = new QLabel("Max distance to match 3-clique: ");
    gridLayout->addWidget(label_5,11,3,1,2);
    gridLayout->addWidget(spin_cmatchdis,11,5,1,1);
    QLabel* label_6 = new QLabel("small segment threshold (0=keep all): ");
    gridLayout->addWidget(label_6,12,0,1,2);
    gridLayout->addWidget(spin_segthr,12,2,1,1);

    //operation zone
    QFrame *line_2 = new QFrame();
    line_2->setFrameShape(QFrame::HLine);
    line_2->setFrameShadow(QFrame::Sunken);
    gridLayout->addWidget(line_2,15,0,1,6);
    check_stitch = new QCheckBox("Stitch Matched Points");
    gridLayout->addWidget(check_stitch,16,0,1,2);
    btn_run = new QPushButton("Run");
    connect(btn_run,     SIGNAL(clicked()), this, SLOT(run()));
    gridLayout->addWidget(btn_run,16,4,1,1);
    btn_quit = new QPushButton("Quit");
    connect(btn_quit,     SIGNAL(clicked()), this, SLOT(reject()));
    gridLayout->addWidget(btn_quit,16,5,1,1);

    btn_run->setEnabled(false);

    setLayout(gridLayout);
}

bool NeuronMatchDialog::load0()
{
    QString fileOpenName;
    fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
            "",
            QObject::tr("Supported file (*.swc *.eswc)"
                ";;Neuron structure	(*.swc)"
                ";;Extended neuron structure (*.eswc)"
                ));
    if(fileOpenName.isEmpty())
        return false;
    NeuronTree* nt = new NeuronTree();
    if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
    {
        *nt = readSWC_file(fileOpenName);
    }
    if(nt->listNeuron.size()<=0){
        v3d_msg("failed to read SWC file: "+fileOpenName);
        return false;
    }
    nt0 = nt;
    fname_nt0 = fileOpenName;
    name_nt0 = QFileInfo(fileOpenName).baseName();
    edit_load0->setText(fname_nt0);

    if(fname_nt0.length() * fname_nt1.length() != 0){
        fname_output = QDir(folder_output).filePath(name_nt1 + "_to_" + name_nt0);
    }

    QDir::setCurrent(QFileInfo(fileOpenName).absoluteFilePath());

    if(fname_nt0.length() * fname_nt1.length() * folder_output.length() != 0){
        btn_run->setEnabled(true);
        edit_output->setText(fname_output);
    }
}

bool NeuronMatchDialog::load1()
{
    QString fileOpenName;
    fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
            "",
            QObject::tr("Supported file (*.swc *.eswc)"
                ";;Neuron structure	(*.swc)"
                ";;Extended neuron structure (*.eswc)"
                ));
    if(fileOpenName.isEmpty())
        return false;
    NeuronTree* nt = new NeuronTree();
    if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
    {
        *nt = readSWC_file(fileOpenName);
    }
    if(nt->listNeuron.size()<=0){
        v3d_msg("failed to read SWC file: "+fileOpenName);
        return false;
    }
    nt1 = nt;
    fname_nt1 = fileOpenName;
    name_nt1 = QFileInfo(fileOpenName).baseName();
    edit_load1->setText(fname_nt1);

    if(fname_nt0.length() * fname_nt1.length() != 0){
        fname_output = QDir(folder_output).filePath(name_nt1 + "_to_" + name_nt0);
    }

    QDir::setCurrent(QFileInfo(fileOpenName).absoluteFilePath());

    if(fname_nt0.length() * fname_nt1.length() * folder_output.length() != 0){
        btn_run->setEnabled(true);
        edit_output->setText(fname_output);
    }
}

bool NeuronMatchDialog::output()
{
    QString fileSaveDir;
    fileSaveDir = QFileDialog::getExistingDirectory(0, QObject::tr("Select Directory to Save Results"),
            "~",QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(fileSaveDir.isEmpty())
        return false;

    folder_output = fileSaveDir;

    fname_output = QDir(folder_output).filePath(name_nt1 + "_to_" + name_nt0);
    edit_output->setText(fname_output);

    QDir::setCurrent(fileSaveDir);

    if(fname_nt0.length() * fname_nt1.length() * folder_output.length() != 0){
        btn_run->setEnabled(true);
    }
}

bool NeuronMatchDialog::outputchange(QString text)
{
    fname_output=text;
}

void NeuronMatchDialog::run()
{
    gridLayout->setEnabled(false);
    //init matching function
    neuron_match_clique matchfunc(nt0,nt1);

    //parameters
    matchfunc.direction=cb_dir->currentIndex();
    matchfunc.zscale=spin_zscale->value();
    matchfunc.angThr_match=cos(spin_ang->value()/180*M_PI);
    matchfunc.pmatchThr = spin_matchdis->value();
    matchfunc.spanCand = spin_searchspan->value();
    matchfunc.cmatchThr = spin_cmatchdis->value();
    matchfunc.segmentThr = spin_segthr->value();

    //init clique and candidate
    matchfunc.init();

    //global match
    qDebug()<<"start global search";
    matchfunc.globalmatch();

    //stitch if checked
    if(check_stitch->isChecked()){
        qDebug()<<"stitch matched points";
        matchfunc.stitch();
        matchfunc.output_stitch(fname_output);
    }

    //iterative local match
    matchfunc.output_candMatchScore(fname_output + "_matchscore.txt");
    matchfunc.output_affine(fname_output,fname_nt0);
    matchfunc.output_matchedMarkers_orgspace(QDir(folder_output).filePath(name_nt0 + "_matched.marker"),QDir(folder_output).filePath(name_nt1 + "_matched.marker"));
    matchfunc.output_parameter(fname_output+"_param.txt");

    gridLayout->setEnabled(true);
    v3d_msg("matching finished");
}

neuron_match_clique::neuron_match_clique(NeuronTree* botNeuron, NeuronTree* topNeuron)
{
    nt0_org=botNeuron;
    nt1_org=topNeuron;

    nt0=new NeuronTree;
    nt1=new NeuronTree;

    spanCand = 20;
    direction = 2;
    midplane = 0;
    angThr_match = cos(M_PI/3);
    angThr_stack = -1;
    dir_range = 100;
    cmatchThr = 100;
    pmatchThr = 100;
    zscale = 1;
    segmentThr = 0;
}

void neuron_match_clique::globalmatch()
{
    if(rankedCliqueMatch.size()<=0)
        return;

    candmatch0.clear();
    candmatch1.clear();

    //search for best unconflict groups of matched points
    double bestEnergy = -1;
    double better_shift_x=0,better_shift_y=0,better_shift_z=0,better_angle=0,better_cent_x=0,better_cent_y=0,better_cent_z=0;

    multimap<int, QVector<int> , std::greater<int> >::iterator iter_conflict=rankedCliqueMatch.begin();
    //try 10 times, accept best one if no better one can be found with current trial
    for(int trial=0; trial<candID0.size()+candID1.size(); trial++){
        if(iter_conflict==rankedCliqueMatch.end()) break;
        //construct in-conflict matching pairs
        QList<int> tmpmatch0, tmpmatch1;
        int c0 = iter_conflict->second.at(0);//.value().at(0);
        int c1 = iter_conflict->second.at(1);
        tmpmatch0.append(cliqueList0.at(c0).idx[0]);
        tmpmatch0.append(cliqueList0.at(c0).idx[1]);
        tmpmatch0.append(cliqueList0.at(c0).idx[2]);
        tmpmatch1.append(cliqueList1.at(c1).idx[0]);
        tmpmatch1.append(cliqueList1.at(c1).idx[1]);
        tmpmatch1.append(cliqueList1.at(c1).idx[2]);
        iter_conflict++;
//This method will push inconflict matching point based on clique match as much as possible first.
//Marked this method because it will be unstable and cause error result
//        int reachConflict = 0; //not reached
//        for(multimap<int, QVector<int> , std::greater<int> >::iterator iter_cpair=rankedCliqueMatch.begin(); iter_cpair != rankedCliqueMatch.end(); iter_cpair++){
//            if(iter_cpair == iter_conflict){
//                reachConflict = 1; //reached, but no new confliction so far
//                continue;
//            }
//            c0=iter_cpair->second.at(0);
//            c1=iter_cpair->second.at(1);
//            //check confliction
//            bool conflict = false;
//            if(!conflict && tmpmatch0.contains(cliqueList0.at(c0).idx[0])){
//                if(tmpmatch1.at(tmpmatch0.indexOf(cliqueList0.at(c0).idx[0])) != cliqueList1.at(c1).idx[0])
//                    conflict = true;
//            }else if(!conflict && tmpmatch1.contains(cliqueList1.at(c1).idx[0])){
//                conflict = true;
//            }
//            if(!conflict && tmpmatch0.contains(cliqueList0.at(c0).idx[1])){
//                if(tmpmatch1.at(tmpmatch0.indexOf(cliqueList0.at(c0).idx[1])) != cliqueList1.at(c1).idx[1])
//                    conflict = true;
//            }else if(!conflict && tmpmatch1.contains(cliqueList1.at(c1).idx[1])){
//                conflict = true;
//            }
//            if(!conflict && tmpmatch0.contains(cliqueList0.at(c0).idx[2])){
//                if(tmpmatch1.at(tmpmatch0.indexOf(cliqueList0.at(c0).idx[2])) != cliqueList1.at(c1).idx[2])
//                    conflict = true;
//            }else if(!conflict && tmpmatch1.contains(cliqueList1.at(c1).idx[2])){
//                conflict = true;
//            }
//            if(!conflict){
//                if(!tmpmatch0.contains(cliqueList0.at(c0).idx[0])){
//                    tmpmatch0.append(cliqueList0.at(c0).idx[0]);
//                    tmpmatch1.append(cliqueList1.at(c1).idx[0]);
//                }
//                if(!tmpmatch0.contains(cliqueList0.at(c0).idx[1])){
//                    tmpmatch0.append(cliqueList0.at(c0).idx[1]);
//                    tmpmatch1.append(cliqueList1.at(c1).idx[1]);
//                }
//                if(!tmpmatch0.contains(cliqueList0.at(c0).idx[2])){
//                    tmpmatch0.append(cliqueList0.at(c0).idx[2]);
//                    tmpmatch1.append(cliqueList1.at(c1).idx[2]);
//                }
//            }
//            if(conflict && reachConflict == 1){
//                iter_conflict = iter_cpair;
//                reachConflict = 2; //reached and new one has been identified
//            }
//        }
        cout<<"matched points: "<<trial<<":";

        //perform affine transform
        while(1){
            cout<<":"<<tmpmatch0.size();

            double tmp_shift_x=0,tmp_shift_y=0,tmp_shift_z=0,tmp_angle=0,tmp_cent_x=0,tmp_cent_y=0,tmp_cent_z=0;
            if(direction==0) tmp_shift_x=-1;
            if(direction==1) tmp_shift_y=-1;
            if(direction==2) tmp_shift_z=-1;
            QList<XYZ> c0,c1,tmpcoord, tmpdir;
            for(int i=0; i<tmpmatch0.size(); i++){
                c0.append(XYZ(candcoord0.at(tmpmatch0.at(i))));
                c1.append(XYZ(candcoord1.at(tmpmatch1.at(i))));
            }
            if(!compute_affine_4dof(c0,c1,tmp_shift_x,tmp_shift_y,tmp_shift_z,tmp_angle,tmp_cent_x,tmp_cent_y,tmp_cent_z,direction)){
                break;
            }
            affine_XYZList(candcoord1, tmpcoord, tmp_shift_x, tmp_shift_y, tmp_shift_z, tmp_angle, tmp_cent_x, tmp_cent_y, tmp_cent_z, direction);
            affine_XYZList(canddircoord1, tmpdir, tmp_shift_x, tmp_shift_y, tmp_shift_z, tmp_angle, tmp_cent_x, tmp_cent_y, tmp_cent_z, direction);
            minus_XYZList(tmpdir,tmpcoord,tmpdir);
            QList<int> tmpMatchMarkers[2];
            tmpMatchMarkers[0]=QList<int>();
            tmpMatchMarkers[1]=QList<int>();
            getMatchPairs_XYZList(candcoord0, tmpcoord, canddir0, tmpdir, candcomponents0, candcomponents1, tmpMatchMarkers, pmatchThr, angThr_match);
            if(tmpMatchMarkers[0].size()>tmpmatch0.size()){
                tmpmatch0=tmpMatchMarkers[0];
                tmpmatch1=tmpMatchMarkers[1];
                better_shift_x=tmp_shift_x;
                better_shift_y=tmp_shift_y;
                better_shift_z=tmp_shift_z;
                better_angle=tmp_angle;
                better_cent_x=tmp_cent_x;
                better_cent_y=tmp_cent_y;
                better_cent_z=tmp_cent_z;
            }else{
                break;
            }
        }
        cout<<endl;

        if(tmpmatch0.size()>bestEnergy){
            bestEnergy = tmpmatch0.size();
            candmatch0 = tmpmatch0;
            candmatch1 = tmpmatch1;
            shift_x = better_shift_x;
            shift_y = better_shift_y;
            shift_z = better_shift_z;
            rotation_ang = better_angle;
            rotation_cx = better_cent_x;
            rotation_cy = better_cent_y;
            rotation_cz = better_cent_z;
            trial=0; //try another 10 times
        }
    }

    pmatch0.clear();
    pmatch1.clear();
    for(int i=0; i<candmatch0.size(); i++){
        pmatch0.append(candID0.at(candmatch0.at(i)));
        pmatch1.append(candID1.at(candmatch1.at(i)));
    }
    qDebug()<<"global match: "<<pmatch0.size()<<" matched points found";

    //affine based on matched points
    affine_nt1();
}

void neuron_match_clique::init()
{
    qDebug()<<"rescale in stacking direction";
    //backup neurons
    backupNeuron(*nt0_org, *nt0);
    backupNeuron(*nt1_org, *nt1);
    if(zscale!=1){
        if(direction == 0){
            proc_neuron_multiply_factor(nt0, zscale, 1, 1);
            proc_neuron_multiply_factor(nt1, zscale, 1, 1);
        }else if(direction == 1){
            proc_neuron_multiply_factor(nt0, 1, zscale, 1);
            proc_neuron_multiply_factor(nt1, 1, zscale, 1);
        }else{
            proc_neuron_multiply_factor(nt0, 1, 1, zscale);
            proc_neuron_multiply_factor(nt1, 1, 1, zscale);
        }
    }

    qDebug()<<"align in stacking direction";
    if(direction==0)
        shift_x=quickMoveNeuron(nt0, nt1, direction);
    else if(direction==1)
        shift_y=quickMoveNeuron(nt0, nt1, direction);
    else
        shift_z=quickMoveNeuron(nt0, nt1, direction);

    midplane=getNeuronTreeMidplane((*nt0), (*nt1), direction);    

    //find candidates
    qDebug()<<"search candidates";
    initNeuronAndCandidate(*nt0,candID0,candcoord0,canddir0,canddircoord0,components0,candcomponents0,parent0,1);
    initNeuronAndCandidate(*nt1,candID1,candcoord1,canddir1,canddircoord1,components1,candcomponents1,parent1,-1);
    qDebug()<<"init neuron 0: cand:"<<candID0.size();
    qDebug()<<"init neuron 1: cand:"<<candID1.size();

    //find cliques
    qDebug()<<"start find cliques";
    getCliques(candID0, candcoord0, canddir0, cliqueList0, cmatchThr, direction);
    getCliques(candID1, candcoord1, canddir1, cliqueList1, cmatchThr, direction);
    qDebug()<<"init neuron 0: 3clique:"<<cliqueList0.size();
    qDebug()<<"init neuron 1: 3clique:"<<cliqueList1.size();

    qDebug()<<"start match cliques";

    //find the matched cliques
    matchCliquesAndCands();
}

void neuron_match_clique::output_parameter(QString fname)
{

    QFile file(fname);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text)){
        v3d_msg("cannot open "+ fname +" for write");
        return;
    }
    QTextStream myfile(&file);
    myfile<<"stack_direction(0:x,1:y,2:z)= "<<direction<<endl;  //stack direction, 0x, 1y, 2z
    myfile<<"candidate_search_plane= "<<midplane<<endl;    //coordinate of the plane in between
    myfile<<"candidate_search_threshold_distance_to_plane= "<<spanCand<<endl;  //searching span from the stack plane for the candidate
    myfile<<"candidate_search_threshold_angle_to_plane(cos(a))= "<<angThr_stack<<endl; //angular threshold for candidate in stack direction (-1 ~ 1), -1 means nothing will be thresholded
    myfile<<"candidate_search_fragments_threshold= "<<segmentThr<<endl;  //threshold to filter out small segments when selecting candidates
    myfile<<"point_match_threshold_distance= "<<pmatchThr<<endl;   //match threshold for points
    myfile<<"point_match_threshold_angular(cos(a))= "<<angThr_match<<endl; //angular threshold for clique match (-1 ~ 1)=cos(theta), when angle is larger than theta (<cos(theta)), the point will not be matched
    myfile<<"clique_match_threshold_distance= "<<cmatchThr<<endl;   //match threshold for length of cliques
    myfile<<"stack_direction_rescale= "<<zscale<<endl;  //resacle stack direction
    myfile<<"segment_length_for_calculate_direction= "<<dir_range<<endl;   //length of section used to calculate direction of dead end

    file.close();
}

void neuron_match_clique::output_candMatchScore(QString fname)
{
    QList<int> xs, ys;
    for(int i=0; i<candmatch0.size(); i++){
        xs.append(candmatch0.at(i));
        ys.append(candmatch1.at(i));
    }
    for(int i=0; i<candID0.size(); i++){
        if(!xs.contains(i))
            xs.append(i);
    }
    for(int i=0; i<candID1.size(); i++){
        if(!ys.contains(i))
            ys.append(i);
    }

    QFile file(fname);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text)){
        v3d_msg("cannot open "+ fname +" for write");
        return;
    }
    QTextStream myfile(&file);
    myfile<<"0\t";
    for(int j=0; j<candmatch0.size(); j++){
        myfile<<j*2+2<<"\t";
    }
    for(int j=candmatch0.size()+MS_x; j<MS_x+MS_y; j++){
        myfile<<j+1<<"\t";
    }
    myfile<<endl;
    for(int i=0; i<xs.size(); i++){
        if(i<candmatch0.size())
            myfile<<i*2+1<<"\t";
        else
            myfile<<i+candmatch0.size()+1<<"\t";
        for(int j=0; j<ys.size(); j++){
            myfile<<CANDMS_ENTRY(xs.at(i),ys.at(j))<<"\t";
        }
        myfile<<endl;
    }
    file.close();
}

void neuron_match_clique::output_matchedMarkers(QString fname, const NeuronTree& nt, QList<int> points)
{
    QFile file(fname);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text)){
        v3d_msg("cannot open "+ fname +" for write");
        return;
    }
    QTextStream myfile(&file);
    myfile<<"##x,y,z,radius,shape,name,comment, color_r,color_g,color_b"<<endl;
    int idx=0;
    //matched candidates first
    for(int i=0; i<points.size(); i++){
        int a=points.at(i);
        myfile<<nt.listNeuron[a].x<<","<<nt.listNeuron[a].y<<","<<nt.listNeuron[a].z<<",0,1, "<<idx++<<", "
             <<", 255, 0, 0"<<endl;
    }
    file.close();
}

void neuron_match_clique::output_matchedMarkers_orgspace(QString fname_0, QString fname_1)
{
    QFile file(fname_0);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text)){
        v3d_msg("cannot open "+ fname_0 +" for write");
        return;
    }
    QTextStream myfile(&file);
    myfile<<"##x,y,z,radius,shape,name,comment, color_r,color_g,color_b"<<endl;
    int idx=0;
    //matched candidates first
    for(int i=0; i<pmatch0.size(); i++){
        int a=pmatch0.at(i);
        int aa=candmatch0.at(i);
        myfile<<nt0_org->listNeuron[a].x<<","<<nt0_org->listNeuron[a].y<<","<<nt0_org->listNeuron[a].z<<",0,1, "<<idx++<<","
             <<"0 "<<candID0.at(aa)<<" -1 "
             <<canddir0[aa].x<<" "<<canddir0[aa].y<<" "<<canddir0[aa].z<<", 255, 0, 0"<<endl;
    }
    file.close();

    QFile file1(fname_1);
    if (!file1.open(QIODevice::WriteOnly|QIODevice::Text)){
        v3d_msg("cannot open "+ fname_1 +" for write");
        return;
    }
    QTextStream myfile1(&file1);
    myfile1<<"##x,y,z,radius,shape,name,comment, color_r,color_g,color_b"<<endl;
    idx=0;
    for(int i=0; i<pmatch0.size(); i++){
        int b=pmatch1.at(i);
        int bb=candmatch1.at(i);
        myfile1<<nt1_org->listNeuron[b].x<<","<<nt1_org->listNeuron[b].y<<","<<nt1_org->listNeuron[b].z<<",0,1, "<<idx++<<","
             <<"1 "<<candID1.at(bb)<<" -1 "
             <<canddir1[bb].x<<" "<<canddir1[bb].y<<" "<<canddir1[bb].z<<", 0, 255, 0"<<endl;
    }
    file1.close();
}

void neuron_match_clique::output_markers_candspace(QString fname)
{
    QFile file(fname);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text)){
        v3d_msg("cannot open "+ fname +" for write");
        return;
    }
    QTextStream myfile(&file);
    myfile<<"##x,y,z,radius,shape,name,comment, color_r,color_g,color_b"<<endl;
    int idx=0;
    //matched candidates first
    for(int i=0; i<candmatch0.size(); i++){
        int a=candmatch0.at(i);
        myfile<<candcoord0[a].x<<","<<candcoord0[a].y<<","<<candcoord0[a].z<<",0,1, "<<idx++<<","
             <<"0 "<<candID0.at(a)<<" "<<idx<<" "
             <<canddir0[a].x<<" "<<canddir0[a].y<<" "<<canddir0[a].z<<", 255, 0, 0"<<endl;

        int b=candmatch1.at(i);
        myfile<<candcoord1[b].x<<","<<candcoord1[b].y<<","<<candcoord1[b].z<<",0,1, "<<idx++<<","
             <<"1 "<<candID1.at(b)<<" "<<idx-1<<" "
             <<canddir1[b].x<<" "<<canddir1[b].y<<" "<<canddir1[b].z<<", 0, 255, 0"<<endl;
    }
    //then unmatched
    for(int i=0; i<candID0.size(); i++){
        if(candmatch0.contains(i)) continue;
        myfile<<candcoord0[i].x<<","<<candcoord0[i].y<<","<<candcoord0[i].z<<",0,1, "<<idx++<<","
             <<"0 "<<candID0.at(i)<<" -1 "
             <<canddir0[i].x<<" "<<canddir0[i].y<<" "<<canddir0[i].z<<", 128, 0, 128"<<endl;
    }
    for(int i=0; i<candID1.size(); i++){
        if(candmatch1.contains(i)) continue;
        myfile<<candcoord1[i].x<<","<<candcoord1[i].y<<","<<candcoord1[i].z<<",0,1, "<<idx++<<","
             <<"1 "<<candID1.at(i)<<" -1 "
             <<canddir1[i].x<<" "<<canddir1[i].y<<" "<<canddir1[i].z<<", 0, 128, 128"<<endl;
    }
    file.close();
}

void neuron_match_clique::output_markers_orgspace(QString fname)
{
    QFile file(fname);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text)){
        v3d_msg("cannot open "+ fname +" for write");
        return;
    }
    QTextStream myfile(&file);
    myfile<<"##x,y,z,radius,shape,name,comment, color_r,color_g,color_b"<<endl;
    int idx=0;
    //matched candidates first
    for(int i=0; i<pmatch0.size(); i++){
        int a=pmatch0.at(i);
        int aa=candmatch0.at(i);
        myfile<<nt0_org->listNeuron[a].x<<","<<nt0_org->listNeuron[a].y<<","<<nt0_org->listNeuron[a].z<<",0,1, "<<idx++<<","
             <<"0 "<<candID0.at(aa)<<" "<<idx<<" "
             <<canddir0[aa].x<<" "<<canddir0[aa].y<<" "<<canddir0[aa].z<<", 255, 0, 0"<<endl;

        int b=pmatch1.at(i);
        int bb=candmatch1.at(i);
        myfile<<nt1_org->listNeuron[b].x<<","<<nt1_org->listNeuron[b].y<<","<<nt1_org->listNeuron[b].z<<",0,1, "<<idx++<<","
             <<"1 "<<candID1.at(bb)<<" "<<idx-1<<" "
             <<canddir1[bb].x<<" "<<canddir1[bb].y<<" "<<canddir1[bb].z<<", 0, 255, 0"<<endl;
    }
    //then unmatched
    for(int i=0; i<candID0.size(); i++){
        if(candmatch0.contains(i)) continue;
        myfile<<nt0_org->listNeuron[candID0.at(i)].x<<","<<nt0_org->listNeuron[candID0.at(i)].y<<","<<nt0_org->listNeuron[candID0.at(i)].z<<",0,1, "<<idx++<<","
             <<"0 "<<candID0.at(i)<<" -1 "
             <<canddir0[i].x<<" "<<canddir0[i].y<<" "<<canddir0[i].z<<", 128, 0, 128"<<endl;
    }
    for(int i=0; i<candID1.size(); i++){
        if(candmatch1.contains(i)) continue;
        myfile<<nt1_org->listNeuron[candID1.at(i)].x<<","<<nt1_org->listNeuron[candID1.at(i)].y<<","<<nt1_org->listNeuron[candID1.at(i)].z<<",0,1, "<<idx++<<","
             <<"1 "<<candID1.at(i)<<" -1 "
             <<canddir1[i].x<<" "<<canddir1[i].y<<" "<<canddir1[i].z<<", 0, 128, 128"<<endl;
    }
    file.close();
}

void neuron_match_clique::output_markers_affinespace(QString fname)
{
    QFile file(fname);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text)){
        v3d_msg("cannot open "+ fname +" for write");
        return;
    }
    QTextStream myfile(&file);
    myfile<<"##x,y,z,radius,shape,name,comment, color_r,color_g,color_b"<<endl;
    int idx=0;
    //matched candidates first
    for(int i=0; i<pmatch0.size(); i++){
        int a=pmatch0.at(i);
        int aa=candmatch0.at(i);
        myfile<<nt0_org->listNeuron[a].x<<","<<nt0_org->listNeuron[a].y<<","<<nt0_org->listNeuron[a].z<<",0,1, "<<idx++<<","
             <<"0 "<<candID0.at(aa)<<" "<<idx<<" "
             <<canddir0[aa].x<<" "<<canddir0[aa].y<<" "<<canddir0[aa].z<<", 255, 0, 0"<<endl;

        int b=pmatch1.at(i);
        int bb=candmatch1.at(i);
        myfile<<nt1_a.listNeuron[b].x<<","<<nt1_a.listNeuron[b].y<<","<<nt1_a.listNeuron[b].z<<",0,1, "<<idx++<<","
             <<"1 "<<candID1.at(bb)<<" "<<idx-1<<" "
             <<canddir1[bb].x<<" "<<canddir1[bb].y<<" "<<canddir1[bb].z<<", 0, 255, 0"<<endl;
    }
    //then unmatched
    for(int i=0; i<candID0.size(); i++){
        if(candmatch0.contains(i)) continue;
        myfile<<nt0_org->listNeuron[candID0.at(i)].x<<","<<nt0_org->listNeuron[candID0.at(i)].y<<","<<nt0_org->listNeuron[candID0.at(i)].z<<",0,1, "<<idx++<<","
             <<"0 "<<candID0.at(i)<<" -1 "
             <<canddir0[i].x<<" "<<canddir0[i].y<<" "<<canddir0[i].z<<", 128, 0, 128"<<endl;
    }
    for(int i=0; i<candID1.size(); i++){
        if(candmatch1.contains(i)) continue;
        myfile<<nt1_a.listNeuron[candID1.at(i)].x<<","<<nt1_a.listNeuron[candID1.at(i)].y<<","<<nt1_a.listNeuron[candID1.at(i)].z<<",0,1, "<<idx++<<","
             <<"1 "<<candID1.at(i)<<" -1 "
             <<canddir1[i].x<<" "<<canddir1[i].y<<" "<<canddir1[i].z<<", 0, 128, 128"<<endl;
    }
    file.close();
}

void neuron_match_clique::output_stitch(QString fname)
{
    if(candmatch0.size()<=0)
        return;
    if(nt0_stitch.listNeuron.size()<=0)
        return;
    if(nt1_stitch.listNeuron.size()<=0)
        return;

    QString fname_neuron0 = fname+"_stithced_nt0.swc";
    //output neuron
    if (!export_list2file(nt0_stitch.listNeuron,fname_neuron0,nt0_org->name))
    {
        v3d_msg("fail to write the output swc file:\n" + fname_neuron0);
        return;
    }

    QString fname_neuron1 = fname+"_stithced_nt1.swc";
    //output neuron
    if (!export_list2file(nt1_stitch.listNeuron,fname_neuron1,nt1_org->name))
    {
        v3d_msg("fail to write the output swc file:\n" + fname_neuron1);
        return;
    }

    //output markers
    QString fname_marker = fname+"_stitched.marker";
    output_matchedMarkers(fname_marker, nt0_stitch, pmatch0);

    //output ano
    QString fname_ano = fname+"_stitched.ano";
    QFile qf_anofile(fname_ano);
    if(!qf_anofile.open(QIODevice::WriteOnly))
    {
        v3d_msg("Cannot open file for writing!");
        return;
    }
    QTextStream out(&qf_anofile);
    out<<"SWCFILE="<<fname_neuron0<<endl;
    out<<"SWCFILE="<<fname_neuron1<<endl;
    qf_anofile.close();
}

void neuron_match_clique::output_affine(QString fname, QString fname_nt0)
{
    if(candmatch0.size()<=0)
        return;

    QString fname_neuron = fname+"_affine.swc";
    //output neuron
    if (!export_list2file(nt1_a.listNeuron,fname_neuron,nt1_org->name))
    {
        v3d_msg("fail to write the output swc file:\n" + fname_neuron);
        return;
    }

    //output markers
    QString fname_marker = fname+"_affine.marker";
    output_markers_affinespace(fname_marker);

    //output amat
    double amat[16];
    getAffineAmat(amat, shift_x, shift_y, shift_z, rotation_ang, rotation_cx, rotation_cy, rotation_cz, direction);
    QString fname_amat = fname+"_amat.txt";
    writeAmat(fname_amat.toStdString().c_str(), amat);

    //output ano
    QString fname_ano = fname+"_affine.ano";
    QFile qf_anofile(fname_ano);
    if(!qf_anofile.open(QIODevice::WriteOnly))
    {
        v3d_msg("Cannot open file for writing!");
        return;
    }
    QTextStream out(&qf_anofile);
    out<<"SWCFILE="<<fname_nt0<<endl;
    out<<"SWCFILE="<<fname_neuron<<endl;
    qf_anofile.close();
}

void neuron_match_clique::affine_nt1()
{
    if(pmatch0.size()<0){
        qDebug()<<"not match point identified, quit align";
        return;
    }

    backupNeuron(*nt1_org, nt1_a);

    double shift_stack = quickMoveNeuron(nt0_org, &nt1_a, direction);

    QList<XYZ> c0, c1;
    for(int i=0; i<pmatch0.size(); i++){
        c0.append(XYZ(nt0_org->listNeuron.at(pmatch0.at(i))));
        c1.append(XYZ(nt1_a.listNeuron.at(pmatch1.at(i))));
    }

    double tmp_shift_x=0,tmp_shift_y=0,tmp_shift_z=0,tmp_angle=0,tmp_cent_x=0,tmp_cent_y=0,tmp_cent_z=0;
    if(direction==0) tmp_shift_x=-1;
    if(direction==1) tmp_shift_y=-1;
    if(direction==2) tmp_shift_z=-1;
    if(!compute_affine_4dof(c0,c1,tmp_shift_x,tmp_shift_y,tmp_shift_z,tmp_angle,tmp_cent_x,tmp_cent_y,tmp_cent_z,direction)){
        return;
    }

    affineNeuron(nt1_a,nt1_a,tmp_shift_x,tmp_shift_y,tmp_shift_z,tmp_angle,tmp_cent_x,tmp_cent_y,tmp_cent_z,direction);
    if(direction==0) tmp_shift_x=shift_stack;
    if(direction==1) tmp_shift_y=shift_stack;
    if(direction==2) tmp_shift_z=shift_stack;
    shift_x = tmp_shift_x;
    shift_y = tmp_shift_y;
    shift_z = tmp_shift_z;
    rotation_ang = tmp_angle;
    rotation_cx = tmp_cent_x;
    rotation_cy = tmp_cent_y;
    rotation_cz = tmp_cent_z;
}

void neuron_match_clique::matchCliquesAndCands()
{
    rankedCliqueMatch.clear();
    candMS.clear();
    MS_x=candID0.size();
    MS_y=candID1.size();
    candMS.resize(MS_x*MS_y,0);
//    candMS=QVector(MS_x*MS_y,0);

    QVector<QVector<int> > cliquePairs;
    for(int i=0; i<cliqueList0.size(); i++){
        for(int j=0; j<cliqueList1.size(); j++){
            //find matching clique
            if(fabs(cliqueList0[i].e[0]-cliqueList1[j].e[0])>cmatchThr) continue;
            if(fabs(cliqueList0[i].e[1]-cliqueList1[j].e[1])>cmatchThr) continue;
            if(fabs(cliqueList0[i].e[2]-cliqueList1[j].e[2])>cmatchThr) continue;
            if(NTDOT(cliqueList0[i].dir[0],cliqueList1[j].dir[0])<angThr_match) continue;
            if(NTDOT(cliqueList0[i].dir[1],cliqueList1[j].dir[1])<angThr_match) continue;
            if(NTDOT(cliqueList0[i].dir[2],cliqueList1[j].dir[2])<angThr_match) continue;

            QVector<int> pair;
            pair<<i<<j;
            cliquePairs<<pair;
            for(int k=0; k<3; k++)
                CANDMS_ENTRY(cliqueList0[i].idx[k],cliqueList1[j].idx[k])++;
        }
    }

    qDebug()<<"found "<<cliquePairs.size()<<" pair of cliques";

    for(int i=0; i<cliquePairs.size(); i++){
        int matchscore=0;
        for(int k=0; k<3; k++){
            matchscore+=CANDMS_ENTRY(cliqueList0[cliquePairs[i].at(0)].idx[k],cliqueList1[cliquePairs[i].at(1)].idx[k]);
        }
        rankedCliqueMatch.insert(pair<int, QVector<int> >(matchscore,cliquePairs[i]) );
    }
}

void neuron_match_clique::stitch()
{
    backupNeuron(*nt0_org, nt0_stitch);
    backupNeuron(nt1_a, nt1_stitch);
    for(int pid=0; pid<pmatch0.size(); pid++){
        stitchMatchedPoint(&nt0_stitch, &nt1_stitch, parent0, parent1, pmatch0.at(pid), pmatch1.at(pid));
    }
}

//orientation should be 1/-1 for smaller/larger stack in direction
void neuron_match_clique::initNeuronAndCandidate(NeuronTree& nt, QList<int>& cand, QList<XYZ>& candcoord, QList<XYZ>& canddir, QList<XYZ>& canddircoord, QList<int>& components, QList<int>& candcomponents, QList<int>& pList, int orientation)
{
    float min=midplane-spanCand;
    float max=midplane+spanCand;

    cand.clear();
    candcoord.clear();
    canddir.clear();
    canddircoord.clear();
    candcomponents.clear();

    QVector<int> childNum(nt.listNeuron.size(), 0);
    QVector<int> connNum(nt.listNeuron.size(), 0);
    QVector<double> sectionLength(nt.listNeuron.size(), 0);
    //QList<int> components;
    components.clear();
    pList.clear();
    QVector<V3DLONG> componentSize;
    QVector<V3DLONG> componentLength;
    V3DLONG curid=0;
    for(V3DLONG i=0; i<nt.listNeuron.size(); i++){
        if(nt.listNeuron.at(i).pn<0){
            connNum[i]--; //root that only have 1 clide will also be a dead end
            components.append(curid); curid++;
            pList.append(-1);
        }
        else{
            int pid = nt.hashNeuron.value(nt.listNeuron.at(i).pn);
            childNum[pid]++;
            connNum[pid]++;
            sectionLength[i]=sqrt(NTDIS(nt.listNeuron.at(i),nt.listNeuron.at(pid)));
            components.append(-1);
            pList.append(pid);
        }
    }
    //connected component
    for(V3DLONG cid=0; cid<curid; cid++){
        QStack<int> pstack;
        int chid, size = 0;
        if(!components.contains(cid)) //should not happen, just in case
            continue;
        if(components.indexOf(cid)!=components.lastIndexOf(cid)) //should not happen
            qDebug("unexpected multiple tree root, please check the code: neuron_stitch_func.cpp");
        //recursively search for child and mark them as the same component
        pstack.push(components.indexOf(cid));
        size++;
        while(!pstack.isEmpty()){
            int pid=pstack.pop();
            chid = -1;
            chid = pList.indexOf(pid,chid+1);
            while(chid>=0){
                pstack.push(chid);
                components[chid]=cid;
                chid=pList.indexOf(pid,chid+1);
                size++;
            }
        }
        componentSize.append(size);
    }
    //component size
    for(V3DLONG cid=0; cid<curid; cid++){
        double length = 0;
        int idx = -1;
        for(V3DLONG i=0; i<componentSize[cid]; i++){
            idx = components.indexOf(cid,idx+1);
            length+=sectionLength[idx];
        }
        componentLength.append(length);
    }


    QList<int> tmpcand; tmpcand.clear();
    for(V3DLONG i=0; i<childNum.size(); i++){
        if(connNum[i]<=0 && componentLength[components[i]]>=segmentThr){
            if(direction==0){//x
                if(nt.listNeuron.at(i).x>min && nt.listNeuron.at(i).x<max){
                    tmpcand.append(i);
                }
            }else if(direction==1){//y
                if(nt.listNeuron.at(i).y>min && nt.listNeuron.at(i).y<max){
                    tmpcand.append(i);
                }
            }else if(direction==2){//z
                if(nt.listNeuron.at(i).z>min && nt.listNeuron.at(i).z<max){
                    tmpcand.append(i);
                }
            }else{//all tips
                tmpcand.append(i);
            }
        }
    }

    for(V3DLONG i=0; i<tmpcand.size(); i++){
        XYZ tmpdir(0,0,0);
        int id = tmpcand[i];
        if(childNum[id]==1){ //single child root
            float lentmp = 0;
            while(lentmp<dir_range && id>=0){
                int sid = pList.indexOf(id);
                if(sid<0) break;
                lentmp+=sectionLength[sid];
                tmpdir.x += nt.listNeuron.at(id).x-nt.listNeuron.at(sid).x;
                tmpdir.y += nt.listNeuron.at(id).y-nt.listNeuron.at(sid).y;
                tmpdir.z += nt.listNeuron.at(id).z-nt.listNeuron.at(sid).z;
                id=sid;
            }
        }else{ //tips
            float lentmp = 0;
            while(lentmp<dir_range && id>=0){
                int sid = pList[id];
                if(sid<0) break;
                lentmp+=sectionLength[id];
                tmpdir.x += nt.listNeuron.at(id).x-nt.listNeuron.at(sid).x;
                tmpdir.y += nt.listNeuron.at(id).y-nt.listNeuron.at(sid).y;
                tmpdir.z += nt.listNeuron.at(id).z-nt.listNeuron.at(sid).z;
                id=sid;
            }
        }
        //normalize direction
        double tmpNorm = sqrt(tmpdir.x*tmpdir.x+tmpdir.y*tmpdir.y+tmpdir.z*tmpdir.z);
        tmpdir.x/=tmpNorm; tmpdir.x*=orientation;
        tmpdir.y/=tmpNorm; tmpdir.y*=orientation;
        tmpdir.z/=tmpNorm; tmpdir.z*=orientation;

        //judgement to the direction to avoid the connection that moves from plan to
        double sa = 1;
        if(direction==0){
            sa = tmpdir.x;
        }else if(direction == 1){
            sa = tmpdir.y;
        }else if(direction == 2){
            sa = tmpdir.z;
        }
        //for test:
        //qDebug()<<i<<":"<<tmpcand[i]<<"; coord"<<nt.listNeuron.at(tmpcand[i]).x<<":"<<nt.listNeuron.at(tmpcand[i]).y<<":"<<nt.listNeuron.at(tmpcand[i]).z<<"; dir"<<tmpdir.x<<":"<<tmpdir.y<<":"<<tmpdir.z<<":"<<tmpNorm;

        if(sa<angThr_stack) continue;

        cand.append(tmpcand[i]);
        candcoord.append(XYZ(nt.listNeuron.at(tmpcand[i])));
        canddir.append(tmpdir);
        canddircoord.append(XYZ(candcoord.last().x+canddir.last().x,candcoord.last().y+canddir.last().y,candcoord.last().z+canddir.last().z));
        candcomponents.append(components.at(tmpcand[i]));
    }
}
