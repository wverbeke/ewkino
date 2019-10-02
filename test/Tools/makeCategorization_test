CC=g++ -Wall -Wextra
CFLAGS= -Wl,--no-as-needed
LDFLAGS=`root-config --glibs --cflags`
SOURCES= Categorization_test.cc  ../../Tools/src/IndexFlattener.cc ../../Tools/src/Categorization.cc ../../Tools/src/stringTools.cc
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=Categorization_test

all: 
	$(CC) $(CFLAGS) $(SOURCES) $(LDFLAGS) -o $(EXECUTABLE)
	
clean:
	rm -rf *o $(EXECUTABLE)
