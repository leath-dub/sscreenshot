SRC=sss.c

PREFIX=/usr/local
BINDIR=${PREFIX}/bin
MANPREFIX=${PREFIX}/share/man

OBJ = ${SRC:.c=}
CFLAGS=-lxcb-cursor -lxcb-image -lxcb -lxcb-shm -lpng16 -lz
CC=${CC:=gcc}

all: options

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
	@echo "\033[0;31mCFLAGS\033[0;37m = ${CFLAGS}"
	@echo "\033[0;31mCC\033[0;37m     = ${CC}"
	@echo

install:
	${CC} -g -o ${BINDIR}/${OBJ} ${SRC} ${CFLAGS}
	chmod +x ${BINDIR}/${OBJ}
	mkdir -p ${MANPREFIX}/man1/
	cp sss.1 ${MANPREFIX}/man1/

uninstall:
	rm -f ${BINDIR}/${OBJ}
	rm -f ${MANPREFIX}/man1/sss.1

here:
	${CC} -g -o ${OBJ} ${SRC} ${CFLAGS} -DSRC_DIR=\"$(shell pwd)\"
	chmod +x ${OBJ}

link:
	${CC} -g -o ${OBJ} ${SRC} ${CFLAGS} -DSRC_DIR=\"$(shell pwd)\"
	chmod +x ${OBJ}
	ln -sf $(shell pwd)/${OBJ} ${BINDIR}/${OBJ}
	mkdir -p ${MANPREFIX}/man1/
	cp sss.1 ${MANPREFIX}/man1/

unlink:
	rm -f ${BINDIR}/${OBJ}
	rm -f $(shell pwd)/${OBJ}
	rm -f ${MANPREFIX}/man1/sss.1

depend:
	@echo "╭┤ dependencies ├╮"
	@echo "│────────────────│"
	@echo "│ libxcb         │"
	@echo "│ libxcb-image   │"
	@echo "│ libxcb-shm     │"
	@echo "│ libxcb-cursor  │"
	@echo "│ libpng         │"
	@echo "╰────────────────╯"
