# include the build environment
include ../../common.mk

#include version information
include ../versions.mk

CFLAGS= -g -Wall -I../driver -fPIC -DCOMPILE_TIME=$(shell date +%s)
CFLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\"

SRCS = libvd80.c libvd80.h

all: libvd80.$(CPU).so libvd80.$(CPU).a

libvd80.$(CPU).o: $(SRCS)

libvd80.$(CPU).so: libvd80.$(CPU).o
	$(CC) $(CFLAGS) -o $@ -shared $^

libvd80.$(CPU).a: libvd80.$(CPU).o
	$(AR) $(ARFLAGS) $@ $^
	$(RANLIB) $@

clean:
	$(RM) *.$(CPU).{o,so,a}

cleanall:
	$(RM) *.{$(ALL_CPUS_COMMAS)}.{o,so,a}

LIBS_LIST=libvd80.$(CPU).a

# use default instalation rule for libs
install: install_libs_global

HEADERS_LIST=libvd80.h

# use default instalation rule for headers
install: install_headers_global
