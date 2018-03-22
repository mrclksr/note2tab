PROGRAM  = note2tab
MANFILE	 = ${PROGRAM}.1
PREFIX   = /usr/local
BINDIR	 = ${PREFIX}/bin
MANDIR	 = ${PREFIX}/man/man1
CFLAGS	+= -Wall -DPROGRAM=\"${PROGRAM}\"

all: ${PROGRAM}

${PROGRAM}: ${PROGRAM}.c
	${CC} -o $@ ${CFLAGS} $<

install: ${PROGRAM} ${MANFILE}
	if [ ! -d ${BINDIR} ]; then mkdir -p ${BINDIR}; fi
	if [ ! -d ${MANDIR} ]; then mkdir -p ${MANDIR}; fi
	install -g 0 -m 0755 -o root ${PROGRAM} ${BINDIR}
	install -g 0 -m 0644 -o root ${MANFILE} ${MANDIR}

clean:
	-rm -f ${PROGRAM}

