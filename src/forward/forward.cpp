/*
 * Author: yys
 */
#include "../../include/util.h"
#include "../../include/Redis.h"
#include "../../include/log.h"

extern int errno;

string thisPID;//本进程号
string thisPgm;//本程序名 程序名+进程号

string globalRedisHost;//全局redis主机
string globalRedisPort;//全局redis端口
string globalRedisPwd = "";//全局redis密码

string localRedisHost;//本地redis主机
string localRedisPort;//本地redis端口
string localRedisPwd = "";//本地redis密码

string judgeAccount;//judge机帐号

string defaultIn;//监听队列名
string defaultOut;//转出队列名
string defaultBack;//回填队列名
string defaultLimit;//根据此队列内消息数量限定是否继续转发消息

int maxMsgNum = 100;

Json::Value ForwardConfig;

bool Config(string path){
  ifstream in(path.c_str(), ios::in);
  if(!in.is_open()){
    #ifdef DEBUG
    LOG->note("open config file failed", true);
    #endif
    return false;
  }

  Json::Reader reader;
  Json::Value root;
  if (!reader.parse(in, root, false)){
    #ifdef DEBUG
    LOG->note("parse config file failed", true);
    #endif
		in.close();
		return false;
  }
 	in.close();

  IF_NULL_STR((judgeAccount = root["judgeAccount"].asString()));

  IF_NULL_STR((globalRedisHost = root["globalRedisHost"].asString()));
  IF_NULL_STR((globalRedisPort = root["globalRedisPort"].asString()));
  globalRedisPwd = root["globalRedisPwd"].asString();

  IF_NULL_STR((localRedisHost = root["localRedisHost"].asString()));
  IF_NULL_STR((localRedisPort = root["localRedisPort"].asString()));
  localRedisPwd = root["localRedisPwd"].asString();

  ForwardConfig = root["forward"];
  IF_NULL_STR((defaultOut   = ForwardConfig["ForwardDefaultOut"].asString()));
  IF_NULL_STR((defaultBack  = ForwardConfig["ForwardDefaultBack"].asString()));
  IF_NULL_STR((defaultLimit = ForwardConfig["ForwardDefaultLimit"].asString()));
  IF_NULL_STR(ForwardConfig["maxMsgNum"].asString());
  maxMsgNum = util->stringToInt(ForwardConfig["maxMsgNum"].asString());

	return true;
}

int main(int argc, char *argv[])
{
  signal(SIGCHLD, SIG_DFL);
  thisPID = util->intToString((int)getpid());
  thisPgm = "F(" + thisPID + ")";
  LOG->setLogName("../log/JudgeLog");
  LOG->setPgName(thisPgm);

	if(!Config("../Config.json")){
    LOG->note("read config error");
		return 0;
	}
  LOG->note("Config Success");
  if(argc == 2){
    defaultIn = argv[1];
  }
  else if(argc == 3){
    defaultIn = argv[1];
    defaultOut = argv[2];
  }
  else{
    LOG->note("launch with error argv");
    return 0;
  }

  LOG->note("Active Success");

  Redis* pGlobalRedis = new Redis();
	Redis* pLocalRedis = new Redis();
	while(!pLocalRedis->connect(localRedisHost, util->stringToInt(localRedisPort), localRedisPwd)){
		LOG->note("connected local Redis failed");
		sleep(5);
	}
  LOG->note("connected local Redis success");

  while(!pGlobalRedis->connect(globalRedisHost, util->stringToInt(globalRedisPort), globalRedisPwd)){
		LOG->note("connected global Redis failed");
		sleep(5);
	}
  LOG->note("connected global Redis success");

  Json::Reader reader;
  Json::Value root;
  Json::Value j_sta;
	list<string> keys;
	keys.push_back(defaultIn);
  while(true){
    string msg;
    while(pLocalRedis->getQueueLen(defaultLimit) > maxMsgNum){
      sleep(1);
    }
    msg = pGlobalRedis->getMsg(keys);
    if(!reader.parse(msg, root)){
      LOG->note("received an error msg");
      #ifdef DEBUG
      LOG->note("received an error msg: " + msg, true);
      #endif
      continue;
    }
    LOG->note("received sid("+ root["sid"].asString() +")");
    pLocalRedis->setMsg(defaultOut, msg);
    LOG->note("forward sid("+ root["sid"].asString() +") to " + defaultOut);
    j_sta["type"]   = "status";
    j_sta["sid"]    = root["sid"].asString();
    j_sta["status"] = "IQ";
    pLocalRedis->setMsg(defaultBack, j_sta.toStyledString());
  }
	exit(0);
}
