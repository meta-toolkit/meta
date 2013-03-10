AR     = ar rcv
RANLIB = ranlib 

HEADERS = blas.h blasp.h
FILES = dnrm2.o daxpy.o ddot.o dscal.o 

CFLAGS = $(OPTFLAGS) 
FFLAGS = $(OPTFLAGS)

blas: $(FILES) $(HEADERS)
	@echo " Creating archive \"$@\"..."
	@$(AR) blas.a $(FILES) > /dev/null
	@$(RANLIB) blas.a > /dev/null

clean:
	- rm -f *.o
	- rm -f *.a
	- rm -f *~

.c.o:
	@echo " Compiling lib/liblinear/blas/$@..."
	@$(CC) $(CFLAGS) -c $*.c


