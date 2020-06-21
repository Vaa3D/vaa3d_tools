#include "getnodelengthdialog.h"

getNodeLengthDialog::getNodeLengthDialog(QWidget* parent, V3DPluginCallback2& callback)
{
    this->callback = &callback;
    swcEdit = new QLineEdit(QObject::tr("ReadSwc"));
    swcEdit->setFixedWidth(400);

    QPushButton* readButton = new QPushButton(QObject::tr("..."));

    lineEdit = new QLineEdit("1");

    slider = new QSlider(Qt::Horizontal);
    slider->setMinimum(0);
    slider->setMaximum(100);
    slider->setValue(1);

    connect(slider,SIGNAL(valueChanged(int)),this,SLOT(setLineEditValue(int)));
    connect(readButton,SIGNAL(clicked()),this,SLOT(readSWC()));

    QGridLayout* layoutg = new QGridLayout();
    layoutg->addWidget(swcEdit,1,1);
    layoutg->addWidget(readButton,1,2);
    QHBoxLayout* layouth = new QHBoxLayout();
    layouth->addWidget(lineEdit);
    layouth->addWidget(slider);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addLayout(layoutg);
    layout->addLayout(layouth);

    setLayout(layout);
    setWindowTitle(QObject::tr("set threshold"));
}

void getNodeLengthDialog::readSWC(){
    QString swcPath =  QFileDialog::getOpenFileName(this, QObject::tr("Read SWC"), "", QObject::tr("Neuron Structure files (*.swc *.eswc)"));
    swcEdit->setText(swcPath);
    nt = readSWC_file(swcPath);

    cur = callback->open3DViewerForSingleSurfaceFile(swcPath);

}

void getNodeLengthDialog::setLineEditValue(int value){
    int pos = slider->value();
    cout<<"pos: "<<pos<<endl;
    QString str = QString("%1").arg(pos);
    lineEdit->setText(str);

    cout<<"----------------------start-------------------------"<<endl;
//    v3dhandle a = callback->curHiddenSelectedWindow();
    QList<NeuronTree>* trees =  callback->getHandleNeuronTrees_Any3DViewer(cur);
    cout<<"------------------------aaaaaaaaaaaaaaaaaa----------"<<endl;
    cout<<"tree size: "<<trees->size()<<endl;
    NeuronTree& t = (*trees)[0];
    for(int i=0; i<nt.listNeuron.size(); i++){
        if(nt.listNeuron.at(i).r>=pos){
            t.listNeuron[i].r = 10;
        }else {
            t.listNeuron[i].r = 1;
        }
    }
    callback->update_3DViewer(cur);
    std::cout<<"-----------------------bbbbbbbbbbbbbbbbbbbb----------"<<endl;
//    callback->update_3DViewer(cur);
//    callback->getHandleNeuronTrees_3DGlobalViewer()
}


