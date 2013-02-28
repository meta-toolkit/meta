SEARCH = search
SEARCHOBJS = tokenizers/parse_tree.o index/document.o index/ram_index.o \
    tokenizers/tree_tokenizer.o tokenizers/ngram_tokenizer.o io/textfile.o io/parser.o \
    index/lexicon.o index/inverted_index.o io/compressed_file_reader.o io/compressed_file_writer.o \
    index/postings.o tokenizers/tokenizer.o index/chunk_list.o index/structs.o stemmers/porter2_stemmer.o \
    classify/knn.o io/config_reader.o classify/confusion_matrix.o

TESTER = tester
TESTEROBJS = $(SEARCHOBJS)

PLOT = features 
PLOTOBJS = $(SEARCHOBJS)

LEARN = learn
LEARNOBJS = tokenizers/parse_tree.o tokenizers/tree_tokenizer.o \
    tokenizers/ngram_tokenizer.o io/textfile.o io/parser.o tokenizers/tokenizer.o \
    index/chunk_list.o index/structs.o stemmers/porter2_stemmer.o \
    io/config_reader.o index/document.o

LM = lm
LMOBJS = tokenizers/ngram_tokenizer.o index/document.o io/textfile.o io/parser.o \
		 tokenizers/tokenizer.o stemmers/porter2_stemmer.o

TESTS = test/porter2_stemmer_test.h test/parse_tree_test.h \
    test/compressed_file_test.h test/unit_test.h

TEMPLATES = util/invertible_map.h util/invertible_map.cpp util/common.h util/common.cpp

CC = g++ -std=c++0x -fopenmp -I.
#CCOPTS = -g -O0
CCOPTS = -O3
LINKER = g++ -std=c++0x -fopenmp -I.

LIBDIRS = lib/liblinear-1.92
CLEANDIRS = tokenizers io index util stemmers classify test model lib/liblinear-1.92

all: $(SEARCH) $(TESTER) $(PLOT) $(LEARN) $(LM)
	for dir in $(LIBDIRS) ; do make -C $$dir ; done

$(LM): $(LMOBJS) lm.cpp model/ngram_distribution.h model/ngram_distribution.cpp
	$(LINKER) lm.cpp -o $@ $(LMOBJS)

$(SEARCH): $(SEARCHOBJS) main.cpp $(TEMPLATES)
	$(LINKER) main.cpp -o $@ $(SEARCHOBJS)

$(TESTER): $(TESTEROBJS) test/tester.cpp $(TEMPLATES) $(TESTS)
	$(LINKER) test/tester.cpp -o $@ $(TESTEROBJS)

$(PLOT): $(PLOTOBJS) features.cpp $(TEMPLATES)
	$(LINKER) features.cpp -o $@ $(PLOTOBJS)

$(LEARN): $(LEARNOBJS) learn.cpp $(TEMPLATES)
	$(LINKER) learn.cpp -o $@ $(LEARNOBJS)

%.o : %.cpp $(wildcard *.h)
	$(CC) $(CCOPTS) -c $(@:.o=.cpp) -o $@

clean:
	for dir in $(CLEANDIRS) ; do rm -rf $$dir/*.o ; done
	for dir in $(LIBDIRS) ; do make -C $$dir clean ; done
	rm -f preprocessor/*.class evalConfig.ini evalOutput
	rm -f $(SEARCH) $(TESTER) $(PLOT) $(LEARN) $(LM) *.o

tidy:
	rm -rf ./doc *.chunk postingsFile lexiconFile termid.mapping docid.mapping docs.lengths *compressed.txt
