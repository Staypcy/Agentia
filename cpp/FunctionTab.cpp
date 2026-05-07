#include"FunctionTab.h"

FunctionTab::FunctionTab()
{
}

void FunctionTab::creatFunction(const FunctionStruct &func, std::function<FunctionResult (const QJsonObject &)> executer)
{
    funcSet.push_back(func);
    executers[func.name]=executer;
}

/*void FunctionTab::addFunction(QVector<QVector<Gridcell>>gridset)
{
    FunctionStruct func1;
    func.name="perceive_env";
    func.description="获取指定坐标周围3x3范围内的建筑和资源信息";
    func.parameters=R"({"type":"object","properties":{"x":{"type":"integer"},"y":{"type":"integer"}},"required":["x","y"]})";

    this->creatFunction(
    {
         func1,
         [this](const QJsonObject& params)->FunctionResult{
             int x=params["x"];
             int y=params["y"];

             QJsonArray env;
             if(gridset.isEmpty()||gridset[0].size()!=0){
                 int col = gridset.size();
                 int row = gridset[0].size();
                 for(int i=x-1;i<=i+1;i++){
                     for(int j=y-1;j<=j+1;j++){
                         if ((x >= 0 && x < col) && (y >= 0 && y < row)){
                             QJsonObject cell;
                             cell["x"] = x+i;
                             cell["y"] = y+j;
                             cell["building"] = gridset[x+i][y+j].build;
                             cell["resource"] = gridset[x+i][y+j].resource;
                             env.append(cell);
                         }
                     }
                 }
                 QJsonDocument doc(env);
                 return {true, doc.toJson().toStdString()};
             }
             return {false," "};
         }
    });
/*
    FunctionStruct func2;
    func2.name="take_action";
    func2.description="执行指定的动作/操作";
    func2.parameters=R"({"type":"object","properties":{"operation":{"type":"string","enum":["type":"Staying","MoveUp","MoveDown","MoveRight","MoveLeft","Work","Interact"]}},"required":["operation"]})";
    this->creatFunction(
        func2,
        [this](const QJsonObject& params) -> FunctionResult {

        }
    );

}*/

std::string FunctionTab::getFuncJson()
{
    QJsonArray tools;
    for(const auto& func:funcSet){
        QJsonObject tool;
        tool["name"]=QString::fromStdString(func.name);
        tool["description"]=QString::fromStdString(func.description);
        tool["parameters"]=QJsonDocument::fromJson(
            QByteArray::fromStdString(func.parameters)
            ).object();

        tools.append(tool);
    }
    return QJsonDocument(tools).toJson().toStdString();
}

FunctionResult FunctionTab::execute(const std::string &name, const QJsonObject &params)
{
    if(executers.find(name)!=executers.end()){
        return executers[name](params);
    }
    return {false,"无此函数\工具"};
}
