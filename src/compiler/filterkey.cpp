#include "filterkey.h"

FilterKey::FilterKey(){
  invalidWord = "";
}

FilterKey::FilterKey(string invalidWord){
  this->setDefaultInvalid(invalidWord);
}

FilterKey::~FilterKey(){
  defaultInvalid.clear();
  pointInvalid.clear();
}

void FilterKey::setDefaultInvalid(string invalidWord){
  this->defaultInvalid.clear();
  this->defaultInvalid = util->split(invalidWord, ' ');
}

void FilterKey::setPointInvalid(string invalidWord){
  this->pointInvalid.clear();
  this->pointInvalid = util->split(invalidWord, ' ');
}

string FilterKey::getInvalidWord(){
  return this->invalidWord;
}

bool FilterKey::filter(string code, string keys){
  if(keys != ""){
    this->setPointInvalid(keys);
  }
  int len = defaultInvalid.size();
  for(int i = 0; i < len; ++i){
    if(code.find(defaultInvalid[i]) != -1){
      invalidWord = defaultInvalid[i];
      return false;
    }
  }

  len = pointInvalid.size();
  for(int i = 0; i < len; ++i){
    if(code.find(pointInvalid[i]) != -1){
      invalidWord = pointInvalid[i];
      return false;
    }
  }
  return true;
}
