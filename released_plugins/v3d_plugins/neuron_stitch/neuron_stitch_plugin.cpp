/* neuron_stitch_plugin.cpp
 * This plugin is for link neuron segments across stacks.
 * 2014-10-07 : by Hanbo Chen
 */

#include "v3d_message.h"
#include <vector>
#include "neuron_stitch_plugin.h"
#include "neuron_stitch_func.h"
#include "neuron_tipspicker_dialog.h"
#include "neuron_match_clique.h"
#include "neuron_geometry_dialog.h"
#include "../../../v3d_main/neuron_editing/neuron_xforms.h"
#include <iostream>
#include <fstream>
#include "performance_timer.h"

using namespace std;

Q_EXPORT_PLUGIN2(neuron_stitch, neuron_stitch);

QList<NeuronGeometryDialog* > dialogList;

QStringList neuron_stitch::menulist() const
{
	return QStringList() 
        <<tr("auto_stitch_neuron_SWC")
        <<tr("live_stitch_neuron_SWC")
        <<tr("manually_affine_neuron_SWC")
        <<tr("find_border_tips_SWC_image")
        <<tr("transform_neuron_SWC_by_affine_matrix")
        <<tr("transform_markers_by_affine_matrix")
        <<tr("combine_marker_files")
		<<tr("about");
}

QStringList neuron_stitch::funclist() const
{
	return QStringList()
        <<tr("neuron_stitch_auto")
      //  <<tr("tmp_test")
		<<tr("help");
}

void neuron_stitch::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("auto_stitch_neuron_SWC"))
    {
        domatch(callback, parent);
    }
    else if (menu_name == tr("live_stitch_neuron_SWC"))
    {
        dostitch(callback, parent);
    }
    else if (menu_name == tr("manually_affine_neuron_SWC"))
    {
        doadjust(callback, parent);
    }
    else if (menu_name == tr("find_border_tips_SWC_image"))
    {
        dosearch(callback, parent);
    }
    else if (menu_name == tr("transform_neuron_SWC_by_affine_matrix"))
    {
        dotransform_swc(callback, parent);
    }
    else if (menu_name == tr("transform_markers_by_affine_matrix"))
    {
        dotransform_marker(callback, parent);
    }
    else if (menu_name == tr("combine_marker_files"))
    {
        docombine_marker(callback, parent);
    }
	else
	{
		v3d_msg(tr("This plugin is for link neuron segments across stacks.. "
			"Developed by Hanbo Chen, 2014-10-07"));
	}
}

