EXE = authormatch
EXEOBJS = parse_tree.o document.o

CC = g++ -std=c++0x
CCOPTS = -g -O0 -c
LINKER = g++ -std=c++0x

all: $(EXE)

$(EXE): $(EXEOBJS) main.cpp
	$(LINKER) main.cpp -o $(EXE) $(EXEOBJS)

document.o: document.h document.cpp
	$(CC) $(CCOPTS) document.cpp

parse_tree.o: parse_tree.h parse_tree.cpp
	$(CC) $(CCOPTS) parse_tree.cpp

clean:
	-rm -rf *.o $(EXE)
