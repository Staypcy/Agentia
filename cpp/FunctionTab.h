#pragma once
#include <string>
#include <functional>
#include <unordered_map>
#include <QJsonObject>
#include<QJsonArray>
#include <QJsonDocument>

struct FunctionStruct{
    std::string name;
    std::string description;
    std::string parameters;
};

struct FunctionResult{
    bool success;
    std::string data;
};

class FunctionTab{
public:
    FunctionTab();
    void creatFunction(const FunctionStruct &func, std::function<FunctionResult (const QJsonObject &)> executer);

    std::string getFuncJson();

    FunctionResult execute(const std::string& name,const QJsonObject&params);

private:
    std::vector<FunctionStruct>funcSet;
    std::unordered_map<std::string,std::function<FunctionResult(const QJsonObject&)>>executers;
};
