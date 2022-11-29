# sss - simple screenshot
# see LICENSE for license info
.POSIX:

include config.mk

SRC = sss.c
OBJ = $(SRC:.c=.o)
# CFLAGS=-lxcb-cursor -lxcb-image -lxcb -lxcb-shm -lpng16 -lz

all: options sss

options:
	@echo
	@printf "\033[0;32m╭───┤ SSS ├──────────╮\n"
	@echo "│────────────────────│"
	@echo "│ \033[0;37msss build options:\033[0;32m │"
	@echo "│   \033[0;37m├───> install\033[0;32m    │"
	@echo "│   \033[0;37m├───> here\033[0;32m       │"
	@echo "│   \033[0;37m├───> link\033[0;32m       │"
	@echo "│   \033[0;37m├───> unlink\033[0;32m     │"
	@echo "│   \033[0;37m└───> uninstall\033[0;32m  │"
	@echo "╰────────────────────╯\033[0;37m"
	@echo
	@echo "\033[0;31mCFLAGS\033[0;37m  = $(STCFLAGS)"
	@echo "\033[0;31mLDFLAGS\033[0;37m = $(STLDFLAGS)"
	@echo "\033[0;31mCC\033[0;37m      = $(CC)"
	@echo

sss: $(OBJ)
	$(CC) -o $@ $(OBJ) $(STLDFLAGS)

install: sss
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f sss $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/sss
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	cp sss.1 $(DESTDIR)$(MANPREFIX)/man1/sss.1
	chmod 644 $(DESTDIR)$(MANPREFIX)/man1/sss.1

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/sss
	rm -f $(DESTDIR)$(MANPREFIX)/man1/sss.1

here: sss

link: sss
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	ln -sf $(shell pwd)/sss $(DESTDIR)$(PREFIX)/bin/sss
	chmod 755 sss
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	ln -sf $(shell pwd)/sss.1 $(DESTDIR)$(MANPREFIX)/man1/sss.1
	chmod 644 sss.1

unlink:
	rm -f $(DESTDIR)$(PREFIX)/bin/sss
	rm -f $(DESTDIR)$(MANPREFIX)/man1/sss.1

depend:
	@echo "╭┤ dependencies ├╮"
	@echo "│────────────────│"
	@echo "│ libxcb         │"
	@echo "│ libxcb-image   │"
	@echo "│ libxcb-shm     │"
	@echo "│ libxcb-cursor  │"
	@echo "│ libpng         │"
	@echo "╰────────────────╯"

.PHONY: install here link unlink uninstall depend
