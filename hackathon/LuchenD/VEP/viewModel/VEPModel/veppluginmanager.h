#ifndef VEPPLUGINMANAGER_H
#define VEPPLUGINMANAGER_H

#include <bits/stdc++.h>
#include "extern/pybind11/pybind11.h"

#include "PluginRequestCallback.h"


class VEPPluginManager
{
public:
    explicit VEPPluginManager();
    ~VEPPluginManager() = default;

    void createProFile(QString dir, QString pluginName);
    void createPluginHeader(QString dir, std::shared_ptr<PluginRequestCallback> pluginRequestCallback);


private:


};

#endif // VEPPLUGINMANAGER_H
