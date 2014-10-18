/****************************************************************************
**
** neuron_geometry_dialog.cpp
** referance: surfaceobj_geometry_dialog.cpp
** by Hanbo Chen 2014.10.8
**
****************************************************************************/

#include "neuron_geometry_dialog.h"
#include "neuron_stitch_func.h"
#include "../../../v3d_main/neuron_editing/neuron_xforms.h"
#include "marker_match_dialog.h"

#define MY_ANGLE_TICK 1
#define B_USE_NEGATIVE_SCALING	1
#define MY_PI 3.141592635
#define MY_MATCH_DIS 100 //this is squared distance, when real distance is smaller than sqrt(DIS), the point will be matched

using namespace std;

NeuronGeometryDialog::NeuronGeometryDialog(V3DPluginCallback2 * cb, V3dR_MainWindow* inwin)
{
    if(!inwin)
        return;
    v3dwin=inwin;
    callback = cb;
    v3dcontrol = callback->getView3DControl_Any3DViewer(v3dwin);

    //load neuron tree
    ntList=cb->getHandleNeuronTrees_Any3DViewer(v3dwin);
    if(ntList->size()!=2){ //this should not happen
        v3d_msg("The number of neurons in the window is not 2");
        return;
    }

    //backup neuron tree
    nt_bkList.clear();
    type_bk.clear();
    for(int i=0; i<ntList->size(); i++){
        NeuronTree nt_tmp;
        backupNeuron(ntList->at(i), nt_tmp);
        nt_bkList.append(nt_tmp);

        NeuronTree * p_tree;
        p_tree = (NeuronTree *)(&(ntList->at(i)));
        ntpList.append(p_tree);

        QList<int> typeList; typeList.clear();
        for(int j=0; j<ntList->at(i).listNeuron.size(); j++){
            typeList.append(ntList->at(i).listNeuron[j].type);
        }
        type_bk.append(typeList);

        ntpList[i]->color.r = ntpList[i]->color.g = ntpList[i]->color.b = ntpList[i]->color.a = NULL;
    }

    resetInternalStates();
    updateContent();
    v3dcontrol->enableClipBoundingBox(false);

    //get marker handle and link them to neuron
    mList = cb->getHandleImageMarkerList_Any3DViewer(v3dwin);
    cur_marker_num = mList->size();
    link_marker_neuron_force();

    if(v3dwin){
        callback->update_3DViewer(v3dwin);
    }
    //v3dcontrol->enableShowBoundingBox(false);
}


void NeuronGeometryDialog::enterEvent(QEvent *e)
{
    bool isclosed = true;
    //check if current window is closed
    if (!callback)
        return;

    //search to see if the window is still open
    QList <V3dR_MainWindow *> allWindowList = callback->getListAll3DViewers();
    for (V3DLONG i=0;i<allWindowList.size();i++)
    {
        if(allWindowList.at(i)==v3dwin){
            isclosed = false;
            break;
        }
    }

    //close the window
    if(isclosed){
        this->hide();
        QDialog::enterEvent(e);
        return;
    }

    //update marker link when there is new markers
    if(cur_marker_num != mList->size()){
        if(cur_marker_num < mList->size()){ //link the last added one
            //link_new_marker_neuron();
            pushButton_affineByMarkers->setEnabled(false);
            pushButton_linkMatchingMarker->setEnabled(false);
            pushButton_linkMarkerNeuron->setDefault(true);
            cur_marker_num = mList->size();
        }else{ //adjust the order
//            update_markers_delete();
            pushButton_affineByMarkers->setEnabled(false);
            pushButton_linkMatchingMarker->setDefault(true);
            cur_marker_num = mList->size();
        }
    }

    QDialog::enterEvent(e);
}

//basic functions
void NeuronGeometryDialog::updateContent()
{
    create();
}

void NeuronGeometryDialog::resetInternalStates()
{
    for(int i=0; i<2; i++){
        cur_shift_x[i] = cur_shift_y[i] = cur_shift_z[i] = 0;
        cur_scale_x[i] = cur_scale_y[i] = cur_scale_z[i] = 1;
        cur_gscale_x[i] = cur_gscale_y[i] = cur_gscale_z[i] = 1;
        cur_scale_r[i] = 1;
        cur_rotate_x[i] = cur_rotate_y[i] = cur_rotate_z[i] = 0;

        getNeuronTreeBound(ntList->at(i), cur_minx[i], cur_miny[i],cur_minz[i],cur_maxx[i],
                           cur_maxy[i],cur_maxz[i],cur_mmx[i],cur_mmy[i],cur_mmz[i]);

        cur_flip_x[i] = cur_flip_y[i] = cur_flip_z[i] = false;

        for(int j=0; j<16; j++) cur_tmat[i][j]=0;
        cur_tmat[i][0] = cur_tmat[i][5] = cur_tmat[i][10] = cur_tmat[i][15] = 1;
    }

    ant=0;
    stack_dir=2;
}

void NeuronGeometryDialog::resetInternalStates(int i)
{
    cur_shift_x[i] = cur_shift_y[i] = cur_shift_z[i] = 0;
    cur_scale_x[i] = cur_scale_y[i] = cur_scale_z[i] = 1;
    cur_gscale_x[i] = cur_gscale_y[i] = cur_gscale_z[i] = 1;
    cur_scale_r[i] = 1;
    cur_rotate_x[i] = cur_rotate_y[i] = cur_rotate_z[i] = 0;

    getNeuronTreeBound(ntList->at(i), cur_minx[i], cur_miny[i],cur_minz[i],cur_maxx[i],
                       cur_maxy[i],cur_maxz[i],cur_mmx[i],cur_mmy[i],cur_mmz[i]);

    cur_flip_x[i] = cur_flip_y[i] = cur_flip_z[i] = false;

    for(int j=0; j<16; j++) cur_tmat[i][j]=0;
    cur_tmat[i][0] = cur_tmat[i][5] = cur_tmat[i][10] = cur_tmat[i][15] = 1;
}