bool neuron_stitch::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("neuron_stitch_auto"))
	{
        cout<<"==== neuron stack auto stitcher ===="<<endl;
        if(infiles.size()!=2 || outfiles.size()!=1)
        {
            qDebug("ERROR: please set input and output!");
            printHelp();
            return false;
        }

        //load neurons
        QString fname_nt0 = ((vector<char*> *)(input.at(0).p))->at(0);
        QString fname_nt1 = ((vector<char*> *)(input.at(0).p))->at(1);
        QString fname_output = ((vector<char*> *)(output.at(0).p))->at(0);
        NeuronTree* nt0 = new NeuronTree();
        if (fname_nt0.toUpper().endsWith(".SWC") || fname_nt0.toUpper().endsWith(".ESWC")){
            *nt0 = readSWC_file(fname_nt0);
        }
        if(nt0->listNeuron.size()<=0){
            qDebug()<<"failed to read SWC file: "<<fname_nt0;
            return false;
        }
        NeuronTree* nt1 = new NeuronTree();
        if (fname_nt1.toUpper().endsWith(".SWC") || fname_nt1.toUpper().endsWith(".ESWC")){
            *nt1 = readSWC_file(fname_nt1);
        }
        if(nt1->listNeuron.size()<=0){
            qDebug()<<"failed to read SWC file: "<<fname_nt1;
            return false;
        }

        neuron_match_clique matchfunc(nt0, nt1);

        //get parameters
        vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
        if(paras.size()>0){
            int tmp=atoi(paras.at(0));
            if(tmp>=0 && tmp<=2)
                matchfunc.direction=tmp;
            else
                cerr<<"error: wrong stack direction: "<<tmp<<"; use default value 2 (z direction)"<<endl;
        }
        if(paras.size()>1){
            double tmp=atof(paras.at(1));
            if(tmp>1e-10)
                matchfunc.zscale=tmp;
            else
                cerr<<"error: wrong rescale value: "<<tmp<<"; use default value 1"<<endl;
        }
        if(paras.size()>2){
            double tmp=atof(paras.at(2));
            if(tmp>0&&tmp<180)
                matchfunc.angThr_match=cos(tmp/180*M_PI);
            else
                cerr<<"error: wrong angular threshold: "<<tmp<<"; use default value 60"<<endl;
        }
        if(paras.size()>3){
            double tmp=atof(paras.at(3));
            if(tmp>=0)
                matchfunc.pmatchThr=tmp;
            else
                cerr<<"error: wrong point match distance threshold: "<<tmp<<"; use default value 100"<<endl;
        }
        if(paras.size()>4){
            double tmp=atof(paras.at(4));
            if(tmp>0)
                matchfunc.cmatchThr=tmp;
            else
                cerr<<"error: wrong triangle match distance threshold: "<<tmp<<"; use default value 100"<<endl;
        }
        if(paras.size()>5){
            int tmp=atoi(paras.at(5));
            if(tmp>0)
                matchfunc.maxClique3Num=tmp;
            else
                cerr<<"error: wrong max number of triangles: "<<tmp<<"; use default value 10000"<<endl;
        }
        if(paras.size()>6){
            double tmp=atof(paras.at(6));
            if(tmp>0)
                matchfunc.spanCand=tmp;
            else
                cerr<<"error: wrong border tips seraching span: "<<tmp<<"; use default value 20"<<endl;
        }
        if(paras.size()>7){
            double tmp=atof(paras.at(7));
            if(tmp>0)
                matchfunc.gapThr=tmp;
            else
                cerr<<"error: wrong border tips gap filter: "<<tmp<<"; use default value 0, no tips will be filtered by gap"<<endl;
        }
        if(paras.size()>8){
            double tmp=atof(paras.at(8));
            if(tmp>0)
                matchfunc.segmentThr=tmp;
            else
                cerr<<"error: wrong border tips segment filter: "<<tmp<<"; use default value 0, no tips will be filtered by segment"<<endl;
        }
        if(paras.size()>9){
            int tmp=atoi(paras.at(9));
            if(tmp>0)
                matchfunc.spineLengthThr=tmp;
            else
                cerr<<"error: wrong border tips spine size filter: "<<tmp<<"; use default value 0, no tips will be considered as spine"<<endl;
        }
        if(paras.size()>10){
            double tmp=atof(paras.at(10));
            if(tmp>=0 & tmp<=180)
                matchfunc.spineAngThr=cos(tmp/180*M_PI);
            else
                cerr<<"error: wrong border tips spine angle filter: "<<tmp<<"; use default value 180, no spine will be filtered by angle"<<endl;
        }
        if(paras.size()>11){
            double tmp=atof(paras.at(11));
            if(tmp>=0)
                matchfunc.spineRadiusThr=tmp;
            else
                cerr<<"error: wrong border tips spine radius filter: "<<tmp<<"; use default value 0, no tips will be considered as spine"<<endl;
        }

        //do match
        //init clique and candidate
        matchfunc.init();
        //global match
        cout<<"start global search"<<endl;
        matchfunc.globalmatch();
        //stitch
        cout<<"stitch matched points"<<endl;
        matchfunc.stitch();

        //save results
        matchfunc.output_stitch(fname_output);
        matchfunc.output_candMatchScore(fname_output + "_matchscore.txt");
        matchfunc.output_affine(fname_output,fname_nt0);
        matchfunc.output_matchedMarkers_orgspace(fname_output+"_nt0_matched.marker",fname_output+"_nt1_matched.marker");
        matchfunc.output_parameter(fname_output+"_param.txt");

        cout<<("matching finished")<<endl;
    }
    else if (func_name == tr("tmp_test"))
    {
        doperformancetest(input, output, callback);
    }
	else if (func_name == tr("help"))
	{
        printHelp();
    }
	else return false;

	return true;
}

void neuron_stitch::printHelp()
{
    cout<<"\nUsage: v3d -x neuron_stitch -f neuron_stitch_auto -i <input_first.swc> <input_second.swc> -o <output_base> "
       <<"-p <stack dir=2(x/y/z=0/1/2)> <stack rescale=1> <match angle thr=60> <match distance thr=100> <triangle match thr=100> <max number of triangles=10000> "<<
            "<border tips serch span=20> <border tips gap filter=0> <border tips segment filter=0> <border tips spine filter_size=0> "<<
            "<border tips spine filter_angle=180> <border tips spine filter_radius=0>"<<endl;
    cout<<"\n";
}

void neuron_stitch::domatch(V3DPluginCallback2 &callback, QWidget *parent)
{
    NeuronMatchDialog * myDialog = NULL;
    //myDialog = new NeuronMatchDialog(&callback, v3dwin);
    myDialog = new NeuronMatchDialog();
    myDialog->exec();
}

