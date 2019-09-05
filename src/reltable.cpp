#include "reltable.h"
#include "globalheader.h"
#include "relrecord.h"


RelocationTable::RelocationTable(string name){
    this->sectionName=name;
    records=new list<RelocationRecord*>();
}

RelocationTable::~RelocationTable() {
    list<RelocationRecord*>::iterator it = records->begin();
    for (int i = 0; i < records->size(); i++) {
        RelocationRecord *rec = *it;
        delete rec;
        advance(it, 1);
    }
    delete records;

}

void RelocationTable::addRecord(RelocationRecord *record){
    records->push_back(record);
}

ostream& operator<<(ostream &os,const RelocationTable &table) {
    os<<"\n-------#.rel."<<table.sectionName<<"-------\n";
    os<<" OFFSET |   TYPE   | VALUE \n";
    list<RelocationRecord*>::iterator it = table.records->begin();
    for (int i = 0; i < table.records->size(); i++) {
        RelocationRecord *rec = *it;
        os<<*rec;
        advance(it, 1);
    }
    return os;
}