void NeuronGeometryDialog::create()
{
    setupUi(this);

    //marker operations
    pushButton_generateMarker->setEnabled(false);
    pushButton_linkMatchingMarker->setEnabled(false);
    pushButton_affineByMarkers->setEnabled(false);

    connect(pushButton_linkMarkerNeuron, SIGNAL(clicked()), this, SLOT(link_marker_neuron_force()));
    connect(pushButton_linkMatchingMarker, SIGNAL(clicked()), this, SLOT(link_markers()));
    connect(pushButton_affineByMarkers, SIGNAL(clicked()), this, SLOT(affine_markers()));

    //basic operations
    connect(pushButton_ok, SIGNAL(clicked()), this, SLOT(accept()));
    connect(pushButton_cancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(pushButton_reset, SIGNAL(clicked()), this, SLOT(reset()));
    connect(pushButton_save, SIGNAL(clicked()), this, SLOT(save_affine_mat()));

    //default values and events
    doubleSpinBox_shift_x->setRange(-100000,100000); doubleSpinBox_shift_x->setValue(cur_shift_x[ant]);
    doubleSpinBox_shift_y->setRange(-100000,100000); doubleSpinBox_shift_y->setValue(cur_shift_y[ant]);
    doubleSpinBox_shift_z->setRange(-100000,100000); doubleSpinBox_shift_z->setValue(cur_shift_z[ant]);

    connect(doubleSpinBox_shift_x, SIGNAL(valueChanged(double)), this, SLOT(shift_x(double)));
    connect(doubleSpinBox_shift_y, SIGNAL(valueChanged(double)), this, SLOT(shift_y(double)));
    connect(doubleSpinBox_shift_z, SIGNAL(valueChanged(double)), this, SLOT(shift_z(double)));

    doubleSpinBox_scale_x->setRange(1,100000); doubleSpinBox_scale_x->setValue(cur_scale_x[ant]*1000);
    doubleSpinBox_scale_y->setRange(1,100000); doubleSpinBox_scale_y->setValue(cur_scale_y[ant]*1000);
    doubleSpinBox_scale_z->setRange(1,100000); doubleSpinBox_scale_z->setValue(cur_scale_z[ant]*1000);
    doubleSpinBox_scale_r->setRange(1,100000); doubleSpinBox_scale_r->setValue(cur_scale_r[ant]*1000);

    connect(doubleSpinBox_scale_x, SIGNAL(valueChanged(double)), this, SLOT(scale_x(double)));
    connect(doubleSpinBox_scale_y, SIGNAL(valueChanged(double)), this, SLOT(scale_y(double)));
    connect(doubleSpinBox_scale_z, SIGNAL(valueChanged(double)), this, SLOT(scale_z(double)));
    connect(doubleSpinBox_scale_r, SIGNAL(valueChanged(double)), this, SLOT(scale_r(double)));

#ifdef B_USE_NEGATIVE_SCALING
    double scale_lower_limit=-100000;
#else
    double scale_lower_limit=1;
#endif

    doubleSpinBox_gscale_x->setRange(scale_lower_limit,100000); doubleSpinBox_gscale_x->setValue(cur_gscale_x[ant]*1000);
    doubleSpinBox_gscale_y->setRange(scale_lower_limit,100000); doubleSpinBox_gscale_y->setValue(cur_gscale_y[ant]*1000);
    doubleSpinBox_gscale_z->setRange(scale_lower_limit,100000); doubleSpinBox_gscale_z->setValue(cur_gscale_z[ant]*1000);

    connect(doubleSpinBox_gscale_x, SIGNAL(valueChanged(double)), this, SLOT(gscale_x(double)));
    connect(doubleSpinBox_gscale_y, SIGNAL(valueChanged(double)), this, SLOT(gscale_y(double)));
    connect(doubleSpinBox_gscale_z, SIGNAL(valueChanged(double)), this, SLOT(gscale_z(double)));

    highlight_dial(dial_x);
    highlight_dial(dial_y);
    highlight_dial(dial_z);

    dial_x->setValue(cur_rotate_x[ant]);
    dial_y->setValue(cur_rotate_y[ant]);
    dial_z->setValue(cur_rotate_z[ant]);

    connect(dial_x, SIGNAL(valueChanged(int)), this, SLOT(rotate_around_x_dial(int)));
    connect(dial_y, SIGNAL(valueChanged(int)), this, SLOT(rotate_around_y_dial(int)));
    connect(dial_z, SIGNAL(valueChanged(int)), this, SLOT(rotate_around_z_dial(int)));

    connect(this, SIGNAL(xRotationChanged(double)), doubleSpinBox_rotate_x, SLOT(setValue(double)));
    connect(this, SIGNAL(yRotationChanged(double)), doubleSpinBox_rotate_y, SLOT(setValue(double)));
    connect(this, SIGNAL(zRotationChanged(double)), doubleSpinBox_rotate_z, SLOT(setValue(double)));

    dial_x->setDisabled(true);
    dial_y->setDisabled(true);

    doubleSpinBox_rotate_x->setRange(0,360); doubleSpinBox_rotate_x->setValue(cur_rotate_x[ant]); doubleSpinBox_rotate_x->setSingleStep(0.1);
    doubleSpinBox_rotate_y->setRange(0,360); doubleSpinBox_rotate_y->setValue(cur_rotate_y[ant]); doubleSpinBox_rotate_y->setSingleStep(0.1);
    doubleSpinBox_rotate_z->setRange(0,360); doubleSpinBox_rotate_z->setValue(cur_rotate_z[ant]); doubleSpinBox_rotate_z->setSingleStep(0.1);

    connect(doubleSpinBox_rotate_x, SIGNAL(valueChanged(double)), this, SLOT(rotate_around_x_spin(double)));
    connect(doubleSpinBox_rotate_y, SIGNAL(valueChanged(double)), this, SLOT(rotate_around_y_spin(double)));
    connect(doubleSpinBox_rotate_z, SIGNAL(valueChanged(double)), this, SLOT(rotate_around_z_spin(double)));

    connect(this, SIGNAL(xRotationChanged(int)), dial_x, SLOT(setValue(int)));
    connect(this, SIGNAL(yRotationChanged(int)), dial_y, SLOT(setValue(int)));
    connect(this, SIGNAL(zRotationChanged(int)), dial_z, SLOT(setValue(int)));

    doubleSpinBox_rotate_x->setDisabled(true);
    doubleSpinBox_rotate_y->setDisabled(true);

    connect(pushButton_refresh_rcent,SIGNAL(clicked()), this, SLOT(refresh_rcent()));

    doubleSpinBox_rcent_x->setValue(cur_mmx[ant]);
    doubleSpinBox_rcent_y->setValue(cur_mmy[ant]);
    doubleSpinBox_rcent_z->setValue(cur_mmz[ant]);

//    pushButton_refresh_rcent->setEnabled(false);
//    doubleSpinBox_rcent_x->setEnabled(false);
//    doubleSpinBox_rcent_y->setEnabled(false);
//    doubleSpinBox_rcent_z->setEnabled(false);

    checkBox_flip_x->setChecked(cur_flip_x[ant]);
    checkBox_flip_y->setChecked(cur_flip_y[ant]);
    checkBox_flip_z->setChecked(cur_flip_z[ant]);

    connect(checkBox_flip_x, SIGNAL(stateChanged(int)), this, SLOT(flip_x(int)));
    connect(checkBox_flip_y, SIGNAL(stateChanged(int)), this, SLOT(flip_y(int)));
    connect(checkBox_flip_z, SIGNAL(stateChanged(int)), this, SLOT(flip_z(int)));

    comboBox_sd->addItem(QString::fromUtf8("x"));
    comboBox_sd->addItem(QString::fromUtf8("y"));
    comboBox_sd->addItem(QString::fromUtf8("z"));
    comboBox_sd->addItem(QString::fromUtf8("free"));
    comboBox_sd->setCurrentIndex(2);
    connect(comboBox_sd, SIGNAL(currentIndexChanged(int)), this, SLOT(change_stackdir(int)));

    for(int i=0; i<ntList->size(); i++)
        comboBox_ant->addItem(ntList->at(i).name);
    connect(comboBox_ant, SIGNAL(currentIndexChanged(int)), this, SLOT(change_ant(int)));

    connect(pushButton_updatebox,SIGNAL(clicked()), this, SLOT(update_boundingbox()));
    connect(pushButton_quickmove,SIGNAL(clicked()), this, SLOT(quickmove()));

    doubleSpinBox_hide->setValue(100.0);
    doubleSpinBox_hide->setRange(0,100000);
    doubleSpinBox_hide->setDisabled(true);
    connect(doubleSpinBox_hide,SIGNAL(valueChanged(double)), this, SLOT(hide_branch(double)));
    connect(checkBox_hide,SIGNAL(stateChanged(int)), this, SLOT(hide_branch_check(int)));

    doubleSpinBox_highlight_matchpoint->setValue(100.0);
    doubleSpinBox_highlight_matchpoint->setRange(0,100000);
    doubleSpinBox_highlight_matchpoint->setDisabled(true);
    connect(doubleSpinBox_highlight_matchpoint,SIGNAL(valueChanged(double)), this, SLOT(highlight_matchpoint(double)));
    connect(checkBox_highlight_matchpoint,SIGNAL(stateChanged(int)), this, SLOT(highlight_matchpoint_check(int)));

    doubleSpinBox_highlight_boundpoint->setValue(50.0);
    doubleSpinBox_highlight_boundpoint->setRange(0,100000);
    doubleSpinBox_highlight_boundpoint->setDisabled(true);
    connect(doubleSpinBox_highlight_boundpoint,SIGNAL(valueChanged(double)), this, SLOT(highlight_boundpoint(double)));
    connect(checkBox_highlight_boundpoint,SIGNAL(stateChanged(int)), this, SLOT(highlight_boundpoint_check(int)));

    connect(pushButton_neurontype,SIGNAL(clicked()), this, SLOT(change_neurontype()));
}

void NeuronGeometryDialog::change_ant(int idx)
{
    ant=idx;

    doubleSpinBox_shift_x->setValue(cur_shift_x[ant]);
    doubleSpinBox_shift_y->setValue(cur_shift_y[ant]);
    doubleSpinBox_shift_z->setValue(cur_shift_z[ant]);

    doubleSpinBox_scale_x->setValue(cur_scale_x[ant]*1000);
    doubleSpinBox_scale_y->setValue(cur_scale_y[ant]*1000);
    doubleSpinBox_scale_z->setValue(cur_scale_z[ant]*1000);
    doubleSpinBox_scale_r->setValue(cur_scale_r[ant]*1000);

    dial_x->setValue(cur_rotate_x[ant]);
    dial_y->setValue(cur_rotate_y[ant]);
    dial_z->setValue(cur_rotate_z[ant]);

    checkBox_flip_x->setChecked(cur_flip_x[ant]);
    checkBox_flip_y->setChecked(cur_flip_y[ant]);
    checkBox_flip_z->setChecked(cur_flip_z[ant]);

    doubleSpinBox_gscale_x->setValue(cur_gscale_x[ant]*1000);
    doubleSpinBox_gscale_y->setValue(cur_gscale_y[ant]*1000);
    doubleSpinBox_gscale_z->setValue(cur_gscale_z[ant]*1000);

    doubleSpinBox_rcent_x->setValue(cur_mmx[ant]);
    doubleSpinBox_rcent_y->setValue(cur_mmy[ant]);
    doubleSpinBox_rcent_z->setValue(cur_mmz[ant]);
}

void NeuronGeometryDialog::save_affine_mat()
{
    //input file name
    QString fileDefaultName = ntList->at(ant).name+QString("_amat.txt");
    //write new SWC to file
    QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
            fileDefaultName,
            QObject::tr("Supported file (*.txt)"
                ";;Affine Matrix    (*.txt)"
                ));
    if (!writeAmat(fileSaveName.toStdString().c_str(),cur_tmat[ant]))
    {
        v3d_msg("fail to write the output affine transform matrix file.");
        return;
    }
}

