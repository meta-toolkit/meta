SEARCH = search
SEARCHOBJS = parse_tree.o document.o search.o index.o util.o

CC = g++ -std=c++0x
CCOPTS = -g -O0
LINKER = g++ -std=c++0x

all: $(SEARCH)

$(SEARCH): $(SEARCHOBJS) main.cpp
	$(LINKER) main.cpp -o $(SEARCH) $(SEARCHOBJS)

%.o : %.cpp $(wildcard *.h)
	$(CC) $(CCOPTS) -c $(@:.o=.cpp) -o $@

clean:
	-rm -rf *.o $(SEARCH)
