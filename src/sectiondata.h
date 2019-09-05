
#ifndef SECTIONDATA_H
#define SECTIONDATA_H

#include "globalheader.h"
#include <list>

class SectionData{
private:
    list<int> *data;
    string sectionName;
public:
    SectionData(string name);
    friend ostream& operator<<(ostream &os, const SectionData &sec);
    void addByte(int byte);
    void addWord(int word);
    void addInt(int op);
    string getName();
    ~SectionData();
};

#endif