void NeuronGeometryDialog::reject()
{
    for(int i=0; i<ntList->size(); i++)
    {
        ntpList[i]->copyGeometry(nt_bkList.at(i));
        copyProperty(nt_bkList[i],ntList->at(i));
    }

    update_markers();

    if(v3dwin){
        callback->update_NeuronBoundingBox(v3dwin);
        callback->update_3DViewer(v3dwin);
    }

    show_branch();
    v3dcontrol->enableClipBoundingBox(true);
    QDialog::hide();
}

void NeuronGeometryDialog::accept()
{
    for(int i=0; i<ntList->size(); i++)
    {
        copyProperty(nt_bkList[i],ntList->at(i));
    }

    if(v3dwin){
        callback->update_NeuronBoundingBox(v3dwin);
        callback->update_3DViewer(v3dwin);
    }

    show_branch();
    v3dcontrol->enableClipBoundingBox(true);
    QDialog::hide();
}

void NeuronGeometryDialog::change_stackdir(int dir)
{
    stack_dir=dir;
    switch(stack_dir)
    {
    case 0:
        dial_x->setDisabled(false);
        dial_y->setDisabled(true);
        dial_z->setDisabled(true);

        doubleSpinBox_rotate_x->setDisabled(false);
        doubleSpinBox_rotate_y->setDisabled(true);
        doubleSpinBox_rotate_z->setDisabled(true);

        pushButton_quickmove->setDisabled(false);
        checkBox_hide->setEnabled(true);
        break;
    case 1:
        dial_x->setDisabled(true);
        dial_y->setDisabled(false);
        dial_z->setDisabled(true);

        doubleSpinBox_rotate_x->setDisabled(true);
        doubleSpinBox_rotate_y->setDisabled(false);
        doubleSpinBox_rotate_z->setDisabled(true);

        pushButton_quickmove->setDisabled(false);
        checkBox_hide->setEnabled(true);
        break;
    case 2:
        dial_x->setDisabled(true);
        dial_y->setDisabled(true);
        dial_z->setDisabled(false);

        doubleSpinBox_rotate_x->setDisabled(true);
        doubleSpinBox_rotate_y->setDisabled(true);
        doubleSpinBox_rotate_z->setDisabled(false);

        pushButton_quickmove->setDisabled(false);
        checkBox_hide->setEnabled(true);
        break;
    case 3:
        dial_x->setDisabled(false);
        dial_y->setDisabled(false);
        dial_z->setDisabled(false);

        doubleSpinBox_rotate_x->setDisabled(false);
        doubleSpinBox_rotate_y->setDisabled(false);
        doubleSpinBox_rotate_z->setDisabled(false);

        pushButton_quickmove->setDisabled(true);
        show_branch();
        checkBox_hide->setChecked(false);
        checkBox_hide->setEnabled(false);
        doubleSpinBox_hide->setEnabled(false);
    }

    highlight_points(false);
}


//visualization tools
void NeuronGeometryDialog::update_boundingbox()
{
    if(v3dwin){
        callback->update_NeuronBoundingBox(v3dwin);
        callback->update_3DViewer(v3dwin);
    }
}

