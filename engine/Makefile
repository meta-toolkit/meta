INDEX = index
INDEXOBJS = parse_tree.o document.o

SEARCH = search
SEARCHOBJS = parse_tree.o document.o

CC = g++ -std=c++0x
CCOPTS = -g -O0 -c
LINKER = g++ -std=c++0x

all: $(INDEX) $(SEARCH)

$(INDEX): $(INDEXOBJS) $(INDEX).cpp
	$(LINKER) $(INDEX).cpp -o $(INDEX) $(INDEXOBJS)

$(SEARCH): $(SEARCHOBJS) $(SEARCH).cpp
	$(LINKER) $(SEARCH).cpp -o $(SEARCH) $(SEARCHOBJS)

document.o: document.h document.cpp
	$(CC) $(CCOPTS) document.cpp

parse_tree.o: parse_tree.h parse_tree.cpp
	$(CC) $(CCOPTS) parse_tree.cpp

clean:
	-rm -rf *.o $(INDEX) $(SEARCH)
