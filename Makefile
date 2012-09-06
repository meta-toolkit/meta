SEARCH = search
SEARCHOBJS = tokenizers/parse_tree.o index/document.o index/ram_index.o tokenizers/pos_tree_tokenizer.o \
    index/index.o tokenizers/level_tree_tokenizer.o tokenizers/ngram_tokenizer.o io/textfile.o io/parser.o \
    index/lexicon.o index/inverted_index.o io/compressed_file_reader.o io/compressed_file_writer.o \
    index/postings.o tokenizers/tokenizer.o index/chunk_list.o index/structs.o stemmers/porter2_stemmer.o

TESTER = tester
TESTEROBJS = $(SEARCHOBJS)

TEMPLATES = util/invertible_map.h util/invertible_map.cpp util/common.h util/common.cpp

CC = g++ -std=c++0x -fopenmp -I.
#CCOPTS = -g -O0
CCOPTS = -O3
LINKER = g++ -std=c++0x -fopenmp -I.

CLEANDIRS = tokenizers io index util stemmers

all: $(SEARCH) $(TESTER) $(SANDERS) $(GENERIC)

$(SEARCH): $(SEARCHOBJS) main.cpp $(TEMPLATES)
	$(LINKER) main.cpp -o $@ $(SEARCHOBJS)

$(TESTER): $(TESTEROBJS) tester.cpp $(TEMPLATES)
	$(LINKER) tester.cpp -o $@ $(TESTEROBJS)

%.o : %.cpp $(wildcard *.h)
	$(CC) $(CCOPTS) -c $(@:.o=.cpp) -o $@

clean:
	for dir in $(CLEANDIRS) ; do rm -rf $$dir/*.o ; done
	rm -f $(SEARCH) $(TESTER)

tidy:
	rm -rf ./doc *.chunk postingsFile lexiconFile termid.mapping docid.mapping docs.lengths *compressed.txt
