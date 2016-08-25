#include "Redis.h"

Redis::Redis(){
	;
}

Redis::~Redis(){
	redisFree(con);
}

bool Redis::connect(string host, int port, string password){
	this->con = redisConnect(host.c_str(), port);
	if(this->con->err){
		LOG->note("connect to " + host + " error! msg:" + util->intToString(this->con->err));
		redisFree(con);
		return 0;
	}
	if(password != ""){
		string pwd = "auth " + password;
		reply = (redisReply*)redisCommand(con, pwd.c_str());
		if(!(reply->type == REDIS_REPLY_STATUS && strcmp(reply->str, "OK") == 0)){
			LOG->note("redis msg:" + (string)reply->str);
			freeReplyObject(reply);
			redisFree(con);
			return 0;
		}
		freeReplyObject(reply);
	}
	_host = host;
	_port = port;
	_password = password;
	return true;
}

bool Redis::reConnect(){
	this->con = redisConnect(_host.c_str(), _port);
	if(this->con->err){
		LOG->note("redis reconnect failed! msg:" + util->intToString(this->con->err));
		redisFree(con);
		return 0;
	}
	if(_password != ""){
		string pwd = "auth " + _password;
		reply = (redisReply*)redisCommand(con, pwd.c_str());
		if(!(reply->type == REDIS_REPLY_STATUS && strcmp(reply->str, "OK") == 0)){
			LOG->note("redis msg:" + (string)reply->str);
			freeReplyObject(reply);
			redisFree(con);
			return 0;
		}
		freeReplyObject(reply);
	}
	LOG->note("redis msg: reconnect success");
	return true;
}

string Redis::getMsg(list<string> keys){
	string msg = "";
	string cmd = "blpop ";
	string key;
	list<string>::iterator it;
	for(it = keys.begin(); it != keys.end(); ++it){
		key += (*it + " ");
	}
	cmd = cmd + key + "0";
	while(NULL == (reply = (redisReply*)redisCommand(con, cmd.c_str()))){
		LOG->note("redis msg: connected error, try to reconnect");
		while(!this->reConnect()){
			sleep(5);
		}
	}
	msg += (string)"" + reply->element[1]->str;
	freeReplyObject(reply);
	return msg;
}

void Redis::setMsg(string key, string value){
	string cmd = "rpush " + key + " %s";
	while(NULL == (reply = (redisReply*)redisCommand(con, cmd.c_str(), value.c_str()))){
		LOG->note("redis msg: connected error, try to reconnect");
		while(!this->reConnect()){
			sleep(5);
		}
	}
	freeReplyObject(reply);
}

int Redis::getQueueLen(string name){
	int len;
	string cmd = "llen " + name;
	while(NULL == (reply = (redisReply*)redisCommand(con, cmd.c_str()))){
		LOG->note("redis msg: connected error, try to reconnect");
		while(!this->reConnect()){
			sleep(5);
		}
	}
	len = reply->integer;
	freeReplyObject(reply);
	return len;
}
