#ifndef INEURONIO_H
#define INEURONIO_H
#include "ineuronprocessobject.h"
class INeuronIO : public INeuronProcessObject
{
public:
    virtual ~INeuronIO(){}
    virtual void SetInputFileName(const std::string&)=0;
    virtual void SetOutputFileName(const std::string&)=0;
};

#endif // INEURONIO_H
