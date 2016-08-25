#pragma once

#include "util.h"

#define LOG Log::getInstance()

class Log{
public:
  ~Log();
public:
  void setLogName(string name);
  void setPgName(string name);
  void note(string msg, bool debug = false);
public:
  static Log* getInstance();
private:
  Log();
private:
  string logName;
  string pgName;
  string ftime;
  time_t nowTime;
  time_t lastTime;
  struct tm *ptmNow;
  char ctmnow[128];
  const time_t dayTime = 84600;

  ofstream out;
private:
  static Log* instance;
};
