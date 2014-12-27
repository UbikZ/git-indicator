CC=gcc

NAME=git-indicator

CDIR=src
ODIR=obj
IDIR=include
BDIR=bin

VERB=-Wall

CFLAGS=-lpthread -g $(VERB) -I$(IDIR) \
`pkg-config --libs --cflags gtk+-2.0 appindicator-0.1 libgit2 libnotify glib-2.0 libssh2`

OFILES = main.o git.o indicator.o file.o notify-osd.o
OBJ = $(patsubst %,$(ODIR)/%,$(OFILES))

$(ODIR)/%.o: $(CDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(NAME): $(OBJ)
	$(CC) -o $(BDIR)/$@ $^ $(CFLAGS)
	ln -s `pwd` ~/.$@/app

configure:
	[ ! -d ~/.$(NAME) ] && mkdir ~/.$(NAME)
	[ ! -f ~/.config/autostart/$(NAME).desktop ] && \
	cp $(NAME).desktop ~/.config/autostart
	[ ! -f ~/.$(NAME)/.conf ] && \
	find ~/ -type d -name '*.git' | sed "s/\.git//g" | \
	egrep -v '(bundle|tests|vendor|.composer)' > ~/.$(NAME)/.conf

install:
	[ ! -f /usr/local/bin/$(NAME) ] && cp $(NAME) /usr/local/bin/$(NAME)

uninstall:
	[ -d ~/.$(NAME) ] && rm -Rf ~/.$(NAME)
	[ -f ~/.config/autostart/$(NAME).desktop ] && rm ~/.config/autostart/$(NAME).desktop
	[ -f /usr/local/bin/$(NAME) ] && rm /usr/local/bin/$(NAME)

clean:
	rm -f $(BDIR)/* $(ODIR)/*.o *~ $(IDIR)/*~ _*

.PHONY: configure install uninstall clean
