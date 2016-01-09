#ifndef INEURONDATAOBJECT_H
#define INEURONDATAOBJECT_H
#include <memory>
#include <string>
class INeuronProcessObject;
class INeuronDataObject;
#ifdef _WIN32
typedef std::tr1::shared_ptr<INeuronDataObject> DataPointer;
typedef std::tr1::shared_ptr<const INeuronDataObject> ConstDataPointer;
#define nullptr 0
#else
typedef std::shared_ptr<INeuronDataObject> DataPointer;
typedef std::shared_ptr<const INeuronDataObject> ConstDataPointer;
#endif

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
