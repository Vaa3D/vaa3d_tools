#ifndef INEURONDATAOBJECT_H
#define INEURONDATAOBJECT_H
#include <memory>
#include <string>
class INeuronProcessObject;
class INeuronDataObject;
typedef std::shared_ptr<INeuronDataObject> DataPointer;
typedef std::shared_ptr<const INeuronDataObject> ConstDataPointer;

class INeuronDataObject
{
public:
    virtual bool IsEmpty()const=0;
    virtual void SetProcessObject(INeuronProcessObject* p){m_ProcessObject = p;}
    virtual INeuronProcessObject *GetProcessObject()const{return m_ProcessObject;}
    virtual void ReleaseProcessObject(){m_ProcessObject = nullptr;}
    const std::string& GetIdentifyName()const{return identifyName;}
    virtual ~INeuronDataObject(){}
protected:
    std::string identifyName;
    INeuronProcessObject* m_ProcessObject;//father process

};

#endif // NEURONDATAOBJECT_H
