IDIR =../include
CC=gcc
CFLAGS=-Wall -Wfatal-errors -Wno-unused-variable -I$(IDIR)

ODIR=obj
LDIR =../lib

COMPILE=seq-jacobi
LIBS=-lm


_DEPS = 
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = seq-jacobi.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(COMPILE): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f $(ODIR)/*.o *~  $(INCDIR)/*~ 
