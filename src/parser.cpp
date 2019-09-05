#include "parser.h"
#include "globalheader.h"
#include <algorithm>

Parser::Parser(string fileName) {
    ifstream inputFile(fileName, ios::in);
    int numOfLines = count(istreambuf_iterator<char>(inputFile),
                           istreambuf_iterator<char>(), '\n') + 1;
    vector = new string*[numOfLines];
    vectorSize = numOfLines;
    inputFile.seekg(0);
    parseFromFile(inputFile);

    inputFile.close();


}

Parser::~Parser() {

    for (int i=0; i<vectorSize; i++){
        if (vector[i]!=NULL)
            delete vector[i];
    }
	delete[] vector;
}

void Parser::parseFromFile(ifstream& file) {
    char line[40];
    int i = 0;

    while (file) {
        file.getline(line, 40);
        vector[i++]=new string(line);
    }
}

string** Parser::getParsed() {
    return vector;
}

int Parser::getVectorSize(){
    return vectorSize;
}

