# Makefile - requires GNU make
# try running gmake instead of make

# directories
BINROOT = /usr/local/bin
MANROOT = /usr/local/man/man1

# tools
CC = gcc
INSTALL = install
MKDIR = mkdir

# flags
CFLAGS = -Wall -I.
LDFLAGS =

### all
all: chunk

### options
opt:
	@echo "### CC      = ${CC}"
	@echo "### CFLAGS  = ${CFLAGS}"
	@echo "### LDFLAGS = ${LDFLAGS}"
	@echo "### BINROOT = ${BINROOT}"
	@echo "### MANROOT = ${MANROOT}"
	@echo "### INSTALL = ${INSTALL}"

### program
chunk: chunk.c
	@echo "### chunk"
	${CC} ${CFLAGS} chunk.c ${LDFLAGS} -o $@

### install
install: chunk
	@echo "### install"
	${MKDIR} -m 0755 -p ${BINROOT}
	${INSTALL} -m 0755 chunk ${BINROOT}/chunk
	${MKDIR} -m 0755 -p ${MANROOT}
	${INSTALL} -m 0644 chunk.1 ${MANROOT}/chunk.1

### uninstall
uninstall:
	@echo "### uninstall"
	rm -f ${BINROOT}/chunk
	rm -f ${MANROOT}/chunk.1

### clean
clean:
	@echo "### clean"
	rm -f chunk
