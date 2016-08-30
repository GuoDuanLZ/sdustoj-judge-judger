#include "Comparator.h"

Comparator::Comparator(){
	isspj = false;
	spjCmd = "";
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

void Comparator::setSPJCmd(string cmd){
	this->spjCmd = cmd;
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

string Comparator::getSPJCmd(){
	return this->spjCmd;
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
				struct rlimit time_limit;
				time_limit.rlim_cur 	= 5;
				time_limit.rlim_max 	= 6;
        if((tpid = fork()) == 0){
					setrlimit(RLIMIT_CPU, &time_limit);
					vector<string> parameters = util->split(spjCmd, ' ');
					string fileName = parameters[0];
					parameters.erase(parameters.begin());
					parameters.push_back(stdInputName);
					parameters.push_back(stdOutputName);
					parameters.push_back(userOutputName);
					char** argv = nullptr;
					util->transformCmd(parameters, argv);
					int ret = execv(fileName.c_str(), argv);
          exit(ret);
        }
        else{
					struct timeval startv, nowv;
					gettimeofday(&startv, NULL);
					while(1){
							waitpid(tpid, &testStatus, WNOHANG);
							usleep(5000);
							gettimeofday(&nowv, NULL);
							int absTimeUsed = nowv.tv_sec - startv.tv_sec;
							if(absTimeUsed > 5){//5s limit
									ptrace(PTRACE_KILL, tpid, NULL, NULL);
									waitpid(tpid, &testStatus, WNOHANG);
									#ifdef DEBUG
									LOG->note("SPJ TLE error", true);
									#endif
									return RESULT_WA;
							}
							if(WIFEXITED(testStatus)){
									int ret = WEXITSTATUS(testStatus);
									if(ret == 255){
											waitpid(tpid, &testStatus, WNOHANG);
											#ifdef DEBUG
											LOG->note("SPJ error");
											#endif
											return RESULT_WA;
									}
									else{
											waitpid(tpid, &testStatus, WNOHANG);
											#ifdef DEBUG
											LOG->note("SPJ AC");
											#endif
											return RESULT_AC;
									}
							}
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
