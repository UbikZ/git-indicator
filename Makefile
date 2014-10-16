CC=gcc

CDIR=src
ODIR=obj
IDIR=include
BDIR=bin

VERB=-O2 -Wchar-subscripts -Wcomment -Wformat=2 -Wimplicit-int \
-Werror-implicit-function-declaration -Wmain -Wparentheses \
-Wsequence-point -Wreturn-type -Wswitch -Wtrigraphs -Wunused \
-Wuninitialized -Wunknown-pragmas -Wfloat-equal -Wundef \
-Wshadow -Wpointer-arith -Wbad-function-cast -Wwrite-strings \
-Wconversion -Wsign-compare -Waggregate-return -Wstrict-prototypes \
-Wmissing-prototypes -Wmissing-declarations -Wmissing-noreturn \
-Wformat -Wmissing-format-attribute -Wno-deprecated-declarations \
-Wpacked -Wredundant-decls -Wnested-externs -Winline -Wlong-long \
-Wunreachable-code

CFLAGS=$(VERB) -I$(IDIR) `pkg-config --libs --cflags gtk+-2.0 appindicator-0.1`

OFILES = indicator.o test.o
OBJ = $(patsubst %,$(ODIR)/%,$(OFILES))

$(ODIR)/%.o: $(CDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

indicator: $(OBJ)
	$(CC) -o $(BDIR)/$@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(BDIR)/* $(ODIR)/*.o *~ core $(IDIR)/*~ 
