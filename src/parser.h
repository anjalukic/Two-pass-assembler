#ifndef PARSER_H_
#define PARSER_H_

#include "globalheader.h"

class Parser{
private:
    ifstream file;
    string** vector;
    int vectorSize;
public:
    Parser(string fileName);
    void parseFromFile(ifstream& file);
    string** getParsed();
    int getVectorSize();
    ~Parser();
};


#endif
