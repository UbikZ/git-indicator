CC=gcc

NAME=git-indicator

CDIR=src
ODIR=obj
IDIR=include
BDIR=bin

VERB=-Wall

CFLAGS=-lpthread -g $(VERB) -I$(IDIR) \
`pkg-config --libs --cflags gtk+-2.0 appindicator-0.1 libgit2`

OFILES = main.o git.o indicator.o file.o
OBJ = $(patsubst %,$(ODIR)/%,$(OFILES))

$(ODIR)/%.o: $(CDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(NAME): $(OBJ)
	$(CC) -o $(BDIR)/$@ $^ $(CFLAGS)
	[ ! -d "~/$@" ] && mkdir ~/$@
	ln -s `pwd`/$(BDIR)/$@ ~/$@ && touch ~/$@/.conf

.PHONY: clean

clean:
	rm -f $(BDIR)/* $(ODIR)/*.o *~ core $(IDIR)/*~ _*
	[ -d ~/$(NAME) ] && rm -Rf ~/$(NAME)
