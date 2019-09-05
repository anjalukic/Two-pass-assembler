/*
TODO:
  - directives - check for regular expressions
  - return from first pass checkIf - true/false, error if not found
  - errors handling
*/
#include "assembler.h"
#include "globalheader.h"
#include <list>
#include <iterator>
#include <regex>
#include <cmath>
#include "relrecord.h"
#include "reltable.h"
#include "sectiondata.h"
#include <ctype.h>

ofstream *Assembler::file = NULL;
int Assembler::lc = 0;
int Assembler::sectionIndex = 0;
bool Assembler::endFlag = false;
list<Symbol *> Assembler::symbolList = list<Symbol *>();
list<RelocationTable *> Assembler::relTableList = list<RelocationTable *>();
list<SectionData *> Assembler::sectionList = list<SectionData *>();
RelocationTable* Assembler::currRelTable=0;
SectionData* Assembler::currSecData=0;


void Assembler::firstPass(string **input, int size) {
    symbolList.push_back(new Symbol("UND", 0, 0, 0, " - "));
    for (int i = 0; i < size; i++) {
        if (endFlag) break;
        string line = *input[i];
        if (!line.empty()) {
            int startIndex = line.find_first_not_of(" \t");
            line = line.substr(startIndex);
            checkIfLabelFP(line);


        }
    }
    printSymbolTable();
}

void Assembler::secondPass(string **input, int size) {

    lc = 0;
    sectionIndex = 0;

    endFlag = false;
    for (int i = 0; i < size; i++) {
        if (endFlag) break;
        string line = *input[i];
        if (!line.empty()) {
            int startIndex = line.find_first_not_of(" \t");
            line = line.substr(startIndex);
            checkIfDirectiveSP(line);
            checkIfInstructionSP(line);


        }
    }


     printSymbolTable();
     printSections();

    Symbol::printFlag=false;
    //write to file

    list<Symbol *>::iterator it = symbolList.begin();
    for (int i = 0;i < symbolList.size();i++) {
        Symbol *sym = *it;
        *file << *sym;
        advance(it,1);
    }

    list<RelocationTable *>::iterator iter = relTableList.begin();
    for (int i = 0; i < relTableList.size(); i++) {
        RelocationTable *tab = *iter;
        *file << *tab;
        advance(iter, 1);
    }

    list<SectionData *>::iterator ite = sectionList.begin();
    for (int i = 0;i < sectionList.size();i++) {
        SectionData *data = *ite;
        *file << *data;
        advance(ite,1);
    }

    file->close();
    delete file;


    //delete symbolTable
    it = symbolList.begin();
    for (int i = 0;i < symbolList.size();i++) {
        Symbol *sym = *it;
        delete sym;
        advance(it,1);
    }

    //delete relocation tables and sections
    iter = relTableList.begin();
    for (int i = 0; i < relTableList.size(); i++) {
        RelocationTable *tab = *iter;
        delete tab;
        advance(iter, 1);
    }
    ite = sectionList.begin();
    for (int i = 0;i < sectionList.size();i++) {
        SectionData *data = *ite;
        delete data;
        advance(ite,1);
    }

}

void Assembler::setOutput(string outputName) {
    file = new ofstream(outputName);

}

