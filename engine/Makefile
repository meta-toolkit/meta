SEARCH = search
SEARCHOBJS = parse_tree.o document.o search.o index.o util.o

CC = g++ -std=c++0x
CCOPTS = -g -O0 -c
LINKER = g++ -std=c++0x

all: $(SEARCH)

$(SEARCH): $(SEARCHOBJS) main.cpp
	$(LINKER) main.cpp -o $(SEARCH) $(SEARCHOBJS)

search.o: engine.h search.cpp
	$(CC) $(CCOPTS) search.cpp

index.o: engine.h index.cpp
	$(CC) $(CCOPTS) index.cpp

util.o: engine.h util.cpp
	$(CC) $(CCOPTS) util.cpp

document.o: document.h document.cpp
	$(CC) $(CCOPTS) document.cpp

parse_tree.o: parse_tree.h parse_tree.cpp
	$(CC) $(CCOPTS) parse_tree.cpp

clean:
	-rm -rf *.o $(SEARCH)
