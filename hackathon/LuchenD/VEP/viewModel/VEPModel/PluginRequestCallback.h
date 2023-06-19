#ifndef PLUGINREQUESTCALLBACK_H
#define PLUGINREQUESTCALLBACK_H

#include <QString>


class PluginRequestCallback {

public:
    PluginRequestCallback() = default;
    virtual ~PluginRequestCallback() = default;


    virtual void onSuccess(const QString plguinName) = 0;
    virtual void onFailure(const int code, const QString msg) = 0;
};



class StdFunctionPluginRequestCallbackWrapper: public PluginRequestCallback {

public:
    // 在构造函数以lambda表达式形式实现回调
    StdFunctionPluginRequestCallbackWrapper(
            std::function<void(const QString &plguinName)> success,
            std::function<void(const int code, const QString msg)> failure) :
            successFunc(std::move(success)),
            failureFunc(std::move(failure)) {};

    void onSuccess(const QString pluginName) override {
        successFunc(pluginName);
    }

    void onFailure(const int code, const QString msg) override {
        failureFunc(code, msg);
    }

private:
    std::function<void(const QString &plguinName)> successFunc;
    std::function<void(const int code, const QString msg)> failureFunc;



};


#endif // PLUGINREQUESTCALLBACK_H