void Assembler::checkIfDirectiveFP(string line) {
    if (line[0] != '.') return;
    int startIndex;
    string directive;
    if ((startIndex = line.find_first_of(" ")) != string::npos) {
        directive = line.substr(0, startIndex);
        line = line.substr(startIndex + 1, string::npos);
        if (line[0] == ' ') {
            int ind = line.find_first_not_of(" ");
            line = line.substr(ind);
        }
    } else {
        directive = line;
        if (startIndex = directive.find_first_of("\r") != string::npos) {
            directive = directive.substr(0, startIndex);
        }
    }
    int endOfSym, it;
    Symbol *temp;
    regex reg;
    smatch matches;
    string help;
    string secName, flags;
    list<Symbol *>::iterator iter;
    switch (hashDirective(directive)) {
        case EQU:
            reg = ("\s*(\\w+)\s*,\s*(\\w+)\s*");
            if (regex_search(line, matches, reg)) {
                help = matches[2];
                if (!checkIfSymbolExists(matches[1], 1))
                    symbolList.push_back(new Symbol(matches[1], atoi(help.c_str()), 1, sectionIndex, " - "));
            }
            break;
        case SECTION:
            //operands parsing
            reg = ("(\\w+)");
            it = 0;
            for (auto i = sregex_iterator(line.begin(), line.end(), reg);
                 i != sregex_iterator();
                 ++i) {
                matches = *i;
                it++;
                if (it == 1) secName = matches.str();
                else flags = matches.str();
            }
            //check if section name already exists
            if (checkIfSymbolExists(secName, 0)) return;
            //adding a new section
            iter = symbolList.begin();
            advance(iter, sectionIndex);
            temp = *iter;
            temp->setSize(lc);
            symbolList.push_back(temp = new Symbol(secName, 0, 0, Symbol::getCurrIndex(), "rw"));
            sectionIndex = Symbol::getCurrIndex() - 1;
            lc = 0;
            if (it == 2) {
                temp->setRwx(flags);
            }
            break;
        case TEXT:
            iter = symbolList.begin();
            advance(iter, sectionIndex);
            temp = *iter;
            temp->setSize(lc);
            symbolList.push_back(new Symbol("text", 0, 0, Symbol::getCurrIndex(), "x"));
            sectionIndex = Symbol::getCurrIndex() - 1;
            lc = 0;
            break;
        case DATA:
            iter = symbolList.begin();
            advance(iter, sectionIndex);
            temp = *iter;
            temp->setSize(lc);
            symbolList.push_back(new Symbol("data", 0, 0, Symbol::getCurrIndex(), "rw"));
            sectionIndex = Symbol::getCurrIndex() - 1;
            lc = 0;
            break;
        case BSS:
            iter = symbolList.begin();
            advance(iter, sectionIndex);
            temp = *iter;
            temp->setSize(lc);
            symbolList.push_back(new Symbol("bss", 0, 0, Symbol::getCurrIndex(), "rw"));
            sectionIndex = Symbol::getCurrIndex() - 1;
            lc = 0;
            break;
        case EXTERN:
            endOfSym = line.find_first_of(", \0");
            symbolList.push_back(temp = new Symbol(line.substr(0, endOfSym), lc, 1, 0, " - "));
            temp->setGlobal();
            it = 0;
            while ((it = line.find_first_of(',')) != string::npos) {
                line = line.substr(it + 1, string::npos);
                line = line.substr(line.find_first_not_of(' '), string::npos);
                endOfSym = line.find_first_of(" ,\0");
                symbolList.push_back(temp = new Symbol(line.substr(0, endOfSym), lc, 1, 0, " - "));
                temp->setGlobal();
            }
            break;
        case BYTE:
        case CHAR:
            iter = symbolList.begin();
            advance(iter, sectionIndex);
            temp = *iter;
            if (temp->getName()=="bss") {cout<<"error - byte directive in bss\n"; return;}
            reg = ("([0-9]+|[a-zA-Z][a-zA-Z0-9]*)");
            for (auto it = sregex_iterator(line.begin(), line.end(), reg);
                 it != sregex_iterator();
                 ++it) {
                //matches = *it;
                //  help = matches.str();
                lc++;
            }
            break;
        case WORD:
        case SHORT:
            iter = symbolList.begin();
            advance(iter, sectionIndex);
            temp = *iter;
            if (temp->getName()=="bss") {cout<<"error - byte directive in bss\n"; return;}
            reg = ("([0-9]+|[a-zA-Z][a-zA-Z0-9]*)");
            for (auto it = sregex_iterator(line.begin(), line.end(), reg);
                 it != sregex_iterator();
                 ++it) {
                //matches = *it;
                //  help = matches.str();
                lc+=2;
            }
            break;
        case ALIGN:
            iter = symbolList.begin();
            advance(iter, sectionIndex);
            temp = *iter;
            if (temp->getName()=="bss") {cout<<"error - align directive in bss\n"; return;}
            reg = ("(\\d+)");
            it = 0;
            int array[3];
            for (auto iter = sregex_iterator(line.begin(), line.end(), reg);
                 iter != sregex_iterator();
                 ++iter) {
                matches = *iter;
                array[it++] = atoi((matches.str()).c_str());
            }
            if (it == 3) {
                //3 operanda
                int power = (int) pow(2, array[0]);
                if (lc % power == 0) break;
                else if (array[0] < array[2]) {
                    int rem = lc % power;
                    lc += power - rem;
                } else break;
            } else if (it == 2 || it == 1) {
                //2 ili 1 operand
                int power = (int) pow(2, array[0]);
                if (lc % power == 0) break;
                else {
                    int rem = lc % power;
                    lc += power - rem;
                }
            } else cout << "error ";
            break;
        case SKIP:
            reg = ("\s*(\\w+)\s*");
            if (regex_search(line, matches, reg)) {
                help = matches[1];
                lc += atoi(help.c_str());
            }
            break;
        case END:
            iter = symbolList.begin();
            advance(iter, sectionIndex);
            temp = *iter;
            temp->setSize(lc);
            endFlag = true;
            break;
        case INT:
        case LONG:
            iter = symbolList.begin();
            advance(iter, sectionIndex);
            temp = *iter;
            if (temp->getName()=="bss") {cout<<"error - byte directive in bss\n"; return;}
            reg = ("([0-9]+|[a-zA-Z][a-zA-Z0-9]*)");
            for (auto it = sregex_iterator(line.begin(), line.end(), reg);
                 it != sregex_iterator();
                 ++it) {
                //matches = *it;
                //  help = matches.str();
                lc+=4;
            }
            break;
            //.asciz?
        case RODATA:
            iter = symbolList.begin();
            advance(iter, sectionIndex);
            temp = *iter;
            temp->setSize(lc);
            symbolList.push_back(new Symbol("rodata", 0, 0, Symbol::getCurrIndex(), "r"));
            sectionIndex = Symbol::getCurrIndex() - 1;
            lc = 0;
            break;
        case NOTFOUND:
            cout << " - unknown directive\n";
        default:
            ;

    }

}

void Assembler::checkIfLabelFP(string line) {

    if (line[0]=='.') {checkIfDirectiveFP(line);return;}
    int index = 0;
    if ((index = line.find(':')) == string::npos) {checkIfInstructionFP(line);return;}
    while ((index = line.find(':')) != string::npos){
        string label=line.substr(0,index);
        label=removeAllSpaces(label);
        if (!checkIfSymbolExists(label, 1)) {
            symbolList.push_back(new Symbol(label, lc, 1, sectionIndex, " - "));
        }
        line=line.substr(index+1);
    }
    if (line.empty()) return;
    index = line.find_first_not_of(" \t");
    line = line.substr(index);
    checkIfDirectiveFP(line);
   checkIfInstructionFP(line);
}

Directives Assembler::hashDirective(string in) {
    if (in == ".equ") return EQU;
    if (in == ".section") return SECTION;
    if (in == ".text") return TEXT;
    if (in == ".data") return DATA;
    if (in == ".bss") return BSS;
    if (in == ".extern") return EXTERN;
    if (in == ".byte") return BYTE;
    if (in == ".word")return WORD;
    if (in == ".align") return ALIGN;
    if (in == ".skip") return SKIP;
    if (in == ".end") return END;
    if (in == ".int") return INT;
    if (in == ".long") return LONG;
    if (in == ".global") return GLOBAL;
    if (in == ".rodata") return RODATA;
    if (in == ".char") return CHAR;
    if (in == ".short") return SHORT;
    return NOTFOUND;
}

