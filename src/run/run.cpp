#include "../../include/util.h"
#include "../../include/Redis.h"
#include "../../include/log.h"

#include "okcalls.h"

#ifdef __i386
#define REG_SYSCALL orig_eax
#define REG_RET eax
#define REG_ARG0 ebx
#define REG_ARG1 ecx
#else
#define REG_SYSCALL orig_rax
#define REG_RET rax
#define REG_ARG0 rdi
#define REG_ARG1 rsi
#endif

#define BUFFER_SIZE 512

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

string inputFileName;//标准输入文件名
string outputFileName;//程序输出文件名

string fileName;//运行命令文件名

Json::Value runConfig;

int languageType;

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

  runConfig = root["run"];
  IF_NULL_STR((defaultIn = runConfig["RunDefaultIn"].asString()));
  IF_NULL_STR((defaultOut = runConfig["RunDefaultOut"].asString()));
  IF_NULL_STR((defaultBack = runConfig["RunDefaultRet"].asString()));

	return true;
}

int call_counter[512];

void init_syscalls_limits(int lang)
{
    int i;
    memset(call_counter, 0, sizeof(call_counter));
		if (lang <= LT_CPP)   // C & C++
    {
        for (i = 0; LANG_CC[i]; i++)
        {
            call_counter[LANG_CV[i]] = LANG_CC[i];
        }
    }
    //else if (lang == LT_PASCAL)     // Pascal
    //{
    //     for (i = 0; LANG_PC[i]; i++)
    //         call_counter[LANG_PV[i]] = LANG_PC[i];
    //}
    else if (lang == LT_JAVA)     // Java
    {
        for (i = 0; LANG_JC[i]; i++)
            call_counter[LANG_JV[i]] = LANG_JC[i];
    }
    //else if (lang == LT_RUBY)     // Ruby
    //{
    //     for (i = 0; LANG_RC[i]; i++)
    //         call_counter[LANG_RV[i]] = LANG_RC[i];
    //}
    //else if (lang == LT_BASH)     // Bash
    //{
    //     for (i = 0; LANG_BC[i]; i++)
    //         call_counter[LANG_BV[i]] = LANG_BC[i];
    //}
    else if (lang == LT_PYTHON)    // Python
    {
        for (i = 0; LANG_YC[i]; i++)
            call_counter[LANG_YV[i]] = LANG_YC[i];
    }
    //else if (lang == LT_PHP)    // php
    //{
    //     for (i = 0; LANG_PHC[i]; i++)
    //         call_counter[LANG_PHV[i]] = LANG_PHC[i];
    //}
    //else if (lang == 8)    // perl
    //{
    //     for (i = 0; LANG_PLC[i]; i++)
    //         call_counter[LANG_PLV[i]] = LANG_PLC[i];
    //}
    //else if (lang == LT_MONOCS)    // mono c#
    //{
    //     for (i = 0; LANG_CSC[i]; i++)
    //         call_counter[LANG_CSV[i]] = LANG_CSC[i];
    // }
    // else if (lang == LT_OBJC)  //objective c
    // {
    //     for (i = 0; LANG_OC[i]; i++)
    //         call_counter[LANG_OV[i]] = LANG_OC[i];
    // }
    // else if (lang == LT_FBASIC)  //free basic
    // {
    //     for (i = 0; LANG_BASICC[i]; i++)
    //         call_counter[LANG_BASICV[i]] = LANG_BASICC[i];
    // }
}