void NeuronGeometryDialog::hide_branch(double s)
{
    if(!checkBox_hide->isChecked()){
        return;
    }

    v3dcontrol->enableClipBoundingBox(true);

    double midplan = 0, max = 0, min = 0;
    if(stack_dir==0) //x
    {
        if(cur_mmx[0]>cur_mmx[1]){
            midplan=(cur_minx[0]+cur_maxx[1])/2;
        }else{
            midplan=(cur_minx[1]+cur_maxx[0])/2;
        }
        max=cur_maxx[0]>cur_maxx[1]?cur_maxx[0]:cur_maxx[1];
        min=cur_minx[0]<cur_minx[1]?cur_minx[0]:cur_minx[1];
        v3dcontrol->setXClip0((int) ((midplan-s)-min)/(max-min)*200 );
        v3dcontrol->setXClip1((int) ((midplan+s)-min)/(max-min)*200 );
    }
    else if(stack_dir==1) //y
    {
        if(cur_mmy[0]>cur_mmy[1]){
            midplan=(cur_miny[0]+cur_maxy[1])/2;
        }else{
            midplan=(cur_miny[1]+cur_maxy[0])/2;
        }
        max=cur_maxy[0]>cur_maxy[1]?cur_maxy[0]:cur_maxy[1];
        min=cur_miny[0]<cur_miny[1]?cur_miny[0]:cur_miny[1];
        v3dcontrol->setYClip0((int) ((midplan-s)-min)/(max-min)*200 );
        v3dcontrol->setYClip1((int) ((midplan+s)-min)/(max-min)*200 );
    }
    else if(stack_dir==2) //z
    {
        if(cur_mmz[0]>cur_mmz[1]){
            midplan=(cur_minz[0]+cur_maxz[1])/2;
        }else{
            midplan=(cur_minz[1]+cur_maxz[0])/2;
        }
        max=cur_maxz[0]>cur_maxz[1]?cur_maxz[0]:cur_maxz[1];
        min=cur_minz[0]<cur_minz[1]?cur_minz[0]:cur_minz[1];
        v3dcontrol->setZClip0((int) ((midplan-s)-min)/(max-min)*200 );
        v3dcontrol->setZClip1((int) ((midplan+s)-min)/(max-min)*200 );
    }
}

void NeuronGeometryDialog::hide_branch()
{
    if(!checkBox_hide->isChecked()){
        return;
    }

    double s=doubleSpinBox_hide->value();
    hide_branch(s);
}

void NeuronGeometryDialog::show_branch()
{
    v3dcontrol->setXClip0(0);
    v3dcontrol->setYClip0(0);
    v3dcontrol->setZClip0(0);

    v3dcontrol->setXClip1(200);
    v3dcontrol->setYClip1(200);
    v3dcontrol->setZClip1(200);

    v3dcontrol->enableClipBoundingBox(false);
}

void NeuronGeometryDialog::hide_branch_check(int c)
{
    doubleSpinBox_hide->setDisabled(c == Qt::Unchecked);

    if(c == Qt::Unchecked){
        show_branch();
    }else{
        hide_branch(doubleSpinBox_hide->value());
    }
}


void NeuronGeometryDialog::highlight_boundpoint(double s)
{
    if(!checkBox_highlight_boundpoint->isChecked()){
        return;
    }

    //copy back the property
    for(int i=0; i<ntList->size(); i++)
    {
        copyType(type_bk[i], ntList->at(i));
    }

    //highlight bound point
    float span= (float) s;
    highlight_edgepoint(ntList, span, stack_dir);

    //highlight match point if necessary
    if(checkBox_highlight_matchpoint->isChecked())
    {
        span = (float) doubleSpinBox_highlight_matchpoint->value();
        for(int i=0; i<ntList->size(); i++){
            for(int j=i+1; j<ntList->size(); j++){
                highlight_adjpoint(ntList->at(i), ntList->at(j),span);
            }
        }
    }

    if(v3dwin){
        callback->update_3DViewer(v3dwin);
    }
}

void NeuronGeometryDialog::highlight_matchpoint(double s)
{
    if(!checkBox_highlight_matchpoint->isChecked()){
        return;
    }

    float span=0;

    //copy back the property
    for(int i=0; i<ntList->size(); i++)
    {
        copyType(type_bk[i], ntList->at(i));
    }

    //highlight bound point if necessary
    if(checkBox_highlight_boundpoint->isChecked())
    {
        span = (float) doubleSpinBox_highlight_boundpoint->value();
        highlight_edgepoint(ntList, span, stack_dir);
    }
    //highlight match point
    span = (float) s;
    for(int i=0; i<ntList->size(); i++){
        for(int j=i+1; j<ntList->size(); j++){
            highlight_adjpoint(ntList->at(i), ntList->at(j),span);
        }
    }

    if(v3dwin){
        callback->update_3DViewer(v3dwin);
    }
}

void NeuronGeometryDialog::highlight_points(bool force)
{
    if(force || checkBox_highlight_boundpoint->isChecked() || checkBox_highlight_matchpoint->isChecked())
    {
        float span=0;

        //copy back the property
        for(int i=0; i<ntList->size(); i++)
        {
            copyType(type_bk[i], ntList->at(i));
        }

        //highlight bound point if necessary
        if(checkBox_highlight_boundpoint->isChecked())
        {
            span = (float) doubleSpinBox_highlight_boundpoint->value();
            highlight_edgepoint(ntList, span, stack_dir);
        }
        //highlight match point if necessary
        if(checkBox_highlight_matchpoint->isChecked())
        {
            span = (float) doubleSpinBox_highlight_matchpoint->value();
            for(int i=0; i<ntList->size(); i++)
            {
                for(int j=i+1; j<ntList->size(); j++)
                {
                    highlight_adjpoint(ntList->at(i), ntList->at(j),span);
                }
            }
        }
    }

    update_markers();

    if(v3dwin){
        callback->update_3DViewer(v3dwin);
    }
}

void NeuronGeometryDialog::highlight_matchpoint_check(int c)
{
    doubleSpinBox_highlight_matchpoint->setDisabled(c == Qt::Unchecked);

    highlight_points(true);
}

void NeuronGeometryDialog::highlight_boundpoint_check(int c)
{
    doubleSpinBox_highlight_boundpoint->setDisabled(c == Qt::Unchecked);

    highlight_points(true);
}

void NeuronGeometryDialog::change_neurontype()
{
    bool ok = false;
    int type = QInputDialog::getInt(this, ntList->at(ant).name, "pick up a value (0~6)", 0, 0, 100, 1, &ok);

    if(ok)
    {
        for(int i = 0; i < type_bk[ant].size(); i++){
            type_bk[ant][i] = type;
        }

        copyType(type_bk[ant], ntList->at(ant));

        highlight_points(false);
    }
}

//marker tools
void NeuronGeometryDialog::link_markers()
{
    marker_match_dialog dialog(callback, mList);
    dialog.exec();

    for(int i=0; i<mList->size(); i++){
        mInfoList[i]=mList->at(i).comment;
    }

    pushButton_linkMatchingMarker->setDefault(false);
    pushButton_affineByMarkers->setEnabled(true);
    pushButton_affineByMarkers->setDefault(true);
}

