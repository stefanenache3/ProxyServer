.PHONY: clean

SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
FLAGS=-ggdb3 -Wall -pthread
GPP=g++



all: main

main: $(OBJECTS)
	
	$(GPP) $(OBJECTS) $(FLAGS) -o main
    
%.o: %.cpp
	$(GPP) -c -o $@ $<    

clean:
	rm -f $(OBJECTS) main   

