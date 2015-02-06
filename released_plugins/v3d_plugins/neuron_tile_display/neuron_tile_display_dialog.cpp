#include "neuron_tile_display_dialog.h"
#include "../../../v3d_main/neuron_editing/neuron_xforms.h"



NeuronTileDisplayDialog::NeuronTileDisplayDialog(V3DPluginCallback2 * cb, V3dR_MainWindow* inwin)
{

    v3dwin=inwin;
    callback=cb;

    //create
    spin_x = new QDoubleSpinBox();
    spin_x->setRange(0,100000); spin_x->setValue(0);
//    connect(spin_x, SIGNAL(valueChanged(double)), this, SLOT(tile(double)));
    spin_y = new QDoubleSpinBox();
    spin_y->setRange(0,100000); spin_y->setValue(100);
//    connect(spin_y, SIGNAL(valueChanged(double)), this, SLOT(tile(double)));
    spin_z = new QDoubleSpinBox();
    spin_z->setRange(0,100000); spin_z->setValue(0);
//    connect(spin_z, SIGNAL(valueChanged(double)), this, SLOT(tile(double)));

    btn_quit = new QPushButton("Quit");
    connect(btn_quit, SIGNAL(clicked()), this, SLOT(reject()));
    btn_tile = new QPushButton("Tile");
    connect(btn_tile, SIGNAL(clicked()), this, SLOT(tile()));
    btn_reset = new QPushButton("Reset");
    connect(btn_reset, SIGNAL(clicked()), this, SLOT(slot_reset()));

    //layout
    QGridLayout * gridLayout = new QGridLayout();
  //  gridLayout->addWidget(check_tile,0,0,1,1);
    QLabel* label_1 = new QLabel("X direction steps:");
    gridLayout->addWidget(label_1,1,0,1,2);
    gridLayout->addWidget(spin_x,1,2,1,1);
    QLabel* label_2 = new QLabel("Y direction steps:");
    gridLayout->addWidget(label_2,2,0,1,2);
    gridLayout->addWidget(spin_y,2,2,1,1);
    QLabel* label_3 = new QLabel("Z direction steps:");
    gridLayout->addWidget(label_3,3,0,1,2);
    gridLayout->addWidget(spin_z,3,2,1,1);
    gridLayout->addWidget(btn_tile,4,0,1,1);
    gridLayout->addWidget(btn_reset,4,1,1,1);
    gridLayout->addWidget(btn_quit,4,2,1,1);

    setLayout(gridLayout);

    //load neuron tree
    ntList=callback->getHandleNeuronTrees_Any3DViewer(v3dwin);

    cur_x = cur_y = cur_z = 0;
    cur_num = ntList->size();
}

void NeuronTileDisplayDialog::enterEvent(QEvent *e)
{
    checkwindow();

    QDialog::enterEvent(e);
}

void NeuronTileDisplayDialog::reject()
{
    reset();

    if(v3dwin){
        callback->update_NeuronBoundingBox(v3dwin);
        callback->update_3DViewer(v3dwin);
    }

    QDialog::reject();
}

void NeuronTileDisplayDialog::checkwindow()
{
    //check if current window is closed
    if (!callback){
        this->hide();
        return;
    }

    bool isclosed = true;
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
        return;
    }
}

void NeuronTileDisplayDialog::reset()
{
    if(cur_x>0 || cur_y>0 || cur_z>0){
        double move_x, move_y, move_z;
        for(int i=1; i<ntList->size() && i<cur_num; i++){
            move_x = -cur_x*i;
            move_y = -cur_y*i;
            move_z = -cur_z*i;
            NeuronTree* p = (NeuronTree*)&(ntList->at(i));
            proc_neuron_add_offset(p, move_x, move_y, move_z);
        }
    }
    cur_x = cur_y = cur_z = 0;
    cur_num=ntList->size();
}

void NeuronTileDisplayDialog::slot_reset()
{
    checkwindow();

    reset();

    if(v3dwin){
        callback->update_NeuronBoundingBox(v3dwin);
        callback->update_3DViewer(v3dwin);
    }
}

void NeuronTileDisplayDialog::tile(double dis)
{
    tile();
}

void NeuronTileDisplayDialog::tile()
{
    checkwindow();

    reset();
    {
        double move_x, move_y, move_z;
        cur_x = spin_x->value();
        cur_y = spin_y->value();
        cur_z = spin_z->value();
        for(int i=1; i<ntList->size(); i++){
            move_x = cur_x*i;
            move_y = cur_y*i;
            move_z = cur_z*i;
            NeuronTree* p = (NeuronTree*)&(ntList->at(i));
            proc_neuron_add_offset(p, move_x, move_y, move_z);
        }
    }

    if(v3dwin){
        callback->update_NeuronBoundingBox(v3dwin);
        callback->update_3DViewer(v3dwin);
    }
}



