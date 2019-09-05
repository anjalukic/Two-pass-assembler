#ifndef RELRECORD_H
#define RELRECORD_H

#include "globalheader.h"

class RelocationRecord{
private:
    int offset;
    int type;//0 - aps, 1 - rel
    int value;
public:
    RelocationRecord(int offset, int type, int value);

    friend ostream& operator<<(ostream& os, const RelocationRecord& rec);

};


#endif
