CC = g++ -std=c++0x -Wall
CC_OPTS = -O3
#CC_OPTS = -g -O0
LDFLAGS = -lgsl -lm -lgslcblas

LSOURCE = main.cpp corpus.cpp slda.cpp utils.cpp opt.cpp
LHEADER = corpus.h slda.h utils.h opt.h settings.h document.h

slda: $(LSOURCE) $(LHEADER)
	  $(CC) $(CC_OPTS) $(LSOURCE) -o $@ $(LDFLAGS)

clean:
	-rm -f *.o slda