void neuron_stitch::dosearch(V3DPluginCallback2 &callback, QWidget *parent)
{
    //select the window to operate
    QList <V3dR_MainWindow *> allWindowList = callback.getListAll3DViewers();
    QList <V3dR_MainWindow *> selectWindowList;
    V3dR_MainWindow * v3dwin;
    QList<NeuronTree> * ntTreeList;
    int winid;
    qDebug("search for 3D windows");
    for (V3DLONG i=0;i<allWindowList.size();i++)
    {
        ntTreeList = callback.getHandleNeuronTrees_Any3DViewer(allWindowList[i]);
        if(ntTreeList->size()>0)
            selectWindowList.append(allWindowList[i]);
    }
    qDebug("match and select 3D windows");
    if(selectWindowList.size()<1){
        v3d_msg("Cannot find 3D view with only 2 SWC file. Please load the two SWC file you want to stitch in the same 3D view");
        return;
    }else if(selectWindowList.size()>1){
        //pop up a window to select

        QStringList items;
        for(int i=0; i<selectWindowList.size(); i++){
            items.append(callback.getImageName(selectWindowList[i]));
        }
        bool ok;
        QString selectitem = QInputDialog::getItem(parent, QString::fromUtf8("Neuron Stitcher"), QString::fromUtf8("Select A Window to Operate"), items, 0, false, &ok);
        if(!ok) return;
        for(int i=0; i<selectWindowList.size(); i++){
            if(selectitem==callback.getImageName(selectWindowList[i]))
            {
                winid=i;
                break;
            }
        }
    }else{
        winid=0;
    }
    v3dwin = selectWindowList[winid];

    neuron_tipspicker_dialog * myDialog = NULL;
    myDialog = new neuron_tipspicker_dialog(&callback, v3dwin);
    myDialog->show();
}

void neuron_stitch::dostitch(V3DPluginCallback2 &callback, QWidget *parent)
{
    //select the window to operate
    QList <V3dR_MainWindow *> allWindowList = callback.getListAll3DViewers();
    QList <V3dR_MainWindow *> selectWindowList;
    V3dR_MainWindow * v3dwin;
    QList<NeuronTree> * ntTreeList;
    int winid;
    qDebug("search for 3D windows");
    for (V3DLONG i=0;i<allWindowList.size();i++)
    {
        ntTreeList = callback.getHandleNeuronTrees_Any3DViewer(allWindowList[i]);
        if(ntTreeList->size()==2)
            selectWindowList.append(allWindowList[i]);
    }
    qDebug("match and select 3D windows");
    if(selectWindowList.size()<1){
        v3d_msg("Cannot find 3D view with only 2 SWC file. Please load the two SWC file you want to stitch in the same 3D view");
        return;
    }else if(selectWindowList.size()>1){
        //pop up a window to select

        QStringList items;
        for(int i=0; i<selectWindowList.size(); i++){
            items.append(callback.getImageName(selectWindowList[i]));
        }
        bool ok;
        QString selectitem = QInputDialog::getItem(parent, QString::fromUtf8("Neuron Stitcher"), QString::fromUtf8("Select A Window to Operate"), items, 0, false, &ok);
        if(!ok) return;
        for(int i=0; i<selectWindowList.size(); i++){
            if(selectitem==callback.getImageName(selectWindowList[i]))
            {
                winid=i;
                break;
            }
        }
    }else{
        winid=0;
    }
    v3dwin = selectWindowList[winid];

    NeuronLiveMatchDialog * myDialog = NULL;
    myDialog = new NeuronLiveMatchDialog(&callback, v3dwin);
    myDialog->show();
}

