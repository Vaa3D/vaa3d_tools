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

#define MY_ANGLE_TICK 16
#define B_USE_NEGATIVE_SCALING	1
#define MY_PI 3.141592635

NeuronGeometryDialog::NeuronGeometryDialog(V3DPluginCallback2 * cb, V3dR_MainWindow* inwin)
{
    v3dwin=inwin;
    callback = cb;

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
    }

    resetInternalStates();

    updateContent();
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

        cur_cx[i]=cur_cy[i]=cur_cz[i]=0;
        getNeuronTreeCenter(ntpList[i], cur_cx[i], cur_cy[i], cur_cz[i]);

        cur_flip_x[i] = cur_flip_y[i] = cur_flip_z[i] = false;
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

    cur_cx[i]=cur_cy[i]=cur_cz[i]=0;
    getNeuronTreeCenter(ntpList[i], cur_cx[i], cur_cy[i], cur_cz[i]);

    cur_flip_x[i] = cur_flip_y[i] = cur_flip_z[i] = false;
}

void NeuronGeometryDialog::create()
{
    setupUi(this);

    connect(pushButton_ok, SIGNAL(clicked()), this, SLOT(resetaccept()));
    connect(pushButton_cancel, SIGNAL(clicked()), this, SLOT(resetreject()));
    connect(pushButton_reset, SIGNAL(clicked()), this, SLOT(reset()));

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

    connect(dial_x, SIGNAL(valueChanged(int)), this, SLOT(rotate_around_x(int)));
    connect(dial_y, SIGNAL(valueChanged(int)), this, SLOT(rotate_around_y(int)));
    connect(dial_z, SIGNAL(valueChanged(int)), this, SLOT(rotate_around_z(int)));

    dial_x->setDisabled(true);
    dial_y->setDisabled(true);

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

    checkBox_hide->setDisabled(true);
    doubleSpinBox_hide->setValue(100.0);
    doubleSpinBox_hide->setRange(0,100000);
    doubleSpinBox_hide->setDisabled(true);

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
}

void NeuronGeometryDialog::resetreject()
{
    for(int i=0; i<ntList->size(); i++)
    {
        ntpList[i]->copyGeometry(nt_bkList.at(i));
        copyProperty(nt_bkList[i],ntList->at(i));
    }

    callback->update_NeuronBoundingBox(v3dwin);
    callback->update_3DViewer(v3dwin);

    hide();
}

void NeuronGeometryDialog::resetaccept()
{
    for(int i=0; i<ntList->size(); i++)
    {
        copyProperty(nt_bkList[i],ntList->at(i));
    }

    callback->update_NeuronBoundingBox(v3dwin);
    callback->update_3DViewer(v3dwin);

    hide();
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
        pushButton_quickmove->setDisabled(false);
        break;
    case 1:
        dial_x->setDisabled(true);
        dial_y->setDisabled(false);
        dial_z->setDisabled(true);
        pushButton_quickmove->setDisabled(false);
        break;
    case 2:
        dial_x->setDisabled(true);
        dial_y->setDisabled(true);
        dial_z->setDisabled(false);
        pushButton_quickmove->setDisabled(false);
        break;
    case 3:
        dial_x->setDisabled(false);
        dial_y->setDisabled(false);
        dial_z->setDisabled(false);
        pushButton_quickmove->setDisabled(true);
    }

    highlight_points();
}


//visualization tools
void NeuronGeometryDialog::update_boundingbox()
{
    callback->update_NeuronBoundingBox(v3dwin);
    callback->update_3DViewer(v3dwin);
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

    callback->update_3DViewer(v3dwin);
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

    callback->update_3DViewer(v3dwin);
}

void NeuronGeometryDialog::highlight_points()
{
    if((checkBox_highlight_boundpoint->isChecked()) || (checkBox_highlight_matchpoint->isChecked())){
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
            for(int i=0; i<ntList->size(); i++){
                for(int j=i+1; j<ntList->size(); j++){
                    highlight_adjpoint(ntList->at(i), ntList->at(j),span);
                }
            }
        }
    }

    callback->update_3DViewer(v3dwin);
}

void NeuronGeometryDialog::highlight_matchpoint_check(int c)
{
    doubleSpinBox_highlight_matchpoint->setDisabled(c == Qt::Unchecked);

    highlight_points();
}

