#ifndef DB_BASIC_H
#define DB_BASIC_H
#include <QtGui>


enum BasicDataState { Raw=0,
                      Converting=1,
                      Converted=2,
                      Finished=3
                    };
enum AnnotationState{
    L0=0,
    L1A=1,
    L1B=2,
    L1C=3,
    L1D=4,
    L1=5,
    L2A=6,
    L2B=7,
    L2C=8,
    L2D=9,
    L2=10
};

enum BasicDataPriority
{
    Low=0,
    Normal=1,
    High=2,
    Urgent=3
};
enum AuthorType{
    visitor=0,
    junior=1,
    senior=2,
    analyst=3,
    Admin=4
};
enum BrainRegion
{
    Unknown=0,
    CPU=1,
    Thalamus=2,
    ACB=3,
    other=4
};
struct XYZ {
    union {
    struct {unsigned long x, y, z;};
    };
    XYZ()
    {
        this->x=this->y=this->z=0;
    }

    XYZ(unsigned long px,unsigned long py,unsigned long pz){
        this->x=px;this->y=py;this->z=pz;
    }
};
class BrainBasic{
private:
    long BrainID;
    QString BrainName;
public:
    BrainBasic()
    {
        BrainID=0;
        BrainName="00000";
        State=Raw;
        Priority=Normal;
        Comments="";
        Datapath="";
        Resolution(0,0,0);
    }
    BrainBasic(long id,const QString &name,BasicDataPriority p,const QString& comments,XYZ resolution){
        this->BrainID=id;
        this->BrainName=name;
        this->Priority=p;
        this->Comments=comments;
        this->Resolution=resolution;
    }

    void setID(long id)
    {
        this->BrainID=id;
    }
    void setID(long id,const QString &name)
    {
        this->BrainID=id;
        this->BrainName=name;
    }
    void setPriority(BasicDataPriority p)
    {
        this->Priority=p;
    }
    void setComments(const QString& comments)
    {
        this->Comments=comments;
    }
    void setDatapath(const QString& path)
    {
        this->Datapath=path;
    }
    void setName(QString &name)
    {
        this->BrainName=name;
    }

    long getID()
    {
        return this->BrainID;
    }

    QString getName()
    {
        return this->BrainName;
    }
    XYZ getResolution()
    {
       return this->Resulution;

    }

private:
    BasicDataState BrainState;
    BasicDataPriority Priority;
    QString Comments;
    XYZ Resolution;
    QString Datapath;
};
class Author
{
public:
    unsigned long AuthorID;
    QString AuthorUsername;
public:
    Author()
    {
    AuthorID=0;
    AuthorUsername="V";
    Name="Visitor";
    Type=visitor;
    Comments="";
    }
    Author(unsigned long id,const QString &username,const QString &name,AuthorType type,const QString &comments)
    {
        this->AuthorID=id;
        this->AuthorUsername=username;
        this->Name=name;
        this->Type=type;
        this->Comments=comments;
    }
    void setID(unsigned long id,const QString &username)
    {
        this->AuthorID=id;
        this->AuthorUsername=username;
    }
    QString getUsername()
    {
        return this->AuthorUsername;
    }

private:
    QString Name;
    AuthorType Type;
    QString Comments;
};
class Neuron
{
private:
    BrainBasic MyBrain;
    unsigned long NeuroID;
    QString NeuroName;
    QString NeuroComments;
    XYZ Somapos;
public:
    Neuron()
    {
        this->MyBrain.BrainBasic();
        this->NeuroID=0;
        this->NeuroName="00000";
        this->NeuroComments="";
        this->Somapos.XYZ();
    }
    Neuron(unsigned long neuroID,const QString &neuronName,const QString &neuroComments)
    {
        this->NeuroID=neuroID;
        this->NeuroName=neuronName;
        this->NeuroComments=neuroComments;
        this->MyBrain.BrainBasic();
        this->Somapos.XYZ();
    }
    void setMyBrain(long id,const QString &name,BasicDataPriority p,const QString& comments,XYZ resolution)
    {
        this->MyBrain.BrainBasic(id,name,p,comments,resolution);
    }
    void setSomapos(unsigned long px,unsigned long py,unsigned long pz)
    {
        this->Somapos.XYZ(px,py,pz);
    }
};

class Annotation:public Neuron
{
private:
    unsigned LevelID;
    Author Annotator;
    AnnotationState NowState,PreState;
    QString Comments;
    QString TimeStamp;
    bool Lock;//true,lock annotation state,untill state is changed.
public:
    Annotation()
    {
        this->NowState=this->PreState=L0;
        this->Annotator.Author();
        this->Lock=false;
        this->Comments="";
        this->TimeStamp="2000_1_1_0_0";//year_month_day_hour_second
    }
    Annotation(Author annotator,AnnotationState nowstate,AnnotationState prestate,bool lock,const QString &comments,const QString &timestamp)
    {
        this->NowState=nowstate;
        this->PreState=prestate;
        this->Annotator=annotator;
        this->Lock=lock;
        this->Comments=comments;
        this->TimeStamp=timestamp;
    }
    bool setAnnotationState(AnnotationState nowstate)
    {
        if (this->Lock)
        {
            this->NowState=nowstate;
            return true;
        }
        qDebug()<<"This neuron is locked now"<<endl;
        return false;
    }

};

#endif // DB_BASIC_H