void neuron_stitch::doadjust(V3DPluginCallback2 &callback, QWidget *parent)
{
    //select the window to operate
    QList <V3dR_MainWindow *> allWindowList = callback.getListAll3DViewers();
    QList <V3dR_MainWindow *> selectWindowList;
    V3dR_MainWindow * v3dwin;
    QList<NeuronTree> * ntTreeList;
    int winid;
    qDebug("search for 3D windows");
    for (V3DLONG i=0;i<allWindowList.size();i++)
    {
        ntTreeList = callback.getHandleNeuronTrees_Any3DViewer(allWindowList[i]);
        if(ntTreeList->size()==2)
            selectWindowList.append(allWindowList[i]);
    }
    qDebug("match and select 3D windows");
    if(selectWindowList.size()<1){
        v3d_msg("Cannot find 3D view with only 2 SWC file. Please load the two SWC file you want to stitch in the same 3D view");
        return;
    }else if(selectWindowList.size()>1){
        //pop up a window to select

        QStringList items;
        for(int i=0; i<selectWindowList.size(); i++){
            items.append(callback.getImageName(selectWindowList[i]));
        }
        bool ok;
        QString selectitem = QInputDialog::getItem(parent, QString::fromUtf8("Neuron Stitcher"), QString::fromUtf8("Select A Window to Operate"), items, 0, false, &ok);
        if(!ok) return;
        for(int i=0; i<selectWindowList.size(); i++){
            if(selectitem==callback.getImageName(selectWindowList[i]))
            {
                winid=i;
                break;
            }
        }
    }else{
        winid=0;
    }
    v3dwin = selectWindowList[winid];

    NeuronGeometryDialog * myDialog = NULL;
    myDialog = new NeuronGeometryDialog(&callback, v3dwin);
    myDialog->show();
}

int neuron_stitch::dotransform_swc(V3DPluginCallback2 &callback, QWidget *parent)
{
    //input file name
    QString fileOpenName;
    fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open SWC File"),
            "",
            QObject::tr("Supported file (*.swc *.eswc)"
                ";;Neuron structure	(*.swc)"
                ";;Extended neuron structure (*.eswc)"
                ));
    if(fileOpenName.isEmpty())
        return 0;
    NeuronTree nt;
    if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
    {
        nt = readSWC_file(fileOpenName);
    }

    QString fileMatName = QFileDialog::getOpenFileName(0, QObject::tr("Open Affine Matrix File"),
            "",
            QObject::tr("Supported file (*.txt)"
                ";;Affine Matrix    (*.txt)"
                ));
    if(fileMatName.isEmpty()) return 0;

    double amat[16]={0};
    if (!readAmat(fileMatName.toStdString().c_str(),amat))
    {
            v3d_msg("error read affine transform matrix.");
            return 0;
    }

    proc_neuron_affine(&nt, amat);

    QString fileDefaultName = fileOpenName+QString("_affine.swc");
    //write new SWC to file
    QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
            fileDefaultName,
            QObject::tr("Supported file (*.swc)"
                ";;Neuron structure	(*.swc)"
                ));
    if (!export_list2file(nt.listNeuron,fileSaveName,fileOpenName))
    {
        v3d_msg("fail to write the output swc file.");
        return 0;
    }

    return 1;
}

int neuron_stitch::dotransform_marker(V3DPluginCallback2 &callback, QWidget *parent)
{
    //input file name
    QString fileOpenName;
    fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open marker File"),
            "",
            QObject::tr("Supported file (*.marker)"
                ";;Marker file	(*.marker)"
                ));
    if(fileOpenName.isEmpty())
        return 0;
    QList <ImageMarker> inmarker;
    if (fileOpenName.toUpper().endsWith(".MARKER"))
    {
         inmarker = readMarker_file(fileOpenName);
    }

    QString fileMatName = QFileDialog::getOpenFileName(0, QObject::tr("Open Affine Matrix File"),
            "",
            QObject::tr("Supported file (*.txt)"
                ";;Affine Matrix    (*.txt)"
                ));
    if(fileMatName.isEmpty()) return 0;

    double afmatrix[16]={0};
    if (!readAmat(fileMatName.toStdString().c_str(),afmatrix))
    {
        v3d_msg("error read affine transform matrix.");
        return 0;
    }

    //marker affine
    double x,y,z;
    for(V3DLONG i=0; i<inmarker.size() ; i++)
    {
        ImageMarker* tp = &(inmarker[i]);
        x = afmatrix[0] * tp->x + afmatrix[1] * tp->y + afmatrix[2] * tp->z + afmatrix[3];
        y = afmatrix[4] * tp->x + afmatrix[5] * tp->y + afmatrix[6] * tp->z + afmatrix[7];
        z = afmatrix[8] * tp->x + afmatrix[9] * tp->y + afmatrix[10] * tp->z + afmatrix[11];

        //now update
        tp->x = x;	tp->y = y; tp->z = z;
    }

    QString fileDefaultName = fileOpenName+QString("_affine.marker");
    //write new marker to file
    QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
            fileDefaultName,
            QObject::tr("Supported file (*.marker)"
                ";;Marker	(*.marker)"
                ));
    if(fileSaveName.isEmpty())
        return 0;
    if (!writeMarker_file(fileSaveName, inmarker))
    {
        v3d_msg("fail to write the output marker file.");
        return 0;
    }

    return 1;
}

