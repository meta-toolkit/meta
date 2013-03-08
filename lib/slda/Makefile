CC = g++ -std=c++0x -Wall
CCOPTS = -O3
#CCOPTS = -g -O0
LIBS = -lgsl -lm -lgslcblas
OBJS = corpus.o slda.o utils.o opt.o

slda: $(OBJS) main.cpp document.h settings.h
	  $(CC) $(CCOPTS) main.cpp -o $@ $(OBJS) $(LIBS)

%.o : %.cpp $(wildcard *.h)
	$(CC) $(CCOPTS) -c $(@:.o=.cpp) -o $@ $(LIBS)

clean:
	-rm -f *.o slda
