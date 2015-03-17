#ifndef INEURONPROCESSOBJECT_H
#define INEURONPROCESSOBJECT_H
#include "../ngtypes/ineurondataobject.h"
class ProcessStatus
{
public:
    ProcessStatus(bool arg, const std::string& str):isSucess(arg), identifyName(str){}
    ~ProcessStatus(){}
    bool isSucess;
    std::string identifyName;
};
typedef std::shared_ptr<const ProcessStatus> ConstProcStatPointer;

class INeuronProcessObject
{
public:
    //INeuronProcessObject();
    virtual bool Update()=0;
    virtual void SetInput(ConstDataPointer input){m_Input = input;}
    virtual ConstDataPointer GetOutput()=0;//{return m_Source;}
    virtual DataPointer ReleaseData()=0;
    virtual ~INeuronProcessObject(){}
protected:
    std::string identifyName;
    ConstDataPointer m_Input;
    DataPointer m_Source;//output data

};

#endif // INEURONPROCESSOBJECT_H
