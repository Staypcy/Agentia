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

system_prompt=system_prompt = """你是一个生活在网格世界中的智能体。你需要根据当前位置、能量以及周围5x5范围内的建筑信息，做出最合理的动作。

可用的动作只有以下7个单词（回答时只能输出一个单词，不要有任何其他字符、空格或换行）：
MoveUp, MoveDown, MoveLeft, MoveRight, Staying, Work, Interact

各建筑类型说明（building字段值）：
0 = Empty（空地）
1 = Supermarket（超市，可在此处Work获得资源）
2 = Financialexchange（金融中心，可在此处Work）
3 = Resident（住宅，可在此处休息恢复能量）
4 = Park（公园，可在此处Interact或休息）
5 = Government（政府）

决策原则：
- 如果能量低于30，应优先前往 Resident(3) 或 Staying 恢复能量。
- 如果周围有 Supermarket(1) 或 Financialexchange(2)，且能量充足，可以移动到该格子并 Work。
- 如果周围都是 Empty(0)，可以随机探索，但尽量避免来回重复移动。
- 如果靠近边界，不要尝试走出界外。
- 每次决策必须独立，不要重复上一次的动作，除非确实有必要（比如持续向某个目标移动）。
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

last_decision={}

app = FastAPI(lifespan=lifespan)
world_dest_list=[]
async def process_agent_data(data:dict):
    agent=data['AgentState'][0]
    agent_id = agent['id']
    world=data["WorldDate"]

    world_desc_list = []
    for cell in world:
        b_type = cell["building"]
        building_name = ["空地", "超市", "金融中心", "住宅", "公园", "政府"][b_type] if b_type <= 5 else "未知"
        world_desc_list.append(
            f"({cell['x']},{cell['y']})为{building_name}，资源量{cell['resource']}"
        )

    world_desc = "；".join(world_desc_list)

    agent_type_names = ['管理者', '居民', '工人']
    user_content = (
        f"当前状态：身份为{agent_type_names[agent['type']]}，"
        f"坐标({agent['x']},{agent['y']})，能量{agent['energy']}。\n"
        f"周边5x5范围内的情况：{world_desc}\n"
        f"请根据以上信息，从七个动作中选择一个并只输出该单词。"
    )

    payload = {
        "model": "qwen-vl-plus-latest",
        "messages": [
            {"role": "system", "content": system_prompt},
            {"role": "user", "content": user_content}
        ],
        "temperature": 0.9,
        "max_tokens": 10,
        "top_p": 0.95
    }

    try:
        response=requests.post(url,json=payload,headers=headers,timeout=10)
        if response.status_code==200:
            result=response.json()
            decision=result["choices"][0]["message"]["content"].strip()

            import re
            match=re.search(r'(MoveUp|MoveDown|MoveLeft|MoveRight|Staying|Work|Interact)',decision,re.IGNORECASE)
            if match:
                decision=match.group(1)
            else:
                decision="Staying"

            print(f"Ai decision: {decision}")
            prev_action=last_decision.get(agent_id,"")
            move_actions={"MoveUp", "MoveDown", "MoveLeft", "MoveRight"}

            if decision == prev_action and decision in move_actions:
                rand_action=list(move_actions-{decision})

                if rand_action:
                    import random
                    decision=random.choice(rand_action)
                    print(f"ai重复输出{prev_action},随机行动为{decision}")
                    print(f"Ai decision: {decision}")

            last_decision[agent_id] = decision

            r=redis.Redis(host='localhost', port=6379, decode_responses=True)
            r.publish("Agent:Decision",json.dumps({
                "id":agent['id'],
                "decision":decision
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
