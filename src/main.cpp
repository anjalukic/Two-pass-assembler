#include "globalheader.h"
#include "parser.h"
#include "assembler.h"

int main(int argc, char *argv[]) {

    //get arguments
    string inputFileName;
    string outputFileName;
    if (argc == 4 && (strcmp(argv[1],"-o")==0 || strcmp(argv[2], "-o")==0)) {
        if (strcmp(argv[1],"-o")==0)  {
            outputFileName = argv[2];
            inputFileName = argv[3];
        } else {
            outputFileName = argv[3];
            inputFileName = argv[1];
        }
    } else {
        cout << "error: arguments\n";
        return 0;
    }

    //parse file into array of strings
    Parser* parser=new Parser(inputFileName);
    string** assemblerInput=parser->getParsed();
    int size=parser->getVectorSize();


    Assembler::setOutput(outputFileName);
    Assembler::firstPass(assemblerInput,size);
    Assembler::secondPass(assemblerInput,size);

    delete parser;//delete vector

    return 0;

}
