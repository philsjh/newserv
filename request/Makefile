include ../build.mk

CFLAGS+=$(INCPGSQL)
LDFLAGS+=$(LIBPGSQL)

.PHONY: all
all: request.so

request.so: lrequest.o request.o request_block.o request_fasttrack.o sqrequest.o