Instructions Assembler::hashInstruction(string in) {
    if (in == "halt") return HALT;
    if (in == "xchg" || in =="xchgw" || in =="xchgb") return XCHG;
    if (in == "int") return INTI;
    if (in == "mov" || in =="movb" || in =="movw") return MOV;
    if (in == "add" || in =="addw" || in =="addb") return ADD;
    if (in == "sub" || in =="subw" || in =="subb") return SUB;
    if (in == "mul" || in =="mulw" || in =="mulb") return MUL;
    if (in == "div" || in =="divw" || in =="divb") return DIV;
    if (in == "cmp" || in =="cmpw" || in =="cmpb") return CMP;
    if (in == "not" || in =="notw" || in =="notb") return NOT;
    if (in == "and" || in =="andw" || in =="andb") return AND;
    if (in == "or" || in =="orw" || in =="orb") return OR;
    if (in == "xor" || in =="xorw" || in =="xorb") return XOR;
    if (in == "test" || in =="testw" || in =="testb") return TEST;
    if (in == "shl" || in =="shlw" || in =="shlb") return SHL;
    if (in == "shr" || in =="shrw" || in =="shrb") return SHR;
    if (in == "push" || in =="pushw" || in =="pushb") return PUSH;
    if (in == "pop" || in =="popw" || in =="popb") return POP;
    if (in == "jmp") return JMP;
    if (in == "jeq") return JEQ;
    if (in == "jne") return JNE;
    if (in == "jgt") return JGT;
    if (in == "call") return CALL;
    if (in == "ret") return RET;
    if (in == "iret") return IRET;
    return ILLEGALINSTRUCTION;

}


bool Assembler::checkIfSymbolExists(string name, int type) {
    list<Symbol *>::iterator iter = symbolList.begin();
    Symbol *temp;
    for (int i = 0; i < symbolList.size(); i++) {
        temp = *iter;
        if (temp->getName() == name && temp->getType() == type) {
            cout << "error - symbol not found\n";
            return true;
        }
        advance(iter, 1);
    }
    return false;
}

