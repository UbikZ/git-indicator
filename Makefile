CC=gcc

CDIR=src
ODIR=obj
IDIR=include
BDIR=bin

CFLAGS=-I$(IDIR) `pkg-config --libs --cflags gtk+-2.0 appindicator-0.1`

OFILES = indicator.o test.o
OBJ = $(patsubst %,$(ODIR)/%,$(OFILES))

$(ODIR)/%.o: $(CDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

indicator: $(OBJ)
	$(CC) -o $(BDIR)/$@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(BDIR)/* $(ODIR)/*.o *~ core $(IDIR)/*~ 
