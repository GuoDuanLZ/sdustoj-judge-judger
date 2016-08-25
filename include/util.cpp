#include "util.h"

Util* Util::instance = new Util();

Util* Util::getInstance() {
  return instance;
}

void Util::sortFile(vector<string>& files)
{
	int len = files.size();
	for(int i = 0; i < len; ++i){
		for(int j = 0; j < len - 1 - i; ++j){
			if(files[j] > files[j + 1]){
				string temp = files[j];
				files[j] = files[j + 1];
				files[j + 1] = temp;
			}
		}
	}
}

void Util::getFiles(string dir, vector<string>& files )
{
	DIR *dp;
	struct dirent *entry;
	if((dp = opendir(dir.c_str())) == NULL) {
		fprintf(stderr,"cannot open directory: %s\n", dir.c_str());
		return;
	}
	char filename[1024];
	while((entry = readdir(dp)) != NULL) {
		if(entry->d_type == 8) {
			sprintf(filename,"%s",entry->d_name);
			files.push_back(filename);
		}
	}
	sortFile(files);
	closedir(dp);
}

string Util::getCurPath(){
  string path;
  char buffer[100];
  getcwd(buffer, sizeof(buffer));
  path = buffer;
  return path;
}

string Util::intToString(int i)
{
	char str[15];
  	sprintf(str, "%d", i);
  	return (string) str;
}

int Util::stringToInt(string str)
{
	return atoi(str.c_str());
}

const string Util::currentDateTime()
{
	time_t now = time(NULL);
 	struct tm tstruct;
 	char buf[80];
  	tstruct = *localtime(&now);
  	strftime(buf, sizeof (buf), "%Y-%m-%d %H:%M:%S", &tstruct);

  	return buf;
}

const string Util::currentDate()
{
	time_t now = time(NULL);
  	struct tm tstruct;
  	char buf[80];
  	tstruct = *localtime(&now);
  	strftime(buf, sizeof (buf), "%Y-%m-%d", &tstruct);

  	return buf;
}

vector<string> Util::split(const string &str, char delim, bool removeAppendedNull)
{
	vector<string> elems;
	stringstream ss(str);
  	string item = "";
  	while (getline(ss, item, delim)) {
  		if(item == "") continue;
    	elems.push_back(item);
    	item = "";
  	}
  	if (removeAppendedNull) {
    	while (!elems.empty() && elems.back().empty()) {
      		elems.pop_back();
    	}
  	}
  	if (elems.empty()) {
    	elems.push_back(str);
  	}
  	return elems;
}

vector<string> Util::split(const string &str, char delim)
{
	return split(str, delim, true);
}

string Util::trim(string str)
{
	string spaces = " \t\n\r";
  	int start = str.find_first_not_of(spaces);
  	int end = str.find_last_not_of(spaces);
	if(start == -1 || end == -1){
		return "";
	}
	else{
		return str.substr(start, end - start + 1);
	}
}

string Util::loadAllFromFile(string path, int limit) {
  int lines = 0, tried = 0;
  string res = "", tmps;
  fstream fin(path.c_str(), fstream::in);
  while (fin.fail() && tried < 10) {
    tried++;
    fin.open(path.c_str(), fstream::in);
    return res;
  }
  if (fin.fail()) return res;
  while (getline(fin, tmps)) {
    if (res != "") res += "\n";
    res += tmps;
    lines++;
    if (fin.eof()) break;
    if (limit != -1 && lines > limit) break;
    //getline(fin,tmps);
  }
  fin.close();
  return res;
}

bool Util::isFileExist(string path) {
  int tried = 0;
  if (path == "") return false;
  FILE * fp = fopen(path.c_str(), "r");
  while (fp == nullptr && tried < 5) {
    tried++;
    fp = fopen(path.c_str(), "r");
    usleep(1000);
  }
  if (fp != nullptr) {
    fclose(fp);
    return true;
  }
  return false;
}

bool Util::isFolderExist(string path){
	DIR *dp;
    if ((dp = opendir(path.c_str())) == nullptr)
    {
        return 0;
    }

    closedir(dp);
    return 1;
}

void Util::transformCmd(vector<string> cmd, char** &argv){
  int len = cmd.size();
  argv = new char*[len + 1];
  for(int i = 0; i < len; ++i){
    argv[i] = new char[cmd[i].size() + 1];
    strcpy(argv[i], cmd[i].c_str());
  }
  argv[len] = nullptr;
}

void Util::deleteFile(string path){
	if(path == "*")
		return;
	int ret = system((string("rm ") + path).c_str());
}
