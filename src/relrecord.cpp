#include "relrecord.h"
#include "globalheader.h"


RelocationRecord::RelocationRecord(int offset, int type, int value){
    this->offset=offset;
    this->type=type;
    this->value=value;
}

ostream& operator<<(ostream &os, const RelocationRecord &rec) {
    os<<rec.offset;
    if (rec.offset<10) os<<"       ";
    else if (rec.offset<100) os<<"      ";
    else os<<"     ";
    os<<"|"<<(rec.type?"R_386_PC32":"R_386_32  ")<<"|"<<rec.value<<"\n";
    return os;
}
