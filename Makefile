IDIR =include
CC=gcc
CFLAGS=-Wall -Wfatal-errors -Wno-unused-variable -I$(IDIR)

ODIR=obj
# LDIR =lib

DEST = seq-jacobi
# LIBS=-lm


_DEPS = parallel.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = seq-jacobi.o parallel.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(DEST): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f $(ODIR)/*.o *~ $(DEST) $(INCDIR)/*~ 