void NeuronGeometryDialog::highlight_boundpoint_check(int c)
{
    doubleSpinBox_highlight_boundpoint->setDisabled(c == Qt::Unchecked);

    highlight_points();
}

void NeuronGeometryDialog::change_neurontype()
{
    bool ok;
    int type = QInputDialog::getInt(this, ntList->at(ant).name, "pick up a value (0~6)", 0, 0, 100, 1, &ok);

    if(ok)
    {
        for(int i = 0; i < type_bk[ant].size(); i++){
            type_bk[ant][i] = type;
        }

        highlight_points();
    }
}

//geometry changing tools
void NeuronGeometryDialog::reset()
{
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

    ntpList[ant]->copyGeometry(nt_bkList[ant]);
    copyProperty(nt_bkList.at(ant),ntList->at(ant));

    callback->update_NeuronBoundingBox(v3dwin);
    highlight_points();
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
        cur_cx[ant]+=delta;
        cur_shift_x[ant]+=delta;
        doubleSpinBox_shift_x->setValue(cur_shift_x[ant]);
        break;
    case 1:
        cur_cy[ant]+=delta;
        cur_shift_y[ant]+=delta;
        doubleSpinBox_shift_y->setValue(cur_shift_y[ant]);
        break;
    case 2:
        cur_cz[ant]+=delta;
        cur_shift_z[ant]+=delta;
        doubleSpinBox_shift_z->setValue(cur_shift_z[ant]);
    }

    callback->update_NeuronBoundingBox(v3dwin);
    highlight_points();
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
    angle/=MY_ANGLE_TICK;
    while (angle < -180)   angle += 360;
    while (angle >  180)   angle -= 360;
    return angle;
}

void NeuronGeometryDialog::shift_x(double s)
{
    proc_neuron_add_offset(ntpList[ant], s-cur_shift_x[ant], 0, 0);
    cur_cx[ant] += s-cur_shift_x[ant];
    cur_shift_x[ant] = s;

    highlight_points();
}


void NeuronGeometryDialog::shift_y(double s)
{
    proc_neuron_add_offset(ntpList[ant], 0, s-cur_shift_y[ant], 0);
    cur_cy[ant] += s-cur_shift_y[ant];
    cur_shift_y[ant] = s;

    highlight_points();
}
void NeuronGeometryDialog::shift_z(double s)
{
    proc_neuron_add_offset(ntpList[ant], 0, 0, s-cur_shift_z[ant]);
    cur_cz[ant] += s-cur_shift_z[ant];
    cur_shift_z[ant] = s;

    highlight_points();
}

void NeuronGeometryDialog::scale_x(double s)
{
    proc_neuron_multiply_factor(ntpList[ant], s/(cur_scale_x[ant]*1000), 1, 1);
    cur_scale_x[ant] = s/1000.0;

    highlight_points();
}

void NeuronGeometryDialog::scale_y(double s)
{
    proc_neuron_multiply_factor(ntpList[ant], 1, s/(cur_scale_y[ant]*1000), 1);
    cur_scale_y[ant] = s/1000.0;

    highlight_points();
}

void NeuronGeometryDialog::scale_z(double s)
{
    proc_neuron_multiply_factor(ntpList[ant], 1, 1, s/(cur_scale_z[ant]*1000));
    cur_scale_z[ant] = s/1000.0;

    highlight_points();
}
void NeuronGeometryDialog::gscale_x(double s)
{
#ifdef B_USE_NEGATIVE_SCALING
    if (s<0.0001 && s>=0) {s=0.0001; QMessageBox::warning(0, "invalid value input", "You have entered a very small value which is not allowed. V3D will just use 0.0001 to replace it.");}
    else if (s<0 && s>-0.0001) {s=-0.0001; QMessageBox::warning(0, "invalid value input", "You have entered a very small value which is not allowed. V3D will just use -0.0001 to replace it.");}
#endif

    proc_neuron_gmultiply_factor(ntpList[ant], s/(cur_gscale_x[ant]*1000), 1, 1);
    cur_gscale_x[ant] = s/1000.0;

    highlight_points();
}

