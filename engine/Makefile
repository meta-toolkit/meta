SEARCH = search
SEARCHOBJS = parse_tree.o document.o ram_index.o pos_tree_tokenizer.o \
    level_tree_tokenizer.o ngram_tokenizer.o textfile.o parser.o \
    lexicon.o inverted_index.o compressed_file.o libstemmer/libstemmer.o

CC = g++ -std=c++0x -fopenmp
CCOPTS = -g -O0
LINKER = g++ -std=c++0x -fopenmp

all: $(SEARCH)

$(SEARCH): $(SEARCHOBJS) main.cpp
	$(LINKER) main.cpp -o $(SEARCH) $(SEARCHOBJS)

%.o : %.cpp $(wildcard *.h)
	$(CC) $(CCOPTS) -c $(@:.o=.cpp) -o $@

clean:
	-rm -rf *.o $(SEARCH)

tidy: clean
	-rm -rf ./doc
