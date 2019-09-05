#include "symbol.h"
#include "globalheader.h"

int Symbol::globalIndex=0;
bool Symbol::printFlag=false;

Symbol::Symbol(string name, int value, int type, int section){
    index=globalIndex++;
    this->name=name;
    this->value=value;
    this->size=0;
    this->type=type;
    this->scope='l';
    this->section=section;
    this->rwx="rw";
}

Symbol::Symbol(string name, int value, int type, int section,string rwx){
    index=globalIndex++;
    this->name=name;
    this->value=value;
    this->size=0;
    this->type=type;
    this->scope='l';
    this->section=section;
    this->rwx=rwx;
}

string Symbol::getName(){
    return name;
}
int Symbol::getType(){
    return type;
}

void Symbol::setGlobal(){
    this->scope='g';
}

void Symbol::setSize(int size){
    this->size=size;
}

int Symbol::getCurrIndex(){
    return globalIndex;
}

void Symbol::setRwx(string rwx){
    this->rwx=rwx;
}

ostream& operator<<(ostream& os, const Symbol& sym){
    if (!Symbol::printFlag){
       Symbol::printFlag=true;
       os<<"\n---------------------------Tabela simbola---------------------------\n";
       os<<"   NAME   |   TYPE   | VALUE | SECTION | SCOPE | INDEX | SIZE | FLAGS\n";
    }

    os<<sym.name;
    if (sym.name.size()<10){
        for (int i=0; i<10-sym.name.size();i++)
        os<<" ";
    }

    os<<"|"<<(sym.type?"symbol":"section") ;
    if (sym.type==0) os<<"   ";
    else os<<"    ";

    os<<"|"<<sym.value;
    if (sym.value<10) os<<"      ";
    else if (sym.value<100) os<<"     ";
    else os<<"    ";

    os<<"|"<<sym.section;
    if (sym.section<10) os<<"        ";
    else if (sym.section<100) os<<"       ";
    else os<<"      ";

    os<<"|"<<sym.scope;
    if (sym.scope<10) os<<"       ";
    else if (sym.scope<100) os<<"      ";
    else os<<"     ";

    os<<" |"<<sym.index;
    if (sym.index<10) os<<"      ";
    else if (sym.index<100) os<<"     ";
    else os<<"    ";

    os<< "|"<<sym.size;
    if (sym.size<10) os<<"     ";
    else if (sym.size<100) os<<"    ";
    else os<<"   ";

    os<<"| "<<sym.rwx;

    os<<"\n";
    return os;
}

string Symbol::getRwx() {
    return rwx;
}

int Symbol::getValue() {
    return value;
}

bool Symbol::isGlobal() {
    if (scope=='g') return true;
    else return false;
}

int Symbol::getIndex(){
    return index;
}

int Symbol::getSectionIndex() {
    return section;
}