int main()
{
  signal(SIGCHLD, SIG_DFL);
	thisPID   = util->intToString((int)getpid());
  thisPgm   = "R(" + thisPID + ")";
  beginPath = util->getCurPath();

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

	Json::Reader reader;
  Json::Value root;
  Json::Value temp;
  Json::Value j_sta;
  Json::Value j_ret;
  Json::Value j_test;
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

		string path = beginPath + "/../userSubmition/" + root["sid"].asString();
    //mkdir(path.c_str(), 0777);
    if(chdir(path.c_str()) == -1){
      LOG->note("chdir error");
      return 0;
    }

		inputFileName 	= beginPath + "/../stdData/" + root["mid"].asString() + "/in/" + "in_" + root["tid"].asString();
		outputFileName 	= "pout_" + root["tid"].asString();

    temp = runConfig[root["eid"].asString()];
		if(temp["ltype"].asString() == ""){
      LOG->note("unkonw ltype using LT_CPP as default");
			languageType = LT_CPP;
		}else{
			languageType = util->stringToInt(temp["ltype"].asString());
		}

		int timeLimit 	= util->stringToInt(root["tl"].asString());//ms
		int memoryLimit = util->stringToInt(root["ml"].asString());//KB
		int fsizeLimit 	= util->stringToInt(root["ol"].asString());//KB
		int extendTime 	= 10;//ms

		string result;
		string retMsg;
		pid_t rpid;
		int runStatus 	= 0;
		int memoryUsed 	= 0;
		int timeUsed;

		struct rusage rinfo;
		struct user_regs_struct reg;
		struct rlimit time_limit, fsize_limit;

		time_limit.rlim_cur 	= timeLimit / 1000;
		time_limit.rlim_max 	= timeLimit / 1000 + 1;
		fsize_limit.rlim_cur 	= fsize_limit.rlim_max = fsizeLimit;

    init_syscalls_limits(languageType);

		if((rpid = fork()) < 0){
			LOG->note("sid("+ root["sid"].asString() + ") fork error");
		}
		else if(rpid == 0){
			freopen(inputFileName.c_str(), "r", stdin);
			freopen(outputFileName.c_str(), "w", stdout);

			setrlimit(RLIMIT_CPU, &time_limit);
			setrlimit(RLIMIT_FSIZE, &fsize_limit);
			ptrace(PTRACE_TRACEME, 0, NULL, NULL);

			temp = runConfig[root["eid"].asString()];
			vector<string> parameters = util->split(temp["cmd"].asString(), ' ');
			fileName = parameters[0];
			parameters.erase(parameters.begin());
			char** argv = nullptr;
			util->transformCmd(parameters, argv);

			execv(fileName.c_str(), argv);

			exit(0);
		}
		else{
      int sub = 0;
			struct timeval startv, nowv;
			gettimeofday(&startv, NULL);
			while(1){
				wait4(rpid, &runStatus, 0, &rinfo);
				usleep(5000);
				gettimeofday(&nowv, NULL);
				timeUsed = (rinfo.ru_utime.tv_sec + rinfo.ru_stime.tv_sec) * 1000 + (rinfo.ru_utime.tv_usec + rinfo.ru_stime.tv_usec) / 1000;
				int absTimeUsed = nowv.tv_sec - startv.tv_sec;
				if(absTimeUsed > timeLimit / 1000 + extendTime){
					ptrace(PTRACE_KILL, rpid, NULL, NULL);
					result  = "TLE";
          retMsg  = "Time Limit Exceed(ABS)";
          timeUsed = timeLimit;
          waitpid(rpid, &runStatus, 0);
					break;
				}
				if(timeUsed > timeLimit){
					ptrace(PTRACE_KILL, rpid, NULL, NULL);
					result = "TLE";
          retMsg = "Time Limit Exceed";
          timeUsed = timeLimit;
          waitpid(rpid, &runStatus, 0);
					break;
				}
				if(memoryUsed < getpagesize() * rinfo.ru_minflt)
					memoryUsed = getpagesize() * rinfo.ru_minflt;
				if(WIFEXITED(runStatus)){
					result = "Normal";
					break;
				}
				else if(WIFSIGNALED(runStatus) && WTERMSIG(runStatus) != SIGTRAP){//如果是信号中断并且不是由于ptrace中断导致的
					int signal = WTERMSIG(runStatus);
					switch(signal){
						case SIGXCPU://超过CPU时间软限制
							result = "TLE";
              retMsg = "Time Limit Exceed";
							timeUsed = timeLimit;
							break;
						case SIGXFSZ:
							result = "OLE";
              retMsg = "Time Limit Exceed";
							break;
						default:
							result = "RE";
              retMsg = strsignal(signal);
					}
					ptrace(PTRACE_KILL, rpid, NULL, NULL);
          waitpid(rpid, &runStatus, 0);
					break;
				}
				else if(WIFSTOPPED(runStatus) && WSTOPSIG(runStatus) != SIGTRAP){
					int signal = WSTOPSIG(runStatus);
					switch(signal){
						case SIGXCPU:
							result = "TLE";
              retMsg = "Time Limit Exceed(STOP)";
              timeUsed = timeLimit;
							break;
						case SIGXFSZ:
							result = "OLE";
              retMsg = "Output Limit Exceed(STOP)";
							break;
						default:
							result = "RE";
              retMsg = strsignal(signal);
					}
					ptrace(PTRACE_KILL, rpid, NULL, NULL);
          waitpid(rpid, &runStatus, 0);
					break;
				}
				else if((runStatus >> 8) != 5 && (runStatus >> 8) > 0){
					//不是正常退出也不是信号中断也不是暂停的情况
					result = "RE";
          retMsg = strsignal(runStatus>>8);
					ptrace(PTRACE_KILL, rpid, NULL, NULL);
          waitpid(rpid, &runStatus, 0);
					break;
				}

        // check the system calls
				ptrace(PTRACE_GETREGS, rpid, NULL, &reg);

				if (call_counter[reg.REG_SYSCALL] == 0) {
					char cerror[BUFFER_SIZE];
					sprintf(cerror,"[ERROR] A Not allowed system call: sid:%s callid:%llu\n",
							root["sid"].asString().c_str(), reg.REG_SYSCALL);
					result = "RE";
          retMsg = cerror;
					ptrace(PTRACE_KILL, rpid, NULL, NULL);
          waitpid(rpid, &runStatus, 0);
					break;
				} else {
					if (sub == 1)
						call_counter[reg.REG_SYSCALL]--;
				}
				sub = 1 - sub;

				if(memoryUsed / 1024 > memoryLimit){
					result = "MLE";
          retMsg = "Memory Limit Exceed";
					ptrace(PTRACE_KILL, rpid, NULL, NULL);
          waitpid(rpid, &runStatus, 0);
					break;
				}
				//PTRACE_SYSCALL 使内核在子进程做出系统调用
				//或者准备退出的时候暂停它
				ptrace(PTRACE_SYSCALL, rpid, NULL, NULL);
			}//while(1)
		}//else
    j_sta["type"]   = "status";
    j_sta["sid"]    = root["sid"].asString();
    j_sta["status"] = "RN";
    pLocalRedis->setMsg(defaultBack, j_sta.toStyledString());

    LOG->note("sid(" + root["sid"].asString() + ") run finished");

		if(result != "Normal"){
      j_ret["type"] = "result";
      j_ret["sid"]  = root["sid"].asString();
      j_ret["tid"]  = root["tid"].asString();
      j_ret["time"] = util->intToString(timeUsed);
      j_ret["mem"]  = util->intToString(memoryUsed / 1024);
      j_ret["clen"] = root["clen"].asString();
      j_ret["ret"]  = result;
      j_ret["msg"]  = retMsg;
			pLocalRedis->setMsg(defaultBack, j_ret.toStyledString());

      LOG->note("sid(" + root["sid"].asString() + ") return '" + retMsg + "'");
      #ifdef DEBUG
      LOG->note(j_ret.toStyledString(), true);
      #endif
		}
		else{
      j_test["sid"]    = root["sid"].asString();
      j_test["pid"]    = root["pid"].asString();
      j_test["mid"]    = root["mid"].asString();
      j_test["tid"]    = root["tid"].asString();
      j_test["ttype"]  = root["ttype"].asString();
      j_test["time"]   = util->intToString(timeUsed);
      j_test["mem"]    = util->intToString(memoryUsed / 1024);
      j_test["clen"]   = root["clen"].asString();
      j_test["dtree"]  = root["dtree"].asString();
			pLocalRedis->setMsg(defaultOut, j_test.toStyledString());
      LOG->note("forward sid("+ root["sid"].asString() +") to " + defaultOut);
      #ifdef DEBUG
      LOG->note(j_test.toStyledString(), true);
      #endif
		}
	}
	return 0;
}
