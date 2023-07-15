
PREFIX = `pwd`/../

LIBNAME1 = comport
LIBNAME2 = atcmd

all: static_lib

static_lib:
	@arm-linux-gnueabihf-gcc -c *.c
	ar -rcs lib${LIBNAME1}.a *.o
	ar -rcs lib${LIBNAME2}.a *.o

clean:
	rm -f *.a
	rm -f *.o

distclean:clean
	rm -f *.a

install:
	mkdir -p ${PREFIX}/lib
	mkdir -p ${PREFIX}/include
	cp lib*.a ${PREFIX}/lib
	cp *.h ${PREFIX}/include

uninstall:
	rm -f ${PREFIX}/lib/lib*


