OBJS = assembler.cpp main.cpp parser.cpp symbol.cpp relrecord.cpp reltable.cpp sectiondata.cpp

prog: $(OBJS)
	g++ -std=c++11 -gdwarf-2 $(OBJS) -o asm


clean: 
	rm *^(\.cpp$|\.h$) asm
