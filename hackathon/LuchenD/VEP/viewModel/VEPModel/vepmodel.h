#ifndef VEPMODEL_H
#define VEPMODEL_H

#include <QObject>

#include "VEPModel_global.h"
#include "PluginRequestCallback.h"
#include "PyFunctionCallback.h"


class VEPMODEL_EXPORT VEPModel: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString pluginName READ pluginName WRITE setPluginName NOTIFY pluginNameChanged)

public:
    explicit VEPModel(QObject *parent = nullptr);

    QString pluginName() const;
    void setPluginName(const QString &name);

    ///////  插件huid
    void updatePluginList();

    /**
     * @brief 对view层获得node_port下放function信息
     * @param pyFunctionCallback
     */
    void getPyPortFunction(std::shared_ptr<PyFunctionCallbackWrapper> &pyFunctionCallback);

    /**
     * @brief 执行任务的取消
     */
    void cancel();
    
    
    
signals:
    void pluginNameChanged();


//slots:


private:
    QList<QString> pluginNameList;
    std::unordered_map<QString, QString> pluginMap;


};

#endif // VEPMODEL_H
