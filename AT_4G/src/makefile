
PREFIX = `pwd`/../

LIBNAME1 = comport
LIBNAME2 = atcmd
LIBNAME3 = network
LIBNAME4 = gpio_l

CFLAG= /home/hubenyuan/my_libgpiod/install/include
LDFLAG= /home/hubenyuan/my_libgpiod/install/bin

AR = /opt/buildroot/gcc-5.5-arm926t-2019.02/bin/arm-linux-gcc-ar
CC = /opt/buildroot/gcc-5.5-arm926t-2019.02/bin/arm-linux-gcc

all: static_lib

static_lib:
	${CC} -I ${CFLAG} -L ${LDFLAG} -c *.c -lgpiod
	${AR} -rcs lib${LIBNAME1}.a *.o 
	${AR} -rcs lib${LIBNAME2}.a *.o
	${AR} -rcs lib${LIBNAME3}.a *.o
	${AR} -rcs lib${LIBNAME4}.a *.o

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


