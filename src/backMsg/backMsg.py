import sys
import os
import redis
import time
import json
import urllib
import datetime
from urllib.request import urlopen
from http.client import HTTPConnection


def http_put(url, dir, message):
    """
    向指定的地址通过PUT请求发送数据更新的消息。
    示例：
        http_put('127.0.0.1:8000', '/api-judge/status/judge001/', {
            'type': 'none',
            'sid': '10',
            'tid': '1',
            'status': 'AC',
        })
    :param url: 地址
    :param dir: 目录
    :param message: 消息，字典的格式
    :return: 状态码, 状态码的解释，返回的信息
    """
    conn = HTTPConnection(url)
    headers = {"Content-type": "application/json"}
    params = json.dumps(message)
    conn.request('PATCH', dir, params, headers)
    response = conn.getresponse()
    data = response.read()
    return response.status, response.reason, data

def get_datenow():
    return time.strftime('%Y-%m-%d %H:%M:%S',time.localtime(time.time()))

def LOG(msg):
    f = open("../log/JudgeLog(" + str(datetime.date.today()) + ").log", 'a+')
    try:
        f.write(get_datenow() + " B(" + str(os.getpid()) + ")\t" + msg + "\n");
    finally:
        f.close()

def get_connect(host_, port_, pwd_):
    while True:
        try:
            r = redis.StrictRedis(
                host=host_,
                port=port_,
                password=pwd_,
                db=0,
                socket_connect_timeout=5
            )
            return r
        except Exception as e:
            LOG(str(e))

LOG("Active")

f = open("../Config.json", 'r')
try:
    all_ = f.read()
finally:
    f.close()
all_config = json.loads(all_)

try:
    localHost    = all_config["localRedisHost"]
    localPort    = all_config["localRedisPort"]
    localPwd     = all_config["localRedisPwd"]
    judgeAccount = all_config["judgeAccount"]
    defaultIn    = all_config["backMsg"]["BackMsgDefaultIn"]
    web_address  = all_config["backMsg"]["web_address"]
except Exception as e:
    LOG("Config error: keyError: " + str(e))
    exit()

LOG("Config success")

r = get_connect(localHost, localPort, localPwd)

LOG("connect redis success")

while True:
    try:
        msg = r.blpop([defaultIn])
        LOG("revice a msg")
    except Exception as e:
        LOG(str(e))
        time.sleep(5)
        r = get_connect(localHost, localPort, localPwd)
        continue

    info_str = msg[1]
    try:
        data = json.loads(info_str.decode('utf-8'))
    except Exception as e:
        LOG("error msg: " + str(info_str))
        continue

    urls = {
        'status': '/api-judge/status/' + judgeAccount + '/',
        'result': '/api-judge/results/' + judgeAccount + '/',
        'cmdRet': '/api-judge/commands/' + judgeAccount + '/'
    }

    url = urls[str(data["type"])]
    postdata = data

    print(datetime.datetime.now(), url)
    try:
        print(http_put(web_address, url, postdata))
        LOG("post a msg")
    except Exception as e:
        LOG(str(e))
        continue
