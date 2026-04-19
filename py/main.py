from fastapi import FastAPI
import redis
import json
import asyncio
import requests
from contextlib import asynccontextmanager

#后续使用aioredis来实现真正的异步决策
#网络接口，千问模型api接口
api_key="sk-5463835ade8d4045bb989ff2f5cf2097"
url="https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions"
headers={
    "Authorization":f"Bearer {api_key}",
    "Content-Type": "application/json"
}

system_prompt="""你是一个生活在网格世界的一个agent,你充满好奇心，可以随便逛逛这个世界
                但是，你只能通过回答:MoveUp,MoveDown,MoveLeft,MoveRight,Staying,Work,Interact
                这几个单词来执行你的操作，你的回答的格式也只能是这几个单词，不能出现任何其他的字样，标点符号也不应该出现    
            """
#使用lifespan来管理
async def lifespan(app:FastAPI):
    r=redis.Redis(host='localhost', port=6379, decode_responses=True)
    pubsub=r.pubsub()
    pubsub.subscribe('Agent:State')

    #创建异步任务，来持续监听
    async def listen_redis(asyncicio=None):
        print("正在监听redis的Agent:State频道")
        try:
            for msg in pubsub.listen():#pubsub.listen是阻塞的,异步处理
                if msg['type']=='message':
                    data=json.loads(msg['data'])
                    #处理函数

                    await process_agent_data(data)
        except asyncicio.CancelledError:
            print("redis监听停止")
        finally:
            pubsub.close()

    task=asyncio.create_task(listen_redis())

    yield

    task.cancel()
    await task
    r.close()

app = FastAPI(lifespan=lifespan)
async def process_agent_data(data:dict):
    agent=data['AgentState'][0]
    world=data["WorldDate"]
    #决策逻辑的实现
    user_content=f"当前状态：Type={agent['type']}, Pos=({agent['x']},{agent['y']}), Energy={agent['energy']},你周边的信息（内容格式是Json）:{world}"
#    for temp in world:
#        print(f"temp['x']+","+temp['y']")
#        print(temp['building'])
#        print(temp['resource'])
    payload={
        "model":"qwen-plus",
        "messages":[
            {"role":"system","content":system_prompt},
            {"role":"user","content":user_content}
        ],
        "temperature":1.3,
        "max_tokens":20
    }
    try:
        response=requests.post(url,json=payload,headers=headers,timeout=10)#话说这个timeout参数是干什么的？
        if response.status_code==200:
            result=response.json()
            decision=result["choices"][0]["message"]["content"].strip()

            print(f"Ai decision: {decision}")
            r=redis.Redis(host='localhost', port=6379, decode_responses=True)
            r.publish("Agent:Decision",json.dumps({
                "id":agent['id'],
                "action":decision
            }))
        else:
            print(f"与ai的连接失败")
            print(f"{response.status_code},"
                  f"reason:{response.reason},"
                  f"content:{response.text}")
    except Exception as e:
        print(f"redis 连接失败:{e}")
"""
这里可以确认cpp返回的是Json类型的数据，结构：
由agent本身的状态+agent周边世界的信息+全局信息（全局政策等）
{
  "AgentState": [
    {
      "id": "string",       // Agent 唯一标识，如 "test_0"
      "type": 0,            // 枚举值: 0=Manager, 1=Residenter, 2=Worker
      "x": 10,              // 当前网格 X 坐标 (int)
      "y": 12,              // 当前网格 Y 坐标 (int)
      "energy": 95          // 行动能量值 (int, 0-100)
    }
  ],
  "WorldDate": [
    {
      "x": 8,               // 网格 X 坐标 (int)
      "y": 10,              // 网格 Y 坐标 (int)
      "building": 1,        // 建筑类型: 0=Empty, 1=Supermakert, 2=Financialexchange, 3=Resident, 4=Park, 5=Government
      "resource": 8         // 该网格资源量 (int)
    },
    ... // 最多 25 个对象（5x5 范围内的有效网格）
  ]
}


"{
\"AgentState\":[
    {\"energy\":100,
    \"id\":\"test_0\",
    \"type\":1,
    \"x\":17,
    \"y\":13}],
    
\"WorldDate\":[
    {\"building\":3,
    \"resource\":7,
    \"x\":15,
    \"y\":11},
    {\"building\":0,
    \"resource\":2,
    \"x\":15,\"y\":12},
    {\"building\":2,
    \"resource\":5,
    \"x\":15,
    \"y\":13},
    {\"building\":1,
    \"resource\":0
    ...
"""
@app.get("/")
async def root():
    return {"message": "Hello World"}


@app.get("/hello/{name}")
async def say_hello(name: str):
    return {"message": f"Hello {name}"}
