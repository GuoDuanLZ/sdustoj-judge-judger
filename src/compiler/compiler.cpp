#include "compiler.h"


Compiler::Compiler(){
	this->compileTime = 3;
}

Compiler::~Compiler(){
	;
}

void Compiler::setSrcFileName(string srcFileName){
	this->srcFileName = srcFileName;
}

void Compiler::setExcFileName(string excFileName){
	this->excFileName = excFileName;
}

void Compiler::setOutFileName(string outFileName){
	this->outFileName = outFileName;
}

void Compiler::setErrorFileName(string errorFileName){
	this->errorFileName = errorFileName;
}

void Compiler::setCompileTime(int ct){
	this->compileTime = ct;
}

void Compiler::setCompileCmd(string cmd){
	this->compileCmd = cmd;
	this->parameters = util->split(cmd, ' ');
	this->compilerPath = this->parameters[0];
	this->parameters.erase(this->parameters.begin());
}

string Compiler::getSrcFileName(){
	return this->srcFileName;
}

string Compiler::getExcFileName(){
	return this->excFileName;
}

string Compiler::getOutFileName(){
	return this->outFileName;
}

string Compiler::getErrorFileName(){
	return this->errorFileName;
}

int Compiler::getCompileTime(){
	return this->compileTime;
}

int Compiler::Compile(){
	struct rlimit compileLimit;
	compileLimit.rlim_max = compileLimit.rlim_cur = compileTime;
	int cpid;
	if((cpid = fork()) == 0){
		freopen(errorFileName.c_str(), "w", stderr);
		setrlimit(RLIMIT_CPU, &compileLimit);
		char** exec_argv = NULL;
		util->transformCmd(this->parameters, exec_argv);
		execv(this->compilerPath.c_str(), exec_argv);

		exit(0);
	}
	else{
		int compileStatus, timeUsed;
		struct timeval startv, nowv;
		gettimeofday(&startv, NULL);
		while(1){
			usleep(50000);
			gettimeofday(&nowv, NULL);
			timeUsed = nowv.tv_sec - startv.tv_sec;
			if(waitpid(cpid, &compileStatus, WNOHANG) == 0){
				if(timeUsed > compileTime){
					int ret = system(((string)"kill `pstree -p " + util->intToString(cpid) + " | sed 's/(/\\n(/g' | grep '(' | " "sed 's/(\\(.*\\)).*/\\1/' | tr \"\\n\" \" \"`").c_str());
					waitpid(cpid, &compileStatus, 0);
					return COMPILE_OT;
				}
			}
			else if(WIFSIGNALED(compileStatus) && WTERMSIG(compileStatus) != 0){
				int ret = system(((string)"kill `pstree -p " + util->intToString(cpid) + " | sed 's/(/\\n(/g' | grep '(' | " "sed 's/(\\(.*\\)).*/\\1/' | tr \"\\n\" \" \"`").c_str());
				waitpid(cpid, &compileStatus, 0);
				return COMPILE_INTERRUPT;
			}
			else if(WIFSTOPPED(compileStatus)){
				if(WSTOPSIG(compileStatus) != 0){
					int ret = system(((string)"kill `pstree -p " + util->intToString(cpid) + " | sed 's/(/\\n(/g' | grep '(' | " "sed 's/(\\(.*\\)).*/\\1/' | tr \"\\n\" \" \"`").c_str());
					waitpid(cpid, &compileStatus, 0);
					return COMPILE_INTERRUPT;
				}
			}
			else if(WIFEXITED(compileStatus) != 0){
				waitpid(cpid, &compileStatus, 0);
				break;
			}
		}
	}
	//这里判断是否生成了可执行文件
	//如果execl执行失败 就不会生成
	if(!util->isFileExist(outFileName))
		return COMPILE_ERROR;

	return COMPILE_ACCEPT;
}

void Compiler::cleanErrorFile(){
	if(util->isFileExist(errorFileName))
		util->deleteFile(errorFileName);
}
