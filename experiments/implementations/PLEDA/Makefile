CC = g++ -O2 -Wno-deprecated -ftree-vectorize

all: PlackettLuceEDA

PlackettLuceEDA: main.o LOP.o Tools.o Individual.o Population.o PlackettLuce.o PLEDA.o random.o SFMT.o
	$(CC) main.o LOP.o Tools.o Individual.o Population.o PlackettLuce.o PLEDA.o random.o SFMT.o -lstdc++ -lm -o PlackettLuceEDA

main.o: main.cpp PLEDA.hpp Individual.h ParameterSetting.h LOP.h random.h
	$(CC) -c main.cpp

LOP.o: LOP.cpp LOP.h Tools.h
	$(CC) -c LOP.cpp

Tools.o: Tools.cpp Tools.h Variables.h
	$(CC) -c Tools.cpp

Individual.o: Individual.cc Individual.h Tools.h Variables.h
	$(CC) -c Individual.cc

Population.o: Population.cc Population.h Individual.h
	$(CC) -c Population.cc

PlackettLuce.o: PlackettLuce.cpp PlackettLuce.h Population.h LOP.h Tools.h
	$(CC) -c PlackettLuce.cpp

PLEDA.o: PLEDA.cpp PLEDA.hpp LOP.h Tools.h Population.h PlackettLuce.h
	$(CC) -c PLEDA.cpp

random.o: random.cpp random.h SFMT.h
	$(CC) -c random.cpp

SFMT.o: SFMT.c SFMT.h SFMT-params.h SFMT-common.h SFMT-params19937.h
	$(CC) -c SFMT.c

clean:
	rm -f core *.o PlackettLuceEDA

