#include "Comparator.h"

Comparator::Comparator(){
	isspj = false;
	spjPath = "";
}

Comparator::~Comparator(){
	;
}

void Comparator::setStdInputName(string stdInputName){
    this->stdInputName = stdInputName;
}

void Comparator::setStdOutputName(string stdOutputName){
    this->stdOutputName = stdOutputName;
}

void Comparator::setUserOutputName(string userOutputName){
    this->userOutputName = userOutputName;
}

void Comparator::setSPJPath(string spjPath){
	this->spjPath = spjPath;
}

void Comparator::setIsSPJ(bool isSpj){
	this->isspj = isSpj;
}

string Comparator::getStdInputName(){
    return this->stdInputName;
}

string Comparator::getStdOutputName(){
    return this->stdOutputName;
}

string Comparator::getUserOutputName(){
    return this->userOutputName;
}

string Comparator::getSPJPath(){
	return this->spjPath;
}

bool Comparator::getIsSPJ(){
	return this->isspj;
}

int Comparator::compare(){
    if(!isspj){
        int ret = compare_normal();
	    return ret;
    }
    else{
        //SPJ should have stdOutput too!!!
        int tpid;
        int testStatus;
        if((tpid = fork()) == 0){
            int ret = execl(spjPath.c_str(), spjPath.c_str(), stdInputName.c_str(), stdOutputName.c_str(), userOutputName.c_str(), NULL);
            exit(ret);
        }
        else{
            waitpid(tpid, &testStatus, 0);
            if(WIFEXITED(testStatus)){
                int ret = WEXITSTATUS(testStatus);
                if(ret == 255){
                    return -1;
                }
                else{
                    return ret;
                }
            }
            else{
                return -1;
            }
        }
    }
}

int Comparator::compare_normal(){
	bool aced = true, peed = true, wa = false;
    FILE *program_out, *standard_out;
    int eofp = EOF, eofs = EOF;
    program_out = fopen(userOutputName.c_str(), "r");
		if(program_out == nullptr){
			LOG->note("open program_out file failed");
			return RESULT_JE;
		}
    standard_out = fopen(stdOutputName.c_str(), "r");
		if(standard_out == nullptr){
			LOG->note("open standard_out file failed");
			return RESULT_JE;
		}
    char po_char, so_char;
    while (1) {
      while ((eofs = fscanf(standard_out, "%c", &so_char)) != EOF &&
          so_char == '\r');
      while ((eofp = fscanf(program_out, "%c", &po_char)) != EOF &&
          po_char == '\r');
      if (eofs == EOF || eofp == EOF) break;
      if (so_char != po_char) {
        aced = false;
        break;
      }
    }
    while ((so_char == '\n' || so_char == '\r') &&
        ((eofs = fscanf(standard_out, "%c", &so_char)) != EOF));
    while ((po_char == '\n' || po_char == '\r') &&
        ((eofp = fscanf(program_out, "%c", &po_char)) != EOF));
    if (eofp != eofs) aced = false;
    fclose(program_out);
    fclose(standard_out);

    if(aced)
        return RESULT_AC;//AC


    program_out = fopen(userOutputName.c_str(), "r");
    standard_out = fopen(stdOutputName.c_str(), "r");
    while (1) {
        while ((eofs = fscanf(standard_out, "%c", &so_char)) != EOF &&
            (so_char == ' ' || so_char == '\n' || so_char == '\r'));
        while ((eofp = fscanf(program_out, "%c", &po_char)) != EOF &&
            (po_char == ' ' || po_char == '\n' || po_char == '\r'));
        if (eofs == EOF || eofp == EOF) break;
        if (so_char != po_char) {
          peed = false;
          break;
        }
    }
    while ((so_char == ' ' || so_char == '\n' || so_char == '\r') &&
      ((eofs = fscanf(standard_out, "%c", &so_char)) != EOF));
    while ((po_char == ' ' || po_char == '\n' || po_char == '\r') &&
      ((eofp = fscanf(program_out, "%c", &po_char)) != EOF));
    if (eofp != eofs) {
        peed = false;
    }
    fclose(program_out);
    fclose(standard_out);

    if(peed)
        return RESULT_PE;//PE
    // }

    program_out = fopen(userOutputName.c_str(), "r");
    standard_out = fopen(stdOutputName.c_str(), "r");
    while (1) {
        while ((eofs = fscanf(standard_out, "%c", &so_char)) != EOF &&
            (so_char == ' ' || so_char == '\n' || so_char == '\r' || ispunct(so_char)));
        while ((eofp = fscanf(program_out, "%c", &po_char)) != EOF &&
            (po_char == ' ' || po_char == '\n' || po_char == '\r' || ispunct(po_char)));
        if (eofs == EOF || eofp == EOF) break;
        if (so_char != po_char) {
          wa = true;
          break;
        }
    }
    while ((so_char == ' ' || so_char == '\n' || so_char == '\r' || ispunct(so_char)) &&
      ((eofs = fscanf(standard_out, "%c", &so_char)) != EOF));
    while ((po_char == ' ' || po_char == '\n' || po_char == '\r' || ispunct(po_char)) &&
      ((eofp = fscanf(program_out, "%c", &po_char)) != EOF));
    if (eofp != eofs) {
        wa = true;
    }
    fclose(program_out);
    fclose(standard_out);

    if(!wa)
        return RESULT_PTE;
    else
        return RESULT_WA;//WA
}
