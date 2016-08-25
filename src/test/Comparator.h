#pragma once

#include "../../include/util.h"
#include "../../include/Redis.h"
#include "../../include/log.h"

class Comparator{

public:
	Comparator();
	virtual ~Comparator();

public:
	void setStdInputName(string stdInputName);
	void setStdOutputName(string stdOutputName);
	void setUserOutputName(string userOutputName);
	void setSPJPath(string spjPath);
	void setIsSPJ(bool isspj);

	string getStdInputName();
	string getStdOutputName();
	string getUserOutputName();
	string getSPJPath();
	bool getIsSPJ();

	int compare();

private:
	int compare_normal();
private:
	string stdInputName;
	string stdOutputName;
	string userOutputName;
	string spjPath;

	bool isspj;
};
