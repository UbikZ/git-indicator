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
DAUTO=~/.config/autostart

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
	mkdir ~/.$(NAME)

	# Create configuration file with repositories
	find ~/ -type d -name '*.git' | sed "s/\.git//g" | \
	egrep -v '(bundle|tests|vendor|.composer)' > ~/.$(NAME)/.conf

	# Copy autostart configuration
	cp $(NAME).desktop $(DAUTO)

install:
	# Copy bash script
	cp $(NAME) $(DBIN)/$(NAME)

	# Copy assets in /usr/local/...
	mkdir -p $(DSHARE)/$(NAME)/img && cp $(ADIR)/* $(DSHARE)/$(NAME)/img

	# Copy binary in /usr/local
	mkdir -p $(DSHARE)/$(NAME)/bin && cp $(BDIR)/* $(DSHARE)/$(NAME)/bin

uninstall:
	rm -Rf ~/.$(NAME) $(DBIN)/$(NAME) $(DAUTO)/$(NAME).desktop $(DSHARE)/$(NAME)

clean:
	rm -f $(BDIR)/* $(ODIR)/*.o *~ $(IDIR)/*~ _*

.PHONY: configure install uninstall clean
