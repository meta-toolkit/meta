SEARCH = search
FEATURES = features
LEARN = learn
LM = lm
SLDATEST = slda-test
CLUSTERTEST = cluster-test

SRC = $(wildcard src/io/*.cpp) $(wildcard src/model/*.cpp) $(wildcard src/cluster/*.cpp) \
	  $(wildcard src/classify/*.cpp) $(wildcard src/util/*.cpp) $(wildcard src/tokenizers/*.cpp) \
	  $(wildcard src/index/*.cpp) $(wildcard src/stemmers/*.cpp) $(wildcard lib/slda/*.cpp) \
	  $(wildcard lib/liblinear/*.cpp)

TEMPLATES = $(wildcard src/io/*.tcc) $(wildcard src/model/*.tcc) $(wildcard src/cluster/*.tcc) \
	  $(wildcard src/classify/*.tcc) $(wildcard src/util/*.tcc) $(wildcard src/tokenizers/*.tcc) \
	  $(wildcard src/index/*.tcc) $(wildcard src/stemmers/*.tcc) $(wildcard lib/slda/*.tcc) \
	  $(wildcard lib/liblinear/*.tcc)

OBJ := $(SRC:.cpp=.o)
OBJ := $(shell echo $(OBJ) | sed -e "s/src/obj/g" | sed -e "s/lib/obj/g")

# libraries compiled *separately*
LIBDIRS = lib/liblinear-1.92

SLDALIBS = -lgsl -lm -lgslcblas

CXX = clang++ -Wall -std=c++11 -I./include -I./src
#CXXFLAGS = -g -O0
CXXFLAGS = -O3
LINKER = clang++ -Wall -std=c++11 -I./include -I./src

all: $(SEARCH) $(FEATURES) $(LEARN) $(LM) $(SLDATEST) $(CLUSTERTEST)
	for dir in $(LIBDIRS) ; do make -C $$dir ; done

create_dirs:
	@mkdir -p obj/io obj/model obj/classify obj/util obj/cluster \
		obj/tokenizers obj/index obj/stemmers obj/slda obj/liblinear

$(LM): $(OBJ) $(LM).cpp $(TEMPLATES)
	@echo " Linking \"$@\"..."
	@$(LINKER) -o $@ $(LM).cpp $(OBJ) $(SLDALIBS)

$(SEARCH): $(OBJ) $(SEARCH).cpp $(TEMPLATES)
	@echo " Linking \"$@\"..."
	@$(LINKER) -o $@ $(SEARCH).cpp $(OBJ) $(SLDALIBS)

$(FEATURES): $(OBJ) $(FEATURES).cpp $(TEMPLATES)
	@echo " Linking \"$@\"..."
	@$(LINKER) -o $@ $(FEATURES).cpp $(OBJ) $(SLDALIBS)

$(LEARN): $(OBJ) $(LEARN).cpp $(TEMPLATES)
	@echo " Linking \"$@\"..."
	@$(LINKER) -o $@ $(LEARN).cpp $(OBJ) $(SLDALIBS)

$(SLDATEST): $(OBJ) $(SLDATEST).cpp $(TEMPLATES)
	@echo " Linking \"$@\"..."
	@$(LINKER) -o $@ $(SLDATEST).cpp $(OBJ) $(SLDALIBS)

$(CLUSTERTEST): $(OBJ) $(CLUSTERTEST).cpp $(TEMPLATES)
	@echo " Linking \"$@\"..."
	@$(LINKER) -o $@ $(CLUSTERTEST).cpp $(OBJ) $(SLDALIBS)

obj/%.o : lib/%.cpp lib/%.h | create_dirs
	@echo " Compiling $@..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

obj/%.o : src/%.cpp include/%.h | create_dirs
	@echo " Compiling $@..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	-for dir in $(LIBDIRS) ; do make -C $$dir clean; done
	-rm -rf obj
	-rm -f $(SEARCH) $(FEATURES) $(LEARN) $(LM) $(SLDATEST)

tidy:
	-rm -rf ./doc *.chunk postingsFile lexiconFile termid.mapping docid.mapping docs.lengths *compressed.txt