void neuron_stitch::docombine_marker(V3DPluginCallback2 &callback, QWidget *parent)
{
    //input file 1
    QString fileOpenName;
    fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open marker File"),
            "",
            QObject::tr("Supported file (*.marker)"
                ";;Marker file	(*.marker)"
                ));
    if(fileOpenName.isEmpty())
        return;
    QList <ImageMarker> inmarker;
    if (fileOpenName.toUpper().endsWith(".MARKER"))
    {
         inmarker = readMarker_file(fileOpenName);
    }

    //input file 2
    QString fileOpenName_2;
    fileOpenName_2 = QFileDialog::getOpenFileName(0, QObject::tr("Open marker File"),
            fileOpenName,
            QObject::tr("Supported file (*.marker)"
                ";;Marker file	(*.marker)"
                ));
    if(fileOpenName_2.isEmpty())
        return;
    QList <ImageMarker> inmarker_2;
    if (fileOpenName.toUpper().endsWith(".MARKER"))
    {
         inmarker_2 = readMarker_file(fileOpenName_2);
    }

    //output file
    for(V3DLONG i=0; i<inmarker_2.size() ; i++){
        inmarker.append(inmarker_2[i]);
    }

    QString fileDefaultName = QFileInfo(fileOpenName).dir().filePath(QFileInfo(fileOpenName).baseName()+
                                                                     "_"+QFileInfo(fileOpenName_2).baseName()+QString("_combine.marker"));
    //write new marker to file
    QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
            fileDefaultName,
            QObject::tr("Supported file (*.marker)"
                ";;Marker	(*.marker)"
                ));
    if(fileSaveName.isEmpty())
        return;
    if (!writeMarker_file(fileSaveName, inmarker))
    {
        v3d_msg("fail to write the output marker file.");
        return;
    }
}