void Assembler::checkIfDirectiveSP(string line) {
    int index = 0;
    while ((index = line.find(':')) != string::npos){
        line=line.substr(index+1);
    }
    if (line.empty()) return;
    index = line.find_first_not_of(" \t");
    line = line.substr(index);
    if (line[0] != '.') return;

    //parsing operands
    int startIndex; string directive;
    if ((startIndex = line.find_first_of(" ")) != string::npos) {
        directive = line.substr(0, startIndex);
        line = line.substr(startIndex + 1, string::npos);
        if (line[0] == ' ') {
            int ind = line.find_first_not_of(" ");
            line = line.substr(ind);
        }
    } else {
        directive = line;
        if (startIndex = directive.find_first_of("\r") != string::npos) {
            directive = directive.substr(0, startIndex);
        }
    }
    //line equals operands , parsed directive
    int endOfSym, it;
    int flag,op;
    Symbol *temp;
    regex reg;
    smatch matches;
    string help;
    string secName, flags;
    list<Symbol *>::iterator iter;
    switch (hashDirective(directive)) {
         case SECTION:
             //operands parsing
             reg = ("(\\w+)");
             it = 0;
             for (auto i = sregex_iterator(line.begin(), line.end(), reg);
                  i != sregex_iterator();
                  ++i) {
                 matches = *i;
                 it++;
                 if (it == 1) secName = matches.str();
                 else flags = matches.str();
             }
             //adding a new section
             lc = 0;
            currRelTable=new RelocationTable(secName);
            currSecData=new SectionData(secName);
            relTableList.push_back(currRelTable);
            sectionList.push_back(currSecData);
             break;

        case TEXT:
            currRelTable=new RelocationTable("text");
            currSecData=new SectionData("text");
            relTableList.push_back(currRelTable);
            sectionList.push_back(currSecData);
            lc = 0;
            break;

        case DATA:
            currRelTable=new RelocationTable("data");
            currSecData=new SectionData("data");
            relTableList.push_back(currRelTable);
            sectionList.push_back(currSecData);
            lc = 0;
            break;
        case BSS:
            currRelTable=0;
            currSecData=0;
            lc = 0;
            break;

        case BYTE:
        case CHAR:
            if (currSecData==0) {cout<<"error - directive in wrong section\n";return;}
            reg = ("([0-9]+|[a-zA-Z][a-zA-Z0-9]*)");
            for (auto it = sregex_iterator(line.begin(), line.end(), reg);
                 it != sregex_iterator();
                 ++it) {
                matches = *it;
                string help=matches.str();
                if (isalpha(help[0])){
                    if ((temp=findSymbol(help))==0) {cout<<"error - symbol "<<help<<" not found\n"; return;}
                    if (temp->isGlobal()){
                        currSecData->addByte(0);
                        currRelTable->addRecord(new RelocationRecord(lc,0,temp->getIndex()));
                        lc++;
                    }else{
                        currSecData->addByte(temp->getValue());
                        currRelTable->addRecord(new RelocationRecord(lc,0,temp->getSectionIndex()));
                        lc++;
                    }
                }else{
                    op = atoi(matches.str().c_str());
                    if (op>255) {cout<<"error - value too big for byte\n";return;}
                    currSecData->addByte(op);
                    lc++;
                }
            }
            break;
        case WORD:
        case SHORT:
            if (currSecData==0) {cout<<"error - directive in wrong section\n";return;}
            reg = ("([0-9]+|[a-zA-Z][a-zA-Z0-9]*)");
            for (auto it = sregex_iterator(line.begin(), line.end(), reg);
                 it != sregex_iterator();
                 ++it) {
                matches = *it;
                string help=matches.str();
                if (isalpha(help[0])){
                    if ((temp=findSymbol(help))==0) {cout<<"error - symbol "<<help<<" not found\n"; return;}
                    if (temp->isGlobal()){
                        currSecData->addWord(0);
                        currRelTable->addRecord(new RelocationRecord(lc,0,temp->getIndex()));
                        lc+=2;
                    }else{
                        currSecData->addWord(temp->getValue());
                        currRelTable->addRecord(new RelocationRecord(lc,0,temp->getSectionIndex()));
                        lc+=2;
                    }
                }else{
                    op = atoi(matches.str().c_str());
                    if (op>65535) {cout<<"error - value too big for byte\n";return;}
                    currSecData->addWord(op);
                    lc+=2;
                }
            }
            break;
        case ALIGN:
            if (currSecData==0) {cout<<"error - directive in wrong section\n";return;}
            reg = ("(\\d+)");
            it = 0;
            int array[3];
            for (auto iter = sregex_iterator(line.begin(), line.end(), reg);
                 iter != sregex_iterator();
                 ++iter) {
                matches = *iter;
                array[it++] = atoi((matches.str()).c_str());
            }
            if (it == 3) {
                //3 operanda
                int power = (int) pow(2, array[0]);
                if (lc % power == 0) return;

                if (array[0] < array[2]) {
                    int add = power-(lc % power);
                    lc += add;
                    for (int i=0; i<add; i++) currSecData->addByte(array[1]);
                } else return;
            } else if (it == 2 || it == 1) {
                //2 ili 1 operand
                int power = (int) pow(2, array[0]);
                if (lc % power == 0) return;
                int add = power-(lc % power);
                lc += add;
                if (it == 2) {
                    for (int i = 0; i < add; i++) currSecData->addByte(array[1]);
                } else{
                    for (int i=0; i<add; i++) currSecData->addByte(0);
                }

            } else {cout << "error in align directive\n";return;}
            break;
            case SKIP:
                 reg = ("\s*(\\w+)\s*");
                 if (regex_search(line, matches, reg)) {
                     help = matches[1];
                     int add=atoi(help.c_str());
                     lc += add;
                     if (currSecData!=0) {
                         for (int i = 0; i < add; i++)
                             currSecData->addByte(0);
                     }

                 }
                 break;
             case END:
                 endFlag = true;
                 break;

        case INT:
        case LONG:
            if (currSecData==0) {cout<<"error - directive in wrong section\n";return;}
            reg = ("([0-9]+|[a-zA-Z][a-zA-Z0-9]*)");
            for (auto it = sregex_iterator(line.begin(), line.end(), reg);
                 it != sregex_iterator();
                 ++it) {
                matches = *it;
                string help=matches.str();
                if (isalpha(help[0])){
                    if ((temp=findSymbol(help))==0) {cout<<"error - symbol "<<help<<" not found\n"; return;}
                    if (temp->isGlobal()){
                        currSecData->addInt(0);
                        currRelTable->addRecord(new RelocationRecord(lc,0,temp->getIndex()));
                        lc+=4;
                    }else{
                        currSecData->addInt(temp->getValue());
                        currRelTable->addRecord(new RelocationRecord(lc,0,temp->getSectionIndex()));
                        lc+=4;
                    }
                }else{
                    op = atoi(matches.str().c_str());
                    if (op>2147483647) {cout<<"error - value too big for byte\n";return;}
                    currSecData->addInt(op);
                    lc+=4;
                }
            }
            break;
            //.asciz?

        case RODATA:
            currRelTable=new RelocationTable("rodata");
            currSecData=new SectionData("rodata");
            relTableList.push_back(currRelTable);
            sectionList.push_back(currSecData);
            lc = 0;
            break;
        case GLOBAL:
            endOfSym = line.find_first_of(", \0");
            help=line.substr(0, endOfSym);
            flag=0;
            iter = symbolList.begin();
            for (int i = 0; i < symbolList.size(); i++) {
                temp = *iter;
                if (temp->getName() == help && temp->getType()==1) {temp->setGlobal();flag=1;break;}
                advance(iter, 1);
            }
            it = 0;
            while ((it = line.find_first_of(',')) != string::npos) {
                flag=0;
                line = line.substr(it + 1, string::npos);
                line = line.substr(line.find_first_not_of(' '), string::npos);
                endOfSym = line.find_first_of(" ,\0");
                help=line.substr(0,endOfSym);
                iter= symbolList.begin();
                for (int i = 0; i < symbolList.size(); i++) {
                    temp = *iter;
                    if (temp->getName() == help && temp->getType()==1) {temp->setGlobal();flag=1;break;}
                    advance(iter, 1);
                }
            }
            if (flag==0) cout<<"error in .global directive - symbol not found\n";
            break;
        case NOTFOUND:
            cout << " - unknown directive\n";

        default:

            ;

    }

}

