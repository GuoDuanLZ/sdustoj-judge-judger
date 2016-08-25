#include "log.h"

Log* Log::instance = new Log();

Log::Log(){
  this->nowTime = time(0);
  this->ptmNow = localtime(&this->nowTime);
  this->lastTime = this->nowTime;
  strftime(ctmnow,sizeof(ctmnow),"%Y-%m-%d",ptmNow); //格式化时间
  this->ftime = ctmnow;
}

Log* Log::getInstance() {
  return instance;
}

void Log::setLogName(string name){
  this->logName = name;
}

void Log::setPgName(string name){
  this->pgName = name;
}

void Log::note(string msg, bool debug){
  this->nowTime = time(0);
  if(this->nowTime - this->lastTime >= this->dayTime){
    this->ptmNow = localtime(&this->nowTime);
    this->lastTime = this->nowTime;
    strftime(ctmnow,sizeof(ctmnow),"%Y-%m-%d",ptmNow); //格式化时间
    this->ftime = ctmnow;
  }
  string name;
  if(debug){
    name = (this->logName + "_Debug(" + this->ftime + ").log");
  }
  else{
    name = (this->logName + "(" + this->ftime + ").log");
  }
  this->out.open(name.c_str(), ios::app | ios::out);
  out << util->currentDateTime() << " " + this->pgName + "\t" + msg << endl;
  this->out.close();
}
