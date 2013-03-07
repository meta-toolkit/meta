CC = g++
LDFLAGS = -lgsl -lm -lgslcblas

LSOURCE = main.cpp corpus.cpp slda.cpp utils.cpp opt.cpp
LHEADER = corpus.h slda.h utils.h opt.h settings.h

slda: $(LSOURCE) $(HEADER)
	  $(CC) $(LSOURCE) -o $@ $(LDFLAGS)

clean:
	-rm -f *.o slda