void Assembler::checkIfInstructionSP(string line) {
    if (line[0]=='.') return;
    int index = 0;
    while ((index = line.find(':')) != string::npos){
        line=line.substr(index+1);
    }
    int startIndex;
    if ((startIndex= line.find_first_not_of(" \t"))!=string::npos)
        line = line.substr(startIndex);

    if (line.empty()) return;
    //line starts with instruction
    //parsing operands
    string instruction,operands;
    if ((startIndex = line.find_first_of(" ")) != string::npos) {
        instruction = line.substr(0, startIndex);
        operands = line.substr(startIndex + 1, string::npos);
        operands=removeAllSpaces(operands);
    } else {
        instruction = line;
        if (startIndex = instruction.find_first_of("\r") != string::npos) {
            instruction = instruction.substr(0, startIndex);
        }
    }

    //line equals operands , parsed instruction
    string operand1="";
    string operand2="";
    if (!operands.empty()) {
        if ((index = operands.find_first_of(',')) != string::npos) {
            operand1 = operands.substr(0, index);
            operand2 = operands.substr(index + 1, string::npos);
        }else {
            operand1=operands;
        }
    }
    //instruction and operands parsed
    int oc;
    unsigned bmask=~4;//with and
    unsigned wmask=4;//with or
    int am1,am2;
    am1=addressingMode(operand1);
    am2=addressingMode(operand2);
    bool isByteInstr=isByteInstruction(instruction);
    switch (hashInstruction(instruction)){
        case HALT:
            oc=1<<3;
            currSecData->addByte(oc);
            lc+=1;
            break;
        case XCHG:
            if (am1<=0 || am2<=0) {cout<<"Error - operands in exchange instruction\n"; return;}
            oc=2<<3;
            if (isByteInstr) oc=oc&bmask;
            else oc=oc|wmask;
            currSecData->addByte(oc);
            lc++;
            if (!writeOperand(operand1,isByteInstr) ||
                !writeOperand(operand2,isByteInstr)) {
                cout<<"error - operands\n";
                return;
            }
            break;
        case INTI:
            if (am1<0) {cout<<"Error - operands in int instruction\n"; return;}
            if (operand2!="") {cout<<"error - too many operands for int\n"; return;}
            oc=3<<3;
            currSecData->addByte(oc);
            lc++;
            if (!writeOperand(operand1,isByteInstr)) {
                cout<<"error - operands\n";
                return;
            }
            break;
        case MOV:
            if (am1<=0 || am2<0) {cout<<"Error - operands in add instruction\n"; return;}
            oc=4<<3;
            if (isByteInstr) oc=oc&bmask;
            else oc=oc|wmask;
            currSecData->addByte(oc);
            lc++;
            if (!writeOperand(operand1,isByteInstr) ||
                !writeOperand(operand2,isByteInstr)) {
                cout<<"error - operands\n";
                return;
            }
            break;
        case ADD:
            if (am1<=0 || am2<0) {cout<<"Error - operands in add instruction\n"; return;}
            oc=5<<3;
            if (isByteInstr) oc=oc&bmask;
            else oc=oc|wmask;
            currSecData->addByte(oc);
            lc++;
            if (!writeOperand(operand1,isByteInstr) ||
                !writeOperand(operand2,isByteInstr)) {
                cout<<"error - operands\n";
                return;
            }
            break;
        case SUB:
            if (am1<=0 || am2<0) {cout<<"Error - operands in instruction\n"; return;}
            oc=6<<3;
            if (isByteInstr) oc=oc&bmask;
            else oc=oc|wmask;
            currSecData->addByte(oc);
            lc++;
            if (!writeOperand(operand1,isByteInstr) ||
                !writeOperand(operand2,isByteInstr)) {
                cout<<"error - operands\n";
                return;
            }
            break;
        case MUL:
            if (am1<=0 || am2<0) {cout<<"Error - operands in instruction\n"; return;}
            oc=7<<3;
            if (isByteInstr) oc=oc&bmask;
            else oc=oc|wmask;
            currSecData->addByte(oc);
            lc++;
            if (!writeOperand(operand1,isByteInstr) ||
                !writeOperand(operand2,isByteInstr)) {
                cout<<"error - operands\n";
                return;
            }
            break;
        case DIV:
            if (am1<=0 || am2<0) {cout<<"Error - operands in instruction\n"; return;}
            oc=8<<3;
            if (isByteInstr) oc=oc&bmask;
            else oc=oc|wmask;
            currSecData->addByte(oc);
            lc++;
            if (!writeOperand(operand1,isByteInstr) ||
                !writeOperand(operand2,isByteInstr)) {
                cout<<"error - operands\n";
                return;
            }
            break;
        case CMP:
            if (am1<0 || am2<0) {cout<<"Error - operands in instruction\n"; return;}
            oc=9<<3;
            if (isByteInstr) oc=oc&bmask;
            else oc=oc|wmask;
            currSecData->addByte(oc);
            lc++;
            if (!writeOperand(operand1,isByteInstr) ||
                !writeOperand(operand2,isByteInstr)) {
                cout<<"error - operands\n";
                return;
            }
            break;
        case NOT:
            if (am1<=0) {cout<<"Error - operands in instruction\n"; return;}
            if (operand2!="") {cout<<"error - too many operands for int\n"; return;}
            oc=10<<3;
            if (isByteInstr) oc=oc&bmask;
            else oc=oc|wmask;
            currSecData->addByte(oc);
            lc++;
            if (!writeOperand(operand1,isByteInstr)) {
                cout<<"error - operands\n";
                return;
            }
            break;
        case AND:
            if (am1<=0 || am2<0) {cout<<"Error - operands in instruction\n"; return;}
            oc=11<<3;
            if (isByteInstr) oc=oc&bmask;
            else oc=oc|wmask;
            currSecData->addByte(oc);
            lc++;
            if (!writeOperand(operand1,isByteInstr) ||
                !writeOperand(operand2,isByteInstr)) {
                cout<<"error - operands\n";
                return;
            }
            break;
        case OR:
            if (am1<=0 || am2<0) {cout<<"Error - operands in instruction\n"; return;}
            oc=12<<3;
            if (isByteInstr) oc=oc&bmask;
            else oc=oc|wmask;
            currSecData->addByte(oc);
            lc++;
            if (!writeOperand(operand1,isByteInstr) ||
                !writeOperand(operand2,isByteInstr)) {
                cout<<"error - operands\n";
                return;
            }
            break;
        case XOR:
            if (am1<=0 || am2<0) {cout<<"Error - operands in instruction\n"; return;}
            oc=13<<3;
            if (isByteInstr) oc=oc&bmask;
            else oc=oc|wmask;
            currSecData->addByte(oc);
            lc++;
            if (!writeOperand(operand1,isByteInstr) ||
                !writeOperand(operand2,isByteInstr)) {
                cout<<"error - operands\n";
                return;
            }
            break;
        case TEST:
            if (am1<0 || am2<0) {cout<<"Error - operands in instruction\n"; return;}
            oc=14<<3;
            if (isByteInstr) oc=oc&bmask;
            else oc=oc|wmask;
            currSecData->addByte(oc);
            lc++;
            if (!writeOperand(operand1,isByteInstr) ||
                !writeOperand(operand2,isByteInstr)) {
                cout<<"error - operands\n";
                return;
            }
            break;
        case SHL:
            if (am1<=0 || am2<0) {cout<<"Error - operands in instruction\n"; return;}
            oc=15<<3;
            if (isByteInstr) oc=oc&bmask;
            else oc=oc|wmask;
            currSecData->addByte(oc);
            lc++;
            if (!writeOperand(operand1,isByteInstr) ||
                !writeOperand(operand2,isByteInstr)) {
                cout<<"error - operands\n";
                return;
            }
            break;
        case SHR:
            if (am1<=0 || am2<0) {cout<<"Error - operands in instruction\n"; return;}
            oc=16<<3;
            if (isByteInstr) oc=oc&bmask;
            else oc=oc|wmask;
            currSecData->addByte(oc);
            lc++;
            if (!writeOperand(operand1,isByteInstr) ||
                !writeOperand(operand2,isByteInstr)) {
                cout<<"error - operands\n";
                return;
            }
            break;
        case PUSH:
            if (am1<0 || operand2!="") {cout<<"Error - operands in instruction\n"; return;}
            oc=17<<3;
            if (isByteInstr) oc=oc&bmask;
            else oc=oc|wmask;
            currSecData->addByte(oc);
            lc++;
            if (!writeOperand(operand1,isByteInstr)) {
                cout<<"error - operands\n";
                return;
            }
            break;
        case POP:
            if (am1<=0 || operand2!="") {cout<<"Error - operands in instruction\n"; return;}
            oc=18<<3;
            if (isByteInstr) oc=oc&bmask;
            else oc=oc|wmask;
            currSecData->addByte(oc);
            lc++;
            if (!writeOperand(operand1,isByteInstr)) {
                cout<<"error - operands\n";
                return;
            }
            break;
            case JMP:case JEQ:case JNE: case JGT: case CALL:
            if (am1<0 || operand2!="") {cout<<"Error - operands in instruction\n"; return;}
            if (instruction=="jmp") oc=19<<3;
            else if (instruction=="jeq") oc=20<<3;
            else if (instruction=="jne") oc==21<<3;
            else if (instruction=="jgt") oc==22<<3;
            else oc==23<<3;
            currSecData->addByte(oc);
            lc++;
            if (!writeOperand(operand1,isByteInstr)) {
                cout<<"error - operands\n";
                return;
            }
            break;
        case RET:
            oc=24<<3;
            currSecData->addByte(oc);
            lc+=1;
            break;
        case IRET:
            oc=25<<3;
            currSecData->addByte(oc);
            lc+=1;
            break;
    }


}

