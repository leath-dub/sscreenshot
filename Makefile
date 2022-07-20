SRC=sss.c
BINDIR=/usr/local/bin
OBJ = ${SRC:.c=}
CFLAGS=-lxcb-image -lxcb -lxcb-shm -lpng16 -lz
CC=gcc

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
	${CC} -g -o ${BINDIR}/${OBJ} ${SRC} ${CFLAGS} -DSRC_DIR=\"$(shell pwd)\"
	chmod +x ${BINDIR}/${OBJ}

uninstall:
	rm -f ${BINDIR}/${OBJ}

here:
	${CC} -g -o ${OBJ} ${SRC} ${CFLAGS} -DSRC_DIR=\"$(shell pwd)\"
	chmod +x ${OBJ}

link:
	${CC} -g -o ${OBJ} ${SRC} ${CFLAGS} -DSRC_DIR=\"$(shell pwd)\"
	chmod +x ${OBJ}
	ln -s $(shell pwd)/${OBJ} ${BINDIR}/${OBJ}

unlink:
	rm -f ${BINDIR}/${OBJ}
	rm -f $(shell pwd)/${OBJ}
