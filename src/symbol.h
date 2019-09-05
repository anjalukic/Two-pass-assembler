#ifndef SYMBOL_H_
#define SYMBOL_H_
#include "globalheader.h"

class Symbol{
private:
    static int globalIndex;
    string name;
    int section;
    int value;
    char scope;
    int index;
    int size;
    int type;// 0 - section, 1 - symbol
    string rwx;

public:
    static bool printFlag;
    Symbol(string name, int value, int type, int section);
    Symbol(string name, int value, int type, int section, string rwx);

    void setGlobal();
    void setSize(int size);
    string getName();
    int getType();
    void setRwx(string rwx);
    string getRwx();
    int getValue();
    bool isGlobal();
    int getIndex();
    int getSectionIndex();
    static int getCurrIndex();

    friend ostream& operator<<(ostream& os, const Symbol& sym);
};


#endif

