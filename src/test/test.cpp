#include "Comparator.h"

string localRedisHost;//本地redis主机
string localRedisPort;//本地redis端口
string localRedisPwd;//本地redis密码

string judgeAccount;//judge机帐号
string thisPID;//本进程号
string thisPgm;//本程序名 程序名+进程号
string beginPath;//程序第一次启动时路径

string defaultIn;//监听队列名
string defaultOut;//转出队列名

string inputFileName;//标准输入文件名
string outputFileName;//程序输出文件名

Json::Value testConfig;

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

  IF_NULL_STR((localRedisHost = root["localRedisHost"].asString()));
  IF_NULL_STR((localRedisPort = root["localRedisPort"].asString()));
  localRedisPwd = root["localRedisPwd"].asString();

  testConfig = root["test"];
  IF_NULL_STR((defaultIn = testConfig["TestDefaultIn"].asString()));
  IF_NULL_STR((defaultOut = testConfig["TestDefaultOut"].asString()));

	return true;
}

int main()
{
  signal(SIGCHLD, SIG_DFL);
  beginPath = util->getCurPath();
  thisPID   = util->intToString((int)getpid());
  thisPgm   = "T(" + thisPID + ")";
  LOG->setLogName(beginPath + "/../log/JudgeLog");
  LOG->setPgName(thisPgm);
  LOG->note("Active Success");

	if(!Config("../Config.json")){
    LOG->note("read config error");
		return 0;
	}
  LOG->note("Config Success");

	Redis* pLocalRedis = new Redis();
  while(!pLocalRedis->connect(localRedisHost, util->stringToInt(localRedisPort), localRedisPwd)){
    LOG->note("connected local Redis failed");
    sleep(5);
  }
  LOG->note("connected local Redis success");

  Comparator* pComparator = new Comparator();

	Json::Reader reader;
  Json::Value root;
  Json::Value temp;
  Json::Value j_ret;
	list<string> keys;
	keys.push_back(defaultIn);
	while(true){
    string msg;
    msg = pLocalRedis->getMsg(keys);
    if(!reader.parse(msg, root)){
      LOG->note("received an error msg");
      #ifdef DEBUG
      LOG->note("received an error msg: " + msg, true);
      #endif
      continue;
    }
		LOG->note("received sid("+ root["sid"].asString() +")");
		string path;
    if(root["ttype"].asString() != "normal"){
      path = beginPath + "/../SPJ/" + root["mid"].asString() + "/" + root["pid"].asString();
      pComparator->setIsSPJ(true);
      temp = testConfig[root["ttype"].asString()];
      pComparator->setSPJCmd(temp["cmd"].asString());
LOG->note(temp["cmd"].asString());
    }
    else{
      path = beginPath + "/../userSubmition/" + root["sid"].asString();
      pComparator->setIsSPJ(false);
    }
    string tempPath = beginPath + "/../stdData/" + root["mid"].asString();
    pComparator->setStdInputName(tempPath + "/in/in_" + root["tid"].asString());
    pComparator->setStdOutputName(tempPath + "/out/out_" + root["tid"].asString());
    pComparator->setUserOutputName(beginPath + "/../userSubmition/" + root["sid"].asString() +"/pout_" + root["tid"].asString());

    //mkdir(path.c_str(), 0777);
    if(chdir(path.c_str()) == -1){
      LOG->note("chdir error");
      return 0;
    }

    int res = pComparator->compare();
		string result;
		switch(res){
			case RESULT_AC:
				result = "AC";
				break;
			case RESULT_PE:
				result = "PE";
				break;
			case RESULT_PTE:
				result = "PTE";
				break;
      case RESULT_WA:
  			result = "WA";
  			break;
			default:
				result = "UNKNOWN" + util->intToString(res);
		}
    j_ret["type"] = "result";
    j_ret["sid"]  =  root["sid"].asString();
    j_ret["tid"]  = root["tid"].asString();
    j_ret["time"] = root["time"].asString();
    j_ret["mem"]  = root["mem"].asString();
    j_ret["clen"] = root["clen"].asString();
    j_ret["ret"]  = result;
    j_ret["msg"]  = result;
    pLocalRedis->setMsg(defaultOut, j_ret.toStyledString());

    LOG->note("Test Finish");
	}
	return 0;
}
