#ifndef ASSEMBLER_H_
#define ASSEMBLER_H_
#include "globalheader.h"
#include "symbol.h"
#include <list>
#include "sectiondata.h"
#include "reltable.h"

enum Directives{
    NOTFOUND,
    INT,
    LONG,
    SHORT,
    GLOBAL,
    RODATA,
    CHAR,
    EQU,
    SECTION,
    TEXT,
    DATA,
    BSS,
    EXTERN,
    BYTE,
    WORD,
    ALIGN,
    SKIP,
    END
};

enum Instructions{
    HALT,
    XCHG,
    INTI,
    MOV,
    ADD,
    SUB,
    MUL,
    DIV,
    CMP,
    NOT,
    AND,
    OR,
    XOR,
    TEST,
    SHL,
    SHR,
    PUSH,
    POP,
    JMP,
    JEQ,
    JNE,
    JGT,
    CALL,
    RET,
    IRET,
    ILLEGALINSTRUCTION
};

class Assembler{
private:
    static ofstream *file;
    static int lc; //location counter
    static int sectionIndex;//index of current section
    static list<Symbol*> symbolList;
    static list<RelocationTable*> relTableList;
    static list<SectionData*> sectionList;
    static bool endFlag;
    static RelocationTable* currRelTable;
    static SectionData* currSecData;

    static void checkIfDirectiveFP(string line);//first pass
    static void checkIfDirectiveSP(string line);//second pass
    static void checkIfLabelFP(string line);//first pass
    static void checkIfInstructionSP(string line);//second pass
    static Directives hashDirective(string in);
    static Instructions hashInstruction(string in);
    static bool checkIfSymbolExists(string name, int type);
    static string removeAllSpaces(string in);
    static void checkIfInstructionFP(string line);
    static void printSymbolTable();
    static void printSections();
    static bool instructionAllowed();
    static int addressingMode(string operand);
    static int numOfBytesPerOperand(string instruction,int addressingMode);
    static bool isByteInstruction(string instruction);
    static bool writeOperand(string operand,bool isByteInstr);
    static Symbol* findSymbol(string name);
    //static int findCurrSectionIndex();


public:
    static void setOutput(string outputName);
    static void firstPass(string** input, int size);
    static void secondPass(string** input, int size);


};


#endif

