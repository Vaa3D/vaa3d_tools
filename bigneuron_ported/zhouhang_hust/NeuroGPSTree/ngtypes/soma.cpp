#include "soma.h"

Soma::Soma(INeuronProcessObject *p)
{
    m_ProcessObject = p;
    identifyName =  std::string("Soma");
}

bool Soma::IsEmpty() const
{
    return m_Source.empty();
}

void Soma::push_back(const Cell &obj)
{
    m_Source.push_back(obj);
}
