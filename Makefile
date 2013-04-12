SEARCH = search
LEARN = learn
LM = lm
CLASSIFY = classify-test
SLDATEST = feature-select
CLUSTERTEST = cluster-test
THREADPOOLTEST = threadpool-test
PARALLELFORTEST = parallel-for-test
LDATEST = lda-test
LDATOPICS = lda-topics

SRC = $(wildcard src/io/*.cpp) $(wildcard src/model/*.cpp) $(wildcard src/cluster/*.cpp) \
	  $(wildcard src/classify/*.cpp) $(wildcard src/util/*.cpp) $(wildcard src/tokenizers/*.cpp) \
	  $(wildcard src/index/*.cpp) $(wildcard src/stemmers/*.cpp) $(wildcard lib/slda/*.cpp) \
	  $(wildcard lib/liblinear/*.cpp) $(wildcard src/topics/*.cpp)

TEMPLATES = $(wildcard include/io/*.tcc) $(wildcard include/model/*.tcc) $(wildcard include/cluster/*.tcc) \
	  $(wildcard include/classify/*.tcc) $(wildcard include/util/*.tcc) $(wildcard include/tokenizers/*.tcc) \
	  $(wildcard include/index/*.tcc) $(wildcard include/stemmers/*.tcc) $(wildcard lib/slda/*.tcc) \
	  $(wildcard lib/liblinear/*.tcc) $(wildcard include/topics/*.tcc)

TEMPLATE_HEADERS := $(TEMPLATES:.tcc=.h)
TEMPLATE_HEADERS := $(shell echo $(TEMPLATE_HEADERS) | sed -e "s/src/include/g")

# manually add template headers here that do not have a corresponding .tcc file
TEMPLATE_HEADERS += include/cluster/agglomerative_clustering.h \
					include/cluster/basic_single_link_policy.h \
					include/cluster/point.h \
					include/parallel/thread_pool.h include/parallel/parallel_for.h

OBJ := $(SRC:.cpp=.o)
OBJ := $(shell echo $(OBJ) | sed -e "s/src/obj/g" | sed -e "s/lib/obj/g")

# libraries compiled *separately*
LIBDIRS = lib/liblinear-1.92

SLDALIBS = -lgsl -lm -lgslcblas
#CXXRTLIBS = 
CXXRTLIBS = -lcxxrt -ldl
THREADLIBS = -pthread
#STDLIBFLAGS = 
STDLIBFLAGS = -stdlib=libc++

LIBS = $(SLDALIBS) $(CXXRTLIBS) $(THREADLIBS) $(STDLIBFLAGS)

CXX = clang++ -Wall -pedantic -std=c++11 -I./include -I./src
CXXFLAGS = -O3 -pthread $(STDLIBFLAGS)
#CXXFLAGS = -g -O0 -pthread $(STDLIBFLAGS)
LINKER = clang++ -Wall -pedantic -std=c++11 -I./include -I./src

all: $(SEARCH) $(LEARN) $(LM) $(SLDATEST) $(CLUSTERTEST) $(CLASSIFY) \
	 $(THREADPOOLTEST) $(PARALLELFORTEST) $(LDATEST) $(LDATOPICS)
	@for dir in $(LIBDIRS) ; do make -C $$dir ; done

create_dirs:
	@mkdir -p obj/io obj/model obj/classify obj/util obj/cluster \
		obj/tokenizers obj/index obj/stemmers obj/slda obj/liblinear \
		obj/topics

$(LM): $(OBJ) $(LM).cpp $(TEMPLATES) $(TEMPLATE_HEADERS)
	@echo " Linking \"$@\"..."
	@$(LINKER) -o $@ $(LM).cpp $(OBJ) $(LIBS)

$(CLASSIFY): $(OBJ) $(CLASSIFY).cpp $(TEMPLATES) $(TEMPLATE_HEADERS)
	@echo " Linking \"$@\"..."
	@$(LINKER) -o $@ $(CLASSIFY).cpp $(OBJ) $(LIBS)

$(SEARCH): $(OBJ) $(SEARCH).cpp $(TEMPLATES) $(TEMPLATE_HEADERS)
	@echo " Linking \"$@\"..."
	@$(LINKER) -o $@ $(SEARCH).cpp $(OBJ) $(LIBS)

$(LEARN): $(OBJ) $(LEARN).cpp $(TEMPLATES) $(TEMPLATE_HEADERS)
	@echo " Linking \"$@\"..."
	@$(LINKER) -o $@ $(LEARN).cpp $(OBJ) $(LIBS)

$(SLDATEST): $(OBJ) $(SLDATEST).cpp $(TEMPLATES) $(TEMPLATE_HEADERS)
	@echo " Linking \"$@\"..."
	@$(LINKER) -o $@ $(SLDATEST).cpp $(OBJ) $(LIBS)

$(CLUSTERTEST): $(OBJ) $(CLUSTERTEST).cpp $(TEMPLATES) $(TEMPLATE_HEADERS)
	@echo " Linking \"$@\"..."
	@$(LINKER) -o $@ $(CLUSTERTEST).cpp $(OBJ) $(LIBS)

$(THREADPOOLTEST): $(THREADPOOLTEST).cpp $(TEMPLATES) $(TEMPLATE_HEADERS)
	@echo " Linking \"$@\"..."
	@$(LINKER) -o $@ $(THREADPOOLTEST).cpp $(LIBS)
	
$(PARALLELFORTEST): $(PARALLELFORTEST).cpp $(TEMPLATES) $(TEMPLATE_HEADERS)
	@echo " Linking \"$@\"..."
	@$(LINKER) -o $@ $@.cpp $(LIBS)
	
$(LDATEST): $(OBJ) $(LDATEST).cpp $(TEMPLATES) $(TEMPLATE_HEADERS)
	@echo " Linking \"$@\"..."
	@$(LINKER) -o $@ $(LDATEST).cpp $(OBJ) $(LIBS)
	
$(LDATOPICS): $(OBJ) $(LDATOPICS).cpp $(TEMPLATES) $(TEMPLATE_HEADERS)
	@echo " Linking \"$@\"..."
	@$(LINKER) -o $@ $(LDATOPICS).cpp $(OBJ) $(LIBS)

obj/%.o : lib/%.cpp lib/%.h | create_dirs
	@echo " Compiling $@..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

obj/%.o : src/%.cpp include/%.h | create_dirs
	@echo " Compiling $@..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: doc clean tidy

doc: meta.doxygen
	@echo " Compiling doxygen..."
	@-rm -rf doc
	@-doxygen $< &> /dev/null

clean:
	@for dir in $(LIBDIRS) ; do make -C $$dir clean; done
	-rm -rf obj
	-rm -f $(SEARCH) $(LEARN) $(LM) $(SLDATEST) $(CLUSTERTEST) $(THREADPOOLTEST) $(PARALLELFORTEST) $(LDATEST) $(LDATOPICS) $(CLASSIFY)

tidy:
	-rm -rf ./doc *.chunk postingsFile lexiconFile termid.mapping docid.mapping docs.lengths *compressed.txt *.terms *.phi *.theta
