#include "maindialog.h"

MainDialog::MainDialog(const QString &path, QWidget *parent)
    :QDialog(parent)
{

    setupDBpath(path);
    setWindowTitle(tr("MorphoHub-LevelControl"));
    MainInit();
    createMainView();
    setLayout(mainLayout);
    this->dialogReady=true;
}
void MainDialog::createMainView()
{
    //function combox
    functionlistQLabel=new QLabel(tr("Level Control Function: "));
    functionlistQComboBox=new QComboBox();
    functionlistQComboBox->addItems(functionlist);
    functionlistQComboBox->setCurrentIndex(1);
    connect(functionlistQComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(OnfunctionlistQComboBox_slot(int)));
    //SdataID & SomaID
    QLabel *sdatidQlabel=new QLabel(tr("SDataID:"));
    sdataidLineEdit=new QLineEdit();
    sdataidLineEdit->setText("00000");
    sdataidLineEdit->setEnabled(false);
    QLabel *somaidQlabel=new QLabel(tr("SomaID:"));
    somaidLineEdit=new QLineEdit();
    somaidLineEdit->setText("00000");
    somaidLineEdit->setEnabled(false);

    //Author
    QLabel* authorLabel=new QLabel(tr("Author"));
    authorLineEdit=new QLineEdit();
    authorLineEdit->setText("Visitor");
    authorLineEdit->setEnabled(false);
    changeAuthorNameButton=new QPushButton(tr("Change To Your Name"));
    changeAuthorNameButton->setEnabled(true);
    connect(changeAuthorNameButton,SIGNAL(clicked()),this,SLOT(changeAuthorNameButton_slot()));

    //checkerlist
    QLabel* checkersLabel=new QLabel(tr("Checkers"));
    checkersLineEdit=new QLineEdit();
    checkersLineEdit->setText("");
    checkersLineEdit->setEnabled(false);
    addcheckerName=new QPushButton("Add Your Name");
    connect(addcheckerName,SIGNAL(clicked()),this,SLOT(addcheckerName_slot()));

    //nowlevel
    QLabel* nowLevelLabel=new QLabel(tr("Now Level (WorkingSpace)"));
    nowlevelLineEdit=new QLineEdit();
    nowlevelLineEdit->setText("");
    nowlevelLineEdit->setEnabled(false);
    //nextlevel:
    nextlevelLabel=new QLabel(tr("Next Level List (Neuron will %1 to the following level.)").arg(choseFuction));
    //nl1:workingspace nl2:NeuronArchives nl3:finished nl4:release
    //nl1
    QLabel* nextlevel_WorkingSpace_Label=new QLabel(tr("1.WorkingSpace: "));
    nextlevel_WorkingSpace_LineEdit=new QLineEdit();
    nextlevel_WorkingSpace_LineEdit->setText("");
    nextlevel_WorkingSpace_LineEdit->setEnabled(false);
    nextlevel_WorkingSpace_CheckBox=new QCheckBox();
    nextlevel_WorkingSpace_CheckBox->setCheckState(Qt::Checked);
//    nextlevel_WorkingSpace_CheckBox->setCheckable(true);
    //nl2
    QLabel* nextlevel_NeuronArchives_Label=new QLabel(tr("2.NeuronArchives: "));
    nextlevel_NeuronArchives_LineEdit=new QLineEdit();
    nextlevel_NeuronArchives_LineEdit->setText("");
    nextlevel_NeuronArchives_LineEdit->setEnabled(false);
    nextlevel_NeuronArchives_CheckBox=new QCheckBox();
    nextlevel_NeuronArchives_CheckBox->setCheckState(Qt::Checked);
//    nextlevel_NeuronArchives_CheckBox->setCheckable(true);
    //nl3
    QLabel* nextlevel_Finished_Label=new QLabel(tr("3.Finished: "));
    nextlevel_Finished_LineEdit=new QLineEdit();
    nextlevel_Finished_LineEdit->setText("");
    nextlevel_Finished_LineEdit->setEnabled(false);
    nextlevel_Finished_CheckBox=new QCheckBox();
    nextlevel_Finished_CheckBox->setCheckState(Qt::Unchecked);
//    nextlevel_Finished_CheckBox->setCheckable(true);

    //button: okay & cancel
    okayButton=new QPushButton("YES");
    cancelButton=new QPushButton("Cancel");
    connect(okayButton,SIGNAL(clicked()),this,SLOT(okayButton_slot()));
    connect(cancelButton,SIGNAL(clicked()),this,SLOT(cancelButton_slot()));
    //layout
    mainLayout=new QGridLayout;
    mainLayout->addWidget(functionlistQLabel,0,0,1,2,Qt::AlignLeft);
    mainLayout->addWidget(functionlistQComboBox,0,2,1,2);

    mainLayout->addWidget(sdatidQlabel,1,0,1,1);
    mainLayout->addWidget(sdataidLineEdit,1,1,1,1);
    mainLayout->addWidget(somaidQlabel,1,2,1,1);
    mainLayout->addWidget(somaidLineEdit,1,3,1,1);

    mainLayout->addWidget(authorLabel,2,0,1,1);
    mainLayout->addWidget(authorLineEdit,2,1,1,1);
    mainLayout->addWidget(changeAuthorNameButton,2,2,1,2);

    mainLayout->addWidget(checkersLabel,3,0,1,1);
    mainLayout->addWidget(checkersLineEdit,3,1,1,2);
    mainLayout->addWidget(addcheckerName,3,3,1,1);

    mainLayout->addWidget(nowLevelLabel,4,0,1,2);
    mainLayout->addWidget(nowlevelLineEdit,4,2,1,2);

    mainLayout->addWidget(nextlevelLabel,5,0,1,4);
    mainLayout->addWidget(nextlevel_WorkingSpace_Label,6,0,1,1);
    mainLayout->addWidget(nextlevel_WorkingSpace_LineEdit,6,1,1,2);
    mainLayout->addWidget(nextlevel_WorkingSpace_CheckBox,6,3,1,1);

    mainLayout->addWidget(nextlevel_NeuronArchives_Label,7,0,1,1);
    mainLayout->addWidget(nextlevel_NeuronArchives_LineEdit,7,1,1,2);
    mainLayout->addWidget(nextlevel_NeuronArchives_CheckBox,7,3,1,1);

    mainLayout->addWidget(nextlevel_Finished_Label,8,0,1,1);
    mainLayout->addWidget(nextlevel_Finished_LineEdit,8,1,1,2);
    mainLayout->addWidget(nextlevel_Finished_CheckBox,8,3,1,1);

    mainLayout->addWidget(cancelButton,9,0,1,2);
    mainLayout->addWidget(okayButton,9,2,1,2);

}
void MainDialog::MainInit()
{
    functionlist<<"Update"<<"Commit"<<"Check"<<"Skip"<<"Rollback"<<"Reassign"<<"Release";
    choseFuction="Commit";
    curNeuron.SdataID="00000";
    curNeuron.SomaID="00000";
    curNeuron.author.UserID="Visitor";
    curNeuron.checkers="";
    curNeuron.levelID="Unknown";
    this->dialogReady=false;
}
void MainDialog::updateMainView()
{
    //0.clear old state
    nextlevel_WorkingSpace_LineEdit->setText("");
    nextlevel_NeuronArchives_LineEdit->setText("");
    nextlevel_Finished_LineEdit->setText("");
    addcheckerName->setEnabled(true);
    //1. update window title
    this->setWindowTitle(QObject::tr("MorphoHub-%1-%2").arg(choseFuction).arg(author.UserID));
    //2. update Neuron basic Info
    if(functionlistQComboBox->currentIndex()==1)
        changeAuthorNameButton->setEnabled(true);
    else
        changeAuthorNameButton->setEnabled(false);
    sdataidLineEdit->setText(curNeuron.SdataID);
    somaidLineEdit->setText(curNeuron.SomaID);
    authorLineEdit->setText(curNeuron.author.UserID);
    checkersLineEdit->setText(curNeuron.checkers);
    nowlevelLineEdit->setText(curNeuron.levelID);
    nextlevelLabel->setText(tr("Next Level List (Neuron will %1 to the following level.)").arg(choseFuction));
    //3.According to the chosen function and levelID get the next level list:
    QStringList nextlevelist=getNextlevelList();
    //4.display
    if(nextlevelist.size()==0)
        return;
    else if(nextlevelist.size()==1)
    {
        nextlevel_WorkingSpace_LineEdit->setText(nextlevelist.at(0));
    }
    else if(nextlevelist.size()==2)
    {
        nextlevel_WorkingSpace_LineEdit->setText(nextlevelist.at(0));
        nextlevel_NeuronArchives_LineEdit->setText(nextlevelist.at(1));
    }
    else if(nextlevelist.size()==3)
    {
        nextlevel_WorkingSpace_LineEdit->setText(nextlevelist.at(0));
        nextlevel_NeuronArchives_LineEdit->setText(nextlevelist.at(1));
        nextlevel_Finished_LineEdit->setText(nextlevelist.at(2));
    }
    else
    {
        QMessageBox::warning(this,tr("Level Error"),tr("Next list is out of list. Please contact Admin!"));
        return;
    }
}
QStringList MainDialog::getNextlevelList()
{
    QStringList outlist;
    outlist.clear();
    //0.preparation
    InitofAnnotationProtocol();
    //1. get the premission from annotation protocol conf file.
    int funcindex =functionlistQComboBox->currentIndex();
    switch(funcindex)
    {
    case 0:break;
    case 1://commit function
    {
        QString  workingspacelevel=ApforthisDialog.APFCommit(curNeuron.levelID);
        if(!workingspacelevel.isEmpty())
        {
            outlist.append(workingspacelevel);
            //check archive
            bool archive=ApforthisDialog.APFArchive(workingspacelevel);
            if(archive)
            {
                outlist.append(workingspacelevel);
            }
            //check finished
            QString finishedlevel=ApforthisDialog.APFFinished(workingspacelevel);
            if(!finishedlevel.isEmpty())
            {
                outlist.append(finishedlevel);
            }
        }
    }
        break;
    case 2://check function
    {
        QString  workingspacelevel=ApforthisDialog.APFCheck(curNeuron.levelID);
        if(!workingspacelevel.isEmpty())
        {
            outlist.append(workingspacelevel);
            //check archive
            bool archive=ApforthisDialog.APFArchive(workingspacelevel);
            if(archive)
            {
                outlist.append(workingspacelevel);
            }
            //check finished
            QString finishedlevel=ApforthisDialog.APFFinished(workingspacelevel);
            if(!finishedlevel.isEmpty())
            {
                outlist.append(finishedlevel);
            }
        }
    }
        break;
    case 3://skip function
    {
        QString  workingspacelevel=ApforthisDialog.APFSkip(curNeuron.levelID);
        if(!workingspacelevel.isEmpty())
        {
            outlist.append(workingspacelevel);
            //check archive
            bool archive=ApforthisDialog.APFArchive(workingspacelevel);
            if(archive)
            {
                outlist.append(workingspacelevel);
            }
            //check finished
            QString finishedlevel=ApforthisDialog.APFFinished(workingspacelevel);
            if(!finishedlevel.isEmpty())
            {
                outlist.append(finishedlevel);
            }
        }
    }
        break;
    }
    //
    return outlist;
}

