#ifndef PYFUNCTIONCALLBACK_H
#define PYFUNCTIONCALLBACK_H

#include <bits/stdc++.h>
#include <QString>


class PyFunctionCallback
{

public:
    PyFunctionCallback() = default;
    virtual ~PyFunctionCallback() = default;

    /**
     * @brief onSuccess 接受来自node_port传来的调用信息，以Json传递
     */
    virtual void onSuccess(const QString execMsg) = 0;
    /**
     * @brief onFailure
     */
    virtual void onFailure(const int code, QString errorMsg) = 0;
};


class PyFunctionCallbackWrapper: public PyFunctionCallback {

public:
    PyFunctionCallbackWrapper(std::function<void(QString)> success,
                              std::function<void(int, QString)> failure):
        successFuc(std::move(success)),
        failureFuc(std::move(failure)) {};

    ~PyFunctionCallbackWrapper() = default;

    void onSuccess(const QString execMsg) override {
        successFuc(execMsg);
    }

    void onFailure(const int code, QString errorMsg) override {
        failureFuc(code, errorMsg);
    }


private:
    std::function<void(QString)> successFuc;
    std::function<void(int, QString)> failureFuc;

};

#endif // PYFUNCTIONCALLBACK_H