void NeuronGeometryDialog::link_marker_neuron_force()
{
    if(mList->size()<=0){
//        v3d_msg("Cannot find markers. Please define some markers first");
//        qDebug("Cannot find markers. Please define some markers first");
        return;
    }

    ImageMarker *p;
    double dis;
    int info[4];
    vector<int> count(ntList->size(),0);
    mInfoList.clear();

    for(int i=0; i<mList->size(); i++){
        double mdis=MY_MATCH_DIS;
        int nid=-1;
        int pid=0;
        //qDebug()<<mList->at(i).x<<":"<<mList->at(i).y<<":"<<mList->at(i).z<<endl;
        for(int j=0; j<ntList->size(); j++){
            for(int k=0; k<ntList->at(j).listNeuron.size(); k++){
                dis=NTDIS(ntList->at(j).listNeuron[k],mList->at(i));
                if(dis<mdis){
                    mdis=dis;
                    nid=j;
                    pid=k;
                }
            }
        }
        p = (ImageMarker *)&(mList->at(i));
        p->name=QString::number(i);
        if(nid>=0){
            p->comment=QString::number(nid) + " " + QString::number(pid) + " -1";
            mInfoList.append(QString::number(nid) + " " + QString::number(pid) + " -1");
            count[nid]++;
        }else{
            mInfoList.append(QString::number(-1));
        }
    }

    cur_marker_num = mList->size();

    update_markers();

    pushButton_linkMatchingMarker->setEnabled(true);
    pushButton_affineByMarkers->setEnabled(false);
    pushButton_linkMarkerNeuron->setDefault(false);
    pushButton_linkMatchingMarker->setDefault(true);

    if(mList->size()>=0)
        v3d_msg(QString::number(mList->size())+QString::fromUtf8(" makers found\n")+
                QString::number(count[0])+QString::fromUtf8(" markers are aligned to neuron 0\n")+
                QString::number(count[1])+QString::fromUtf8(" markers are aligned to neuron 1"));
}

void NeuronGeometryDialog::link_new_marker_neuron()
{
    if(mList->size()<=0 || cur_marker_num>=mList->size()){
        return;
    }

    if(mList->size()!=cur_marker_num+1){ //should not happen
        v3d_msg("error: unexpected thing happens when linking new marker to neuron. Please check and click relink button if necessary.");
        return;
    }

    ImageMarker *p = (ImageMarker *)&(mList->last());
    double dis;
    int info[4];
    double mdis=MY_MATCH_DIS;
    int nid=-1;
    int pid=0;
    //qDebug()<<mList->at(i).x<<":"<<mList->at(i).y<<":"<<mList->at(i).z<<endl;
    for(int j=0; j<ntList->size(); j++){
        for(int k=0; k<ntList->at(j).listNeuron.size(); k++){
            dis=NTDIS(ntList->at(j).listNeuron[k],mList->last());
            if(dis<mdis){
                mdis=dis;
                nid=j;
                pid=k;
            }
        }
    }
    p->name=QString::number(mList->size()-1);
    if(nid>=0){
        p->comment=QString::number(nid) + " " + QString::number(pid) + " -1";
        mInfoList.append(QString::number(nid) + " " + QString::number(pid) + " -1");
    }else{
        mInfoList.append(QString::number(-1));
    }

    for(int i=0; i<mList->size()-1; i++){
         p = (ImageMarker *)&(mList->at(i));
         p->comment = mInfoList[i];
         p->name = QString::number(i);
    }

    update_markers();
}

void NeuronGeometryDialog::update_markers_delete()
{
    //find the one that is deleted first
    int dmid=-1;    //the id of deleted neuron
    for(int i=0; i<mList->size();i++){
        if(mList->at(i).name != QString::number(i)){
            dmid=i;
            break;
        }
    }
    //update the information of the rest
    mInfoList.removeAt(dmid);
    int info[4];
    for(int i=0; i<mList->size();i++){
        get_marker_info(mList->at(i),info);
        if(info[2]==i) info[2]=-1;
        if(info[2]>i) info[2]--;
        if(info[3]!=i){
            if(info[3]-1!=i){ //should not happen
                v3d_msg("error: unexpected things happend for marker info, please check marker: "+QString::number(i));
            }else{
                info[3]--;
            }
        }
        update_marker_info(mList->at(i),info);
        mInfoList[i] = mList->at(i).comment;
    }
}

void NeuronGeometryDialog::update_markers()
{
    if(mList->size()<=0){
        return;
    }
    int info[4];
    ImageMarker *p;
    //get info of each marker
    for(int i=0; i<mList->size(); i++){
        if(get_marker_info(mList->at(i),info))
        {
            if(info[0]>-1&&info[0]<ntList->size()){
                if(info[1]>-1&&info[1]<ntList->at(info[0]).listNeuron.size())
                {
                    p = (ImageMarker *)&(mList->at(i));
                    p->x=ntList->at(info[0]).listNeuron.at(info[1]).x;
                    p->y=ntList->at(info[0]).listNeuron.at(info[1]).y;
                    p->z=ntList->at(info[0]).listNeuron.at(info[1]).z;
//                    p->type=ntList->at(info[0]).listNeuron.at(info[1]).type;
//                    p->color.r=ntList->at(info[0]).listNeuron.at(info[1]).color.r;
//                    p->color.g=ntList->at(info[0]).listNeuron.at(info[1]).color.g;
//                    p->color.b=ntList->at(info[0]).listNeuron.at(info[1]).color.b;
//                    p->color.a=ntList->at(info[0]).listNeuron.at(info[1]).color.a;
                }
            }
        }
    }
}

