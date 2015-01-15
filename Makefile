# Compiler
CC=gcc

NAME=git-indicator

# Src directories
ADIR=assets
CDIR=src
ODIR=obj
IDIR=include
BDIR=bin

# Dest directories
DBIN=/usr/local/bin
DSHARE=/usr/local/share
DAUTO=.config/autostart

# Options
VERB=-Wall
CFLAGS=-lpthread -g $(VERB) -I$(IDIR) \
`pkg-config --libs --cflags gtk+-2.0 appindicator-0.1 libgit2 libnotify glib-2.0 libssh2`

# Object files
OFILES = main.o git.o indicator.o file.o notify-osd.o
OBJ = $(patsubst %,$(ODIR)/%,$(OFILES))

# Cmd
$(ODIR)/%.o: $(CDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(NAME): $(OBJ)
	$(CC) -o $(BDIR)/$@ $^ $(CFLAGS)

configure:
	# Create home application directory
	[ ! -d ~/.$(NAME) ] && mkdir ~/.$(NAME)
	# Copy autostart configuration
	[ ! -f ~/.config/autostart/$(NAME).desktop ] && \
	cp $(NAME).desktop ~/.config/autostart
	# Create configuration file with repositories
	[ ! -f ~/.$(NAME)/.conf ] && \
	find ~/ -type d -name '*.git' | sed "s/\.git//g" | \
	egrep -v '(bundle|tests|vendor|.composer)' > ~/.$(NAME)/.conf
	# Copy assets in /usr/local/...
	[ ! -d $(DSHARE)/$(NAME)/img ] && mkdir -p $(DSHARE)/$(NAME)/img && \
	cp $(ADIR)/* $(DSHARE)/$(NAME)/img
	# Copy binary in /usr/local
	[ ! -d $(DSHARE)/$(NAME)/bin ] && mkdir -p $(DSHARE)/$(NAME)/bin && \
	cp $(BDIR)/* $(DSHARE)/$(NAME)/bin

install:
	[ ! -f $(DBIN)/$(NAME) ] && cp $(NAME) $(DBIN)/$(NAME)

uninstall:
	[ -d ~/.$(NAME) ] && rm -Rf ~/.$(NAME)
	[ -f ~/.config/autostart/$(NAME).desktop ] && rm ~/.config/autostart/$(NAME).desktop
	[ -f $(DBIN)/$(NAME) ] && rm $(DBIN)/$(NAME)
	[ -d $(DSHARE)/$(NAME) ] && rm -rf $(DSHARE)/$(NAME)

clean:
	rm -f $(BDIR)/* $(ODIR)/*.o *~ $(IDIR)/*~ _*

.PHONY: configure install uninstall clean