void Assembler::printSymbolTable() {
    list<Symbol *>::iterator it = symbolList.begin();
    for (int i = 0; i < symbolList.size(); i++) {
        Symbol *sym = *it;
        cout << *sym;
        advance(it, 1);
    }
}

void Assembler::printSections() {
    list<RelocationTable *>::iterator iter = relTableList.begin();
    for (int i = 0; i < relTableList.size(); i++) {
        RelocationTable *tab = *iter;
        cout << *tab;
        advance(iter, 1);
    }

    list<SectionData *>::iterator ite = sectionList.begin();
    for (int i = 0;i < sectionList.size();i++) {
        SectionData *data = *ite;
        cout << *data;
        advance(ite,1);
    }
}

string Assembler::removeAllSpaces(string in){
    int index;
    string ret=in;
    while ((index=ret.find_first_of(" \t"))!=string::npos){
        ret=ret.substr(0,index).append(ret.substr(index+1,string::npos));
    }
    return ret;
}

void Assembler::checkIfInstructionFP(string line) {
    int startIndex,index;
    if ((startIndex= line.find_first_not_of(" \t"))!=string::npos)
        line = line.substr(startIndex);
    if (line.empty()) return;
    if (line[0]=='.') return;
    //line starts with instruction
    //parsing operands
    string instruction,operands;
    if ((startIndex = line.find_first_of(" ")) != string::npos) {
        instruction = line.substr(0, startIndex);
        operands = line.substr(startIndex + 1, string::npos);
        operands=removeAllSpaces(operands);
    } else {
        instruction = line;
        if (startIndex = instruction.find_first_of("\r") != string::npos) {
            instruction = instruction.substr(0, startIndex);
        }
    }

    //line equals operands , parsed instruction
    string operand1="";
    string operand2="";
    if (!operands.empty()) {
        if ((index = operands.find_first_of(',')) != string::npos) {
            operand1 = operands.substr(0, index);
            operand2 = operands.substr(index + 1, string::npos);
        }else {
            operand1=operands;
        }
    }
    if (!instructionAllowed()) {cout<<"error - instruction outside section\n";return;}
    int am1,am2;
    am1=addressingMode(operand1);
    am2=addressingMode(operand2);
    lc+=3;
    lc+=numOfBytesPerOperand(instruction,am1);
    lc+=numOfBytesPerOperand(instruction,am2);

}