void MainDialog::InitofAnnotationProtocol()
{
    QString confpath=ApforthisDialog.ApConfPath;
    if (confpath.isEmpty())
        return;
    QFile scanconffile(confpath);
    if(!scanconffile.exists())
    {
        int reply;
        reply=QMessageBox::warning(this,"File Not Found",QObject::tr("Can't find configuration file of Annotation protocol!\n")+
                                   QObject::tr("MorphoHub will create an initialized annotation protocol: %1?\n").arg(confpath),
                                   QMessageBox::Ok,QMessageBox::Cancel);
        if(reply==QMessageBox::Cancel)
            return;
        else
        {
            if(scanconffile.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                //write head
                QString data="StartLevel_EndLevel";
                for(int i=0;i<ApforthisDialog.protocolLevel.size();i++)
                {
                    data=data+","+ApforthisDialog.protocolLevel.at(i);
                }
                data+="\n";
                scanconffile.write(data.toAscii());
                //write inside
                for(int i=0;i<ApforthisDialog.protocolLevel.size();i++)
                {
                    QString startlevel=ApforthisDialog.protocolLevel.at(i);
                    QString levelrules=startlevel;
                    for(int j=0;j<ApforthisDialog.protocolLevel.size();j++)
                    {
                        QString endlevel=ApforthisDialog.protocolLevel.at(j);
                        QString startlevel_endlevel=startlevel+"_"+endlevel;;
                        AnnotationProtocolFunction apfunc=ApforthisDialog.protocolrules.value(startlevel_endlevel);
                        int apfuncint=apfunc;
                        levelrules+=(","+(QString::number(apfuncint)));
                    }
                    qDebug()<<levelrules;
                    levelrules+="\n";
                    scanconffile.write(levelrules.toAscii());
                }
                scanconffile.close();
            }
        }
    }
    if(scanconffile.exists())
    {
        return;
    }
}