void NeuronGeometryDialog::affine_markers()
{
    pushButton_affineByMarkers->setDefault(false);

    //get markers informations
    int info[4];
    QList<int> nidList;
    QList<int> pidList;
    QList<int> midList;
    for(int i=0; i<mList->size(); i++){
        if(get_marker_info(mList->at(i),info)){
            nidList.append(info[0]);
            pidList.append(info[1]);
            midList.append(info[2]);
        }else{
            nidList.append(-1);
            pidList.append(-1);
            midList.append(-1);
        }
    }
    if(nidList.size()!=mList->size()){ //should not happen
        qDebug("unexpected error in acquring marker informations");
        return;
    }

    //get matched coordinates
    QList<XYZ> c0;
    QList<XYZ> c1;
    for(int i=0; i<nidList.size(); i++)
    {
        if(nidList[i]==0){ //do have info and is the first neuron (only match once)
            int mid0=midList[i];
            if(mid0>-1 && mid0<midList.size()){ //do have eligable match
                if(i==midList[mid0]){ //check cross match
                    if(nidList[mid0]==1){ //make sure from different neurons
                        int pid0=pidList[i];
                        int pid1=pidList[mid0];
                        if(pid0>-1 && pid0<ntList->at(0).listNeuron.size() &&
                                pid1>-1 && pid1<ntList->at(1).listNeuron.size()){ //check neuron point eligibility
                            XYZ tmp0(XYZ(ntList->at(0).listNeuron[pid0]));
                            XYZ tmp1(XYZ(ntList->at(1).listNeuron[pid1]));
                            c0.append(tmp0);
                            c1.append(tmp1);
                        }
                    }
                }
            }
        }
    }
    qDebug()<<"identified "<<c0.size()<<" pairs of matching point";

    //calculate transformation - align 1 to 0
    double shift_x,shift_y,shift_z,angle_r,cent_x,cent_y,cent_z;
    if(ant=0){
        compute_affine_4dof(c1,c0,shift_x,shift_y,shift_z,angle_r,cent_x,cent_y,cent_z,stack_dir);
    }else if(ant=1){
        compute_affine_4dof(c0,c1,shift_x,shift_y,shift_z,angle_r,cent_x,cent_y,cent_z,stack_dir);
    }

    //apply transformation matrix
    //shift
    doubleSpinBox_shift_x->setValue(cur_shift_x[ant]+shift_x);
    doubleSpinBox_shift_y->setValue(cur_shift_y[ant]+shift_y);
    doubleSpinBox_shift_z->setValue(cur_shift_z[ant]+shift_z);
    //rotation
    if(angle_r!=0){
        doubleSpinBox_rcent_x->setValue(cent_x);
        doubleSpinBox_rcent_y->setValue(cent_y);
        doubleSpinBox_rcent_z->setValue(cent_z);
        switch(stack_dir)
        {
        case 0:
            doubleSpinBox_rotate_x->setValue(cur_rotate_x[ant]+angle_r);
            break;
        case 1:
            doubleSpinBox_rotate_y->setValue(cur_rotate_y[ant]+angle_r);
            break;
        case 2:
            doubleSpinBox_rotate_z->setValue(cur_rotate_z[ant]+angle_r);
            break;
    }
    }
    qDebug()<<"done matching"<<endl;
}

//geometry changing tools
void NeuronGeometryDialog::reset()
{
    ntpList[ant]->copyGeometry(nt_bkList[ant]);
    copyProperty(nt_bkList.at(ant),ntList->at(ant));
    copyType(nt_bkList.at(ant),type_bk[ant]);
    update_markers();

    resetInternalStates(ant);

    doubleSpinBox_shift_x->setValue(cur_shift_x[ant]);
    doubleSpinBox_shift_y->setValue(cur_shift_y[ant]);
    doubleSpinBox_shift_z->setValue(cur_shift_z[ant]);

    doubleSpinBox_scale_x->setValue(cur_scale_x[ant]*1000);
    doubleSpinBox_scale_y->setValue(cur_scale_y[ant]*1000);
    doubleSpinBox_scale_z->setValue(cur_scale_z[ant]*1000);
    doubleSpinBox_scale_r->setValue(cur_scale_r[ant]*1000);

    dial_x->setValue(cur_rotate_x[ant]);
    dial_y->setValue(cur_rotate_y[ant]);
    dial_z->setValue(cur_rotate_z[ant]);

    checkBox_flip_x->setChecked(cur_flip_x[ant]);
    checkBox_flip_y->setChecked(cur_flip_y[ant]);
    checkBox_flip_z->setChecked(cur_flip_z[ant]);

    doubleSpinBox_gscale_x->setValue(cur_gscale_x[ant]*1000);
    doubleSpinBox_gscale_y->setValue(cur_gscale_y[ant]*1000);
    doubleSpinBox_gscale_z->setValue(cur_gscale_z[ant]*1000);

    doubleSpinBox_rcent_x->setValue(cur_mmx[ant]);
    doubleSpinBox_rcent_y->setValue(cur_mmy[ant]);
    doubleSpinBox_rcent_z->setValue(cur_mmz[ant]);

    if(v3dwin){
        callback->update_NeuronBoundingBox(v3dwin);
    }
    highlight_points(false);
}

void NeuronGeometryDialog::quickmove()
{
    if(stack_dir==3)
        return;

    //get the stack that has larger coordinate
    QString text;
    int first_nt = ant;
    if(stack_dir == 0)
        text = "Select the stack that has larger X coordinate (right)";
    else if(stack_dir == 1)
        text = "Select the stack that has larger Y coordinate (front)";
    if(stack_dir == 2)
        text = "Select the stack that has larger Z coordinate (top)";
    QStringList items;
    for(int i=0; i<ntList->size(); i++){
        items.append(ntList->at(i).name);
    }
    bool ok;
    QString firstitem = QInputDialog::getItem(this, QString::fromUtf8("Quick Stick Neuron"), text, items, ant, false, &ok);
    if(!ok)
        return;
    for(int i=0; i<ntList->size(); i++){
        if(firstitem==ntList->at(i).name)
        {
            first_nt=i;
            break;
        }
    }

    double delta = quickMoveNeuron(ntList,ant,stack_dir,first_nt);
    switch(stack_dir){
    case 0:
        cur_minx[ant]+=delta;
        cur_maxx[ant]+=delta;
        cur_mmx[ant]+=delta;
        doubleSpinBox_rcent_x->setValue(cur_mmx[ant]);
        cur_shift_x[ant]+=delta;
        cur_tmat[ant][3]+=delta;
        doubleSpinBox_shift_x->setValue(cur_shift_x[ant]);
        break;
    case 1:
        cur_miny[ant]+=delta;
        cur_maxy[ant]+=delta;
        cur_mmy[ant]+=delta;
        doubleSpinBox_rcent_y->setValue(cur_mmy[ant]);
        cur_shift_y[ant]+=delta;
        cur_tmat[ant][7]+=delta;
        doubleSpinBox_shift_y->setValue(cur_shift_y[ant]);
        break;
    case 2:
        cur_minz[ant]+=delta;
        cur_maxz[ant]+=delta;
        cur_mmz[ant]+=delta;
        doubleSpinBox_rcent_z->setValue(cur_mmz[ant]);
        cur_shift_z[ant]+=delta;
        cur_tmat[ant][11]+=delta;
        doubleSpinBox_shift_z->setValue(cur_shift_z[ant]);
    }

    if(v3dwin){
        callback->update_NeuronBoundingBox(v3dwin);
    }
    highlight_points(false);
}


void highlight_dial(QDial *d)
{
    QPalette pe = d->palette();
    pe.setBrush(QPalette::Button, pe.highlight());
    d->setPalette(pe);

    d->setRange(0, 360 * MY_ANGLE_TICK);
    d->setSingleStep(1);
    d->setPageStep(10 * MY_ANGLE_TICK);
    d->setWrapping(true);
    d->setNotchesVisible(true);
}

double NORMALIZE_ROTATION_Angle( double angle )
{
    while (angle < -180)   angle += 360;
    while (angle >  180)   angle -= 360;
    return angle;
}

