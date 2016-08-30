/*
 * Author: yys
 */
#include "../../include/util.h"
#include "../../include/Redis.h"
#include "../../include/log.h"

class FilterKey{
public:
  FilterKey();
  FilterKey(string invalidWord);
  ~FilterKey();
public:
  void setDefaultInvalid(string invalidWord);
  void setPointInvalid(string invalidWord);
  string getInvalidWord();
  bool filter(string code, string keys = "");
private:
  vector<string> defaultInvalid;
  vector<string> pointInvalid;
  string invalidWord;
};