void MainDialog::setFunction(const QString& func)
{
    int funcindex=functionlist.indexOf(func);
    if(funcindex!=-1)
    {
        choseFuction=func;
        functionlistQComboBox->setCurrentIndex(funcindex);
        if(funcindex==1)
            changeAuthorNameButton->setEnabled(true);
    }
    else
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr("This function is not registered, Please contact JSD."),QObject::tr("Ok"));
    }
}

void MainDialog::setCurNeuron(ReconstructionInfo inputNeuron)
{
    if(!inputNeuron.SdataID.isEmpty() &&
            !inputNeuron.SomaID.isEmpty()&&
            !inputNeuron.author.UserID.isEmpty()&&
            !inputNeuron.updateTime.isEmpty()&&
            !inputNeuron.levelID.isEmpty())
    {
        curNeuron=inputNeuron;
    }
//    else
//    {
//        QMessageBox::warning(this,tr("Input Error"),tr("The format of this data is wrong."));
//        return;
//    }
}
void MainDialog::setAnnotator(Annotator inputauthor)
{
    if(!inputauthor.UserID.isEmpty())
    {
        author=inputauthor;
        this->setWindowTitle(QObject::tr("MorphoHub-LevelControl-%2").arg(author.UserID));
    }
    else
    {
        QMessageBox::warning(this,tr("Input Error"),tr("UserID is invalid. Please contact Admin!"));
        return;
    }
}

