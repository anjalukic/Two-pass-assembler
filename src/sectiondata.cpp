
#include "sectiondata.h"
#include "globalheader.h"
#include <list>
#include <iterator>

SectionData::SectionData(string name) {
    data=new list<int>();
    sectionName=name;
}

ostream& operator<<(ostream &os, const SectionData &sec){
    os<<"\n---------."<<sec.sectionName<<"---------\n";
    list<int>::iterator it = sec.data->begin();
    int i;
    for (i = 0; i < sec.data->size(); ) {
        int byte= *it;
        if (byte<16 && byte>=0) os<<"0";
        if (byte<0) {
            unsigned char temp=byte;
            byte=temp;
        }
        os << hex<<byte<<" ";

        i++;
        if (i%8==0) os<<"\n";
        advance(it, 1);
    }
    if (i%8!=0) os<<"\n";
    return os;
}

SectionData::~SectionData() {
    delete data;
}

void SectionData::addByte(int byte) {
    data->push_back(byte);
}

string SectionData::getName() {
    return sectionName;
}

void SectionData::addWord(int word){
    data->push_back(word & 255);
    data->push_back((word>>8)&255);
}

void SectionData::addInt(int op){
    data->push_back(op & 255);
    data->push_back((op>>8)& 255);
    data->push_back((op>>16)& 255);
    data->push_back((op>>24)& 255);
}