NeuronXYTileDialog::NeuronXYTileDialog(V3DPluginCallback2 * cb, V3dR_MainWindow* inwin)
{

    v3dwin=inwin;
    callback=cb;

    //create
    spin_x = new QDoubleSpinBox();
    spin_x->setRange(0,100000); spin_x->setValue(100);
    spin_y = new QDoubleSpinBox();
    spin_y->setRange(0,100000); spin_y->setValue(100);
    spin_ratial = new QDoubleSpinBox();
    spin_ratial->setRange(0.01,100000); spin_ratial->setValue(1);

    btn_quit = new QPushButton("Quit");
    connect(btn_quit, SIGNAL(clicked()), this, SLOT(reject()));
    btn_tile = new QPushButton("Tile");
    connect(btn_tile, SIGNAL(clicked()), this, SLOT(tile()));
    btn_reset = new QPushButton("Reset");
    connect(btn_reset, SIGNAL(clicked()), this, SLOT(slot_reset()));

    //layout
    QGridLayout * gridLayout = new QGridLayout();
    QLabel* label_1 = new QLabel("X direction steps:");
    gridLayout->addWidget(label_1,1,0,1,2);
    gridLayout->addWidget(spin_x,1,2,1,1);
    QLabel* label_2 = new QLabel("Y direction steps:");
    gridLayout->addWidget(label_2,2,0,1,2);
    gridLayout->addWidget(spin_y,2,2,1,1);
    QLabel* label_3 = new QLabel("desired X:Y ratial:");
    gridLayout->addWidget(label_3,3,0,1,2);
    gridLayout->addWidget(spin_ratial,3,2,1,1);
    gridLayout->addWidget(btn_tile,4,0,1,1);
    gridLayout->addWidget(btn_reset,4,1,1,1);
    gridLayout->addWidget(btn_quit,4,2,1,1);

    setLayout(gridLayout);

    //load neuron tree
    ntList=callback->getHandleNeuronTrees_Any3DViewer(v3dwin);

    cur_x.clear();
    cur_y.clear();
    cur_num = ntList->size();
}

void NeuronXYTileDialog::enterEvent(QEvent *e)
{
    checkwindow();

    QDialog::enterEvent(e);
}

void NeuronXYTileDialog::reject()
{
    reset();

    if(v3dwin){
        callback->update_NeuronBoundingBox(v3dwin);
        callback->update_3DViewer(v3dwin);
    }

    QDialog::reject();
}

void NeuronXYTileDialog::checkwindow()
{
    //check if current window is closed
    if (!callback){
        this->hide();
        return;
    }

    bool isclosed = true;
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
        return;
    }
}

void NeuronXYTileDialog::reset()
{
    if(cur_x.size()>0 && (cur_y.size() == cur_x.size())){
        double move_x, move_y, move_z=0;
        for(int i=1; i<ntList->size() && i<cur_num; i++){
            move_x = -cur_x[i];
            move_y = -cur_y[i];
            NeuronTree* p = (NeuronTree*)&(ntList->at(i));
            proc_neuron_add_offset(p, move_x, move_y, move_z);
        }
    }
    cur_x.clear();
    cur_y.clear();
    cur_num=ntList->size();
}

void NeuronXYTileDialog::slot_reset()
{
    checkwindow();

    reset();

    if(v3dwin){
        callback->update_NeuronBoundingBox(v3dwin);
        callback->update_3DViewer(v3dwin);
    }
}

void NeuronXYTileDialog::tile()
{
    checkwindow();

    reset();

    if(ntList->size()<=1)
        return;

    double dx=spin_x->value();
    double dy=spin_y->value();
    double rate=spin_ratial->value();
    //search for the best Nx
    int Nt=ntList->size(), Nx;
    double bestRate=rate*1e16;
    //try all possible Nx
    for(int tmpNx=Nt; tmpNx>0; tmpNx--){
        double xmin=ntList->at(0).listNeuron.at(0).x;
        double xmax=ntList->at(0).listNeuron.at(0).x;
        double ymin=ntList->at(0).listNeuron.at(0).y;
        double ymax=ntList->at(0).listNeuron.at(0).y;
        double move_x, move_y;
        int i=0;
        for(int y=0; y<Nt/tmpNx+1; y++){
            for(int x=0; x<tmpNx; x++){
                if(i>=ntList->size())
                    break;
                move_x = dx*x;
                move_y = dy*y;
                NeuronTree* p = (NeuronTree*)&(ntList->at(i));
                for(int j=0; j<p->listNeuron.size(); j++){
                    xmin=MIN(xmin,p->listNeuron.at(j).x+x*dx);
                    xmax=MAX(xmax,p->listNeuron.at(j).x+x*dx);
                    ymin=MIN(ymin,p->listNeuron.at(j).y+y*dy);
                    ymax=MAX(ymax,p->listNeuron.at(j).y+y*dy);
                }

                i++;
            }
            if(i>=ntList->size())
                break;
        }
        double tmpRate=(xmax-xmin)/(ymax-ymin);
        if(abs(bestRate/rate-1)>abs(tmpRate/rate-1)){
            bestRate=tmpRate;
            Nx=tmpNx;
        }
    }
    qDebug()<<"XY Tile Display: X direction number "<<Nx<<"; X/Y rate: "<<bestRate;
    {
        double move_x, move_y, move_z=0;
        int i=0;
        for(int y=0; y<Nt/Nx+1; y++){
            for(int x=0; x<Nx; x++){
                if(i>=ntList->size())
                    break;
                move_x = dx*x;
                move_y = dy*y;
                NeuronTree* p = (NeuronTree*)&(ntList->at(i));
                proc_neuron_add_offset(p, move_x, move_y, move_z);
                cur_x.push_back(move_x);
                cur_y.push_back(move_y);
                //qDebug()<<"cojoc: "<<move_x<<":"<<move_y<<"\t"<<x<<":"<<y<<":"<<Nx;
                i++;
            }
            if(i>=ntList->size())
                break;
        }
    }

    if(v3dwin){
        callback->update_NeuronBoundingBox(v3dwin);
        callback->update_3DViewer(v3dwin);
    }
}
