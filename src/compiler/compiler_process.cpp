#include "compiler.h"
#include "filterkey.h"

string localRedisHost;//本地redis主机
string localRedisPort;//本地redis端口
string localRedisPwd;//本地redis密码

string judgeAccount;//judge机帐号
string thisPID;//本进程号
string thisPgm;//本程序名 程序名+进程号
string beginPath;//程序第一次启动时路径

string defaultIn;//监听队列名
string defaultOut;//转出队列名
string defaultBack;//消息回填队列名

Json::Value compilerConfig;

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

  compilerConfig = root["compiler"];
  IF_NULL_STR((defaultIn = compilerConfig["CompileDefaultIn"].asString()));
  IF_NULL_STR((defaultOut = compilerConfig["CompileDefaultOut"].asString()));
  IF_NULL_STR((defaultBack = compilerConfig["CompileDefaultRet"].asString()));

	return true;
}

int main(){
  signal(SIGCHLD, SIG_DFL);
  beginPath = util->getCurPath();
  thisPID = util->intToString((int)getpid());
  thisPgm = "C(" + thisPID + ")";
  LOG->setLogName("../log/JudgeLog");
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

  if(!util->isFolderExist("../userSubmition")){
    mkdir("../userSubmition", 0777);
  }

  Compiler *pCompiler = new Compiler();
  FilterKey *pFilter = new FilterKey();

  Json::Reader reader;
  Json::Value root;
  Json::Value temp;
  Json::Value j_sta;
  Json::Value j_ret;
  Json::Value j_arr;
  Json::Value j_run;
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

    j_ret["type"] = "result";
    j_ret["sid"]  = root["sid"].asString();

    if(!pFilter->filter(root["code"].asString(), root["InvalidWord"].asString())){
      j_ret["ret"]  = "IW";
      j_ret["clen"] = util->intToString(root["code"].asString().length());
      j_ret["msg"]  = "Invalid Word: " + pFilter->getInvalidWord();
      pLocalRedis->setMsg(defaultBack, j_ret.toStyledString());
      LOG->note("sid(" + root["sid"].asString() + ") return 'IW'");
      continue;
    }

    string path = beginPath + "/../userSubmition/" + root["sid"].asString();

    if(root["sid"].asString() == "SPJ"){
      path = beginPath + "/../SPJ/" + root["mid"].asString();
      mkdir(path.c_str(), 0777);
      path += "/" + root["pid"].asString();
    }

    mkdir(path.c_str(), 0777);
    if(chdir(path.c_str()) == -1){
      LOG->note("chdir error");
      return 0;
    }

    temp = compilerConfig[root["eid"].asString()];
    string srcFilePath = path + "/" + temp["codefile"].asString();
		ofstream outCode(srcFilePath.c_str(), ios::out);
		outCode << root["code"].asString() << endl;
		outCode.close();
    LOG->note("produce code sid(" + root["sid"].asString() + ")");

    int ret = COMPILE_ACCEPT;
    if(temp["cmd"].asString() != "NULL"){
      pCompiler->setSrcFileName(temp["codefile"].asString());
      pCompiler->setExcFileName(temp["exefile"].asString());
      pCompiler->setOutFileName(temp["outfile"].asString());
      pCompiler->setErrorFileName("compile_error");
      pCompiler->setCompileCmd(temp["cmd"].asString());

      ret = pCompiler->Compile();
      j_sta["type"]   = "status";
      j_sta["sid"]    = root["sid"].asString();
      j_sta["status"] = "CP";
      LOG->note("sid(" + root["sid"].asString() + ") compile finished");
      pLocalRedis->setMsg(defaultBack, j_sta.toStyledString());
    }
    if(root["sid"].asString() == "SPJ"){
      if(ret == COMPILE_ACCEPT){
        j_ret["ret"]  = "success";
        j_ret["msg"] = "compile success";
      }
      else{
        j_ret["ret"]  = "failed";
        j_ret["msg"]  = util->loadAllFromFile(pCompiler->getErrorFileName(), -1);
      }
      j_ret["clen"] = util->intToString(root["code"].asString().length());
      pLocalRedis->setMsg(defaultBack, j_ret.toStyledString());
      LOG->note("sid(" + root["sid"].asString() + ") return");
      continue;
    }
		j_arr         = root["tid"];
    int dataNum = j_arr.size();
    switch(ret){
      case COMPILE_ACCEPT:
        j_run["sid"]    = root["sid"].asString();
        j_run["pid"]    = root["pid"].asString();
        j_run["mid"]    = root["mid"].asString();
        j_run["eid"]    = root["eid"].asString();
        j_run["ttype"]  = root["ttype"].asString();
        j_run["tl"]     = root["tl"].asString();
        j_run["ml"]     = root["ml"].asString();
        j_run["ol"]     = root["ol"].asString();
        j_run["clen"]   = util->intToString(root["code"].asString().length());
        j_run["dtree"]  = root["dtree"].asString();
				for(int i = 0; i < dataNum; ++i){
					j_run["tid"] = j_arr[i].asString();
					pLocalRedis->setMsg(defaultOut, j_run.toStyledString());
				}
        LOG->note("dispatch sid(" + root["sid"].asString() + ")");
        break;
      case COMPILE_ERROR:
        j_ret["ret"]  = "CE";
        j_ret["clen"] = util->intToString(root["code"].asString().length());
        j_ret["msg"]  = util->loadAllFromFile(pCompiler->getErrorFileName(), -1);
        pLocalRedis->setMsg(defaultBack, j_ret.toStyledString());
        LOG->note("sid(" + root["sid"].asString() + ") return 'CE'");
        break;
      case COMPILE_OT:
        j_ret["ret"]  = "CE";
        j_ret["clen"] = util->intToString(root["code"].asString().length());
        j_ret["msg"]  = "compile time out limit";
        pLocalRedis->setMsg(defaultBack, j_ret.toStyledString());
        LOG->note("sid(" + root["sid"].asString() + ") return 'CTOL'");
        break;
      case COMPILE_INTERRUPT:
        j_ret["ret"]  = "CE";
        j_ret["clen"] = util->intToString(root["code"].asString().length());
        j_ret["msg"]  = "compile interrupt error";
        pLocalRedis->setMsg(defaultBack, j_ret.toStyledString());
        LOG->note("sid(" + root["sid"].asString() + ") return 'CIE'");
        break;
    }
  }
  return 0;
}
