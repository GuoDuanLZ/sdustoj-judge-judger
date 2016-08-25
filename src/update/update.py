# -*- coding:utf-8 -*-

from mongoengine import *
import mongoengine
import redis
import json
import os
import time
import datetime


class TestData(Document):
    tid = StringField(max_length=64, unique=True)
    mid = StringField(max_length=16)
    fin = FileField()
    fout = FileField()

    meta = {
        'indexes': [
            'mid',
            {
                'fields': ['tid'],
                'unique': 'True'
            },
        ]
    }


def getConfig():
    with open('../Config.json', 'r') as f:
        conf = json.load(f)
    return conf


def connectRedis(conf):
    while True:
        try:
            redis_q = redis.StrictRedis(
                host=conf['localRedisHost'],
                port=conf['localRedisPort'],
                password=conf['localRedisPwd']
            )
        except Exception as e:
            LOG('Connect redis failed :' + e)
            time.sleep(5)
        else:
            LOG('Connect redis success')
            break
    return redis_q


def connectDB(conf):
    while True:
        try:
            mongoengine.connect(
                host=conf['update']['DBHost'],
                port=conf['update']['DBPort'],
                authentication_source=conf['update']['authentication_source'],
                db=conf['update']['db'],
                username=conf['update']['username'],
                password=conf['update']['password']
            )
        except Exception as e:
            LOG('Connect to DB failed :' + e)
            time.sleep(5)
        else:
            break


def analysMsg(jsonMsg):
    info = json.loads(jsonMsg)
    return info['t']


def removeFiles(path):
    # print(path)
    if os.path.isfile(path):
        try:
            os.remove(path)
        except Exception as e:
            LOG('Remove file failed :' + e)
    elif os.path.isdir(path):
        for file in os.listdir(path):
            file = os.path.join(path, file)
            removeFiles(file)
        try:
            os.rmdir(path)
        except Exception as e:
            LOG('Remove dir failed :' + e)


def downloadData(info):
    datas = []
    if info['mid'] == '*':
        removeFiles(os.getcwd() + '/../stdData/')
        # Download all data
        for dataInfo in TestData.objects:
            datas.append(dataInfo)
    else:
        if info['tid'] == '*':
            removeFiles(os.getcwd() + '/../stdData/' + info['mid'])
            # Download all data from the specific source
            for dataInfo in TestData.objects.filter(mid=info['mid']):
                datas.append(dataInfo)
        else:
            # Download some data from the specific source
            for testID in info['tid']:
                for dataInfo in TestData.objects.filter(mid=info['mid'], tid=testID):
                    datas.append(dataInfo)
    for data in datas:
        inputDir = os.getcwd() + '/../stdData/' + data.mid + '/in'
        outputDir = os.getcwd() + '/../stdData/' + data.mid + '/out'
        if not os.path.exists(inputDir):
            os.makedirs(inputDir)
        if not os.path.exists(outputDir):
            os.makedirs(outputDir)
        inputData = data.fin.read()
        outputData = data.fout.read()
        if inputData is not None:
            with open(inputDir + '/in_' + data.tid, 'wb') as f:
                f.write(inputData)
        if outputData is not None:
            with open(outputDir + '/out_' + data.tid, 'wb') as f:
                f.write(outputData)


def removeData(info):
    if info['tid'] == '*':
        # Remove all data from the specific source
        removeFiles(os.getcwd() + '/../stdData/' + info['mid'])
    else:
        # Remove some data from the specific source
        for testID in info['tid']:
            removeFiles(os.getcwd() + '/../stdData/' + info['mid'] + '/in/in_' + testID)
            removeFiles(os.getcwd() + '/../stdData/' + info['mid'] + '/out/out_' + testID)


def updateData(info):
    if info['opt'] == 'insert' or info['opt'] == 'update':
        downloadData(info)
    elif info['opt'] == 'remove':
        removeData(info)


def sendMsg(redis_q, info, result, result_msg):
    try:
        redis_q.rpush(conf['update']['UpdateDefaultOut'], toJson(info, result, result_msg))
    except Exception as e:
        print('Send message failed :', e)


def toJson(info, result, result_msg):
    dic = {}
    dic['type'] = 'cmdRet'
    dic['id'] = info['id']
    dic['act'] = info['opt']
    dic['dest'] = info['mid']
    dic['result'] = result
    dic['msg'] = result_msg
    return json.dumps(dic)


def get_datenow():
    return time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(time.time()))


def LOG(msg):
    f = open("../log/JudgeLog(" + str(datetime.date.today()) + ").log", 'a+')
    try:
        f.write(get_datenow() + " U(" + str(os.getpid()) + ")\t" + msg + "\n")
    finally:
        f.close()


if __name__ == '__main__':
    try:
        conf = getConfig()
    except Exception as e:
        LOG('Get config-file failed :' + e)
        raise SystemExit
    else:
        LOG('Get config-file success')

    redis_q = connectRedis(conf)
    connectDB(conf)

    stdData = os.getcwd() + '/../stdData'
    if not os.path.exists(stdData):
        os.mkdir(stdData)
    while(True):
        info = {}
        try:
            jsonMsg = redis_q.blpop(conf['update']['UpdateDefaultIn'])
        except Exception as e:
            LOG('Get message failed :' + e)
        else:
            LOG('Get message')

        try:
            info = analysMsg(jsonMsg[1].decode('utf-8'))
        except Exception as e:
            LOG('AnalysMsg failed :' + e)
        else:
            LOG('AnalysMsg')

        try:
            updateData(info)
        except Exception as e:
            LOG('Update data failed :' + e)
            sendMsg(redis_q, info, 'failed', str(e))
        else:
            LOG('Update data success')
            sendMsg(redis_q, info, 'success', 'success')