void MainDialog::setupDBpath(const QString &path)
{
    if(path.isEmpty())
    {
        QMessageBox::warning(this,tr("Path Error"),tr("please setup dbpath"));
        return;
    }
    this->dbpath=path;
}
void MainDialog::setupAnnotationProtocol(AnnotationProtocol inputAP)
{
    this->ApforthisDialog=inputAP;
}

//slots
void MainDialog::changeAuthorNameButton_slot()
{
    //when this button is clicked. the author of the cur reconstruction will change to the operator.
    authorLineEdit->setText(author.UserID);
}
void MainDialog::OnfunctionlistQComboBox_slot(int index)
{
    if(index!=-1)
    {
        choseFuction=functionlistQComboBox->currentText();
        //1.update
        updateMainView();
    }
}

void MainDialog::addcheckerName_slot()
{
    QString precheckers=checkersLineEdit->text();
    if(precheckers.isEmpty())
        checkersLineEdit->setText(author.UserID);
    else
        checkersLineEdit->setText(precheckers+"&"+author.UserID);
    addcheckerName->setEnabled(false);
}
void MainDialog::okayButton_slot()
{
    //1.get next level
}
void MainDialog::cancelButton_slot()
{
    this->close();
}

MainDialog::~MainDialog()
{

}
MainDialog::MainDialog()
{
    qDebug()<<"MainDialog init default";
}