bool Assembler::instructionAllowed(){
    Symbol* temp;
    list<Symbol*>::iterator iter=symbolList.begin();
    advance(iter,sectionIndex);
    temp=*iter;
    if (temp->getRwx().find_first_of('x')!=string::npos) return true;
    return false;
}

int Assembler::addressingMode(string operand) {
    regex reg;smatch match;
    //pc relative with symbol ($)
    reg=("^\\$[a-zA-Z][0-9a-zA-Z]*$");
    if (regex_search(operand, match, reg)) {
        return 4;
    }

    //immed
    reg = ("^-?[0-9]+$");//decimal
    if (regex_search(operand, match, reg)) {
        string temp=match[0];
        int immed=atoi(temp.c_str());

        if (immed>=-32768 && immed<32768) return 0;
        else return -1;
    }
    reg = ("^0[xX][0-9a-fA-F]+$");//hex
    if (regex_search(operand,match,reg)) {
        string temp=match[0];
        int immed=stoi(temp,0,16);
        if (immed>=0 && immed<=65535) return 0;
        else return -1;
    }
    reg= ("^&(\\w+)$");//symbol
    if (regex_search(operand,match,reg)) return 0;

    //regdir
    reg = ("^(r[0-7]|pc|sp)(l|h)?$");
    if (regex_search(operand,match,reg)) {
        return 1;
    }

    //regind
    reg = ("^\\[(r[0-7]|pc|sp)\\]$");
    if (regex_search(operand,match,reg)) return 2;

    //regind8 or regind16
    reg = ("^(r[0-7]|pc|sp)\\[(-?[0-9]+)\\]$");
    if (regex_search(operand,match,reg)) {
        string temp=match[2];
        int offset=atoi(temp.c_str());
        if (offset>=-128 &&  offset<=127) return 3;
        else if (offset>=-32768 && offset<32768) return 4;
    }

    //regind with symbol
    reg = ("^(r[0-7]|pc|sp)\\[[a-zA-Z][0-9a-zA-Z]*\\]$");
    if (regex_search(operand,match,reg)) return 4;

    //memdir
    reg=("^([a-zA-Z][0-9a-zA-Z]*|\\*([0-9]*))$");
    if (regex_search(operand,match,reg)) {
        if (match[2]!=""){
            string temp=match[2];
            int offset=atoi(temp.c_str());
            if (offset>=0 &&  offset<65535) return 5;
        }else return 5;
    }
    return -1;
}

int Assembler::numOfBytesPerOperand(string instruction, int addressingMode) {
    switch (addressingMode){
    case -1:return -1;
    case 1: case 2: return 0;
    case 3: return 1;
    case 4: case 5: return 2;
    case 0:
        if (instruction[instruction.size()-1]=='b' && instruction!="sub") return 1;
        else return 2;
    default: return -1;
    }
}

bool Assembler::isByteInstruction(string instruction) {
    if (instruction[instruction.size()-1]=='b' && instruction!="sub") return true;
    return false;
}

