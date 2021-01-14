LINKER = g++
CC = g++
LIBS = -lstdc++fs -lboost_program_options

CFLAGS = -O2 -fPIC -std=c++17
LDFLAGS = -Wl,-s

SOURCES = $(wildcard ./*.cpp) 
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = tensor_test

all: clean $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(LINKER) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

.cpp.o:
	$(CC) $(CFLAGS) $< -c -o $@

clean:
	rm -f *.o