void NeuronGeometryDialog::shift_x(double s)
{
    proc_neuron_add_offset(ntpList[ant], s-cur_shift_x[ant], 0, 0);
    cur_minx[ant]+=s-cur_shift_x[ant];
    cur_maxx[ant]+=s-cur_shift_x[ant];
    cur_mmx[ant]+=s-cur_shift_x[ant]; doubleSpinBox_rcent_x->setValue(cur_mmx[ant]);
    cur_tmat[ant][3] += s-cur_shift_x[ant];
    cur_shift_x[ant] = s;

    if(stack_dir==0){
        hide_branch();
    }
    highlight_points(false);
}


void NeuronGeometryDialog::shift_y(double s)
{
    proc_neuron_add_offset(ntpList[ant], 0, s-cur_shift_y[ant], 0);
    cur_miny[ant]+=s-cur_shift_y[ant];
    cur_maxy[ant]+=s-cur_shift_y[ant];
    cur_mmy[ant]+=s-cur_shift_y[ant]; doubleSpinBox_rcent_y->setValue(cur_mmy[ant]);
    cur_tmat[ant][7] += s-cur_shift_y[ant];
    cur_shift_y[ant] = s;

    if(stack_dir==1){
        hide_branch();
    }
    highlight_points(false);
}
void NeuronGeometryDialog::shift_z(double s)
{
    proc_neuron_add_offset(ntpList[ant], 0, 0, s-cur_shift_z[ant]);
    cur_minz[ant]+=s-cur_shift_z[ant];
    cur_maxz[ant]+=s-cur_shift_z[ant];
    cur_mmz[ant]+=s-cur_shift_z[ant]; doubleSpinBox_rcent_z->setValue(cur_mmz[ant]);
    cur_tmat[ant][11] += s-cur_shift_z[ant];
    cur_shift_z[ant] = s;

    if(stack_dir==2){
        hide_branch();
    }
    highlight_points(false);
}

void NeuronGeometryDialog::scale_x(double s)
{
    proc_neuron_multiply_factor(ntpList[ant], s/(cur_scale_x[ant]*1000), 1, 1);
    cur_tmat[ant][0] *= s/(cur_scale_x[ant]*1000);
    cur_tmat[ant][3] += (1-s/(cur_scale_x[ant]*1000))*cur_mmx[ant];
    cur_minx[ant]=(cur_minx[ant]-cur_mmx[ant])*s/(cur_scale_x[ant]*1000)+cur_mmx[ant];
    cur_maxx[ant]=(cur_maxx[ant]-cur_mmx[ant])*s/(cur_scale_x[ant]*1000)+cur_mmx[ant];
    cur_scale_x[ant] = s/1000.0;

    if(stack_dir==0){
        hide_branch();
    }
    highlight_points(false);
}

void NeuronGeometryDialog::scale_y(double s)
{
    proc_neuron_multiply_factor(ntpList[ant], 1, s/(cur_scale_y[ant]*1000), 1);
    cur_tmat[ant][5] *= s/(cur_scale_y[ant]*1000);
    cur_tmat[ant][7] += (1-s/(cur_scale_y[ant]*1000))*cur_mmy[ant];
    cur_miny[ant]=(cur_miny[ant]-cur_mmy[ant])*s/(cur_scale_y[ant]*1000)+cur_mmy[ant];
    cur_maxy[ant]=(cur_maxy[ant]-cur_mmy[ant])*s/(cur_scale_y[ant]*1000)+cur_mmy[ant];
    cur_scale_y[ant] = s/1000.0;

    if(stack_dir==1){
        hide_branch();
    }
    highlight_points(false);
}

void NeuronGeometryDialog::scale_z(double s)
{
    proc_neuron_multiply_factor(ntpList[ant], 1, 1, s/(cur_scale_z[ant]*1000));
    cur_tmat[ant][10] *= s/(cur_scale_z[ant]*1000);
    cur_tmat[ant][11] += (1-s/(cur_scale_z[ant]*1000))*cur_mmz[ant];
    cur_minz[ant]=(cur_minz[ant]-cur_mmz[ant])*s/(cur_scale_z[ant]*1000)+cur_mmz[ant];
    cur_maxz[ant]=(cur_maxz[ant]-cur_mmz[ant])*s/(cur_scale_z[ant]*1000)+cur_mmz[ant];
    cur_scale_z[ant] = s/1000.0;

    if(stack_dir==2){
        hide_branch();
    }
    highlight_points(false);
}
void NeuronGeometryDialog::gscale_x(double s)
{
#ifdef B_USE_NEGATIVE_SCALING
    if (s<0.0001 && s>=0) {s=0.0001; QMessageBox::warning(0, "invalid value input", "You have entered a very small value which is not allowed. V3D will just use 0.0001 to replace it.");}
    else if (s<0 && s>-0.0001) {s=-0.0001; QMessageBox::warning(0, "invalid value input", "You have entered a very small value which is not allowed. V3D will just use -0.0001 to replace it.");}
#endif

    proc_neuron_gmultiply_factor(ntpList[ant], s/(cur_gscale_x[ant]*1000), 1, 1);
    cur_tmat[ant][0] *= s/(cur_gscale_x[ant]*1000);
    cur_minx[ant] *= s/(cur_gscale_x[ant]*1000);
    cur_maxx[ant] *= s/(cur_gscale_x[ant]*1000);
    cur_mmx[ant] *= s/(cur_gscale_x[ant]*1000);  doubleSpinBox_rcent_x->setValue(cur_mmx[ant]);
    cur_gscale_x[ant] = s/1000.0;

    if(stack_dir==0){
        hide_branch();
    }
    highlight_points(false);
}

void NeuronGeometryDialog::gscale_y(double s)
{
#ifdef B_USE_NEGATIVE_SCALING
    if (s<0.0001 && s>=0) {s=0.0001; QMessageBox::warning(0, "invalid value input", "You have entered a very small value which is not allowed. V3D will just use 0.0001 to replace it.");}
    else if (s<0 && s>-0.0001) {s=-0.0001; QMessageBox::warning(0, "invalid value input", "You have entered a very small value which is not allowed. V3D will just use -0.0001 to replace it.");}
#endif

    proc_neuron_gmultiply_factor(ntpList[ant], 1, s/(cur_gscale_y[ant]*1000), 1);
    cur_tmat[ant][5] *= s/(cur_gscale_y[ant]*1000);
    cur_miny[ant] *= s/(cur_gscale_y[ant]*1000);
    cur_maxy[ant] *= s/(cur_gscale_y[ant]*1000);
    cur_mmy[ant] *= s/(cur_gscale_y[ant]*1000); doubleSpinBox_rcent_y->setValue(cur_mmy[ant]);
    cur_gscale_y[ant] = s/1000.0;

    if(stack_dir==1){
        hide_branch();
    }
    highlight_points(false);
}

