#pragma once
#include <string>
#include "marco.h"
using namespace std;

class Excep {
protected:
    string inform;
public:
    Excep(string inform) : inform(inform) {}
    string getInform() { return inform; }
};

class unassignedEvalExcep : public Excep {
public:
    unassignedEvalExcep() : Excep("unassigned Eval") {}
};

class unassignedAssignedExcep : public Excep {
public:
    unassignedAssignedExcep() : Excep("unassigned Assigned") {}
};

class cannotAssignedExcep : public Excep { //这个或许应该加上类型存储
public:
    cannotAssignedExcep() : Excep("cannot Assigned") {}
};

class cannotEvaledExcep : public Excep {
public:
    cannotEvaledExcep() : Excep("cannot Evaled") {}
};

class VarRefUnbindExcep : public Excep {
public:
    VarRefUnbindExcep() : Excep("VarRef Unbind") {}
};

class argumentNumExceedingExcep : public Excep {
public:
    argumentNumExceedingExcep() : Excep("argument Num Exceeding") {}
};

class parameterNumExceedingExcep : public Excep {
public:
    parameterNumExceedingExcep() : Excep("parameter Num Exceeding") {}
};

enum callCheckMismatchType{NumberMismatch, TypeMisMatch};
class callCheckMismatchExcep : public Excep
{
private:
    unsigned int type;
public:
    callCheckMismatchExcep(unsigned int type) : Excep("callCheck Mismatch:"), type(type) { inform+=to_string(type); }
    unsigned int getType() {return this->type;}
};

class addSonExcep : public Excep
{
private:
    unsigned int type;
public:
    addSonExcep(unsigned int type) : Excep("add Son Excep:"), type(type) { inform+=to_string(type); }
    unsigned int getType() {return this->type;}
};


class lexicalExcep : public Excep {
public:
    lexicalExcep(string inform) : Excep("lexical Error: "+inform) {}
};

class parseExcep : public Excep {
public:
    parseExcep(string inform) : Excep("parse Error: " + inform) {}
};