//temporary function for paper writting purpose
void neuron_stitch::doperformancetest(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    cout<<"==== neuron stack auto stitcher ===="<<endl;
    if(infiles.size()!=2 || outfiles.size()!=1 || inparas.size()!=1)
    {
        qDebug("ERROR: please set input and output!");
        qDebug()<<"v3d -x neuron_stitch -f tmp_test -i nt_lower.swc nt_upper.swc -p nt_upper_truth.swc -o output";
        return;
    }

    //load neurons
    QString fname_nt0 = ((vector<char*> *)(input.at(0).p))->at(0);
    QString fname_nt1 = ((vector<char*> *)(input.at(0).p))->at(1);
    QString fname_nt1_truth = ((vector<char*> *)(input.at(1).p))->at(0);
    QString fname_output = ((vector<char*> *)(output.at(0).p))->at(0);
    NeuronTree* nt0 = new NeuronTree();
    if (fname_nt0.toUpper().endsWith(".SWC") || fname_nt0.toUpper().endsWith(".ESWC")){
        *nt0 = readSWC_file(fname_nt0);
    }
    if(nt0->listNeuron.size()<=0){
        qDebug()<<"failed to read SWC file: "<<fname_nt0;
        return;
    }
    NeuronTree* nt1 = new NeuronTree();
    if (fname_nt1.toUpper().endsWith(".SWC") || fname_nt1.toUpper().endsWith(".ESWC")){
        *nt1 = readSWC_file(fname_nt1);
    }
    if(nt1->listNeuron.size()<=0){
        qDebug()<<"failed to read SWC file: "<<fname_nt1;
        return;
    }
    NeuronTree* nt1_truth = new NeuronTree();
    if (fname_nt1_truth.toUpper().endsWith(".SWC") || fname_nt1_truth.toUpper().endsWith(".ESWC")){
        *nt1_truth = readSWC_file(fname_nt1_truth);
    }
    if(nt1_truth->listNeuron.size()<=0){
        qDebug()<<"failed to read SWC file: "<<fname_nt1_truth;
        return;
    }

    double _spanCand = 40;
    int _direction = 2;
    double _angThr_match = cos(70.0/180.0*M_PI); //50 degree
    double _cmatchThr = 100;
    double _pmatchThr = 100;
    double _zscale = 2;
    double _segmentThr = 0;
    double _gapThr = 0;
    int _spineLengthThr = 0;
    double _spineAngThr = -1;
    double _spineRadiusThr = 0;
    int _maxClique3Num = 6400;

    vector<double> walltimes;
    vector<double> cputimes;
    vector<int> num_TP, num_FP, num_FN, num_truth, num_bp0, num_bp1, num_tri0, num_tri1, num_trimatch;
    vector<int> tri_number;
    vector<double> pmatchThr,pmatchAng,tmatchThr;
    vector<double> distance, xydistance;

    //triangle number
    for(int i=0; i<10; i++){
        int tri=100*pow(2.0,i);
        //test triangle number and running time
        //init matching function
        double tmpnum[11];
        NeuronTree nt0_run;
        backupNeuron(*nt0, nt0_run);
        NeuronTree nt1_run;
        backupNeuron(*nt1, nt1_run);
        neuron_match_clique matchfunc(&nt0_run,&nt1_run);

        //parameters
        matchfunc.direction=_direction;
        matchfunc.zscale=_zscale;
        matchfunc.angThr_match=_angThr_match;
        matchfunc.pmatchThr = _pmatchThr;
        matchfunc.spanCand = _spanCand;
        matchfunc.cmatchThr = _cmatchThr;
        matchfunc.segmentThr = _segmentThr;
        matchfunc.gapThr = _gapThr;
        matchfunc.spineLengthThr = _spineLengthThr;
        matchfunc.spineAngThr = _spineAngThr;
        matchfunc.spineRadiusThr = _spineRadiusThr;

        matchfunc.maxClique3Num = tri;

        tri_number.push_back(matchfunc.maxClique3Num);
        pmatchThr.push_back(matchfunc.pmatchThr);
        pmatchAng.push_back(matchfunc.angThr_match);
        tmatchThr.push_back(matchfunc.cmatchThr);

        double wall0 = get_wall_time();
        double cpu0  = get_cpu_time();
        //init clique and candidate
        matchfunc.init();

        //global match
        matchfunc.globalmatch();
        //  Stop timers
        double wall1 = get_wall_time();
        double cpu1  = get_cpu_time();

        matchfunc.examineMatchingResult(tmpnum, nt1_truth);

        qDebug()<<"cojoc: "<<wall1-wall0<<":"<<cpu1-cpu0<<":"<<(int)tmpnum[0]<<":"<<(int)tmpnum[1]<<":"<<(int)tmpnum[2]<<":"<<(int)tmpnum[3]<<":"<<(int)tmpnum[4]<<":"<<(int)tmpnum[5]<<":"
               <<(int)tmpnum[6]<<":"<<(int)tmpnum[7]<<":"<<(int)tmpnum[8]<<":"<<tmpnum[9]<<":"<<tmpnum[10];
        walltimes.push_back(wall1-wall0);
        cputimes.push_back(cpu1-cpu0);
        num_TP.push_back((int)tmpnum[0]);
        num_FP.push_back((int)tmpnum[1]);
        num_FN.push_back((int)tmpnum[2]);
        num_truth.push_back((int)tmpnum[3]);
        num_bp0.push_back((int)tmpnum[4]);
        num_bp1.push_back((int)tmpnum[5]);
        num_tri0.push_back((int)tmpnum[6]);
        num_tri1.push_back((int)tmpnum[7]);
        num_trimatch.push_back((int)tmpnum[8]);
        distance.push_back(tmpnum[9]);
        xydistance.push_back(tmpnum[10]);
    }

    //point match distance
    for(double param=20; param<=300; param+=20){
        //test triangle number and running time
        //init matching function
        double tmpnum[11];
        NeuronTree nt0_run;
        backupNeuron(*nt0, nt0_run);
        NeuronTree nt1_run;
        backupNeuron(*nt1, nt1_run);
        neuron_match_clique matchfunc(&nt0_run,&nt1_run);

        //parameters
        matchfunc.direction=_direction;
        matchfunc.zscale=_zscale;
        matchfunc.angThr_match=_angThr_match;
        matchfunc.spanCand = _spanCand;
        matchfunc.cmatchThr = _cmatchThr;
        matchfunc.segmentThr = _segmentThr;
        matchfunc.maxClique3Num = _maxClique3Num;
        matchfunc.gapThr = _gapThr;
        matchfunc.spineLengthThr = _spineLengthThr;
        matchfunc.spineAngThr = _spineAngThr;
        matchfunc.spineRadiusThr = _spineRadiusThr;

        matchfunc.pmatchThr = param;

        tri_number.push_back(matchfunc.maxClique3Num);
        pmatchThr.push_back(matchfunc.pmatchThr);
        pmatchAng.push_back(matchfunc.angThr_match);
        tmatchThr.push_back(matchfunc.cmatchThr);

        double wall0 = get_wall_time();
        double cpu0  = get_cpu_time();
        //init clique and candidate
        matchfunc.init();

        //global match
        matchfunc.globalmatch();
        //  Stop timers
        double wall1 = get_wall_time();
        double cpu1  = get_cpu_time();

        matchfunc.examineMatchingResult(tmpnum, nt1_truth);

        qDebug()<<"cojoc: "<<wall1-wall0<<":"<<cpu1-cpu0<<":"<<(int)tmpnum[0]<<":"<<(int)tmpnum[1]<<":"<<(int)tmpnum[2]<<":"<<(int)tmpnum[3]<<":"<<(int)tmpnum[4]<<":"<<(int)tmpnum[5]<<":"
               <<(int)tmpnum[6]<<":"<<(int)tmpnum[7]<<":"<<(int)tmpnum[8]<<":"<<tmpnum[9]<<":"<<tmpnum[10];
        walltimes.push_back(wall1-wall0);
        cputimes.push_back(cpu1-cpu0);
        num_TP.push_back((int)tmpnum[0]);
        num_FP.push_back((int)tmpnum[1]);
        num_FN.push_back((int)tmpnum[2]);
        num_truth.push_back((int)tmpnum[3]);
        num_bp0.push_back((int)tmpnum[4]);
        num_bp1.push_back((int)tmpnum[5]);
        num_tri0.push_back((int)tmpnum[6]);
        num_tri1.push_back((int)tmpnum[7]);
        num_trimatch.push_back((int)tmpnum[8]);
        distance.push_back(tmpnum[9]);
        xydistance.push_back(tmpnum[10]);
    }

    //point match angular threshold
    for(double param=10; param<181; param+=10){
        //test triangle number and running time
        //init matching function
        double tmpnum[11];
        NeuronTree nt0_run;
        backupNeuron(*nt0, nt0_run);
        NeuronTree nt1_run;
        backupNeuron(*nt1, nt1_run);
        neuron_match_clique matchfunc(&nt0_run,&nt1_run);

        //parameters
        matchfunc.direction=_direction;
        matchfunc.zscale=_zscale;
        matchfunc.pmatchThr = _pmatchThr;
        matchfunc.spanCand = _spanCand;
        matchfunc.cmatchThr = _cmatchThr;
        matchfunc.segmentThr = _segmentThr;
        matchfunc.maxClique3Num = _maxClique3Num;
        matchfunc.gapThr = _gapThr;
        matchfunc.spineLengthThr = _spineLengthThr;
        matchfunc.spineAngThr = _spineAngThr;
        matchfunc.spineRadiusThr = _spineRadiusThr;

        matchfunc.angThr_match=cos(param/180*M_PI);

        tri_number.push_back(matchfunc.maxClique3Num);
        pmatchThr.push_back(matchfunc.pmatchThr);
        pmatchAng.push_back(matchfunc.angThr_match);
        tmatchThr.push_back(matchfunc.cmatchThr);

        double wall0 = get_wall_time();
        double cpu0  = get_cpu_time();
        //init clique and candidate
        matchfunc.init();

        //global match
        matchfunc.globalmatch();
        //  Stop timers
        double wall1 = get_wall_time();
        double cpu1  = get_cpu_time();

        matchfunc.examineMatchingResult(tmpnum, nt1_truth);

        qDebug()<<"cojoc: "<<wall1-wall0<<":"<<cpu1-cpu0<<":"<<(int)tmpnum[0]<<":"<<(int)tmpnum[1]<<":"<<(int)tmpnum[2]<<":"<<(int)tmpnum[3]<<":"<<(int)tmpnum[4]<<":"<<(int)tmpnum[5]<<":"
               <<(int)tmpnum[6]<<":"<<(int)tmpnum[7]<<":"<<(int)tmpnum[8]<<":"<<tmpnum[9]<<":"<<tmpnum[10];
        walltimes.push_back(wall1-wall0);
        cputimes.push_back(cpu1-cpu0);
        num_TP.push_back((int)tmpnum[0]);
        num_FP.push_back((int)tmpnum[1]);
        num_FN.push_back((int)tmpnum[2]);
        num_truth.push_back((int)tmpnum[3]);
        num_bp0.push_back((int)tmpnum[4]);
        num_bp1.push_back((int)tmpnum[5]);
        num_tri0.push_back((int)tmpnum[6]);
        num_tri1.push_back((int)tmpnum[7]);
        num_trimatch.push_back((int)tmpnum[8]);
        distance.push_back(tmpnum[9]);
        xydistance.push_back(tmpnum[10]);
    }

    //triangle match distance
    for(double param=20; param<=300; param+=20){
        //test triangle number and running time
        //init matching function
        double tmpnum[11];
        NeuronTree nt0_run;
        backupNeuron(*nt0, nt0_run);
        NeuronTree nt1_run;
        backupNeuron(*nt1, nt1_run);
        neuron_match_clique matchfunc(&nt0_run,&nt1_run);

        //parameters
        matchfunc.direction=_direction;
        matchfunc.zscale=_zscale;
        matchfunc.angThr_match=_angThr_match;
        matchfunc.pmatchThr = _pmatchThr;
        matchfunc.spanCand = _spanCand;
        matchfunc.segmentThr = _segmentThr;
        matchfunc.maxClique3Num = _maxClique3Num;
        matchfunc.gapThr = _gapThr;
        matchfunc.spineLengthThr = _spineLengthThr;
        matchfunc.spineAngThr = _spineAngThr;
        matchfunc.spineRadiusThr = _spineRadiusThr;

        matchfunc.cmatchThr = param;

        tri_number.push_back(matchfunc.maxClique3Num);
        pmatchThr.push_back(matchfunc.pmatchThr);
        pmatchAng.push_back(matchfunc.angThr_match);
        tmatchThr.push_back(matchfunc.cmatchThr);

        double wall0 = get_wall_time();
        double cpu0  = get_cpu_time();
        //init clique and candidate
        matchfunc.init();

        //global match
        matchfunc.globalmatch();
        //  Stop timers
        double wall1 = get_wall_time();
        double cpu1  = get_cpu_time();

        matchfunc.examineMatchingResult(tmpnum, nt1_truth);

        qDebug()<<"cojoc: "<<wall1-wall0<<":"<<cpu1-cpu0<<":"<<(int)tmpnum[0]<<":"<<(int)tmpnum[1]<<":"<<(int)tmpnum[2]<<":"<<(int)tmpnum[3]<<":"<<(int)tmpnum[4]<<":"<<(int)tmpnum[5]<<":"
               <<(int)tmpnum[6]<<":"<<(int)tmpnum[7]<<":"<<(int)tmpnum[8]<<":"<<tmpnum[9]<<":"<<tmpnum[10];
        walltimes.push_back(wall1-wall0);
        cputimes.push_back(cpu1-cpu0);
        num_TP.push_back((int)tmpnum[0]);
        num_FP.push_back((int)tmpnum[1]);
        num_FN.push_back((int)tmpnum[2]);
        num_truth.push_back((int)tmpnum[3]);
        num_bp0.push_back((int)tmpnum[4]);
        num_bp1.push_back((int)tmpnum[5]);
        num_tri0.push_back((int)tmpnum[6]);
        num_tri1.push_back((int)tmpnum[7]);
        num_trimatch.push_back((int)tmpnum[8]);
        distance.push_back(tmpnum[9]);
        xydistance.push_back(tmpnum[10]);
    }

    //output
    QString fname_out=fname_output+"_testlog.txt";
    ofstream fp;
    fp.open(fname_out.toStdString().c_str());
    for(int i=0; i<walltimes.size(); i++){
        fp<<tri_number[i]<<"\t";
        fp<<pmatchThr[i]<<"\t";
        fp<<pmatchAng[i]<<"\t";
        fp<<tmatchThr[i]<<"\t";
        fp<<walltimes[i]<<"\t";
        fp<<cputimes[i]<<"\t";
        fp<<num_TP[i]<<"\t";
        fp<<num_FP[i]<<"\t";
        fp<<num_FN[i]<<"\t";
        fp<<num_truth[i]<<"\t";
        fp<<num_bp0[i]<<"\t";
        fp<<num_bp1[i]<<"\t";
        fp<<num_tri0[i]<<"\t";
        fp<<num_tri1[i]<<"\t";
        fp<<num_trimatch[i]<<"\t";
        fp<<distance[i]<<"\t";
        fp<<xydistance[i]<<endl;
    }
    fp.close();

    neuron_match_clique matchfunc(nt0,nt1);
    matchfunc.direction=_direction;
    matchfunc.zscale=_zscale;
    matchfunc.angThr_match=_angThr_match;
    matchfunc.pmatchThr = _pmatchThr;
    matchfunc.spanCand = _spanCand;
    matchfunc.cmatchThr = _cmatchThr;
    matchfunc.segmentThr = _segmentThr;
    matchfunc.maxClique3Num = _maxClique3Num;
    matchfunc.gapThr = _gapThr;
    matchfunc.spineLengthThr = _spineLengthThr;
    matchfunc.spineAngThr = _spineAngThr;
    matchfunc.spineRadiusThr = _spineRadiusThr;
    matchfunc.output_parameter(fname_output+"_param.txt");

    qDebug()<<"testing finished";
}
