from fastapi import FastAPI
import json
import asyncio
import aiohttp
import re
import redis.asyncio as redis_asyncio
from contextlib import asynccontextmanager


#后续使用aioredis来实现真正的异步决策
#网络接口，千问模型api接口
api_key="sk-5b74d3a662ee4c568414ba8d2a4db6cd"
url="https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions"
headers={
    "Authorization":f"Bearer {api_key}",
    "Content-Type": "application/json"
}

system_prompt = """你是一个生活在网格世界中的智能体。你需要根据当前位置、行动能量、精神能量、携带资源以及周围5x5范围内的建筑信息，做出最合理的动作。

可用的动作只有以下7个单词（回答时只能输出一个单词，不要有任何其他字符、空格或换行）：
MoveUp, MoveDown, MoveLeft, MoveRight, Staying, Work, Interact

各建筑类型说明（building字段值）：
0 = Empty（空地）
1 = Supermarket（超市）
2 = Financialexchange（金融中心）
3 = Resident（住宅）
4 = Park（公园）
5 = Government（政府）


规则说明（请仔细阅读，决策时务必遵守）：
- 无论你进行何种行动，都会消耗你的行动能量
- 移动（MoveUp/MoveDown/MoveLeft/MoveRight）：消耗3点资源。若资源不足则移动失败（原地不动，不消耗资源）。
- 工作（Work）：
  - 消耗行动能量5点
  - 工人(Worker) 在金融中心(Financialexchange)：消耗5行动能量，获得15资源。
  - 居民(Residenter) 在超市(Supermakert)：消耗5行动能量，获得15资源。
  - 其他类型或位置工作无效，不会获得资源，但是消耗行动能量5点，和精神能量10点）。
  - 工作会消耗精神能量10点
  - 工作需要你拥有至少10点精神能量
  - 工作会消耗你所处的网格世界点位的资源，当该点位资源下降到5，你执行Work只会消耗你的行动能量，但不会得到资源
- 停留（Staying）：
  - 消耗行动能量5点
  - 在住宅(Resident)：恢复20精神能量（不超过100）。
  - 在超市(Supermakert)且携带资源 ≥ 5：消耗5资源，恢复15行动能量（不超过100）。
  - 在公园(Park)：恢复10精神能量。
  - 其他位置仅原地停留，无特殊效果。
- 交互（Interact）：
  - 消耗行动能量5点
  - 在公园(Park)：恢复5精神能量。
  - 其他位置无效。

ps:在空地是执行任何行动都将使原先的消耗增大1.2倍

决策原则：
- 优先保证携带资源至少3点，以便支持移动探索。
- 行动能量低时，若有资源可去超市恢复；若无资源可原地停留或去工作获得资源。
- 精神能量只会影响状态显示，不影响任何动作执行，你无需为精神能量担忧。
- 保持好奇心，多移动探索世界，避免原地重复相同动作。

重要提示：
- 你是一个测试用的智能体，请尽情四处走动，帮助测试我的Agentia项目。
- 但是如果你的行动能量和精神能量为0，将会受到严重惩罚
- 不用担心出界，系统会自动阻止非法移动。
- 只输出一个动作单词，不要有任何多余字符。
"""
last_decision={}
async def call_llm_aysnc(session:aiohttp.ClientSession,user_content:str):
    payload = {
        "model": "qwen-turbo",  # 你之前用的模型
        "messages": [
            {"role": "system", "content": system_prompt},
            {"role": "user", "content": user_content}
        ],
        "temperature": 1.5,
        "max_tokens": 10,
        "top_p": 0.95
    }
    try:
        async with session.post(url,json=payload,headers=headers,timeout=15) as response:
            if response.status==200:
                result=await response.json()
                decision=result["choices"][0]["message"]["content"].strip()

                match = re.search(r'(MoveUp|MoveDown|MoveLeft|MoveRight|Staying|Work|Interact)',decision,re.IGNORECASE)
                if match:
                    return match.group(1).strip()
                else :
                    return "Staying"
            else:
                print(f"LLM调用失败，{response.status}")
                return "Staying"
    except Exception as e:
        print(f"LLM请求异常,{e}")
        return "Staying"



async def process_agent_data(redis_pub,session,data:dict):
    agent=data['AgentState'][0]
    agent_id = agent['id']
    world=data['WorldDate']

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

    decision=await call_llm_aysnc(session,user_content)
    print(f"{agent_id},{decision}")
    last_decision[agent_id] = decision

    #异步处理
    await redis_pub.publish("Agent:Decision", json.dumps({
        "id":agent_id,
        "decision":decision
    }))


async def listen_redis(app):
    redis=await redis_asyncio.Redis(host='localhost', port=6379, decode_responses=True)
    pubsub=redis.pubsub()
    await pubsub.subscribe('Agent:State')

    async with aiohttp.ClientSession() as session:
        print("开始订阅redis频道 Agent:State")
        try:
            async for msg in pubsub.listen():
                if msg["type"]=="message":
                    data=json.loads(msg['data'])

                    asyncio.create_task(process_agent_data(redis,session,data))

        except Exception as e:
            print("redis 订阅取消")
        finally:
            await pubsub.unsubscribe("Agent:State")
            await redis.close()


@asynccontextmanager
async def lifespan(app:FastAPI):
    task=asyncio.create_task(listen_redis(app))

    yield

    task.cancel()
    try:
        await task
    except asyncio.CancelledError:
        pass


app=FastAPI(lifespan=lifespan)

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
    return {"message": "Agentia Async Decision Service"}


@app.get("/hello/{name}")
async def say_hello(name: str):
    return {"message": f"Hello {name}"}
