#pragma once

#include "../../include/util.h"
#include "../../include/Redis.h"
#include "../../include/log.h"

class Compiler{
public:
	Compiler();
	~Compiler();
public:

	void setSrcFileName(string srcFileName);
	void setExcFileName(string excFileName);
	void setOutFileName(string outFileName);
	void setErrorFileName(string errorFileName);
	void setCompileTime(int ct);

	string getSrcFileName();
	string getExcFileName();
	string getOutFileName();
	string getErrorFileName();

	int getCompileTime();

public:
	void setCompileCmd(string cmd);
	int Compile();
	void cleanErrorFile();
private:
	string compileCmd;
	string compilerPath;	
	vector<string> parameters;

	string srcFileName;
	string excFileName;
	string outFileName;
	string errorFileName;
	int compileTime;
};