bool Assembler::writeOperand(string operand, bool byteInstr) {
    regex reg;smatch match;

    //pc relative with symbol ($) - convert to pc[symbol]
    reg=("^\\$[a-zA-Z][0-9a-zA-Z]*$");
    if (regex_search(operand, match, reg)) {
        int regNum=7;
        int oc=regNum<<1;
        oc=oc|(4<<5);
        currSecData->addByte(oc);
        lc++;
        string symName=match[0];
        symName=symName.substr(1);
        list<Symbol *>::iterator iter = symbolList.begin();
        Symbol *temp;
        for (int i = 0; i < symbolList.size(); i++) {
            temp = *iter;
            if (temp->getName() == symName && temp->getType() == 1) {
                if (temp->isGlobal()){
                    currSecData->addWord(-2);
                    currRelTable->addRecord(new RelocationRecord(lc,1,temp->getIndex()));
                    lc+=2;
                }else{
                    int val=-2;
                    val+=temp->getValue();
                    currSecData->addWord(val);
                    currRelTable->addRecord(new RelocationRecord(lc,1,temp->getSectionIndex()));
                    lc+=2;
                }
                return true;
            }
            advance(iter, 1);
        }
        return false;
    }

    //immed
    reg = ("^-?[0-9]+$");//decimal
    if (regex_search(operand, match, reg)) {
        string temp=match[0];
        int immed=atoi(temp.c_str());
        currSecData->addByte(0);
        lc++;
        if (byteInstr) {
            if (immed<=-128 || immed>=127) return false;
            currSecData->addByte(immed);
            lc++;
        }
        else {
            currSecData->addWord(immed);
            lc+=2;
        }
        return true;
    }
    reg = ("^0[xX][0-9a-fA-F]+$");//hex
    if (regex_search(operand,match,reg)) {
        string temp=match[0];
        int immed=stoi(temp,0,16);
        currSecData->addByte(0);
        lc++;
        if (byteInstr) {
            if (immed<0 || immed>15) return false;
            currSecData->addByte(immed);
            lc++;
        }
        else {
            currSecData->addWord(immed);
            lc+=2;
        }
        return true;
    }

    reg= ("^&(\\w+)$");//symbol
    if (regex_search(operand,match,reg)) {
        string symName=match[1];
        currSecData->addByte(0);
        lc++;
        list<Symbol *>::iterator iter = symbolList.begin();
        Symbol *temp;
        for (int i = 0; i < symbolList.size(); i++) {
            temp = *iter;
            if (temp->getName() == symName && temp->getType() == 1) {
                if (temp->isGlobal()){
                    currSecData->addWord(0);
                    currRelTable->addRecord(new RelocationRecord(lc,0,temp->getIndex()));
                    lc+=2;
                }else{
                    currSecData->addWord(temp->getValue());
                    currRelTable->addRecord(new RelocationRecord(lc,0,temp->getSectionIndex()));
                    lc+=2;
                }
                return true;
            }
            advance(iter, 1);
        }
        return false;
    }

    //regdir
    reg = ("^(r[0-7]|pc|sp)(l|h)?$");
    if (regex_search(operand,match,reg)) {
        string temp=match[0];
        int regNum;
        if (temp.substr(0,2)=="pc") regNum=7;
        else if (temp.substr(0,2)=="sp") regNum=6;
        else regNum=temp[1]-'0';
        int oc=1<<5;
        oc=oc|(regNum<<1);
        if (temp.size()==3){
            if (!byteInstr){return false;}
            if (temp[2]=='h') oc=oc|1;
        }
        currSecData->addByte(oc);
        lc++;
        return true;
    }


    //regind
    reg = ("^\\[(r[0-7]|pc|sp)\\]$");
    if (regex_search(operand,match,reg)) {
            string temp=match[1];
            int regNum;
            if (temp=="pc") regNum=7;
            else if (temp=="sp") regNum=6;
            else regNum=temp[1]-'0';
            int oc=2<<5;
            oc=oc|(regNum<<1);
            currSecData->addByte(oc);
            lc++;
            return true;
    }

    //regind8 or regind16
    reg = ("^(r[0-7]|pc|sp)\\[(-?[0-9]+)\\]$");
    if (regex_search(operand,match,reg)) {
        string reg=match[1];
        int regNum;
        if (reg=="pc") regNum=7;
        else if (reg=="sp") regNum=6;
        else regNum=reg[1]-'0';
        int oc=regNum<<1;
        string temp=match[2];
        int offset=atoi(temp.c_str());
        if (offset>=-128 &&  offset<=127) {//regind8
            oc=oc|(3<<5);
            currSecData->addByte(oc);
            lc++;
            currSecData->addByte(offset);
            lc++;
            return true;
        }
        else if (offset>=-32768 && offset<32768) {//regind16
            oc=oc|(4<<5);
            currSecData->addByte(oc);
            lc++;
            currSecData->addWord(offset);
            lc++;
            return true;
        }
        return false;
    }

    //regind with symbol
    reg = ("^(r[0-7]|pc|sp)\\[([a-zA-Z][0-9a-zA-Z]*)\\]$");
    if (regex_search(operand,match,reg)) {
        string reg=match[1];
        int regNum;
        if (reg=="pc") regNum=7;
        else if (reg=="sp") regNum=6;
        else regNum=reg[1]-'0';
        int oc=regNum<<1;
        oc=oc|(4<<5);
        currSecData->addByte(oc);
        lc++;
        string symName=match[2];
        list<Symbol *>::iterator iter = symbolList.begin();
        Symbol *temp;
        for (int i = 0; i < symbolList.size(); i++) {
            temp = *iter;
            if (temp->getName() == symName && temp->getType() == 1) {
                if (temp->isGlobal()){
                    currSecData->addWord(0);
                    currRelTable->addRecord(new RelocationRecord(lc,0,temp->getIndex()));
                    lc+=2;
                }else{
                    currSecData->addWord(temp->getValue());
                    currRelTable->addRecord(new RelocationRecord(lc,0,temp->getSectionIndex()));
                    lc+=2;
                }
                return true;
            }
            advance(iter, 1);
        }
        return false;
    }

    //memdir
    reg=("^([a-zA-Z][0-9a-zA-Z]*|\\*([0-9]*))$");
    if (regex_search(operand,match,reg)) {
        int oc=5<<5;
        currSecData->addByte(oc);
        lc++;
        if (match[2]!=""){
            string temp=match[2];
            int offset=atoi(temp.c_str());
            currSecData->addWord(offset);
            lc+=2;
            return true;
        }else {
            string symName=match[1];
            list<Symbol *>::iterator iter = symbolList.begin();
            Symbol *temp;
            for (int i = 0; i < symbolList.size(); i++) {
                temp = *iter;
                if (temp->getName() == symName && temp->getType() == 1) {
                    if (temp->isGlobal()){
                        currSecData->addWord(0);
                        currRelTable->addRecord(new RelocationRecord(lc,0,temp->getIndex()));
                        lc+=2;
                    }else{
                        currSecData->addWord(temp->getValue());
                        currRelTable->addRecord(new RelocationRecord(lc,0,temp->getSectionIndex()));
                        lc+=2;
                    }
                    return true;
                }
                advance(iter, 1);
            }
            return true;
        }
    }

    return false;
}

Symbol* Assembler::findSymbol(string name) {
    list<Symbol *>::iterator iter = symbolList.begin();
    Symbol *temp;
    for (int i = 0; i < symbolList.size(); i++) {
        temp = *iter;
        if (temp->getName() == name) {
            return temp;
        }
        advance(iter, 1);
    }
    return 0;
}

/*
int Assembler::findCurrSectionIndex() {
    if (currSecData==0) return -1;
    string name=currSecData->getName();
    list<Symbol*>::iterator it=symbolList.begin();
    Symbol *temp;
    for (int i=0; i<symbolList.size();i++){
        temp=*it;
        if (temp->getName()==name && temp->getType()==0){
            return temp->getIndex();
        }
        advance(it,1);
    }
    return -1;
}
 */
