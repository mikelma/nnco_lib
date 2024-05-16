CPP = g++
CC = gcc
CPPFLAGS = -O2 -Wall
CCFLAGS = -Wall
INCLUDES = -I"/usr/local" -Ieigen-3.4.0/
LFLAGS = -L"/usr/lib"
LIBS = -lm

all: main

clean:
	rm *.o gradientSearch

zip:
	zip -r gradientSearch.zip *.cpp *.c *.h Makefile instances/*

SFMT.o: SFMT.c SFMT.h SFMT-common.h SFMT-params.h SFMT-params19937.h
	$(CC) -O3 -finline-functions -fomit-frame-pointer -DNDEBUG -fno-strict-aliasing --param max-inline-insns-single=1800 -Wmissing-prototypes -Wall -std=c99 -DSFMT_MEXP=19937 -c SFMT.c

random.o: random.cpp random.h
	$(CPP) $(CPPFLAGS) $(INCLUDES) -c random.cpp $(LFLAGS) $(LIBS)

timer.o: timer.cpp timer.h
	$(CPP) $(CPPFLAGS) $(INCLUDES) -c timer.cpp $(LFLAGS) $(LIBS)

utils.o: utils.cpp utils.h
	$(CPP) $(CPPFLAGS) $(INCLUDES) -c utils.cpp $(LFLAGS) $(LIBS)

PlackettLuce.o: PlackettLuce.cpp PlackettLuce.h
	$(CPP) $(CPPFLAGS) $(INCLUDES) -c PlackettLuce.cpp $(LFLAGS) $(LIBS)

BradleyTerry.o: BradleyTerry.cpp BradleyTerry.h
	$(CPP) $(CPPFLAGS) $(INCLUDES) -c BradleyTerry.cpp $(LFLAGS) $(LIBS)

LopInstance.o: LopInstance.cpp LopInstance.h
	$(CPP) $(CPPFLAGS) $(INCLUDES) -c LopInstance.cpp $(LFLAGS) $(LIBS)

utility_functions.o: utility_functions.cpp utility_functions.h
	$(CPP) $(CPPFLAGS) $(INCLUDES) -c utility_functions.cpp $(LFLAGS) $(LIBS)

algorithms.o: algorithms.cpp algorithms.h
	$(CPP) $(CPPFLAGS) $(INCLUDES) -c algorithms.cpp $(LFLAGS) $(LIBS)

OBJECTS = SFMT.o random.o timer.o utils.o PlackettLuce.o LopInstance.o utility_functions.o algorithms.o BradleyTerry.o

main: main.cpp $(OBJECTS)
	$(CPP) $(CPPFLAGS) $(INCLUDES) main.cpp $(OBJECTS) $(LFLAGS) $(LIBS) -o gradientSearch
