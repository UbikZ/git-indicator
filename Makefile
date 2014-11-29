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

.PHONY: clean

install:
	[ ! -d "~/.$(NAME)" ] && mkdir ~/.$(NAME)
	[ ! -f "~/.$(NAME)/.conf" ] && ln -s `pwd`/$(BDIR)/$(NAME) ~/.$(NAME) && \
	find ~/ -type d -name '*.git' | \
	egrep -v '(bundle|tests|vendor|.composer)' > ~/.$(NAME)/.conf

clean:
	rm -f $(BDIR)/* $(ODIR)/*.o *~ core $(IDIR)/*~ _*
	[ -d ~/.$(NAME) ] && rm -Rf ~/.$(NAME)
