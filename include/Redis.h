#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <hiredis/hiredis.h>

#include "log.h"

using namespace std;

class Redis{
public:
	Redis();
	~Redis();
public:
	bool connect(string host, int port, string password = "");
	string getMsg(list<string> keys);
	int getQueueLen(string name);
	void setMsg(string key, string value);	
private:
	redisContext* con;
	redisReply* reply;

	string _host;
	int _port;
	string _password;
private:
	bool reConnect();
};
