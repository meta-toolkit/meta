EXE = authormatch
EXEOBJS = parse_tree.o

CC = g++ -std=c++0x
CCOPTS = -O3 -c
LINKER = g++ -std=c++0x

all: $(EXE)

$(EXE): $(EXEOBJS)
	$(LINKER) main.cpp -o $(EXE) $(EXEOBJS)

parse_tree.o: parse_tree.h parse_tree.cpp
	$(CC) $(CCOPTS) parse_tree.cpp

clean:
	-rm -rf *.o $(EXE)
