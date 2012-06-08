SEARCH = search
SEARCHOBJS = parse_tree.o document.o ram_index.o pos_tree_tokenizer.o \
    level_tree_tokenizer.o ngram_tokenizer.o textfile.o parser.o \
    lexicon.o inverted_index.o compressed_file_reader.o compressed_file_writer.o \
    libstemmer/libstemmer.o

COMPRESSION_TEST = compress
COMPRESS_OBJS = compressed_file_reader.o compressed_file_writer.o

CC = g++ -std=c++0x -fopenmp
CCOPTS = -g -O0
#CCOPTS = -O3
LINKER = g++ -std=c++0x -fopenmp

all: $(SEARCH) $(COMPRESSION_TEST)

$(SEARCH): $(SEARCHOBJS) main.cpp
	$(LINKER) main.cpp -o $(SEARCH) $(SEARCHOBJS)

$(COMPRESSION_TEST): $(COMPRESS_OBJS) compress.cpp
	$(LINKER) compress.cpp -o $(COMPRESSION_TEST) $(COMPRESS_OBJS)

%.o : %.cpp $(wildcard *.h)
	$(CC) $(CCOPTS) -c $(@:.o=.cpp) -o $@

clean:
	-rm -rf *.o $(SEARCH) $(COMPRESSION_TEST)

tidy: clean
	-rm -rf ./doc
