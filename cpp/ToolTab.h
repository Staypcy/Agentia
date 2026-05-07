#pragma once
#include<string>
#include<QJsonObject>
#include<functional>
#include<unordered_map>
#include<QJsonDocument>

struct FunctionStruct{
    std::string name;
    std::string description;
    std::string parameters;
};

struct FunctionResult{
    bool success;
    std::string data;
};

class FuntionTab{
public:
    void creatFunction();
    std::string getFuncsJson();
    FunctionResult execute(const std::string&name,const QJsonObject&params);
private:
    std::vector<FunctionStruct>functionSet;
    std::unordered_map
        <std::string,std::function<FunctionStruct(const QJsonObject&)>> executers;
};