void NeuronGeometryDialog::gscale_y(double s)
{
#ifdef B_USE_NEGATIVE_SCALING
    if (s<0.0001 && s>=0) {s=0.0001; QMessageBox::warning(0, "invalid value input", "You have entered a very small value which is not allowed. V3D will just use 0.0001 to replace it.");}
    else if (s<0 && s>-0.0001) {s=-0.0001; QMessageBox::warning(0, "invalid value input", "You have entered a very small value which is not allowed. V3D will just use -0.0001 to replace it.");}
#endif

    proc_neuron_gmultiply_factor(ntpList[ant], 1, s/(cur_gscale_y[ant]*1000), 1);
    cur_gscale_y[ant] = s/1000.0;

    highlight_points();
}

void NeuronGeometryDialog::gscale_z(double s)
{
#ifdef B_USE_NEGATIVE_SCALING
    if (s<0.0001 && s>=0) {s=0.0001; QMessageBox::warning(0, "invalid value input", "You have entered a very small value which is not allowed. V3D will just use 0.0001 to replace it.");}
    else if (s<0 && s>-0.0001) {s=-0.0001; QMessageBox::warning(0, "invalid value input", "You have entered a very small value which is not allowed. V3D will just use -0.0001 to replace it.");}
#endif

    proc_neuron_gmultiply_factor(ntpList[ant], 1, 1, s/(cur_gscale_z[ant]*1000));
    cur_gscale_z[ant] = s/1000.0;

    highlight_points();
}

void NeuronGeometryDialog::scale_r(double s)
{
    proc_neuron_multiply_factor_radius(ntpList[ant], s/(cur_scale_r[ant]*1000));
    cur_scale_r[ant] = s/1000.0;

    highlight_points();
}

void NeuronGeometryDialog::rotate_around_x(int v)
{
    double afmatrix[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    double a = NORMALIZE_ROTATION_Angle( v-cur_rotate_x[ant] ) / 180.0 * MY_PI;
    afmatrix[5] = cos(a); afmatrix[6] = -sin(a);
    afmatrix[9] = -afmatrix[6]; afmatrix[10] = afmatrix[5];
    proc_neuron_affine_around_center(ntpList[ant], afmatrix, cur_cx[ant], cur_cy[ant], cur_cz[ant]);
    cur_rotate_x[ant] = v;

    highlight_points();
}

void NeuronGeometryDialog::rotate_around_y(int v)
{
    double afmatrix[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    double a = NORMALIZE_ROTATION_Angle( v-cur_rotate_y[ant] ) / 180.0 * MY_PI;
    afmatrix[0] = cos(a); afmatrix[2] = sin(a);
    afmatrix[8] = -afmatrix[2]; afmatrix[10] = afmatrix[0];
    proc_neuron_affine_around_center(ntpList[ant], afmatrix, cur_cx[ant], cur_cy[ant], cur_cz[ant]);
    cur_rotate_y[ant] = v;

    highlight_points();
}

void NeuronGeometryDialog::rotate_around_z(int v)
{
    double afmatrix[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    double a = NORMALIZE_ROTATION_Angle( v-cur_rotate_z[ant] ) / 180.0 * MY_PI;
    afmatrix[0] = cos(a); afmatrix[1] = -sin(a);
    afmatrix[4] = -afmatrix[1]; afmatrix[5] = afmatrix[0];
    proc_neuron_affine_around_center(ntpList[ant], afmatrix, cur_cx[ant], cur_cy[ant], cur_cz[ant]);
    cur_rotate_z[ant] = v;

    highlight_points();
}

void NeuronGeometryDialog::flip_x(int v)
{
    proc_neuron_mirror(ntpList[ant], true, false, false);
    cur_flip_x[ant] = checkBox_flip_x->isChecked();

    highlight_points();
}

void NeuronGeometryDialog::flip_y(int v)
{
    proc_neuron_mirror(ntpList[ant], false, true, false);
    cur_flip_y[ant] = checkBox_flip_y->isChecked();

    highlight_points();
}

void NeuronGeometryDialog::flip_z(int v)
{
    proc_neuron_mirror(ntpList[ant], false, false, true);
    cur_flip_z[ant] = checkBox_flip_z->isChecked();

    highlight_points();
}