void NeuronGeometryDialog::gscale_z(double s)
{
#ifdef B_USE_NEGATIVE_SCALING
    if (s<0.0001 && s>=0) {s=0.0001; QMessageBox::warning(0, "invalid value input", "You have entered a very small value which is not allowed. V3D will just use 0.0001 to replace it.");}
    else if (s<0 && s>-0.0001) {s=-0.0001; QMessageBox::warning(0, "invalid value input", "You have entered a very small value which is not allowed. V3D will just use -0.0001 to replace it.");}
#endif

    proc_neuron_gmultiply_factor(ntpList[ant], 1, 1, s/(cur_gscale_z[ant]*1000));
    cur_tmat[ant][10] *= s/(cur_gscale_z[ant]*1000);
    cur_minz[ant] *= s/(cur_gscale_z[ant]*1000);
    cur_maxz[ant] *= s/(cur_gscale_z[ant]*1000);
    cur_mmz[ant] *= s/(cur_gscale_z[ant]*1000); doubleSpinBox_rcent_z->setValue(cur_mmz[ant]);
    cur_gscale_z[ant] = s/1000.0;

    if(stack_dir==2){
        hide_branch();
    }
    highlight_points(false);
}

void NeuronGeometryDialog::scale_r(double s)
{
    proc_neuron_multiply_factor_radius(ntpList[ant], s/(cur_scale_r[ant]*1000));
    cur_scale_r[ant] = s/1000.0;

    highlight_points(false);
}

void NeuronGeometryDialog::rotate_around_x_dial(int v)
{
    double angleval=((double)v)/MY_ANGLE_TICK;

    emit xRotationChanged(angleval);
}

void NeuronGeometryDialog::rotate_around_y_dial(int v)
{
    double angleval=((double)v)/MY_ANGLE_TICK;

    emit yRotationChanged(angleval);
}

void NeuronGeometryDialog::rotate_around_z_dial(int v)
{
    double angleval=((double)v)/MY_ANGLE_TICK;

    emit zRotationChanged(angleval);
}

void NeuronGeometryDialog::rotate_around_x_spin(double s)
{
    double afmatrix[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    double a = NORMALIZE_ROTATION_Angle( s-cur_rotate_x[ant] ) / 180.0 * MY_PI;
    if(fabs(a)>1e-5)
    {
        afmatrix[5] = cos(a); afmatrix[6] = -sin(a);
        afmatrix[9] = -afmatrix[6]; afmatrix[10] = afmatrix[5];
        proc_neuron_affine_around_center(ntpList[ant], afmatrix, doubleSpinBox_rcent_x->value(), doubleSpinBox_rcent_y->value(), doubleSpinBox_rcent_z->value());
        multiplyAmat_centerRotate(afmatrix,cur_tmat[ant], doubleSpinBox_rcent_x->value(), doubleSpinBox_rcent_y->value(), doubleSpinBox_rcent_z->value());
        getNeuronTreeBound(ntList->at(ant), cur_minx[ant], cur_miny[ant], cur_minz[ant], cur_maxx[ant], cur_maxy[ant], cur_maxz[ant], cur_mmx[ant], cur_mmy[ant], cur_mmz[ant]);
        cur_rotate_x[ant] = s;
        int dialval = (int) s*MY_ANGLE_TICK;
        emit xRotationChanged(dialval);

        highlight_points(false);
    }
}

void NeuronGeometryDialog::rotate_around_y_spin(double s)
{
    double afmatrix[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    double a = NORMALIZE_ROTATION_Angle( s-cur_rotate_y[ant] ) / 180.0 * MY_PI;
    if(fabs(a)>1e-5)
    {
        afmatrix[0] = cos(a); afmatrix[2] = sin(a);
        afmatrix[8] = -afmatrix[2]; afmatrix[10] = afmatrix[0];
        proc_neuron_affine_around_center(ntpList[ant], afmatrix, doubleSpinBox_rcent_x->value(), doubleSpinBox_rcent_y->value(), doubleSpinBox_rcent_z->value());
        multiplyAmat_centerRotate(afmatrix,cur_tmat[ant], doubleSpinBox_rcent_x->value(), doubleSpinBox_rcent_y->value(), doubleSpinBox_rcent_z->value());
        getNeuronTreeBound(ntList->at(ant), cur_minx[ant], cur_miny[ant], cur_minz[ant], cur_maxx[ant], cur_maxy[ant], cur_maxz[ant], cur_mmx[ant], cur_mmy[ant], cur_mmz[ant]);
        cur_rotate_y[ant] = s;
        int dialval = (int) s*MY_ANGLE_TICK;
        emit yRotationChanged(dialval);

        highlight_points(false);
    }
}

void NeuronGeometryDialog::rotate_around_z_spin(double s)
{
    double afmatrix[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    double a = NORMALIZE_ROTATION_Angle( s-cur_rotate_z[ant] ) / 180.0 * MY_PI;
    if(fabs(a)>1e-5)
    {
        afmatrix[0] = cos(a); afmatrix[1] = -sin(a);
        afmatrix[4] = -afmatrix[1]; afmatrix[5] = afmatrix[0];
        proc_neuron_affine_around_center(ntpList[ant], afmatrix, doubleSpinBox_rcent_x->value(), doubleSpinBox_rcent_y->value(), doubleSpinBox_rcent_z->value());
        multiplyAmat_centerRotate(afmatrix,cur_tmat[ant], doubleSpinBox_rcent_x->value(), doubleSpinBox_rcent_y->value(), doubleSpinBox_rcent_z->value());
        getNeuronTreeBound(ntList->at(ant), cur_minx[ant], cur_miny[ant], cur_minz[ant], cur_maxx[ant], cur_maxy[ant], cur_maxz[ant], cur_mmx[ant], cur_mmy[ant], cur_mmz[ant]);
        cur_rotate_z[ant] = s;
        int dialval = (int) s*MY_ANGLE_TICK;
        emit zRotationChanged(dialval);

        highlight_points(false);
    }
}

void NeuronGeometryDialog::refresh_rcent()
{
    doubleSpinBox_rcent_x->setValue(cur_mmx[ant]);
    doubleSpinBox_rcent_y->setValue(cur_mmy[ant]);
    doubleSpinBox_rcent_z->setValue(cur_mmz[ant]);
}

void NeuronGeometryDialog::flip_x(int v)
{
    proc_neuron_mirror(ntpList[ant], true, false, false);
    cur_flip_x[ant] = checkBox_flip_x->isChecked();
    cur_tmat[ant][0] *= -1;
    double tmp;
    tmp=cur_minx[ant]; cur_minx[ant]=cur_maxx[ant]; cur_maxx[ant]=tmp;

    highlight_points(false);
}

void NeuronGeometryDialog::flip_y(int v)
{
    proc_neuron_mirror(ntpList[ant], false, true, false);
    cur_flip_y[ant] = checkBox_flip_y->isChecked();
    cur_tmat[ant][5] *= -1;
    double tmp;
    tmp=cur_miny[ant]; cur_miny[ant]=cur_maxy[ant]; cur_maxy[ant]=tmp;

    highlight_points(false);
}

void NeuronGeometryDialog::flip_z(int v)
{
    proc_neuron_mirror(ntpList[ant], false, false, true);
    cur_flip_z[ant] = checkBox_flip_z->isChecked();
    cur_tmat[ant][10] *= -1;
    double tmp;
    tmp=cur_minz[ant]; cur_minz[ant]=cur_maxz[ant]; cur_maxz[ant]=tmp;

    highlight_points(false);
}


