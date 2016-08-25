/*
 * File:   util.h
 * Author: yys
 *
 */

#pragma once

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cassert>

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <istream>
#include <sstream>
#include <fstream>

#include <unistd.h>
#include <errno.h>
#include <sys/syscall.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <dirent.h>

#include <json/json.h>

using namespace std;

//#define DEBUG 1

#define util Util::getInstance()

#define IF_NULL_STR(x) if((x) == ""){return false;}

#define COMPILE_ACCEPT		0
#define COMPILE_ERROR 		1
#define COMPILE_OT 				2
#define COMPILE_INTERRUPT 3

#define RESULT_AC 	0
#define RESULT_PE 	1
#define RESULT_PTE 	2
#define RESULT_WA 	3
#define RESULT_JE 	4

#define LT_C 			0
#define LT_CPP 		1
#define LT_PASCAL 2
#define LT_JAVA 	3
#define LT_RUBY 	4
#define LT_BASH 	5
#define LT_PYTHON 6
#define LT_PHP		7
#define LT_PERL		8
#define LT_MONOCS	9
#define LT_OBJC		10
#define LT_FBASIC	11
#define LT_BF 		12
#define LT_OTHER 	13


class Util{
public:
	~Util(){}
public:
	void sortFile(vector<string>& files);
	void getFiles(string dir, vector<string>& files );

	string getCurPath();

	string intToString(int);
	int stringToInt(string);

	const string currentDateTime();
	const string currentDate();

	void transformCmd(vector<string> cmd, char** &argv);

	vector<string> split(const string &, char, bool);
	vector<string> split(const string &, char);

	string trim(string);

	string loadAllFromFile(string path, int limit);
	bool isFileExist(string path);
	bool isFolderExist(string path);
	void deleteFile(string path);

public:
	static Util* getInstance();
private:
	Util(){}
	static Util* instance;
};
