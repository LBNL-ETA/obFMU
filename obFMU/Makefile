# The C++ program
CC=gcc
CPP=g++

CPPFLAGS=-Wno-error -std=c++11 -fPIC -D_GLIBCXX_USE_CXX11_ABI=0
CCFLAGS=-Wno-error -fPIC -D_GLIBCXX_USE_CXX11_ABI=0
CBITSFLAGS=-fvisibility=hidden

# This is for co-simulation
INCLUDES=-DFMI_COSIMULATION -I.
SOURCES_CPP=$(wildcard *.cpp)
SOURCES_C=$(wildcard *.c)
SOURCES_CC=$(wildcard *.cc)

OBJECTS=$(SOURCES_CPP:.cpp=.o)
OBJECTS+=$(SOURCES_C:.c=.o)
OBJECTS+=$(SOURCES_CC:.cc=.o)
LIBRARY=obFMU.so

all: $(OBJECTS) $(LIBRARY)

$(LIBRARY): $(OBJECTS)
	$(CC) -g $(OBJECTS) $(CBITSFLAGS) $(INCLUDES) -shared -o $@

%.o: %.cpp
	$(CPP) -g $(CPPFLAGS) $(CBITSFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.c
	$(CC) -g $(CCFLAGS) $(CBITSFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.cc
	$(CC) -g $(CCFLAGS) $(CBITSFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(LIBRARY)

