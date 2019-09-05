#ifndef RELTABLE_H
#define RELTABLE_H

#include "globalheader.h"
#include <list>
#include "relrecord.h"

class RelocationTable{
private:

    string sectionName;
    list<RelocationRecord*> *records;
public:
    RelocationTable(string name);
    void addRecord(RelocationRecord *record);
    ~RelocationTable();

    friend ostream& operator<<(ostream& os, const RelocationTable& table);

};


#endif
